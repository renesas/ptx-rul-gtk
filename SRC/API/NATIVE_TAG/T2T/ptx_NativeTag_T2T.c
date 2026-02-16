/*
    ---------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause

    Copyright (c) 2025, Renesas Electronics Corporation and/or its affiliates


    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list of
       conditions and the following disclaimer in the documentation and/or other
       materials provided with the distribution.

    3. Neither the name of Renesas nor the names of its
       contributors may be used to endorse or promote products derived from this
       software without specific prior written permission.



    THIS SOFTWARE IS PROVIDED BY Renesas "AS IS" AND ANY EXPRESS
    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL RENESAS OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    ---------------------------------------------------------------

    Project     : PTX2xxR/W
    Module      : API for interfacing with a Type 2 Tag (T2T) card
    File        : ptx_NativeTag_T2T.c

    Description : NativeTag T2T implements the interface to communicate with a T2T NFC card.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NativeTag_T2T.h"
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
/*
 * Command codes and ACK.
 *
 */
#define PTX_T2TOP_READ_CODE                                 (uint8_t)0x30 /* OpCode Read. */
#define PTX_T2TOP_WRITE_CODE                                (uint8_t)0xA2 /* OpCode Write. */
#define PTX_T2TOP_SECTOR_SELECT_CODE                        (uint8_t)0xC2 /* OpCode Sector Select. */
#define PTX_T2TOP_ACK_CODE                                  (uint8_t)0x0A /* T2T acknowledge. */
/*  */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 *  Create the T2T Command Header.
 *
 *  t2t                   Pointer to an existing instance of the T2T component.
 *  commandCode           Command Code to be set.
 *  blockNr               Block Number to be set.
 *  bytesWritten      Current Tx Buffer Index on input, Tx Buffer index after adding Command header on output.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NativeTag_T2T_SetCommandHeader(ptx_NativeTag_T2T_t *t2t, uint8_t commandCode, uint8_t blockNr, uint32_t *bytesWritten);

/*
 *  Data Exchange with a T2T.
 *
 *  t2t                   Pointer to an existing instance of the T2T component.
 *  txLen                 Length of Component-Internal Tx Buffer Data.
 *  rx                Rx Buffer to store response.
 *  rxLen             Maximum Rx Buffer length on input, Response length on output.
 *  msTimeout             Timeout in ms.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NativeTag_T2T_Transceive(ptx_NativeTag_T2T_t *t2t, uint32_t txLen, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NativeTag_T2T_Open (ptx_NativeTag_T2T_t *t2t, ptx_NativeTag_T2T_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != t2t) && (NULL != initParams))
    {
        if ((NULL != initParams->rul) && (NULL != initParams->TxBuffer) && (PTX_T2T_MIN_TX_BUFFER_SIZE <= initParams->TxBufferSize))
        {
            /* clear component */
            (void)memset(t2t, 0, sizeof(ptx_NativeTag_T2T_t));

            /* set members */
            t2t->CompId = Comp_NativeTag_T2T;
            t2t->rul = initParams->rul;
            t2t->TxBuffer = initParams->TxBuffer;

            /* Communicate success to upper layers. */
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T2T_Read (ptx_NativeTag_T2T_t *t2t,
                                     uint8_t blockNr,
                                     uint8_t *rx,
                                     uint32_t *rxLen,
                                     uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;

    if (PTX_COMP_CHECK(t2t, Comp_NativeTag_T2T) && (NULL != rx) && (NULL != rxLen))
    {
        /* set T2T command header */
        status = ptx_NativeTag_T2T_SetCommandHeader(t2t, PTX_T2TOP_READ_CODE, blockNr, &tx_index);
        /* everything ok so far ? - send data*/
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T2T_Transceive(t2t, tx_index, rx, rxLen, msTimeout);
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T2T_Write (ptx_NativeTag_T2T_t *t2t,
                                      uint8_t blockNr,
                                      uint8_t *blockData,
                                      uint8_t blockDataLen,
                                      uint8_t *rx,
                                      uint32_t *rxLen,
                                      uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;

    if (PTX_COMP_CHECK(t2t, Comp_NativeTag_T2T) && (NULL != blockData) && (NULL != rx) && (NULL != rxLen))
    {
        /* set T2T command header */
        status = ptx_NativeTag_T2T_SetCommandHeader(t2t, PTX_T2TOP_WRITE_CODE, blockNr, &tx_index);
        /* everything ok so far ? - send data*/
        if (PTX_STATUS_SUCCESS == status)
        {
            (void)memcpy(&t2t->TxBuffer[tx_index], blockData, blockDataLen);
            tx_index = (uint32_t)(tx_index + blockDataLen);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T2T_Transceive(t2t, tx_index, rx, rxLen, msTimeout);

            /* If residual bits have been received, which is required when writing to the T2T
             * (receives a 4 bit long ACK/NACK response), set the status to success.
             */
            if (PTX_STATUS_E_NSC_RESIDUAL_BITS == status)
            {
                status = PTX_STATUS_SUCCESS;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T2T_SectorSelect (ptx_NativeTag_T2T_t *t2t,
                                             uint8_t secNr,
                                             uint8_t *rx,
                                             uint32_t *rxLen,
                                             uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;
    uint32_t toClear;

    if (PTX_COMP_CHECK(t2t, Comp_NativeTag_T2T) && (NULL != rx) && (NULL != rxLen) && (0xFF > secNr))
    {
        /* sector select does not utilize command header since it does not use a block number */
        /* prepare first packet */
        t2t->TxBuffer[tx_index] = PTX_T2TOP_SECTOR_SELECT_CODE;
        tx_index++;
        t2t->TxBuffer[tx_index] = 0xFF;
        tx_index++;

        /* everything ok so far ? - send first packet*/
        status = ptx_NativeTag_T2T_Transceive(t2t, tx_index, rx, rxLen, msTimeout);

        if (((PTX_STATUS_SUCCESS == status) || (PTX_STATUS_E_NSC_RESIDUAL_BITS == status)) && (rx[0] == PTX_T2TOP_ACK_CODE))
        {
            tx_index = 0;

            /* clear the buffer for second packet */
            toClear = sizeof(t2t->TxBuffer);
            (void)memset(t2t->TxBuffer, 0, toClear);

            /* prepare second packet */
            t2t->TxBuffer[tx_index] = secNr;
            tx_index++;
            t2t->TxBuffer[tx_index] = 0x00; // 2nd byte is RFU
            tx_index++;
            t2t->TxBuffer[tx_index] = 0x00; // 3rd byte is RFU
            tx_index++;
            t2t->TxBuffer[tx_index] = 0x00; // 4th byte is RFU
            tx_index++;

            /* send second packet */
            status = ptx_NativeTag_T2T_Transceive(t2t, tx_index, rx, rxLen, msTimeout);

            /* check PAT */
            if (((PTX_STATUS_E_NSC_RFERROR == status) || (PTX_STATUS_E_GEN_TIMEOUT == status)) && (*rxLen == 0))
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T2T_Close (ptx_NativeTag_T2T_t *t2t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(t2t, Comp_NativeTag_T2T))
    {
        memset(t2t, 0, sizeof(ptx_NativeTag_T2T_t));
        t2t->CompId = Comp_None;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NativeTag_T2T_SetCommandHeader(ptx_NativeTag_T2T_t *t2t, uint8_t commandCode, uint8_t blockNr, uint32_t *bytesWritten)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t index = 0;

    if (PTX_COMP_CHECK(t2t, Comp_NativeTag_T2T) && (NULL != bytesWritten))
    {
        /* set command code */
        t2t->TxBuffer[index] = commandCode;
        index++;

        /* set block number */
        t2t->TxBuffer[index] = blockNr;
        index++;

        *bytesWritten = index;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}
static ptx_Status_t ptx_NativeTag_T2T_Transceive(ptx_NativeTag_T2T_t *t2t, uint32_t txLen, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t timeout_value = msTimeout;

    /* Aside from actual Tag-component, remaining parameters are checked within Data-Exchange function */
    if ((PTX_COMP_CHECK(t2t, Comp_NativeTag_T2T)) && (NULL != rx) && (NULL != rxLen))
    {
        /* send command */
        status = ptx_RUL_Exchange (t2t->rul, &t2t->TxBuffer[0], txLen, rx, rxLen, timeout_value);

        /* Accept the received data as valid if the status is success or
         * if residual bits have been received, which is required when writing to the T2T
         * and receiving a 4 bit long ACK/NACK response.
         */
        if ((PTX_STATUS_SUCCESS != status) && (PTX_STATUS_E_NSC_RESIDUAL_BITS != status))
        {
            *rxLen = 0;
        }
    }

    return status;
}


