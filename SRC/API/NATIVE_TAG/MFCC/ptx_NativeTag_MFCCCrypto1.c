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
    Module      : MFCC Crypto1 Abstraction Layer
    File        : ptx_NativeTag_MFCCCrypto1.c

    Description : Contains a shell of the Crypto1 abstraction layer. In order to use this component,
                  a third-party Crypto-1 implementation is required. A suitable implementation
                  is referenced in the RUL User Manual.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NativeTag_MFCCCrypto1.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NativeTag_MFCCCrypto1_Open (ptx_NativeTag_MFCCCrypto1_t *crypto1Params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != crypto1Params))
    {
        memset(crypto1Params, 0, sizeof(ptx_NativeTag_MFCCCrypto1_t));

        crypto1Params->CompId = Comp_NativeTag_MFCC;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NativeTag_MFCCCrypto1_Close (ptx_NativeTag_MFCCCrypto1_t *crypto1Params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(crypto1Params, Comp_NativeTag_MFCC))
    {
        memset(crypto1Params, 0, sizeof(ptx_NativeTag_MFCCCrypto1_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

void ptx_NativeTag_MFCCCrypto1_Set (ptx_NativeTag_MFCCCrypto1_t *crypto1Params, uint64_t key)
{
    /* Concrete implementation of Crypto1-Cipher needs to be done. */
    if (NULL != crypto1Params)
    {
        PTX_UNUSED_PARAMETER(key);
    }
}

void ptx_NativeTag_MFCCCrypto1_Reset (ptx_NativeTag_MFCCCrypto1_t *crypto1Params)
{
    /* Concrete implementation of Crypto1-Cipher needs to be done. */
    if (NULL != crypto1Params)
    {

    }
}

uint8_t ptx_NativeTag_MFCCCrypto1_Bit(ptx_NativeTag_MFCCCrypto1_t *crypto1Params, uint8_t bitValue, int isEncrypted)
{
    /* Concrete implementation of Crypto1-Cipher needs to be done. */
    uint8_t ret = 0;

    if (NULL != crypto1Params)
    {
        PTX_UNUSED_PARAMETER(bitValue);
        PTX_UNUSED_PARAMETER(isEncrypted);
    }
    else
    {
        ret = 1;
    }

    return ret;
}

uint8_t ptx_NativeTag_MFCCCrypto1_Byte(ptx_NativeTag_MFCCCrypto1_t *crypto1Params, uint8_t byteValue, int isEncrypted)
{
    /* Concrete implementation of Crypto1-Cipher needs to be done. */
    uint8_t ret = 0;

    if (NULL != crypto1Params)
    {
        PTX_UNUSED_PARAMETER(byteValue);
        PTX_UNUSED_PARAMETER(isEncrypted);
    }
    else
    {
        ret = 1;
    }

    return ret;
}

uint32_t ptx_NativeTag_MFCCCrypto1_Word(ptx_NativeTag_MFCCCrypto1_t *crypto1Params, uint32_t wordValue, int isEncrypted)
{
    /* Concrete implementation of Crypto1-Cipher needs to be done. */
    uint32_t ret = 0;

    if (NULL != crypto1Params)
    {
        PTX_UNUSED_PARAMETER(wordValue);
        PTX_UNUSED_PARAMETER(isEncrypted);
    }
    else
    {
        ret = 1;
    }

    return ret;
}

int ptx_NativeTag_MFCCCrypto1_Filter(ptx_NativeTag_MFCCCrypto1_t *crypto1Params)
{
    /* Concrete implementation of Crypto1-Cipher needs to be done. */
    int filter_output = 0;

    if (NULL != crypto1Params)
    {

    }

    return filter_output;
}

uint32_t ptx_NativeTag_MFCCCrypto1_ShiftPRNG(ptx_NativeTag_MFCCCrypto1_t *crypto1Params, uint32_t value, uint32_t nrClocks)
{
    /* Concrete implementation of Crypto1-Cipher needs to be done. */
    uint32_t ret = 0;

    if (NULL != crypto1Params)
    {
        PTX_UNUSED_PARAMETER(value);
        PTX_UNUSED_PARAMETER(nrClocks);
    }
    else
    {
        ret = 1;
    }
    return ret;
}

uint8_t ptx_NativeTag_MFCCCrypto1_GetRandomByte(ptx_NativeTag_MFCCCrypto1_t *crypto1Params)
{
    /* Concrete implementation of Crypto1-Cipher needs to be done. */
    uint8_t ret = 0;

    if (NULL != crypto1Params)
    {

    }
    else
    {
        ret = 1;
    }

    return ret;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
