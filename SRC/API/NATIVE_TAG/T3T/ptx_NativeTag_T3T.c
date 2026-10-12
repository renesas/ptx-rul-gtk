/*
    ---------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause

    Copyright (c) 2026, Renesas Electronics Corporation and/or its affiliates


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
    Module      : API for interfacing with a Type 3 Tag (T3T) card
    File        : ptx_NativeTag_T3T.c

    Description : NativeTag T3T implements the interface to communicate with a T3T NFC card.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL.h"
#include "ptx_NativeTag_T3T.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
/*
 * Communication Defines
 *
 */
#define PTX_T3T_SENSF_REQ_CODE                          (uint8_t)0x00    /* SENSF-REQ Code. */
#define PTX_T3T_SENSF_REQ_SC_NDEF                       (uint16_t)0x12FC /* SENSF-REQ Service Code. */
#define PTX_T3T_CHECK_CODE                              (uint8_t)0x06    /* OpCode Check. */
#define PTX_T3T_UPDATE_CODE                             (uint8_t)0x08    /* OpCode Update. */
#define PTX_T3T_NDEF_BLOCKS_PER_OP                      (uint8_t)0x01    /* Blocks to be handled per operation in special cases. */
/*  */
/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / HELPERS
 * ####################################################################################################################
 */
/*
 *  Data Exchange with a T3T.
 *
 *  t3t                   Pointer to component.
 *  tx                    Tx Buffer containing Data to be sent.
 *  txLen                 Length of Component-Internal Tx Buffer Data.
 *  rx                Rx Buffer to store response.
 *  rxLen             Maximum Rx Buffer length on input, Response length on output.
 *  msTimeout             Timeout in ms.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NativeTag_T3T_Transceive (ptx_NativeTag_T3T_t *t3t,
                                                  uint8_t *tx,
                                                  uint32_t txLen,
                                                  uint8_t *rx,
                                                  uint32_t *rxLen,
                                                  uint32_t msTimeout);

/*
 *  Create the T3T Command Header.
 *
 *  t3t                   Pointer to an existing instance of the T2T component.
 *  commandCode           Command Code to be set.
 *  NFCID2                ID to address the T3T tag.
 *  NFCID2Len             Length of ID.
 *  serviceInfo           Information about Services to be updated.
 *  blockInfo             Information about Blocks to be updated.
 *  bytesWritten      Current Tx Buffer Index on input, Tx Buffer index after adding Command header on output.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NativeTag_T3T_SetCommandHeader (ptx_NativeTag_T3T_t *t3t,
                                                        uint8_t commandCode,
                                                        uint8_t *NFCID2,
                                                        uint8_t NFCID2Len,
                                                        ptx_NativeTag_T3T_Services_t serviceInfo,
                                                        ptx_NativeTag_T3T_Blocks_t blockInfo,
                                                        uint8_t *bytesWritten);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NativeTag_T3T_Open (ptx_NativeTag_T3T_t *t3t, ptx_NativeTag_T3T_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != t3t) && (NULL != initParams))
    {
        if ((NULL != initParams->rul) && (NULL != initParams->TxBuffer) && (PTX_T3T_MIN_TX_BUFFER_SIZE <= initParams->TxBufferSize))
        {
            /* clear component */
            (void)memset(t3t, 0, sizeof(ptx_NativeTag_T3T_t));

            /* set members */
            t3t->CompId = Comp_NativeTag_T3T;
            t3t->rul = initParams->rul;
            t3t->TxBuffer = initParams->TxBuffer;
            t3t->MRTI_Check = initParams->MRTI_Check;
            t3t->MRTI_Update = initParams->MRTI_Update;
            if (PTX_T3T_NFCID2_SIZE == initParams->NFCID2Len)
            {
                memcpy(t3t->NFCID2, initParams->NFCID2, PTX_T3T_NFCID2_SIZE);
            }
            else
            {
                /* ignore, set NFCID2 later */
                memset(t3t->NFCID2, 0, PTX_T3T_NFCID2_SIZE);
            }

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

ptx_Status_t ptx_NativeTag_T3T_SetTagParams (ptx_NativeTag_T3T_t *t3t,
                                             uint8_t *NFCID2,
                                             uint8_t NFCID2Len,
                                             uint8_t nbr,
                                             uint8_t nbw,
                                             ptx_NativeTag_T3T_MRTI_t mrtiInfo)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t mask_a = 0x07;
    uint8_t mask_b = 0x38;
    uint8_t mask_e = 0xC0;
    uint8_t nbr_check_blocks = nbr;
    uint8_t nbr_update_blocks = nbw;
    uint32_t timeout_check;
    uint32_t timeout_update;
    uint16_t t_t3t = 1; /* assumed as 1 since it is usually below that */
    uint16_t last_comp_check = 1;
    uint16_t last_comp_update = 1;

    if (PTX_COMP_CHECK(t3t, Comp_NativeTag_T3T))
    {
        /* set NFCID2 */
        if ((PTX_T3T_NFCID2_SIZE == NFCID2Len) && (NULL != NFCID2))
        {
            memcpy(t3t->NFCID2, NFCID2, PTX_T3T_NFCID2_SIZE);
        }
        else
        {
            memset(t3t->NFCID2, 0, PTX_T3T_NFCID2_SIZE);
        }

        /* 4^n */
        for (uint8_t i = 0; i < ((mrtiInfo.MRTICheck & mask_e) >> 6u); i++)
        {
            last_comp_check = (uint16_t)(last_comp_check * 4u);
        }
        for (uint8_t i = 0; i < ((mrtiInfo.MRTIUpdate & mask_e) >> 6u); i++)
        {
            last_comp_update = (uint16_t)(last_comp_update * 4u);
        }

        /* set timeout value */
        timeout_check = t_t3t * ((((mrtiInfo.MRTICheck & mask_a) +1u) + nbr_check_blocks * (((mrtiInfo.MRTICheck & mask_b) >> 3u) +1u)) * last_comp_check);
        timeout_update = t_t3t * ((((mrtiInfo.MRTIUpdate & mask_a) +1u) + nbr_update_blocks * (((mrtiInfo.MRTIUpdate & mask_b) >> 3u) +1u)) * last_comp_update);

        timeout_check += (20 * nbr_check_blocks);        /* added to avoid floating point */
        timeout_update += (20 * nbr_update_blocks);

        t3t->TagTimeoutCheck = timeout_check;
        t3t->TagTimeoutUpdate = timeout_update;
        status = PTX_STATUS_SUCCESS;
    }

    return status;

}

ptx_Status_t ptx_NativeTag_T3T_SENSF_REQ (ptx_NativeTag_T3T_t *t3t,
                                          uint16_t sc,
                                          uint8_t rc,
                                          uint8_t tsn,
                                          uint8_t *rx,
                                          uint32_t *rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(t3t, Comp_NativeTag_T3T) && (NULL != rx) && (NULL != rxLen))
    {
        status = ptx_RUL_T3T_SensFReq(t3t->rul, sc, rc, tsn, rx, rxLen);
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T3T_Check (ptx_NativeTag_T3T_t *t3t,
                                      ptx_NativeTag_T3T_Services_t serviceInfo,
                                      ptx_NativeTag_T3T_Blocks_t blockInfo,
                                      uint8_t *rx,
                                      uint32_t *rxLen,
                                      uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t tx_index = 0;

    if (PTX_COMP_CHECK(t3t, Comp_NativeTag_T3T) && (NULL != serviceInfo.ServiceCodeList) && (NULL != blockInfo.BlockList) && (NULL != rx) && (NULL != rxLen))
    {
        status = ptx_NativeTag_T3T_SetCommandHeader(t3t, PTX_T3T_CHECK_CODE, t3t->NFCID2, PTX_T3T_NFCID2_SIZE, serviceInfo, blockInfo, &tx_index);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* no need to add the block data field, send data */
            status = ptx_NativeTag_T3T_Transceive(t3t, &t3t->TxBuffer[0], tx_index, rx, rxLen, msTimeout);
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T3T_Update (ptx_NativeTag_T3T_t *t3t,
                                       ptx_NativeTag_T3T_Services_t serviceInfo,
                                       ptx_NativeTag_T3T_Blocks_t blockInfo,
                                       uint8_t *blockData,
                                       uint8_t blockDataLen,
                                       uint8_t *rx,
                                       uint32_t *rxLen,
                                       uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t tx_index = 0;

    if (PTX_COMP_CHECK(t3t, Comp_NativeTag_T3T) && (NULL != serviceInfo.ServiceCodeList) && (NULL != blockInfo.BlockList) && (NULL != rx) && (NULL != rxLen) && (NULL != blockData))
    {
        status = ptx_NativeTag_T3T_SetCommandHeader(t3t, PTX_T3T_UPDATE_CODE, t3t->NFCID2, PTX_T3T_NFCID2_SIZE, serviceInfo, blockInfo, &tx_index);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* add the block data to the tx buffer */
            (void)memcpy(&t3t->TxBuffer[tx_index], &blockData[0], (uint32_t)blockDataLen);
            tx_index = (uint8_t)(tx_index + blockDataLen);

            /* send data */
            status = ptx_NativeTag_T3T_Transceive(t3t, &t3t->TxBuffer[0], tx_index, rx, rxLen, msTimeout);
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T3T_Close (ptx_NativeTag_T3T_t *t3t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(t3t, Comp_NativeTag_T3T))
    {
        memset(t3t, 0, sizeof(ptx_NativeTag_T3T_t));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK(s)
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NativeTag_T3T_Transceive (ptx_NativeTag_T3T_t *t3t,
                                                  uint8_t *tx,
                                                  uint32_t txLen,
                                                  uint8_t *rx,
                                                  uint32_t *rxLen,
                                                  uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(t3t, Comp_NativeTag_T3T) && (NULL != tx) && (0 != txLen) && (NULL != rx) && (0 != rxLen))
    {
        tx[0] = (uint8_t)txLen;
        status = ptx_RUL_Exchange (t3t->rul, tx, txLen, rx, rxLen, msTimeout);

        if (PTX_STATUS_SUCCESS != status)
        {
            *rxLen = 0;
        }
    }

    return status;
}

static ptx_Status_t ptx_NativeTag_T3T_SetCommandHeader (ptx_NativeTag_T3T_t *t3t,
                                                        uint8_t commandCode,
                                                        uint8_t *NFCID2,
                                                        uint8_t NFCID2Len,
                                                        ptx_NativeTag_T3T_Services_t serviceInfo,
                                                        ptx_NativeTag_T3T_Blocks_t blockInfo,
                                                        uint8_t *bytesWritten)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t tx_index = 0;

    if (PTX_COMP_CHECK(t3t, Comp_NativeTag_T3T) && (NULL != bytesWritten) && (PTX_T3T_NFCID2_SIZE == NFCID2Len) && (NULL != NFCID2) && (NULL != serviceInfo.ServiceCodeList) && (NULL != blockInfo.BlockList))
    {
        /* length byte prepended later */
        tx_index++;
        t3t->TxBuffer[tx_index] = commandCode;
        tx_index++;
        (void)memcpy(&t3t->TxBuffer[tx_index], &NFCID2[0], NFCID2Len);
        tx_index = (uint8_t)(tx_index + NFCID2Len);
        t3t->TxBuffer[tx_index] = serviceInfo.NOS;
        tx_index++;
        (void)memcpy(&t3t->TxBuffer[tx_index], &serviceInfo.ServiceCodeList[0], (uint32_t)serviceInfo.ServiceCodeListLen);
        tx_index = (uint8_t)(tx_index + serviceInfo.ServiceCodeListLen);
        t3t->TxBuffer[tx_index] = blockInfo.NOB;
        tx_index++;
        (void)memcpy(&t3t->TxBuffer[tx_index], &blockInfo.BlockList[0], (uint32_t)blockInfo.BlockListLen);
        tx_index = (uint8_t)(tx_index + blockInfo.BlockListLen);

        *bytesWritten = tx_index;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

