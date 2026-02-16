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
    Module      : API for interfacing with a Type 5 Tag (T5T) card
    File        : ptx_NativeTag_T5T.c

    Description : NativeTag T5T implements the interface to communicate with a T5T NFC card.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NativeTag_T5T.h"
#include <string.h>
#include <stddef.h>
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
/*
 *  T5T Native Tag Component
 */
typedef enum ptx_NativeTag_T5TFrameType
{
    T5T_FrameType_Standard,         /* Standard frame type. */
    T5T_FrameType_Special,          /* Special frame type. */
} ptx_NativeTag_T5TFrameType;

/*
 * T5T Request Codes
 *
 */
#define PTX_T5T_READ_SINGLE_BLOCK_REQ_CODE              (uint8_t)0x20   /* Request Code Read Single Block. */
#define PTX_T5T_WRITE_SINGLE_BLOCK_REQ_CODE             (uint8_t)0x21   /* Request Code Write Single Block. */
#define PTX_T5T_LOCK_SINGLE_BLOCK_REQ_CODE              (uint8_t)0x22   /* Request Code Lock Single Block. */
#define PTX_T5T_READ_MULTIPLE_BLOCK_REQ_CODE            (uint8_t)0x23   /* Request Code Read Multiple Blocks. */
#define PTX_T5T_EXT_READ_SINGLE_BLOCK_REQ_CODE          (uint8_t)0x30   /* Request Code Read Single Block for extended memory layout. */
#define PTX_T5T_EXT_WRITE_SINGLE_BLOCK_REQ_CODE         (uint8_t)0x31   /* Request Code Write Single Block for extended memory layout. */
#define PTX_T5T_EXT_LOCK_SINGLE_BLOCK_REQ_CODE          (uint8_t)0x32   /* Request Code Lock Single Block for extended memory layout. */
#define PTX_T5T_EXT_READ_MULTIPLE_BLOCK_REQ_CODE        (uint8_t)0x33   /* Request Code Read Multiple Blocks for extended memory layout. */
#define PTX_T5T_SELECT_REQ_CODE                         (uint8_t)0x25   /* Request Code for tag Select. */
#define PTX_T5T_SLPV_REQ_CODE                           (uint8_t)0x02   /* Request Code for tag Sleep. */
/*  */

/*
 * Request Flag Masks
 *
 */
#define PTX_T5T_COMMAND_CODE_FRAME_OFFSET               (uint8_t)0x01   /* Command frame offset. */
#define PTX_T5T_REQ_FLAG_OPTION_BIT_SET_MASK            (uint8_t)0x40   /* Mask for setting the Option Bit in the Request Flag. */
#define PTX_T5T_REQ_FLAG_AMS_SET_MASK                   (uint8_t)0x20   /* Mask for setting Addressed Mode. */
#define PTX_T5T_REQ_FLAG_HIGHT_DATA_RATE_SET_MASK       (uint8_t)0x02   /* High data rate mask. */
#define PTX_T5T_REQ_FLAG_SELECT_FLAG_SET_MASK           (uint8_t)0x10   /* Selection Flag set mask. */
#define PTX_T5T_CRC_ERROR_INTERNAL                      (uint8_t)0x80   /* Internal error code for retries upon miscommunication. */
#define PTX_T5T_REQ_FLAG_TEMPLATE_MASK                  PTX_T5T_REQ_FLAG_HIGHT_DATA_RATE_SET_MASK /* Template mask. */
/*  */

/*
 * Selection Parameters
 *
 */
#define PTX_T5T_SELECTED_TRUE                            (uint8_t)0x01   /* Internal Select flag true. */
#define PTX_T5T_SELECTED_FALSE                           (uint8_t)0x00   /* Internal Select flag false. */
/*  */

/*
 * Isolated EoF Parameters
 *
 */
#define PTX_T5T_ISOLATED_EOF_LEN                        (uint8_t)2u      /* Isolated EoF length. */
#define PTX_T5T_FDT_V_EOF                               (uint32_t)20     /* FDT waiting time between isolated EoFs. */
/*  */
/*
 * Internal Masks.
 *
 */
#define PTX_T5T_1_BYTE_SHIFT_MASK                       (uint8_t)8u      /* Shift Mask 8 bit. */
#define PTX_T5T_UINT16_LOWER_BYTE_MASK                  (uint16_t)0x00FF /* Mask lower 8 bits of an uint16. */
#define PTX_T5T_UINT16_UPPER_BYTE_MASK                  (uint16_t)0xFF00 /* Mask upper 8 bits of an uint16. */
/*  */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / HELPERS
 * ####################################################################################################################
 */
/*
 *  Set Command Frame Header.
 *
 *  t5t               Pointer to component.
 *  commandCode       Command Code to be set.
 *  optionFlag        Option Flags to be set.
 *  bytesWritten     Command Length.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NativeTag_T5T_SetCommandHeader(ptx_NativeTag_T5T_t *t5t, uint8_t commandCode, uint8_t optionFlag, uint32_t *bytesWritten);

/*
 *  Send Isolated EoF.
 *
 *  t5t               Pointer to component.
 *  rx            Rx Data Storage.
 *  rxLen         Maximum Rx Length on input, Rx Data Length on output.
 *  msTimeout         Timeout in ms.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NativeTag_T5T_IsolatedEoF(ptx_NativeTag_T5T_t *t5t, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout);

/*
 *  Data Exchange with a T5T.
 *
 *  t5t               Pointer to component.
 *  frameType         T5T Frame type.
 *  txLen             Length of Data to be sent in Internal TxBuffer.
 *  rx                Rx Data Storage.
 *  rxLen             Maximum Rx Length on input, Rx Data Length on output.
 *  msTimeout         Timeout in ms.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NativeTag_T5T_Transceive(ptx_NativeTag_T5T_t *t5t, ptx_NativeTag_T5TFrameType frameType, uint32_t txLen, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NativeTag_T5T_Open (ptx_NativeTag_T5T_t *t5t, ptx_NativeTag_T5T_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != t5t) && (NULL != initParams))
    {
        if ((NULL != initParams->rul) && (NULL != initParams->TxBuffer) && (PTX_T5T_MIN_TX_BUFFER_SIZE <= initParams->TxBufferSize))
        {
            /* clear component */
            (void)memset(t5t, 0, sizeof(ptx_NativeTag_T5T_t));

            /* set members */
            t5t->rul = initParams->rul;
            t5t->TxBuffer = initParams->TxBuffer;

            if (0 != initParams->UIDLen)
            {
                if ((PTX_T5T_UID_SIZE == initParams->UIDLen) && (NULL != initParams->UID))
                {
                    t5t->UID = initParams->UID;
                    t5t->UIDLen = initParams->UIDLen;
                    t5t->isSelected = initParams->isSelected;
                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                }
            }
            else
            {
                t5t->UID = NULL;
                t5t->UIDLen = 0;
                status = PTX_STATUS_SUCCESS;
            }

            /* set Component-ID at the end to prevent further calls in case of an error */
            if (PTX_STATUS_SUCCESS == status)
            {
                t5t->CompId = Comp_NativeTag_T5T;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T5T_ReadSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                uint8_t optionFlag,
                                                uint8_t blockNr,
                                                uint8_t *rx,
                                                uint32_t *rxLen,
                                                uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T) && (NULL != rx) && (NULL != rxLen))
    {
        /* set T5T command-header */
        status = ptx_NativeTag_T5T_SetCommandHeader(t5t, PTX_T5T_READ_SINGLE_BLOCK_REQ_CODE, optionFlag, &tx_index);

        /* set command specific parameters */
        if (PTX_STATUS_SUCCESS == status)
        {
            t5t->TxBuffer[tx_index] = blockNr;
            tx_index++;
        }

        /* everything OK so far ? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T5T_Transceive(t5t, T5T_FrameType_Standard, tx_index, rx, rxLen, msTimeout);
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_T5T_WriteSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                 uint8_t optionFlag,
                                                 uint8_t blockNr,
                                                 uint8_t *blockData,
                                                 uint8_t blockDataLen,
                                                 uint8_t *rx,
                                                 uint32_t *rxLen,
                                                 uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;
    ptx_NativeTag_T5TFrameType frame_type = (0 != optionFlag) ? T5T_FrameType_Special : T5T_FrameType_Standard;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T) && (NULL != rx) && (NULL != rxLen) && (NULL != blockData) && (PTX_T5T_MIN_TX_BUFFER_SIZE >= blockDataLen))
    {
        /* set T5T command-header */
        status = ptx_NativeTag_T5T_SetCommandHeader(t5t, PTX_T5T_WRITE_SINGLE_BLOCK_REQ_CODE, optionFlag, &tx_index);

        /* set command specific parameters */
        if (PTX_STATUS_SUCCESS == status)
        {
            t5t->TxBuffer[tx_index] = blockNr;
            tx_index++;

            (void)memcpy(&t5t->TxBuffer[tx_index], blockData, blockDataLen);
            tx_index = (uint32_t)(tx_index + blockDataLen);
        }

        /* everything OK so far ? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T5T_Transceive(t5t, frame_type, tx_index, rx, rxLen, msTimeout);
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_T5T_LockSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                uint8_t optionFlag,
                                                uint8_t blockNr,
                                                uint8_t *rx,
                                                uint32_t *rxLen,
                                                uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;
    ptx_NativeTag_T5TFrameType frame_type = (0 != optionFlag) ? T5T_FrameType_Special : T5T_FrameType_Standard;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T) && (NULL != rx) && (NULL != rxLen))
    {
        /* set T5T command-header */
        status = ptx_NativeTag_T5T_SetCommandHeader(t5t, PTX_T5T_LOCK_SINGLE_BLOCK_REQ_CODE, optionFlag, &tx_index);

        /* set command specific parameters */
        if (PTX_STATUS_SUCCESS == status)
        {
            t5t->TxBuffer[tx_index] = blockNr;
            tx_index++;
        }

        /* everything OK so far ? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T5T_Transceive(t5t, frame_type, tx_index, rx, rxLen, msTimeout);
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_T5T_ReadMultipleBlock (ptx_NativeTag_T5T_t *t5t,
                                                  uint8_t optionFlag,
                                                  uint8_t blockNr,
                                                  uint8_t nrBlocks,
                                                  uint8_t *rx,
                                                  uint32_t *rxLen,
                                                  uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T) && (NULL != rx) && (NULL != rxLen))
    {
        /* set T5T command-header */
        status = ptx_NativeTag_T5T_SetCommandHeader(t5t, PTX_T5T_READ_MULTIPLE_BLOCK_REQ_CODE, optionFlag, &tx_index);

        /* set command specific parameters */
        if (PTX_STATUS_SUCCESS == status)
        {
            t5t->TxBuffer[tx_index] = blockNr;
            tx_index++;

            t5t->TxBuffer[tx_index] = nrBlocks;
            tx_index++;
        }

        /* everything OK so far ? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T5T_Transceive(t5t, T5T_FrameType_Standard, tx_index, rx, rxLen, msTimeout);
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_T5T_ExtReadSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                   uint8_t optionFlag,
                                                   uint16_t blockNr,
                                                   uint8_t *rx,
                                                   uint32_t *rxLen,
                                                   uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T) && (NULL != rx) && (NULL != rxLen))
    {
        /* set T5T command-header */
        status = ptx_NativeTag_T5T_SetCommandHeader(t5t, PTX_T5T_EXT_READ_SINGLE_BLOCK_REQ_CODE, optionFlag, &tx_index);

        /* set command specific parameters */
        if (PTX_STATUS_SUCCESS == status)
        {
            t5t->TxBuffer[tx_index] = (uint8_t)((blockNr & PTX_T5T_UINT16_LOWER_BYTE_MASK) >> 0);
            tx_index++;

            t5t->TxBuffer[tx_index] = (uint8_t)((blockNr & PTX_T5T_UINT16_UPPER_BYTE_MASK) >> PTX_T5T_1_BYTE_SHIFT_MASK);
            tx_index++;
        }

        /* everything OK so far ? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T5T_Transceive(t5t, T5T_FrameType_Standard, tx_index, rx, rxLen, msTimeout);
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_T5T_ExtWriteSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                    uint8_t optionFlag,
                                                    uint16_t blockNr,
                                                    uint8_t *blockData,
                                                    uint8_t blockDataLen,
                                                    uint8_t *rx,
                                                    uint32_t *rxLen,
                                                    uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;
    ptx_NativeTag_T5TFrameType frame_type = (0 != optionFlag) ? T5T_FrameType_Special : T5T_FrameType_Standard;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T) && (NULL != rx) && (NULL != rxLen) && (NULL != blockData) && (PTX_T5T_MIN_TX_BUFFER_SIZE >= blockDataLen))
    {
        /* set T5T command-header */
        status = ptx_NativeTag_T5T_SetCommandHeader(t5t, PTX_T5T_EXT_WRITE_SINGLE_BLOCK_REQ_CODE, optionFlag, &tx_index);

        /* set command specific parameters */
        if (PTX_STATUS_SUCCESS == status)
        {
            t5t->TxBuffer[tx_index] = (uint8_t)((blockNr & PTX_T5T_UINT16_LOWER_BYTE_MASK) >> 0);
            tx_index++;

            t5t->TxBuffer[tx_index] = (uint8_t)((blockNr & PTX_T5T_UINT16_UPPER_BYTE_MASK) >> PTX_T5T_1_BYTE_SHIFT_MASK);
            tx_index++;

            (void)memcpy(&t5t->TxBuffer[tx_index], blockData, blockDataLen);
            tx_index = (uint32_t)(tx_index + blockDataLen);
        }

        /* everything OK so far ? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T5T_Transceive(t5t, frame_type, tx_index, rx, rxLen, msTimeout);
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_T5T_ExtLockSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                   uint8_t optionFlag,
                                                   uint16_t blockNr,
                                                   uint8_t *rx,
                                                   uint32_t *rxLen,
                                                   uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;
    ptx_NativeTag_T5TFrameType frame_type = (0 != optionFlag) ? T5T_FrameType_Special : T5T_FrameType_Standard;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T) && (NULL != rx) && (NULL != rxLen))
    {
        /* set T5T command-header */
        status = ptx_NativeTag_T5T_SetCommandHeader(t5t, PTX_T5T_EXT_LOCK_SINGLE_BLOCK_REQ_CODE, optionFlag, &tx_index);

        /* set command specific parameters */
        if (PTX_STATUS_SUCCESS == status)
        {
            t5t->TxBuffer[tx_index] = (uint8_t)((blockNr & PTX_T5T_UINT16_LOWER_BYTE_MASK) >> 0);
            tx_index++;

            t5t->TxBuffer[tx_index] = (uint8_t)((blockNr & PTX_T5T_UINT16_UPPER_BYTE_MASK) >> PTX_T5T_1_BYTE_SHIFT_MASK);
            tx_index++;
        }

        /* everything OK so far ? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T5T_Transceive(t5t, frame_type, tx_index, rx, rxLen, msTimeout);
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_T5T_ExtReadMultipleBlock (ptx_NativeTag_T5T_t *t5t,
                                                     uint8_t optionFlag,
                                                     uint16_t blockNr,
                                                     uint16_t nrBlocks,
                                                     uint8_t *rx,
                                                     uint32_t *rxLen,
                                                     uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T) && (NULL != rx) && (NULL != rxLen))
    {
        /* set T5T command-header */
        status = ptx_NativeTag_T5T_SetCommandHeader(t5t, PTX_T5T_EXT_READ_MULTIPLE_BLOCK_REQ_CODE, optionFlag, &tx_index);

        /* set command specific parameters */
        if (PTX_STATUS_SUCCESS == status)
        {
            t5t->TxBuffer[tx_index] = (uint8_t)((blockNr & PTX_T5T_UINT16_LOWER_BYTE_MASK) >> 0);
            tx_index++;

            t5t->TxBuffer[tx_index] = (uint8_t)((blockNr & PTX_T5T_UINT16_UPPER_BYTE_MASK) >> PTX_T5T_1_BYTE_SHIFT_MASK);
            tx_index++;

            t5t->TxBuffer[tx_index] = (uint8_t)((nrBlocks & PTX_T5T_UINT16_LOWER_BYTE_MASK) >> 0);
            tx_index++;

            t5t->TxBuffer[tx_index] = (uint8_t)((nrBlocks & PTX_T5T_UINT16_UPPER_BYTE_MASK) >> PTX_T5T_1_BYTE_SHIFT_MASK);
            tx_index++;
        }

        /* everything OK so far ? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T5T_Transceive(t5t, T5T_FrameType_Standard, tx_index, rx, rxLen, msTimeout);
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_T5T_Select (ptx_NativeTag_T5T_t *t5t,
                                       uint8_t optionFlag,
                                       uint8_t *uid,
                                       uint8_t uidLen,
                                       uint8_t *rx,
                                       uint32_t *rxLen,
                                       uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T) && (NULL != rx) && (NULL != rxLen)  && (NULL != uid) && (PTX_T5T_UID_SIZE == uidLen))
    {
        /* set T5T command-header */
        status = ptx_NativeTag_T5T_SetCommandHeader(t5t, PTX_T5T_SELECT_REQ_CODE, optionFlag, &tx_index);

        /* set command specific parameters */
        if (PTX_STATUS_SUCCESS == status)
        {
            (void)memcpy(&t5t->TxBuffer[tx_index], uid, uidLen);
            tx_index = (uint32_t)(tx_index + uidLen);
        }

        /* everything OK so far ? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T5T_Transceive(t5t, T5T_FrameType_Standard, tx_index, rx, rxLen, msTimeout);
        }

        /* check RES_FLAG */
        if (0x00 == rx[0] && (PTX_STATUS_SUCCESS == status))
        {
            t5t->isSelected = PTX_T5T_SELECTED_TRUE;
        }
        else
        {
            t5t->isSelected = PTX_T5T_SELECTED_FALSE;
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_T5T_Sleep (ptx_NativeTag_T5T_t *t5t,
                                      uint8_t optionFlag,
                                      uint8_t *uid,
                                      uint8_t uidLen,
                                      uint8_t *rx,
                                      uint32_t *rxLen,
                                      uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t tx_index = 0;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T) && (NULL != rx) && (NULL != rxLen) && (NULL != uid) && (PTX_T5T_UID_SIZE == uidLen))
    {
        /* set T5T command-header */
        status = ptx_NativeTag_T5T_SetCommandHeader(t5t, PTX_T5T_SLPV_REQ_CODE, optionFlag, &tx_index);

        /* set command specific parameters */
        if (PTX_STATUS_SUCCESS == status)
        {
            (void)memcpy(&t5t->TxBuffer[tx_index], uid, uidLen);
            tx_index = (uint32_t)(tx_index + uidLen);
        }

        /* everything OK so far ? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T5T_Transceive(t5t, T5T_FrameType_Standard, tx_index, rx, rxLen, msTimeout);

            /* Attention: A successful SLEEP-command doesn't send a response - therefore overwrite the status for the caller */
            if (((PTX_STATUS_E_GEN_TIMEOUT == status) || (PTX_STATUS_E_NSC_RFERROR == status)) && (0 == *rxLen))
            {
                status = PTX_STATUS_SUCCESS;
            }
        }

        if ((PTX_STATUS_SUCCESS == status) && (NULL != t5t->UID) && (0 == memcmp(&t5t->UID[0], &uid[0], PTX_T5T_UID_SIZE)))
        {
            t5t->isSelected = PTX_T5T_SELECTED_FALSE;
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_T5T_SetUID (ptx_NativeTag_T5T_t *t5t,
                                       uint8_t *uid,
                                       uint8_t uidLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T))
    {
        if (0 != uidLen)
        {
            /* check if new UID is being set if in SELECTED mode, if yes exit SELECTED mode */
            if ((NULL != t5t->UID) && (PTX_T5T_SELECTED_TRUE == t5t->isSelected))
            {
                memcmp(t5t->UID, uid, PTX_T5T_UID_SIZE) ? (t5t->isSelected = PTX_T5T_SELECTED_FALSE) : (t5t->isSelected = PTX_T5T_SELECTED_TRUE);
            }

            if ((PTX_T5T_UID_SIZE == uidLen) && (NULL != uid))
            {
                t5t->UID = uid;
                t5t->UIDLen = uidLen;

                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
        else
        {
            t5t->UID = NULL;
            t5t->UIDLen = 0;
            t5t->isSelected = PTX_T5T_SELECTED_FALSE;

            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;

}

ptx_Status_t ptx_NativeTag_T5T_Close (ptx_NativeTag_T5T_t *t5t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T))
    {
        memset(t5t, 0, sizeof(ptx_NativeTag_T5T_t));
        t5t->CompId = Comp_None;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK(s)
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NativeTag_T5T_SetCommandHeader(ptx_NativeTag_T5T_t *t5t, uint8_t commandCode, uint8_t optionFlag, uint32_t *bytesWritten)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t req_flag = PTX_T5T_REQ_FLAG_TEMPLATE_MASK;
    uint32_t index = 0;

    if (PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T) && (NULL != bytesWritten))
    {
        /* set Option-Flag ? */
        if (0 != optionFlag)
        {
            req_flag = (uint8_t)(req_flag | PTX_T5T_REQ_FLAG_OPTION_BIT_SET_MASK);
        }

        /* use Addressed-Mode ? */
        if ((NULL != t5t->UID) || ((PTX_T5T_SELECT_REQ_CODE == commandCode) || (PTX_T5T_SLPV_REQ_CODE == commandCode)))
        {
            req_flag = (uint8_t)(req_flag | PTX_T5T_REQ_FLAG_AMS_SET_MASK);
        }

        /* SELECTED mode? */
        if ((PTX_T5T_SELECTED_FALSE != t5t->isSelected) && (PTX_T5T_SELECT_REQ_CODE != commandCode) && (PTX_T5T_SLPV_REQ_CODE != commandCode))
        {
            req_flag = (uint8_t)(req_flag | PTX_T5T_REQ_FLAG_SELECT_FLAG_SET_MASK);
            req_flag = (uint8_t)(req_flag & ~PTX_T5T_REQ_FLAG_AMS_SET_MASK);
        }

        /* set REQ_FLAG-byte */
        t5t->TxBuffer[index] = req_flag;
        index++;

        /* set command-code */
        t5t->TxBuffer[index] = commandCode;
        index++;

        switch (commandCode)
        {
            case PTX_T5T_SELECT_REQ_CODE:
            /* Fall through. */
            case PTX_T5T_SLPV_REQ_CODE:
                /* don't insert UID here - done by caller because the UID is treated as payload in this case */
                break;

            default:
                /* set UID (optional) */
                if ((0 != (req_flag & PTX_T5T_REQ_FLAG_AMS_SET_MASK)) && (NULL != t5t->UID))
                {
                    (void)memcpy(&t5t->TxBuffer[index], &t5t->UID[0], t5t->UIDLen);
                    index = (uint32_t)(index + t5t->UIDLen);
                }
                break;
        }
        *bytesWritten = index;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_NativeTag_T5T_IsolatedEoF(ptx_NativeTag_T5T_t *t5t, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T)) && (NULL != rxLen) && (NULL != rx))
    {
        status = ptx_RUL_T5T_Isolated_EoF(t5t->rul, rx, rxLen, msTimeout);
    }

    return status;
}

static ptx_Status_t ptx_NativeTag_T5T_Transceive(ptx_NativeTag_T5T_t *t5t, ptx_NativeTag_T5TFrameType frameType, uint32_t txLen, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t timeout_value;
    uint32_t rx_len;
    uint8_t retry_cnt = 0;

    /* Aside from actual Tag-component, remaining parameters are checked within Data-Exchange function */
    if ((PTX_COMP_CHECK(t5t, Comp_NativeTag_T5T)) && (NULL != rxLen))
    {
        timeout_value = (T5T_FrameType_Special == frameType) ? 1 : msTimeout;

        while (PTX_T5T_MAX_NR_RETRIES >= retry_cnt)
        {
            /* send command */
            rx_len = *rxLen;
            status = ptx_RUL_Exchange (t5t->rul, &t5t->TxBuffer[0], txLen, rx, &rx_len, timeout_value);

            if ((T5T_FrameType_Special != frameType) &&
                ((PTX_STATUS_E_GEN_TIMEOUT == status) || (PTX_STATUS_E_NSC_RFERROR == status)) &&
                (PTX_T5T_SLPV_REQ_CODE != t5t->TxBuffer[PTX_T5T_COMMAND_CODE_FRAME_OFFSET]) &&
                (0 == t5t->isSelected))
            {
                /* Retry possibly needed. */
                retry_cnt++;
                status = PTX_STATUS_E_NSC_RFERROR;
            }
            else
            {
                /* No retry needed. */
                retry_cnt = PTX_T5T_MAX_NR_RETRIES + 1u;
            }

            /* Special-Frame format is supposed to return a timeout-error after the first RF-exchange -> reset the status */
            if ((T5T_FrameType_Special == frameType) && (PTX_STATUS_E_GEN_TIMEOUT == status))
            {
                status = PTX_STATUS_SUCCESS;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* special-frame format i.e. isolated EoF required ? */
                if (T5T_FrameType_Special == frameType)
                {
                    (void)ptx_RUL_Sleep(t5t->rul, PTX_T5T_FDT_V_EOF);

                    rx_len = *rxLen;
                    status = ptx_NativeTag_T5T_IsolatedEoF (t5t, rx, &rx_len, msTimeout);
                }
            }
        }

        if (PTX_STATUS_SUCCESS != status)
        {
            *rxLen = 0;
        }
        else
        {
            *rxLen = rx_len;
        }
    }

    return status;
}


