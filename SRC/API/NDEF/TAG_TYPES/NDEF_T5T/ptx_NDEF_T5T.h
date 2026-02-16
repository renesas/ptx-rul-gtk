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
    File        : ptx_NDEF_T5T.h

    Description : NFC Data Exchange Format (NDEF) implementation to control NDEF Messages on T5T Devices.
*/

/**
 * \addtogroup NDEF_T5T
 *
 * @{
 */

#if !defined(PTX_NDEF_T5T_H)
#define PTX_NDEF_T5T_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NDEF_Defines.h"
#include "ptx_NativeTag_T5T.h"
#include "ptx_RUL.h"
#include <string.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
/**
 * \name T5T Tag specific size definitions and limits.
 * @{
 */
#define PTX_T5T_DEFAULT_TIMEOUT_MS          (uint32_t)200     /**< T5T Tag specific default timeout. */
#define PTX_T5T_SUPPORTED_VERSION           (uint8_t)0x10     /**< T5T Tag specific default supported version. */
#define PTX_T5T_UID_LEN                     (uint8_t)8u       /**< UID length. */
#define PTX_T5T_INVENTORY_RES_OFFSET_UID    (uint8_t)2        /**< Offset of UID within INVENTORY-RES. */
#define PTX_T5T_OFFSET_RES_FLAG             (uint8_t)0x00     /**< Result flag offset. */
#define PTX_T5T_OFFSET_RES_DATA             (uint8_t)0x01     /**< Result data offset fixed as 1 since "Block Security Status"-byte is never requested */
#define PTX_T5T_RES_FLAG_ERROR_LEN          (uint8_t)1        /**< Result flag error length. */
#define PTX_T5T_RES_FLAG_ERROR_MASK         (uint8_t)0x01     /**< Result flag error mask. */
#define PTX_T5T_SELECT_RES_LEN              (uint8_t)1        /**< Select Response length. */
/** @} */
/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief T5T NDEF OP Capability Container Parameters
 */
typedef struct ptx_NDEF_T5T_CC
{
    uint8_t                         MagicNumber;                /**< Magic Number. */
    uint8_t                         Version;                    /**< Version. */
    uint8_t                         ReadAccess;                 /**< Read permission. */
    uint8_t                         WriteAccess;                /**< Write permission. */
    uint16_t                        MLEN;                       /**< Length. */
    uint16_t                        ExtCommandTypeRequired;     /**< external command type required. */
    uint8_t                         SpecialFrameRequired;       /**< special frame required. */
    uint8_t                         MultiBlockReadSupported;    /**< Multi block read supported. */
    uint8_t                         LockBlockSupported;         /**< Lock Block supported. */
    uint8_t                         Size;                       /**< Size. */

} ptx_NDEF_T5T_CC_t;

/**
 * \brief T5T NDEF OP Component
 */
typedef struct ptx_NDEF_T5T
{
    /* Components */
    ptx_Comps_t                     CompId;                 /**< Component Id. */

    ptx_NativeTag_T5T_t             t5t;                    /**< T5T Native Tag Component. */
    ptx_NDEF_TagLifeCycle_t         LifeCycle;              /**< Tag Life-Cycle. */
    uint8_t                         *RxBuffer;              /**< Internal Rx-Buffer provided by upper layer (may be shared with other components). */
    uint32_t                        RxBufferSize;           /**< Internal Rx-Buffer size. */
    uint8_t                         *WorkBuffer;            /**< Internal Work-Buffer. */
    uint32_t                        WorkBufferSize;         /**< Internal Work-Buffer size. */

    /* Tag Type specific members */
    ptx_NDEF_T5T_CC_t               CCParams;               /**< T5T specific CC parameters. */

    /* General Info and / or Greedy Collection related Parameters */
    uint8_t                         BlockSize;              /**< Block size. */
    uint16_t                        NDEF_TLV_POS_BN;        /**< NDEF TLV POS BN. */
    uint8_t                         NDEF_TLV_POS_BY;        /**< NDEF TLV POS BY. */
    uint16_t                        NDEF_TLV_LENGTH;        /**< NDEF TLV length. */

} ptx_NDEF_T5T_t;

/**
 * \brief T5T NDEF OP Initialization Parameters
 */
typedef struct ptx_NDEF_T5T_InitParams
{
    ptx_RUL_t           *rul;                   /**< RUL provided by upper layer */
    uint8_t             *TxBuffer;              /**< Internal tx buffer. Used for component internal exchanges (Recommended size: 256 bytes) */
    uint32_t            TxBufferSize;           /**< Size of Internal TX-Buffer */
    uint8_t             *RxBuffer;              /**< Internal rx buffer. Used for component internal exchanges (Recommended size: 256 bytes) */
    uint32_t            RxBufferSize;           /**< Size of internal Rx-Buffer */
    uint8_t             *WorkBuffer;            /**< Internal work buffer. Used to store intermediate information (Recommended size: 256 bytes) */
    uint32_t            WorkBufferSize;         /**< Internal Work-Buffer size. */

    uint8_t             isSelected;             /**< Flag indicating if commands are to be sent in SELECTED mode (0 unselected, 1 selected) */
    uint8_t             isAddressed;            /**< Flag indicating if commands are to be sent in ADDRESSED mode (0 non-addressed, 1 addressed) */

} ptx_NDEF_T5T_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initialize / Open the T5T OP Component.
 *
 * \param[in]   ndefT5T             Pointer to an existing instance of the T5T-OP component.
 * \param[in]   initParams          Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEF_T5T_Open (ptx_NDEF_T5T_t *ndefT5T, ptx_NDEF_T5T_InitParams_t *initParams);

/**
 * \brief Formats a Type 5 Tag to INITIALIZED state.
 *
 * \param[in]   ndefT5T             Pointer to an existing instance of the T5T-OP component.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEF_T5T_FormatTag (ptx_NDEF_T5T_t *ndefT5T);

/**
 * \brief Checks if a NDEF-message is present on the given Tag (or not).
 *
 * \param[in]   ndefT5T             Pointer to an existing instance of the T5T-OP component.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEF_T5T_CheckMessage (ptx_NDEF_T5T_t *ndefT5T);

/**
 * \brief Reads a NDEF-message from a given Tag.
 *
 * \param[in]     ndefT5T             Pointer to an existing instance of the T5T-OP component.
 * \param[in]     msgBuffer           Pointer to buffer holding the read NDEF-message.
 * \param[in,out] msgLen              Size of the buffer (in), Length of the read NDEF-message (out).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEF_T5T_ReadMessage (ptx_NDEF_T5T_t *ndefT5T, uint8_t *msgBuffer, uint32_t *msgLen);

/**
 * \brief Writes a NDEF-message onto a given Tag.
 *
 * \param[in] ndefT5T             Pointer to an existing instance of the T5T-OP component.
 * \param[in] msgBuffer           Pointer to buffer holding the NDEF-message to write (NULL -> empty NDEF-message is written).
 * \param[in] msgLen              Size of NDEF-message (0 -> empty NDEF-message is written).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEF_T5T_WriteMessage (ptx_NDEF_T5T_t *ndefT5T, uint8_t *msgBuffer, uint32_t msgLen);

/**
 * \brief Puts a Tag into READ-ONLY state (Attention: This is a irreversible Operation!).
 *
 * \param[in]   ndefT5T             Pointer to an existing instance of the T5T-OP component.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEF_T5T_LockTag (ptx_NDEF_T5T_t *ndefT5T);

/**
 * \brief Un-initialize / Close the T5T OP Component
 *
 * \param[in]   ndefT5T             Pointer to an existing instance of the T5T-OP component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_T5T_Close (ptx_NDEF_T5T_t *ndefT5T);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
