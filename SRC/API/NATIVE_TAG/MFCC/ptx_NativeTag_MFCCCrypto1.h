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
    Module      : MFCC Crypto1 Abstraction Layer
    File        : ptx_NativeTag_MFCCCrypto1.h

    Description : Contains a shell of the Crypto1 abstraction layer. In order to use this component,
                  a third-party Crypto-1 implementation is required. A suitable implementation
                  is referenced in the RUL User Manual.
*/
/**
 * \addtogroup MFCC
 *
 * @{
 */

#if !defined(PTX_NATIVETAG_MFCCCRYPTO1_H)
#define PTX_NATIVETAG_MFCCCRYPTO1_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stddef.h>

#include "ptx_Components.h"
#include "ptx_Status.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */



/**
 * \brief Main Component Struture
 */
typedef struct ptx_NativeTag_MFCCCrypto1
{
    ptx_Comps_t                                     CompId;                     /**< Component ID */
    void                                            *CryptoState;               /**< Custom Crypto State (potentially 3rd party). */

} ptx_NativeTag_MFCCCrypto1_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[in]       crypto1Params   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_MFCCCrypto1_Open (ptx_NativeTag_MFCCCrypto1_t *crypto1Params);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       crypto1Params   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_MFCCCrypto1_Close (ptx_NativeTag_MFCCCrypto1_t *crypto1Params);

/**
 * \brief Initializes the internal states of the Crypto1-engine with a given key to perform a AUTHENT-command.
 *
 * \param[in]   crypto1Params       Implementation dependent Crypto1-engine parameters.
 * \param[in]   key                 Key used for Authentication-command.
 *
 */
void ptx_NativeTag_MFCCCrypto1_Set (ptx_NativeTag_MFCCCrypto1_t *crypto1Params, uint64_t key);

/**
 * \brief Resets the internal states of the Crypto1-engine.
 *
 * \param[in]   crypto1Params       Implementation dependent Crypto1-engine parameters.
 *
 */
void ptx_NativeTag_MFCCCrypto1_Reset (ptx_NativeTag_MFCCCrypto1_t *crypto1Params);

/**
 * \brief Encrypts/Decrypts a Bit.
 *
 * \param[in]   crypto1Params       Implementation dependent Crypto1-engine parameters.
 * \param[in]   bitValue            Bit value.
 * \param[in]   isEncrypted         Indicator if bit is already encrypted or not.
 *
 * \return Encrypted / Decrypted bit value.
 */
uint8_t ptx_NativeTag_MFCCCrypto1_Bit(ptx_NativeTag_MFCCCrypto1_t *crypto1Params, uint8_t bitValue, int isEncrypted);

/**
 * \brief Encrypts/Decrypts a Byte.
 *
 * \param[in]   crypto1Params       Implementation dependent Crypto1-engine parameters.
 * \param[in]   byteValue           Byte value.
 * \param[in]   isEncrypted         Indicator if byte is already encrypted or not.
 *
 * \return Encrypted / Decrypted byte value.
 */
uint8_t ptx_NativeTag_MFCCCrypto1_Byte(ptx_NativeTag_MFCCCrypto1_t *crypto1Params, uint8_t byteValue, int isEncrypted);

/**
 * \brief Encrypts/Decrypts a Word.
 *
 * \param[in]   crypto1Params       Implementation dependent Crypto1-engine parameters.
 * \param[in]   wordValue           Word value.
 * \param[in]   isEncrypted         Indicator if word is already encrypted or not.
 *
 * \return Encrypted / Decrypted word value.
 */
uint32_t ptx_NativeTag_MFCCCrypto1_Word(ptx_NativeTag_MFCCCrypto1_t *crypto1Params, uint32_t wordValue, int isEncrypted);

/**
 * \brief Return single bit of non-linear filter function.
 *
 * \param[in]   crypto1Params       Implementation dependent Crypto1-engine parameters.
 *
 * \return Output of non-linear filter function.
 */
int ptx_NativeTag_MFCCCrypto1_Filter(ptx_NativeTag_MFCCCrypto1_t *crypto1Params);

/**
 * \brief Shifts/clocks value through internal PRNG-register by n-steps.
 *
 * \param[in]   crypto1Params       Implementation dependent Crypto1-engine parameters.
 * \param[in]   value               Value to be shifted.
 * \param[in]   nrClocks            Number of steps.
 *
 * \return PRNG output value.
 */
uint32_t ptx_NativeTag_MFCCCrypto1_ShiftPRNG(ptx_NativeTag_MFCCCrypto1_t *crypto1Params, uint32_t value, uint32_t nrClocks);

/**
 * \brief Returns a random byte.
 *
 * \param[in]   crypto1Params       Implementation dependent Crypto1-engine parameters.
 *
 * \return Random byte value.
 */
uint8_t ptx_NativeTag_MFCCCrypto1_GetRandomByte(ptx_NativeTag_MFCCCrypto1_t *crypto1Params);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
