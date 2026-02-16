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
    Module      : NFC Data Exchange Format (NDEF) T5T
    File        : ptx_NDEF_T5T.c

    Description : NFC Data Exchange Format (NDEF) implementation to control NDEF Messages on T5T Devices.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL.h"
#include "ptx_NDEF_T5T.h"
#include "ptx_NativeTag_T5T.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
/*
 * CC_Parameter Positions and Offsets
 *
 */
#define PTX_T5T_CC_BLOCK_NUMBER                           (uint16_t)0x0000  /* CC block number. */
#define PTX_T5T_CC_OFFSET_MAGIC_NUMBER                    (uint8_t)0x00     /* CC block offset for Magic Number. */
#define PTX_T5T_CC_OFFSET_VERSION_ACCESS                  (uint8_t)0x01     /* CC block offset for version and access conditions. */
#define PTX_T5T_CC_OFFSET_MLEN_BASIC                      (uint8_t)0x02     /* CC block offset for MLEN. */
#define PTX_T5T_CC_OFFSET_MLEN_EXTENDED                   (uint8_t)0x06     /* CC block offset for extended MLEN. */
#define PTX_T5T_CC_OFFSET_ADDITIONAL_FEATURE              (uint8_t)0x03     /* CC block offset for additional feature information. */
/*  */

/*
 * CC-Parameters and Masks
 *
 */
#define PTX_T5T_CC_ADD_INFO_SPECIAL_FRAME_MASK            (uint8_t)0x10     /* Mask for additional information byte containing Special Frame features. */
#define PTX_T5T_CC_ADD_INFO_LOCK_BLOCK_MASK               (uint8_t)0x08     /* Mask for additional information byte containing block locking features. */
#define PTX_T5T_CC_ADD_INFO_MBREAD_MASK                   (uint8_t)0x01     /* Mask for additional information byte containing multiple block read features. */
#define PTX_T5T_CC_MAGIC_NUMBER_E1                        (uint8_t)0xE1     /* Magic Number E1h. */
#define PTX_T5T_CC_MAGIC_NUMBER_E2                        (uint8_t)0xE2     /* Magic number E2h. */
#define PTX_T5T_CC_VERSION_MASK                           (uint8_t)0xF0     /* Mask for version information. */
#define PTX_T5T_CC_READ_ACCESS_MASK                       (uint8_t)0x0C     /* Mask for read access condition. */
#define PTX_T5T_CC_WRITE_ACCESS_MASK                      (uint8_t)0x03     /* Mask for write access condition. */
/*  */

/*
 * TLV Parameters
 *
 */
#define PTX_T5T_NDEF_MESSAGE_TLV_T                        (uint8_t)0x03     /* NDEF TLV T-value. */
#define PTX_T5T_TERMINATOR_TLV_T                          (uint8_t)0xFE     /* Terminator TLV T-value. */
#define PTX_T5T_TLV_HEADER_SHORT                          (uint8_t)2u       /* NDEF TLV header length short. */
#define PTX_T5T_TLV_HEADER_LONG                           (uint8_t)4u       /* NDEF TLV header length long. */
#define PTX_T5T_TLV_LENGTHTHRESHOLD                       (uint8_t)0xFF     /* Length threshold for longer NDEF TLV header. */
/*  */

/*
 * Read Operation Flags
 *
 */
#define PTX_T5T_2ND_READ_OP_NOT_REQUIRED                  (uint8_t)0x00     /* Internal flag for further reads in NDEF length detection, no extra read. */
#define PTX_T5T_2ND_READ_OP_GET_FULL_LENGTH               (uint8_t)0x01     /* Internal flag for further reads in NDEF length detection, extra read to get 1 byte length. */
#define PTX_T5T_2ND_READ_OP_3BL_GET_PART_LENGTH           (uint8_t)0x02     /* Internal flag for further reads in NDEF length detection, extra read to get part of 3 byte length. */
#define PTX_T5T_2ND_READ_OP_3BL_GET_FULL_LENGTH           (uint8_t)0x03     /* Internal flag for further reads in NDEF length detection, extra read to get full 3 byte length. */
/*  */


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / HELPERS
 * ####################################################################################################################
 */
/*
 *  Get CC Information.
 *
 *  t5t           Pointer to component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T5TGetCCInfo (ptx_NDEF_T5T_t *ndefT5T);

/*
 *  Handle CC Information.
 *
 *   t5t           Pointer to component.
 *  ndefTLVFound  Flag whether an NDEF TLV has been found.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T5T_HandleCCInfo(ptx_NDEF_T5T_t *ndefT5T, uint8_t *ndefTLVFound);

/*
 *  Read a single Block of Data.
 *
 *  t5t               Pointer to component.
 *  blockNumber       Block Number to read from.
 *  rx            Rx Data Storage.
 *  rxLen         Maximum Rx Length on input, Rx Data Length on output.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T5T_ReadBlock (ptx_NDEF_T5T_t *ndefT5T, uint16_t blockNumber, uint8_t *rx, uint32_t *rxLen);

/*
 *  Write a single block of Data.
 *
 *  t5t               Pointer to component.
 *  blockNumber       Block Number to read from.
 *  blockData         Data to be written.
 *  blockDataLen      Length of Data to be written.
 *  rx            Rx Data Storage.
 *  rxLen         Maximum Rx Length on input, Rx Data Length on output.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T5T_WriteBlock (ptx_NDEF_T5T_t *ndefT5T, uint16_t blockNumber, uint8_t *blockData, uint8_t blockDataLen, uint8_t *rx, uint32_t *rxLen);
/*
 *  Lock a single Block.
 *
 *  t5t               Pointer to component.
 *  blockNumber       Block Number to lock.
 *  rx            Rx Data Storage.
 *  rxLen         Maximum Rx Length on input, Rx Data Length on output.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T5T_LockBlock (ptx_NDEF_T5T_t *ndefT5T, uint16_t blockNumber, uint8_t *rx, uint32_t *rxLen);

/*
 *  Update L-Field of NDEF TLV.
 *
 *  t5t               Pointer to component.
 *  messageLen        Message Length.
 *  lengthSize        Size of L-Field.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T5T_UpdateLength (ptx_NDEF_T5T_t *ndefT5T, uint16_t messageLen, uint8_t lengthSize);

/*
 *  Read NDEF Message via Single Bock Read Operations.
 *
 *  ndefT5T         Pointer to component.
 *  msgBuffer     Buffer to store Message.
 *  msgLen        Max Length of Message on input, Length of Message on output.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T5TReadNDEFSingleBlocks(ptx_NDEF_T5T_t *ndefT5T, uint8_t *msgBuffer, uint32_t *msgLen);

/*
 *  Read NDEF Message via Multiple Bock Read Operations.
 *
 *  ndefT5T         Pointer to component.
 *  msgBuffer     Buffer to store Message.
 *  msgLen        Max Length of Message on input, Length of Message on output.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T5TReadNDEFMultipleBlocks(ptx_NDEF_T5T_t *ndefT5T, uint8_t *msgBuffer, uint32_t *msgLen);

/*
 *  Write NDEF Message Loop.
 *
 *  ndefT5T         Pointer to component.
 *  msgLen            Length of Message.
 *  msgBuffer         Message Buffer.
 *  offset            Offset within first block.
 *  currentBlock      Position on tag to start at.
 *  tlvHeaderLen      Size of NDEF TLV Header.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T5T_WriteMessageLoop(ptx_NDEF_T5T_t *ndefT5T, uint32_t msgLen, uint8_t *msgBuffer, uint8_t offset, uint16_t currentBlock, uint8_t tlvHeaderLen);

/*
 *  Lock Blocks Loop.
 *
 *  ndefT5T             Pointer to component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T5T_LockLoop(ptx_NDEF_T5T_t *ndefT5T);

/*
 *  Look for TLVs in current data.
 *
 *  ndefT5T             Pointer to component.
 * [in, out] currentByteNr    Current Block Position on Tag.
 *  data                  Data read from Tag, one Block.
 *  dataLen               Length of Data read from Tag.
 *
 *  nefTlvFound          Flag whether NDEF TLV has been found.
 *  terminatorFound      Flag whether Terminator TLV has been found.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T5T_ParseTLV(ptx_NDEF_T5T_t *ndefT5T, uint16_t *currentByteNr, uint8_t *data, uint32_t dataLen, uint8_t *ndefTlvFound, uint8_t *terminatorFound);

/*
 *  MTU finder (in units of Blocks)
 *
 *  ndefT5T             Pointer to component.
 *  nbrBlocks             Number of blocks you want to handle
 *
 * returns Maximum number of blocks to be handled.
 */
static uint16_t ptx_NDEF_T5T_SetMTU (ptx_NDEF_T5T_t *ndefT5T, uint16_t nbrBlocks);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NDEF_T5T_Open (ptx_NDEF_T5T_t *ndefT5T, ptx_NDEF_T5T_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ndefT5T) && (NULL != initParams))
    {
        if ((NULL != initParams->RxBuffer) &&
            (0 != initParams->RxBufferSize) &&
            (NULL != initParams->WorkBuffer) &&
            (0 != initParams->WorkBufferSize))
        {
            /* clear component */
            (void)memset(ndefT5T, 0, sizeof(ptx_NDEF_T5T_t));

            ndefT5T->RxBuffer = initParams->RxBuffer;
            ndefT5T->RxBufferSize = initParams->RxBufferSize;
            ndefT5T->WorkBuffer = initParams->WorkBuffer;
            ndefT5T->WorkBufferSize = initParams->WorkBufferSize;
            ndefT5T->LifeCycle = TagLC_NoNDEFTag;

            /* initialize lower layer component */
            ptx_NativeTag_T5T_InitParams_t T5T_init_params;
            (void)memset(&T5T_init_params, 0, sizeof(ptx_NativeTag_T5T_InitParams_t));
            T5T_init_params.rul = initParams->rul;
            T5T_init_params.TxBuffer = initParams->TxBuffer;
            T5T_init_params.TxBufferSize = initParams->TxBufferSize;
            T5T_init_params.isSelected = initParams->isSelected;

            if (initParams->isAddressed)
            {
                T5T_init_params.UID = &initParams->rul->CardRegistry.ActiveCard->TechParam.TypeV.InventoryRes[PTX_T5T_INVENTORY_RES_OFFSET_UID];
                T5T_init_params.UIDLen = PTX_T5T_UID_LEN;
            }
            else
            {
                T5T_init_params.UIDLen = 0u;
                T5T_init_params.UID = NULL;
            }

            status = ptx_NativeTag_T5T_Open(&ndefT5T->t5t, &T5T_init_params);

            /* set Component-ID at the end to prevent further calls in case of an error */
            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT5T->CompId = Comp_T5TOP;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T5T_FormatTag (ptx_NDEF_T5T_t *ndefT5T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP))
    {
        status = PTX_STATUS_E_NDEF_UNSUPPORTED;

    }

    return status;
}

ptx_Status_t ptx_NDEF_T5T_CheckMessage (ptx_NDEF_T5T_t *ndefT5T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t ndef_tlv_found = 0;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP))
    {
        /* set UID */
        status = ptx_NativeTag_T5T_SetUID(&ndefT5T->t5t,
                                          &ndefT5T->t5t.UID[0],
                                          (uint8_t)ndefT5T->t5t.UIDLen);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* read CC-parameters */
            status = ptx_NDEF_T5TGetCCInfo (ndefT5T);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NDEF_T5T_HandleCCInfo(ndefT5T, &ndef_tlv_found);
        }

        /* update Life-Cycle of Tag */
        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 != ndef_tlv_found)
            {
                if (0 != ndefT5T->CCParams.WriteAccess)
                {
                    ndefT5T->LifeCycle = (0 != ndefT5T->NDEF_TLV_LENGTH) ? TagLC_ReadWrite : TagLC_Initialised;
                }
                else
                {
                    ndefT5T->LifeCycle = TagLC_ReadOnly;
                }
            }
            else
            {
                status = PTX_STATUS_E_NDEF_NO_MESSAGES;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T5T_ReadMessage (ptx_NDEF_T5T_t *ndefT5T, uint8_t *msgBuffer, uint32_t *msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t msg_buffer_size;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP) && (NULL != msgBuffer) && (NULL != msgLen))
    {
        msg_buffer_size = *msgLen;

        if (0 != msg_buffer_size)
        {
            switch (ndefT5T->LifeCycle)
            {
                case TagLC_Initialised:
                case TagLC_ReadWrite:
                case TagLC_ReadOnly:
                    /* OK */
                    status = PTX_STATUS_SUCCESS;
                    break;

                default:
                    status = PTX_STATUS_E_NDEF_INVALID_TAG_STATE;
                    break;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* set UID */
                status = ptx_NativeTag_T5T_SetUID(&ndefT5T->t5t,
                                                  &ndefT5T->t5t.UID[0],
                                                  (uint8_t)ndefT5T->t5t.UIDLen);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                uint8_t ndefTlvFound = 0;
                status = ptx_NDEF_T5T_HandleCCInfo(ndefT5T, &ndefTlvFound);

                if ((PTX_STATUS_SUCCESS == status) && (0 != ndefTlvFound))
                {
                    if (0 != ndefT5T->CCParams.MultiBlockReadSupported)
                    {
                        status = ptx_NDEF_T5TReadNDEFMultipleBlocks(ndefT5T, msgBuffer, &msg_buffer_size);
                    }
                    else
                    {
                        status = ptx_NDEF_T5TReadNDEFSingleBlocks(ndefT5T, msgBuffer, &msg_buffer_size);
                    }
                }
            }

            /* everything correctly processed ? */
            if ((PTX_STATUS_SUCCESS == status) && (msg_buffer_size == ndefT5T->NDEF_TLV_LENGTH))
            {
                *msgLen = (uint32_t)(ndefT5T->NDEF_TLV_LENGTH);
            }
            else
            {
                *msgLen = 0;
            }
        }
        else
        {
            status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T5T_WriteMessage (ptx_NDEF_T5T_t *ndefT5T, uint8_t *msgBuffer, uint32_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t length_size;
    uint16_t current_block_nr = 0;
    uint32_t rx_len;
    uint16_t mlen_loss = 0;
    uint8_t ndef_tlv_header_size;
    uint8_t init_block_write_offset = 0;
    uint16_t msg_len;
    uint8_t *msg_buffer;
    uint8_t EMPTY_NDEF_MESSAGE[] = {0xD0, 0x00, 0x00};

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP))
    {
        switch (ndefT5T->LifeCycle)
        {
            case TagLC_Initialised:
            case TagLC_ReadWrite:
                /* OK */
                status = PTX_STATUS_SUCCESS;
                break;

            default:
                status = PTX_STATUS_E_NDEF_INVALID_TAG_STATE;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* set UID */
            status = ptx_NativeTag_T5T_SetUID(&ndefT5T->t5t,
                                              &ndefT5T->t5t.UID[0],
                                              (uint8_t)ndefT5T->t5t.UIDLen);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* determine NDEF-message to write */
            if ((NULL != msgBuffer) && (0 != msgLen))
            {
                msg_buffer = msgBuffer;
                msg_len = (uint16_t)msgLen;

            }
            else
            {
                msg_buffer = &EMPTY_NDEF_MESSAGE[0];
                msg_len = (uint16_t)sizeof(EMPTY_NDEF_MESSAGE);
            }

            length_size = (msg_len <= ((uint32_t)0xFE)) ? (uint8_t)0x01 : (uint8_t)0x03;
            ndef_tlv_header_size = (uint8_t)(length_size + 1);

            mlen_loss = (uint16_t)(ndef_tlv_header_size + ndefT5T->NDEF_TLV_POS_BY);

            /* does NDEF-message fit at all onto the Tag ? */
            if (msg_len <= (uint32_t)(ndefT5T->CCParams.MLEN - mlen_loss - ((ndefT5T->BlockSize * ndefT5T->NDEF_TLV_POS_BN) - ndefT5T->CCParams.Size)))
            {
                /* reset L-field */
                status = ptx_NDEF_T5T_UpdateLength (ndefT5T, 0, length_size);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /*
                     * determine block-address to start writing (NDEF_TLV_POS_BN defines the block where the NDEF-TLV starts,
                     * but not the actual NDEF-message, which could be stored in the next block;
                     */
                    current_block_nr = ndefT5T->NDEF_TLV_POS_BN;
                    if ((uint8_t)((ndefT5T->NDEF_TLV_POS_BY + ndef_tlv_header_size)) < ndefT5T->BlockSize)
                    {
                        init_block_write_offset = (uint8_t)(ndefT5T->NDEF_TLV_POS_BY + ndef_tlv_header_size);

                    }
                    else
                    {
                        init_block_write_offset = (uint8_t)((ndefT5T->NDEF_TLV_POS_BY + ndef_tlv_header_size) % ndefT5T->BlockSize);
                        current_block_nr++;
                    }

                    /* if the offset != 0, we have to read the current block first */
                    if (0 != init_block_write_offset)
                    {
                        /* read block */
                        status = ptx_NDEF_T5T_ReadBlock(ndefT5T, current_block_nr, &ndefT5T->RxBuffer[0], &rx_len);

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            if ((rx_len > 1) && ((rx_len - 1) <= ndefT5T->WorkBufferSize))
                            {
                                (void)memcpy(&ndefT5T->WorkBuffer[0], &ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA], (uint32_t)(init_block_write_offset));
                            }
                            else
                            {
                                status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                            }
                        }
                    }
                }
            }
            else
            {
                status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T5T_WriteMessageLoop(ndefT5T, msg_len, msg_buffer, init_block_write_offset, current_block_nr, ndef_tlv_header_size);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* set intended size of L-field */
                status = ptx_NDEF_T5T_UpdateLength (ndefT5T, msg_len, length_size);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T5T_LockTag (ptx_NDEF_T5T_t *ndefT5T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t rx_len;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP))
    {
        switch (ndefT5T->LifeCycle)
        {
            case TagLC_ReadWrite:
                /* transition to READ-ONLY i.e. locked-state can only be done if the Tag supports the Lock-commands */
                if (0 == ndefT5T->CCParams.LockBlockSupported)
                {
                    status = PTX_STATUS_E_NDEF_UNSUPPORTED;
                }
                else
                {
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            default:
                status = PTX_STATUS_E_NDEF_INVALID_TAG_STATE;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* set UID */
            status = ptx_NativeTag_T5T_SetUID(&ndefT5T->t5t,
                                              &ndefT5T->t5t.UID[0],
                                              (uint8_t)ndefT5T->t5t.UIDLen);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* clear write-access by reading block 0 (CC) and update the "Version & Access-Condition"-field at offset 2  */
            status = ptx_NDEF_T5T_ReadBlock(ndefT5T, 0, &ndefT5T->RxBuffer[0], &rx_len);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* skip status-flag at offset 0 */
                rx_len--;

                if (rx_len <=  ndefT5T->WorkBufferSize)
                {
                    (void)memcpy(&ndefT5T->WorkBuffer[0], &ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA], (uint32_t)(rx_len));
                }
                else
                {
                    status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* clear write-access bits */
                    ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_VERSION_ACCESS] = (uint8_t)(ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_VERSION_ACCESS] | PTX_T5T_CC_WRITE_ACCESS_MASK);

                    /* update block 0 */
                    status = ptx_NDEF_T5T_WriteBlock(ndefT5T, 0, &ndefT5T->WorkBuffer[0], ndefT5T->BlockSize, &ndefT5T->RxBuffer[0], &rx_len);
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T5T_LockLoop(ndefT5T);
            }
        }

    }

    return status;
}

ptx_Status_t ptx_NDEF_T5T_Close (ptx_NDEF_T5T_t *ndefT5T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP))
    {
        status = ptx_NativeTag_T5T_Close(&ndefT5T->t5t);

    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK(s)
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NDEF_T5TGetCCInfo (ptx_NDEF_T5T_t *ndefT5T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t rx_len;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP))
    {
        /* reset current CC-parameters */
        (void)memset(&ndefT5T->CCParams, 0, sizeof(ptx_NDEF_T5T_CC_t));

        /* read CC-block */
        status = ptx_NDEF_T5T_ReadBlock(ndefT5T, PTX_T5T_CC_BLOCK_NUMBER, &ndefT5T->RxBuffer[0], &rx_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            ndefT5T->BlockSize = (uint8_t)(rx_len - 1);

            switch (ndefT5T->BlockSize)
            {
                case (uint8_t)4:
                case (uint8_t)8:
                case (uint8_t)16:
                case (uint8_t)32:
                    /* everything OK */
                    break;

                default:
                    ndefT5T->LifeCycle = TagLC_NoNDEFTag;
                    status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
                    break;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /*
                 * Note: The CC consists either of
                 *       - 4 bytes (MLEN-parameter at offset 3 != 0) or
                 *       - 8 bytes (MLEN-parameter at offset 3 == 0).
                 *
                 *       If the block-size is set to 4 but the CC consists of 8 byte, a 2nd read-operation is required.
                 */

                /* copy first BlockSize * bytes of CC as they're valid in any case */
                if (ndefT5T->BlockSize <= ndefT5T->WorkBufferSize)
                {
                    (void)memcpy(&ndefT5T->WorkBuffer[0], &ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA], (uint32_t)ndefT5T->BlockSize);
                }
                else
                {
                    status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    ndefT5T->CCParams.Size = (uint8_t)4u;

                    /* 2nd read-operation required ? */
                    if (0 == ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_MLEN_BASIC])
                    {
                        ndefT5T->CCParams.Size = (uint8_t)8u;

                        if ((uint8_t)4 == ndefT5T->BlockSize)
                        {
                            status = ptx_NDEF_T5T_ReadBlock(ndefT5T, (uint16_t)(PTX_T5T_CC_BLOCK_NUMBER + 1), &ndefT5T->RxBuffer[0], &rx_len);

                            if (PTX_STATUS_SUCCESS == status)
                            {
                                /* copy 2nd half of CC from already available data */
                                (void)memcpy(&ndefT5T->WorkBuffer[4], &ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA], (uint32_t)4);
                            }
                        }
                    }
                }
            }

            /* parse CC-data */
            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT5T->CCParams.MagicNumber = ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_MAGIC_NUMBER];

                /* NDEF-Tag at all ?*/
                switch (ndefT5T->CCParams.MagicNumber)
                {
                    case PTX_T5T_CC_MAGIC_NUMBER_E1:
                        /* OK - nothing to do */
                        break;

                    case PTX_T5T_CC_MAGIC_NUMBER_E2:
                        /* OK - EXT_*-commands are required for block-addresses above 0xFF */
                        ndefT5T->CCParams.ExtCommandTypeRequired = (uint8_t)0x01;
                        break;

                    default:
                        /* not ready for NDEF - update Life-Cycle and cancel operation */
                        ndefT5T->LifeCycle = TagLC_NoNDEFTag;
                        status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
                        break;
                }

                /* check supported mapping version and set Read-/Write-Access information */
                if (PTX_STATUS_SUCCESS == status)
                {
                    ndefT5T->CCParams.Version = (uint8_t)((ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_VERSION_ACCESS] & PTX_T5T_CC_VERSION_MASK) >> 2);

                    if (((ndefT5T->CCParams.Version & 0xF0) == (PTX_T5T_SUPPORTED_VERSION & 0xF0)) && ((ndefT5T->CCParams.Version & 0x0F) >= (PTX_T5T_SUPPORTED_VERSION & 0x0F)))
                    {
                        if (0 == (ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_VERSION_ACCESS] & PTX_T5T_CC_READ_ACCESS_MASK))
                        {
                            ndefT5T->CCParams.ReadAccess = (uint8_t)0x01;
                        }

                        if (0 == (ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_VERSION_ACCESS] & PTX_T5T_CC_WRITE_ACCESS_MASK))
                        {
                            ndefT5T->CCParams.WriteAccess = (uint8_t)0x01;
                        }

                    }
                    else
                    {
                        status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
                    }
                }

                /* set memory-length (MLEN) and feature flags */
                if (PTX_STATUS_SUCCESS == status)
                {
                    if (0 != ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_MLEN_BASIC])
                    {
                        ndefT5T->CCParams.MLEN = (uint16_t)(ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_MLEN_BASIC] * 8);

                    }
                    else
                    {
                        ndefT5T->CCParams.MLEN = (uint16_t)(((ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_MLEN_EXTENDED + 0] << 8) |
                                                             ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_MLEN_EXTENDED + 1]) * 8);
                    }

                    if (0 != (ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_ADDITIONAL_FEATURE] & PTX_T5T_CC_ADD_INFO_SPECIAL_FRAME_MASK))
                    {
                        ndefT5T->CCParams.SpecialFrameRequired = (uint8_t)0x01;
                    }

                    if (0 != (ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_ADDITIONAL_FEATURE] & PTX_T5T_CC_ADD_INFO_LOCK_BLOCK_MASK))
                    {
                        ndefT5T->CCParams.LockBlockSupported = (uint8_t)0x01;
                    }

                    if (0 != (ndefT5T->WorkBuffer[PTX_T5T_CC_OFFSET_ADDITIONAL_FEATURE] & PTX_T5T_CC_ADD_INFO_MBREAD_MASK))
                    {
                        ndefT5T->CCParams.MultiBlockReadSupported = (uint8_t)0x01;
                    }
                }
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T5T_HandleCCInfo(ptx_NDEF_T5T_t *ndefT5T, uint8_t *ndefTLVFound)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t current_block_nr = 0;
    uint16_t current_byte_nr = 0;
    uint8_t quit_loop = 0;
    uint8_t ndef_tlv_found = 0;
    uint8_t terminator_tlv_found = 0;
    uint8_t init_block_read_offset = 0;
    uint32_t rx_len;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP) && (NULL != ndefTLVFound))
    {
        /* reset Greedy-collection */
        ndefT5T->NDEF_TLV_POS_BN = 0;
        ndefT5T->NDEF_TLV_POS_BY = 0;
        ndefT5T->NDEF_TLV_LENGTH = 0;

        /* determine starting block number and overall read index */
        switch (ndefT5T->BlockSize)
        {
            case (uint8_t)4:
                current_block_nr = ((uint8_t)4 == ndefT5T->CCParams.Size) ? (uint16_t)1 : (uint16_t)2;
                init_block_read_offset = 0;
                break;

            case (uint8_t)8:
                current_block_nr = ((uint8_t)4 == ndefT5T->CCParams.Size) ? (uint16_t)0 : (uint16_t)1;
                init_block_read_offset = (0 == current_block_nr) ? (uint8_t)(ndefT5T->BlockSize - ndefT5T->CCParams.Size) : 0;
                break;

            default:
                current_block_nr = 0;
                init_block_read_offset = (uint8_t)(ndefT5T->CCParams.Size);
                break;
        }

        status = PTX_STATUS_SUCCESS;

        /* Current Byte Number starts off as first byte of first block. */
        current_byte_nr = (uint16_t)(current_block_nr * ndefT5T->BlockSize);

        /* search for the NDEF-TLV */
        while ((current_block_nr < ((ndefT5T->CCParams.MLEN + ndefT5T->CCParams.Size) / ndefT5T->BlockSize)) && (PTX_STATUS_SUCCESS == status) && (0 == quit_loop))
        {
            /* read block */
            status = ptx_NDEF_T5T_ReadBlock(ndefT5T, current_block_nr, &ndefT5T->RxBuffer[0], &rx_len);

            if ((PTX_STATUS_SUCCESS == status) && (rx_len >= (uint32_t)((1 + ndefT5T->BlockSize))))
            {
                /* skip status-flag at offset 0 */
                rx_len--;

                current_byte_nr = (uint16_t)(current_byte_nr + init_block_read_offset);

                status = ptx_NDEF_T5T_ParseTLV(ndefT5T, &current_byte_nr, &ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA], rx_len, &ndef_tlv_found, &terminator_tlv_found);
                quit_loop = (uint8_t)(quit_loop | terminator_tlv_found | ndef_tlv_found | status);

                /* reset initial block offset because after 1st read, every further block gets read starting from offset 0 */
                init_block_read_offset = 0;
                current_block_nr = (uint16_t)(current_byte_nr / ndefT5T->BlockSize);
            }
        }

        *ndefTLVFound = ndef_tlv_found;
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T5T_ReadBlock (ptx_NDEF_T5T_t *ndefT5T, uint16_t blockNumber, uint8_t *rx, uint32_t *rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Aside from actual Tag-component, remaining parameters are checked at lower-layers */
    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP) && (NULL != rxLen))
    {
        *rxLen = (uint32_t)ndefT5T->RxBufferSize;

        /* send standard- or Extended-Command depending on Block-Number */
        if (0 == ndefT5T->CCParams.ExtCommandTypeRequired)
        {
            status = ptx_NativeTag_T5T_ReadSingleBlock (&ndefT5T->t5t,
                                                        0,
                                                        (uint8_t)blockNumber,
                                                        rx,
                                                        rxLen,
                                                        PTX_T5T_DEFAULT_TIMEOUT_MS);

        }
        else
        {
            status = ptx_NativeTag_T5T_ExtReadSingleBlock (&ndefT5T->t5t,
                                                           0,
                                                           blockNumber,
                                                           rx,
                                                           rxLen,
                                                           PTX_T5T_DEFAULT_TIMEOUT_MS);
        }

        /* check response (if applicable); complete packet incl. RES_FLAG return to calling function */
        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 != *rxLen)
            {
                if (0 != (rx[PTX_T5T_OFFSET_RES_FLAG] & PTX_T5T_RES_FLAG_ERROR_MASK))
                {
                    status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
                }
            }
            else
            {
                status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T5T_WriteBlock (ptx_NDEF_T5T_t *ndefT5T, uint16_t blockNumber, uint8_t *blockData, uint8_t blockDataLen, uint8_t *rx, uint32_t *rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Aside from actual Tag-component, remaining parameters are checked at lower-layers */
    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP))
    {
        *rxLen = (uint32_t)ndefT5T->RxBufferSize;

        /* send standard- or Extended-Command depending on Block-Number */
        if (0 == ndefT5T->CCParams.ExtCommandTypeRequired)
        {
            status = ptx_NativeTag_T5T_WriteSingleBlock (&ndefT5T->t5t,
                                                         ndefT5T->CCParams.SpecialFrameRequired,
                                                         (uint8_t)blockNumber,
                                                         blockData,
                                                         blockDataLen,
                                                         rx,
                                                         rxLen,
                                                         PTX_T5T_DEFAULT_TIMEOUT_MS);

        }
        else
        {
            status = ptx_NativeTag_T5T_ExtWriteSingleBlock (&ndefT5T->t5t,
                                                            ndefT5T->CCParams.SpecialFrameRequired,
                                                            blockNumber,
                                                            blockData,
                                                            blockDataLen,
                                                            rx,
                                                            rxLen,
                                                            PTX_T5T_DEFAULT_TIMEOUT_MS);
        }

        /* check response (if applicable); complete packet incl. RES_FLAG return to calling function */
        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 != *rxLen)
            {
                if (0 != (rx[PTX_T5T_OFFSET_RES_FLAG] & PTX_T5T_RES_FLAG_ERROR_MASK))
                {
                    status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
                }
            }
            else
            {
                status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
            }
        }

    }

    return status;
}

static ptx_Status_t ptx_NDEF_T5T_LockBlock (ptx_NDEF_T5T_t *ndefT5T, uint16_t blockNumber, uint8_t *rx, uint32_t *rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Aside from actual Tag-component, remaining parameters are checked at lower-layers */
    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP))
    {
        *rxLen = (uint32_t)ndefT5T->RxBufferSize;

        /* send standard- or Extended-Command depending on Block-Number */
        if (0 == ndefT5T->CCParams.ExtCommandTypeRequired)
        {
            status = ptx_NativeTag_T5T_LockSingleBlock (&ndefT5T->t5t,
                                                        ndefT5T->CCParams.SpecialFrameRequired,
                                                        (uint8_t)blockNumber,
                                                        rx,
                                                        rxLen,
                                                        PTX_T5T_DEFAULT_TIMEOUT_MS);

        }
        else
        {
            status = ptx_NativeTag_T5T_ExtLockSingleBlock (&ndefT5T->t5t,
                                                           ndefT5T->CCParams.SpecialFrameRequired,
                                                           blockNumber,
                                                           rx,
                                                           rxLen,
                                                           PTX_T5T_DEFAULT_TIMEOUT_MS);
        }

        /* check response (if applicable); complete packet incl. RES_FLAG return to calling function */
        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 != *rxLen)
            {
                if (0 != (rx[PTX_T5T_OFFSET_RES_FLAG] & PTX_T5T_RES_FLAG_ERROR_MASK))
                {
                    status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
                }
            }
            else
            {
                status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
            }
        }

    }

    return status;
}

static ptx_Status_t ptx_NDEF_T5T_UpdateLength (ptx_NDEF_T5T_t *ndefT5T, uint16_t messageLen, uint8_t lengthSize)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint16_t current_block_nr;
    uint32_t rx_len;
    uint8_t i;
    uint8_t nr_blocks_to_process;
    uint8_t bytes_processed = 0;

    /* Aside from actual Tag-component, remaining parameters are checked at lower-layers */
    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP) && (((uint8_t)1 == lengthSize) || ((uint8_t)3 == lengthSize)))
    {
        switch (ndefT5T->LifeCycle)
        {
            case TagLC_Initialised:
            case TagLC_ReadWrite:
                /* OK */
                status = PTX_STATUS_SUCCESS;
                break;

            default:
                status = PTX_STATUS_E_NDEF_INVALID_TAG_STATE;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* does L-field completely fit into current block ? */
            if ((uint8_t)(ndefT5T->NDEF_TLV_POS_BY + lengthSize) < ndefT5T->BlockSize)
            {
                current_block_nr = ndefT5T->NDEF_TLV_POS_BN;
                nr_blocks_to_process = (uint8_t)1;

            }
            else
            {
                /* does the L-field start at the next block i.e. is the T-field byte the last one of the current block ? */
                if (ndefT5T->NDEF_TLV_POS_BY == (uint8_t)(ndefT5T->BlockSize - 1))
                {
                    current_block_nr = (uint8_t)(ndefT5T->NDEF_TLV_POS_BN + 1);
                    nr_blocks_to_process = (uint8_t)1;

                }
                else
                {
                    /* L-field is split between current and next block */
                    current_block_nr = ndefT5T->NDEF_TLV_POS_BN;
                    nr_blocks_to_process = (uint8_t)2;
                }
            }

            /* read n-blocks to update the length field at once */
            bytes_processed = 0;
            for (i = 0; i < nr_blocks_to_process; i++)
            {
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T5T_ReadBlock(ndefT5T, current_block_nr, &ndefT5T->RxBuffer[0], &rx_len);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* skip status-flag at offset 0 */
                    rx_len--;

                    if (rx_len <= ndefT5T->WorkBufferSize)
                    {
                        (void)memcpy(&ndefT5T->WorkBuffer[bytes_processed], &ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA], rx_len);
                    }
                    else
                    {
                        status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                    }
                    bytes_processed = (uint8_t)(bytes_processed + rx_len);
                }
            }

            /* update L-field */
            if  ((uint8_t)1 == lengthSize)
            {
                ndefT5T->WorkBuffer[(ndefT5T->NDEF_TLV_POS_BY + 1) % ndefT5T->BlockSize] = (uint8_t)messageLen;
            }
            else
            {
                ndefT5T->WorkBuffer[(ndefT5T->NDEF_TLV_POS_BY + 1) % ndefT5T->BlockSize] = ((0 != messageLen) ? ((uint8_t)0xFF) : ((uint8_t)0x00));
                ndefT5T->WorkBuffer[(ndefT5T->NDEF_TLV_POS_BY + 2) % ndefT5T->BlockSize] = (uint8_t)((messageLen & 0xFF00) >> 8);
                ndefT5T->WorkBuffer[(ndefT5T->NDEF_TLV_POS_BY + 3) % ndefT5T->BlockSize] = (uint8_t)((messageLen & 0x00FF) >> 0);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* write L-field */
                bytes_processed = 0;
                for (i = 0; i < nr_blocks_to_process; i++)
                {
                    status = ptx_NDEF_T5T_WriteBlock(ndefT5T, current_block_nr, &ndefT5T->WorkBuffer[bytes_processed], ndefT5T->BlockSize, &ndefT5T->RxBuffer[0], &rx_len);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        bytes_processed = (uint8_t)(bytes_processed + ndefT5T->BlockSize);
                        current_block_nr++;
                    }
                    else
                    {
                        /* error - skip further write-operations */
                        break;
                    }
                }
            }
        }

        /* if everything went OK, update the TLV-length of the message */
        if (PTX_STATUS_SUCCESS == status)
        {
            ndefT5T->NDEF_TLV_LENGTH = messageLen;
        }

    }

    return status;
}

static ptx_Status_t ptx_NDEF_T5TReadNDEFSingleBlocks(ptx_NDEF_T5T_t *ndefT5T, uint8_t *msgBuffer, uint32_t *msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t msg_buffer_size;
    uint16_t current_block_nr;
    uint8_t ndef_tlv_header_size;
    uint8_t init_block_read_offset;
    uint32_t rx_len;
    uint32_t bytes_read = 0;
    uint16_t bytes_received;
    uint16_t bytes_to_copy;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP) && (NULL != msgBuffer) && (NULL != msgLen))
    {
        msg_buffer_size = *msgLen;

        /*
         * determine block-address to start reading (NDEF_TLV_POS_BN defines the block where the NDEF-TLV starts,
         * but not the actual NDEF-message, which could be stored in the next block;
         */
        current_block_nr = ndefT5T->NDEF_TLV_POS_BN;
        ndef_tlv_header_size = (ndefT5T->NDEF_TLV_LENGTH < (uint16_t)PTX_T5T_TLV_LENGTHTHRESHOLD) ? (uint8_t)PTX_T5T_TLV_HEADER_SHORT : (uint8_t)PTX_T5T_TLV_HEADER_LONG;

        if ((uint8_t)((ndefT5T->NDEF_TLV_POS_BY + ndef_tlv_header_size)) < ndefT5T->BlockSize)
        {
            init_block_read_offset = (uint8_t)(ndefT5T->NDEF_TLV_POS_BY + ndef_tlv_header_size);

        }
        else
        {
            init_block_read_offset = (uint8_t)((ndefT5T->NDEF_TLV_POS_BY + ndef_tlv_header_size) % ndefT5T->BlockSize);
            current_block_nr++;
        }

        status = PTX_STATUS_SUCCESS;

        while ((bytes_read < ndefT5T->NDEF_TLV_LENGTH) && (bytes_read < (uint16_t)(ndefT5T->CCParams.MLEN - ndef_tlv_header_size)) && (PTX_STATUS_SUCCESS == status))
        {
            /* read block */
            status = ptx_NDEF_T5T_ReadBlock(ndefT5T, current_block_nr, &ndefT5T->RxBuffer[0], &rx_len);

            if ((PTX_STATUS_SUCCESS == status) && (rx_len >= init_block_read_offset))
            {
                /* skip status-flag at offset 0 */
                rx_len--;

                bytes_received = (uint16_t)(rx_len - init_block_read_offset);

                if ((ndefT5T->NDEF_TLV_LENGTH - bytes_read) > bytes_received)
                {
                    bytes_to_copy = bytes_received;
                }
                else
                {
                    bytes_to_copy = (uint16_t)(ndefT5T->NDEF_TLV_LENGTH - bytes_read);
                }

                /* does received data still fit into the output-buffer ? */
                if ((msg_buffer_size) >= (bytes_to_copy + bytes_read))
                {
                    /* copy NDEF-message content to output buffer */
                    (void)memcpy(&msgBuffer[bytes_read], &ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA + init_block_read_offset], bytes_to_copy);
                    bytes_read = (uint16_t)(bytes_read + bytes_to_copy);
                }
                else
                {
                    status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                }
            }

            init_block_read_offset = 0;
            current_block_nr++;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            *msgLen = bytes_read;
        }
        else
        {
            *msgLen = 0;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T5TReadNDEFMultipleBlocks(ptx_NDEF_T5T_t *ndefT5T, uint8_t *msgBuffer, uint32_t *msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t msg_buffer_size;
    uint16_t nbr_blocks;
    uint16_t nbr_blocks_total;
    uint16_t nbr_blocks_read = 0;
    uint8_t ndef_tlv_header_size;
    uint32_t rx_len = 0;
    uint8_t offset = 0;
    uint8_t first_read = 1;
    uint32_t bytes_read = 0;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP) && (NULL != msgBuffer) && (NULL != msgLen))
    {
        msg_buffer_size = *msgLen;

        ndef_tlv_header_size = (ndefT5T->NDEF_TLV_LENGTH < (uint16_t)PTX_T5T_TLV_LENGTHTHRESHOLD) ? (uint8_t)PTX_T5T_TLV_HEADER_SHORT : (uint8_t)PTX_T5T_TLV_HEADER_LONG;

        /* t5t->NDEF_TLV_LENGTH/PTX_T5T_BLOCKSIZE + 1 OR 0 dependent on divisibility */
        nbr_blocks_total = (uint16_t)(((ndefT5T->NDEF_TLV_LENGTH + ndefT5T->NDEF_TLV_POS_BY + ndef_tlv_header_size) / ndefT5T->BlockSize) + (((ndefT5T->NDEF_TLV_LENGTH + ndefT5T->NDEF_TLV_POS_BY + ndef_tlv_header_size) % ndefT5T->BlockSize) ? (1) : (0)));

        status = PTX_STATUS_SUCCESS;

        while ((nbr_blocks_total > nbr_blocks_read) && (PTX_STATUS_SUCCESS == status))
        {
            nbr_blocks = ptx_NDEF_T5T_SetMTU(ndefT5T, (uint16_t)(nbr_blocks_total - nbr_blocks_read));

            rx_len = ndefT5T->RxBufferSize;

            if (0 == nbr_blocks)
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
            else
            {
                if (1 != ndefT5T->CCParams.ExtCommandTypeRequired)
                {
                    /* Reading multiple blocks returns NB+1 blocks, so we call the functions with one less than what we calculated. */
                    status = ptx_NativeTag_T5T_ReadMultipleBlock(&ndefT5T->t5t, 0, (uint8_t)(ndefT5T->NDEF_TLV_POS_BN + nbr_blocks_read), (uint8_t)(nbr_blocks - 1u), &ndefT5T->RxBuffer[0], &rx_len, PTX_T5T_DEFAULT_TIMEOUT_MS);
                }
                else
                {
                    /* Reading multiple blocks returns NB+1 blocks, so we call the functions with one less than what we calculated. */
                    status = ptx_NativeTag_T5T_ExtReadMultipleBlock(&ndefT5T->t5t, 0, (uint16_t)(ndefT5T->NDEF_TLV_POS_BN + nbr_blocks_read), (uint16_t)(nbr_blocks - 1u), &ndefT5T->RxBuffer[0], &rx_len, PTX_T5T_DEFAULT_TIMEOUT_MS);
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                offset = (first_read) ? ((uint8_t)(ndef_tlv_header_size + PTX_T5T_OFFSET_RES_DATA + ndefT5T->NDEF_TLV_POS_BY)) : (PTX_T5T_OFFSET_RES_DATA);
                if ((msg_buffer_size >= (rx_len - offset)))
                {
                    (void) memcpy(&msgBuffer[bytes_read], &ndefT5T->RxBuffer[offset], rx_len - offset);

                    nbr_blocks_read = (uint16_t)(nbr_blocks_read + nbr_blocks);
                    bytes_read = (uint32_t)(bytes_read + rx_len - offset);

                    /* Set flag. */
                    first_read = 0;
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
                }
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* MsgLen is all bytes read without TLV-Header, beginning offsets, and end offsets. */
            *msgLen = bytes_read;
        }
        else
        {
            *msgLen = 0;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T5T_WriteMessageLoop(ptx_NDEF_T5T_t *ndefT5T, uint32_t msgLen, uint8_t *msgBuffer, uint8_t offset, uint16_t currentBlock, uint8_t tlvHeaderLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t bytes_written = 0;
    uint16_t current_block_nr = currentBlock;
    uint8_t begin_offset = offset;
    uint8_t bytes_to_copy;
    uint32_t rx_len = 0;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP) && (0 != msgLen) && (NULL != msgBuffer))
    {
        status = PTX_STATUS_SUCCESS;

        while ((bytes_written != msgLen) && (bytes_written < (uint16_t)(ndefT5T->CCParams.MLEN - tlvHeaderLen)) && (PTX_STATUS_SUCCESS == status))
        {
            /* prepare NDEF-data for next write-cycle */
            if (msgLen - bytes_written + begin_offset >= ndefT5T->BlockSize)
            {
                /* Need this full block */
                bytes_to_copy = (uint8_t)(ndefT5T->BlockSize - (begin_offset));
            }
            else
            {
                /* Fits into this block */
                bytes_to_copy = (uint8_t)(msgLen - bytes_written);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* last (or first and only) block to be written completely or only partly ? */
                    if ((((uint32_t)(bytes_to_copy + bytes_written) == msgLen)) && (bytes_to_copy != ndefT5T->BlockSize))
                    {
                        if (ndefT5T->BlockSize <= ndefT5T->WorkBufferSize)
                        {
                            (void)memcpy(&ndefT5T->WorkBuffer[begin_offset],
                                         &ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA + begin_offset],
                                         (uint32_t)(ndefT5T->BlockSize - begin_offset));
                        }
                        else
                        {
                            status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                        }
                    }
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                uint16_t temp_var = (uint16_t)(bytes_to_copy + begin_offset);
                if (temp_var <= (uint16_t)ndefT5T->WorkBufferSize)
                {
                    (void)memcpy(&ndefT5T->WorkBuffer[begin_offset], &msgBuffer[bytes_written], bytes_to_copy);
                }
                else
                {
                    status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* enough space left to store the (optional) TERMINATOR-TLV ? */
                if ((((uint32_t)(bytes_to_copy + bytes_written) == msgLen)) && (bytes_to_copy < ndefT5T->BlockSize))
                {
                    ndefT5T->WorkBuffer[bytes_to_copy + begin_offset] = PTX_T5T_TERMINATOR_TLV_T;
                }

                /* write data to Tag */
                status = ptx_NDEF_T5T_WriteBlock(ndefT5T, current_block_nr, &ndefT5T->WorkBuffer[0], ndefT5T->BlockSize, &ndefT5T->RxBuffer[0], &rx_len);

                if (PTX_STATUS_SUCCESS == status)
                {
                    bytes_written = (uint16_t)(bytes_written + bytes_to_copy);
                    begin_offset = 0;
                    current_block_nr++;
                }
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T5T_LockLoop(ptx_NDEF_T5T_t *ndefT5T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint16_t nr_blocks_to_lock;
    uint16_t current_block_nr = 0;
    uint32_t rx_len;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP))
    {
        /* Physically lock every available block (Attention: THIS IS NOT REVERSIBLE!) */
        nr_blocks_to_lock = (uint16_t)((ndefT5T->CCParams.MLEN / ndefT5T->BlockSize) +
                                       (uint16_t)(ndefT5T->CCParams.Size / ndefT5T->BlockSize) +
                                       (uint16_t)(((0 != ndefT5T->CCParams.MLEN % ndefT5T->BlockSize) ? (1u) : (0u)) +
                                                  ((0 != ndefT5T->CCParams.Size % ndefT5T->BlockSize) ? (1u) : (0u))));

        for (current_block_nr = 0; current_block_nr < nr_blocks_to_lock; current_block_nr++)
        {
            /* lock block-by-block */
            status = ptx_NDEF_T5T_LockBlock (ndefT5T, current_block_nr, &ndefT5T->RxBuffer[0], &rx_len);

            if (PTX_STATUS_SUCCESS != status)
            {
                /* Exit loop. */
                current_block_nr = nr_blocks_to_lock;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T5T_ParseTLV(ptx_NDEF_T5T_t *ndefT5T, uint16_t *currentByteNr, uint8_t *data, uint32_t dataLen, uint8_t *ndefTlvFound, uint8_t *terminatorFound)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t current_byte = 0;
    uint16_t current_block = 0;
    uint8_t length_field_len = 0;
    uint8_t second_read_operation = 0;
    uint32_t rx_len = 0;
    uint16_t tlv_len = 0;
    uint8_t tlv_type = 0;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP) && (NULL != currentByteNr) && (NULL != data) && (0 != dataLen) && (NULL != ndefTlvFound) && (NULL != terminatorFound))
    {
        current_byte = *currentByteNr;
        current_block = (uint16_t)(current_byte / ndefT5T->BlockSize);

        for (uint32_t idx = (uint32_t)(current_byte % ndefT5T->BlockSize); idx < dataLen; idx++)
        {
            tlv_type = data[idx];

            /* T-field of NDEF-TLV found? Limit to 1 TLV. */
            if ((0 != tlv_type) && (PTX_T5T_TERMINATOR_TLV_T != tlv_type))
            {
                /* try to read L-field (Attention: Might consist of 1 or 3 bytes; can be split over next block ...) */
                if ((uint8_t)(idx + 1) < (uint8_t)dataLen)
                {
                    /* 1- or 3-byte L-field ?*/
                    if ((uint8_t)0xFE >= data[idx + 1])
                    {
                        /* 1-byte L-field */
                        tlv_len = (uint16_t)data[idx + 1];
                        length_field_len = 1;
                    }
                    else
                    {
                        length_field_len = 3;

                        /* can complete L-field be read (2 additional bytes) ? */
                        if ((uint8_t)(idx + 3) < (uint8_t)dataLen)
                        {
                            tlv_len = (uint16_t)((data[idx + 2] << 8) | (data[idx + 3]));

                            /* can at least 1st part of L-field be read (1 additional byte) ? */
                        }
                        else if ((uint8_t)(idx + 2) < (uint8_t)dataLen)
                        {
                            tlv_len = (uint16_t)((data[idx + 2] << 8));
                            second_read_operation = (uint8_t)PTX_T5T_2ND_READ_OP_3BL_GET_PART_LENGTH;

                        }
                        else
                        {
                            second_read_operation = (uint8_t)PTX_T5T_2ND_READ_OP_3BL_GET_FULL_LENGTH;
                        }
                    }
                }
                else
                {
                    second_read_operation = (uint8_t)PTX_T5T_2ND_READ_OP_GET_FULL_LENGTH;
                }

                if (PTX_T5T_NDEF_MESSAGE_TLV_T == tlv_type)
                {
                    ndefT5T->NDEF_TLV_POS_BN = current_block;
                    ndefT5T->NDEF_TLV_POS_BY = (uint8_t)(idx + (ndefT5T->BlockSize - dataLen));
                }

                /* get L-field from next block (if necessary) */
                if (PTX_T5T_2ND_READ_OP_NOT_REQUIRED == second_read_operation)
                {
                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    current_block++;

                    /* read block */
                    status = ptx_NDEF_T5T_ReadBlock(ndefT5T, current_block, &ndefT5T->RxBuffer[0], &rx_len);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        switch (second_read_operation)
                        {
                            case PTX_T5T_2ND_READ_OP_GET_FULL_LENGTH:
                                /* 1- or 3-byte L-field ?*/
                                if ((uint8_t)0xFE >= ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA + 0])
                                {
                                    /* 1-byte L-field */
                                    tlv_len = (uint16_t)ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA + 0];

                                }
                                else
                                {
                                    tlv_len = (uint16_t)((ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA + 0] << 8) | \
                                                         (ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA + 1]));
                                }
                                break;

                            case PTX_T5T_2ND_READ_OP_3BL_GET_PART_LENGTH:
                                tlv_len = (uint16_t)(ndefT5T->NDEF_TLV_LENGTH |
                                                     (uint16_t)((ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA + 0])));
                                break;

                            case PTX_T5T_2ND_READ_OP_3BL_GET_FULL_LENGTH:
                                tlv_len = (uint16_t)((ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA + 0] << 8) | \
                                                     (ndefT5T->RxBuffer[PTX_T5T_OFFSET_RES_DATA + 1]));
                                break;

                            default:
                                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                                break;
                        }
                    }
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    if (PTX_T5T_NDEF_MESSAGE_TLV_T == tlv_type)
                    {
                        ndefT5T->NDEF_TLV_LENGTH = tlv_len;
                        *ndefTlvFound = (uint8_t)1;
                    }

                    /* Increment current byte number overall and within data block. */
                    current_byte = (uint16_t)(current_byte + (1u + length_field_len + tlv_len));
                    idx = idx + (1u + length_field_len + tlv_len);
                }
            }
            else
            {
                /* T-field of TERMINATOR-TLV found ? */
                if (PTX_T5T_TERMINATOR_TLV_T == tlv_type)
                {
                    /* Terminator-TLV determines the end of the reading-procedure */
                    *terminatorFound = (uint8_t)1;
                }
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            *currentByteNr = current_byte;
        }
    }

    return status;
}

static uint16_t ptx_NDEF_T5T_SetMTU (ptx_NDEF_T5T_t *ndefT5T, uint16_t nbrBlocks)
{
    uint8_t transfer_len;
    uint16_t max_nbr_blocks = 0;

    if (PTX_COMP_CHECK(ndefT5T, Comp_T5TOP))
    {
        transfer_len = 253u - 1u; /* max bytes - RES_FLAG */

        if (transfer_len < (nbrBlocks * ndefT5T->BlockSize))
        {
            max_nbr_blocks = (transfer_len / ndefT5T->BlockSize);
        }
        else
        {
            max_nbr_blocks = nbrBlocks;
        }
    }

    return max_nbr_blocks;
}

