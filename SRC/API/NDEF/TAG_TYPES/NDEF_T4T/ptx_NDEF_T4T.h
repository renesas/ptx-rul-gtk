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
    Module      : NFC Data Exchange Format (NDEF) T4T
    File        : ptx_NDEF_T4T.h

    Description : NFC Data Exchange Format (NDEF) implementation to control NDEF Messages on T4T Devices.
*/

/**
 * \addtogroup NDEF_T4T
 *
 * @{
 */

#if !defined(PTX_NDEF_T4T_H)
#define PTX_NDEF_T4T_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NativeTag_T4T.h"
#include "ptx_NDEF_Defines.h"
#include "ptx_Status.h"
#include "ptx_Components.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
/**
 * \name T4T Tag specific data length definitions.
 * @{
 */
#define PTX_T4T_DEFAULT_TIMEOUT_MS      (uint32_t)50000 /**< Default timeout value in ms. */
#define PTX_T4T_MAXIMUM_NLEN_LENGTH     (uint8_t)4u     /**< Maximum NLEN length. */
#define PTX_T4T_CC_LEN                  (uint8_t)2u     /**< CC length. */
#define PTX_T4T_FILEIDENTIFIER_LEN      (uint8_t)2u     /**< File Identifier length. */
/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */
/**
 * \brief T4T NDEF OP Capability Container Parameters
 */
typedef struct ptx_NDEF_T4T_CC
{
    uint8_t                         CCLen[PTX_T4T_CC_LEN];  /**< Length of the CC file, 2 bytes */
    uint8_t                         MappingVersion;         /**< Mapping version info */
    uint8_t                         MappingMajor;           /**< Mapping Major version. */
    uint8_t                         MappingMinor;           /**< Mapping Minor version. */
    uint16_t                        MLeDigit;               /**< Digit containing maximum RAPDU size */
    uint16_t                        MLcDigit;               /**< Digit containing maximum CAPDU size */
    uint8_t                         NDEFTLV[10];            /**< Stores NDEF TLV, 8 bytes for mapping version 20h, 10 bytes for 30h, checked accordingly */
    uint8_t                         NDEFFileIdentifier[PTX_T4T_FILEIDENTIFIER_LEN];  /**< NDEF file identifier, 2 bytes */
    uint32_t                        NDEFFileSize;           /**< Size of the NDEF file in bytes, 2 bytes */
    uint8_t                         NDEFAccessRead;         /**< Access condition for read */
    uint8_t                         NDEFAccessWrite;        /**< Access condition for write */

} ptx_NDEF_T4T_CC_t;

/**
 * \brief T4T NDEF file contents
 */
typedef struct ptx_NDEF_T4T_NLEN
{
    uint8_t                         NLEN[PTX_T4T_MAXIMUM_NLEN_LENGTH]; /**< NLEN or ENLEN field */
    uint8_t                         NbrNLENBytes;           /**< Number of relevant (E)NLEN bytes, either 2 or 4 */
    uint32_t                        DigitNLEN;              /**< Digit representation of (E)NLEN */

} ptx_NDEF_T4T_NLEN_t;


/**
 * \brief Main Component Struture
 */
typedef struct ptx_NDEF_T4T
{
    /* Components */
    ptx_Comps_t                     CompId;                /**< Component Id */

    ptx_NativeTag_T4T_t             t4t;                   /**< T4T Native Tag Component */
    ptx_NDEF_TagLifeCycle_t         LifeCycle;             /**< Tag Life-Cycle */
    uint8_t                         *RxBuffer;             /**< Internal Rx-Buffer provided by upper layer (may be shared with other components) */
    uint32_t                        RxBufferSize;          /**< Internal Rx-Buffer size */

    /* Tag Type specific members */
    ptx_NDEF_T4T_CC_t               CCParams;              /**< T4T specific CC parameters */

    ptx_NDEF_T4T_NLEN_t             NLEN;                  /**< T4T NDEF file contents */
} ptx_NDEF_T4T_t;

/**
 * \brief Main Component initialisation Parameters
 */
typedef struct ptx_NDEF_T4T_InitParams
{
    ptx_RUL_t                       *rul;               /**< Allocated instance of RUL component */
    uint8_t                         *TxBuffer;          /**< Internal tx buffer. Used for component internal exchanges (Recommended size: 256 bytes) */
    uint32_t                        TxBufferSize;       /**< Size of TX Buffer */
    uint8_t                         *RxBuffer;          /**< Internal rx buffer. Used for component internal exchanges (Recommended size: 256 bytes) */
    uint32_t                        RxBufferSize;       /**< Size of Internal Rx-Buffer */

} ptx_NDEF_T4T_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component initialisation.
 *
 * \param[in]       ndefT4T         Pointer to component.
 * \param[in]       initParams      Pointer to initialisation parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_T4T_Open (ptx_NDEF_T4T_t *ndefT4T, ptx_NDEF_T4T_InitParams_t *initParams);

/**
 * \brief Component De-initialisation.
 *
 * \param[in]       ndefT4T         Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_T4T_Close (ptx_NDEF_T4T_t *ndefT4T);

/**
 * \brief Formats a Type 4 Tag to INITIALIZED state.
 *
 * \param[in]   ndefT4T             Pointer to an existing instance of the T4T-OP component.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEF_T4T_FormatTag (ptx_NDEF_T4T_t *ndefT4T);

/**
 * \brief Checks if a NDEF-message is present on the given Tag (or not).
 *
 * \param[in]   ndefT4T             Pointer to an existing instance of the T4T-OP component.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEF_T4T_CheckMessage (ptx_NDEF_T4T_t *ndefT4T);

/**
 * \brief Reads a NDEF-message from a given Tag.
 *
 * \param[in]     ndefT4T             Pointer to an existing instance of the T4T-OP component.
 * \param[in]     msgBuffer           Pointer to buffer holding the read NDEF-message.
 * \param[in,out] msgLen              Size of the buffer (in), Length of the read NDEF-message (out).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEF_T4T_ReadMessage (ptx_NDEF_T4T_t *ndefT4T, uint8_t *msgBuffer, uint32_t *msgLen);

/**
 * \brief Writes a NDEF-message onto a given Tag.
 *
 * \param[in] ndefT4T             Pointer to an existing instance of the T4T-OP component.
 * \param[in] msgBuffer           Pointer to buffer holding the NDEF-message to write (NULL -> empty NDEF-message is written).
 * \param[in] msgLen              Size of NDEF-message (0 -> empty NDEF-message is written).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEF_T4T_WriteMessage (ptx_NDEF_T4T_t *ndefT4T, uint8_t *msgBuffer, uint32_t msgLen);

/**
 * \brief Puts a Tag into READ-ONLY state (Attention: This is a irreversible Operation!).
 *
 * \param[in]   ndefT4T             Pointer to an existing instance of the T4T-OP component.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEF_T4T_LockTag (ptx_NDEF_T4T_t *ndefT4T);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
