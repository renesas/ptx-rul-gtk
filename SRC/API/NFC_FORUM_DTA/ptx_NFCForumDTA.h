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
    Module      : NFC Forum Device Test Application (DTA)
    File        : ptx_NFCForumDTA.h

    Description : NFC Forum Device Test Application (DTA) contains the implementation of the Test Application
                  defined by NFC Forum, which can be used for NFC Forum Compliance Testing.
*/

/**
 * \addtogroup DTA
 * @{
 */

#if !defined(PTX_NFCFORUMDTA_H)
#define PTX_NFCFORUMDTA_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>

#include "ptx_Status.h"
#include "ptx_Components.h"

#include "ptx_NFCForumDTA_Configuration.h"

#include "ptx_NFCForumDTA_DataExchange.h"
#include "ptx_NFCForumDTA_Discovery.h"
#include "ptx_NFCForumDTA_Select_Card.h"

#include "ptx_NFCForumDTA_LLCP_Processing.h"
#include "ptx_NFCForumDTA_SNEP_Processing.h"

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
 * \brief Returns the current NFC Forum DTA Version
 *
 * \param[out]    version     Version of the NFC Forum DTA implementation
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return Others                                 See \ref ptx_Status_t.
*/
ptx_Status_t ptx_NFCForumDTA_GetVersion(uint8_t *version);

/**
 * \brief NFC Forum DTA Component Initialisation.
 *
 * Initializes the component, and sets up all internal pointers, etc.
 * Performs the initialization of the hardware
 * Including FW-Download, RF-Configuration, etc.
 *
 * \param[in,out]   dta         Pointer to DTA component.
 * \param[in]       initParams  Pointer to initialisation parameters.
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER     Invalid input parameter.
 * \return Others                                 See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NFCForumDTA_Init(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_InitParams_t *initParams);

/**
 * \brief NFC Forum DTA Component Deinitialization.
 *
 * \param[in]   dta     Pointer to NFC Forum DTA component.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 */
ptx_Status_t ptx_NFCForumDTA_DeInit(ptx_NFCForumDTA_t *dta);

/**
 * \brief Start NFC Forum DTA request
 *
 * Begins an NFC Forum DTA Execution using the current configuration
 *
 * \param[in]       dta         Pointer to NFC Forum DTA component.
 * \param[in]       testCfg     Pointer to test parameters.
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER     Invalid input parameter.
 * \return Others                                 See \ref ptx_Status_t.
*/
ptx_Status_t ptx_NFCForumDTA_StartDTARequest(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);

/**
 * \brief Repeats NFC Forum DTA Request for repetitions
 *
 * Repeats the DTA Execution the given number of times
 * If endless is requested, the DTA Request will be continuously executed.
 * The endless execution can be exited by setting quitEndlessExecution to 1 using the application callback.
 *
 * \param[in]       dta         Pointer to DTA component.
 * \param[in]       testCfg     Pointer to test parameters.
 * \param[in]       repetitions Number of times the test shall be repeated.
 * \param[in]       endless     Enables endless execution of the DTA Request.
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return Others                                 See \ref ptx_Status_t.
*/
ptx_Status_t ptx_NFCForumDTA_RepeatTest(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg, uint32_t repetitions, uint8_t endless);

/**
 * \brief Registers the additional checks callback
 *
 * The callback can be used to preemptively exit the dta execution
 *
 * \param[in]       dta         Pointer to DTA Component
 * \param[in]       cb          Function pointer to callback to register
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER     Invalid input parameter.
*/
ptx_Status_t ptx_NFCForumDTA_RegisterApplicationCallback(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_ApplicationCallback_t cb);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

