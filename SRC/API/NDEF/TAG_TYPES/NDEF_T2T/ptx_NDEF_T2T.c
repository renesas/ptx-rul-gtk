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
    Module      : NFC Data Exchange Format (NDEF) T2T
    File        : ptx_NDEF_T2T.c

    Description : NFC Data Exchange Format (NDEF) implementation to control NDEF Messages on T2T Devices.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NDEF_T2T.h"
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * Buffer limit checkers
 *
 */
#define PTX_T2T_LOCKAREA_BUFSIZE            (uint8_t)(PTX_T2T_MAX_NUMBER_LOCK_CONTROL * 2)   /* Buffer size for storing Lock Areas. */
#define PTX_T2T_RSVDAREA_BUFSIZE            (uint8_t)(PTX_T2T_MAX_NUMBER_MEMORY_CONTROL * 2) /* Buffer size for storing Reserved Areas. */
/*  */

/*
 * Important Block Numbers
 *
 */
#define PTX_T2T_STATIC_LOCK_BLOCK_NUMBER    (uint8_t)0x02    /* Block number containing Static Lock Bits. */
#define PTX_T2T_CC_BLOCK_NUMBER             (uint8_t)0x03    /* Block number containing CC. */
#define PTX_T2T_TLV_AREA_BEGIN_BLOCK        (uint16_t)0x0004 /* Block number of first TLV area block. */
/*  */

/*
 * CC Offsets and Parameters
 *
 */
#define PTX_T2T_CC_OFFSET_MAGIC_NUMBER      (uint8_t)0x00    /* CC Block offset for Magic Number. */
#define PTX_T2T_CC_OFFSET_VERSION_INFO      (uint8_t)0x01    /* CC Block offset for version information. */
#define PTX_T2T_CC_OFFSET_TAG_SIZE          (uint8_t)0x02    /* CC Block offset for tag memory size. */
#define PTX_T2T_CC_OFFSET_ACCESS_COND       (uint8_t)0x03    /* CC Block offset for access conditions. */
#define PTX_T2T_CC_MAGIC_NUMBER             (uint8_t)0xE1    /* T2T Magic Number. */
#define PTX_T2T_CC_ACCESS_BITS              (uint8_t)0x0F    /* Mask for access condition bits. */
#define PTX_T2T_CC_MLEN_FACTOR              (uint8_t)8u      /* Multiplication factor for tag size in bytes. */
/*  */

/*
 * Bit Shift Masks
 *
 */
#define PTX_T2T_MASK_15                     (uint8_t)0x0F    /* Mask 15. */
#define PTX_T2T_MASK_8                      (uint8_t)0x08    /* Mask 8. */
#define PTX_T2T_MASK_4                      (uint8_t)0x04    /* Mask 4. */
#define PTX_T2T_MASK_1                      (uint8_t)0x01    /* Mask 1. */
/*  */

/*
 * TLV Parameters
 *
 */
#define PTX_T2T_NULL_TLV_T                  (uint8_t)0x00    /* NULL TLV T-value */
#define PTX_T2T_LOCK_CONTROL_TLV_T          (uint8_t)0x01    /* Lock Control TLV T-value. */
#define PTX_T2T_MEMORY_CONTROL_TLV_T        (uint8_t)0x02    /* Memory Control TLV T-value. */
#define PTX_T2T_NDEF_TLV_T                  (uint8_t)0x03    /* NDEF TLV T-value. */
#define PTX_T2T_TERMINATOR_TLV_T            (uint8_t)0xFE    /* Terminator TLV T-value. */
#define PTX_T2T_LOCK_CONTROL_TLV_LENGTH     (uint8_t)0x03    /* Lock Control TLV L-value. */
#define PTX_T2T_MEMORY_CONTROL_TLV_LENGTH   (uint8_t)0x03    /* Memory Control TLV L-value. */
#define PTX_T2T_TERMINATOR_TLV_LENGTH       (uint8_t)0x01    /* Terminator TLV length. */
#define PTX_T2T_TERMINATOR_TLV_PRESENT      (uint8_t)0x01    /* Flag to use when a Terminator TLV is found. */
#define PTX_T2T_NO_TERMINATOR_TLV           (uint8_t)0x00    /* Flag to use when no Terminator TLV is found. */
#define PTX_T2T_NDEF_L_FIELD_SHORT          (uint8_t)1u      /* NDEF TLV L-value for short TLVs. */
#define PTX_T2T_NDEF_L_FIELD_LONG           (uint8_t)3u      /* NDEF TLV L-value for long TLVs. */
#define PTX_T2T_MAX_NDEF_BYTE_OFFSET        (uint8_t)3u      /* Maximum offset of data byte within a block. */
#define PTX_T2T_NDEF_TLV_T_OFFSET           (uint8_t)0u      /* NDEF TLV T-value offset. */
#define PTX_T2T_NDEF_TLV_L_OFFSET           (uint8_t)1u      /* NDEF TLV L-value offset. */
/*  */

/*
 * Operation Size Informations
 *
 */
#define PTX_T2T_BLOCKSIZE                   (uint8_t)4u      /* T2T block size. */
#define PTX_T2T_BLOCKS_PER_READ             (uint8_t)4u      /* T2T blocks read per read operation. */
#define PTX_T2T_MIN_SIZE_FOR_DLA            (uint8_t)0x30    /* Minimum memory size for dynamic lock areas to be present. */
#define PTX_T2T_MAX_NBR_NDEF_TLV            (uint8_t)1u      /* Internal maximum of NDEF TLVs to be handled. */
#define PTX_T2T_SECTORSIZE                  (uint16_t)256u   /* T2T sector size. */
#define PTX_T2T_LENGTHTHRESHOLD             (uint8_t)0xFF    /* Max Length for 1 L-Byte, above 3 L-Bytes are used. */
#define PTX_T2T_SECTORTHRESHOLD             (uint8_t)0xFF    /* Block Address threshold within a block where a new Sector will be needed. */
/*  */

/*
 * Access Condition Values
 *
 */
#define PTX_T2T_ACCESS_GRANTED              (uint8_t)0x00    /* Access condition granted. */
#define PTX_T2T_ACCESS_DENIED               (uint8_t)0xFF    /* Access condition denied. */
/*  */

/*
 * Lock Procedure Parameters
 *
 */
#define PTX_T2T_BYTES_LOCKED_PER_BIT        (uint8_t)8u      /* Default number of bytes locked per lock bit. */
#define PTX_T2T_LOCKBITS                    (uint8_t)0xFF    /* All lock bits of a lock byte set to locked state. */
#define PTX_T2T_ACCESS_READONLY             (uint8_t)0xFF    /* Access condition Read Only. */
/*  */

/*
 * Versioning Parameters
 *
 */
#define PTX_T2T_VALID_MAJOR_VERSION         (uint8_t)1u      /* Major version supported. */
#define PTX_T2T_VALID_MINOR_VERSION         (uint8_t)0u      /* Smallest minor version supported. */
/*  */

/*
 * Passive Timeout Acknowledge according to NFC-Forum Standard.
 *
 */
#define PTX_T2T_PAT_MS                      (uint8_t)2u      /* Passive acknowledge timeout. */
/*  */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / HELPERS
 * ####################################################################################################################
 */
/*
 *  Get CC Parameters.
 *
 *  ndefT2T         Pointer to component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_GetCCInfo (ptx_NDEF_T2T_t *ndefT2T);

/*
 *  Handle Tag LC with CC Information.
 *
 *  ndefT2T         Pointer to component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_HandleCCInfo(ptx_NDEF_T2T_t *ndefT2T);

/*
 *  Read Data Blocks.
 *
 *  ndefT2T         Pointer to component.
 *  blockNumber       Block Number to start at.
 *  rx            Rx Buffer to write Data into.
 *  rxLen         Rx Buffer Length, maximum on input, Data length on output.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_ReadBlocks  (ptx_NDEF_T2T_t *ndefT2T, uint8_t blockNumber, uint8_t *rx, uint32_t *rxLen);

/*
 *  Write Data Block.
 *
 *  ndefT2T         Pointer to component.
 *  blockNumber       Block Number to write to.
 *  blockData         Data to write.
 *  blockDataLen      Length of Data to write.
 *  rx            Rx Buffer to write Response into.
 *  rxLen         Rx Buffer Length, maximum on input, Response length on output.
 *  msTimeout         Timeout in ms.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_WriteBlock  (ptx_NDEF_T2T_t *ndefT2T, uint8_t blockNumber, uint8_t *blockData, uint8_t blockDataLen, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout);

/*
 *  Select a Sector.
 *  ndefT2T         Pointer to component.
 *  secNr             Sector Number to select.
 *  rx            Rx Buffer to write Response into.
 *  rxLen         Rx Buffer Length, maximum on input, Response length on output.
 *  msTimeout         Timeout in ms.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_SectorSelect(ptx_NDEF_T2T_t *ndefT2T, uint8_t secNr, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout);
/*
 *  Read all available NDEF TLVs.
 *
 *  ndefT2T             Pointer to component.
 *  msgBuffer         Buffer to store Messages.
 *  msgBufferOffset   Offset of Buffer to start filling on input, Buffer length at output.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_ReadNDEFTLVs (ptx_NDEF_T2T_t *ndefT2T, uint32_t msgBufferSize, uint8_t *msgBuffer, uint16_t *msgBufferOffset);

/*
 *  Read Loop for a single NDEF TLV.
 *
 *            ndefT2T       Pointer to component.
 *            msgLen          Message length to read.
 *        msgBuffer       Buffer to store Messages.
 *        bytesRead       Number of bytes read.
 *        msgBufferOffset Offset within message buffer for data read.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_ReadLoop(ptx_NDEF_T2T_t *ndefT2T, uint32_t msgLen, uint8_t *msgBuffer, uint16_t *bytesRead, uint16_t *msgBufferOffset);

/*
 *  Specific TLV to be overwritten.
 *
 *            ndefT2T       Pointer to component.
 *            tx              Message data to be written.
 *            txLen           Length of Message.
 *            ndefTLVIndex    Index of TLV to be overwritten. If greater that existing number, new TLV is added.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_OverwriteMessage (ptx_NDEF_T2T_t *ndefT2T, uint8_t *tx, uint32_t txLen, uint8_t ndefTLVIndex);

/*
 *  A function called after getting the CC, sweeping the TLV area and getting all already present TLVs on the tag.
 *
 *  ndefT2T          Pointer to component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_TLVAreaCrawler(ptx_NDEF_T2T_t *ndefT2T);

/*
 *  Find possible TLVs from last read in WorkBuffer.
 *
 *  ndefT2T          Pointer to component, contains WorkBuffer to be analyzed.
 *  bytesToSkip      Pointer to counter of skip-able bytes.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_IdentifyTLV   (ptx_NDEF_T2T_t *ndefT2T, uint16_t *bytesToSkip);

/*
 *  Handle Identification of possible NDEF TLV.
 *
 *  ndefT2T         Pointer to component, contains WorkBuffer to be analyzed.
 *  offset          Offset in internal WorkBuffer to start at, offset at which to continue after analysis.
 *  bytesToSkip     Pointer to counter of skip-able bytes.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_NDEFTLV_Handler(ptx_NDEF_T2T_t *ndefT2T, uint32_t *offset, uint16_t *bytesToSkip);

/*
 *  Update NDEF Length TLV L field.
 *
 *  ndefT2T         Pointer to component.
 *  msgLen            Length to be set.
 *  lengthSize        Number of Length Bytes to be set.
 *  ndefTLVIndex      Index of NDEF TLV which is to be updated.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_UpdateLength  (ptx_NDEF_T2T_t *ndefT2T, uint16_t msgLen, uint8_t lengthSize, uint8_t ndefTLVIndex);

/*
 *  Process LockControl TLVs.
 *
 *  ndefT2T             Pointer to component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_DLAProcessor  (ptx_NDEF_T2T_t *ndefT2T);

/*
 *  Process MemoryControl TLVs.
 *
 *  ndefT2T             Pointer to component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_RAProcessor   (ptx_NDEF_T2T_t *ndefT2T);

/*
 *  Check if Area that is to be written to contains a DLA or RA.
 *
 *  ndefT2T             Pointer to component.
 *
 *  bytesToProcess    Number of Bytes to be written on input, Number of Bytes that can be written on output.
 *  preserveBytes        Byte Addresses which need to be preserved.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_AreaChecker   (ptx_NDEF_T2T_t *ndefT2T, uint16_t *bytesToProcess, uint16_t *preserveBytes);

/*
 *  DLA and RA handling during Write.
 *
 *  ndefT2T             Pointer to component.
 *  bytesToWrite      Number of Bytes to be written.
 *  bytesWritten      Number of Bytes written after preservation of DLA and RA.
 *  endOffset             Offset at the end of the last Block to be written.
 *  preserveAddresses     List of Byte Addresses to be preserved.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_DataPreservation(ptx_NDEF_T2T_t *ndefT2T, uint16_t bytesToWrite, uint16_t *bytesWritten, uint8_t endOffset, uint16_t preserveAddresses[]);

/*
 *  Function to check whether writing Data would overlap with other TLVs.
 *
 *  ndefT2T             Pointer to component.
 *  overwriteByteAddress  Either start address of TLV to be overwritten, or start address of new memory space.
 *  overwriteLength       Length of new TLV oject.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_TLVProtection (ptx_NDEF_T2T_t *ndefT2T, uint16_t overwriteByteAddress, uint16_t overwriteLength);

/*
 *  Generate a NDEF TLV.
 *
 *  ndefT2T             Pointer to component.
 *  tlvLen                Length of the TLV. Dependent on this, either 1 or 3 L-bytes are set.
 * [in.out] buff              Buffer receiving the TLV, contains the V data on input.
 *  buffLen           Buffer length. Contains maximum length on input.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_TLVGenerator(ptx_NDEF_T2T_t *ndefT2T, uint32_t tlvLen, uint8_t *buff, uint16_t *buffLen);

/*
 *  Loop over a DLA to lock it.
 *
 *  ndefT2T               Pointer to component.
 *  nbrLockBytes            Number of Bytes to be locked.
 *  nbrReservedBits         Number of reserved, non-lock bits.
 *  firstOffset             Offset of first Byte within the block.
 *  lastOffset              Offset of last Byte within the block.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_DLALoop (ptx_NDEF_T2T_t *ndefT2T,
                                          uint16_t nbrLockBytes,
                                          uint8_t nbrReservedBits,
                                          uint8_t firstOffset,
                                          uint8_t lastOffset);

/*
 *  Default DLA Lock procedure.
 *
 *  ndefT2T               Pointer to component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_LockDefault(ptx_NDEF_T2T_t *ndefT2T);

/*
 *  Lock using DLAs specified in LockControl TLVs.
 *
 *  ndefT2T               Pointer to component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_LockDLA(ptx_NDEF_T2T_t *ndefT2T);
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NDEF_T2T_Open (ptx_NDEF_T2T_t *ndefT2T, ptx_NDEF_T2T_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ndefT2T) && (NULL != initParams))
    {
        if ((NULL != initParams->RxBuffer) &&
            (0 != initParams->RxBufferSize) &&
            (NULL != initParams->WorkBuffer) &&
            (0 != initParams->WorkBufferSize))
        {
            /* clear component */
            (void)memset(ndefT2T, 0, sizeof(ptx_NDEF_T2T_t));

            ndefT2T->WorkBuffer = initParams->WorkBuffer;
            ndefT2T->WorkBufferSize = initParams->WorkBufferSize;
            ndefT2T->RxBuffer = initParams->RxBuffer;
            ndefT2T->RxBufferSize = initParams->RxBufferSize;
            ndefT2T->LifeCycle = TagLC_NoNDEFTag;

            /* initialise lower layer component */
            ptx_NativeTag_T2T_InitParams_t t2t_init_params;
            (void)memset(&t2t_init_params, 0, sizeof(ptx_NativeTag_T2T_InitParams_t));
            t2t_init_params.rul = initParams->rul;
            t2t_init_params.TxBuffer = initParams->TxBuffer;
            t2t_init_params.TxBufferSize = initParams->TxBufferSize;

            status = ptx_NativeTag_T2T_Open(&ndefT2T->t2t, &t2t_init_params);

            /* set Component-ID at the end to prevent further calls in case of an error */
            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT2T->CompId = Comp_T2TOP;
            }

        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T2T_FormatTag (ptx_NDEF_T2T_t *ndefT2T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        status = PTX_STATUS_E_NDEF_UNSUPPORTED;
    }

    return status;
}

ptx_Status_t ptx_NDEF_T2T_CheckMessage (ptx_NDEF_T2T_t *ndefT2T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint16_t non_ndef_bytes = 0;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        /* reset TLV data */
        (void)memset(&ndefT2T->TLVs, 0, sizeof(ptx_NDEF_T2T_TLV_t));

        status = ptx_NDEF_T2T_GetCCInfo(ndefT2T);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NDEF_T2T_HandleCCInfo(ndefT2T);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* read the whole tag and look for TLVs */
                status = ptx_NDEF_T2T_TLVAreaCrawler(ndefT2T);
            }

            /* find NDEF available space */
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_DLAProcessor(ndefT2T);
                if (PTX_STATUS_SUCCESS == status)
                {
                    non_ndef_bytes = (uint16_t)(non_ndef_bytes + (ndefT2T->TLVs.NumberOfLockTLVs * (PTX_T2T_LOCK_CONTROL_TLV_LENGTH + 2))); /* X (fixed) value bytes, 1 T and 1 L byte per TLV */
                    for (uint8_t i = 0; i < ndefT2T->TLVs.NumberOfLockTLVs; i++)
                    {
                        non_ndef_bytes = (uint16_t)(non_ndef_bytes + (ndefT2T->TLVs.LockControlTLVs[i].LockArea[1] - ndefT2T->TLVs.LockControlTLVs[i].LockArea[0]));
                    }

                    status = ptx_NDEF_T2T_RAProcessor(ndefT2T);
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        non_ndef_bytes = (uint16_t)(non_ndef_bytes + (ndefT2T->TLVs.NumberOfMemoryTLVs * (PTX_T2T_MEMORY_CONTROL_TLV_LENGTH + 2))); // X (fixed) value bytes, 1 T and 1 L byte per TLV
                        for (uint8_t i = 0; i < ndefT2T->TLVs.NumberOfMemoryTLVs; i++)
                        {
                            non_ndef_bytes = (uint16_t)(non_ndef_bytes + (ndefT2T->TLVs.MemoryControlTLVs[i].RsvdArea[1] - ndefT2T->TLVs.MemoryControlTLVs[i].RsvdArea[0]));
                        }

                        if (    (PTX_T2T_NO_TERMINATOR_TLV != ndefT2T->TLVs.TerminatorTLV.TerminatorTLVFound)
                             && (0U != ndefT2T->TLVs.NumberOfNdefTLVs)  )
                        {
                            /* Only update the non_ndef_bytes if there is at least one TLV found, otherwise might be a leftover 0xFE byte */
                            non_ndef_bytes++;
                        }

                        ndefT2T->TLVs.AvailableNdefLength = (uint16_t)(ndefT2T->CCParams.Size - non_ndef_bytes);
                    }
                }
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ndefT2T->LastOperationCheck = 1u;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T2T_ReadMessage (ptx_NDEF_T2T_t *ndefT2T, uint8_t *msgBuffer, uint32_t *msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t *msg_buffer;
    uint32_t msg_buffer_size;
    uint16_t msg_buffer_offset = 0;



    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (NULL != msgBuffer) && (NULL != msgLen))
    {
        msg_buffer_size = *msgLen;
        if (0 != msg_buffer_size)
        {
            msg_buffer = msgBuffer;

            switch (ndefT2T->LifeCycle)
            {
                case TagLC_Initialised:
                /* FALLTHROUGH */
                case TagLC_ReadWrite:
                /* FALLTHROUGH */
                case TagLC_ReadOnly:
                    status = PTX_STATUS_SUCCESS;
                    break;

                default:
                    status = PTX_STATUS_E_NDEF_INVALID_TAG_STATE;

            }
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_ReadNDEFTLVs(ndefT2T, msg_buffer_size, msg_buffer, &msg_buffer_offset);

                if (PTX_STATUS_SUCCESS == status)
                {
                    *msgLen = msg_buffer_offset;
                }
                else
                {
                    *msgLen = 0;
                }

                ndefT2T->LastOperationCheck = 0u;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T2T_WriteMessage (ptx_NDEF_T2T_t *ndefT2T, uint8_t *msgBuffer, uint32_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;



    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (NULL != msgBuffer))
    {
        status = ptx_NDEF_T2T_OverwriteMessage(ndefT2T, msgBuffer, msgLen, 0);
        ndefT2T->LastOperationCheck = 0u;
    }

    return status;
}

ptx_Status_t ptx_NDEF_T2T_LockTag (ptx_NDEF_T2T_t *ndefT2T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t rx_len;


    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        /* set static lock area lock bits to 1 */
        if (0 != ndefT2T->SectorParams.CurrentSector)
        {
            status = ptx_NDEF_T2T_SectorSelect(ndefT2T, 0, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_PAT_MS);
        }
        else
        {
            /* So far no errors. */
            status = PTX_STATUS_SUCCESS;
        }

        /* set CC access condition to read-only since CC will be locked by static lock bytes */
        if (PTX_STATUS_SUCCESS == status)
        {
            ndefT2T->WorkBuffer[PTX_T2T_CC_OFFSET_MAGIC_NUMBER] = ndefT2T->CCParams.MagicNumber;
            ndefT2T->WorkBuffer[PTX_T2T_CC_OFFSET_VERSION_INFO] = ndefT2T->CCParams.Version;
            ndefT2T->WorkBuffer[PTX_T2T_CC_OFFSET_TAG_SIZE]     = (uint8_t)ndefT2T->CCParams.MLEN;

            /* set CC access condition to read only */
            ndefT2T->WorkBuffer[PTX_T2T_CC_OFFSET_ACCESS_COND] = (uint8_t)(PTX_T2T_ACCESS_READONLY >> PTX_T2T_MASK_4);

            rx_len = ndefT2T->RxBufferSize;
            status = ptx_NDEF_T2T_WriteBlock(ndefT2T, PTX_T2T_CC_BLOCK_NUMBER, &ndefT2T->WorkBuffer[0], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);
            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT2T->LifeCycle = TagLC_ReadOnly;
            }
        }

        /* set static lock bytes */
        if (PTX_STATUS_SUCCESS == status)
        {
            rx_len = ndefT2T->RxBufferSize;
            status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, PTX_T2T_STATIC_LOCK_BLOCK_NUMBER, &ndefT2T->RxBuffer[0], &rx_len);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* copy internal byte 8 and 9 to work buffer */
                (void)memcpy(&ndefT2T->WorkBuffer[0], &ndefT2T->RxBuffer[0], (uint32_t)2);

                /* set buffer component for static lock bytes */
                ndefT2T->WorkBuffer[2] = PTX_T2T_LOCKBITS;
                ndefT2T->WorkBuffer[3] = PTX_T2T_LOCKBITS;

                status = ptx_NDEF_T2T_WriteBlock(ndefT2T, PTX_T2T_STATIC_LOCK_BLOCK_NUMBER, &ndefT2T->WorkBuffer[0], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);
            }
        }

        if ((PTX_T2T_MIN_SIZE_FOR_DLA < ndefT2T->CCParams.Size) && (PTX_STATUS_SUCCESS == status))
        {
            if (0 == ndefT2T->TLVs.NumberOfLockTLVs)
            {
                status = ptx_NDEF_T2T_LockDefault(ndefT2T);
            }
            else
            {
                status = ptx_NDEF_T2T_LockDLA(ndefT2T);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T2T_Close (ptx_NDEF_T2T_t *ndefT2T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;



    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        status = ptx_NativeTag_T2T_Close(&ndefT2T->t2t);
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK(s)
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NDEF_T2T_GetCCInfo (ptx_NDEF_T2T_t *ndefT2T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t rx_len;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        /* reset current CC-parameters */
        (void)memset(&ndefT2T->CCParams, 0, sizeof(ptx_NDEF_T2T_CC_t));
        (void)memset(&ndefT2T->SectorParams, 0, sizeof(ptx_NDEF_T2T_Sector_t));

        /* read CC-block and 3 following */
        status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, PTX_T2T_CC_BLOCK_NUMBER, &ndefT2T->RxBuffer[0], &rx_len);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        (void)memcpy(&ndefT2T->WorkBuffer[0], &ndefT2T->RxBuffer[0], PTX_T2T_BLOCKSIZE);

        ndefT2T->CCParams.MagicNumber = ndefT2T->WorkBuffer[PTX_T2T_CC_OFFSET_MAGIC_NUMBER];
        ndefT2T->CCParams.Version = ndefT2T->WorkBuffer[PTX_T2T_CC_OFFSET_VERSION_INFO];
        ndefT2T->CCParams.MLEN = ndefT2T->WorkBuffer[PTX_T2T_CC_OFFSET_TAG_SIZE];
        ndefT2T->CCParams.Access = ndefT2T->WorkBuffer[PTX_T2T_CC_OFFSET_ACCESS_COND];

        ndefT2T->CCParams.ReadAccess = (ndefT2T->CCParams.Access >> PTX_T2T_MASK_4);
        ndefT2T->CCParams.WriteAccess = (uint8_t)((PTX_T2T_CC_ACCESS_BITS != (ndefT2T->CCParams.Access & PTX_T2T_MASK_15)) ? (PTX_T2T_ACCESS_GRANTED) : (PTX_T2T_ACCESS_DENIED));

        ndefT2T->CCParams.VersionMajor = (ndefT2T->CCParams.Version >> PTX_T2T_MASK_4);
        ndefT2T->CCParams.VersionMinor = (ndefT2T->CCParams.Version & PTX_T2T_MASK_15);

        ndefT2T->CCParams.Size = (uint16_t)((uint16_t)ndefT2T->CCParams.MLEN * PTX_T2T_CC_MLEN_FACTOR); /* MLEN in Bytes */
        ndefT2T->CCParams.NumberOfBlocks = (uint16_t)((uint16_t)ndefT2T->CCParams.Size / (uint16_t)PTX_T2T_BLOCKSIZE);

        ndefT2T->SectorParams.NumberOfSectors = (uint8_t)((ndefT2T->CCParams.NumberOfBlocks / PTX_T2T_SECTORSIZE) + 1);
        if (0 == (ndefT2T->CCParams.NumberOfBlocks % PTX_T2T_SECTORSIZE))
        {
            ndefT2T->SectorParams.NumberOfSectors--;
        }
    }
    else
    {
        /* nothing to do, return error status */
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_HandleCCInfo(ptx_NDEF_T2T_t *ndefT2T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        switch (ndefT2T->CCParams.MagicNumber)
        {
            case PTX_T2T_CC_MAGIC_NUMBER:
                ndefT2T->LifeCycle = TagLC_Initialised;
                status = PTX_STATUS_SUCCESS;
                break;
            default:
                ndefT2T->LifeCycle = TagLC_NoNDEFTag;
                status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
                break;
        }

        /*
         * Minor version of device is 0, so check is unnecessary.
         * In case of change to device minor version, the following check should be added to if:
         * || PTX_T2T_VALID_MINOR_VERSION > ndefT2T->CCParams.VersionMinor
         */
        if ((PTX_T2T_VALID_MAJOR_VERSION != ndefT2T->CCParams.VersionMajor) && (PTX_STATUS_SUCCESS == status))
        {
            ndefT2T->LifeCycle = TagLC_NoNDEFTag;
            status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (ndefT2T->CCParams.ReadAccess)
            {
                case PTX_T2T_ACCESS_GRANTED:
                    break;

                default:
                    status = PTX_STATUS_E_NDEF_TAG_ACCESS_DENIED;
            }
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            switch (ndefT2T->CCParams.WriteAccess)
            {
                case PTX_T2T_ACCESS_GRANTED:
                    ndefT2T->LifeCycle = TagLC_ReadWrite;
                    break;
                case PTX_T2T_ACCESS_DENIED:
                    ndefT2T->LifeCycle = TagLC_ReadOnly;
                    break;

                default:
                    break;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_ReadBlocks (ptx_NDEF_T2T_t *ndefT2T, uint8_t blockNumber, uint8_t *rx, uint32_t *rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (NULL != rx) && (NULL != rxLen))
    {
        *rxLen = (uint32_t)ndefT2T->RxBufferSize;
        status = ptx_NativeTag_T2T_Read(&ndefT2T->t2t, blockNumber, rx, rxLen, PTX_T2T_DEFAULT_TIMEOUT_MS);

        if (PTX_STATUS_SUCCESS == status)
        {
            if ((0 != *rxLen) && ((PTX_T2T_BLOCKS_PER_READ * PTX_T2T_BLOCKSIZE) != *rxLen))
            {
                status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
            }
        }
        else
        {
            status = PTX_STATUS_E_NSC_RFERROR;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_WriteBlock(ptx_NDEF_T2T_t *ndefT2T,
                                            uint8_t blockNumber,
                                            uint8_t *blockData,
                                            uint8_t blockDataLen,
                                            uint8_t *rx,
                                            uint32_t *rxLen,
                                            uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (NULL != blockData) && (NULL != rx) && (NULL != rxLen))
    {
        *rxLen = ndefT2T->RxBufferSize;
        status = ptx_NativeTag_T2T_Write(&ndefT2T->t2t, blockNumber, blockData, blockDataLen, rx, rxLen, msTimeout);

        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 != *rxLen)
            {
                status = (PTX_T2T_ACK_CODE != rx[0]) ? (PTX_STATUS_E_NDEF_PROTOCOL_ERROR) : (status);
            }
            else
            {
                status = PTX_STATUS_E_NSC_RFERROR;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_SectorSelect(ptx_NDEF_T2T_t *ndefT2T,
                                              uint8_t secNr,
                                              uint8_t *rx,
                                              uint32_t *rxLen,
                                              uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (NULL != rx) && (NULL != rxLen) && (ndefT2T->SectorParams.NumberOfSectors >= (secNr + 1)))
    {
        *rxLen = ndefT2T->RxBufferSize;
        status = ptx_NativeTag_T2T_SectorSelect(&ndefT2T->t2t, secNr, rx, rxLen, msTimeout);

        if (PTX_STATUS_SUCCESS == status)
        {
            ndefT2T->SectorParams.CurrentSector = secNr;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_ReadNDEFTLVs (ptx_NDEF_T2T_t *ndefT2T, uint32_t msgBufferSize, uint8_t *msgBuffer, uint16_t *msgBufferOffset)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t current_sector = 0;
    uint16_t msg_length = 0;
    uint8_t length_field_len = 0;
    uint8_t length_field_offset = 0;
    uint8_t ndef_bytes_to_jump;
    uint16_t msg_buffer_offset = 0;
    uint32_t msg_buffer_size;
    uint16_t bytes_read = 0;
    uint8_t len_block_offset = 0;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (NULL != msgBuffer) && (NULL != msgBufferOffset))
    {
        msg_buffer_offset = *msgBufferOffset;
        msg_buffer_size = msgBufferSize;

        for (uint8_t idx = 0; idx < ndefT2T->TLVs.NumberOfNdefTLVs; idx++)
        {
            ndefT2T->CurrentByteAddress = ndefT2T->TLVs.NDEFTLV[idx].ByteAddress;
            ndefT2T->CurrentBlockAddress = (ndefT2T->CurrentByteAddress / PTX_T2T_BLOCKSIZE);
            ndefT2T->CurrentBlock = (uint8_t)(ndefT2T->CurrentBlockAddress % PTX_T2T_SECTORSIZE);

            /* check sector */
            if (ndefT2T->CurrentBlock / PTX_T2T_SECTORSIZE != ndefT2T->SectorParams.CurrentSector)
            {
                current_sector = (uint8_t)(ndefT2T->CurrentBlockAddress / PTX_T2T_SECTORSIZE);
                status = ptx_NDEF_T2T_SectorSelect(ndefT2T, current_sector, &ndefT2T->RxBuffer[0], &ndefT2T->RxLen, PTX_T2T_PAT_MS);
            }
            else
            {
                status = PTX_STATUS_SUCCESS;
            }

            /* If the last thing we did was not a CheckMsg, we need to read the L field again. */
            if ((PTX_STATUS_SUCCESS == status) && (!ndefT2T->LastOperationCheck))
            {
                status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->RxBuffer[0], &ndefT2T->RxLen);

                if (PTX_STATUS_SUCCESS == status)
                {
                    length_field_offset = (uint8_t)((ndefT2T->CurrentByteAddress + 1u) % PTX_T2T_BLOCKSIZE);
                    length_field_len = (PTX_T2T_LENGTHTHRESHOLD != ndefT2T->RxBuffer[length_field_offset]) ? (PTX_T2T_NDEF_L_FIELD_SHORT) : (PTX_T2T_NDEF_L_FIELD_LONG);
                }

                ndefT2T->TLVs.NDEFTLV[idx].LengthByteOffset = length_field_offset;

                if (PTX_T2T_NDEF_L_FIELD_SHORT == length_field_len)
                {
                    /* copy length block and get new length */
                    ndefT2T->TLVs.NDEFTLV[idx].Length = (uint16_t)ndefT2T->RxBuffer[length_field_offset];
                    memcpy(&ndefT2T->TLVs.NDEFTLV[idx].LengthBlock[0], &ndefT2T->RxBuffer[0], PTX_T2T_BLOCKSIZE);
                }
                else
                {
                    /* copy either one or two blocks of data and get new length */
                    ndefT2T->TLVs.NDEFTLV[idx].Length = (uint16_t)((((uint16_t)ndefT2T->RxBuffer[length_field_offset + 1] << PTX_T2T_MASK_8) | ndefT2T->RxBuffer[length_field_offset + 2]));
                    len_block_offset = (1u <= length_field_offset) ? (0x00) : (0x01);
                    memcpy(&ndefT2T->TLVs.NDEFTLV[idx].LengthBlock[0],
                           &ndefT2T->RxBuffer[PTX_T2T_BLOCKSIZE * len_block_offset],
                           (uint32_t)(PTX_T2T_BLOCKSIZE + (PTX_T2T_BLOCKSIZE * len_block_offset)));
                }
            }

            /* skip L part of TLV and go to first V-byte */
            if (PTX_STATUS_SUCCESS == status)
            {
                msg_length = ndefT2T->TLVs.NDEFTLV[idx].Length;
                if (PTX_T2T_LENGTHTHRESHOLD <= msg_length)
                {
                    ndef_bytes_to_jump = 1 + PTX_T2T_NDEF_L_FIELD_LONG;
                }
                else
                {
                    ndef_bytes_to_jump = 1 + PTX_T2T_NDEF_L_FIELD_SHORT;
                }

                ndefT2T->CurrentByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + ndef_bytes_to_jump); //position of first NDEF value byte
            }

            /* check if msgBuffer has enough space for the message */
            if (msg_buffer_size < msg_length)
            {
                status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_ReadLoop(ndefT2T, msg_length, msgBuffer, &bytes_read, &msg_buffer_offset);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                if (msg_length == bytes_read)
                {
                    msg_buffer_offset = (uint16_t)(msg_buffer_offset + msg_length);
                    *msgBufferOffset = msg_buffer_offset;
                }
                else
                {
                    status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
                }
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_ReadLoop(ptx_NDEF_T2T_t *ndefT2T, uint32_t msgLen, uint8_t *msgBuffer, uint16_t *bytesRead, uint16_t *msgBufferOffset)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t end_offset = 0;
    uint8_t current_sector = ndefT2T->SectorParams.CurrentSector;
    uint8_t previous_sector = current_sector;
    uint16_t bytes_read = 0;
    uint16_t bytes_to_read = 0;
    uint8_t block_offset = 0;
    uint16_t preserve_bytes[PTX_T2T_BLOCKS_PER_READ * PTX_T2T_BLOCKSIZE]; /* reset every operation, read max of 16 */
    uint16_t msg_buffer_offset = 0;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (NULL != msgBuffer) && (NULL != bytesRead) && (NULL != msgBufferOffset))
    {
        bytes_read = *bytesRead;
        msg_buffer_offset = *msgBufferOffset;

        status = PTX_STATUS_SUCCESS;

        while ((msgLen > bytes_read) && (PTX_STATUS_SUCCESS == status))
        {
            block_offset = ndefT2T->CurrentByteAddress % PTX_T2T_BLOCKSIZE;
            ndefT2T->CurrentBlockAddress = (ndefT2T->CurrentByteAddress / PTX_T2T_BLOCKSIZE);
            ndefT2T->CurrentBlock = (uint8_t)(ndefT2T->CurrentBlockAddress % PTX_T2T_SECTORSIZE);

            end_offset = 0;

            if (current_sector != (ndefT2T->CurrentBlockAddress + PTX_T2T_BLOCKS_PER_READ) / PTX_T2T_SECTORSIZE)
            {
                bytes_to_read = (uint16_t)(((PTX_T2T_BLOCKS_PER_READ * PTX_T2T_BLOCKSIZE) - (ndefT2T->CurrentByteAddress % (PTX_T2T_BLOCKS_PER_READ * PTX_T2T_BLOCKSIZE))) - block_offset);
            }
            else
            {
                bytes_to_read = (uint16_t)((PTX_T2T_BLOCKSIZE * PTX_T2T_BLOCKS_PER_READ) - block_offset);
            }

            if (previous_sector != (ndefT2T->CurrentBlockAddress / PTX_T2T_SECTORSIZE))
            {
                current_sector = (uint8_t)(ndefT2T->CurrentBlockAddress / PTX_T2T_SECTORSIZE);
                status = ptx_NDEF_T2T_SectorSelect(ndefT2T, current_sector, &ndefT2T->RxBuffer[0], &ndefT2T->RxLen, PTX_T2T_PAT_MS);
                previous_sector = current_sector;
            }

            /* Next read results in more bytes than needed */
            if (msgLen < (bytes_read + bytes_to_read))
            {
                bytes_to_read = (uint16_t)(msgLen - bytes_read);
                end_offset = (uint8_t)((PTX_T2T_BLOCKSIZE * PTX_T2T_BLOCKS_PER_READ) - block_offset - bytes_to_read);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_AreaChecker(ndefT2T, &bytes_to_read, &preserve_bytes[0]);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* If there are areas to be ignored nearby, read blocks one at a time. */
                    if (0 != preserve_bytes[0])
                    {
                        bytes_to_read = PTX_T2T_BLOCKSIZE;

                        /* Areas always start at block offset 0. */
                        if (ndefT2T->CurrentByteAddress != preserve_bytes[0])
                        {
                            bytes_to_read = PTX_T2T_BLOCKSIZE;
                        }
                        else
                        {
                            bytes_to_read--;
                            /* Look if the full block is reserved. */
                            for (uint8_t i = 1; i < PTX_T2T_BLOCKSIZE; i++)
                            {
                                if (0 != preserve_bytes[i])
                                {
                                    bytes_to_read--;
                                }
                            }
                        }

                        if (0 != bytes_to_read)
                        {
                            /* Read a single block. */
                            status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->RxBuffer[0], &ndefT2T->RxLen);
                        }

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            /* Skip the full block. */
                            ndefT2T->CurrentByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + PTX_T2T_BLOCKSIZE);
                            memcpy(&msgBuffer[msg_buffer_offset + bytes_read], &ndefT2T->RxBuffer[PTX_T2T_BLOCKSIZE - bytes_to_read], (uint32_t)bytes_to_read);
                        }
                    }
                    else
                    {
                        /* No special areas, proceed with full read. */
                        if (PTX_STATUS_SUCCESS == status)
                        {
                            /* Read Next 4 Blocks */
                            status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->RxBuffer[0], &ndefT2T->RxLen);
                        }

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            ndefT2T->CurrentByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + bytes_to_read - end_offset);
                            memcpy(&msgBuffer[msg_buffer_offset + bytes_read], &ndefT2T->RxBuffer[block_offset], (uint32_t)bytes_to_read);
                        }
                    }

                    if ((PTX_STATUS_SUCCESS == status) && (0 != bytes_to_read))
                    {
                        bytes_read = (uint16_t)(bytes_read + bytes_to_read);

                        if (msgLen < bytes_read)
                        {
                            bytes_read = (uint16_t)(bytes_read - end_offset);
                        }
                        else
                        {
                            block_offset = 0;
                            end_offset = 0;
                        }
                    }
                }
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            *bytesRead = bytes_read;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_OverwriteMessage (ptx_NDEF_T2T_t *ndefT2T, uint8_t *tx, uint32_t txLen, uint8_t ndefTLVIndex)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t ndef_tlv_header_size;
    uint8_t first_ndef_byte_offset = 0;

    uint16_t msg_len;
    uint8_t *msg_Buffer;
    uint32_t rx_len;

    uint8_t emptyMsg = 0;
    uint8_t EMPTY_NDEF_MSG_T2T[] = {0xD8, 0x00, 0x00, 0x00};

    uint8_t previous_sector = 0;
    uint8_t current_sector = 0;

    uint16_t bytes_written = 0;
    uint16_t bytes_to_write;
    uint16_t bytes_to_write_checker;

    uint16_t preserve_bytes[PTX_T2T_BLOCKS_PER_READ * PTX_T2T_BLOCKSIZE]; // reset every operation, write max of 4 but uses same preserve buffer
    uint8_t next_preserve = 0;

    uint8_t end_offset = 0;
    uint8_t terminator_offset = 0;

    uint8_t length_size = 0;
    uint8_t last_block[PTX_T2T_BLOCKSIZE] = {0x00, 0x00, 0x00, 0x00};
    uint16_t last_block_len = PTX_T2T_BLOCKSIZE;
    uint8_t last_write = 0;
    uint8_t last_block_write = 0;
    uint8_t last_block_tlv_terminator_write = 0;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        switch (ndefT2T->LifeCycle)
        {
            case TagLC_Initialised:
            /* FALLTHROUGH */
            case TagLC_ReadWrite:
                status = PTX_STATUS_SUCCESS;
                break;

            case TagLC_ReadOnly:
                status = PTX_STATUS_E_NDEF_TAG_ACCESS_DENIED;
                break;

            default:
                status = PTX_STATUS_E_NDEF_INVALID_TAG_STATE;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if ((0 != txLen) && (NULL != tx))
            {
                msg_Buffer = tx;
                msg_len = (uint16_t)txLen;
            }
            else
            {
                msg_Buffer = &EMPTY_NDEF_MSG_T2T[0];
                msg_len = sizeof(EMPTY_NDEF_MSG_T2T);
                emptyMsg = 1;
            }

            length_size = (msg_len <= ((uint32_t)(PTX_T2T_LENGTHTHRESHOLD - 1))) ? (uint8_t)PTX_T2T_NDEF_L_FIELD_SHORT : (uint8_t)PTX_T2T_NDEF_L_FIELD_LONG;
            ndef_tlv_header_size = (uint8_t)(length_size + 1);

            /* Need to make new TLV? */
            if (ndefT2T->TLVs.NumberOfNdefTLVs >= ndefTLVIndex + 1)
            {
                /* Will the data fit if we had ALL NDEF space for it? */
                if (txLen <= (uint32_t)ndefT2T->TLVs.AvailableNdefLength)
                {
                    status = ptx_NDEF_T2T_TLVProtection(ndefT2T, ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].ByteAddress, (uint16_t)txLen);
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* reset L-field */
                    status = ptx_NDEF_T2T_UpdateLength(ndefT2T, 0, length_size, ndefTLVIndex);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* set internal memory position tracking */
                        ndefT2T->CurrentByteAddress = ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].ByteAddress;
                        ndefT2T->CurrentByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + ndef_tlv_header_size);
                        ndefT2T->CurrentBlockAddress = (ndefT2T->CurrentByteAddress / PTX_T2T_BLOCKSIZE);
                        ndefT2T->CurrentBlock = (uint8_t)(ndefT2T->CurrentBlockAddress % PTX_T2T_SECTORSIZE);

                        /* check sector */
                        if (ndefT2T->CurrentBlock / PTX_T2T_SECTORSIZE != ndefT2T->CurrentBlockAddress / PTX_T2T_SECTORSIZE)
                        {
                            current_sector = (uint8_t)(ndefT2T->CurrentBlockAddress / PTX_T2T_SECTORSIZE);
                            status = ptx_NDEF_T2T_SectorSelect(ndefT2T, current_sector, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_PAT_MS);
                        }

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            previous_sector = current_sector;
                            first_ndef_byte_offset = ndefT2T->CurrentByteAddress % PTX_T2T_BLOCKSIZE;
                            bytes_to_write = (uint16_t)(PTX_T2T_BLOCKSIZE - first_ndef_byte_offset);
                            status = ptx_NDEF_T2T_AreaChecker(ndefT2T, &bytes_to_write, &preserve_bytes[0]);

                            /* preservation read needed? */
                            if ((PTX_STATUS_SUCCESS == status) &&
                                ((bytes_to_write != PTX_T2T_BLOCKSIZE - first_ndef_byte_offset) ||
                                 (0 != first_ndef_byte_offset)) &&
                                (0 != bytes_to_write))
                            {
                                status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->RxBuffer[0], &rx_len);
                                (void)memcpy(&ndefT2T->WorkBuffer[0], &ndefT2T->RxBuffer[0], (uint32_t)PTX_T2T_BLOCKSIZE);
                            }
                            (void)memcpy(&ndefT2T->WorkBuffer[first_ndef_byte_offset], &msg_Buffer[0], (uint32_t)bytes_to_write);
                            if (PTX_STATUS_SUCCESS == status)
                            {
                                /* Special case, manually preserve Data. */
                                (void)memcpy(&ndefT2T->WorkBuffer[0], &ndefT2T->RxBuffer[0], (uint32_t)(first_ndef_byte_offset));

                                for (uint8_t i = 0; i < (PTX_T2T_BLOCKSIZE - bytes_to_write - first_ndef_byte_offset); i++)
                                {
                                    next_preserve = (uint8_t)(preserve_bytes[i] - ndefT2T->CurrentByteAddress);

                                    if ((PTX_T2T_MAX_NDEF_BYTE_OFFSET) >= next_preserve)
                                    {
                                        ndefT2T->WorkBuffer[first_ndef_byte_offset + next_preserve] = ndefT2T->RxBuffer[first_ndef_byte_offset + next_preserve];
                                    }
                                }
                            }
                        }
                    }

                    if ((PTX_STATUS_SUCCESS == status) && (0 != bytes_to_write))
                    {
                        rx_len = ndefT2T->RxBufferSize;
                        status = ptx_NDEF_T2T_WriteBlock(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->WorkBuffer[0], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);
                    }
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* increment counters, reset preservation */
                        bytes_written = (uint16_t)(bytes_written + bytes_to_write);
                        (void)memset(&preserve_bytes[0], 0, PTX_T2T_BLOCKSIZE);
                        ndefT2T->CurrentByteAddress = (uint16_t)((ndefT2T->CurrentBlockAddress + 1) * 4);
                    }
                }
            }
            else
            {
                /* Try to make new TLV. */
                status = ptx_NDEF_T2T_TLVGenerator(ndefT2T, txLen, &msg_Buffer[0], &msg_len);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* New TLV was created, stored in msg_Buffer[], at index = 0 (including T and L values which will be written) */
                    memcpy(&ndefT2T->WorkBuffer[0], &msg_Buffer[0], (size_t)(msg_len));
                }
            }

            /* write loop until all data is written */
            while ((PTX_STATUS_SUCCESS == status) && ((msg_len > bytes_written) || (last_block_tlv_terminator_write)))
            {
                bytes_to_write = PTX_T2T_BLOCKSIZE;
                bytes_to_write_checker = bytes_to_write;
                if (msg_len < (bytes_written + PTX_T2T_BLOCKSIZE))
                {
                    bytes_to_write = (uint16_t)(msg_len - bytes_written);
                    bytes_to_write_checker = bytes_to_write;
                    end_offset = (uint8_t)(PTX_T2T_BLOCKSIZE - bytes_to_write);
                }

                /* have we reached the last data block? Could we add the terminator TLV? */
                if ((msg_len == (bytes_written + bytes_to_write) &&
                     (ndefT2T->TLVs.NumberOfNdefTLVs == ndefTLVIndex + 1) &&
                     (ndefT2T->TLVs.NumberOfMemoryTLVs == 0) &&
                     (ndefT2T->TLVs.NumberOfLockTLVs == 0) &&
                     (ndefT2T->CCParams.Size >= msg_len + 1 + length_size + PTX_T2T_TERMINATOR_TLV_LENGTH)) &&
                     (!last_block_tlv_terminator_write))
                {
                    /* last block, add terminator */
                    if (bytes_to_write != PTX_T2T_BLOCKSIZE)
                    {
                        /* Last block (1, 2 or 3 bytes of data + PTX_T2TOP_TERMINATOR_TLV_T) */
                        terminator_offset = (uint8_t)(bytes_to_write % PTX_T2T_BLOCKSIZE);
                        memcpy(&last_block[0], &msg_Buffer[bytes_written], bytes_to_write);
                        last_block[terminator_offset] = PTX_T2T_TERMINATOR_TLV_T;
                        bytes_to_write++;
                        bytes_to_write_checker++;
                        last_write = 1;
                    }
                    else
                    {
                        /* Last block (4bytes of data) -> no room for PTX_T2TOP_TERMINATOR_TLV_T */
                        last_block_write = 1;
                    }
                }

                ndefT2T->CurrentBlockAddress = (uint16_t)(ndefT2T->CurrentByteAddress / PTX_T2T_BLOCKSIZE);
                ndefT2T->CurrentBlock = (uint8_t)(ndefT2T->CurrentBlockAddress % PTX_T2T_SECTORSIZE);

                if (previous_sector != (ndefT2T->CurrentBlockAddress / PTX_T2T_SECTORSIZE))
                {
                    current_sector = (uint8_t)(ndefT2T->CurrentBlockAddress / PTX_T2T_SECTORSIZE);
                    status = ptx_NDEF_T2T_SectorSelect(ndefT2T, current_sector, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_PAT_MS);
                    previous_sector = current_sector;
                }

                if ((PTX_STATUS_SUCCESS == status) && (!last_block_tlv_terminator_write))
                {
                    status = ptx_NDEF_T2T_AreaChecker(ndefT2T, &bytes_to_write, &preserve_bytes[0]);
	                /* if we have to skip bytes in the middle, we need to read them first */
	                if ((bytes_to_write_checker != bytes_to_write) && (PTX_STATUS_SUCCESS == status) && (0 != bytes_to_write))
	                {
	                    rx_len = ndefT2T->RxBufferSize;
	                    status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->RxBuffer[0], &rx_len);

	                    if (PTX_STATUS_SUCCESS == status)
	                    {
	                        if (PTX_T2T_BLOCKSIZE <= ndefT2T->WorkBufferSize)
	                        {
	                            (void)memcpy(&ndefT2T->WorkBuffer[0], &msg_Buffer[bytes_written], PTX_T2T_BLOCKSIZE);
	                            status = ptx_NDEF_T2T_DataPreservation(ndefT2T, bytes_to_write, &bytes_written, end_offset, &preserve_bytes[0]);
	                        }
	                        else
	                        {
	                            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
	                        }
	                    }
	                }
	                else
	                {
	                    /* uninterrupted write */
	                    if ((PTX_STATUS_SUCCESS == status) && (0 != bytes_to_write))
	                    {
	                        if (bytes_to_write <= ndefT2T->WorkBufferSize)
	                        {
	                            if (last_write)
	                            {
	                                    /* Last write -> a block which includes data + PTX_T2TOP_TERMINATOR_TLV_T */
	                                (void)memcpy(&ndefT2T->WorkBuffer[0], &last_block[0], bytes_to_write);
	                                rx_len = ndefT2T->RxBufferSize;
	                            }
	                            else
	                            {
	                                (void)memcpy(&ndefT2T->WorkBuffer[0], &msg_Buffer[bytes_written], bytes_to_write);
	                                rx_len = ndefT2T->RxBufferSize;
	                            }
	                        }
	                        else
	                        {
	                            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
	                        }
	                    }
	                }

	                if (0 != bytes_to_write)
	                {
	                    /* write data, with possible preservation from above */
	                    status = ptx_NDEF_T2T_WriteBlock(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->WorkBuffer[0], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);
	                }

	                if (PTX_STATUS_SUCCESS == status)
	                {
	                    if (0 != bytes_to_write)
	                    {
	                        bytes_written = (uint16_t)(bytes_written + bytes_to_write);
	                        ndefT2T->CurrentByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + (bytes_to_write - end_offset));
	                    }
	                    else
	                    {
	                        ndefT2T->CurrentByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + PTX_T2T_BLOCKSIZE);
	                    }
	                }

	                if (last_block_write)
	                {
	                    /* We wrote last block (4bytes of data), set this flag to perform one more loop (to handle the correct sectors) */
	                    last_block_tlv_terminator_write = 1;
	                }
                }
                else
                {
                    /* Write extra block just for the terminator */
                    if (PTX_STATUS_SUCCESS == status)
                	{
                    	status = ptx_NDEF_T2T_AreaChecker(ndefT2T, &last_block_len, &preserve_bytes[0]);

                    	/* if we cannot write the last block fully without collision, we won't (terminator is optional) */
                    	if ((PTX_T2T_BLOCK_SIZE == last_block_len) && (PTX_STATUS_SUCCESS == status))
                        {

                            /* Try to fetch the contents of the last block */
                            status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->RxBuffer[0], &rx_len);

                            if (PTX_STATUS_SUCCESS == status)
                            {
                                (void)memcpy(&ndefT2T->WorkBuffer[0],&ndefT2T->RxBuffer[0],PTX_T2T_BLOCK_SIZE);
                                /* Change only the 1st byte, leaving the rest of the block unchanged */
                                ndefT2T->WorkBuffer[0] = PTX_T2T_TERMINATOR_TLV_T;
                                status = ptx_NDEF_T2T_WriteBlock(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->WorkBuffer[0], PTX_T2T_BLOCK_SIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);
                            }
                        }

                        last_block_tlv_terminator_write = 0;
                    }
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /*
                 * update internal length block with space allocated for length bytes
                 * and NDEF-message bytes already set
                 */
                if (PTX_T2T_NDEF_L_FIELD_SHORT == length_size)
                {
                    ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthByteOffset] = 0x00;
                    memcpy(&ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthByteOffset + 1u],
                           &msg_Buffer[0],
                           (uint32_t)(PTX_T2T_BLOCKSIZE - (ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthByteOffset + 1u)));
                }
                else
                {
                    memset(&ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthByteOffset],
                           0x00,
                           PTX_T2T_NDEF_L_FIELD_LONG);

                    memcpy(&ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthByteOffset + 1u],
                           &msg_Buffer[0],
                           (uint32_t)(PTX_T2T_BLOCKSIZE - (ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthByteOffset + 1u)));
                }
                status = ptx_NDEF_T2T_UpdateLength(ndefT2T, msg_len, length_size, ndefTLVIndex);
            }
        }

        if ((ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].Length == bytes_written) && (PTX_STATUS_SUCCESS == status))
        {
            if (0 == emptyMsg)
            {
                ndefT2T->LifeCycle = TagLC_ReadWrite;
            }
            else
            {
                ndefT2T->LifeCycle = TagLC_Initialised;
                status = ptx_NDEF_T2T_UpdateLength(ndefT2T, 0, length_size, ndefTLVIndex);
            }
        }
    }

    return status;
}


/* look for TLVs after CheckMessage has read the memory */
static ptx_Status_t ptx_NDEF_T2T_TLVAreaCrawler (ptx_NDEF_T2T_t *ndefT2T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        /* Reset position to beginning of TLV area. */
        ndefT2T->CurrentByteAddress = (uint16_t)(PTX_T2T_TLV_AREA_BEGIN_BLOCK * PTX_T2T_BLOCKSIZE);
        ndefT2T->CurrentBlock = PTX_T2T_TLV_AREA_BEGIN_BLOCK;
        ndefT2T->CurrentBlockAddress = PTX_T2T_TLV_AREA_BEGIN_BLOCK;
        ndefT2T->SectorParams.CurrentSector = 0u;
        uint8_t current_sector = ndefT2T->SectorParams.CurrentSector;
        uint16_t nbr_mem_bytes = ndefT2T->CCParams.Size;

        uint8_t nbr_dyn_lock_bit = 0;
        uint8_t nbr_dyn_lock_byte = 0;

        uint16_t bytes_to_skip = 0;

        status = PTX_STATUS_SUCCESS;

        /* Sweep the TLV Memory area. */
        while ((nbr_mem_bytes >= ndefT2T->CurrentByteAddress) && (PTX_T2T_MAX_NBR_NDEF_TLV > ndefT2T->TLVs.NumberOfNdefTLVs) && (PTX_T2T_NO_TERMINATOR_TLV == ndefT2T->TLVs.TerminatorTLV.TerminatorTLVFound) && (PTX_STATUS_SUCCESS == status))
        {
            if (((PTX_T2T_SECTORTHRESHOLD * (ndefT2T->SectorParams.CurrentSector + 1)) < ndefT2T->CurrentBlockAddress) && (ndefT2T->SectorParams.NumberOfSectors >= (current_sector + 1)))
            {
                status = ptx_NDEF_T2T_SectorSelect(ndefT2T, (uint8_t)(ndefT2T->SectorParams.CurrentSector + 1), &ndefT2T->RxBuffer[0], &ndefT2T->RxLen, PTX_T2T_PAT_MS);
            }
            else
            {
                /* No more sectors. */
                status = PTX_STATUS_SUCCESS;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT2T->CurrentBlock = (uint8_t)(ndefT2T->CurrentBlockAddress % PTX_T2T_SECTORSIZE);
                status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, (uint8_t)ndefT2T->CurrentBlock, &ndefT2T->RxBuffer[0], &ndefT2T->RxLen);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_IdentifyTLV(ndefT2T, &bytes_to_skip);
            }

            bytes_to_skip = (PTX_T2T_BLOCKSIZE * PTX_T2T_BLOCKS_PER_READ > bytes_to_skip) ? (PTX_T2T_BLOCKSIZE * PTX_T2T_BLOCKS_PER_READ) : (bytes_to_skip);

            ndefT2T->CurrentByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + bytes_to_skip);
            ndefT2T->CurrentBlockAddress = ndefT2T->CurrentByteAddress / PTX_T2T_BLOCKSIZE;
        }

        if ((PTX_T2T_MIN_SIZE_FOR_DLA < ndefT2T->CCParams.Size) && (0 == ndefT2T->TLVs.NumberOfLockTLVs))
        {
            nbr_dyn_lock_bit = (uint8_t)((ndefT2T->CCParams.Size - ((PTX_T2T_TLV_AREA_BEGIN_BLOCK * PTX_T2T_BLOCKSIZE) * 8u)) / 8u);
            nbr_dyn_lock_bit = (0 != ((ndefT2T->CCParams.Size - ((PTX_T2T_TLV_AREA_BEGIN_BLOCK * PTX_T2T_BLOCKSIZE) * 8u)) % 8u)) ? ((uint8_t)(nbr_dyn_lock_bit + 1)) : (nbr_dyn_lock_bit);
            nbr_dyn_lock_byte = nbr_dyn_lock_bit / 8u;
            nbr_dyn_lock_byte = (0 != (nbr_dyn_lock_bit % 8u)) ? ((uint8_t)(nbr_dyn_lock_byte + 1)) : (nbr_dyn_lock_byte);

            if (ndefT2T->SectorParams.NumberOfSectors < ((ndefT2T->CCParams.Size + (PTX_T2T_TLV_AREA_BEGIN_BLOCK * PTX_T2T_BLOCKSIZE) + nbr_dyn_lock_byte) / PTX_T2T_SECTORSIZE) + 1)
            {
                ndefT2T->SectorParams.NumberOfSectors++;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_IdentifyTLV(ptx_NDEF_T2T_t *ndefT2T, uint16_t *bytesToSkip)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t bytes_to_skip = 0;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (NULL != bytesToSkip))
    {
        memcpy(&ndefT2T->WorkBuffer[0], &ndefT2T->RxBuffer[0], ndefT2T->RxLen);
        for (uint32_t idx = 0; idx < ndefT2T->RxLen; idx++)
        {
            switch (ndefT2T->WorkBuffer[idx])
            {
                /* Remember Lock- and Memory-TLV positions to handle them in the processor functions. */
                case PTX_T2T_LOCK_CONTROL_TLV_T:
                    /* if L != PTX_T2T_LOCK_CONTROL_TLV_LENGTH skip, not a TLV. */
                    if (PTX_T2T_LOCK_CONTROL_TLV_LENGTH == ndefT2T->WorkBuffer[idx + 1])
                    {
                        if (PTX_T2T_MAX_NUMBER_LOCK_CONTROL > ndefT2T->TLVs.NumberOfLockTLVs)
                        {
                            ndefT2T->TLVs.LockControlTLVs[ndefT2T->TLVs.NumberOfLockTLVs].ByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + idx);
                            ndefT2T->TLVs.NumberOfLockTLVs++;
                            status = PTX_STATUS_SUCCESS;
                        }
                        else
                        {
                            status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                        }
                        /* step over TLV, T+L+L*ValueBytes. */
                        idx = (uint32_t)(idx + (PTX_T2T_LOCK_CONTROL_TLV_LENGTH + 1));
                        bytes_to_skip = (uint16_t)(bytes_to_skip + (PTX_T2T_LOCK_CONTROL_TLV_LENGTH + 2));
                    }
                    break;

                case PTX_T2T_MEMORY_CONTROL_TLV_T:
                    /* if L != PTX_T2T_MEMORY_CONTROL_TLV_LENGTH skip, not a TLV. */
                    if (PTX_T2T_MEMORY_CONTROL_TLV_LENGTH == ndefT2T->WorkBuffer[idx + 1])
                    {
                        if (PTX_T2T_MAX_NUMBER_MEMORY_CONTROL > ndefT2T->TLVs.NumberOfMemoryTLVs)
                        {
                            ndefT2T->TLVs.MemoryControlTLVs[ndefT2T->TLVs.NumberOfMemoryTLVs].ByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + idx);
                            ndefT2T->TLVs.NumberOfMemoryTLVs++;
                            status = PTX_STATUS_SUCCESS;
                        }
                        else
                        {
                            status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                        }
                        /* step over TLV, T+L+L*ValueBytes. */
                        idx = (uint32_t)(idx + (PTX_T2T_MEMORY_CONTROL_TLV_LENGTH + 1));
                        bytes_to_skip = (uint16_t)(bytes_to_skip + (PTX_T2T_LOCK_CONTROL_TLV_LENGTH + 2));
                    }
                    break;

                case PTX_T2T_NDEF_TLV_T:
                    status = ptx_NDEF_T2T_NDEFTLV_Handler(ndefT2T, &idx, &bytes_to_skip);
                    break;

                case PTX_T2T_TERMINATOR_TLV_T:
                    ndefT2T->TLVs.TerminatorTLV.ByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + idx);
                    ndefT2T->TLVs.TerminatorTLV.TerminatorTLVFound = PTX_T2T_TERMINATOR_TLV_PRESENT;
                    status = PTX_STATUS_SUCCESS;

                    /* exit loop */
                    idx = (uint32_t)ndefT2T->RxLen;
                    break;

                case PTX_T2T_NULL_TLV_T:
                default:
                    break;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            *bytesToSkip = bytes_to_skip;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_NDEFTLV_Handler(ptx_NDEF_T2T_t *ndefT2T, uint32_t *offset, uint16_t *bytesToSkip)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t length_byte_combined;
    uint16_t bytes_to_skip = 0;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (offset != NULL) && (NULL != bytesToSkip))
    {
        uint32_t idx = *offset;
        uint32_t len_block_idx = 0;
        uint32_t tlv_idx = 0;

        if (PTX_T2T_MAX_NUMBER_NDEFTLVS > ndefT2T->TLVs.NumberOfNdefTLVs)
        {
            status = PTX_STATUS_SUCCESS;
            ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].ByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + idx);
            tlv_idx = idx;

            /* cache offset within block containing length byte(s), which is directly after T-byte */
            ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].LengthByteOffset = (uint8_t)((ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].ByteAddress + 1u) % PTX_T2T_BLOCK_SIZE);

            /* check if the length byte/s is/are out of bounds of current work buffer content */
            if ((PTX_T2T_BLOCKSIZE * PTX_T2T_BLOCKS_PER_READ - PTX_T2T_BLOCKSIZE) < idx)
            {
                /* length byte(s) out of bounds, check sector and add another block */
                if (((ndefT2T->CurrentBlock + PTX_T2T_BLOCKS_PER_READ) / PTX_T2T_SECTORSIZE) != (ndefT2T->CurrentBlock / PTX_T2T_SECTORSIZE))
                {
                    status = ptx_NDEF_T2T_SectorSelect(ndefT2T, (uint8_t)((ndefT2T->CurrentBlock + (PTX_T2T_BLOCKS_PER_READ - 1)) / PTX_T2T_SECTORSIZE), &ndefT2T->RxBuffer[0], &ndefT2T->RxLen, PTX_T2T_PAT_MS);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, (uint8_t)(ndefT2T->CurrentBlock + PTX_T2T_BLOCKS_PER_READ), &ndefT2T->RxBuffer[0], &ndefT2T->RxLen);
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* add the fifth block to the work buffer */
                        (void)memcpy(&ndefT2T->WorkBuffer[PTX_T2T_BLOCKSIZE * PTX_T2T_BLOCKS_PER_READ], &ndefT2T->RxBuffer[0], (uint32_t)(PTX_T2T_BLOCKSIZE));
                    }
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* proceed with length byte(s) */
                idx++;
                /* cache 2 blocks that could both contain parts of the TLV length */
                len_block_idx = tlv_idx - (tlv_idx % PTX_T2T_BLOCKSIZE);
                memcpy(&ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].LengthBlock[0], &ndefT2T->WorkBuffer[len_block_idx], PTX_T2T_BLOCKSIZE * 2u);

                if (PTX_T2T_LENGTHTHRESHOLD == ndefT2T->WorkBuffer[idx])
                {
                    /* 3 byte L */
                    length_byte_combined = (uint16_t)(((uint16_t)ndefT2T->WorkBuffer[idx + 1] << PTX_T2T_MASK_8) | ndefT2T->WorkBuffer[idx + 2]);
                    ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].Length = length_byte_combined;
                    idx = (uint32_t)(idx + 2); /* Skip leftover L bytes. */
                    bytes_to_skip = (uint16_t)(bytes_to_skip + 3); /* Skip all L bytes. */

                    /* are the length bytes in the first, second, or both cached blocks? */
                    if (1 < len_block_idx)
                    {
                        /* first or both */
                    }
                    else
                    {
                        /* second, push it into first */
                        memcpy(&ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].LengthBlock[0], &ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].LengthBlock[PTX_T2T_BLOCKSIZE], PTX_T2T_BLOCKSIZE);
                    }
                }
                else
                {
                    /* 1 byte L */
                    ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].Length = ndefT2T->WorkBuffer[idx];
                    bytes_to_skip++; /* Skip L byte. */
                }

                /* Skip data bytes of this TLV. */
                bytes_to_skip = (uint16_t)(bytes_to_skip + ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].Length);
                idx = idx + ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].Length;

                ndefT2T->TLVs.NumberOfNdefTLVs++;
            }
        }
        else
        {
            status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
        }

        *offset = idx;
        *bytesToSkip = bytes_to_skip;
    }

    return status;
}

/* update L of NDEF TLV */
static ptx_Status_t ptx_NDEF_T2T_UpdateLength(ptx_NDEF_T2T_t *ndefT2T, uint16_t msgLen, uint8_t lengthSize, uint8_t ndefTLVIndex)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t rx_len;
    uint16_t length_byte_address;
    uint16_t block_address_ndef_len;
    uint8_t length_byte_sector = 0;
    uint8_t block_number_ndef_len;
    uint16_t new_len = msgLen;
    uint8_t lengthBytes[3];
    uint8_t three_len_prefix = 0x00;


    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        switch (ndefT2T->LifeCycle)
        {
            case TagLC_Initialised:
            /* FALLTHROUGH */
            case TagLC_ReadWrite:
                status = PTX_STATUS_SUCCESS;
                break;

            case TagLC_ReadOnly:
                status = PTX_STATUS_E_NDEF_TAG_ACCESS_DENIED;
                break;

            default:
                status = PTX_STATUS_E_NDEF_INVALID_TAG_STATE;
                break;
        }


        /* L is 1 byte */
        if ((PTX_STATUS_SUCCESS == status) && (PTX_T2T_NDEF_L_FIELD_SHORT == lengthSize))
        {
            length_byte_address = (uint16_t)(ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].ByteAddress + PTX_T2T_NDEF_L_FIELD_SHORT);

            block_address_ndef_len = (length_byte_address / PTX_T2T_BLOCKSIZE);
            block_number_ndef_len = (uint8_t)(block_address_ndef_len % PTX_T2T_SECTORSIZE);
            length_byte_sector = (uint8_t)(block_address_ndef_len / PTX_T2T_SECTORSIZE);
            if (ndefT2T->SectorParams.CurrentSector != length_byte_sector)
            {
                status = ptx_NDEF_T2T_SectorSelect(ndefT2T, length_byte_sector, &ndefT2T->RxBuffer[0], &ndefT2T->RxLen, PTX_T2T_PAT_MS);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[(length_byte_address % PTX_T2T_BLOCKSIZE)] = (uint8_t)new_len;
                (void)memcpy(&ndefT2T->WorkBuffer[0], &ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[0], PTX_T2T_BLOCKSIZE);

                rx_len = ndefT2T->RxBufferSize;
                status = ptx_NDEF_T2T_WriteBlock(ndefT2T, block_number_ndef_len, &ndefT2T->WorkBuffer[0], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].Length = new_len;

                if (0 == new_len)
                {
                    ndefT2T->LifeCycle = TagLC_Initialised;
                }
                else
                {
                    ndefT2T->LifeCycle = TagLC_ReadWrite;
                }
            }
        }

        /* L is 3 bytes */
        if ((PTX_STATUS_SUCCESS == status) && (PTX_T2T_NDEF_L_FIELD_LONG == lengthSize))
        {
            length_byte_address = (uint16_t)(ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].ByteAddress + 1);
            block_address_ndef_len = (length_byte_address / PTX_T2T_BLOCKSIZE);
            block_number_ndef_len = (uint8_t)(block_address_ndef_len % PTX_T2T_SECTORSIZE);
            length_byte_sector = (uint8_t)(block_address_ndef_len / PTX_T2T_SECTORSIZE);

            three_len_prefix = (new_len != 0) ? (PTX_T2T_LENGTHTHRESHOLD) : (0x00);
            lengthBytes[0] = three_len_prefix;
            lengthBytes[1] = (uint8_t)((new_len & 0xFF00) >> PTX_T2T_MASK_8);
            lengthBytes[2] = (uint8_t)(new_len & 0x00FF);


            if (ndefT2T->SectorParams.CurrentSector != length_byte_sector)
            {
                status = ptx_NDEF_T2T_SectorSelect(ndefT2T, length_byte_sector, &ndefT2T->RxBuffer[0], &ndefT2T->RxLen, PTX_T2T_PAT_MS);
            }

            if ((PTX_STATUS_SUCCESS == status) && (PTX_T2T_BLOCKSIZE / 2) > (length_byte_address % PTX_T2T_BLOCKSIZE))
            {
                (void)memcpy(&ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[(length_byte_address % PTX_T2T_BLOCKSIZE)], &lengthBytes[0], sizeof(lengthBytes));
                rx_len = ndefT2T->RxBufferSize;
                status = ptx_NDEF_T2T_WriteBlock(ndefT2T, block_number_ndef_len, &ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[0], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);
            }
            else
            {
                if (PTX_STATUS_SUCCESS == status)
                {
                    (void)memcpy(&ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[(length_byte_address % PTX_T2T_BLOCKSIZE)], &lengthBytes[0], (uint32_t)((uint32_t)sizeof(lengthBytes) + 1 - (uint32_t)(PTX_T2T_BLOCKSIZE - (length_byte_address % PTX_T2T_BLOCKSIZE))));
                    rx_len = ndefT2T->RxBufferSize;
                    status = ptx_NDEF_T2T_WriteBlock(ndefT2T, block_number_ndef_len, &ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[0], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);

                    /* check sector length */
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        if (PTX_T2T_SECTORTHRESHOLD > block_number_ndef_len)
                        {
                            /* second write is in new sector */
                            status = ptx_NDEF_T2T_SectorSelect(ndefT2T, (uint8_t)(ndefT2T->SectorParams.CurrentSector + 1), &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_PAT_MS);
                            if (PTX_STATUS_SUCCESS == status)
                            {
                                /* write to block 0 of next sector */
                                status = ptx_NDEF_T2T_WriteBlock(ndefT2T, 0, &ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[PTX_T2T_BLOCKSIZE], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);
                            }
                        }
                        else
                        {
                            /* second write is in same sector */
                            status = ptx_NDEF_T2T_WriteBlock(ndefT2T, (uint8_t)(block_number_ndef_len + 1), &ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].LengthBlock[PTX_T2T_BLOCKSIZE], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);
                        }
                    }
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT2T->TLVs.NDEFTLV[ndefTLVIndex].Length = new_len;
                ndefT2T->LifeCycle = TagLC_ReadWrite;
            }
        }
    }

    return status;
}

/* looks through LockControlTLVs and saves the DLAs */
static ptx_Status_t ptx_NDEF_T2T_DLAProcessor (ptx_NDEF_T2T_t *ndefT2T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t lock_byte_address;

    uint8_t DLAPosition;
    /* contains: */
    uint8_t NbrMajorOffsets;
    uint8_t NbrMinorOffsets;

    uint16_t DLA_NbrLockBits;
    uint16_t DLA_NbrBytes;

    uint8_t DLAControl;
    /* contains: */
    uint8_t BLPLB_Index;
    uint8_t MOS_DLA;

    uint16_t DLA_FirstByteAddress;
    uint16_t MajorOffset_Size_DLA;


    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        status = PTX_STATUS_SUCCESS;
        for (uint8_t LockTLV = 0; LockTLV < ndefT2T->TLVs.NumberOfLockTLVs; LockTLV++)
        {
            lock_byte_address = ndefT2T->TLVs.LockControlTLVs[LockTLV].ByteAddress;

            if (ndefT2T->SectorParams.CurrentSector != lock_byte_address / PTX_T2T_SECTORSIZE)
            {
                status = ptx_NDEF_T2T_SectorSelect(ndefT2T, (uint8_t)(lock_byte_address / PTX_T2T_SECTORSIZE), &ndefT2T->RxBuffer[0], &ndefT2T->RxLen, PTX_T2T_PAT_MS);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, (uint8_t)(lock_byte_address / PTX_T2T_BLOCKSIZE), &ndefT2T->RxBuffer[0], &ndefT2T->RxLen);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                (void)memcpy(&ndefT2T->WorkBuffer[0], &ndefT2T->RxBuffer[0], PTX_T2T_BLOCKSIZE * 2);
                DLAPosition = ndefT2T->WorkBuffer[(lock_byte_address % PTX_T2T_BLOCKSIZE) + 2];
                DLA_NbrLockBits = (uint16_t)ndefT2T->WorkBuffer[(lock_byte_address % PTX_T2T_BLOCKSIZE) + 3];
                if (0 == DLA_NbrLockBits)
                {
                    DLA_NbrLockBits = PTX_T2T_SECTORSIZE;
                }
                DLAControl = ndefT2T->WorkBuffer[(lock_byte_address % PTX_T2T_BLOCKSIZE) + 4];

                NbrMajorOffsets = (DLAPosition >> PTX_T2T_MASK_4);
                NbrMinorOffsets = (DLAPosition & 0x0F);

                DLA_NbrBytes = (uint16_t)(DLA_NbrLockBits / 8);
                if (0 != (DLA_NbrLockBits % 8))
                {
                    DLA_NbrBytes++;
                }

                BLPLB_Index = (DLAControl >> PTX_T2T_MASK_4);
                MOS_DLA = (DLAControl & 0x0F);

                ndefT2T->TLVs.LockControlTLVs[LockTLV].BytesLockedPerLockBit = (uint8_t)(1 << BLPLB_Index);

                MajorOffset_Size_DLA = (uint16_t)(1 << MOS_DLA);
                DLA_FirstByteAddress = (uint16_t)((NbrMajorOffsets * MajorOffset_Size_DLA) + NbrMinorOffsets);

                ndefT2T->TLVs.LockControlTLVs[LockTLV].NbrReservedBits = (uint8_t)(ndefT2T->TLVs.LockControlTLVs[LockTLV].BytesLockedPerLockBit -
                                                                                   (DLA_NbrLockBits % ndefT2T->TLVs.LockControlTLVs[LockTLV].BytesLockedPerLockBit));
                ndefT2T->TLVs.LockControlTLVs[LockTLV].LockArea[0] = (uint16_t)DLA_FirstByteAddress;
                ndefT2T->TLVs.LockControlTLVs[LockTLV].LockArea[1] = (uint16_t)(DLA_FirstByteAddress + DLA_NbrBytes);
                ndefT2T->TLVs.LockControlTLVs[LockTLV].InternalArea[0] = ((uint16_t)(PTX_T2T_BLOCKSIZE - 1 != ((DLA_FirstByteAddress + DLA_NbrBytes) -1) % PTX_T2T_BLOCKSIZE)) ?
                                                                         ((uint16_t)(ndefT2T->TLVs.LockControlTLVs[LockTLV].LockArea[1] + 1)) :
                                                                         (0u);
                if (0 != ndefT2T->TLVs.LockControlTLVs[LockTLV].InternalArea[0])
                {
                    ndefT2T->TLVs.LockControlTLVs[LockTLV].InternalArea[1] = (uint16_t)(ndefT2T->TLVs.LockControlTLVs[LockTLV].InternalArea[0] +
                                                                                        (PTX_T2T_BLOCKSIZE - (ndefT2T->TLVs.LockControlTLVs[LockTLV].InternalArea[0] % PTX_T2T_BLOCKSIZE)));
                }
                else
                {
                    /* assign InternalArea End for easier checking. */
                    ndefT2T->TLVs.LockControlTLVs[LockTLV].InternalArea[1] = ndefT2T->TLVs.LockControlTLVs[LockTLV].LockArea[1];
                }
            }
        }
    }

    return status;
}

/* looks through LockControlTLVs and saves the reserved areas */
static ptx_Status_t ptx_NDEF_T2T_RAProcessor(ptx_NDEF_T2T_t *ndefT2T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t mem_byte_address;

    uint8_t RAPosition;
    /* contains: */
    uint8_t NbrMajorOffsets;
    uint8_t NbrMinorOffsets;

    uint16_t Reserved_Area_Size;

    uint8_t RAControl;
    /* contains: */
    uint8_t MOS_RA;

    uint16_t RA_FirstByteAddress;
    uint16_t MajorOffset_Size_RA;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        status = PTX_STATUS_SUCCESS;
        for (uint8_t MemoryTLV = 0; MemoryTLV < ndefT2T->TLVs.NumberOfMemoryTLVs; MemoryTLV++)
        {
            mem_byte_address = ndefT2T->TLVs.MemoryControlTLVs[MemoryTLV].ByteAddress;
            status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, (uint8_t)(mem_byte_address / PTX_T2T_BLOCKSIZE), &ndefT2T->RxBuffer[0], &ndefT2T->RxLen);
            if (PTX_STATUS_SUCCESS == status)
            {
                (void)memcpy(&ndefT2T->WorkBuffer[0], &ndefT2T->RxBuffer[0], PTX_T2T_BLOCKSIZE * 2);
                RAPosition = ndefT2T->WorkBuffer[(mem_byte_address % PTX_T2T_BLOCKSIZE) + 2];
                Reserved_Area_Size = (uint16_t)ndefT2T->WorkBuffer[(mem_byte_address % PTX_T2T_BLOCKSIZE) + 3];
                if (0 == Reserved_Area_Size)
                {
                    Reserved_Area_Size = PTX_T2T_SECTORSIZE;
                }
                RAControl = ndefT2T->WorkBuffer[(mem_byte_address % PTX_T2T_BLOCKSIZE) + PTX_T2T_BLOCKSIZE];

                NbrMajorOffsets = (RAPosition >> PTX_T2T_MASK_4);
                NbrMinorOffsets = (RAPosition & 0x0F);

                MOS_RA = (RAControl & 0x0F);

                MajorOffset_Size_RA = (uint16_t)(1 << MOS_RA);
                RA_FirstByteAddress = (uint16_t)((NbrMajorOffsets * MajorOffset_Size_RA) + NbrMinorOffsets);

                ndefT2T->TLVs.MemoryControlTLVs[MemoryTLV].RsvdArea[0] = (uint16_t)RA_FirstByteAddress;
                ndefT2T->TLVs.MemoryControlTLVs[MemoryTLV].RsvdArea[1] = (uint16_t)(RA_FirstByteAddress + Reserved_Area_Size);
            }
        }
    }

    return status;
}

/* check RAs and DLAs against current address to ignore, bytes to process is to be set before the operation to either T2T block length or T2T block length * 4 depending on operation read or write  */
static ptx_Status_t ptx_NDEF_T2T_AreaChecker (ptx_NDEF_T2T_t *ndefT2T, uint16_t *bytesToProcess, uint16_t *preserveBytes)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t bytes_to_process = *bytesToProcess;

    uint8_t preserve_counter = 0;

    uint16_t begin_byte = ndefT2T->CurrentByteAddress;
    uint16_t end_byte = (uint16_t)(begin_byte + bytes_to_process);

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (NULL != bytesToProcess) && (NULL != preserveBytes))
    {
        /* reset preserve bytes container */
        (void)memset(&preserveBytes[0], 0, (uint32_t)(sizeof(uint16_t) * (PTX_T2T_BLOCKSIZE * PTX_T2T_BLOCKS_PER_READ)));

        for (uint16_t check_address = begin_byte; check_address < end_byte; check_address++)
        {
            /* check DLAs */
            for (uint8_t idx = 0; idx < ndefT2T->TLVs.NumberOfLockTLVs; idx++)
            {
                if ((ndefT2T->TLVs.LockControlTLVs[idx].LockArea[0] <= check_address) && (ndefT2T->TLVs.LockControlTLVs[idx].InternalArea[1] > check_address))
                {
                    preserveBytes[preserve_counter] = check_address;
                    preserve_counter++;
                    bytes_to_process--;
                }
            }

            /* check RAs */
            for (uint8_t idx = 0; idx < ndefT2T->TLVs.NumberOfMemoryTLVs; idx++)
            {
                if ((ndefT2T->TLVs.MemoryControlTLVs[idx].RsvdArea[0] <= check_address) && (ndefT2T->TLVs.MemoryControlTLVs[idx].RsvdArea[1] > check_address))
                {
                    preserveBytes[preserve_counter] = check_address;
                    preserve_counter++;
                    bytes_to_process--;
                }
            }
        }
        *bytesToProcess = bytes_to_process;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_DataPreservation(ptx_NDEF_T2T_t *ndefT2T, uint16_t bytesToWrite, uint16_t *bytesWritten, uint8_t endOffset, uint16_t preserveAddresses[])
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t rx_len = 0;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (NULL != bytesWritten) && (NULL != preserveAddresses))
    {
        uint16_t bytes_written = *bytesWritten;
        uint8_t next_preserve = 0;

        memcpy(&ndefT2T->WorkBuffer[0], &ndefT2T->RxBuffer[0], (uint32_t)(PTX_T2T_BLOCKSIZE - bytesToWrite - endOffset));

        for (uint8_t i = 0; i < (PTX_T2T_BLOCKSIZE - bytesToWrite - endOffset); i++)
        {
            next_preserve = (uint8_t)(preserveAddresses[i] - ndefT2T->CurrentByteAddress);

            if (PTX_T2T_MAX_NDEF_BYTE_OFFSET >= next_preserve)
            {
                ndefT2T->WorkBuffer[next_preserve] = ndefT2T->RxBuffer[next_preserve];
            }
        }
        rx_len = ndefT2T->RxBufferSize;
        status = ptx_NDEF_T2T_WriteBlock(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->WorkBuffer[0], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);

        (void)memset(&preserveAddresses[0], 0, PTX_T2T_BLOCKSIZE);
        bytes_written = (uint16_t)(bytes_written + bytesToWrite);
        *bytesWritten = bytes_written;
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_TLVProtection (ptx_NDEF_T2T_t *ndefT2T, uint16_t overwriteByteAddress, uint16_t overwriteLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint16_t tlv_address = 0;
    uint16_t tlv_len = 0;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        status = PTX_STATUS_SUCCESS;
        for (uint8_t idx = 0; idx < ndefT2T->TLVs.NumberOfNdefTLVs; idx++)
        {
            tlv_address = ndefT2T->TLVs.NDEFTLV[idx].ByteAddress;
            tlv_len = ndefT2T->TLVs.NDEFTLV[idx].Length + 1; // Length of NDEF TLV + length information byte

            /* for NDEF TLVs, overwriting is an option */
            if (tlv_address != overwriteByteAddress)
            {
                /* if not the TLV to be overwritten, check if it would overlap with other NDEF TLVs. */
                if (((tlv_address <= overwriteByteAddress) && ((tlv_address + tlv_len) >= overwriteByteAddress))
                    || (tlv_address <= (overwriteByteAddress + overwriteLength)))
                {
                    status = PTX_STATUS_E_NDEF_TLV_OVERWRITE_ERROR;
                }
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            for (uint8_t idx = 0; idx < ndefT2T->TLVs.NumberOfLockTLVs; idx++)
            {
                tlv_address = ndefT2T->TLVs.LockControlTLVs[idx].ByteAddress;
                tlv_len = PTX_T2T_LOCK_CONTROL_TLV_LENGTH + 1; // Length of Lock Control TLV + length information byte length

                if (((tlv_address <= overwriteByteAddress) && ((tlv_address + tlv_len) >= overwriteByteAddress))
                    || ((tlv_address <= (overwriteByteAddress + overwriteLength)) && (tlv_address >= overwriteByteAddress)))
                {
                    status = PTX_STATUS_E_NDEF_TLV_OVERWRITE_ERROR;
                }

            }
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            for (uint8_t idx = 0; idx < ndefT2T->TLVs.NumberOfMemoryTLVs; idx++)
            {
                tlv_address = ndefT2T->TLVs.MemoryControlTLVs[idx].ByteAddress;
                tlv_len = PTX_T2T_MEMORY_CONTROL_TLV_LENGTH + 1; // Length of Memory Control TLV + length information byte length;

                if (((tlv_address <= overwriteByteAddress) && ((tlv_address + tlv_len) >= overwriteByteAddress))
                    || ((tlv_address <= (overwriteByteAddress + overwriteLength)) && (tlv_address >= overwriteByteAddress)))
                {
                    status = PTX_STATUS_E_NDEF_TLV_OVERWRITE_ERROR;
                }

            }
        }
    }

    return status;
}


static ptx_Status_t ptx_NDEF_T2T_TLVGenerator(ptx_NDEF_T2T_t *ndefT2T, uint32_t tlvLen, uint8_t *buff, uint16_t *buffLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t index = 0;
    uint16_t new_begin_address;
    uint8_t new_len_field;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP) && (NULL != buff) && (NULL != buffLen))
    {
        uint16_t msg_len = *buffLen;

        new_begin_address = (uint16_t)(PTX_T2T_TLV_AREA_BEGIN_BLOCK * PTX_T2T_BLOCKSIZE);

        new_len_field = (msg_len >= PTX_T2T_LENGTHTHRESHOLD) ? (PTX_T2T_NDEF_L_FIELD_LONG) : (PTX_T2T_NDEF_L_FIELD_SHORT);

        /* Reset position to beginning of TLV area. */
        ndefT2T->CurrentByteAddress = (uint16_t)(PTX_T2T_TLV_AREA_BEGIN_BLOCK * PTX_T2T_BLOCKSIZE);
        ndefT2T->CurrentBlock = PTX_T2T_TLV_AREA_BEGIN_BLOCK;
        ndefT2T->CurrentBlockAddress = PTX_T2T_TLV_AREA_BEGIN_BLOCK;
        ndefT2T->SectorParams.CurrentSector = 0u;

        /* Search memory until we find a space the TLV will fit into, or until we run out of space. */
        do
        {
            status = ptx_NDEF_T2T_TLVProtection(ndefT2T, new_begin_address, (uint16_t)(msg_len + new_len_field));
            new_begin_address = (uint16_t)(new_begin_address + PTX_T2T_BLOCKSIZE);
        }
        while ((PTX_STATUS_SUCCESS != status) && (ndefT2T->CCParams.Size > new_begin_address));

        if (PTX_STATUS_SUCCESS == status)
        {
            memcpy(&ndefT2T->WorkBuffer[0], &buff[0], msg_len);

            if (PTX_T2T_LENGTHTHRESHOLD > tlvLen)
            {
                /* 1 L-Byte */
                if (msg_len <= (tlvLen + (PTX_T2T_NDEF_L_FIELD_SHORT + 1)))
                {
                    index = (uint32_t)(tlvLen + (PTX_T2T_NDEF_L_FIELD_SHORT + 1));

                    /* Push data back to make space for T- and L-Bytes. */
                    while (index > 1)
                    {
                        buff[index] = buff[index - (PTX_T2T_NDEF_L_FIELD_SHORT + 1)];
                        index--;
                    }
                    *buffLen = (uint16_t)(tlvLen + (PTX_T2T_NDEF_L_FIELD_SHORT + 1));

                    /* Fill the beginning of the buffer with T and L */
                    buff[PTX_T2T_NDEF_TLV_T_OFFSET] = PTX_T2T_NDEF_TLV_T;
                    buff[PTX_T2T_NDEF_TLV_L_OFFSET] = (uint8_t)tlvLen;

                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    *buffLen = 0;
                    status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                }
            }
            else
            {
                /* 3 L-Byte */
                if (*buffLen <= (tlvLen + (PTX_T2T_NDEF_L_FIELD_LONG + 1)))
                {
                    index = (uint32_t)(tlvLen + (PTX_T2T_NDEF_L_FIELD_LONG + 1));

                    /* Push data back to make space for T- and L-Bytes. */
                    while (index > 1)
                    {
                        buff[index] = buff[index - (PTX_T2T_NDEF_L_FIELD_LONG + 1)];
                        index--;
                    }
                    *buffLen = (uint16_t)(tlvLen + (PTX_T2T_NDEF_L_FIELD_LONG + 1));

                    /* Fill the beginning of the buffer with T and L */
                    buff[PTX_T2T_NDEF_TLV_T_OFFSET] = PTX_T2T_NDEF_TLV_T;
                    buff[PTX_T2T_NDEF_TLV_L_OFFSET] = 0xFFu;
                    buff[PTX_T2T_NDEF_TLV_L_OFFSET + 1] = (uint8_t)((tlvLen & 0xFF00u) >> PTX_T2T_MASK_8);
                    buff[PTX_T2T_NDEF_TLV_L_OFFSET + 2] = (uint8_t)(tlvLen & 0x00FFu);

                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    *buffLen = 0;
                    status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Enough resources -> TLV was created */
                ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].ByteAddress = ndefT2T->CurrentByteAddress;
                ndefT2T->TLVs.NDEFTLV[ndefT2T->TLVs.NumberOfNdefTLVs].Length = (uint16_t)tlvLen;
                ndefT2T->TLVs.NumberOfNdefTLVs = 1;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_DLALoop (ptx_NDEF_T2T_t *ndefT2T,
                                          uint16_t nbrLockBytes,
                                          uint8_t nbrReservedBits,
                                          uint8_t firstOffset,
                                          uint8_t lastOffset)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t LockedBytes = 0;
    uint8_t bytes_to_write;
    uint8_t incomplete_lock_byte;
    uint8_t current_sector = 0;
    uint32_t rx_len;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        status = PTX_STATUS_SUCCESS;
        bytes_to_write = (uint8_t)nbrLockBytes;
        current_sector = ndefT2T->SectorParams.CurrentSector;

        while ((nbrLockBytes > LockedBytes) && (PTX_STATUS_SUCCESS == status) && ((bytes_to_write - PTX_T2T_BLOCKSIZE) > 0))
        {
            ndefT2T->CurrentBlockAddress = (ndefT2T->CurrentByteAddress / PTX_T2T_BLOCKSIZE);
            ndefT2T->CurrentBlock = (uint8_t)(ndefT2T->CurrentBlockAddress % PTX_T2T_SECTORSIZE);
            /* check sector */
            if (current_sector !=  (ndefT2T->CurrentBlockAddress / PTX_T2T_SECTORSIZE))
            {
                current_sector = (uint8_t)(ndefT2T->CurrentBlockAddress / PTX_T2T_SECTORSIZE);
                status = ptx_NDEF_T2T_SectorSelect(ndefT2T, current_sector, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_PAT_MS);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* read first block of DLA (following 3 ignored) */
                status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->RxBuffer[0], &rx_len);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* check for leading non-DLA bytes and put them into the work buffer to be written, then fill remaining bytes with 0xFF */
                    if (0 != firstOffset)
                    {
                        (void)memcpy(&ndefT2T->WorkBuffer[0], &ndefT2T->RxBuffer[0], (uint32_t)firstOffset);
                    }
                    (void)memset(&ndefT2T->WorkBuffer[firstOffset], (uint8_t)PTX_T2T_LOCKBITS, (uint32_t)(PTX_T2T_BLOCKSIZE - firstOffset));

                    /* write the leading non-DLA blocks and the lock bits set to 1 */
                    rx_len = ndefT2T->RxBufferSize;
                    status = ptx_NDEF_T2T_WriteBlock(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->WorkBuffer[0], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);
                }
            }
            bytes_to_write = (uint8_t)(bytes_to_write - (PTX_T2T_BLOCKSIZE - firstOffset));
            LockedBytes = (uint16_t)(LockedBytes + (PTX_T2T_BLOCKSIZE - firstOffset));
            ndefT2T->CurrentByteAddress = (uint16_t)(ndefT2T->CurrentByteAddress + (PTX_T2T_BLOCKSIZE - firstOffset));
            firstOffset = 0;
        }

        /* last block handled separately due to possible offset and reserved bits */
        ndefT2T->CurrentBlockAddress = (ndefT2T->CurrentByteAddress / PTX_T2T_BLOCKSIZE);
        ndefT2T->CurrentBlock = (uint8_t)(ndefT2T->CurrentBlockAddress % PTX_T2T_SECTORSIZE);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* check sector */
            if ((ndefT2T->CurrentBlock != (ndefT2T->CurrentBlockAddress / PTX_T2T_SECTORSIZE)))
            {
                current_sector = (uint8_t)(ndefT2T->CurrentBlockAddress / PTX_T2T_SECTORSIZE);
                status = ptx_NDEF_T2T_SectorSelect(ndefT2T, current_sector, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_PAT_MS);
            }

            /* read last block (3 following blocks are ignored) */
            status = ptx_NDEF_T2T_ReadBlocks(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->RxBuffer[0], &rx_len);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* fill the DLA bytes with 0xFF and put trailing non-DLA bytes into work buffer */
            (void)memcpy(&ndefT2T->WorkBuffer[0], &ndefT2T->RxBuffer[0], (uint32_t)PTX_T2T_BLOCKSIZE);
            (void)memset(&ndefT2T->WorkBuffer[firstOffset], PTX_T2T_ACCESS_READONLY, (uint32_t)((lastOffset + 1) - firstOffset));

            /* handle reserved bits if last DLA byte is not completely filled with lock bits */
            if (0 != nbrReservedBits)
            {
                /* DLA bytes are filled lsb to msb, push x 0s into the byte for reserved bits */
                incomplete_lock_byte = (uint8_t)((uint8_t)PTX_T2T_LOCKBITS >> nbrReservedBits);
                (void)memcpy(&ndefT2T->WorkBuffer[lastOffset], &incomplete_lock_byte, (uint32_t)1);
            }
            rx_len = ndefT2T->RxBufferSize;
            status = ptx_NDEF_T2T_WriteBlock(ndefT2T, ndefT2T->CurrentBlock, &ndefT2T->WorkBuffer[0], PTX_T2T_BLOCKSIZE, &ndefT2T->RxBuffer[0], &rx_len, PTX_T2T_DEFAULT_TIMEOUT_MS);
            if (PTX_STATUS_SUCCESS == status)
            {
                LockedBytes = (uint16_t)(LockedBytes + ((lastOffset + 1) - firstOffset));
            }
            if ((LockedBytes == nbrLockBytes) && (PTX_STATUS_SUCCESS == status))
            {
                /* everything ok, all lock bits set */
            }
            else
            {
                status = PTX_STATUS_E_NDEF_ASSERTION_ERROR;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_LockDefault(ptx_NDEF_T2T_t *ndefT2T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t NbrLockBits;
    uint16_t NbrLockBytes;
    uint8_t first_lock_byte_offset;
    uint8_t last_lock_byte_offset;
    uint16_t last_lock_byte_address;
    uint8_t nbr_reserved_bits = 0;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        /* use default setting for dynamic lock bits */
        NbrLockBits = (uint16_t)((ndefT2T->CCParams.Size - PTX_T2T_MIN_SIZE_FOR_DLA) / PTX_T2T_BYTES_LOCKED_PER_BIT);
        if (0 != ((ndefT2T->CCParams.Size - PTX_T2T_MIN_SIZE_FOR_DLA) % PTX_T2T_BYTES_LOCKED_PER_BIT))
        {
            NbrLockBits = (uint16_t)(NbrLockBits + ((ndefT2T->CCParams.Size - PTX_T2T_MIN_SIZE_FOR_DLA) % PTX_T2T_BYTES_LOCKED_PER_BIT));
        }

        NbrLockBytes = (uint16_t)(NbrLockBits / PTX_T2T_BYTES_LOCKED_PER_BIT);
        if (0 != (NbrLockBits % PTX_T2T_BYTES_LOCKED_PER_BIT))
        {
            NbrLockBytes++;
            nbr_reserved_bits = (uint8_t)(PTX_T2T_BYTES_LOCKED_PER_BIT - (NbrLockBits % PTX_T2T_BYTES_LOCKED_PER_BIT));
        }

        /* go to first byte after TLV area */
        ndefT2T->CurrentByteAddress = (uint16_t)(ndefT2T->CCParams.Size + (PTX_T2T_TLV_AREA_BEGIN_BLOCK * PTX_T2T_BLOCKSIZE));

        first_lock_byte_offset = (ndefT2T->CurrentByteAddress % PTX_T2T_BLOCKSIZE);
        last_lock_byte_address = (uint16_t)(ndefT2T->CurrentByteAddress + NbrLockBytes - 1);
        last_lock_byte_offset  = (last_lock_byte_address % PTX_T2T_BLOCKSIZE);

        status = ptx_NDEF_T2T_DLALoop(ndefT2T, NbrLockBytes, nbr_reserved_bits, first_lock_byte_offset, last_lock_byte_offset);
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_LockDLA(ptx_NDEF_T2T_t *ndefT2T)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t NbrLockBytes;
    uint8_t first_lock_byte_offset;
    uint8_t last_lock_byte_offset;
    uint8_t nbr_reserved_bits = 0;
    uint16_t first_dla_address;
    uint16_t last_dla_address;

    if (PTX_COMP_CHECK(ndefT2T, Comp_T2TOP))
    {
        /* use lock control TLVs */

        /* iterate over all DLAs */
        for (uint8_t LockTLV = 0; LockTLV < ndefT2T->TLVs.NumberOfLockTLVs; LockTLV++)
        {
            first_dla_address = ndefT2T->TLVs.LockControlTLVs[LockTLV].LockArea[0];
            last_dla_address  = ndefT2T->TLVs.LockControlTLVs[LockTLV].LockArea[1];
            nbr_reserved_bits = ndefT2T->TLVs.LockControlTLVs[LockTLV].NbrReservedBits;

            ndefT2T->CurrentByteAddress = first_dla_address;
            first_lock_byte_offset = (ndefT2T->CurrentByteAddress % PTX_T2T_BLOCKSIZE);
            last_lock_byte_offset = (last_dla_address % PTX_T2T_BLOCKSIZE);

            NbrLockBytes = (uint16_t)(last_dla_address - first_dla_address);

            status = ptx_NDEF_T2T_DLALoop(ndefT2T, NbrLockBytes, nbr_reserved_bits, first_lock_byte_offset, last_lock_byte_offset);
        }
    }

    return status;
}

