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
    Module      : NFC Data Exchange Format (NDEF) T3T
    File        : ptx_NDEF_T3T.c

    Description : NFC Data Exchange Format (NDEF) implementation to control NDEF Messages on T3T Devices.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NDEF_T3T.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
/*
 * NDEF Specific Communication Defines
 *
 */
#define PTX_T3TOP_READ_ATTRIBUTE_INFORMATION_BLOCK_SC           (uint16_t)0x000B        /* Service Code for reading Attribute Information Block. */
#define PTX_T3TOP_WRITE_ATTRIBUTE_INFORMATION_BLOCK_SC          (uint16_t)0x0009        /* Service Code for writing Attribute Information Block. */
#define PTX_T3TOP_ATTRIBUTE_INFORMATION_BYTES                   (uint8_t)0x0E           /* Number of Attribute Information Block data bytes. */
#define PTX_T3TOP_ATTRIBUTE_INFORMATION_BYTES_CHECKSUM          (uint8_t)0x10           /* Number of Attribute Information Block data bytes, including checksum. */
#define PTX_T3TOP_SENSF_REQ_SC_NDEF                             (uint16_t)0x12FC        /* Service Code for SENSF-REQ. */
#define PTX_T3TOP_READ_NDEF_SC                                  (uint16_t)0x000B        /* Service Code for NDEF Read. */
#define PTX_T3TOP_WRITE_NDEF_SC                                 (uint16_t)0x0009        /* Service Code for NDEF Write. */
#define PTX_T3TOP_NOS_NDEF                                      (uint8_t)0x01           /* Number of Services for NDEF operations. */
#define PTX_T3TOP_NOB_NDEF                                      (uint8_t)0x01           /* Number of Blocks for specific NDEF operations. */
#define PTX_T3TOP_BLOCK_SIZE                                    (uint8_t)0x10           /* T3T block size. */
#define PTX_T3TOP_NBR_INTERNAL                                  (uint8_t)14u            /* Internal maximum blocks for read operations. */
#define PTX_T3TOP_NBW_INTERNAL                                  (uint8_t)14u            /* Internal maximum blocks for write operations. */
#define PTX_T3TOP_BLOCKLIST_2BYTES_BASE                         (uint16_t)0x8000        /* Block number base for 2 byte block numbers. */
#define PTX_T3TOP_BLOCKLIST_3BYTES_BASE                         (uint32_t)0x00000000    /* Block number base for 3 byte block numbers. */
/*  */

/*
 * Access Flags
 *
 */
#define PTX_T3TOP_WRITE_FLAG_ON                                 (uint8_t)0x0F           /* Attribute Information Block Write Flag ON. */
#define PTX_T3TOP_WRITE_FLAG_OFF                                (uint8_t)0x00           /* Attribute Information Block Write Flag OFF. */
#define PTX_T3TOP_READ_ONLY_FLAG                                (uint8_t)0x00           /* Attribute Information Block access condition Read Only. */
/*  */

/*
 * Response Offsets and Status Flags
 *
 */
#define PTX_T3TOP_CHECK_RSP_OFFSET_DATA                         (uint8_t)0x0D           /* Offset of data in a read response frame. */
#define PTX_T3TOP_CHECK_RSP_OFFSET_STATUSFLAG1                  (uint8_t)0x0A           /* Offset of status flag 1 in a read response frame. */
#define PTX_T3TOP_CHECK_RSP_OFFSET_STATUSFLAG2                  (uint8_t)0x0B           /* Offset of status flag 2 in a read response frame. */
#define PTX_T3TOP_CHECK_RSP_OFFSET_NUMBER_BYTES                 (uint8_t)0x0C           /* Offset of length information in a read response frame. */
#define PTX_T3TOP_STATUSFLAG1_OK                                (uint8_t)0x00           /* Status Flag 1 OK. */
#define PTX_T3TOP_STATUSFLAG2_OK                                (uint8_t)0x00           /* Status Flag 2 OK. */
#define PTX_T3TOP_UPDATE_RSP_OFFSET_STATUSFLAG1                 (uint8_t)0x0A           /* Offset of status flag 1 in a write response frame. */
#define PTX_T3TOP_UPDATE_RSP_OFFSET_STATUSFLAG2                 (uint8_t)0x0B           /* Offset of status flag 2 in a write response frame. */
#define PTX_T3TOP_UPDATE_RSP_LENGTH                             (uint8_t)0x0C           /* Update response length. */
#define PTX_T3TOP_UPDATE_RSP_STATUSFLAG2_ACCESS_DENIED          (uint8_t)0x70           /* Status Flag 2 Access Denied. */
#define PTX_T3TOP_UPDATE_RSP_STATUSFLAG2_MAX_NUM_WRITES         (uint8_t)0x71           /* Status Flag 2 Maximum number of writes to tag reached. */
/*  */

/*
 * Attribute information Offsets
 *
 */
#define PTX_T3T_ATTRIB_OFFSET_VERSION                           (uint8_t)0u             /* Attribute Information Block offset for version information. */
#define PTX_T3T_ATTRIB_OFFSET_MAX_READ_BLOCKS                   (uint8_t)1u             /* Attribute Information Block offset for maximum number of blocks handled per read operation. */
#define PTX_T3T_ATTRIB_OFFSET_MAX_WRITE_BLOCKS                  (uint8_t)2u             /* Attribute Information Block offset for maximum number of blocks handled per write operation. */
#define PTX_T3T_ATTRIB_OFFSET_NBR_AVAILABLE_BLOCKS_HIGH         (uint8_t)3u             /* Attribute Information Block offset for upper byte of number of available blocks. */
#define PTX_T3T_ATTRIB_OFFSET_NBR_AVAILABLE_BLOCKS_LOW          (uint8_t)4u             /* Attribute Information Block offset for lower byte of number of available blocks. */
#define PTX_T3T_ATTRIB_OFFSET_RFU                               (uint8_t)5u             /* Attribute Information Block offset for RFU bytes. */
#define PTX_T3T_ATTRIB_OFFSET_WRITE_FLAG                        (uint8_t)9u             /* Attribute Information Block offset for Write Flag. */
#define PTX_T3T_ATTRIB_OFFSET_RWFLAG                            (uint8_t)10u            /* Attribute Information Block offset for access conditions. */
#define PTX_T3T_ATTRIB_OFFSET_NDEF_DATA_LENGTH_HIGH             (uint8_t)11u            /* Attribute Information Block offset for upper byte of available NDEF data length. */
#define PTX_T3T_ATTRIB_OFFSET_NDEF_DATA_LENGTH_MID              (uint8_t)12u            /* Attribute Information Block offset for middle byte of available NDEF data length. */
#define PTX_T3T_ATTRIB_OFFSET_NDEF_DATA_LENGTH_LOW              (uint8_t)13u            /* Attribute Information Block offset for lower byte of available NDEF data length. */
#define PTX_T3T_ATTRIB_OFFSET_CHECKSUM_HIGH                     (uint8_t)14u            /* Attribute Information Block offset for upper byte of checksum. */
#define PTX_T3T_ATTRIB_OFFSET_CHECKSUM_LOW                      (uint8_t)15u            /* Attribute Information Block offset for lower byte of checksum. */
/*  */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / HELPERS
 * ####################################################################################################################
 */
/*
 *  T3T Read Operation.
 *
 *  ndefT3T             Pointer to component.
 *  NFCID2                Tag ID.
 *  NFCID2Len             Length of Tag ID.
 *  serviceInfo           Information about Services to be updated.
 *  blockInfo             Information about Blocks to be updated.
 *  rx                Rx Buffer to store response.
 *  rxLen             Maximum Rx Buffer length on input, Response length on output.
 *  msTimeout             Timeout in ms.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NDEF_T3T_Check  (ptx_NDEF_T3T_t *ndefT3T,
                                         ptx_NativeTag_T3T_Services_t serviceInfo,
                                         ptx_NativeTag_T3T_Blocks_t blockInfo,
                                         uint8_t *rx,
                                         uint32_t *rxLen,
                                         uint32_t msTimeout);

/*
 *  T3T Write Operation.
 *
 *  ndefT3T             Pointer to component.
 *  NFCID2                Tag ID.
 *  NFCID2Len             Length of Tag ID.
 *  serviceInfo           Information about Services to be updated.
 *  blockInfo             Information about Blocks to be updated.
 *  blockData             Data to be written.
 *  blockDataLen          Length of Data to be written.
 *  rx                Rx Buffer to store response.
 *  rxLen             Maximum Rx Buffer length on input, Response length on output.
 *  msTimeout             Timeout in ms.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NDEF_T3T_Update (ptx_NDEF_T3T_t *ndefT3T,
                                         ptx_NativeTag_T3T_Services_t serviceInfo,
                                         ptx_NativeTag_T3T_Blocks_t blockInfo,
                                         uint8_t *blockData,
                                         uint8_t blockDataLen,
                                         uint8_t *rx,
                                         uint32_t *rxLen,
                                         uint32_t msTimeout);

/*
 *  Read Message loop.
 *
 *      ndefT3T             Pointer to component.
 *     msgLen                Length of Message read.
 *  msgBuffer             Message data buffer.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NDEF_T3T_ReadLoop(ptx_NDEF_T3T_t *ndefT3T, uint32_t *msgLen, uint8_t *msgBuffer);

/*
 *  Write Message Loop.
 *
 *  msgLen                Length of Message data.
 *  msgBuffer             Message data buffer.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NDEF_T3T_WriteLoop (ptx_NDEF_T3T_t *ndefT3T, uint32_t msgLen, uint8_t *msgBuffer);

/*
 *  Get Attribute Information.
 *
 *  ndefT3T             Pointer to component.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NDEF_T3T_GetAttributeInformationBlock (ptx_NDEF_T3T_t *ndefT3T);

/*
 *  Set Attribute Information.
 *
 *  ndefT3T             Pointer to component.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NDEF_T3T_SetAttributeInformationBlock (ptx_NDEF_T3T_t *ndefT3T);

/*
 *  Calculate Communication Checksum.
 *
 *  ndefT3T             Pointer to component.
 *  checksum             Calculated Checksum.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NDEF_T3T_CalculateChecksum (ptx_NDEF_T3T_t *ndefT3T, uint16_t *checksum);

/*
 *  Generate a BlockList, assuming all blocks from startBlock onwards are to be used.
 *
 *  blockList            BlockList.
 *  blockListLen      Maximum BlockList Capacity on input, Block List Length on output.
 *  nob                   Number of Blocks.
 *  maxNob                Maximum Number of Blocks.
 *  startBlock            First Block in the List.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_NDEF_T3T_GenerateBlockList(uint8_t* blockList, uint16_t *blockListLen, uint16_t nob, uint16_t maxNob, uint16_t startBlock);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NDEF_T3T_Open (ptx_NDEF_T3T_t *ndefT3T, ptx_NDEF_T3T_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;


    if ((NULL != ndefT3T) && (NULL != initParams))
    {
        if ((NULL != initParams->rul) &&
            (NULL != initParams->TxBuffer) &&
            (0 != initParams->TxBufferSize) &&
            (NULL != initParams->RxBuffer) &&
            (0 != initParams->RxBufferSize))
        {
            /* clear component */
            (void)memset(ndefT3T, 0, sizeof(ptx_NDEF_T3T_t));

            ndefT3T->RxBuffer = initParams->RxBuffer;
            ndefT3T->RxBufferSize = (uint32_t)(initParams->RxBufferSize);
            ndefT3T->LifeCycle = TagLC_NoNDEFTag;


            memcpy(ndefT3T->NFCID2, &initParams->rul->CardRegistry.ActiveCard->TechParam.TypeF.SensFRes[PTX_T3T_SENSFRES_OFFSET_NFCID2], PTX_T3T_NFCID2_SIZE);
            ndefT3T->MRTI_Check = initParams->rul->CardRegistry.ActiveCard->TechParam.TypeF.SensFRes[PTX_T3T_SENSFRES_OFFSET_MRTI_CHECK];
            ndefT3T->MRTI_Update = initParams->rul->CardRegistry.ActiveCard->TechParam.TypeF.SensFRes[PTX_T3T_SENSFRES_OFFSET_MRTI_UPDATE];

            /* initialise lower layer component */
            ptx_NativeTag_T3T_InitParams_t T3T_init_params;
            (void)memset(&T3T_init_params, 0, sizeof(ptx_NativeTag_T3T_InitParams_t));

            T3T_init_params.rul = initParams->rul;
            T3T_init_params.TxBuffer = initParams->TxBuffer;
            T3T_init_params.TxBufferSize = (uint32_t)(initParams->TxBufferSize);
            memcpy(T3T_init_params.NFCID2, ndefT3T->NFCID2, PTX_T3T_NFCID2_SIZE);
            T3T_init_params.NFCID2Len = PTX_T3T_NFCID2_SIZE;
            T3T_init_params.MRTI_Check = ndefT3T->MRTI_Check;
            T3T_init_params.MRTI_Update = ndefT3T->MRTI_Update;

            status = ptx_NativeTag_T3T_Open(&ndefT3T->t3t, &T3T_init_params);

            /* set Component-ID at the end to prevent further calls in case of an error */
            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT3T->CompId = Comp_T3TOP;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T3T_FormatTag (ptx_NDEF_T3T_t *ndefT3T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP))
    {
        status = PTX_STATUS_E_NDEF_UNSUPPORTED;
    }

    return status;
}

ptx_Status_t ptx_NDEF_T3T_CheckMessage (ptx_NDEF_T3T_t *ndefT3T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t nfcid2[PTX_T3T_NFCID2_SIZE];
    ptx_NativeTag_T3T_MRTI_t mrti_info;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP))
    {
        /* set NFCID2 */
        (void)memcpy(&nfcid2[0], &ndefT3T->NFCID2[0], PTX_T3T_NFCID2_SIZE);
        mrti_info.MRTICheck = ndefT3T->MRTI_Check;
        mrti_info.MRTIUpdate = ndefT3T->MRTI_Update;

        status =  ptx_NativeTag_T3T_SetTagParams(&ndefT3T->t3t,
                                                 &nfcid2[0],
                                                 PTX_T3T_NFCID2_SIZE,
                                                 1u,           /* Until we know the maximum amount of blocks, assume 1. */
                                                 1u,
                                                 mrti_info);


        if (PTX_STATUS_SUCCESS == status)
        {
            /* read attribute information block (equivalent to CC params) */
            status = ptx_NDEF_T3T_GetAttributeInformationBlock(ndefT3T);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (PTX_T3TOP_READ_ONLY_FLAG != ndefT3T->CCParams.RWFlag)
            {
                ndefT3T->LifeCycle = (0 != ndefT3T->CCParams.Ln) ? TagLC_ReadWrite : TagLC_Initialised;

            }
            else
            {
                ndefT3T->LifeCycle = TagLC_ReadOnly;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T3T_ReadMessage (ptx_NDEF_T3T_t *ndefT3T, uint8_t *msgBuffer, uint32_t *msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t msg_buffer_size;
    uint8_t nfcid2[PTX_T3T_NFCID2_SIZE];
    ptx_NativeTag_T3T_MRTI_t mrti_info;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP) && (NULL != msgBuffer) && (NULL != msgLen))
    {
        msg_buffer_size = *msgLen;

        if (0 != msg_buffer_size)
        {
            switch (ndefT3T->LifeCycle)
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

            /* check if previous writing procedures have finished */
            if (PTX_T3TOP_WRITE_FLAG_ON == ndefT3T->CCParams.WriteFlag)
            {
                status = PTX_STATUS_E_NDEF_T3T_WRITE_IN_PROGRESS;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* set NFCID2 */
                (void)memcpy(&nfcid2[0], &ndefT3T->NFCID2[0], (uint32_t)PTX_T3T_NFCID2_SIZE);
                mrti_info.MRTICheck = ndefT3T->MRTI_Check;
                mrti_info.MRTIUpdate = ndefT3T->MRTI_Update;


                status =  ptx_NativeTag_T3T_SetTagParams(&ndefT3T->t3t,
                                                         &nfcid2[0],
                                                         PTX_T3T_NFCID2_SIZE,
                                                         ndefT3T->CCParams.NbrInt,
                                                         ndefT3T->CCParams.NbwInt,
                                                         mrti_info);


                if (PTX_STATUS_SUCCESS == status)
                {
                    /* read blocks */
                    status = ptx_NDEF_T3T_ReadLoop(ndefT3T, msgLen, msgBuffer);
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

    }

    return status;
}

ptx_Status_t ptx_NDEF_T3T_WriteMessage (ptx_NDEF_T3T_t *ndefT3T, uint8_t *msgBuffer, uint32_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t nfcid2[PTX_T3T_NFCID2_SIZE];
    uint16_t msg_len;
    uint8_t EMPTY_NDEF_MESSAGE[] = {0x0D, 0x00, 0x00};
    uint8_t *msg_buffer;
    uint16_t nbr_data_blocks;

    ptx_NativeTag_T3T_MRTI_t mrti_info;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP))
    {
        switch (ndefT3T->LifeCycle)
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
            /* set NFCID2 */
            (void)memcpy(&nfcid2[0], &ndefT3T->NFCID2[0], (uint32_t)PTX_T3T_NFCID2_SIZE);
            mrti_info.MRTICheck = ndefT3T->MRTI_Check;
            mrti_info.MRTIUpdate = ndefT3T->MRTI_Update;

            status =  ptx_NativeTag_T3T_SetTagParams(&ndefT3T->t3t,
                                                     &nfcid2[0],
                                                     PTX_T3T_NFCID2_SIZE,
                                                     ndefT3T->CCParams.NbrInt,
                                                     ndefT3T->CCParams.NbwInt,
                                                     mrti_info);
        }

        /* check for other write procedure in progress */
        if (PTX_T3TOP_WRITE_FLAG_ON == ndefT3T->CCParams.WriteFlag)
        {
            status = PTX_STATUS_E_NDEF_INVALID_TAG_STATE;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* check for empty message */
            if ((0 != msgLen) && (NULL != msgBuffer))
            {
                msg_buffer = msgBuffer;
                msg_len = (uint16_t)msgLen;
            }
            else
            {
                msg_buffer = &EMPTY_NDEF_MESSAGE[0];
                msg_len = sizeof(EMPTY_NDEF_MESSAGE);
            }

            nbr_data_blocks = (uint16_t)(msg_len / PTX_T3TOP_BLOCK_SIZE);
            if (0 != (msg_len % PTX_T3TOP_BLOCK_SIZE))
            {
                nbr_data_blocks++;
            }

            if (ndefT3T->CCParams.NmaxB >= nbr_data_blocks)
            {
                /* write flag needs to be set to ON */
                ndefT3T->CCParams.WriteFlag = PTX_T3TOP_WRITE_FLAG_ON;
                status = ptx_NDEF_T3T_SetAttributeInformationBlock(ndefT3T);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T3T_WriteLoop(ndefT3T, msg_len, msg_buffer);
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* write is done, check if write flag needs to be set to OFF and write Ln field */

                    ndefT3T->CCParams.WriteFlag = PTX_T3TOP_WRITE_FLAG_OFF;

                    /* update message length infos */
                    ndefT3T->CCParams.Ln = (uint32_t)msgLen;
                    ndefT3T->CCParams.Nbc = (ndefT3T->CCParams.Ln % 16);
                    if (0 != (ndefT3T->CCParams.Ln % 16))
                    {
                        ndefT3T->CCParams.Nbc++;
                    }
                    status = ptx_NDEF_T3T_SetAttributeInformationBlock(ndefT3T);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        switch (msgLen)
                        {
                            case 0:
                                ndefT3T->LifeCycle = TagLC_Initialised;
                                break;
                            default:
                                ndefT3T->LifeCycle = TagLC_ReadWrite;
                                break;
                        }
                    }
                }
                else
                {
                    /* write might have failed, write flag needs to be set to OFF */
                    ndefT3T->CCParams.WriteFlag = PTX_T3TOP_WRITE_FLAG_OFF;
                    (void)ptx_NDEF_T3T_SetAttributeInformationBlock(ndefT3T);
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T3T_LockTag (ptx_NDEF_T3T_t *ndefT3T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t nfcid2[PTX_T3T_NFCID2_SIZE];
    ptx_NativeTag_T3T_MRTI_t mrti_info;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP))
    {
        if (0 == ndefT3T->CCParams.Ln)
        {
            status = PTX_STATUS_E_NDEF_INVALID_TAG_STATE;
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* set NFCID2 */
            (void)memcpy(&nfcid2[0], &ndefT3T->NFCID2[0], (uint32_t)PTX_T3T_NFCID2_SIZE);
            mrti_info.MRTICheck = ndefT3T->MRTI_Check;
            mrti_info.MRTIUpdate = ndefT3T->MRTI_Update;

            status =  ptx_NativeTag_T3T_SetTagParams(&ndefT3T->t3t,
                                                     &nfcid2[0],
                                                     PTX_T3T_NFCID2_SIZE,
                                                     ndefT3T->CCParams.NbrInt,
                                                     ndefT3T->CCParams.NbwInt,
                                                     mrti_info);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ndefT3T->CCParams.RWFlag = PTX_T3TOP_READ_ONLY_FLAG;

            status = ptx_NDEF_T3T_SetAttributeInformationBlock(ndefT3T);

            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT3T->LifeCycle = TagLC_ReadOnly;
            }
        }

    }

    return status;
}

ptx_Status_t ptx_NDEF_T3T_Close (ptx_NDEF_T3T_t *ndefT3T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP))
    {
        status = ptx_NativeTag_T3T_Close(&ndefT3T->t3t);
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK(s)
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NDEF_T3T_Check (ptx_NDEF_T3T_t *ndefT3T,
                                        ptx_NativeTag_T3T_Services_t serviceInfo,
                                        ptx_NativeTag_T3T_Blocks_t blockInfo,
                                        uint8_t *rx,
                                        uint32_t *rxLen,
                                        uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t num_bytes_res;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP) &&
        (NULL != serviceInfo.ServiceCodeList) && (NULL != blockInfo.BlockList) &&
        (0 != serviceInfo.ServiceCodeListLen) && (0 != blockInfo.BlockListLen) &&
        (0 != serviceInfo.NOS) && (0 != blockInfo.NOB) &&
        (NULL != rx) && (NULL != rxLen))
    {
        *rxLen = (uint32_t)ndefT3T->RxBufferSize;

        /* send native tag command */
        status = ptx_NativeTag_T3T_Check(&ndefT3T->t3t, serviceInfo, blockInfo, rx, rxLen, msTimeout);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* handle response */
            if ((0 != *rxLen) && (PTX_T3TOP_CHECK_RSP_OFFSET_DATA <= *rxLen))
            {
                /* OK, check response */
                if ((PTX_T3TOP_STATUSFLAG1_OK != rx[PTX_T3TOP_CHECK_RSP_OFFSET_STATUSFLAG1]) || (PTX_T3TOP_STATUSFLAG2_OK != rx[PTX_T3TOP_CHECK_RSP_OFFSET_STATUSFLAG2]))
                {
                    /* error, no block data received */
                    status = PTX_STATUS_E_NDEF_TAG_NACK;
                }
                else
                {
                    /* OK, result stays in RxBuffer */
                    num_bytes_res = (uint8_t)(rx[PTX_T3TOP_CHECK_RSP_OFFSET_NUMBER_BYTES] * PTX_T3TOP_BLOCK_SIZE);

                    memmove(ndefT3T->RxBuffer, &ndefT3T->RxBuffer[PTX_T3TOP_CHECK_RSP_OFFSET_DATA], sizeof(uint8_t) * num_bytes_res);
                    *rxLen = num_bytes_res;
                }
            }
            else
            {
                status = PTX_STATUS_E_NSC_RFERROR;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T3T_Update (ptx_NDEF_T3T_t *ndefT3T,
                                         ptx_NativeTag_T3T_Services_t serviceInfo,
                                         ptx_NativeTag_T3T_Blocks_t blockInfo,
                                         uint8_t *blockData,
                                         uint8_t blockDataLen,
                                         uint8_t *rx,
                                         uint32_t *rxLen,
                                         uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP) &&
        (NULL != serviceInfo.ServiceCodeList) && (NULL != blockInfo.BlockList) &&
        (0 != serviceInfo.ServiceCodeListLen) && (0 != blockInfo.BlockListLen) &&
        (0 != serviceInfo.NOS) && (0 != blockInfo.NOB) &&
        (NULL != rx) && (NULL != rxLen) && (NULL != blockData))
    {
        *rxLen = (uint32_t)ndefT3T->RxBufferSize;

        /* send native tag command */
        status = ptx_NativeTag_T3T_Update(&ndefT3T->t3t, serviceInfo, blockInfo, blockData, blockDataLen, rx, rxLen, msTimeout);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* handle response */
            if (0 != *rxLen)
            {
                /* OK, check response */
                if ((PTX_T3TOP_UPDATE_RSP_LENGTH <= *rxLen) && (PTX_T3TOP_STATUSFLAG1_OK != rx[PTX_T3TOP_UPDATE_RSP_OFFSET_STATUSFLAG1]) && (PTX_T3TOP_STATUSFLAG2_OK != rx[PTX_T3TOP_UPDATE_RSP_OFFSET_STATUSFLAG2]))
                {
                    /* error */
                    switch (rx[PTX_T3TOP_UPDATE_RSP_OFFSET_STATUSFLAG2])
                    {
                        case PTX_T3TOP_UPDATE_RSP_STATUSFLAG2_ACCESS_DENIED:
                            /* cannot write to memory */
                            status = PTX_STATUS_E_NDEF_TAG_ACCESS_DENIED;
                            break;
                        case PTX_T3TOP_UPDATE_RSP_STATUSFLAG2_MAX_NUM_WRITES:
                            /* operation successful, but exceeded the max number of writes to memory */
                            break;
                        default:
                            status = PTX_STATUS_E_NSC_RFERROR;
                            break;
                    }

                }
                else
                {
                    /* OK */
                }
            }
            else
            {
                status = PTX_STATUS_E_NSC_RFERROR;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T3T_ReadLoop(ptx_NDEF_T3T_t *ndefT3T, uint32_t *msgLen, uint8_t *msgBuffer)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t current_block = 1;
    uint32_t rx_len;
    uint32_t bytes_to_copy;
    uint16_t nr_ndef_blocks = ndefT3T->CCParams.Nbc;
    uint16_t blocks_to_read = 0;
    uint32_t bytes_read = 0;
    uint32_t msg_len = 0;

    /* One Entry is 2 bytes. */
    uint8_t service_code_list[PTX_T3TOP_NOS_NDEF * 2];
    service_code_list[0] = (PTX_T3TOP_READ_NDEF_SC & 0xFF);
    service_code_list[1] = (PTX_T3TOP_READ_NDEF_SC >> 8u);

    uint8_t block_list[PTX_T3TOP_NBR_INTERNAL * 3u];
    uint16_t block_list_len = PTX_T3TOP_NBR_INTERNAL * 3u;

    ptx_NativeTag_T3T_Services_t service_info;
    ptx_NativeTag_T3T_Blocks_t block_info;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP) && (NULL != msgLen) && (NULL != msgBuffer))
    {
        msg_len = (uint32_t)(*msgLen);

        status = PTX_STATUS_SUCCESS;
        while ((ndefT3T->CCParams.Nbc > (current_block - 1)) && (PTX_STATUS_SUCCESS == status))
        {
            block_list_len = PTX_T3TOP_NBR_INTERNAL * 3u;

            if ((nr_ndef_blocks - (current_block - 1)) <= ndefT3T->CCParams.NbrInt)
            {
                blocks_to_read = (uint16_t)(nr_ndef_blocks - (current_block - 1));
            }
            else
            {
                blocks_to_read = ndefT3T->CCParams.NbrInt;
            }
            status = ptx_NDEF_T3T_GenerateBlockList(block_list, &block_list_len, blocks_to_read, ndefT3T->CCParams.NbrInt, current_block);

            if (PTX_STATUS_SUCCESS == status)
            {
                block_info.NOB = (uint8_t)blocks_to_read;
                block_info.BlockList = block_list;
                block_info.BlockListLen = (uint8_t)block_list_len;

                service_info.NOS = PTX_T3TOP_NOS_NDEF;
                service_info.ServiceCodeList = service_code_list;
                service_info.ServiceCodeListLen = PTX_T3TOP_NOS_NDEF * 2u;

                status = ptx_NDEF_T3T_Check(ndefT3T, service_info, block_info, ndefT3T->RxBuffer, &rx_len, ndefT3T->t3t.TagTimeoutCheck);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* in case of last read operation only copy NDEF bytes to message buffer */
                bytes_to_copy = (uint32_t)((ndefT3T->CCParams.Ln < (uint32_t)(bytes_read + (uint32_t)(blocks_to_read * PTX_T3TOP_BLOCK_SIZE)))
                                           ? (uint32_t)(ndefT3T->CCParams.Ln - bytes_read)
                                           : (uint32_t)(blocks_to_read * PTX_T3TOP_BLOCK_SIZE));

                if (msg_len >= (bytes_read + bytes_to_copy))
                {
                    (void) memcpy(&msgBuffer[bytes_read], &ndefT3T->RxBuffer[0], bytes_to_copy);

                    bytes_read = (uint32_t)(bytes_read + bytes_to_copy);
                    current_block = (uint16_t)(current_block + blocks_to_read);
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
                }
            }
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

static ptx_Status_t ptx_NDEF_T3T_WriteLoop (ptx_NDEF_T3T_t *ndefT3T, uint32_t msgLen, uint8_t *msgBuffer)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t nbr_padding_bytes = 0;
    uint32_t bytes_written = 0;
    uint16_t current_block = 1;
    uint16_t blocks_to_write = 0;
    uint16_t nbr_data_blocks = 0;
    uint32_t rx_len;

    /* One Entry is 2 bytes. */
    uint8_t service_code_list[PTX_T3TOP_NOS_NDEF * 2];
    service_code_list[0] = (PTX_T3TOP_WRITE_NDEF_SC & 0xFF);
    service_code_list[1] = (PTX_T3TOP_WRITE_NDEF_SC >> 8u);

    uint8_t block_data[PTX_T3TOP_BLOCK_SIZE * PTX_T3TOP_NBW_INTERNAL];
    uint8_t block_list[PTX_T3TOP_NBR_INTERNAL * 3u];
    uint16_t block_list_len = PTX_T3TOP_NBR_INTERNAL * 3u;

    ptx_NativeTag_T3T_Services_t service_info;
    ptx_NativeTag_T3T_Blocks_t block_info;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP) && (NULL != msgBuffer) && (0 != msgLen))
    {
        status = PTX_STATUS_SUCCESS;

        nbr_data_blocks = (uint16_t)(msgLen / PTX_T3TOP_BLOCK_SIZE);
        if (0 != (msgLen % PTX_T3TOP_BLOCK_SIZE))
        {
            nbr_data_blocks++;
        }

        if (ndefT3T->CCParams.NmaxB < nbr_data_blocks)
        {
            status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
        }

        while ((msgLen > bytes_written) && (PTX_STATUS_SUCCESS == status))
        {
            block_list_len = PTX_T3TOP_NBW_INTERNAL * 3u;

            if ((nbr_data_blocks - (current_block - 1)) <= ndefT3T->CCParams.NbwInt)
            {
                blocks_to_write = (uint16_t)(nbr_data_blocks - (current_block - 1));
            }
            else
            {
                blocks_to_write = ndefT3T->CCParams.NbwInt;
            }
            status = ptx_NDEF_T3T_GenerateBlockList(block_list, &block_list_len, blocks_to_write, ndefT3T->CCParams.NbwInt, current_block);

            /* find out if padding is needed and fill data block */
            if ((msgLen < (bytes_written + (uint32_t)((blocks_to_write * PTX_T3TOP_BLOCK_SIZE)))) && (0 != ((msgLen - bytes_written) % PTX_T3TOP_BLOCK_SIZE)) && (PTX_STATUS_SUCCESS == status))
            {
                nbr_padding_bytes = (uint8_t)(PTX_T3TOP_BLOCK_SIZE - ((msgLen - bytes_written) % PTX_T3TOP_BLOCK_SIZE));
            }
            else
            {
                nbr_padding_bytes = 0;
            }

            if (((PTX_T3TOP_BLOCK_SIZE * PTX_T3TOP_NBW_INTERNAL) >= ((PTX_T3TOP_BLOCK_SIZE * blocks_to_write) - nbr_padding_bytes)) && (msgLen >= (uint32_t)(bytes_written + (uint32_t)((PTX_T3TOP_BLOCK_SIZE * blocks_to_write) - nbr_padding_bytes))))
            {
                (void)memcpy(&block_data[0], &msgBuffer[bytes_written], (uint32_t)(PTX_T3TOP_BLOCK_SIZE * blocks_to_write) - nbr_padding_bytes);
            }
            else
            {
                status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
            }

            if ((0 != nbr_padding_bytes) && ((PTX_T3TOP_BLOCK_SIZE * PTX_T3TOP_NBW_INTERNAL) > (PTX_T3TOP_BLOCK_SIZE * blocks_to_write) - nbr_padding_bytes))
            {
                (void)memset(&block_data[(PTX_T3TOP_BLOCK_SIZE * blocks_to_write) - nbr_padding_bytes], 0x00, nbr_padding_bytes);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                block_info.NOB = (uint8_t)blocks_to_write;
                block_info.BlockList = block_list;
                block_info.BlockListLen = (uint8_t)block_list_len;

                service_info.NOS = PTX_T3TOP_NOS_NDEF;
                service_info.ServiceCodeList = service_code_list;
                service_info.ServiceCodeListLen = PTX_T3TOP_NOS_NDEF * 2u;

                status = ptx_NDEF_T3T_Update(ndefT3T, service_info, block_info, block_data, (uint8_t)(PTX_T3TOP_BLOCK_SIZE * block_info.NOB), ndefT3T->RxBuffer, &rx_len, ndefT3T->t3t.TagTimeoutUpdate);

                if (PTX_STATUS_SUCCESS == status)
                {
                    bytes_written = (uint32_t)(bytes_written + (uint32_t)((PTX_T3TOP_BLOCK_SIZE * blocks_to_write) - nbr_padding_bytes));
                    current_block = (uint16_t)(current_block + blocks_to_write);
                    (void)memset(&block_data[0], 0x00, PTX_T3TOP_BLOCK_SIZE);
                }
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T3T_CalculateChecksum (ptx_NDEF_T3T_t *ndefT3T, uint16_t *checksum)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP) && (NULL != checksum))
    {
        *checksum = 0;
        ndefT3T->CCParams.Checksum = (uint16_t)(((uint16_t)ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_CHECKSUM_HIGH] << 8u) | (uint16_t)ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_CHECKSUM_LOW]);

        for (uint8_t i = 0; i < PTX_T3TOP_ATTRIBUTE_INFORMATION_BYTES; i++)
        {
            *checksum = (uint16_t)(*checksum + ndefT3T->CCParams.AttributeInformationBlock[i]);
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T3T_GetAttributeInformationBlock (ptx_NDEF_T3T_t *ndefT3T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t service_code_list[PTX_T3TOP_NOS_NDEF * 2u];
    uint8_t block_list[PTX_T3TOP_NOB_NDEF * 2u];
    uint32_t rx_len;
    uint16_t checksum = 0;

    ptx_NativeTag_T3T_Services_t service_info;
    ptx_NativeTag_T3T_Blocks_t block_info;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP))
    {
        service_code_list[0] = (PTX_T3TOP_READ_ATTRIBUTE_INFORMATION_BLOCK_SC & 0xFF);
        service_code_list[1] = (PTX_T3TOP_READ_ATTRIBUTE_INFORMATION_BLOCK_SC >> 8u);
        block_list[0] = 0x80;
        block_list[1] = 0x00;

        service_info.NOS = PTX_T3TOP_NOS_NDEF;
        service_info.ServiceCodeList = service_code_list;
        service_info.ServiceCodeListLen = PTX_T3TOP_NOS_NDEF * 2u;

        block_info.NOB = PTX_T3TOP_NOB_NDEF;
        block_info.BlockList = block_list;
        block_info.BlockListLen = PTX_T3TOP_NOB_NDEF * 2u;

        /* read the attribute information block */
        status = ptx_NDEF_T3T_Check(ndefT3T, service_info, block_info, ndefT3T->RxBuffer, &rx_len, ndefT3T->t3t.TagTimeoutCheck);
        if ((PTX_STATUS_SUCCESS == status) && (PTX_T3TOP_ATTRIBUTE_INFORMATION_BYTES_CHECKSUM == rx_len))
        {
            (void)memcpy(&ndefT3T->CCParams.AttributeInformationBlock[0], &ndefT3T->RxBuffer[0], (uint32_t)rx_len);

            status = ptx_NDEF_T3T_CalculateChecksum(ndefT3T, &checksum);

            if (ndefT3T->CCParams.Checksum == checksum)
            {
                /* fill the components */
                ndefT3T->CCParams.Version = ndefT3T->CCParams.AttributeInformationBlock[0];
                if ((PTX_T3T_SUPPORTED_VERSION >> 4u) != (ndefT3T->CCParams.Version >> 4u))
                {
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                }
                ndefT3T->CCParams.MajorVersion = (ndefT3T->CCParams.Version >> 4u);
                ndefT3T->CCParams.MinorVersion = (ndefT3T->CCParams.Version & 0x0F);

                ndefT3T->CCParams.Nbr = ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_MAX_READ_BLOCKS];
                ndefT3T->CCParams.Nbw = ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_MAX_WRITE_BLOCKS];

                /* Check if we can actually do the maximum */
                if (PTX_T3TOP_NBR_INTERNAL < ndefT3T->CCParams.Nbr)
                {
                    ndefT3T->CCParams.NbrInt = PTX_T3TOP_NBR_INTERNAL;
                }
                else
                {
                    ndefT3T->CCParams.NbrInt = ndefT3T->CCParams.Nbr;
                }
                if (PTX_T3TOP_NBW_INTERNAL < ndefT3T->CCParams.Nbw)
                {
                    ndefT3T->CCParams.NbwInt = PTX_T3TOP_NBW_INTERNAL;
                }
                else
                {
                    ndefT3T->CCParams.NbwInt = ndefT3T->CCParams.Nbw;
                }

                /* byte 3 upper, byte 4 lower */
                ndefT3T->CCParams.NmaxB = (uint16_t)(((uint16_t)ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_NBR_AVAILABLE_BLOCKS_HIGH] << 8u) | (uint16_t)ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_NBR_AVAILABLE_BLOCKS_LOW]);

                (void)memcpy(&ndefT3T->CCParams.RFU[0], &ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_RFU], (uint32_t)PTX_T3T_RFU_SIZE);

                ndefT3T->CCParams.WriteFlag = ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_WRITE_FLAG];
                ndefT3T->CCParams.RWFlag = ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_RWFLAG];

                ndefT3T->CCParams.Ln  = (((uint32_t)ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_NDEF_DATA_LENGTH_HIGH] << 16u) | ((uint32_t)ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_NDEF_DATA_LENGTH_MID] << 8u) | (uint32_t)ndefT3T->CCParams.AttributeInformationBlock[PTX_T3T_ATTRIB_OFFSET_NDEF_DATA_LENGTH_LOW]);
                if (0 == ndefT3T->CCParams.Ln % PTX_T3T_BLOCK_SIZE)
                {
                    ndefT3T->CCParams.Nbc = (uint16_t)(ndefT3T->CCParams.Ln / PTX_T3T_BLOCK_SIZE);
                }
                else
                {
                    ndefT3T->CCParams.Nbc = (uint16_t)((ndefT3T->CCParams.Ln / PTX_T3T_BLOCK_SIZE) + 1);
                }
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
        else
        {
            status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T3T_SetAttributeInformationBlock (ptx_NDEF_T3T_t *ndefT3T)
{
    /* translate the attributes back into bytes and write the block */
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t attribute_information[PTX_T3T_BLOCK_SIZE];
    uint8_t service_code_list[PTX_T3TOP_NOS_NDEF * 2u];
    uint8_t block_list[PTX_T3TOP_NOB_NDEF * 2u];
    uint32_t rx_len;
    uint16_t checksum = 0;

    ptx_NativeTag_T3T_Services_t service_info;
    ptx_NativeTag_T3T_Blocks_t block_info;

    if (PTX_COMP_CHECK(ndefT3T, Comp_T3TOP))
    {
        /* get the components into a data block (already updated outside the function) */
        attribute_information[PTX_T3T_ATTRIB_OFFSET_VERSION]  = ndefT3T->CCParams.Version;
        attribute_information[PTX_T3T_ATTRIB_OFFSET_MAX_READ_BLOCKS]  = ndefT3T->CCParams.Nbr;
        attribute_information[PTX_T3T_ATTRIB_OFFSET_MAX_WRITE_BLOCKS]  = ndefT3T->CCParams.Nbw;

        attribute_information[PTX_T3T_ATTRIB_OFFSET_NBR_AVAILABLE_BLOCKS_HIGH]  = (uint8_t)(ndefT3T->CCParams.NmaxB >> 8u);
        attribute_information[PTX_T3T_ATTRIB_OFFSET_NBR_AVAILABLE_BLOCKS_LOW]  = (uint8_t)(ndefT3T->CCParams.NmaxB & 0xFF);

        (void)memcpy(&attribute_information[PTX_T3T_ATTRIB_OFFSET_RFU], &ndefT3T->CCParams.RFU[0], (uint32_t)PTX_T3T_RFU_SIZE);

        attribute_information[PTX_T3T_ATTRIB_OFFSET_WRITE_FLAG]  = ndefT3T->CCParams.WriteFlag;
        attribute_information[PTX_T3T_ATTRIB_OFFSET_RWFLAG] = ndefT3T->CCParams.RWFlag;

        attribute_information[PTX_T3T_ATTRIB_OFFSET_NDEF_DATA_LENGTH_HIGH] = (uint8_t)(ndefT3T->CCParams.Ln >> 16u);
        attribute_information[PTX_T3T_ATTRIB_OFFSET_NDEF_DATA_LENGTH_MID] = (uint8_t)((ndefT3T->CCParams.Ln >> 8u) & 0xFF);
        attribute_information[PTX_T3T_ATTRIB_OFFSET_NDEF_DATA_LENGTH_LOW] = (uint8_t)(ndefT3T->CCParams.Ln & 0xFF);

        memcpy(&ndefT3T->CCParams.AttributeInformationBlock[0], &attribute_information[0], PTX_T3TOP_ATTRIBUTE_INFORMATION_BYTES);

        status = ptx_NDEF_T3T_CalculateChecksum(ndefT3T, &checksum);
        if (PTX_STATUS_SUCCESS == status)
        {
            attribute_information[PTX_T3T_ATTRIB_OFFSET_CHECKSUM_HIGH] = (uint8_t)(checksum >> 8u);
            attribute_information[PTX_T3T_ATTRIB_OFFSET_CHECKSUM_LOW] = (uint8_t)(checksum & 0xFF);

            (void)memcpy(&ndefT3T->CCParams.AttributeInformationBlock[0], &attribute_information[0], (uint32_t)PTX_T3T_BLOCK_SIZE);
            /* write attribute block */
            service_code_list[0] = (PTX_T3TOP_WRITE_ATTRIBUTE_INFORMATION_BLOCK_SC & 0xFF);
            service_code_list[1] = (PTX_T3TOP_WRITE_ATTRIBUTE_INFORMATION_BLOCK_SC >> 8u);
            block_list[0] = (PTX_T3TOP_BLOCKLIST_2BYTES_BASE >> 8u);
            block_list[1] = (PTX_T3TOP_BLOCKLIST_2BYTES_BASE & 0xFF);

            service_info.NOS = PTX_T3TOP_NOS_NDEF;
            service_info.ServiceCodeList = service_code_list;
            service_info.ServiceCodeListLen = PTX_T3TOP_NOS_NDEF * 2u;

            block_info.NOB = PTX_T3TOP_NOB_NDEF;
            block_info.BlockList = block_list;
            block_info.BlockListLen = PTX_T3TOP_NOB_NDEF * 2u;

            status = ptx_NDEF_T3T_Update(ndefT3T, service_info, block_info, attribute_information, sizeof(attribute_information), ndefT3T->RxBuffer, &rx_len, ndefT3T->t3t.TagTimeoutUpdate);
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T3T_GenerateBlockList(uint8_t* blockList, uint16_t *blockListLen, uint16_t nob, uint16_t maxNob, uint16_t startBlock)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t block_list_len = 0;
    uint8_t entry_len = 0;
    uint32_t entry = 0;
    uint16_t list_offset = 0;

    if ((NULL != blockList) && (NULL != blockListLen) && (0 != *blockListLen) && (maxNob >= nob))
    {
        for (uint8_t idx = 0; idx < nob; idx++)
        {
            entry = (uint32_t)(startBlock + idx);
            entry_len = (0xFF < entry) ? (3u) : (2u);

            if (*blockListLen >= block_list_len + entry_len) /* Will the BlockNumber still fit? */
            {
                switch (entry_len)
                {
                    case 2:
                        entry = entry | PTX_T3TOP_BLOCKLIST_2BYTES_BASE;

                        blockList[list_offset] = (uint8_t)(entry >> 8u);
                        list_offset++;
                        blockList[list_offset] = (uint8_t)(entry & 0xFF);
                        list_offset++;
                        status = PTX_STATUS_SUCCESS;
                        break;

                    case 3:
                        entry = entry | PTX_T3TOP_BLOCKLIST_3BYTES_BASE;

                        blockList[list_offset] = (uint8_t)(0x00);
                        list_offset++;
                        blockList[list_offset] = (uint8_t)(entry & 0xFF);
                        list_offset++;
                        blockList[list_offset] = (uint8_t)(((entry >> 8u) & 0xFF));
                        list_offset++;
                        status = PTX_STATUS_SUCCESS;
                        break;

                    default:
                        /* Cannot happen. */
                        status = PTX_STATUS_E_GEN_INTERNAL;
                        break;
                }

                block_list_len = (uint16_t)(block_list_len + entry_len);
            }
            else
            {
                status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            *blockListLen = block_list_len;
        }
        else
        {
            *blockListLen = 0;
        }
    }

    return status;
}

