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
    File        : ptx_NativeTag_T5T.h

    Description : NativeTag T5T implements the interface to communicate with a T5T NFC card.
*/

/**
 * \addtogroup T5T
 *
 * @{
 */

#if !defined(PTX_NATIVETAG_T5T_H)
#define PTX_NATIVETAG_T5T_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Status.h"
#include "ptx_Components.h"
#include "ptx_RUL.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
/**
 * \name T5T Tag specific definitions
 *
 * @{
 */
#define PTX_T5T_UID_SIZE             (uint8_t)8     /**< UID Size */
#define PTX_T5T_UID_INVENTORY_OFFSET (uint8_t)2     /**< UID Offset within INVENTORY_RES */
#define PTX_T5T_MIN_TX_BUFFER_SIZE   (uint32_t)64   /**< Minimum TX Buffer Size */
#define PTX_T5T_MAX_NR_RETRIES       (uint8_t)5     /**< T5T specific Retry Count upon timeout errors for read and write commands. */
/** @} */
/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Main Component Struture
 */
typedef struct ptx_NativeTag_T5T
{
    ptx_Comps_t         CompId;                 /**< Component Id */
    ptx_RUL_t           *rul;                   /**< RUL provided by upper layer */
    uint8_t             *TxBuffer;              /**< Internal Tx-Buffer provided by upper layer (may be shared with other components) */
    uint8_t             *UID;                   /**< UID */
    uint32_t            UIDLen;                 /**< UID Length */
    uint8_t             isSelected;             /**< Flag indicating if commands are to be sent in SELECTED mode (0 unselected, 1 selected) */

} ptx_NativeTag_T5T_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_NativeTag_T5T_InitParams
{
    ptx_RUL_t           *rul;                   /**< RUL provided by upper layer */
    uint8_t             *TxBuffer;              /**< Internal Tx-Buffer provided by upper layer */
    uint32_t            TxBufferSize;           /**< Size of Internal TX-Buffer */
    uint8_t             *UID;                   /**< UID */
    uint8_t             UIDLen;                 /**< Length of UID */
    uint8_t             isSelected;             /**< Flag indicating if commands are to be sent in SELECTED mode (0 unselected, 1 selected) */

} ptx_NativeTag_T5T_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialisation.
 *
 * \param[in]       t5t             Pointer to component.
 * \param[in]       initParams      Pointer to initialisation parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_Open (ptx_NativeTag_T5T_t *t5t, ptx_NativeTag_T5T_InitParams_t *initParams);

/**
 * \brief Component De-Initialisation.
 *
 * \param[in]       t5t   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_Close (ptx_NativeTag_T5T_t *t5t);


/**
 * \brief Performs a READ_SINGLE_BLOCK_REQ-command.
 *
 * \param[in]       t5t             Pointer to an existing instance of the T5T component.
 * \param[in]       optionFlag          Option-flag provided by upper layer ("Request Block Security Status" for Read-alike commands).
 * \param[in]       blockNr             Number of blocks to read.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_ReadSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                uint8_t optionFlag,
                                                uint8_t blockNr,
                                                uint8_t *rx,
                                                uint32_t *rxLen,
                                                uint32_t msTimeout);

/**
 * \brief Performs a WRITE_SINGLE_BLOCK_REQ-command.
 *
 * \param[in]       t5t             Pointer to an existing instance of the T5T component.
 * \param[in]       optionFlag          Option-flag provided by upper layer ("Use Special-Framing"-format for Write-alike commands).
 * \param[in]       blockNr             Number of block to write.
 * \param[in]       blockData           Data to write.
 * \param[in]       blockDataLen        Length of data to write.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_WriteSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                 uint8_t optionFlag,
                                                 uint8_t blockNr,
                                                 uint8_t *blockData,
                                                 uint8_t blockDataLen,
                                                 uint8_t *rx,
                                                 uint32_t *rxLen,
                                                 uint32_t msTimeout);

/**
 * \brief Performs a LOCK_SINGLE_BLOCK_REQ-command.
 *
 * \param[in]       t5t                 Pointer to an existing instance of the T5T component.
 * \param[in]       optionFlag          Option-flag provided by upper layer ("Use Special-Framing"-format for Write-alike commands).
 * \param[in]       blockNr             Number of block to lock.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           application-timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_LockSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                uint8_t optionFlag,
                                                uint8_t blockNr,
                                                uint8_t *rx,
                                                uint32_t *rxLen,
                                                uint32_t msTimeout);

/**
 * \brief Performs a READ_MULTIPLE_BLOCK_REQ-command.
 *
 * \param[in]       t5t             Pointer to an existing instance of the T5T component.
 * \param[in]       optionFlag          Option-flag provided by upper layer ("Request Block Security Status" for Read-alike commands).
 * \param[in]       blockNr             Number of start-block.
 * \param[in]       nrBlocks            Number of blocks to read.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_ReadMultipleBlock (ptx_NativeTag_T5T_t *t5t,
                                                  uint8_t optionFlag,
                                                  uint8_t blockNr,
                                                  uint8_t nrBlocks,
                                                  uint8_t *rx,
                                                  uint32_t *rxLen,
                                                  uint32_t msTimeout);

/**
 * \brief Performs an EXTENDED_READ_SINGLE_BLOCK_REQ-command.
 *
 * \param[in]       t5t             Pointer to an existing instance of the T5T component.
 * \param[in]       optionFlag          Option-flag provided by upper layer ("Request Block Security Status" for Read-alike commands).
 * \param[in]       blockNr             Number of block to read.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_ExtReadSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                   uint8_t optionFlag,
                                                   uint16_t blockNr,
                                                   uint8_t *rx,
                                                   uint32_t *rxLen,
                                                   uint32_t msTimeout);

/**
 * \brief Performs a EXTENDED_WRITE_SINGLE_BLOCK_REQ-command.
 *
 * \param[in]       t5t             Pointer to an existing instance of the T5T component.
 * \param[in]       optionFlag          Option-flag provided by upper layer ("Use Special-Framing"-format for Write-alike commands).
 * \param[in]       blockNr             Number of block to write.
 * \param[in]       blockData           Data to write.
 * \param[in]       blockDataLen        Length of data to write.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_ExtWriteSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                    uint8_t optionFlag,
                                                    uint16_t blockNr,
                                                    uint8_t *blockData,
                                                    uint8_t blockDataLen,
                                                    uint8_t *rx,
                                                    uint32_t *rxLen,
                                                    uint32_t msTimeout);

/**
 * \brief Performs a EXTENDED_LOCK_SINGLE_BLOCK_REQ-command.
 *
 * \param[in]       t5t             Pointer to an existing instance of the T5T component.
 * \param[in]       optionFlag          Option-flag provided by upper layer ("Use Special-Framing"-format for Write-alike commands).
 * \param[in]       blockNr             Number of block to lock.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_ExtLockSingleBlock (ptx_NativeTag_T5T_t *t5t,
                                                   uint8_t optionFlag,
                                                   uint16_t blockNr,
                                                   uint8_t *rx,
                                                   uint32_t *rxLen,
                                                   uint32_t msTimeout);

/**
 * \brief Performs a EXTENDED_READ_MULTIPLE_BLOCK_REQ-command.
 *
 * \param[in]       t5t             Pointer to an existing instance of the T5T component.
 * \param[in]       optionFlag          Option-flag provided by upper layer ("Request Block Security Status" for Read-alike commands).
 * \param[in]       blockNr             Number of start-block.
 * \param[in]       nrBlocks            Number of blocks to read.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_ExtReadMultipleBlock (ptx_NativeTag_T5T_t *t5t,
                                                     uint8_t optionFlag,
                                                     uint16_t blockNr,
                                                     uint16_t nrBlocks,
                                                     uint8_t *rx,
                                                     uint32_t *rxLen,
                                                     uint32_t msTimeout);

/**
 * \brief Performs a SELECT_REQ-command.
 *
 * \param[in]       t5t             Pointer to an existing instance of the T5T component.
 * \param[in]       optionFlag          Option-flag provided by upper layer ("Request Block Security Status" for Read-alike commands).
 * \param[in]       uid                 Pointer to UID of the given Tag.
 * \param[in]       uidLen              Length of UID.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_Select (ptx_NativeTag_T5T_t *t5t,
                                       uint8_t optionFlag,
                                       uint8_t *uid,
                                       uint8_t uidLen,
                                       uint8_t *rx,
                                       uint32_t *rxLen,
                                       uint32_t msTimeout);


/**
 * \brief Performs a SLPV_REQ-command.
 *
 * \param[in]       t5t             Pointer to an existing instance of the T5T component.
 * \param[in]       optionFlag          Option-flag provided by upper layer ("Request Block Security Status" for Read-alike commands).
 * \param[in]       uid                 Pointer to UID of the given Tag.
 * \param[in]       uidLen              Length of UID.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_Sleep (ptx_NativeTag_T5T_t *t5t,
                                      uint8_t optionFlag,
                                      uint8_t *uid,
                                      uint8_t uidLen,
                                      uint8_t *rx,
                                      uint32_t *rxLen,
                                      uint32_t msTimeout);

/**
 * \brief Sets / Updates the UID to be used (needs to be called after "ptx_NativeTag_T5TSelect() is called and addressed-mode shall be used").
 *
 * \param[in]       t5t             Pointer to an existing instance of the T5T component.
 * \param[in]       uid                 Pointer to UID of the given Tag.
 * \param[in]       uidLen              Length of UID.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T5T_SetUID (ptx_NativeTag_T5T_t *t5t,
                                       uint8_t *uid,
                                       uint8_t uidLen);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
