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
    Module      : NFC Forum Device Test Application (DTA) SNEP Processing
    File        : ptx_NFCForumDTA_SNEP_Processing.h

    Description : Contains the implementation to handle SNEP in the NFC Forum DTA. It controls the processing if the
                  NFC Forum DTA is running in SNEP Mode for P2P communication.
*/

/**
 * \addtogroup DTA
 * @{
 */
#if !defined(PTX_NFCFORUM_DTA_SNEP_PROCESSING_)
#define PTX_NFCFORUM_DTA_SNEP_PROCESSING_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NFCForumDTA_Configuration.h"
#include "ptx_RUL_SnepServer.h"
#include "ptx_RUL_SnepClient.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
#define PTX_SNEP_DTA_NORMAL_NDEF_RECORD_LENGTH  37      /**< Length of the normal SNEP DTA NDEF record. */
#define PTX_SNEP_DTA_LONG_NDEF_RECORD_LENGTH    502     /**< Length of the long SNEP DTA NDEF record. */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Runs the SNEP DTA and distinguishes between SNEP Client and SNEP Server test patterns.
 *
 * \param[in]       dta     NFC Forum DTA Component
 * \param[in]       testCfg Test Configuration parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
ptx_Status_t ptx_NFCForumDTA_RunSnepTestOperation(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

