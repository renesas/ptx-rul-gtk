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
    Module      : NFC Forum Device Test Application (DTA) Discovery Handling
    File        : ptx_NFCForumDTA_Discovery.c

    Description : Contains the implementation to handle the discovery in the NFC Forum DTA. It controls the parameter
                  and sequence of a NFC Forum Discovery loop.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NFCForumDTA_Discovery.h"
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/* This is required for Windows and Linux as it is not possible to create a delay with 2 ms because of
 * the minimum possible delay of the implementation with the current OS Timer implementation.
 * Still using 2 ms causes erroneous behavior of the OS Timer.
 */
#if defined(PTX_APP_DIVERSITY_WINDOWS) || defined(PTX_APP_DIVERSITY_LINUX)
    #define PTX_NFCFORUMDTA_DISCOVERY_SLEEP_DELAY (20u)
#else
    #define PTX_NFCFORUMDTA_DISCOVERY_SLEEP_DELAY (2u)
#endif

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 *  Configure discover configuration
 *
 *  dta         NFC Forum DTA Component
 *  testCfg     Test Configuration parameters.
 *  discCfg     Discovery Configuration to edit
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
static ptx_Status_t ptx_NFCForumDTA_ConfigureDiscParameters(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg, ptx_RUL_DiscoverConfig_t *discCfg);

/*
 * Check for any system errors
 *
 *        dta     NFC Forum DTA Component
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
static ptx_Status_t ptx_NFCForumDTA_CheckSystemStatus(ptx_NFCForumDTA_t *dta);

/*
 * Check for activation or discovery of cards in poll mode
 *
 *        dta     NFC Forum DTA Component
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
static ptx_Status_t ptx_NFCForumDTA_CheckPollActivity(ptx_NFCForumDTA_t *dta);

#if !defined(PTX_DISABLE_LISTEN_MODE)
    /*
    * Check for activation in listen mode
    *
    *        dta     NFC Forum DTA Component
    *
    * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
    */
    static ptx_Status_t ptx_NFCForumDTA_CheckListenActivity(ptx_NFCForumDTA_t *dta);
#endif // PTX_DISABLE_LISTEN_MODE

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NFCForumDTA_Start_RF_Discovery(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        ptx_RUL_DiscoverConfig_t discCfg;
        memset(&discCfg, 0, sizeof(ptx_RUL_DiscoverConfig_t));

        status = ptx_NFCForumDTA_ConfigureDiscParameters(dta, testCfg, &discCfg);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_InitiateDiscovery(dta->rul, &discCfg);
        }


        if (PTX_STATUS_SUCCESS == status)
        {
            dta->State = ptx_NFCForumDTA_State_WAIT_4_ACTIVATION;
        }
    }

    return status;
}

ptx_Status_t ptx_NFCForumDTA_Deactivate(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_State_t deactivationType)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
        ptx_RUL_DeactivateParams_t params;
        switch (deactivationType)
        {
            case ptx_NFCForumDTA_State_DEACTIVATE_DISCOVER:
                params.Type = ptx_RUL_DeactivateType_Generic;
                params.State = ptx_RUL_DeactivateState_DiscoveryRfReset;
                status = ptx_RUL_Deactivate(dta->rul, &params);
                break;

            case ptx_NFCForumDTA_State_DEACTIVATE_IDLE:
                params.Type = ptx_RUL_DeactivateType_Generic;
                params.State = ptx_RUL_DeactivateState_Idle;
                status = ptx_RUL_Deactivate(dta->rul, &params);
                break;

            case ptx_NFCForumDTA_State_DEACTIVATE_SLEEP:
                params.Type = ptx_RUL_DeactivateType_Specific;
                params.State = ptx_RUL_DeactivateState_Idle;
                status = ptx_RUL_Deactivate(dta->rul, &params);
                break;

            case ptx_NFCForumDTA_State_DEACTIVATE_RELEASE:
                params.Type = ptx_RUL_DeactivateType_Specific;
                params.State = ptx_RUL_DeactivateState_Idle;
                status = ptx_RUL_Deactivate(dta->rul, &params);
                break;

            case ptx_NFCForumDTA_State_DEACTIVATE_DESLECT:
                params.Type = ptx_RUL_DeactivateType_Specific;
                params.State = ptx_RUL_DeactivateState_Poll;
                status = ptx_RUL_Deactivate(dta->rul, &params);
                if (PTX_STATUS_SUCCESS == status)
                {
                    params.Type = ptx_RUL_DeactivateType_Generic;
                    params.State = ptx_RUL_DeactivateState_Idle;
                    status = ptx_RUL_Deactivate(dta->rul, &params);
                }
                break;

            case ptx_NFCForumDTA_State_DEACTIVATE_LISTEN_SLEEP:
                params.Type = ptx_RUL_DeactivateType_Generic;
                params.State = ptx_RUL_DeactivateState_ListenSleep;
                status =  ptx_RUL_Deactivate(dta->rul, &params);
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        dta->State = ptx_NFCForumDTA_State_FINISHED;
    }

    return status;
}

ptx_Status_t ptx_NFCForumDTA_ProcessDiscovery(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {

        uint8_t discovery_timeout = 0;
        uint32_t discovery_len = 0;

        do
        {
            // Check for any system errors
            status = ptx_NFCForumDTA_CheckSystemStatus(dta);
            if ((PTX_STATUS_SUCCESS == status) && (ptx_NFCForumDTA_State_RESTART_SYSTEM == dta->State))
            {
                // Recoverable System Error detected, exit discovery loop and restart system
                break;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                // Check for Discovery or activation of tags
                status = ptx_NFCForumDTA_CheckPollActivity(dta);
            }

#if !defined(PTX_DISABLE_LISTEN_MODE)
            if (PTX_STATUS_SUCCESS == status)
            {
                // Check for Activation in Host Card Emulation mode
                status = ptx_NFCForumDTA_CheckListenActivity(dta);
            }
#endif // PTX_DISABLE_LISTEN_MODE


            if (PTX_STATUS_SUCCESS == status)
            {
                uint8_t llcp_support = 0;
                (void) ptx_RUL_LLCP_IsLlcpSupported(&dta->llcp, &llcp_support);

                if (0 != llcp_support)
                {
                    dta->State = ptx_NFCForumDTA_State_PROCESS_LLCP;
                }
            }

            if ((PTX_STATUS_SUCCESS == status) && (NULL != dta->applicationCallback))
            {
                // Execute application callback
                dta->applicationCallback(dta);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_Sleep(dta->rul, PTX_NFCFORUMDTA_DISCOVERY_SLEEP_DELAY);
                discovery_len += PTX_NFCFORUMDTA_DISCOVERY_SLEEP_DELAY;

                // Determine if discovery has timed out
                if (discovery_len > testCfg->DiscoveryTimeout)
                {
                    discovery_timeout = 1;
                }
            }

        }
        while ((PTX_STATUS_SUCCESS == status) && (0 == discovery_timeout) && (ptx_NFCForumDTA_State_WAIT_4_ACTIVATION == dta->State) && (!dta->quitDTA));


        // Handle discovery timeout
        if ((PTX_STATUS_SUCCESS == status) && (1 == discovery_timeout) && (ptx_NFCForumDTA_State_WAIT_4_ACTIVATION == dta->State))
        {
            // Deactivate to state idle if the discover has timed out
            dta->State = ptx_NFCForumDTA_State_DEACTIVATE_IDLE;
        }

    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NFCForumDTA_ConfigureDiscParameters(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg, ptx_RUL_DiscoverConfig_t *discCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg) && (NULL != discCfg))
    {
        status = PTX_STATUS_SUCCESS;

        //Poll Parameters
        discCfg->EnablePolling = 1;
        discCfg->PollMode = ptx_RUL_PollMode_NfcForum;
        discCfg->IdleTime = 100;
        discCfg->EnableDDPC = testCfg->EnableDDPC == 0 ? 0 : 1;

        discCfg->DisablePollIsoDep = 0;
        discCfg->PollIsoDepHbr = ptx_RUL_PollIsoDepHBR_Standard;

        if (dta->DTAConfiguration.CON_GB)
        {
            discCfg->DisableNfcDepProtocol = 0;
            discCfg->PollNfcDepAtrReqLen = PTX_LLCP_GENERAL_BYTES_MAX_LEN;
            ptx_RUL_LLCP_GenerateDefaultGeneralBytes(&dta->llcp, discCfg->PollNfcDepAtrReq, &discCfg->PollNfcDepAtrReqLen);
        }

        switch (dta->DTAConfiguration.CON_BITR_NFC_DEP)
        {
            case 0x00: /* Fall Through */
            case 0x01:
                discCfg->PollNfcDepHbr = ptx_RUL_PollNfcDepHBR_Standard;
                break;
            case 0x02:
                discCfg->PollNfcDepHbr = ptx_RUL_PollNfcDepHBR_212;
                break;
            case 0x03:
                discCfg->PollNfcDepHbr = ptx_RUL_PollNfcDepHBR_424;
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        if (dta->DTAConfiguration.CON_POLL_A)
        {
            discCfg->PollTypeA = 1;
            discCfg->PollTypeADeviceLimit = dta->DTAConfiguration.CON_DEVICES_LIMIT_A;
            discCfg->PollTypeAPollFrequency = 1;
        }

        if (dta->DTAConfiguration.CON_POLL_B)
        {
            discCfg->PollTypeB = 1;
            discCfg->PollTypeBDeviceLimit = dta->DTAConfiguration.CON_DEVICES_LIMIT_B;
            discCfg->PollTypeBPollFrequency = 1;
        }

        if (dta->DTAConfiguration.CON_POLL_F)
        {
            discCfg->PollTypeFDeviceLimit = dta->DTAConfiguration.CON_DEVICES_LIMIT_F;
            discCfg->PollTypeFPollFrequency = 1;

            switch (dta->DTAConfiguration.CON_BITR)
            {
                case 0x02:
                    discCfg->PollTypeF212 = 1;
                    break;
                case 0x03:
                    discCfg->PollTypeF424 = 1;
                    break;
                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }
        }

        if (dta->DTAConfiguration.CON_POLL_V)
        {
            discCfg->PollTypeV = 1;
            discCfg->PollTypeVDeviceLimit = dta->DTAConfiguration.CON_DEVICES_LIMIT_V;
            discCfg->PollTypeVPollFrequency = 1;
        }

        if (ptx_NFCForumDTA_DevType_Universal == testCfg->DeviceType)
        {
            if (dta->DTAConfiguration.CON_POLL_ACM)
            {
                discCfg->PollACM = 1;

                switch (dta->DTAConfiguration.CON_BITR_ACM)
                {
                    case 0x01:
                        discCfg->PollAcmBitRate = 0;
                        break;
                    case 0x02:
                        discCfg->PollAcmBitRate = 1;
                        break;
                    case 0x03:
                        discCfg->PollAcmBitRate = 2;
                        break;
                    default:
                        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                        break;
                }
            }

#if !defined(PTX_DISABLE_LISTEN_MODE)
            // Listen Parameters
            uint8_t listenTypeFRequestData[ptx_NFCForumDTA_LISTEN_NFC_F_RD_LEN] = ptx_NFCForumDTA_LISTEN_NFC_F_RD;
            discCfg->ListenMode = 1;
            discCfg->ListenTypeA = 1;
            discCfg->ListenTypeB = 1;
            discCfg->ListenIsoDep = 3;
            discCfg->ListenNfcDep = 1;
            discCfg->ListenNfcDepAtrRes[PTX_LLCP_DEFAULT_ATRRES_WT_INDEX] = PTX_LLCP_DEFAULT_ATRRES_WT_VALUE;
            discCfg->ListenNfcDepAtrRes[PTX_LLCP_DEFAULT_ATRRES_PP_INDEX] = PTX_LLCP_DEFAULT_ATRRES_PP_VALUE;
            discCfg->ListenNfcDepAtrResLen = PTX_LLCP_GENERAL_BYTES_MAX_LEN;
            ptx_RUL_LLCP_GenerateDefaultGeneralBytes(&dta->llcp, &discCfg->ListenNfcDepAtrRes[PTX_LLCP_DEFAULT_ATRRES_GEN_BYTES_OFFSET], &discCfg->ListenNfcDepAtrResLen);
            discCfg->ListenNfcDepAtrResLen = discCfg->ListenNfcDepAtrResLen + PTX_LLCP_DEFAULT_ATRRES_GEN_BYTES_OFFSET;

            discCfg->ListenTypeF212 = 1;
            discCfg->ListenTypeF424 = 1;
            discCfg->ListenTypeFRequestDataLen = ptx_NFCForumDTA_LISTEN_NFC_F_RD_LEN;
            memcpy(discCfg->ListenTypeFRequestData, listenTypeFRequestData, ptx_NFCForumDTA_LISTEN_NFC_F_RD_LEN);

            if (dta->DTAConfiguration.CON_LISTEN_DEP_F)
            {
                discCfg->ListenNfcDep = 3;
            }
#endif // PTX_DISABLE_LISTEN_MODE
        }
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_CheckSystemStatus(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
        // Check current discovery Status
        ptx_RUL_Status_Params_t statusParams;
        memset(&statusParams, 0, sizeof(ptx_RUL_Status_Params_t));
        statusParams.Type = ptx_RUL_StatusType_System;

        // Get discovery status
        status = ptx_RUL_GetStatusInfo(dta->rul, &statusParams);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = PTX_STATUS_SUCCESS;

            /* System-Errors-/-Warnings are critical and needs to be resolved at integration-/application-level.
             * There can be multiple root causes those errors/warnings, but in most cases they are related
             * the Hardware-environment (e.g. PCB, component issues etc.).
             */
            if (ptx_RUL_StatusInfoSystem_ErrorOvercurrent == statusParams.Info.System)
            {
                PTX_LOG_I(&(dta->rul->Log), "WARNING - Overcurrent Limiter activated!\n");
                dta->quitDTA = 1u;
            }

            if (ptx_RUL_StatusInfoSystem_ErrorTemperature == statusParams.Info.System)
            {
                PTX_LOG_I(&(dta->rul->Log), "WARNING - Temperature Limiter activated!\n");
                dta->State = ptx_NFCForumDTA_State_RESTART_SYSTEM;
            }

            if (ptx_RUL_StatusInfoSystem_ErrorOutOfMemory == statusParams.Info.System)
            {
                PTX_LOG_I(&(dta->rul->Log), "WARNING - Out of Memory!\n");
                dta->State = ptx_NFCForumDTA_State_RESTART_SYSTEM;
            }

            if (ptx_RUL_StatusInfoSystem_NSCReset == statusParams.Info.System)
            {
                PTX_LOG_I(&(dta->rul->Log), "WARNING - Unexpected System Reset - exiting Application!\n");
                dta->quitDTA = 1u;
            }
        }
        else
        {
            dta->State = ptx_NFCForumDTA_State_FINISHED;
        }

    }


    return status;
}

static ptx_Status_t ptx_NFCForumDTA_CheckPollActivity(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
        // Check current discovery Status
        ptx_RUL_Status_Params_t statusParams;
        memset(&statusParams, 0, sizeof(ptx_RUL_Status_Params_t));
        statusParams.Type = ptx_RUL_StatusType_Discover;

        // Get discovery status
        status = ptx_RUL_GetStatusInfo(dta->rul, &statusParams);

        if (PTX_STATUS_SUCCESS == status)
        {
            if ((statusParams.Info.Discover == ptx_RUL_StatusInfoDiscover_CardsAvailable) ||
                (statusParams.Info.Discover == ptx_RUL_StatusInfoDiscover_MultiProtocol))
            {
                // Proceed to select procedure if multiple cards are detected
                dta->State = ptx_NFCForumDTA_State_POLL_SELECT_DEVICE;
            }
            else if ((statusParams.Info.Discover == ptx_RUL_StatusInfoDiscover_Active) && (dta->rul->CardRegistry.CardsCnt > 0))
            {
                if (dta->rul->CardRegistry.Cards[0].State == ptx_RUL_CardRfState_Active)
                {
                    // A single card was activated. Proceed with data exchange
                    dta->State = ptx_NFCForumDTA_State_POLL_DATA_EXCHANGE;
                }
                else
                {
                    // Else proceed with activation of the card
                    dta->State = ptx_NFCForumDTA_State_POLL_SELECT_DEVICE;
                }
            }
        }
        else
        {
            dta->State = ptx_NFCForumDTA_State_FINISHED;
        }

    }


    return status;
}

#if !defined(PTX_DISABLE_LISTEN_MODE)
static ptx_Status_t ptx_NFCForumDTA_CheckListenActivity(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (PTX_COMP_CHECK(&dta->hce, Comp_HCE)))
    {
        // Check for new HCE vents
        ptx_RUL_HCE_EventRecord_t *record;
        status = ptx_RUL_HCE_GetEvent(&dta->hce, &record);

        if ((PTX_STATUS_SUCCESS == status) && (RUL_HCE_EventType_Activated == record->EventID))
        {
            // Proceed to Listen data exchange if we have been activated in listen mode
            dta->State = ptx_NFCForumDTA_State_LISTEN_PROCESS_EVENTS;
        }

        // Handle the possibility of no HCE Event
        if (PTX_STATUS_E_GEN_NOT_FOUND == status)
        {
            status = PTX_STATUS_SUCCESS;
        }


    }

    return status;
}
#endif // PTX_DISABLE_LISTEN_MODE