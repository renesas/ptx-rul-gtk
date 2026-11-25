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
    File        : ptx_NDEF_T2T.h

    Description : NFC Data Exchange Format (NDEF) implementation to control NDEF Messages on T2T Devices.
*/

/**
 * \addtogroup NDEF_T2T
 *
 * @{
 */

#if !defined(PTX_NDEF_T2T_H)
#define PTX_NDEF_T2T_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stddef.h>
#include "ptx_Status.h"
#include "ptx_Components.h"

#include "ptx_NativeTag_T2T.h"
#include "ptx_NDEF_Defines.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name Internal defines and limits.
 * @{
 */
#define PTX_T2T_DEFAULT_TIMEOUT_MS          (uint32_t)15   /**< T2T Tag specific default timeout. */
#define PTX_T2T_SUPPORTED_VERSION           (uint8_t)0x11  /**< T2T Tag specific default supported version. */
#define PTX_T2T_MAX_NUMBER_LOCK_CONTROL     (uint8_t)3u    /**< T2T Tag specific default max number of lock controls. */
#define PTX_T2T_MAX_NUMBER_MEMORY_CONTROL   (uint8_t)3u    /**< T2T Tag specific default max number of memory controls. */
#define PTX_T2T_MAX_NUMBER_NDEFTLVS         (uint8_t)1u    /**< T2T NDEF Operations Maximum NDEF TLVs to be handled. */
#define PTX_T2T_BLOCKSIZE                   (uint8_t)4u    /**< T2T block size. */
#define PTX_T2T_INTERNAL_BYTES_LENGTH       (uint8_t)10u   /**< Length of the "Internal Bytes" in the T2T memory. */
/** @} */
/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Variant which block(s) to read first to get the Capability Container CC.
 */
typedef enum ptx_NDEF_T2T_ReadCCVariant
{
    ReadCCVariant_Block_3_Default,                              /**< Reading CC-Block starts at Block 3 (default) */
    ReadCCVariant_Block_0,                                      /**< Reading CC-Block starts at Block 0 */
} ptx_NDEF_T2T_ReadCCVariant_t;

/**
 * \brief T2T NDEF OP Capability Container Parameters
 */
typedef struct ptx_NDEF_T2T_CC
{
    uint8_t                         MagicNumber;                 /**< Magic Number */
    uint8_t                         Version;                     /**< Version */
    uint16_t                        MLEN;                        /**< Length */
    uint8_t                         Access;                      /**< Access contains read and write access */

    uint8_t                         ReadAccess;                  /**< Read permission */
    uint8_t                         WriteAccess;                 /**< Write permission */
    uint8_t                         VersionMajor;                /**< Major Version */
    uint8_t                         VersionMinor;                /**< Minor Version */
    uint32_t                        Size;                        /**< Size in bytes */
    uint16_t                        NumberOfBlocks;              /**< Number of Blocks each containing 4 bytes */

    uint8_t                         InternalBytes[PTX_T2T_INTERNAL_BYTES_LENGTH];  /**< Contains the first 10 bytes of the T2T information if ReadCCVariant is set to ReadCCVariant_Block_0. */
} ptx_NDEF_T2T_CC_t;

/**
 * \brief T2T NDEF TLV position
 */
typedef struct ptx_NDEF_T2T_NDEF_TLV
{
    uint32_t                        ByteAddress;                                /**< Byte address of T byte */
    uint16_t                        Length;                                     /**< Length in byte */
    uint32_t                        ValueByteAddress;                           /**< Byte Address of start of Value */
} ptx_NDEF_T2T_NDEF_TLV_t;

/**
 * \brief T2T Lock Control TLV positions and counter
 */
typedef struct ptx_NDEF_T2T_LOCK_CONTROL_TLV
{
    uint32_t                        ByteAddress;                 /**< Byte addresses of T byte */
    uint32_t                        LockArea[2];                 /**< begin and end byte address of areas [begin,end] */
    uint8_t                         NbrReservedBits;             /**< Reserved Bits for not completely lock-bit-filled bytes */
    uint8_t                         BytesLockedPerLockBit;       /**< Bytes Locked per Lock Bit */
    uint32_t                        InternalArea[2];             /**< Internal Bytes beginning and end. End is always at least the same as LockArea end. */
} ptx_NDEF_T2T_LOCK_CONTROL_TLV_t;

/**
 * \brief T2T Memory Control TLV positions and counter
 */
typedef struct ptx_NDEF_T2T_MEMORY_CONTROL_TLV
{
    uint32_t                        ByteAddress;                 /**< Byte addresses of T byte */
    uint32_t                        RsvdArea[2];                 /**< begin and end byte address of areas [begin,end] */
} ptx_NDEF_T2T_MEMORY_CONTROL_TLV_t;

/**
 * \brief T2T NDEF OP TLV byte addresses and counters
 */
typedef struct ptx_NDEF_T2T_TLV
{
    uint8_t                                            TerminatorTLVFound;                                   /**< Terminator TLV */
    ptx_NDEF_T2T_NDEF_TLV_t                            NDEFTLV[PTX_T2T_MAX_NUMBER_NDEFTLVS];                 /**< NDEF TLV */
    uint8_t                                            NumberOfNdefTLVs;                                     /**< Number of NDEF TLVs */
    ptx_NDEF_T2T_LOCK_CONTROL_TLV_t                    LockControlTLVs[PTX_T2T_MAX_NUMBER_LOCK_CONTROL];     /**< Lock control TLV */
    uint8_t                                            NumberOfLockTLVs;                                     /**< Number of Lock Control TLVs */
    ptx_NDEF_T2T_MEMORY_CONTROL_TLV_t                  MemoryControlTLVs[PTX_T2T_MAX_NUMBER_MEMORY_CONTROL]; /**< Memory control TLV */
    uint8_t                                            NumberOfMemoryTLVs;                                   /**< Number of Memory Control TLVs */
    uint32_t                                           LogicalEndAddress;                                    /**< Available length in byte, not including other TLVs and DLAs/RAs */
} ptx_NDEF_T2T_TLV_t;

/**
 * \brief T2T NDEF OP sector information
 */
typedef struct ptx_NDEF_T2T_Sector
{
    uint8_t                                            CurrentSector;                           /**< Current sector */
} ptx_NDEF_T2T_Sector_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_NDEF_T2T
{
    ptx_Comps_t                     CompId;                                                     /**< Component Id */
    ptx_NativeTag_T2T_t             T2t;                                                        /**< Native Tag component. */
    ptx_NDEF_TagLifeCycle_t         LifeCycle;                                                  /**< Tag Life-Cycle */

    uint8_t                         *RxBuffer;                                                  /**< Internal Rx-Buffer provided by upper layer (may be shared with other components) */
    uint32_t                        RxBufferSize;                                               /**< Internal Maximum Rx-Buffer size */
    uint32_t                        RxLen;                                                      /**< Internal Current RxLen */

    uint8_t                         ReadCache[PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN];                /**< Cached blocks of the last read operation. */
    uint32_t                        ReadCacheAddress;                                           /**< Byte address of the first byte of the read cache. */
    uint8_t                         ReadCacheValid;                                             /**< Whether the read cache contains valid blocks. */

    uint32_t                        LogicalOffsetCache;                                         /**< Cached logical offset to byte address mapping. */
    uint32_t                        AddressCache;                                               /**< Cached byte address to logical offset mapping. */

    uint8_t                         WriteCache[PTX_T2T_BLOCKSIZE];                              /**< Current cached block ready to be written. */
    uint8_t                         WriteCacheValid;                                            /**< Whether the block currently in the write cache is valid. */ 
    uint8_t                         WriteCacheDirty;                                            /**< Whether the write cache is dirty */
    uint32_t                        WriteCacheAddress;                                          /**< Byte address of the first byte of the write cache */

    /* Tag Type specific members */
    ptx_NDEF_T2T_CC_t               CCParams;                                                   /**< T2T CC Parameters */
    ptx_NDEF_T2T_ReadCCVariant_t    ReadCCVariant;                                              /**< Reading initial NDEF-Content/CC-Block starting at Block 0 or 3 */
    ptx_NDEF_T2T_TLV_t              TLVs;                                                       /**< T2T TLVs */
    ptx_NDEF_T2T_Sector_t           SectorParams;                                               /**< T2T Sector Parameters */

    /* General Info and / or Greedy Collecting related Parameters */
    uint8_t                         LastOperationCheck;                                         /**< Flag whether the last operation we did was a CheckMsg. */
} ptx_NDEF_T2T_t;

/**
 * \brief Main Component initialisation Parameters
 */
typedef struct ptx_NDEF_T2T_InitParams
{
    ptx_RUL_t                    *Rul;               /**< RUL component. */
    uint8_t                      *TxBuffer;          /**< Internal tx buffer. Used for component internal exchanges (Recommended size: 256 bytes) */
    uint32_t                     TxBufferSize;       /**< Tx buffer size */
    uint8_t                      *RxBuffer;          /**< Internal rx buffer. Used for component internal exchanges (Recommended size: 256 bytes) */
    uint32_t                     RxBufferSize;       /**< Internal Rx-Buffer size */
    ptx_NDEF_T2T_ReadCCVariant_t ReadCCVariant;      /**< Reading initial NDEF-Content/CC-Block starting at Block 0 or 3 */
} ptx_NDEF_T2T_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component initialisation.
 *
 * \param[in]       ndefT2T         Pointer to component.
 * \param[in]       initParams      Pointer to initialisation parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_T2T_Open (ptx_NDEF_T2T_t *ndefT2T, ptx_NDEF_T2T_InitParams_t *initParams);

/**
 * \brief Component De-initialisation.
 *
 * \param[in]       ndefT2T   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_T2T_Close (ptx_NDEF_T2T_t *ndefT2T);

/**
 * \brief Check for NDEF Messages.
 *
 * \param[in]       ndefT2T   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_T2T_CheckMessage (ptx_NDEF_T2T_t *ndefT2T);

/**
 * \brief NDEF Read Message.
 *
 * \param[in]       ndefT2T         Pointer to component.
 * \param[in]       msgBuffer       Pointer to buffer holding the read NDEF-message.
 * \param[in,out]   msgLen          Size of the buffer (in), Length of the read NDEF-message (out).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_T2T_ReadMessage (ptx_NDEF_T2T_t *ndefT2T, uint8_t *msgBuffer, uint32_t *msgLen);

/**
 * \brief NDEF Write Message. Overwrites first NDEF TLV occurring, if existing.
 *
 * \param[in]       ndefT2T         Pointer to component.
 * \param[in]       msgBuffer       Pointer to buffer holding the NDEF-message to write (NULL -> empty NDEF-message is written).
 * \param[in]       msgLen          Size of NDEF-message (0 -> empty NDEF-message is written).

 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_T2T_WriteMessage (ptx_NDEF_T2T_t *ndefT2T, uint8_t *msgBuffer, uint32_t msgLen);

/**
 * \brief Format NDEF Tag.
 *
 * \param[in]       ndefT2T   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_T2T_FormatTag (ptx_NDEF_T2T_t *ndefT2T);

/**
 * \brief Lock NDEF Tag.
 *
 * \param[in]       ndefT2T   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_T2T_LockTag (ptx_NDEF_T2T_t *ndefT2T);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
