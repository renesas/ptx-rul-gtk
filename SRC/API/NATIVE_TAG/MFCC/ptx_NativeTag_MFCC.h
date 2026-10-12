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
    Module      : API for interfacing with a MFCC card
    File        : ptx_NativeTag_MFCC.h

    Description : This contains an API that can be used to interface with MFCC cards.
                  In order to use this component, a third-party Crypto-1 implementation is
                  required and should be implemented in the ptx_NativeTag_MFCCCrypto1.c file.
*/

/**
 * \addtogroup MFCC
 *
 * @{
 */

#if !defined(PTX_NATIVETAG_MFCC_H)
#define PTX_NATIVETAG_MFCC_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NativeTag_MFCCCrypto1.h"
#include "ptx_Components.h"
#include "ptx_Status.h"
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
 * \name MFCC Tag specific size definitions.
 * @{
 */
#define PTX_MFCC_BITS_EXCHANGE_BUFFER_SIZE         (uint8_t)24u         /**< Min. buffer size for Bit-Exchange function. */
/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */
/**
 * \brief Mifare Compatibility Mode Enable / Disable
 */
typedef enum ptx_NativeTag_MFCC_Mode
{
    MFCC_Mode_Enable,       /**< Enable compatibility */
    MFCC_Mode_Disable,      /**< Disable compatibility */

} ptx_NativeTag_MFCC_Mode_t;

/**
 * \brief Mifare Compatibility Key Type (used for Authentication)
 */
typedef enum ptx_NativeTag_MFCC_KeyType
{
    MFCC_Key_Type_A,        /**< MFCC Key Type A */
    MFCC_Key_Type_B,        /**< MFCC Key Type B */

} ptx_NativeTag_MFCC_KeyType_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_NativeTag_MFCC
{
    ptx_Comps_t                         CompId;                                             /**< Component Id */
    ptx_RUL_t                           *Rul;                                               /**< Pointer to the initialized RUL component. */
    ptx_NativeTag_MFCCCrypto1_t         Crypto1Params;                                      /**< Crypto1 specific Parameters (dependent on implementation) */
    ptx_RUL_BitsExchangeParams_t        RxData;                                             /**< Container for Rx-Data and -Parity. */
    uint8_t                             RxByteBuffer[PTX_MFCC_BITS_EXCHANGE_BUFFER_SIZE];   /**< Rx-Buffer to store received Bytes. */
    uint8_t                             RxBitBuffer[PTX_MFCC_BITS_EXCHANGE_BUFFER_SIZE];    /**< Rx-Buffer to store received Bits. */
    ptx_RUL_BitsExchangeParams_t        TxData;                                             /**< Container for Tx-Data and -Parity. */
    uint8_t                             TxByteBuffer[PTX_MFCC_BITS_EXCHANGE_BUFFER_SIZE];   /**< Tx-Buffer to send Bytes. */
    uint8_t                             TxBitBuffer[PTX_MFCC_BITS_EXCHANGE_BUFFER_SIZE];    /**< Tx-Buffer to send Bits. */
    uint8_t                             IsCrypted;                                          /**< Flag whether encryption is used. */
} ptx_NativeTag_MFCC_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_NativeTag_MFCC_InitParams
{
    ptx_RUL_t                           *Rul;                                               /**< Pointer to the initialized RUL component. */
    void                                *UserContextCrypto1;                                /**< Custom/User-specific Crypto State (potentially 3rd party). */

} ptx_NativeTag_MFCC_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[in]       mfcc            Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_MFCC_Open (ptx_NativeTag_MFCC_t *mfcc, ptx_NativeTag_MFCC_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       mfcc            Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_MFCC_Close (ptx_NativeTag_MFCC_t *mfcc);

/**
 * \brief Enables or disables the Mifare Compatibility Mode.
 *
 * \param[in]   mfcc            Pointer to an existing instance of the Mifare Compatibility.
 * \param[in]   enableMode      Enables / Disables the MFCC-Mode.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_MFCC_Enable (ptx_NativeTag_MFCC_t *mfcc, ptx_NativeTag_MFCC_Mode_t enableMode);

/**
 * \brief Internal reset of component state.
 *
 * \param[in]   mfcc            Pointer to an existing instance of the Mifare Compatibility.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_MFCC_ResetState (ptx_NativeTag_MFCC_t *mfcc);

/**
 * \brief Performs a Mifare AUTHENTICATE-command with Key-A or -B.
 *
 * \param[in]   mfcc            Pointer to an existing instance of the Mifare Compatibility.
 * \param[in]   keyType         Type of Key (A or B).
 * \param[in]   address         Address used for operation.
 * \param[in]   key             Pointer to Key.
 * \param[in]   keyLen          Length of Key (must be 6 Bytes).
 * \param[in]   uid             Pointer to 4-byte part of UID.
 * \param[in]   uidLen          Length of UID (must be 4 Bytes).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_MFCC_Authenticate (ptx_NativeTag_MFCC_t *mfcc,
                                              ptx_NativeTag_MFCC_KeyType_t keyType,
                                              uint8_t address,
                                              uint8_t *key,
                                              uint8_t keyLen,
                                              uint8_t *uid,
                                              uint8_t uidLen);

/**
 * \brief Performs a Mifare READ-command.
 *
 * \param[in]     mfcc          Pointer to an existing instance of the Mifare Compatibility.
 * \param[in]     address       Address used for operation.
 * \param[out]    rx            Pointer to Rx-buffer where the received data from the card shall be stored (min. 20 Bytes!).
 * \param[in,out] rxLen         Size of Rx-buffer (in), Length of the received data (out).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_MFCC_Read (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint8_t *rx, uint32_t *rxLen);

/**
 * \brief Performs a Mifare WRITE-command.
 *
 * \param[in]     mfcc          Pointer to an existing instance of the Mifare Compatibility.
 * \param[in]     address       Address used for operation.
 * \param[in]     blockData     Pointer to block-data to be written.
 * \param[in]     blockDataLen  Length of block-data (must be 16 Bytes).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_MFCC_Write (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint8_t *blockData, uint8_t blockDataLen);

/**
 * \brief Performs a Mifare INCREMENT-command.
 *
 * \param[in]     mfcc          Pointer to an existing instance of the Mifare Compatibility.
 * \param[in]     address       Address used for operation.
 * \param[in]     value         4-Byte Operand / Value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_MFCC_Increment (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint32_t value);

/**
 * \brief Performs a Mifare DECREMENT-command.
 *
 * \param[in]     mfcc          Pointer to an existing instance of the Mifare Compatibility.
 * \param[in]     address       Address used for operation.
 * \param[in]     value         4-Byte Operand / Value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_MFCC_Decrement (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint32_t value);

/**
 * \brief Performs a Mifare TRANSFER-command.
 *
 * \param[in]     mfcc          Pointer to an existing instance of the Mifare Compatibility.
 * \param[in]     address       Address used for operation.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_MFCC_Transfer (ptx_NativeTag_MFCC_t *mfcc, uint8_t address);

/**
 * \brief Performs a Mifare RESTORE-command.
 *
 * \param[in]     mfcc          Pointer to an existing instance of the Mifare Compatibility.
 * \param[in]     address       Address used for operation.
 * \param[in]     value         4-Byte Operand / Value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_MFCC_Restore (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint32_t value);

/**
 * \brief Converts a given Value into the Mifare Classic Value-Block format.
 *
 * \param[in]     mfcc          Pointer to an existing instance of the Mifare Compatibility.
 * \param[in]     address       Address used for operation.
 * \param[in]     value         4-Byte Operand / Value.
 * \param[in]     buffer        Buffer to store formatted value block.
 * \param[in]     bufferSize    Size of Buffer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_MFCC_GetFormattedValueBlock (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint32_t value, uint8_t *buffer, uint8_t bufferSize);

/**
 * \brief Performs an ISO/IEC 14443-3A HALT-command
 *
 * \param[in]     mfcc          Pointer to an existing instance of the Mifare Compatibility.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_MFCC_Halt (ptx_NativeTag_MFCC_t *mfcc);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
