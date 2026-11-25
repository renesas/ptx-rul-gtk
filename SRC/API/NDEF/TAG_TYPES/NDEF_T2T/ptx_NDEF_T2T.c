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
 * Static Lock Bit Offsets
 *
 */
#define PTX_T2T_STATIC_LOCK_BYTE_0_OFFSET   (uint8_t)0x02    /* Offset of static lock byte 0 in PTX_T2T_STATIC_LOCK_BLOCK_NUMBER */
#define PTX_T2T_STATIC_LOCK_BYTE_1_OFFSET   (uint8_t)0x03    /* Offset of static lock byte 1 in PTX_T2T_STATIC_LOCK_BLOCK_NUMBER */
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
#define PTX_T2T_MAX_NULL_TLV_COUNT          (uint8_t)3u      /* Maximum number of back-to-back NULL TLVs. */
/*  */

/*
 * Operation Size Informations
 *
 */
#define PTX_T2T_BLOCKSIZE                   (uint8_t)4u      /* T2T block size. */
#define PTX_T2T_BLOCKS_PER_READ             (uint8_t)4u      /* T2T blocks read per read operation. */
#define PTX_T2T_MIN_SIZE_FOR_DLA            (uint8_t)0x30    /* Minimum memory size for dynamic lock areas to be present. */
#define PTX_T2T_SECTORSIZE                  (uint16_t)256u   /* T2T sector size in blocks. */
#define PTX_T2T_LENGTHTHRESHOLD             (uint8_t)0xFF    /* Length too large for 1 L-Byte, starting from this, 3 L-Bytes are used. */
#define PTX_T2T_SECTORTHRESHOLD             (uint8_t)0xFF    /* Block Address threshold within a block where a new Sector will be needed. */
#define PTX_T2T_LAST_READ_BLOCK             (uint8_t)252u    /* Last block number that should be read in a sector. */
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
/**
 * Get CC Parameters.
 *
 * \param[in] ndefT2t           Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_GetCCInfo (ptx_NDEF_T2T_t *ndefT2t);

/**
 * Handle Tag LC with CC Information.
 *
 * \param[in] ndefT2t           Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_HandleCCInfo(ptx_NDEF_T2T_t *ndefT2t);

/**
 * Read a byte from the byte address. If the correct sector is not selected, also selects the sector.
 * Tries to read from cached read blocks first, if desired address is not cached, performs a T2T read
 * and caches the read blocks.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in] byteAddress       The byte address to read.
 * \param[out] value            Pointer to uint8_t where read value shall be stored.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_ReadByte(ptx_NDEF_T2T_t *ndefT2t, uint32_t byteAddress, uint8_t *value);

/**
 * Read a logical byte from the given logical offset. Logical bytes skip all reserved areas while reading.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in] logicalOffset     Logical offset to read.
 * \param[out] value            Output byte.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_ReadLogicalByte(ptx_NDEF_T2T_t *ndefT2t, uint32_t logicalOffset, uint8_t *value);

/**
 * Read multiple logical bytes. See \ref ptx_NDEF_T2T_ReadLogicalByte.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in] logicalOffset     Logical offset to start reading from.
 * \param[in] length            Number of bytes to read.
 * \param[out] value            Output buffer.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_ReadLogicalBytes(ptx_NDEF_T2T_t *ndefT2t, uint32_t logicalOffset, uint16_t length, uint8_t *value);

/**
 * Skip reserved address ranges for the given address, if it falls inside a reserved range.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in,out] address       Input address, updated if it falls inside a reserved range.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_SkipReservedAddress(ptx_NDEF_T2T_t *ndefT2t, uint32_t *address);

/**
 * Convert a logical offset to a byte address.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in] logicalOffset     Logical offset to translate.
 * \param[out] address          Output address.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_GetAddressFromLogical(ptx_NDEF_T2T_t *ndefT2t, uint32_t logicalOffset, uint32_t *address);

/**
 * Flush pending write cache contents to the tag by writing one block if cache is dirty.
 *
 * \param[in] ndefT2t           Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_FlushWriteCache(ptx_NDEF_T2T_t *ndefT2t);

/**
 * Write a single byte to the given byte address.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in] byteAddress       Byte address.
 * \param[in] value             Byte value to write.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_WriteByte(ptx_NDEF_T2T_t *ndefT2t, uint32_t byteAddress, uint8_t value);

/**
 * Write a byte to the given logical offset. Logical bytes skip all reserved areas while reading.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in] logicalOffset     Logical offset to write.
 * \param[in] value             Byte value to write.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_WriteLogicalByte(ptx_NDEF_T2T_t *ndefT2t, uint32_t logicalOffset, uint8_t value);

/**
 * Select a Sector. Checks if currently selected sector is already correct before selecting the sector.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in] secNr             Sector Number to select.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_SectorSelect(ptx_NDEF_T2T_t *ndefT2t, uint8_t secNr);

/**
 * Read the available NDEF TLV.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in] msgBufferSize     Size of the message buffer.
 * \param[out] msgBuffer        Buffer to store messages.
 * \param[out] msgBufferOffset  Offset of buffer to start filling on input, buffer length on output.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_ReadNDEFTLV (ptx_NDEF_T2T_t *ndefT2t, uint32_t msgBufferSize, uint8_t *msgBuffer, uint16_t *msgBufferOffset);

/**
 * Overwrite the NDEF TLV with a new message if present. If no NDEF TLV is present, creates a new one.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in] tx                Message data to be written.
 * \param[in] txLen             Length of message.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_OverwriteMessage (ptx_NDEF_T2T_t *ndefT2t, uint8_t *tx, uint32_t txLen);

/**
 * A function called after getting the CC, sweeping the TLV area and getting all already present TLVs on the tag.
 *
 * \param[in] ndefT2t           Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_TLVAreaCrawler(ptx_NDEF_T2T_t *ndefT2t);

/**
 * Parses the next TLV based on the logical offset.
 *
 * \param[in] ndefT2t            Pointer to component.
 * \param[in,out] logicalOffset  Pointer to logical offset being parsed.
 * \param[out] parsingComplete   Parsing completion flag.
 * \param[in,out] nullTlvCount   Current number of back-to-back NULL TLVs.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_IdentifyTLV (ptx_NDEF_T2T_t *ndefT2t, uint32_t *logicalOffset, uint8_t *parsingComplete, uint8_t *nullTlvCount);

/**
 * Parse a TLV length field.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in,out] logicalOffset  Pointer to logical offset being parsed.
 * \param[out] length           Output TLV length.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_ParseTLVLength(ptx_NDEF_T2T_t *ndefT2t, uint32_t *logicalOffset, uint16_t *length);

/**
 * Store parsed TLV metadata.
 *
 * \param[in] ndefT2t           Pointer to component.
 * \param[in] type              TLV type.
 * \param[in] typeAddress       Logical address of the TLV type field.
 * \param[in] valueAddress      Logical address of the TLV value field.
 * \param[in] length            TLV length.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_StoreTLV(ptx_NDEF_T2T_t *ndefT2t, uint8_t type, uint32_t typeAddress, uint32_t valueAddress, uint16_t length);

/**
 * Loop over a DLA to lock it.
 *
 * \param[in] ndefT2t              Pointer to component.
 * \param[in] currentByteAddress   Byte address of the first lock byte.
 * \param[in] nbrLockBytes         Number of bytes to be locked.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_DLALoop (ptx_NDEF_T2T_t *ndefT2t,
                                          uint32_t currentByteAddress,
                                          uint16_t nbrLockBytes,
                                          uint8_t nbrReservedBits);

/**
 * Default DLA Lock procedure.
 *
 * \param[in] ndefT2t           Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_LockDefault(ptx_NDEF_T2T_t *ndefT2t);

/**
 * Lock using DLAs specified in LockControl TLVs.
 *
 * \param[in] ndefT2t           Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T2T_LockDLA(ptx_NDEF_T2T_t *ndefT2t);
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NDEF_T2T_Open (ptx_NDEF_T2T_t *ndefT2t, ptx_NDEF_T2T_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ndefT2t) && (NULL != initParams))
    {
        if ((NULL != initParams->RxBuffer) && (0 != initParams->RxBufferSize))
        {
            if(ptx_RUL_CardProtocolType_T2T == initParams->Rul->CardRegistry.ActiveProtocol)
            {
                /* clear component */
                (void)memset(ndefT2t, 0, sizeof(ptx_NDEF_T2T_t));

                ndefT2t->RxBuffer = initParams->RxBuffer;
                ndefT2t->RxBufferSize = initParams->RxBufferSize;
                ndefT2t->LifeCycle = TagLC_NoNDEFTag;
                ndefT2t->LogicalOffsetCache = 0;
                ndefT2t->AddressCache = PTX_T2T_TLV_AREA_BEGIN_BLOCK * PTX_T2T_BLOCKSIZE;
                ndefT2t->ReadCCVariant = initParams->ReadCCVariant;

                /* initialise lower layer component */
                ptx_NativeTag_T2T_InitParams_t t2t_init_params;
                (void)memset(&t2t_init_params, 0, sizeof(ptx_NativeTag_T2T_InitParams_t));
                t2t_init_params.Rul = initParams->Rul;
                t2t_init_params.TxBuffer = initParams->TxBuffer;
                t2t_init_params.TxBufferSize = initParams->TxBufferSize;

                status = ptx_NativeTag_T2T_Open(&ndefT2t->T2t, &t2t_init_params);

                /* set Component-ID at the end to prevent further calls in case of an error */
                if (PTX_STATUS_SUCCESS == status)
                {
                    ndefT2t->CompId = Comp_T2TOP;
                }
            }
            else
            {
                status = PTX_STATUS_E_GEN_NOT_PERMITTED;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T2T_FormatTag (ptx_NDEF_T2T_t *ndefT2t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            status = PTX_STATUS_E_NDEF_UNSUPPORTED;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T2T_CheckMessage (ptx_NDEF_T2T_t *ndefT2t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            /* reset TLV data */
            (void)memset(&ndefT2t->TLVs, 0, sizeof(ptx_NDEF_T2T_TLV_t));

            status = ptx_NDEF_T2T_GetCCInfo(ndefT2t);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_HandleCCInfo(ndefT2t);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* clear read cache because compliance tests want the TLV reads to start at block 4 explicitly. */
                ndefT2t->ReadCacheValid = 0;

                /* read the whole tag and look for TLVs */
                status = ptx_NDEF_T2T_TLVAreaCrawler(ndefT2t);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT2t->LastOperationCheck = 1u;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T2T_ReadMessage (ptx_NDEF_T2T_t *ndefT2t, uint8_t *msgBuffer, uint32_t *msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t *msg_buffer;
    uint32_t msg_buffer_size;
    uint16_t msg_buffer_offset = 0;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP) && (NULL != msgBuffer) && (NULL != msgLen) && (0 != *msgLen))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            msg_buffer_size = *msgLen;
            if (0 != msg_buffer_size)
            {
                msg_buffer = msgBuffer;

                switch (ndefT2t->LifeCycle)
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
                    status = ptx_NDEF_T2T_ReadNDEFTLV(ndefT2t, msg_buffer_size, msg_buffer, &msg_buffer_offset);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        *msgLen = msg_buffer_offset;
                    }
                    else
                    {
                        *msgLen = 0;
                    }

                    ndefT2t->LastOperationCheck = 0u;
                }
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T2T_WriteMessage (ptx_NDEF_T2T_t *ndefT2t, uint8_t *msgBuffer, uint32_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            switch (ndefT2t->LifeCycle)
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
                uint8_t empty_ndef_message[4] = {0xD8, 0x00, 0x00, 0x00};
                uint8_t empty_ndef_message_length = 4;

                if ((NULL == msgBuffer) || (0 == msgLen))
                {
                    status = ptx_NDEF_T2T_OverwriteMessage(ndefT2t, empty_ndef_message, empty_ndef_message_length);
                }
                else
                {
                    status = ptx_NDEF_T2T_OverwriteMessage(ndefT2t, msgBuffer, msgLen);
                }
                ndefT2t->LastOperationCheck = 0u;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T2T_LockTag (ptx_NDEF_T2T_t *ndefT2t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            if (0 != ndefT2t->SectorParams.CurrentSector)
            {
                status = ptx_NDEF_T2T_SectorSelect(ndefT2t, 0);
            }
            else
            {
                /* So far no errors. */
                status = PTX_STATUS_SUCCESS;
            }

            /* set CC access condition to read-only since CC will be locked by static lock bytes */
            if (PTX_STATUS_SUCCESS == status)
            {
                /* set CC access condition to read only */
                status = ptx_NDEF_T2T_WriteByte(ndefT2t, PTX_T2T_CC_BLOCK_NUMBER * PTX_T2T_BLOCKSIZE + PTX_T2T_CC_OFFSET_ACCESS_COND, (uint8_t)(PTX_T2T_ACCESS_READONLY >> PTX_T2T_MASK_4));

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T2T_FlushWriteCache(ndefT2t);
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    ndefT2t->LifeCycle = TagLC_ReadOnly;
                }
            }

            /* set static lock bytes */
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_WriteByte(ndefT2t, PTX_T2T_STATIC_LOCK_BLOCK_NUMBER * PTX_T2T_BLOCKSIZE + PTX_T2T_STATIC_LOCK_BYTE_0_OFFSET, PTX_T2T_LOCKBITS);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T2T_WriteByte(ndefT2t, PTX_T2T_STATIC_LOCK_BLOCK_NUMBER * PTX_T2T_BLOCKSIZE + PTX_T2T_STATIC_LOCK_BYTE_1_OFFSET, PTX_T2T_LOCKBITS);
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T2T_FlushWriteCache(ndefT2t);
                }
            }

            if ((PTX_T2T_MIN_SIZE_FOR_DLA < ndefT2t->CCParams.Size) && (PTX_STATUS_SUCCESS == status))
            {
                if (0 == ndefT2t->TLVs.NumberOfLockTLVs)
                {
                    status = ptx_NDEF_T2T_LockDefault(ndefT2t);
                }
                else
                {
                    status = ptx_NDEF_T2T_LockDLA(ndefT2t);
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }
    return status;
}

ptx_Status_t ptx_NDEF_T2T_Close (ptx_NDEF_T2T_t *ndefT2t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            status = ptx_NativeTag_T2T_Close(&ndefT2t->T2t);
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK(s)
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NDEF_T2T_GetCCInfo (ptx_NDEF_T2T_t *ndefT2t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            /* reset current CC-parameters */
            (void)memset(&ndefT2t->CCParams, 0, sizeof(ptx_NDEF_T2T_CC_t));
            (void)memset(&ndefT2t->SectorParams, 0, sizeof(ptx_NDEF_T2T_Sector_t));

            status = PTX_STATUS_SUCCESS;

            /* read CC-block */
            uint8_t read_index = 0;
            uint8_t cc_block[PTX_T2T_BLOCKSIZE] = {0};
            while ((PTX_STATUS_SUCCESS == status) && (PTX_T2T_BLOCKSIZE > read_index))
            {
                status = ptx_NDEF_T2T_ReadByte(ndefT2t, PTX_T2T_CC_BLOCK_NUMBER * PTX_T2T_BLOCKSIZE + read_index, &cc_block[read_index]);
                read_index += 1;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                if(ndefT2t->ReadCCVariant == ReadCCVariant_Block_0)
                {
                    /* Read the first 10 bytes for internal processing. */
                    read_index = 0;
                    while ((PTX_STATUS_SUCCESS == status) && (PTX_T2T_INTERNAL_BYTES_LENGTH > read_index))
                    {
                        status = ptx_NDEF_T2T_ReadByte(ndefT2t, read_index, &ndefT2t->CCParams.InternalBytes[read_index]);
                        read_index += 1;
                    }
                }
                else
                {
                    /* Set the internal bytes to 0. */
                    memset(&ndefT2t->CCParams.InternalBytes[0], 0, PTX_T2T_INTERNAL_BYTES_LENGTH);
                }

                ndefT2t->CCParams.MagicNumber = cc_block[PTX_T2T_CC_OFFSET_MAGIC_NUMBER];
                ndefT2t->CCParams.Version = cc_block[PTX_T2T_CC_OFFSET_VERSION_INFO];
                ndefT2t->CCParams.MLEN = cc_block[PTX_T2T_CC_OFFSET_TAG_SIZE];
                ndefT2t->CCParams.Access = cc_block[PTX_T2T_CC_OFFSET_ACCESS_COND];

                ndefT2t->CCParams.ReadAccess = (ndefT2t->CCParams.Access >> PTX_T2T_MASK_4);
                ndefT2t->CCParams.WriteAccess = (uint8_t)((PTX_T2T_CC_ACCESS_BITS != (ndefT2t->CCParams.Access & PTX_T2T_MASK_15)) ? (PTX_T2T_ACCESS_GRANTED) : (PTX_T2T_ACCESS_DENIED));

                ndefT2t->CCParams.VersionMajor = (ndefT2t->CCParams.Version >> PTX_T2T_MASK_4);
                ndefT2t->CCParams.VersionMinor = (ndefT2t->CCParams.Version & PTX_T2T_MASK_15);

                ndefT2t->CCParams.Size = (uint32_t)((uint32_t)ndefT2t->CCParams.MLEN * PTX_T2T_CC_MLEN_FACTOR); /* MLEN in Bytes */
                ndefT2t->CCParams.NumberOfBlocks = (uint16_t)((uint16_t)ndefT2t->CCParams.Size / (uint16_t)PTX_T2T_BLOCKSIZE);
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_HandleCCInfo(ptx_NDEF_T2T_t *ndefT2t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            switch (ndefT2t->CCParams.MagicNumber)
            {
                case PTX_T2T_CC_MAGIC_NUMBER:
                    ndefT2t->LifeCycle = TagLC_Initialised;
                    status = PTX_STATUS_SUCCESS;
                    break;
                default:
                    ndefT2t->LifeCycle = TagLC_NoNDEFTag;
                    status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
                    break;
            }

            /*
            * Minor version of device is 0, so check is unnecessary.
            * In case of change to device minor version, the following check should be added to if:
            * || PTX_T2T_VALID_MINOR_VERSION > ndefT2t->CCParams.VersionMinor
            */
            if ((PTX_T2T_VALID_MAJOR_VERSION != ndefT2t->CCParams.VersionMajor) && (PTX_STATUS_SUCCESS == status))
            {
                ndefT2t->LifeCycle = TagLC_NoNDEFTag;
                status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                switch (ndefT2t->CCParams.ReadAccess)
                {
                    case PTX_T2T_ACCESS_GRANTED:
                        break;

                    default:
                        status = PTX_STATUS_E_NDEF_TAG_ACCESS_DENIED;
                }
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                switch (ndefT2t->CCParams.WriteAccess)
                {
                    case PTX_T2T_ACCESS_GRANTED:
                        ndefT2t->LifeCycle = TagLC_ReadWrite;
                        break;
                    case PTX_T2T_ACCESS_DENIED:
                        ndefT2t->LifeCycle = TagLC_ReadOnly;
                        break;

                    default:
                        break;
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_ReadByte(ptx_NDEF_T2T_t *ndefT2t, uint32_t byteAddress, uint8_t *value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP) && (NULL != value))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            /* If there is an outstanding write, we first have to write the block. */
            status = ptx_NDEF_T2T_FlushWriteCache(ndefT2t);

            if (PTX_STATUS_SUCCESS == status)
            {
                uint32_t block_address = byteAddress / PTX_T2T_BLOCKSIZE;
                uint8_t sector_number = (uint8_t)(block_address / PTX_T2T_SECTORSIZE);
                uint16_t block_in_sector = (uint16_t)(block_address % PTX_T2T_SECTORSIZE);
                uint8_t read_block = (PTX_T2T_LAST_READ_BLOCK < block_in_sector) ? PTX_T2T_LAST_READ_BLOCK : (uint8_t)block_in_sector;
                uint32_t cache_address = (uint32_t)(sector_number * PTX_T2T_SECTORSIZE * PTX_T2T_BLOCKSIZE) + ((uint32_t)read_block * PTX_T2T_BLOCKSIZE);

                /* If requested byte is not already cached, perform a read */
                if ((0U == ndefT2t->ReadCacheValid) || (byteAddress < ndefT2t->ReadCacheAddress) ||
                    (byteAddress >= (ndefT2t->ReadCacheAddress + PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN)))
                {
                    status = ptx_NDEF_T2T_SectorSelect(ndefT2t, sector_number);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        ndefT2t->RxLen = ndefT2t->RxBufferSize;
                        status = ptx_NativeTag_T2T_Read(&ndefT2t->T2t, read_block, &ndefT2t->RxBuffer[0], &ndefT2t->RxLen, PTX_T2T_DEFAULT_TIMEOUT_MS);

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            if ((0 != ndefT2t->RxLen) && ((PTX_T2T_BLOCKS_PER_READ * PTX_T2T_BLOCKSIZE) != ndefT2t->RxLen))
                            {
                                status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
                            }
                        }
                        else
                        {
                            status = PTX_STATUS_E_NSC_RFERROR;
                        }

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            ndefT2t->ReadCacheAddress = cache_address;
                            ndefT2t->ReadCacheValid = 1U;
                            (void)memcpy(&ndefT2t->ReadCache[0], &ndefT2t->RxBuffer[0], ndefT2t->RxLen);
                        }
                    }
                }

                /* return requested byte from cache */
                if (PTX_STATUS_SUCCESS == status)
                {
                    *value = ndefT2t->ReadCache[byteAddress - ndefT2t->ReadCacheAddress];
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_ReadLogicalByte(ptx_NDEF_T2T_t *ndefT2t, uint32_t logicalOffset, uint8_t *value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP) && (NULL != value))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            uint32_t address = 0;

            status = ptx_NDEF_T2T_GetAddressFromLogical(ndefT2t, logicalOffset, &address);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_ReadByte(ndefT2t, address, value);
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_ReadLogicalBytes(ptx_NDEF_T2T_t *ndefT2t, uint32_t logicalOffset, uint16_t length, uint8_t *value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP) && (NULL != value))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            status = PTX_STATUS_SUCCESS;
            
            uint32_t index = 0;
            
            while ((length > index) && (PTX_STATUS_SUCCESS == status))
            {
                status = ptx_NDEF_T2T_ReadLogicalByte(ndefT2t, logicalOffset, &value[index]);

                if (PTX_STATUS_SUCCESS == status)
                {
                    logicalOffset += 1;
                    index += 1;
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }
    return status;
}

static ptx_Status_t ptx_NDEF_T2T_SkipReservedAddress(ptx_NDEF_T2T_t *ndefT2t, uint32_t *address)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP) && (NULL != address))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            uint8_t address_changed = 0;
            uint32_t safe_address = *address;

            do
            {
                address_changed = 0U;

                /* increment address for every intersecting reserved area */
                for (uint8_t tlvIndex = 0U; tlvIndex < ndefT2t->TLVs.NumberOfLockTLVs; tlvIndex++)
                {
                    /* Check for begin of lock area and end of internal area. 
                    * If no internal area is present because the DLA ends at the last byte of a block,
                    * InternalArea[1] is set to LockArea[1] so the check works regardless.
                    */
                    if ((ndefT2t->TLVs.LockControlTLVs[tlvIndex].LockArea[0] <= safe_address) && (ndefT2t->TLVs.LockControlTLVs[tlvIndex].InternalArea[1] > safe_address))
                    {
                        safe_address = ndefT2t->TLVs.LockControlTLVs[tlvIndex].InternalArea[1];
                        address_changed = 1U;
                    }
                }
                
                /* increment address for every intersecting reserved area */
                for (uint8_t tlvIndex = 0U; tlvIndex < ndefT2t->TLVs.NumberOfMemoryTLVs; tlvIndex++)
                {
                    if ((ndefT2t->TLVs.MemoryControlTLVs[tlvIndex].RsvdArea[0] <= safe_address) && (ndefT2t->TLVs.MemoryControlTLVs[tlvIndex].RsvdArea[1] > safe_address))
                    {
                        safe_address = ndefT2t->TLVs.MemoryControlTLVs[tlvIndex].RsvdArea[1];
                        address_changed = 1U;
                    }
                }
            }
            while (0U != address_changed);

            *address = safe_address;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_GetAddressFromLogical(ptx_NDEF_T2T_t *ndefT2t, uint32_t logicalOffset, uint32_t *address)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP) && (NULL != address))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            uint32_t logical_index = 0;
            uint32_t physical_address = 0;

            status = PTX_STATUS_SUCCESS;

            /* Check if we have the current offset cached */
            if ((ndefT2t->LogicalOffsetCache <= logicalOffset))
            {
                logical_index = ndefT2t->LogicalOffsetCache;
                physical_address = ndefT2t->AddressCache;
            }
            else
            {
                logical_index = 0U;
                physical_address = PTX_T2T_TLV_AREA_BEGIN_BLOCK * PTX_T2T_BLOCKSIZE;
            }

            /* find correct byte address for logical address while skipping reserved areas */
            while ((logicalOffset > logical_index) && (PTX_STATUS_SUCCESS == status))
            {
                status = ptx_NDEF_T2T_SkipReservedAddress(ndefT2t, &physical_address);

                physical_address++;
                logical_index++;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* we might be on the end of a reserved area, check once again */
                status = ptx_NDEF_T2T_SkipReservedAddress(ndefT2t, &physical_address);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT2t->LogicalOffsetCache = logicalOffset;
                ndefT2t->AddressCache = physical_address;
                *address = physical_address;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_FlushWriteCache(ptx_NDEF_T2T_t *ndefT2t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            status = PTX_STATUS_SUCCESS;

            if ((0u != ndefT2t->WriteCacheValid) && (0u != ndefT2t->WriteCacheDirty))
            {
                uint32_t block_address = ndefT2t->WriteCacheAddress / PTX_T2T_BLOCKSIZE;
                uint8_t sector_number = (uint8_t)(block_address / PTX_T2T_SECTORSIZE);
                uint8_t block_in_sector = (uint8_t)(block_address % PTX_T2T_SECTORSIZE);

                status = ptx_NDEF_T2T_SectorSelect(ndefT2t, sector_number);

                if (PTX_STATUS_SUCCESS == status)
                {
                    ndefT2t->RxLen = ndefT2t->RxBufferSize;
                    status = ptx_NativeTag_T2T_Write(&ndefT2t->T2t, block_in_sector, ndefT2t->WriteCache, PTX_T2T_BLOCKSIZE, &ndefT2t->RxBuffer[0], &ndefT2t->RxLen, PTX_T2T_DEFAULT_TIMEOUT_MS);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        if (0 != ndefT2t->RxLen)
                        {
                            status = (PTX_T2T_ACK_CODE != ndefT2t->RxBuffer[0]) ? (PTX_STATUS_E_NDEF_PROTOCOL_ERROR) : (status);
                        }
                    }
                    else
                    {
                        status = PTX_STATUS_E_NSC_RFERROR;
                    }

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* update read cache if necessary */
                        if ((0U != ndefT2t->ReadCacheValid) &&
                            (ndefT2t->WriteCacheAddress >= ndefT2t->ReadCacheAddress) &&
                            ((ndefT2t->WriteCacheAddress + PTX_T2T_BLOCKSIZE) <= (ndefT2t->ReadCacheAddress + PTX_T2T_BLOCKS_PER_READ * PTX_T2T_BLOCKSIZE)))
                        {
                            uint8_t cache_index = (uint8_t)(ndefT2t->WriteCacheAddress - ndefT2t->ReadCacheAddress);
                            (void)memcpy(&ndefT2t->ReadCache[cache_index], ndefT2t->WriteCache, PTX_T2T_BLOCKSIZE);
                        }

                        ndefT2t->WriteCacheDirty = 0U;
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_WriteByte(ptx_NDEF_T2T_t *ndefT2t, uint32_t byteAddress, uint8_t value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            status = PTX_STATUS_SUCCESS;
            uint32_t block_address = byteAddress - (byteAddress % PTX_T2T_BLOCKSIZE);

            if ((0U == ndefT2t->WriteCacheValid) || (block_address != ndefT2t->WriteCacheAddress))
            {
                status = ptx_NDEF_T2T_FlushWriteCache(ndefT2t);

                if (PTX_STATUS_SUCCESS == status)
                {
                    ndefT2t->WriteCacheValid = 0U;

                    for (uint8_t byte_number = 0U; byte_number < PTX_T2T_BLOCKSIZE; byte_number++)
                    {
                        status = ptx_NDEF_T2T_ReadByte(ndefT2t, block_address + byte_number, &ndefT2t->WriteCache[byte_number]);

                        if (PTX_STATUS_SUCCESS != status)
                        {
                            break;
                        }
                    }

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        ndefT2t->WriteCacheAddress = block_address;
                        ndefT2t->WriteCacheValid = 1U;
                        ndefT2t->WriteCacheDirty = 0U;
                    }
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT2t->WriteCache[(byteAddress - block_address)] = value;
                ndefT2t->WriteCacheDirty = 1U;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_WriteLogicalByte(ptx_NDEF_T2T_t *ndefT2t, uint32_t logicalOffset, uint8_t value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            uint32_t address = 0;

            status = ptx_NDEF_T2T_GetAddressFromLogical(ndefT2t, logicalOffset, &address);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_WriteByte(ndefT2t, address, value);
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_SectorSelect(ptx_NDEF_T2T_t *ndefT2t, uint8_t secNr)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            if (ndefT2t->SectorParams.CurrentSector == secNr)
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                ndefT2t->RxLen = ndefT2t->RxBufferSize;
                status = ptx_NativeTag_T2T_SectorSelect(&ndefT2t->T2t, secNr, &ndefT2t->RxBuffer[0], &ndefT2t->RxLen, PTX_T2T_PAT_MS);

                if (PTX_STATUS_SUCCESS == status)
                {
                    ndefT2t->SectorParams.CurrentSector = secNr;
                    ndefT2t->ReadCacheValid = 0u;
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_ReadNDEFTLV (ptx_NDEF_T2T_t *ndefT2t, uint32_t msgBufferSize, uint8_t *msgBuffer, uint16_t *msgBufferOffset)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP) && (NULL != msgBuffer) && (NULL != msgBufferOffset))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            /* If the last thing we did was not a CheckMsg, do a CheckMsg. */
            if (!ndefT2t->LastOperationCheck)
            {
                status = ptx_NDEF_T2T_CheckMessage(ndefT2t);
            }
            else
            {
                status = PTX_STATUS_SUCCESS;
            }

            if ((PTX_STATUS_SUCCESS == status) && (ndefT2t->TLVs.NumberOfNdefTLVs > 0))
            {
                if (ndefT2t->TLVs.NDEFTLV[0].Length > msgBufferSize)
                {
                    status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T2T_ReadLogicalBytes(ndefT2t, ndefT2t->TLVs.NDEFTLV[0].ValueByteAddress, ndefT2t->TLVs.NDEFTLV[0].Length, &msgBuffer[0]);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        *msgBufferOffset = ndefT2t->TLVs.NDEFTLV[0].Length;
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_OverwriteMessage (ptx_NDEF_T2T_t *ndefT2t, uint8_t *tx, uint32_t txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP) && (NULL != tx) && (0 != txLen))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            status = PTX_STATUS_SUCCESS; 
            
            uint8_t length_field_size = (PTX_T2T_LENGTHTHRESHOLD <= txLen) ? PTX_T2T_NDEF_L_FIELD_LONG : PTX_T2T_NDEF_L_FIELD_SHORT;
            uint32_t type_address = 0;

            if (0U < ndefT2t->TLVs.NumberOfNdefTLVs)
            {
                type_address = ndefT2t->TLVs.NDEFTLV[0].ByteAddress;
            }
            else
            {
                type_address = ndefT2t->TLVs.LogicalEndAddress;
            }

            uint32_t required_length = (uint32_t)1U + length_field_size + txLen;

            if (((uint32_t)(ndefT2t->CCParams.Size - type_address) < required_length))
            {
                status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
            }
            
            uint8_t write_terminator = 0;
            if (PTX_STATUS_SUCCESS == status)
            {
                write_terminator = ((uint32_t)(ndefT2t->CCParams.Size - type_address) > required_length) ? 1 : 0;
            }

            /*
            * Invalidate an existing NDEF first by setting length to 0.
            */
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_WriteLogicalByte(ndefT2t, type_address + 1, 0);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T2T_FlushWriteCache(ndefT2t);
                }
            }

            /* write NDEF TLV T part */
            if ((PTX_STATUS_SUCCESS == status) && (0U == ndefT2t->TLVs.NumberOfNdefTLVs))
            {
                status = ptx_NDEF_T2T_WriteLogicalByte(ndefT2t, type_address, PTX_T2T_NDEF_TLV_T);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T2T_FlushWriteCache(ndefT2t);
                }
            }

            uint32_t value_offset = type_address + 1U + length_field_size;

            uint32_t byte_index = 0U;
            while ((txLen > byte_index) && (PTX_STATUS_SUCCESS == status))
            {
                status = ptx_NDEF_T2T_WriteLogicalByte(ndefT2t, value_offset + byte_index, tx[byte_index]);
                byte_index++;
            }

            /* write Terminator TLV if space is available */
            if ((PTX_STATUS_SUCCESS == status) && (0U != write_terminator))
            {
                status = ptx_NDEF_T2T_WriteLogicalByte(ndefT2t, value_offset + txLen, PTX_T2T_TERMINATOR_TLV_T);
            
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T2T_FlushWriteCache(ndefT2t);
                }
            }

            /*
            * For an extended length, commit the two-byte value while the visible
            * length is still zero. The 0xFF marker is written last.
            */
            if ((PTX_STATUS_SUCCESS == status) && (PTX_T2T_NDEF_L_FIELD_LONG == length_field_size))
            {
                status = ptx_NDEF_T2T_WriteLogicalByte(ndefT2t, type_address + 2U, (uint8_t)(txLen >> 8U));

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T2T_WriteLogicalByte(ndefT2t, type_address + 3U, (uint8_t)txLen);
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T2T_WriteLogicalByte(ndefT2t, type_address + 1U, PTX_T2T_LENGTHTHRESHOLD);
                }
            }
            else if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_WriteLogicalByte(ndefT2t, type_address + 1U, (uint8_t)txLen);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_FlushWriteCache(ndefT2t);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT2t->WriteCacheValid = 0U;
                ndefT2t->ReadCacheValid = 0U;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_TLVAreaCrawler (ptx_NDEF_T2T_t *ndefT2t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            uint32_t logical_offset = 0;
            uint8_t parsing_complete = 0;
            uint8_t null_tlv_count = 0;

            do {
                status = ptx_NDEF_T2T_IdentifyTLV(ndefT2t, &logical_offset, &parsing_complete, &null_tlv_count);
            } while ((0U == parsing_complete) && (ndefT2t->TLVs.NumberOfNdefTLVs < PTX_T2T_MAX_NUMBER_NDEFTLVS) && (PTX_STATUS_SUCCESS == status));
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_IdentifyTLV(ptx_NDEF_T2T_t *ndefT2t, uint32_t *logicalOffset, uint8_t *parsingComplete, uint8_t *nullTlvCount)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP) && (NULL != logicalOffset) && (NULL != parsingComplete) && (NULL != nullTlvCount))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            uint32_t logical_offset = *logicalOffset;
            uint8_t type = 0;
            uint32_t type_offset = logical_offset;
            uint32_t type_physical_address = 0;
            uint8_t null_tlv_count = *nullTlvCount;

            status = ptx_NDEF_T2T_ReadLogicalByte(ndefT2t, logical_offset, &type);

            if (PTX_STATUS_SUCCESS == status)
            {
                type_physical_address = ndefT2t->AddressCache;
                logical_offset += 1;

                switch (type)
                {
                    case PTX_T2T_NULL_TLV_T:
                    {
                        if (PTX_T2T_MAX_NULL_TLV_COUNT == null_tlv_count)
                        {
                            *parsingComplete = 1U;

                            if ((PTX_T2T_MAX_NULL_TLV_COUNT + 1) == logical_offset)
                            {
                                ndefT2t->TLVs.LogicalEndAddress = 0;
                            }
                        }
                        else
                        {
                            null_tlv_count += 1;
                            *logicalOffset = logical_offset;
                            ndefT2t->TLVs.LogicalEndAddress = logical_offset;
                        }
                        break;
                    }
                    case PTX_T2T_TERMINATOR_TLV_T:
                    {
                        *parsingComplete = 1U;
                        break;
                    }
                    default:
                    {
                        /* Reset NULL TLV counter */
                        null_tlv_count = 0;

                        uint16_t length = 0;
                        uint32_t value_offset = 0;
                        uint32_t logical_offset_adjustment = 0;

                        status = ptx_NDEF_T2T_ParseTLVLength(ndefT2t, &logical_offset, &length);

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            value_offset = logical_offset;
                            uint8_t number_of_memory_tlvs = ndefT2t->TLVs.NumberOfMemoryTLVs;
                            status = ptx_NDEF_T2T_StoreTLV(ndefT2t, type, type_offset, value_offset, length);

                            /* a MemCtrlTLV might overlap with itself, so we have to adjust logical positioning */
                            if ((PTX_STATUS_SUCCESS == status) && (PTX_T2T_MEMORY_CONTROL_TLV_T == type) &&
                                (number_of_memory_tlvs < ndefT2t->TLVs.NumberOfMemoryTLVs))
                            {
                                uint32_t tlv_end_address = ndefT2t->AddressCache + 1U;
                                uint32_t reserved_start = ndefT2t->TLVs.MemoryControlTLVs[number_of_memory_tlvs].RsvdArea[0];
                                uint32_t reserved_end = ndefT2t->TLVs.MemoryControlTLVs[number_of_memory_tlvs].RsvdArea[1];
                                uint32_t overlap_start = (reserved_start > type_physical_address) ? reserved_start : type_physical_address;
                                uint32_t overlap_end = (reserved_end < tlv_end_address) ? reserved_end : tlv_end_address;

                                if (overlap_end > overlap_start)
                                {
                                    logical_offset_adjustment = overlap_end - overlap_start;
                                }
                            }
                        }

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            logical_offset = (uint32_t)(logical_offset + length - logical_offset_adjustment);
                            *logicalOffset = logical_offset;
                        }

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            ndefT2t->TLVs.LogicalEndAddress = logical_offset;
                        }

                        break;
                    }
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                *nullTlvCount = null_tlv_count;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_ParseTLVLength(ptx_NDEF_T2T_t *ndefT2t, uint32_t *logicalOffset, uint16_t *length)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP) && (NULL != logicalOffset) && (NULL != length))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            uint8_t first_length = 0;
            uint32_t logical_offset = *logicalOffset;

            status = ptx_NDEF_T2T_ReadLogicalByte(ndefT2t, logical_offset, &first_length);

            if (PTX_STATUS_SUCCESS == status)
            {
                logical_offset += 1;

                /* Check for long or short L field */
                if (PTX_T2T_LENGTHTHRESHOLD == first_length)
                {
                    uint8_t length_high = 0;
                    uint8_t length_low = 0;

                    status = ptx_NDEF_T2T_ReadLogicalByte(ndefT2t, logical_offset, &length_high);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        logical_offset += 1;
                        status = ptx_NDEF_T2T_ReadLogicalByte(ndefT2t, logical_offset, &length_low);
                    }

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        logical_offset += 1;
                        *length = (uint16_t)(((uint16_t)length_high << 8U) | length_low);
                    }
                }
                else
                {
                    *length = first_length;
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                *logicalOffset = logical_offset;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_StoreTLV(ptx_NDEF_T2T_t *ndefT2t, uint8_t type, uint32_t typeAddress, uint32_t valueAddress, uint16_t length)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            switch (type)
            {
                case PTX_T2T_LOCK_CONTROL_TLV_T:
                    if (PTX_T2T_MAX_NUMBER_LOCK_CONTROL > ndefT2t->TLVs.NumberOfLockTLVs)
                    {
                        uint8_t data[PTX_T2T_LOCK_CONTROL_TLV_LENGTH] = {0};
                        status = ptx_NDEF_T2T_ReadLogicalBytes(ndefT2t, valueAddress, length, &data[0]);

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            uint8_t DLAPosition = data[0];
                            uint16_t DLA_NbrLockBits = data[1];
                            if (0 == DLA_NbrLockBits)
                            {
                                DLA_NbrLockBits = PTX_T2T_SECTORSIZE;
                            }
                            uint8_t DLAControl = data[2];

                            uint8_t NbrMajorOffsets = (DLAPosition >> PTX_T2T_MASK_4);
                            uint8_t NbrMinorOffsets = (DLAPosition & 0x0F);

                            uint16_t DLA_NbrBytes = (uint16_t)(DLA_NbrLockBits / 8);
                            if (0 != (DLA_NbrLockBits % 8))
                            {
                                DLA_NbrBytes++;
                            }

                            uint8_t BLPLB_Index = (DLAControl >> PTX_T2T_MASK_4);
                            uint8_t MOS_DLA = (DLAControl & 0x0F);

                            ptx_NDEF_T2T_LOCK_CONTROL_TLV_t *tlv = &ndefT2t->TLVs.LockControlTLVs[ndefT2t->TLVs.NumberOfLockTLVs];

                            tlv->BytesLockedPerLockBit = (uint8_t)(1 << BLPLB_Index);

                            uint32_t MajorOffset_Size_DLA = (uint32_t)(1 << MOS_DLA);
                            uint32_t DLA_FirstByteAddress = (uint32_t)((NbrMajorOffsets * MajorOffset_Size_DLA) + NbrMinorOffsets);

                            uint8_t remBits = (uint8_t)(DLA_NbrLockBits % 8u);
                            tlv->NbrReservedBits = (uint8_t)((remBits == 0u) ? 0u : (8u - remBits));
                            tlv->LockArea[0] = (uint32_t)DLA_FirstByteAddress;
                            tlv->LockArea[1] = (uint32_t)(DLA_FirstByteAddress + DLA_NbrBytes);
                            tlv->InternalArea[0] = ((uint32_t)(PTX_T2T_BLOCKSIZE - 1 != ((DLA_FirstByteAddress + DLA_NbrBytes) - 1) % PTX_T2T_BLOCKSIZE)) ?
                                                                                    ((uint32_t)(tlv->LockArea[1])) : (0u);
                            if (0 != tlv->InternalArea[0])
                            {
                                tlv->InternalArea[1] = (uint32_t)(tlv->InternalArea[0] + (PTX_T2T_BLOCKSIZE - (tlv->InternalArea[0] % PTX_T2T_BLOCKSIZE)));
                            }
                            else
                            {
                                /* assign InternalArea End for easier checking. */
                            tlv->InternalArea[1] = tlv->LockArea[1];
                            }

                            tlv->ByteAddress = typeAddress;
                            ndefT2t->TLVs.NumberOfLockTLVs++;
                        }
                    }
                    else
                    {
                        status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                    }
                    break;

                case PTX_T2T_MEMORY_CONTROL_TLV_T:
                    if (PTX_T2T_MAX_NUMBER_MEMORY_CONTROL > ndefT2t->TLVs.NumberOfMemoryTLVs)
                    {
                        uint8_t data[PTX_T2T_MEMORY_CONTROL_TLV_LENGTH] = {0};
                        status = ptx_NDEF_T2T_ReadLogicalBytes(ndefT2t, valueAddress, length, &data[0]);

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            uint8_t RAPosition = data[0];
                            uint32_t Reserved_Area_Size = data[1];
                            if (0 == Reserved_Area_Size)
                            {
                                Reserved_Area_Size = PTX_T2T_SECTORSIZE;
                            }
                            uint8_t RAControl = data[2];
                            
                            uint8_t NbrMajorOffsets = (RAPosition >> PTX_T2T_MASK_4);
                            uint8_t NbrMinorOffsets = (RAPosition & 0x0F);
                            
                            uint8_t MOS_RA = (RAControl & 0x0F);
                            
                            uint32_t MajorOffset_Size_RA = (uint32_t)(1 << MOS_RA);
                            uint32_t RA_FirstByteAddress = (uint32_t)((NbrMajorOffsets * MajorOffset_Size_RA) + NbrMinorOffsets);
                            
                            ndefT2t->TLVs.MemoryControlTLVs[ndefT2t->TLVs.NumberOfMemoryTLVs].ByteAddress = typeAddress;
                            ndefT2t->TLVs.MemoryControlTLVs[ndefT2t->TLVs.NumberOfMemoryTLVs].RsvdArea[0] = (uint32_t)RA_FirstByteAddress;
                            ndefT2t->TLVs.MemoryControlTLVs[ndefT2t->TLVs.NumberOfMemoryTLVs].RsvdArea[1] = (uint32_t)(RA_FirstByteAddress + Reserved_Area_Size);
                            ndefT2t->TLVs.NumberOfMemoryTLVs++;
                        }
                    }
                    else
                    {
                        status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                    }
                    break;

                case PTX_T2T_NDEF_TLV_T:
                    ndefT2t->TLVs.NDEFTLV[ndefT2t->TLVs.NumberOfNdefTLVs].ByteAddress = typeAddress;
                    ndefT2t->TLVs.NDEFTLV[ndefT2t->TLVs.NumberOfNdefTLVs].ValueByteAddress = valueAddress;
                    ndefT2t->TLVs.NDEFTLV[ndefT2t->TLVs.NumberOfNdefTLVs].Length = length;
                    ndefT2t->TLVs.NumberOfNdefTLVs += 1;
                    status = PTX_STATUS_SUCCESS; 
                    break;

                default:
                    status = PTX_STATUS_SUCCESS;
                    break;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}


static ptx_Status_t ptx_NDEF_T2T_DLALoop (ptx_NDEF_T2T_t *ndefT2t,
                                          uint32_t currentByteAddress,
                                          uint16_t nbrLockBytes,
                                          uint8_t nbrReservedBits)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t lockedBytes = 0;
    uint8_t lock_value;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            status = PTX_STATUS_SUCCESS;

            /* write all lock bytes except the last one as fully locked */
            while ((lockedBytes < (uint16_t)(nbrLockBytes - 1u)) && (PTX_STATUS_SUCCESS == status))
            {
                status = ptx_NDEF_T2T_WriteByte(ndefT2t, currentByteAddress, PTX_T2T_LOCKBITS);
                currentByteAddress++;
                lockedBytes++;
            }

            /* write the last lock byte, accounting for reserved bits */
            if (PTX_STATUS_SUCCESS == status)
            {
                if (0u != nbrReservedBits)
                {
                    lock_value = (uint8_t)(PTX_T2T_LOCKBITS >> nbrReservedBits);
                }
                else
                {
                    lock_value = PTX_T2T_LOCKBITS;
                }
                status = ptx_NDEF_T2T_WriteByte(ndefT2t, currentByteAddress, lock_value);
                if (PTX_STATUS_SUCCESS == status)
                {
                    lockedBytes++;
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T2T_FlushWriteCache(ndefT2t);
            }

            if ((PTX_STATUS_SUCCESS == status) && (lockedBytes != nbrLockBytes))
            {
                status = PTX_STATUS_E_NDEF_ASSERTION_ERROR;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_LockDefault(ptx_NDEF_T2T_t *ndefT2t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t nbr_reserved_bits = 0;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            /* use default setting for dynamic lock bits */
            uint16_t NbrLockBits = (uint16_t)((ndefT2t->CCParams.Size - PTX_T2T_MIN_SIZE_FOR_DLA) / PTX_T2T_BYTES_LOCKED_PER_BIT);
            if (0 != ((ndefT2t->CCParams.Size - PTX_T2T_MIN_SIZE_FOR_DLA) % PTX_T2T_BYTES_LOCKED_PER_BIT))
            {
                NbrLockBits = (uint16_t)(NbrLockBits + ((ndefT2t->CCParams.Size - PTX_T2T_MIN_SIZE_FOR_DLA) % PTX_T2T_BYTES_LOCKED_PER_BIT));
            }

            uint16_t NbrLockBytes = (uint16_t)(NbrLockBits / PTX_T2T_BYTES_LOCKED_PER_BIT);
            if (0 != (NbrLockBits % PTX_T2T_BYTES_LOCKED_PER_BIT))
            {
                NbrLockBytes++;
                nbr_reserved_bits = (uint8_t)(PTX_T2T_BYTES_LOCKED_PER_BIT - (NbrLockBits % PTX_T2T_BYTES_LOCKED_PER_BIT));
            }

            /* go to first byte after TLV area */
            uint32_t start_byte_address = (uint32_t)(ndefT2t->CCParams.Size + (PTX_T2T_TLV_AREA_BEGIN_BLOCK * PTX_T2T_BLOCKSIZE));

            status = ptx_NDEF_T2T_DLALoop(ndefT2t, start_byte_address, NbrLockBytes, nbr_reserved_bits);
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T2T_LockDLA(ptx_NDEF_T2T_t *ndefT2t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT2t, Comp_T2TOP))
    {
        if(ptx_RUL_CardProtocolType_T2T == ndefT2t->T2t.Rul->CardRegistry.ActiveProtocol)
        {
            /* iterate over all DLAs */
            for (uint8_t LockTLV = 0; LockTLV < ndefT2t->TLVs.NumberOfLockTLVs; LockTLV++)
            {
                uint32_t first_dla_address = ndefT2t->TLVs.LockControlTLVs[LockTLV].LockArea[0];
                uint32_t last_dla_address  = ndefT2t->TLVs.LockControlTLVs[LockTLV].LockArea[1];
                uint8_t nbr_reserved_bits = ndefT2t->TLVs.LockControlTLVs[LockTLV].NbrReservedBits;

                uint16_t NbrLockBytes = (uint16_t)(last_dla_address - first_dla_address);

                status = ptx_NDEF_T2T_DLALoop(ndefT2t, first_dla_address, NbrLockBytes, nbr_reserved_bits);

                if (PTX_STATUS_SUCCESS != status)
                {
                    break;
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}
