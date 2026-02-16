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
    Module      : NFC Forum Device Test Application (DTA) T5T Processing
    File        : ptx_NfcForumDTA_T5T_Processing.h

    Description : Contains the implementation to handle the processing of T5T in the NFC Forum DTA. It controls the
                  processing if the NFC Forum DTA is communicating with a card in T5T Mode.
*/

/**
 * \addtogroup DTA
 * @{
 */

#if !defined(PTX_NFC_FORUM_DTA_T5T_PROCESSING_H)
#define PTX_NFC_FORUM_DTA_T5T_PROCESSING_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>

#include "ptx_Status.h"
#include "ptx_Components.h"

#include "ptx_NFCForumDTA_Configuration.h"

#include "ptx_RUL.h"

#include "ptx_NSC_RfSetParameter.h"

#include "ptx_NDEF.h"

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
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief T5T Processing
 * Process NFC Forum DTA T5T specific test scenarios depending on provided pattern number
 *
 * \param[in]        dta               Pointer to initialized dta component.
 * \param[in]        testCfg           Test configuration
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
ptx_Status_t ptx_NFCForumDTA_ProcessT5T(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);

/**
 * \brief T5T Sleep Select procedure
 *
 * \param[in]        dta               Pointer to initialized dta component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
ptx_Status_t ptx_NFCForumDTA_T5TSleepSelect(ptx_NFCForumDTA_t *dta);

/**
 * \brief T5T Mode selection based on Pattern Number
 *
 * \param[in]        dta               Pointer to initialized dta component.
 * \param[in]        testCfg           Test configuration
 * \param[out]       ndefInitParams    Ndef Initialization paramters which will be modified based on the provided test configuration
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
ptx_Status_t ptx_NFCForumDTA_T5TMode(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg, ptx_NDEF_InitParams_t *ndefInitParams);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

