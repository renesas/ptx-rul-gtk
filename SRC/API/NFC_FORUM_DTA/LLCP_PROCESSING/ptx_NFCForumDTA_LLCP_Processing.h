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
    Module      : NFC Forum Device Test Application (DTA) LLCP Processing
    File        : ptx_NFCForumDTA_LLCP_Processing.h

    Description : Contains the implementation to handle LLCP in the NFC Forum DTA. It controls the processing if the
                  NFC Forum DTA is running in LLCP Mode for P2P communication.
*/

/**
 * \addtogroup DTA
 * @{
 */
#if !defined(PTX_NFCFORUM_DTA_LLCP_PROCESSING_)
#define PTX_NFCFORUM_DTA_LLCP_PROCESSING_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NFCForumDTA_Configuration.h"
#include "ptx_RUL_LLCP.h"

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
 * \brief The state machine states for the connection less LLCP scenario.
 */
typedef enum ptx_RUL_LLCP_DTA_ClScenarioState
{
    CL_SCENARIO_STATE_CHECK_TEST_START_CMD, /**< Connection less DTA state to connect via SAP number. */
    CL_SCENARIO_STATE_SEND_SNL_REQUEST,     /**< Connection less DTA state to send a service name lookup (SNL) request. */
    CL_SCENARIO_STATE_RECEIVE_UI_PDU,       /**< Connection less DTA state to receive an unnumbered information PDU. */
    CL_SCENARIO_STATE_SEND_UI_PDU           /**< Connection less DTA state to send an unnumbered information PDU. */
} ptx_RUL_LLCP_DTA_ClScenarioState_t;

/**
 * \brief The state machine states for the connection oriented LLCP scenario.
 */
typedef enum ptx_RUL_LLCP_DTA_CoScenarioState
{
    CO_SCENARIO_STATE_CONNECT_VIA_SAP,  /**< Connection oriented DTA state to connect via SAP number. */
    CO_SCENARIO_STATE_CONNECT_VIA_NAME, /**< Connection oriented DTA state to connect via SAP name. */
    CO_SCENARIO_STATE_CONNECT_VIA_SNL,  /**< Connection oriented DTA state to connect via service name lookup. */
    CO_SCENARIO_STATE_CONNECT_FAIL,     /**< Connection oriented DTA state in case of connection failure. */
    CO_SCENARIO_STATE_RECEIVE_I_PDU,    /**< Connection oriented DTA state to receive an I PDU. */
    CO_SCENARIO_STATE_SEND_I_PDU        /**< Connection oriented DTA state to send an I PDU. */
} ptx_RUL_LLCP_DTA_CoScenarioState_t;

/**
 * \brief Enum representing the LLCP test pattern numbers.
 */
typedef enum ptx_RUL_LLCP_DTA_TestProfile
{
    TestProfile_h1200,                  /**< Test profile for DTA pattern 0x1200 */
    TestProfile_h1240,                  /**< Test profile for DTA pattern 0x1240 */
    TestProfile_h1280,                  /**< Test profile for DTA pattern 0x1280 */
    TestProfile_MAX                     /**< Test profile maximum */
} ptx_RUL_LLCP_DTA_TestProfile_t;

/**
 * \brief RUL LLCP DTA main structure
 */
typedef struct ptx_RUL_LLCP_DTA
{
    ptx_NFCForumDTA_t                  *Dta;                /**< Pointer to the NFC Forum DTA. */
    ptx_RUL_LLCP_t                     *Llcp;               /**< Pointer to the LLCP component. */
    ptx_RUL_LLCP_Link_t                *ClInbound;          /**< Connection less inbound logical link. */
    ptx_RUL_LLCP_Link_t                *CoInbound;          /**< Connection oriented inbound logical link. */
    ptx_RUL_LLCP_Link_t                *ClOutbound;         /**< Connection less outbound logical link. */
    ptx_RUL_LLCP_Link_t                *CoOutbound;         /**< Connection oriented outbound logical link. */
    ptx_RUL_LLCP_DTA_CoScenarioState_t  CoScenarioState;    /**< Connection oriented scenario state. */
    ptx_RUL_LLCP_DTA_ClScenarioState_t  ClScenarioState;    /**< Connection less scenario state. */
    uint8_t                             SetupBySap;         /**< Flag indicating that connection setup shalll happen via SAP number. */
    uint8_t                             SetupByName;        /**< Flag indicating that connection setup shalll happen via SAP name. */
    uint8_t                             SetupBySnl;         /**< Flag indicating that connection setup shalll happen via service name lookup. */
    uint8_t                             QuitScenario;       /**< Flag indicating to quit the DTA processing loop. */
    uint8_t                             OutBoundSapAddress; /**< Temporary storage for the outbound SAP address. */
    ptx_Status_t                        LlcpStatus;         /**< Status of the last LLCP operation. */
} ptx_RUL_LLCP_DTA_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Main entry point for the LLCP NFC Forum DTA.
 *
 * \param[in]       dta     NFC Forum DTA Component
 * \param[in]       testCfg Test Configuration parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
ptx_Status_t ptx_NFCForumDTA_RunLlcpTestOperation(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */