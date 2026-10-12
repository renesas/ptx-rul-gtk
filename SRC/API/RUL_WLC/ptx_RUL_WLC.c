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
    Module      : RUL Wireless Charging (WLC)
    File        : ptx_RUL_WLC.c

    Description : RUL Wireless Charging (WLC) implements NFC WLC capabilities with the PTX2xxW chip. This module grants
                  NFC Forum compliant WLC-P functionality with compatibility to all compliant WLC-L hardware. Additional
                  it is possible to use the API to create and define custom WLC protocols.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdbool.h>
#include "ptx_RUL_WLC.h"
#include "ptx_RUL.h"
#include "ptx_HIP.h"
#include "ptx_RUL_WLC_Proprietary.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_WLC_PERCENT_FRACTION                                (100u)
#define PTX_WLC_LUT_IDX_OFFSET                                  (10)
#define PTX_WLC_PRECISION_SHIFT                                 (24)
#define PTX_WLC_CTRL_CNTR_RESET_VALUE                           (0xFF)

#define PTX_WLC_TX_MULTIPLIER_MAX                               (177u)
#define PTX_WLC_CHARGING_TIMEOUT_SAFETY_MARGIN_PERCENT          (10u)
#define PTX_WLC_CHARGING_TIMEOUT_SAFETY_MARGIN_TOLERANCE        (5u)
#define PTX_WLC_STOP_DETECTION_SUPPORTED                        (0u)
#define PTX_WLC_SHIFT_DIVBY4                                    (2u)
#define PTX_WLC_POLLER_MIN_STEP_INC                             (0x01)
#define PTX_WLC_POLLER_MIN_STEP_DEC                             (0xFF)

#define PTX_WLC_SLEEP_PRESENCE_CHECK_DURATION_TYPE_A            (5u)
#define PTX_WLC_SLEEP_PRESENCE_CHECK_DURATION_TYPE_B            (5u)
#define PTX_WLC_SLEEP_PRESENCE_CHECK_DURATION_TYPE_F            (20u)
#define PTX_WLC_SLEEP_PRESENCE_CHECK_DURATION_TYPE_V            (5u)
#define PTX_WLC_SLEEP_PRESENCE_CHECK_TIMEOUT_FACTOR             (10u)

/**
 * The lookup table is required for NFC Forum compliant power regulation.
 * It avoids the need for square and square root calculations.
 * s_LUT[] = sqrt(1+power_adj/100) * 256) where power_adj is increased by 5 in a range of -50 - 100
 */
static const uint16_t s_LUT[] = { 181, 189, 198, 206, 214, 221, 228, 236, 242, 249, 256, 262, 268, 274, 280,
                                 286, 291, 297, 302, 308, 313, 318, 323, 328, 333, 338, 343, 348, 352, 357, 362 };

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/** Internal Initialization related functions. */
static ptx_Status_t ptx_RUL_WLC_CheckInitParamsCoherence(ptx_RUL_WLC_InitParams_t const *initParams);
static ptx_Status_t ptx_RUL_WLC_ResetListenerParameter(ptx_RUL_WLC_t *ctx);
static ptx_Status_t ptx_RUL_WLC_SelectCompatibleProtocol(uint8_t supportedProtocols, ptx_RUL_CardProtocolType_t *selectedProtocol);

/** Internal NDEF Read/Write related functions. */
static ptx_Status_t ptx_RUL_WLC_ReadNdefMessage(ptx_RUL_WLC_t *ctx, ptx_NDEFRecord_t *records, uint32_t *recordsLen);
static ptx_Status_t ptx_RUL_WLC_WriteNdefMessage(ptx_RUL_WLC_t *ctx, ptx_NDEFRecord_t *records, uint32_t recordsLen);
static ptx_Status_t ptx_RUL_WLC_ReadWLCCAPMessage(ptx_RUL_WLC_t *ctx);
static ptx_Status_t ptx_RUL_WLC_ParseWLCCAPRecord(ptx_RUL_WLC_t *ctx);
static ptx_Status_t ptx_RUL_WLC_ParseWLCCAPRecordWlcMode(ptx_RUL_WLC_t *ctx);
static ptx_Status_t ptx_RUL_WLC_ParseWLCCAPRecordNegotiatedWlcMode(ptx_RUL_WLC_t *ctx);
static ptx_Status_t ptx_RUL_WLC_ReadWLCCTLMessage(ptx_RUL_WLC_t *ctx);
static ptx_Status_t ptx_RUL_WLC_ParseWLCCTLRecord(ptx_RUL_WLC_t *ctx);

/** Internal WPT cycle related functions. */
static uint8_t ptx_RUL_WLC_TxMultiplierToPercent(uint8_t reg);
static ptx_Status_t ptx_RUL_WLC_ComputeRequiredTxMultiplier(ptx_RUL_WLC_t *ctx);
static void ptx_RUL_WLC_AddChargingTimeoutSafetyMargin(uint32_t *timeout);
static ptx_Status_t ptx_RUL_WLC_ProcessWptStopNtf(void *ctx, ptx_NSC_WLC_WptStop_Ntf_t *ntf, uint8_t *ntf_used);

/** Internal Sleep State related functions. */
static ptx_Status_t ptx_RUL_WLC_PresenceCheck(ptx_RUL_WLC_t *ctx, uint8_t *isAvailable);
static ptx_Status_t ptx_RUL_WLC_CheckListenerPresence(ptx_RUL_WLC_t *ctx, ptx_RUL_CardRfTechParam_t cardParams, uint8_t *isAvailable);

/** Internal Error/Deactivation related functions. */
static ptx_Status_t ptx_RUL_WLC_DeactivateListener(ptx_RUL_WLC_t *ctx);
static void ptx_RUL_WLC_NtfCallback(ptx_RUL_WLC_t *wlc, ptx_RUL_WLC_Notification_t Notification, void *data, uint32_t dataLen);



/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_RUL_WLC_Init(ptx_RUL_WLC_t *ctx, ptx_RUL_WLC_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams) && PTX_COMP_CHECK(initParams->Rul, Comp_RUL) && (NULL != initParams->Log))
    {
        PTX_LOG_I(initParams->Log, "ptx_RUL_WLC_Init");

        /**
         * The user can set WptPostPowerLevel/WptPowerLevelInitial to 0. In that case the WLC-Poller will
         * take the default power level, that is also used for the initial polling.
         */
        if((0 == initParams->WptPostPwrLevel) || (0 == initParams->WptPowerLevelInitial))
        {
            uint8_t tx_multiplier;
            status = ptx_NSC_RfConfig_GetParam(&initParams->Rul->Nsc, ParamId_PollPwrLvl0, &tx_multiplier);
            
            if(0 == initParams->WptPostPwrLevel)
            {
                initParams->WptPostPwrLevel = tx_multiplier;
            }
            if(0 == initParams->WptPowerLevelInitial)
            {
                initParams->WptPowerLevelInitial = tx_multiplier;
            }
        }
        else 
        {
            status = PTX_STATUS_SUCCESS;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_WLC_CheckInitParamsCoherence(initParams);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_Extension_Endpoints_t endpoints;
            memset(&endpoints, 0, sizeof(ptx_RUL_Extension_Endpoints_t));

            endpoints.CompId         = Comp_WLC;
            endpoints.Extension      = ctx;
            endpoints.ProcessWptStop = ptx_RUL_WLC_ProcessWptStopNtf;

            status = ptx_RUL_RegisterExtensionEndpoints(initParams->Rul, &endpoints);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->State = State_NFCLE_StartPollListener;
            ctx->CompId = Comp_WLC;
            memcpy(&ctx->InitParams, initParams, sizeof(ptx_RUL_WLC_InitParams_t));

            ctx->Log = initParams->Log;

        }
        else
        {
            ctx->State = State_Uninitialized;
            ctx->CompId = Comp_None;
            memset(&ctx->InitParams, 0, sizeof(ptx_RUL_WLC_InitParams_t));
        }

    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_DeInit(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_WLC_DeInit");

        status = ptx_HAL_Tmr_Stop(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_RemoveExtensionEndpoints(ctx->InitParams.Rul, ctx->CompId);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->State = State_Uninitialized;
            ctx->CompId = Comp_None;
            memset(&ctx->InitParams, 0, sizeof(ptx_RUL_WLC_InitParams_t));
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_GetListenerType(ptx_RUL_WLC_t *ctx, ptx_RUL_WLC_ListenerType_t *type)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_WLC_GetListenerType");

        status = PTX_STATUS_SUCCESS;
        *type = ListenerType_Generic;

        /* Check, if there are any cards available. */
        if ( (0u != ctx->InitParams.Rul->CardRegistry.CardsCnt)
             /* Check for Type A. */
             && (ptx_RUL_CardRfTechType_A == ctx->InitParams.Rul->CardRegistry.ActiveCard->TechType)
             /* Check for 7 Byte UID. */
             && (PTX_WLC_PROPRIETARY_T2T_UID_SIZE == ctx->InitParams.Rul->CardRegistry.ActiveCard->TechParam.TypeA.NfcId1Len)
             /* Check for Renesas Technology Corp. manufacturer code. */
             && ( (REN_TECH_CORP_MANUFACTURER_CODE == ctx->InitParams.Rul->CardRegistry.ActiveCard->TechParam.TypeA.NfcId1[0]) ||
             /* Check for Renesas Electronics Corp. manufacturer code. */
                  (REN_ELEC_CORP_MANUFACTURER_CODE == ctx->InitParams.Rul->CardRegistry.ActiveCard->TechParam.TypeA.NfcId1[0])) )
        {
            *type = ListenerType_Proprietary;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StartPowerTransfer(ptx_RUL_WLC_t *ctx, ptx_NSC_WLC_WptOptions_t *wptOptions)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL) && (NULL != wptOptions))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_WLC_StartPowerTransfer");

        /* 
         * Manually activate the WLC extension endpoint once a Power Transfer starts to ensure that the NSC_WLC_WPT_STOP_NTF 
         * is routed correctly to the callback ptx_RUL_WLC_ProcessWptStopNtf.
         */
        ctx->InitParams.Rul->ActiveExtensionEndpoint = &ctx->InitParams.Rul->ExtensionEndpoints[PTX_RUL_EXTENSION_INDEX_WLC];

        if ( (WptState_Idle == ctx->WptState) || (WptState_ICD == ctx->WptState) )
        {
            status = ptx_NSC_WLC_WptStart(&(ctx->InitParams.Rul->Nsc), wptOptions);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->WptState = WptState_Ongoing;
        }
        else
        {
            ctx->WptState = WptState_Error;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StopPowerTransfer(ptx_RUL_WLC_t *ctx, uint8_t postPowerLevel)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_WLC_StopPowerTransfer");

        if (WptState_Ongoing == ctx->WptState)
        {
            status = ptx_NSC_WLC_WptStop(&(ctx->InitParams.Rul->Nsc), postPowerLevel);

            /* There are two success cases of this function. 1. Either it returns PTX_STATUS_SUCCESS and therefore
             * successfully stops the WPT Cycle and the WPT state is in Idle, or 2. If we receive a WPT Stop Ntf during
             * the WPT Stop Command process. For this corner case, it is possible that the WPT State was already set to
             * Idle or ICD and it returns a PTX_STATUS_E_GEN_INVALID_STATE error. For this case we overwrite the status
             * to PTX_STATUS_SUCCESS and return the current WPT State.
             */

            if ((PTX_STATUS_SUCCESS == status) || ((PTX_STATUS_E_GEN_INVALID_STATE == status) && (WptState_Idle == ctx->WptState)))
            {
                ctx->WptState = WptState_Idle;
                status = PTX_STATUS_SUCCESS;
            }
            else if ((PTX_STATUS_E_GEN_INVALID_STATE == status) && (WptState_ICD == ctx->WptState))
            {
                ctx->WptState = WptState_ICD;
                status = PTX_STATUS_SUCCESS;
            }
        }

        if (PTX_STATUS_SUCCESS != status)
        {
            ctx->WptState = WptState_Error;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_GetPowerTransferState(ptx_RUL_WLC_t *ctx, ptx_RUL_WLC_WptState_t *state)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && (NULL != state))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_WLC_GetPowerTransferState");

        *state = ctx->WptState;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_RunPollerStateMachine(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_WLC_RunPollerStateMachine");

        if (0u == ctx->StateMachineActive)
        {
            ctx->StateMachineActive = 1u;

            ptx_RUL_Status_Params_t status_params;
            status_params.Type = ptx_RUL_StatusType_System;
            status = ptx_RUL_GetStatusInfo(ctx->InitParams.Rul, &status_params);

            do
            {
                /* Check regularly for System-state/-health (e.g. temperature/over-current errors etc.) */
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_RUL_GetStatusInfo(ctx->InitParams.Rul, &status_params);
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    if (ptx_RUL_StatusInfoSystem_OK != status_params.Info.System)
                    {
                        /* System-Errors-/-Warnings are critical and needs to be resolved at integration-/application-level.
                         * There can be multiple root causes those errors/warnings, but in most cases they are related
                         * the Hardware-environment (e.g. PCB, component issues etc.).
                         */
                        ctx->State = State_Error;
                    }
                }
                else
                {
                    ctx->State = State_Error;
                }

                switch (ctx->State)
                {
                    case State_NFCLE_StartPollListener:
                        status = ptx_RUL_WLC_StateNFCLE_StartPollListener(ctx);
                        break;

                    case State_NFCLE_PollListener:
                        status = ptx_RUL_WLC_StateNFCLE_PollListener(ctx);
                        break;

                    case State_NFCLE_ActivateListener:
                        status = ptx_RUL_WLC_StateNFCLE_ActivateListener(ctx);
                        break;

                    case State_WCCA_ReadCapabilityRecord:
                        status = ptx_RUL_WLC_StateWCCA_ReadCapabilityRecord(ctx);
                        break;

                    case State_WCCA_StartPollListenerReady:
                        status = ptx_RUL_WLC_StateWCCA_StartPollListenerReady(ctx);
                        break;

                    case State_WCCA_PollListenerReady:
                        status = ptx_RUL_WLC_StateWCCA_PollListenerReady(ctx);
                        break;

                    case State_WCC_WriteInfoRecord:
                        status = ptx_RUL_WLC_StateWCC_WriteInfoRecord(ctx);
                        break;

                    case State_WCC_ReadControlRecord:
                        status = ptx_RUL_WLC_StateWCC_ReadControlRecord(ctx);
                        break;

                    case State_WPT_StartStaticCharging:
                        status = ptx_RUL_WLC_StateWPT_StartStaticCharging(ctx);
                        break;

                    case State_WPT_StartNegotiatedCharging:
                        status = ptx_RUL_WLC_StateWPT_StartNegotiatedCharging(ctx);
                        break;

                    case State_WPT_PollCharging:
                        status = ptx_RUL_WLC_StateWPT_PollCharging(ctx);
                        break;

                    case State_Sleep_StartSleep:
                        status = ptx_RUL_WLC_StateSleep_StartSleep(ctx);
                        break;

                    case State_Sleep_PollSleep:
                        status = ptx_RUL_WLC_StateSleep_PollSleep(ctx);
                        break;

                    case State_Off_DeactivateListener:
                        status = ptx_RUL_WLC_StateOff_DeactivateListener(ctx);
                        break;

                    default: /*< Fall through */
                    case State_Error:
                        status = ptx_RUL_WLC_StateError_ProcessError(ctx, status, status_params.Info.System);
                        break;
                }
            }
            while (1u == ctx->StateMachineActive);
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }
    return status;
}



/*
 * ####################################################################################################################
 * FUNCTIONS FOR POLLER STATE MACHINE
 * ####################################################################################################################
 */

ptx_Status_t ptx_RUL_WLC_StateNFCLE_StartPollListener(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateNFCLE_StartPollListener");

        if ((State_NFCLE_StartPollListener == ctx->State) && (1u == ctx->StateMachineActive))
        {
            /* Wakeup Poller from potential Standby/Sleep state and start a discovery. */
            status = ptx_RUL_SetPowerMode(ctx->InitParams.Rul, PowerMode_Wakeup);

            /* 
             * If the SetPowerMode command returns an invalid state error, this is accepted as a success.
             * It represents the case, that the chip is already Awake.
             */
            if (PTX_STATUS_E_GEN_INVALID_STATE == status)
            {
                status = PTX_STATUS_SUCCESS;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_InitiateDiscovery(ctx->InitParams.Rul, &ctx->InitParams.NfcLeDiscoverConfig);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->State = State_NFCLE_PollListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_PollingStarted, NULL, 0);
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StateNFCLE_PollListener(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateNFCLE_PollListener");

        if ((State_NFCLE_PollListener == ctx->State) && (1u == ctx->StateMachineActive))
        {
            /* Retrieve discovery information from system. */
            ptx_RUL_Status_Params_t status_params;
            status_params.Type = ptx_RUL_StatusType_Discover;
            status = ptx_RUL_GetStatusInfo(ctx->InitParams.Rul, &status_params);

            switch (status_params.Info.Discover)
            {
                case ptx_RUL_StatusInfoDiscover_NoCard:
                    /* OK - do nothing and continue the discovery loop. */
                    break;

                case ptx_RUL_StatusInfoDiscover_Active:
                {
                    /* A card was activated during discovery with only one card in the registry, directly proceed to read out the WLC_CAP record. */
                    ptx_RUL_CardRegistry_t *card_registry;
                    status = ptx_RUL_GetCardRegistry(ctx->InitParams.Rul, &card_registry);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        status = ptx_RUL_WLC_ResetListenerParameter(ctx);
                        if (PTX_STATUS_SUCCESS == status)
                        {
                            ctx->State = State_WCCA_ReadCapabilityRecord;
                            ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDetected, &(card_registry->CardsCnt), sizeof(uint32_t));
                            ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerActivated, &(card_registry->ActiveCard), sizeof(ptx_RUL_CardParams_t));
                        }
                    }


                    break;
                }

                case ptx_RUL_StatusInfoDiscover_CardsAvailable:
                case ptx_RUL_StatusInfoDiscover_MultiProtocol:
                {
                    /* Multiple Cards have been discovered or a single card with multiple available protocols. For this
                     * case we need to perform the WLC activation procedure.
                     */
                    ptx_RUL_CardRegistry_t *card_registry;
                    status = ptx_RUL_GetCardRegistry(ctx->InitParams.Rul, &card_registry);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        ctx->State = State_NFCLE_ActivateListener;
                        ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDetected, &(card_registry->CardsCnt), sizeof(uint32_t));
                    }
                    break;

                }
                default:
                    break;
            }

        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StateNFCLE_ActivateListener(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateNFCLE_ActivateListener");

        if ((State_NFCLE_ActivateListener == ctx->State) && (1u == ctx->StateMachineActive))
        {
            ptx_RUL_CardRegistry_t *card_registry;
            status = ptx_RUL_GetCardRegistry(ctx->InitParams.Rul, &card_registry);

            if (PTX_STATUS_SUCCESS == status)
            {

                /* In case something goes wrong with a chosen card, such as during the activation of
                 * the listener or if it is not a WLC-L capable device, we are restarting from the initial polling state,
                 * deactivating the field and restart the discovery. In the standard case, all listeners are still in the
                 * field with no new devices, this would result in, that the same card would be used again and might fail with the
                 * same error. This way, we never try the other listeners in the registry, even if there is a WLC-L capable device.
                 * To prevent this, we cycle through all cards from the current registry and skip the last used
                 * card on the next try. With this we can try all cards which are currently available in the RF field. To ensure
                 * correct behavior, we need to keep track of the total number of cards in the registry. Any changes will lead
                 * in a reset of the cycle counter to start with the first card again. Additional once all cards have been tried
                 * it starts from the beginning.
                 */
                if ((0 != memcmp(&(ctx->CardRegistryCardCnt), &card_registry->CardsCnt, sizeof(uint32_t))) || (ctx->CardRegistryCardId >= card_registry->CardsCnt))
                {
                    ctx->CardRegistryCardCnt = card_registry->CardsCnt;
                    ctx->CardRegistryCardId = 0;
                }
                else if (ctx->CardRegistryCardId < card_registry->CardsCnt)
                {
                    ctx->CardRegistryCardId++;
                }

                for (uint32_t i = ctx->CardRegistryCardId; i < ctx->CardRegistryCardCnt; i++)
                {
                    ptx_RUL_CardProtocolType_t selected_protocol;
                    /* Search for a compatible protocol, to talk to the listener for WLC. */
                    status = ptx_RUL_WLC_SelectCompatibleProtocol(card_registry->Cards[i].Protocols, &selected_protocol);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        ptx_RUL_CardProtocol_t card_protocol;
                        memset(&card_protocol, 0, sizeof(ptx_RUL_CardProtocol_t));

                        card_protocol.Type = selected_protocol;

                        status = ptx_RUL_SelectCard(ctx->InitParams.Rul, i, card_protocol);
                    }

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        ctx->CardRegistryCardId = i;
                        status = ptx_RUL_WLC_ResetListenerParameter(ctx);
                    }

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        ctx->State = State_WCCA_ReadCapabilityRecord;
                        ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerActivated, &(card_registry->ActiveCard), sizeof(ptx_RUL_CardParams_t));
                        break;
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}


ptx_Status_t ptx_RUL_WLC_StateWCCA_ReadCapabilityRecord(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateWCCA_ReadCapabilityRecord");

        if ((State_WCCA_ReadCapabilityRecord == ctx->State) && (1u == ctx->StateMachineActive))
        {
            ptx_RUL_CardRegistry_t *card_registry;
            status = ptx_RUL_GetCardRegistry(ctx->InitParams.Rul, &card_registry);

            /* Check if the active card is a T5T and if there are multiple cards present, as it must be selected 
             * before exchanging any NDEF data to prevent other T5T cards to also to respond to commands. 
             */
            if ((PTX_STATUS_SUCCESS == status) && (card_registry->ActiveCard->Protocols == ptx_RUL_SupportedProtocols_T5T) && (1 < card_registry->CardsCnt))
            {
                uint8_t rx[PTX_T5T_SELECT_RES_LEN] = {0};
                uint32_t rxLen = PTX_T5T_SELECT_RES_LEN;
                status = ptx_NativeTag_T5T_Select(&(ctx->NdefComp.tagCtx.t5t.t5t), 0, &(card_registry->ActiveCard->TechParam.TypeV.InventoryRes[PTX_T5T_INVENTORY_RES_OFFSET_UID]), PTX_T5T_UID_LEN, rx, &rxLen, PTX_T5T_DEFAULT_TIMEOUT_MS);

                if((PTX_STATUS_SUCCESS != status) || (PTX_T5T_RES_FLAG_ERROR_LEN != rxLen) || (0 != (rx[PTX_T5T_OFFSET_RES_FLAG] & PTX_T5T_RES_FLAG_ERROR_MASK)))
                {
                    status = PTX_STATUS_E_NDEF_TAG_NACK;
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Check if a WLCCAP NDEF message is present on the listener and parse its data. */
                status = ptx_RUL_WLC_ReadWLCCAPMessage(ctx);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_WLC_ParseWLCCAPRecord(ctx);
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}


ptx_Status_t ptx_RUL_WLC_StateWCCA_StartPollListenerReady(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateWCCA_StartPollListenerReady");

        if ((State_WCCA_StartPollListenerReady == ctx->State) && (1u == ctx->StateMachineActive))
        {
            uint32_t waiting_time_ms;
            status = ptx_NDEFRecord_WLCCAP_GetCapWtMillis(&ctx->RecordWlcCap, &waiting_time_ms);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_HAL_Tmr_Start(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, HAL_DRV_Tmr_Unit_mSec, waiting_time_ms);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->State = State_WCCA_PollListenerReady;
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StateWCCA_PollListenerReady(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateWCCA_PollListenerReady");

        if ((State_WCCA_PollListenerReady == ctx->State) && (1u == ctx->StateMachineActive))
        {
            uint8_t is_elapsed = 0;
            status = ptx_HAL_Tmr_IsElapsed(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, &is_elapsed);

            if (PTX_STATUS_SUCCESS == status)
            {
                if (0 != is_elapsed)
                {
                    ctx->CntrWlcCapNwt++;
                    ctx->State = State_WCCA_ReadCapabilityRecord;
                }
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StateWCC_WriteInfoRecord(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateWCC_WriteInfoRecord");

        if ((State_WCC_WriteInfoRecord == ctx->State) && (1u == ctx->StateMachineActive))
        {
            ptx_NDEFRecord_t records[1];
            uint8_t number_of_pwr_steps;

            /* Build WLCINF record. */
            status = ptx_NDEFRecord_Init(&records[0]);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEFRecord_WLCINF_Init(&ctx->RecordWlcInf);
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEFRecord_WLCINF_SetPTX(&ctx->RecordWlcInf, ptx_RUL_WLC_TxMultiplierToPercent(ctx->TxMultiplier));
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEFRecord_WLCINF_SetWptStopSupport(&ctx->RecordWlcInf, PTX_WLC_STOP_DETECTION_SUPPORTED);
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEFRecord_WLCINF_SetPowerClass(&ctx->RecordWlcInf, ctx->InitParams.WlcInfoPowerClass);
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                number_of_pwr_steps = ctx->InitParams.WptPowerLevelMax - ctx->InitParams.WptPowerLevelMin;
                status = ptx_NDEFRecord_WLCINF_SetTotPowerSteps(&ctx->RecordWlcInf, number_of_pwr_steps);
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEFRecord_WLCINF_SetCurPowerStep(&ctx->RecordWlcInf, ctx->TxMultiplier - number_of_pwr_steps);
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEFRecord_WLCINF_SetNextMinStepInc(&ctx->RecordWlcInf, (int8_t) PTX_WLC_POLLER_MIN_STEP_INC);
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEFRecord_WLCINF_SetNextMinStepDec(&ctx->RecordWlcInf, (int8_t) PTX_WLC_POLLER_MIN_STEP_DEC);
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEFRecord_WLCINF_Create(&records[0], &ctx->RecordWlcInf);
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_WLC_WriteNdefMessage(ctx, records, 1u);
            }

            /* Wait for a certain time before proceeding to read the WLCCTL record as defined in the WLCCAP record.
             * A Margin Delay is added as defined in the Init Parameters.
             */
            uint16_t rd_wt;
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEFRecord_WLCCAP_GetRdWtInt(&ctx->RecordWlcCap, &rd_wt);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Maximum possible waiting time is ~2.55s (incl. margin delay waiting time). */
                uint32_t waiting_time_ms = rd_wt + ctx->InitParams.WlcInfoWaitingTimeMarginDelay;

                status = ptx_RUL_Sleep(ctx->InitParams.Rul, waiting_time_ms);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->State = State_WCC_ReadControlRecord;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_InformationRecordSent, &ctx->RecordWlcInf, sizeof(ptx_NDEFRecord_WLCINF_t));
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StateWCC_ReadControlRecord(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateWCC_ReadControlRecord");

        if ((State_WCC_ReadControlRecord == ctx->State) && (1u == ctx->StateMachineActive))
        {
            status = ptx_RUL_WLC_ReadWLCCTLMessage(ctx);

            if ((PTX_STATUS_SUCCESS == status) && (1 == ctx->RecordWlcCtlValid))
            {
                status = ptx_RUL_WLC_ParseWLCCTLRecord(ctx);
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StateWPT_StartStaticCharging(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateWPT_StartStaticCharging");

        if ((State_WPT_StartStaticCharging == ctx->State) && (1u == ctx->StateMachineActive))
        {
            /* Query cap_wt field */
            uint32_t cap_wt_ms;
            status = ptx_NDEFRecord_WLCCAP_GetCapWtMillis(&ctx->RecordWlcCap, &cap_wt_ms);

            ptx_NSC_WLC_WptOptions_t wpt_options;
            if (PTX_STATUS_SUCCESS == status)
            {

                /* Options for WPT cycle. */
                wpt_options.WptAmp = ctx->TxMultiplier;
                wpt_options.NfcAmp = wpt_options.WptAmp;
                wpt_options.WptDur = cap_wt_ms;
                wpt_options.IcdEn  = ctx->InitParams.WptIcdEnable;
                wpt_options.IcdGt  = ctx->InitParams.WptIcdGuardTime;

                /* Start WPT cycle */
                ctx->WptState = WptState_Idle;
                status = ptx_RUL_WLC_StartPowerTransfer(ctx, &wpt_options);
            }

            uint32_t timeout_ms;
            if (PTX_STATUS_SUCCESS == status)
            {
                /* Power cycle started, let's report to the application. */
                ptx_RUL_WLC_NtfCallback(ctx, Notification_StaticWptCycleStarted, &wpt_options, sizeof(ptx_NSC_WLC_WptOptions_t));

                /* Retrieve waiting time (=charging time) in milliseconds */
                status = ptx_NDEFRecord_WLCCAP_GetCapWtMillis(&ctx->RecordWlcCap, &timeout_ms);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Add 10% safety margin to timeout */
                ptx_RUL_WLC_AddChargingTimeoutSafetyMargin(&timeout_ms);

                status = ptx_HAL_Tmr_Start(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, HAL_DRV_Tmr_Unit_mSec, timeout_ms);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->State = State_WPT_PollCharging;
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StateWPT_StartNegotiatedCharging(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateWPT_StartNegotiatedCharging");

        if ((State_WPT_StartNegotiatedCharging == ctx->State) && (1u == ctx->StateMachineActive))
        {
            uint32_t wpt_duration_ms;
            uint32_t hold_off_duration_ms;

            /* Query wpt_duration and calculate time in milliseconds. */
            status = ptx_NDEFRecord_WLCCTL_GetWptDurationMillis(&ctx->RecordWlcCtl, &wpt_duration_ms);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Query hold_off_duration and calculate time in milliseconds. */
                status = ptx_NDEFRecord_WLCCTL_GetHoldOffWtMillis(&ctx->RecordWlcCtl, &hold_off_duration_ms);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Protocol error, if hold_off_duration_ms > 25% of wpt_duration_ms */
                if ((wpt_duration_ms >> PTX_WLC_SHIFT_DIVBY4) >= hold_off_duration_ms)
                {
                    /* Wait for hold_off_duration before changing power level */
                    status = ptx_RUL_Sleep(ctx->InitParams.Rul, hold_off_duration_ms);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* Calculating requested power level. */
                        status = ptx_RUL_WLC_ComputeRequiredTxMultiplier(ctx);
                    }

                    ptx_NSC_WLC_WptOptions_t wpt_options;
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        wpt_options.WptAmp = ctx->TxMultiplier;
                        
                        /* WPT power level is too high, therefore reduce level below threshold to the defined pwr level. */
                        if (ctx->InitParams.WptPostPwrThreshold < ctx->TxMultiplier)
                        {
                            wpt_options.NfcAmp = ctx->InitParams.WptPostPwrLevel;
                        }
                        /* Even though, the WPT power level is below the threshold, since option B is selected, set to the defined pwr level. */
                        else if ((ctx->InitParams.WptPostPwrThreshold >= ctx->TxMultiplier) && (ctx->InitParams.WptPostPwrOption == WptPostPwrOption_B))
                        {
                            wpt_options.NfcAmp = ctx->InitParams.WptPostPwrLevel;
                        }
                        /* Since the WPT power level is below the threshold and option A is selected, leave on WPT power level. */
                        else if ((ctx->InitParams.WptPostPwrThreshold >= ctx->TxMultiplier) && (ctx->InitParams.WptPostPwrOption == WptPostPwrOption_A))
                        {
                            wpt_options.NfcAmp = ctx->TxMultiplier;
                        }

                        wpt_options.WptDur = wpt_duration_ms - hold_off_duration_ms - ctx->InitParams.WptPostSettleTime;
                        wpt_options.IcdEn  = ctx->InitParams.WptIcdEnable;
                        wpt_options.IcdGt  = ctx->InitParams.WptIcdGuardTime;

                        ctx->WptState = WptState_Idle;
                        status = ptx_RUL_WLC_StartPowerTransfer(ctx, &wpt_options);
                    }

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* Wait for the process to be finished. */
                        uint32_t wpt_timeout_ms = wpt_duration_ms;

                        /* Add additional 10% margin to timeout. */
                        ptx_RUL_WLC_AddChargingTimeoutSafetyMargin(&wpt_timeout_ms);

                        /* Power cycle started, let's report to the application. */
                        ptx_RUL_WLC_NtfCallback(ctx, Notification_NegotiatedWptCycleStarted, &wpt_options, sizeof(ptx_NSC_WLC_WptOptions_t));

                        status = ptx_HAL_Tmr_Start(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, HAL_DRV_Tmr_Unit_mSec, wpt_timeout_ms);
                    }
                }
                else
                {
                    /* hold_off_duration violation */
                    status = PTX_STATUS_WLC_INVALID_HOLD_OFF_DURATION;
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->State = State_WPT_PollCharging;
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}


ptx_Status_t ptx_RUL_WLC_StateWPT_PollCharging(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateWPT_PollCharging");

        if ((State_WPT_PollCharging == ctx->State) && (1u == ctx->StateMachineActive))
        {
            ptx_RUL_Status_Params_t status_params;
            status_params.Type = ptx_RUL_StatusType_System;
            status = ptx_RUL_GetStatusInfo(ctx->InitParams.Rul, &status_params);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Check if charging cycle successfully terminated. Once the WPT State is idle we internally terminated
                 * the WPT cycle.
                 */
                if ( WptState_Idle == ctx->WptState )
                {
                    status = ptx_HAL_Tmr_Stop(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1);

                    ptx_NDEFRecord_WLCCAP_Mode_t mode;
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        status = ptx_NDEFRecord_WLCCAP_GetMode(&ctx->RecordWlcCap, &mode);
                    }

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        if (Mode_Static == mode)
                        {
                            ctx->State = State_WCCA_ReadCapabilityRecord;
                        }
                        else
                        {
                            status = ptx_RUL_Sleep(ctx->InitParams.Rul, ctx->InitParams.WptPostSettleTime);

                            uint8_t info_req;
                            if (PTX_STATUS_SUCCESS == status)
                            {
                                status = ptx_NDEFRecord_WLCCTL_GetWptInfoReq(&ctx->RecordWlcCtl, &info_req);
                            }

                            if (PTX_STATUS_SUCCESS == status)
                            {
                                /* Check if the Listener wants us to omit the WLCINF field in the next negotiation cycle. */
                                if (info_req)
                                {
                                    ctx->State = State_WCC_WriteInfoRecord;
                                }
                                else
                                {
                                    ctx->State = State_WCC_ReadControlRecord;
                                }
                            }
                        }
                    }

                    ptx_RUL_WLC_NtfCallback(ctx, Notification_WptCycleFinished, &(ctx->WptState), sizeof(ptx_RUL_WLC_WptState_t));
                }
                else if ( WptState_ICD == ctx->WptState ) /*< Foreign object detected during WPT cycle. */
                {
                    status = ptx_HAL_Tmr_Stop(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        status = PTX_STATUS_WLC_WPT_ICD;
                    }
                    ptx_RUL_WLC_NtfCallback(ctx, Notification_WptCycleFinished, &(ctx->WptState), sizeof(ptx_RUL_WLC_WptState_t));
                }
                else
                {
                    /* No WPT event happened. */
                    uint8_t is_elapsed = 0;
                    status = ptx_HAL_Tmr_IsElapsed(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, &is_elapsed);

                    if ((PTX_STATUS_SUCCESS == status) && (0 != is_elapsed)) /*< Timeout and no response from PTX2xxR/W. We should never end up in here. */
                    {
                        status = PTX_STATUS_WLC_WPT_TIMEOUT;
                    }

                    if (PTX_STATUS_SUCCESS != status)
                    {
                        (void) ptx_RUL_WLC_StopPowerTransfer(ctx, ctx->InitParams.WptPowerLevelMin);
                        (void) ptx_HAL_Tmr_Stop(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1);
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }


    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StateSleep_StartSleep(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateSleep_StartSleep");

        if ((State_Sleep_StartSleep == ctx->State) && (1u == ctx->StateMachineActive))
        {
            ptx_NDEFRecord_WLCCAP_Mode_t mode;
            uint32_t wait_duration_ms;
            status = ptx_NDEFRecord_WLCCAP_GetMode(&ctx->RecordWlcCap, &mode);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* When 'mode' field in CAP is set to 'BAT_FULL' wait for CAP_WT, otherwise wait for WPT_DUR. */
                if (Mode_BattFull == mode)
                {
                    status = ptx_NDEFRecord_WLCCAP_GetCapWtMillis(&ctx->RecordWlcCap, &wait_duration_ms);
                }
                else
                {
                    status = ptx_NDEFRecord_WLCCTL_GetWptDurationMillis(&ctx->RecordWlcCtl, &wait_duration_ms);
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_WLC_DeactivateListener(ctx);
            }

            uint32_t sleep_interval = 0;
            if (PTX_STATUS_SUCCESS == status)
            {
                if (ctx->InitParams.SleepInterval > wait_duration_ms)
                {
                    sleep_interval = wait_duration_ms;
                }
                else
                {
                    sleep_interval = ctx->InitParams.SleepInterval;
                }

                status = ptx_HAL_Tmr_Start(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, HAL_DRV_Tmr_Unit_mSec, sleep_interval);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->SleepInterval = sleep_interval;
                ctx->SleepTotalDuration = wait_duration_ms;
                ctx->SleepRemainingDuration = ctx->SleepTotalDuration;
                ctx->State = State_Sleep_PollSleep;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_SleepStarted, &(ctx->SleepTotalDuration), sizeof(uint32_t));
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StateSleep_PollSleep(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateSleep_PollSleep");

        if ((State_Sleep_PollSleep == ctx->State) && (1u == ctx->StateMachineActive))
        {
            uint8_t is_elapsed = 0;
            status = ptx_HAL_Tmr_IsElapsed(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, &is_elapsed);

            if ((PTX_STATUS_SUCCESS == status) && (0 != is_elapsed))
            {
                uint8_t is_available = 0;

                status = ptx_HAL_Tmr_Stop(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1);

                if (ctx->SleepRemainingDuration >= ctx->SleepInterval)
                {
                    ctx->SleepRemainingDuration -= ctx->SleepInterval;
                }
                else
                {
                    ctx->SleepRemainingDuration = 0;
                }

                if ((PTX_STATUS_SUCCESS == status) && (0 != ctx->SleepRemainingDuration))
                {
                    status = ptx_RUL_WLC_PresenceCheck(ctx, &is_available);
                }

                if ( (0u == is_available) || (0 == ctx->SleepRemainingDuration))
                {
                    /* If the sleep duration is over or the listener was removed, restart the WLC State Machine. */
                    ctx->State = State_NFCLE_StartPollListener;
                    ptx_RUL_WLC_NtfCallback(ctx, Notification_SleepFinished, &(ctx->SleepRemainingDuration), sizeof(uint32_t));
                }
                else if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_HAL_Tmr_Start(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, HAL_DRV_Tmr_Unit_mSec, ctx->SleepInterval);
                }
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }

    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StateOff_DeactivateListener(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateOff_DeactivateListener");

        if ((State_Off_DeactivateListener == ctx->State) && (1u == ctx->StateMachineActive))
        {
            status = ptx_RUL_WLC_DeactivateListener(ctx);

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->State = State_NFCLE_StartPollListener;
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }
    }

    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_StateError_ProcessError(ptx_RUL_WLC_t *ctx, ptx_Status_t wlcStatus, ptx_RUL_StatusInfoSystem_t systemInfo)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_StateError_ProcessError");

        status = wlcStatus;

        if ((State_Error == ctx->State) && (1u == ctx->StateMachineActive))
        {
            /* Process all WLC Status values. */
            if (PTX_STATUS_WLC_INVALID_STATE == status)
            {
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_InvalidState, NULL, 0);

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
            else if (PTX_STATUS_WLC_INCOMPATIBLE_TAG_PROTOCOL == status)
            {
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_IncompatibleTagProtocol, NULL, 0);

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
            else if (PTX_STATUS_WLC_INVALID_WLC_PROTOCOL_VERSION == status)
            {
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_InvalidWlcProtocolVersion, NULL, 0);

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
            else if (PTX_STATUS_WLC_CAP_RECORD_MISSING == status)
            {
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_CapabilityRecordMissing, NULL, 0);

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
            else if (PTX_STATUS_WLC_NEGO_WAIT_MAX_NUM_RETRIES == status)
            {
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_NegoWaitMaxRetries, NULL, 0);

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
            else if (PTX_STATUS_WLC_CTL_RECORD_MISSING == status)
            {
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_ControlRecordMissing, NULL, 0);

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
            else if (PTX_STATUS_WLC_CTL_MAX_NUM_RETRIES == status)
            {
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_ControlRecordMaxRetries, NULL, 0);

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
            else if (PTX_STATUS_WLC_INVALID_MODE == status)
            {
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_InvalidMode, NULL, 0);

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
            else if (PTX_STATUS_WLC_PROTOCOL_ERROR == status)
            {
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_ProtocolError, NULL, 0);

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
            else if (PTX_STATUS_WLC_INVALID_HOLD_OFF_DURATION == status)
            {
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_InvalidHoldoffDuration, NULL, 0);

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
            else if (PTX_STATUS_WLC_WPT_ICD == status)
            {
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_WptIcd, NULL, 0);

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
            else if (PTX_STATUS_WLC_WPT_TIMEOUT == status)
            {
                /* This timeout should never happen. If it is the case, the application needs to restart the HW and SW. */
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_WptTimeout, &status, sizeof(ptx_Status_t));

                /* Exit the state machine if the status was not cleaned by the callback of the notification. */
                if (PTX_STATUS_SUCCESS != status)
                {
                    ctx->StateMachineActive = 0;
                }
            }
            else if (ptx_RUL_StatusInfoSystem_OK != systemInfo)
            {
                /* System-Errors-/-Warnings are critical and needs to be resolved at integration-/application-level.
                 * There can be multiple root causes those errors/warnings, but in most cases they are related
                 * the Hardware-environment (e.g. PCB, component issues etc.).
                 * If any System Error happens, the application needs to restart the HW and SW.
                 */
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_SystemStatusInfo, &systemInfo, sizeof(ptx_RUL_StatusInfoSystem_t));

                /* Exit the state machine if the system information was not cleaned by the callback of the notification. */
                if (ptx_RUL_StatusInfoSystem_OK != systemInfo)
                {
                    ctx->StateMachineActive = 0;
                }

            }
            else
            {
                /* In case of any other Status, restart the state machine and deactivate the active listener. */
                ptx_RUL_WLC_NtfCallback(ctx, Notification_Error_Internal, &status, sizeof(ptx_Status_t));

                /* Deactivate the Activated Listener. */
                ctx->State = State_Off_DeactivateListener;
                ptx_RUL_WLC_NtfCallback(ctx, Notification_ListenerDeactivated, NULL, 0);
                status = PTX_STATUS_SUCCESS;
            }
        }
        else
        {
            status = PTX_STATUS_WLC_INVALID_STATE;
        }

    }


    if (PTX_STATUS_SUCCESS != status)
    {
        ctx->State = State_Error;
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_RUL_WLC_CheckInitParamsCoherence(ptx_RUL_WLC_InitParams_t const *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    if (NULL != initParams &&
        (initParams->WptPowerLevelMax >= initParams->WptPostPwrLevel) &&
        (initParams->WptPowerLevelMin <= initParams->WptPostPwrLevel) &&
        (initParams->WptPowerLevelMax >= initParams->WptPostPwrThreshold) &&
        (initParams->WptPowerLevelMin <= initParams->WptPostPwrThreshold) &&
        (initParams->WptPowerLevelMax >= initParams->WptPowerLevelInitial) &&
        (initParams->WptPowerLevelMin <= initParams->WptPowerLevelInitial) &&
        (WptPostPwrOption_A == initParams->WptPostPwrOption || WptPostPwrOption_B == initParams->WptPostPwrOption))
    {
        PTX_LOG_V(initParams->Log, "ptx_RUL_WLC_CheckInitParamsCoherence");

        status = PTX_STATUS_SUCCESS;
    }
    return status;
}

static ptx_Status_t ptx_RUL_WLC_ResetListenerParameter(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_ResetListenerParameter");

        ctx->WptState = WptState_Idle;
        ctx->CntrWlcCtlNwcc = 0u;
        ctx->CntrWlcCapNwt = 0u;
        ctx->WlcCtlCnt = PTX_WLC_CTRL_CNTR_RESET_VALUE;
        ctx->RecordWlcStaiAvailable = 0u;
        ctx->RecordDevInfoAvailable = 0u;
        ctx->SleepTotalDuration = 0;
        ctx->SleepRemainingDuration = 0;
        ctx->RecordWlcCtlValid = 0;

        ctx->NdefInitParams.rx = ctx->NdefRxBuffer;
        ctx->NdefInitParams.rxLen = sizeof(ctx->NdefRxBuffer);
        ctx->NdefInitParams.tx = ctx->NdefTxBuffer;
        ctx->NdefInitParams.txLen = sizeof(ctx->NdefTxBuffer);
        ctx->NdefInitParams.workBuffer = ctx->NdefWorkBuffer;
        ctx->NdefInitParams.workBufferLen = sizeof(ctx->NdefWorkBuffer);
        ctx->NdefInitParams.rul = ctx->InitParams.Rul;
        ctx->TxMultiplier = ctx->InitParams.WptPowerLevelInitial;
            
        status = ptx_NDEF_Open(&ctx->NdefComp, &ctx->NdefInitParams);


    }

    return status;
}

static ptx_Status_t ptx_RUL_WLC_SelectCompatibleProtocol(uint8_t supportedProtocols, ptx_RUL_CardProtocolType_t *selectedProtocol)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != selectedProtocol)
    {
        *selectedProtocol = ptx_RUL_CardProtocolType_Undefined;

        if (0 != (supportedProtocols & ptx_RUL_SupportedProtocols_T2T) )
        {
            *selectedProtocol = ptx_RUL_CardProtocolType_T2T;
        }

        if (0 != (supportedProtocols & ptx_RUL_SupportedProtocols_T3T))
        {
            *selectedProtocol = ptx_RUL_CardProtocolType_T3T;
        }

        if (0 != (supportedProtocols & ptx_RUL_SupportedProtocols_T4T))
        {
            *selectedProtocol = ptx_RUL_CardProtocolType_T4T;
        }

        if (0 != (supportedProtocols & ptx_RUL_SupportedProtocols_T5T))
        {
            *selectedProtocol = ptx_RUL_CardProtocolType_T5T;
        }

        if (ptx_RUL_CardProtocolType_Undefined == *selectedProtocol)
        {
            status = PTX_STATUS_WLC_INCOMPATIBLE_TAG_PROTOCOL;
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_WLC_ReadNdefMessage(ptx_RUL_WLC_t *ctx, ptx_NDEFRecord_t *records, uint32_t *recordsLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL) && (NULL != records) && (NULL != recordsLen))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_ReadNdefMessage");

        uint32_t len = PTX_WLC_MAX_NDEF_MESSAGE_BUFFER_SIZE;
        status = ptx_NDEF_ReadMessage(&ctx->NdefComp, ctx->NdefMsgBuffer, &len);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NDEFMessage_Parse(ctx->NdefMsgBuffer, len, records, recordsLen, false);
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_WLC_WriteNdefMessage(ptx_RUL_WLC_t *ctx, ptx_NDEFRecord_t *records, uint32_t recordsLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL) && (NULL != records))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_WriteNdefMessage");

        uint32_t len = ctx->NdefComp.txSize;

        /* Create one contiguous NDEF message from individual records and store it in the NDEF message working buffer. */
        status = ptx_NDEFMessage_Create(records, recordsLen, ctx->NdefMsgBuffer, &len);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NDEF_WriteMessage(&ctx->NdefComp, ctx->NdefMsgBuffer, len);
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_WLC_ReadWLCCAPMessage(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_ReadWLCCAPMessage");

        status = ptx_NDEF_CheckMessage(&ctx->NdefComp);

        ptx_NDEFRecord_t record_buffer[PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE];
        uint32_t records_len = PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE;
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_WLC_ReadNdefMessage(ctx, record_buffer, &records_len);
        }

        /* Reserve some memory for storing the IDs of records of interest (WLCCAP, WLCSTAI, etc.) */
        uint32_t filtered_rec_ids[PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE];
        uint32_t filtered_rec_len = (records_len < PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE ? records_len : PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE);
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Find the WLCCAP record within the read NDEF records. */
            status = ptx_NDEFMessage_FilterRecordsByType(record_buffer, &filtered_rec_len, PTX_NDEFRECORD_TYPE_RTD_WLCCAP, PTX_NDEFRECORD_TYPE_RTD_WLCCAP_LEN, filtered_rec_ids);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Verify that only one single record of type 'WLCCAP' was found, everything else is an error case. */
            if (1u == filtered_rec_len)
            {
                uint32_t cap_record_id = filtered_rec_ids[0];
                status = ptx_NDEFRecord_WLCCAP_Parse(&record_buffer[cap_record_id], &ctx->RecordWlcCap);

                if (PTX_STATUS_SUCCESS == status)
                {
                    ptx_RUL_WLC_NtfCallback(ctx, Notification_CapabilityRecordReceived, &ctx->RecordWlcCap, sizeof(ptx_NDEFRecord_WLCCAP_t));

                    /* Read any optional records from listener. Optional records are always located AFTER the mandatory ones. */
                    ptx_NDEFRecord_t *optional_records = &record_buffer[cap_record_id + 1];
                    uint32_t optional_records_len = records_len - cap_record_id - 1;

                    /* Parse WLCSTAI record (if existent). */
                    ptx_Status_t optional_status;
                    optional_status = ptx_NDEFMessage_FilterRecordsByType(optional_records, &optional_records_len, PTX_NDEFRECORD_TYPE_RTD_WLCSTAI, PTX_NDEFRECORD_TYPE_RTD_WLCSTAI_LEN, filtered_rec_ids);

                    if ((PTX_STATUS_SUCCESS == optional_status) && (1u == optional_records_len))
                    {
                        optional_status = ptx_NDEFRecord_WLCSTAI_Parse(&optional_records[filtered_rec_ids[0]], &ctx->RecordWlcStai);
                        if (PTX_STATUS_SUCCESS == optional_status)
                        {
                            ctx->RecordWlcStaiAvailable = 1u;
                            ptx_RUL_WLC_NtfCallback(ctx, Notification_StatusInfoRecordReceived, &ctx->RecordWlcStai, sizeof(ptx_NDEFRecord_WLCSTAI_t));
                        }
                    }
                    optional_records_len = records_len - cap_record_id - 1u;

                    /* Parse DeviceInformation record (if existent). */
                    optional_status = ptx_NDEFMessage_FilterRecordsByType(optional_records, &optional_records_len, PTX_NDEFRECORD_TYPE_RTD_DEVICE_INFO, PTX_NDEFRECORD_TYPE_RTD_DEVICE_INFO_LEN, filtered_rec_ids);

                    if ((PTX_STATUS_SUCCESS == optional_status) && (1u == optional_records_len))
                    {
                        optional_status = ptx_NDEFRecord_Di_Parse(&optional_records[filtered_rec_ids[0]], &ctx->RecordDevInfo);
                        if (PTX_STATUS_SUCCESS == optional_status)
                        {
                            ctx->RecordDevInfoAvailable = 1u;
                            ptx_RUL_WLC_NtfCallback(ctx, Notification_DeviceInfoRecordReceived, &ctx->RecordDevInfo, sizeof(ptx_NDEFRecord_DI_t));
                        }
                    }

                }
            }
            else
            {
                status = PTX_STATUS_WLC_CAP_RECORD_MISSING;
            }
        }
    }

    return status;

}

static ptx_Status_t ptx_RUL_WLC_ParseWLCCAPRecord(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_ParseWLCCAPRecord");

        /* If there is a WLCCAP record, the Poller is connected to a valid WLC-L device. */
        ptx_RUL_CardRegistry_t *card_registry;
        status = ptx_RUL_GetCardRegistry(ctx->InitParams.Rul, &card_registry);

        if (PTX_STATUS_SUCCESS == status)
        {
            memcpy(&ctx->Card, card_registry->ActiveCard, sizeof(ptx_RUL_CardParams_t));

            /* Start interpreting WLCCAP record. */
            uint8_t major_prot_version;
            uint8_t minor_prot_version;
            status = ptx_NDEFRecord_WLCCAP_GetProtVersion(&ctx->RecordWlcCap, &major_prot_version, &minor_prot_version);
            if (PTX_STATUS_SUCCESS == status)
            {
                /* If Poller and Listener support the same major WLC protocol version number (regardless of minor protocol number), continue. */
                if (ctx->InitParams.WlcCapVersionMajor == major_prot_version)
                {
                    status = ptx_RUL_WLC_ParseWLCCAPRecordWlcMode(ctx);
                }
                else
                {
                    /* Major protocol version not matching */
                    status = PTX_STATUS_WLC_INVALID_WLC_PROTOCOL_VERSION;
                }
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_WLC_ParseWLCCAPRecordWlcMode(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_ParseWLCCAPRecordWlcMode");

        ptx_NDEFRecord_WLCCAP_Mode_t mode;
        status = ptx_NDEFRecord_WLCCAP_GetMode(&ctx->RecordWlcCap, &mode);

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (mode)
            {
                case Mode_Static:
                {
                    /* Go to static charging for CAP_WT, then re-read capability record again. */
                    ctx->State = State_WPT_StartStaticCharging;
                    break;
                }
                case Mode_Negotiated:
                {
                    status = ptx_RUL_WLC_ParseWLCCAPRecordNegotiatedWlcMode(ctx);
                    break;
                }
                case Mode_BattFull:
                {
                    /* Shut down RF field, wait for CAP_WT before turning it on again (except for a periodic discovery as a presence check). */
                    ctx->State = State_Sleep_StartSleep;
                    break;
                }
                case Mode_RFU: /*< Illegal state --> Protocol error */
                {
                    status = PTX_STATUS_WLC_INVALID_MODE;
                    break;
                }
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_WLC_ParseWLCCAPRecordNegotiatedWlcMode(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_ParseWLCCAPRecordNegotiatedWlcMode");

        ptx_RUL_CardRegistry_t *card_registry;
        status = ptx_RUL_GetCardRegistry(ctx->InitParams.Rul, &card_registry);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Negotiated mode is only allowed if one single listener is in the field. */
            if (1u == card_registry->CardsCnt)
            {
                uint8_t nego_wait;
                status = ptx_NDEFRecord_WLCCAP_GetNegWait(&ctx->RecordWlcCap, &nego_wait);
                if (PTX_STATUS_SUCCESS == status)
                {
                    if (0u == nego_wait)
                    {
                        /* Set CAP retry counter to zero. */
                        ctx->CntrWlcCapNwt = 0;

                        /* Waiting Waiting for a required duration before the listener is capable of receiving the WLCINFO message.
                         * It can range between 1 ms and 10 ms.
                         */
                        uint8_t wr_wt_int;
                        status = ptx_NDEFRecord_WLCCAP_GetWrWtInt(&ctx->RecordWlcCap, &wr_wt_int);
                        if (PTX_STATUS_SUCCESS == status)
                        {
                            wr_wt_int++; /*< Safety margin */

                            ptx_RUL_Sleep(ctx->InitParams.Rul, wr_wt_int);
                        }

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            ctx->State = State_WCC_WriteInfoRecord;
                        }
                    }
                    else
                    {
                        uint8_t n_wt_retries;
                        status = ptx_NDEFRecord_WLCCAP_GetNWtRetries(&ctx->RecordWlcCap, &n_wt_retries);
                        if (PTX_STATUS_SUCCESS == status)
                        {
                            if (ctx->CntrWlcCapNwt >= n_wt_retries)
                            {
                                /* Reached maximum number of allowed retries. */
                                status = PTX_STATUS_WLC_NEGO_WAIT_MAX_NUM_RETRIES;
                            }
                            else
                            {
                                /* Listener not ready yet, go to wait state. */
                                ctx->State = State_WCCA_StartPollListenerReady;
                            }
                        }
                    }
                }
            }
            else
            {
                /* In case Listener requested negotiated mode, but other Listeners were found during discovery
                 * we are not allowed to increase the field strength --> stay in static mode.
                 */
                ctx->State = State_WPT_StartStaticCharging;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_WLC_ReadWLCCTLMessage(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_ReadWLCCTLMessage");

        ptx_NDEFRecord_t record_buffer[PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE];
        uint32_t records_len = PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE;

        /* Read and parse NDEF message of Listener.
         * 'records_len' gets updated with the real number of read records.
         */
        status = ptx_RUL_WLC_ReadNdefMessage (ctx, record_buffer, &records_len);

        uint32_t filtered_rec_pos[PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE];
        uint32_t filtered_rec_len = (records_len < PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE ? records_len : PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE);
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Find WLCCTL record within retrieved NDEF message. */
            status = ptx_NDEFMessage_FilterRecordsByType(record_buffer, &filtered_rec_len, PTX_NDEFRECORD_TYPE_RTD_WLCCTL, PTX_NDEFRECORD_TYPE_RTD_WLCCTL_LEN, filtered_rec_pos);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Verify that only one single record of type 'WLCCTL' was found. */
            if (1u == filtered_rec_len)
            {
                /* 'ctl_rec_pos' contains position of CTL record within NDEF message. */
                uint32_t ctl_rec_pos = filtered_rec_pos[0];
                status = ptx_NDEFRecord_WLCCTL_Parse (&record_buffer[ctl_rec_pos], &ctx->RecordWlcCtl);

                if (PTX_STATUS_SUCCESS == status)
                {
                    ptx_RUL_WLC_NtfCallback (ctx, Notification_ControlRecordReceived, &ctx->RecordWlcCtl, sizeof(ptx_NDEFRecord_WLCCTL_t));
                    ctx->RecordWlcCtlValid = 1;

                    /* Read any optional records from Listener. Optional records are always located AFTER the mandatory ones. */
                    ptx_NDEFRecord_t *optional_records = &record_buffer[ctl_rec_pos + 1];
                    uint32_t optional_rec_len = records_len - ctl_rec_pos - 1;

                    /* Parse WLCSTAI record (if existent). */
                    ptx_Status_t optional_status;
                    optional_status = ptx_NDEFMessage_FilterRecordsByType(optional_records, &optional_rec_len, PTX_NDEFRECORD_TYPE_RTD_WLCSTAI, PTX_NDEFRECORD_TYPE_RTD_WLCSTAI_LEN, filtered_rec_pos);
                    if ((PTX_STATUS_SUCCESS == optional_status) && (1u == optional_rec_len))
                    {
                        optional_status = ptx_NDEFRecord_WLCSTAI_Parse(&optional_records[filtered_rec_pos[0]], &ctx->RecordWlcStai);
                        if (PTX_STATUS_SUCCESS == optional_status)
                        {
                            ctx->RecordWlcStaiAvailable = 1u;
                            ptx_RUL_WLC_NtfCallback (ctx, Notification_StatusInfoRecordReceived, &ctx->RecordWlcStai, sizeof(ptx_NDEFRecord_WLCSTAI_t));
                        }
                    }
                }
                else
                {
                    ctx->RecordWlcCtlValid = 0;
                }
            }
            else
            {
                /* No/Multiple CTL field available. Check if WLCINF record is still within the tags memory, which indicates that we have not waited long enough. */
                ctx->RecordWlcCtlValid = 0;

                filtered_rec_len = (records_len < PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE ? records_len : PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE);
                status = ptx_NDEFMessage_FilterRecordsByType(record_buffer, &filtered_rec_len, PTX_NDEFRECORD_TYPE_RTD_WLCINF, PTX_NDEFRECORD_TYPE_RTD_WLCINF_LEN, filtered_rec_pos);

                if ( (PTX_STATUS_SUCCESS == status) && (1u == filtered_rec_len) )
                {
                    /* In case the Listener didn't manage to update it's NDEF message in time by replacing thw WLCINF
                     * record, the WLC-P has to re-read the memory after a specified waiting time.
                     */

                    ctx->CntrWlcCtlNwcc++;
                    if (ctx->CntrWlcCtlNwcc <= ctx->InitParams.WlcCtrlMaxReadRetries)
                    {
                        uint8_t waiting_time_ms = ctx->InitParams.WlcCtrlReadRetryDelay;
                        ptx_RUL_Sleep(ctx->InitParams.Rul, waiting_time_ms);

                        ctx->State = State_WCC_ReadControlRecord;
                    }
                }

                if ((1u != filtered_rec_len) ||
                    (ctx->CntrWlcCtlNwcc > ctx->InitParams.WlcCtrlMaxReadRetries))
                {
                    /* No/Multiple CTL field available after several retries. */
                    status = PTX_STATUS_WLC_CTL_RECORD_MISSING;
                }
            }
        }


    }

    return status;
}

static ptx_Status_t ptx_RUL_WLC_ParseWLCCTLRecord(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_ParseWLCCTLRecord");

        /* Start interpreting WLCCTL record. */
        uint8_t error;
        status = ptx_NDEFRecord_WLCCTL_GetErrorFlag(&ctx->RecordWlcCtl, &error);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Did the Listener report a protocol error? */
            if (!error)
            {
                /* The Listener's CTL record contains a counter. If this counter changes,
                 * we know that the CTL record was updated by the Listener and contains 'fresh' data.
                 */
                uint8_t ctl_ctr;
                status = ptx_NDEFRecord_WLCCTL_GetCounter(&ctx->RecordWlcCtl, &ctl_ctr);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Compare current counter value with previous one. */
                    if ( ((PTX_WLC_CTRL_CNTR_RESET_VALUE != ctx->WlcCtlCnt) && (ctx->WlcCtlCnt != ctl_ctr)) || ((ctx->WlcCtlCnt == PTX_WLC_CTRL_CNTR_RESET_VALUE) && (0x00 == ctl_ctr)) )
                    {
                        /* Save current counter value for next cycle. */
                        ctx->WlcCtlCnt = ctl_ctr;

                        uint8_t rd_conf;
                        status = ptx_NDEFRecord_WLCCAP_GetRdConf(&ctx->RecordWlcCap, &rd_conf);

                        /* Check if Listener requires a read confirmation (implemented by writing an empty NDEF record). */
                        if ((PTX_STATUS_SUCCESS == status) && (rd_conf))
                        {
                            /* Waiting time will be between 1ms and 10ms (+ 1 ms margin delay) before writing
                             * the empty record.
                             */
                            uint8_t wr_wt_int;
                            status = ptx_NDEFRecord_WLCCAP_GetWrWtInt(&ctx->RecordWlcCap, &wr_wt_int);
                            if (PTX_STATUS_SUCCESS == status)
                            {
                                wr_wt_int++;
                                status = ptx_RUL_Sleep(ctx->InitParams.Rul, wr_wt_int);
                            }

                            /* Write empty record to confirm that data has been read. */
                            ptx_NDEFRecord_t empty_record;
                            if (PTX_STATUS_SUCCESS == status)
                            {
                                status = ptx_NDEFRecord_CreateEmptyRecord(&empty_record);
                            }

                            if (PTX_STATUS_SUCCESS == status)
                            {
                                status = ptx_RUL_WLC_WriteNdefMessage(ctx, &empty_record, 1u);
                            }
                        }

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            uint8_t wpt_request;
                            status = ptx_NDEFRecord_WLCCTL_GetWptReq(&ctx->RecordWlcCtl, &wpt_request);
                            if (PTX_STATUS_SUCCESS == status)
                            {
                                ctx->CntrWlcCtlNwcc = 0;
                                /* Check if a WPT cycle is requested. */
                                if (wpt_request)
                                {
                                    ctx->State = State_WPT_StartNegotiatedCharging;
                                }
                                else
                                {
                                    /* Deactivate Listener & turn off RF-field for a sleep waiting time duration. */
                                    ctx->State = State_Sleep_StartSleep;
                                }
                            }
                        }
                    }
                    else
                    {
                        /* In case the Listener didn't manage to update it's NDEF message in time the
                         * WLC-P has to retry after a specified waiting time.
                         */
                        ctx->CntrWlcCtlNwcc++;
                        if (ctx->CntrWlcCtlNwcc <= ctx->InitParams.WlcCtrlMaxReadRetries)
                        {
                            uint8_t waiting_time_ms = ctx->InitParams.WlcCtrlReadRetryDelay;

                            status = ptx_RUL_Sleep(ctx->InitParams.Rul, waiting_time_ms);

                            ctx->State = State_WCC_ReadControlRecord;
                        }
                        else
                        {
                            status = PTX_STATUS_WLC_CTL_MAX_NUM_RETRIES;
                        }
                    }
                }
            }
            else
            {
                status = PTX_STATUS_WLC_PROTOCOL_ERROR;
            }
        }
    }

    return status;
}

static uint8_t ptx_RUL_WLC_TxMultiplierToPercent(uint8_t reg)
{

    uint32_t res = reg * ((PTX_WLC_PERCENT_FRACTION << PTX_WLC_PRECISION_SHIFT) / PTX_WLC_TX_MULTIPLIER_MAX);
    return (uint8_t) (res >> PTX_WLC_PRECISION_SHIFT);
}

static ptx_Status_t ptx_RUL_WLC_ComputeRequiredTxMultiplier(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_ComputeRequiredTxMultiplier");

        int8_t power_adj;
        /* Query the desired power adjust level. */
        status = ptx_NDEFRecord_WLCCTL_GetPowerAdjReq(&ctx->RecordWlcCtl, &power_adj);

        ptx_RUL_WLC_ListenerType_t listenerType;
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_WLC_GetListenerType(ctx, &listenerType);  
        }           
        
        if (PTX_STATUS_SUCCESS == status)
        {
            int32_t required_tx_multiplier = 0;

            if(ListenerType_Generic == listenerType)
            {
                /**
                 * The following section contains the regulation calculation for the NFC Forum compliant power regulation.
                 */

                /* 
                 * Compute required power level for next WPT cycle:
                 * In that case the TxMultiplier used for the previous Wireless Power Transfer (WPT) cycle, is serving as the
                 * basis of comparison for determining the power level of the upcoming WPT, since the Listener requests 
                 * relative adjustments (more/less power) to it.
                 * 
                 * For the very first WPT cycle, the basis depends on the initialization parameter WptPowerLevelInitial:
                 * - If set to 0, the default polling power level from the RF configuration is used.
                 * - If set to a non-zero value, it directly sets the initial power level for comparison.
                 */
                required_tx_multiplier = ((ctx->TxMultiplier * s_LUT[PTX_WLC_LUT_IDX_OFFSET + power_adj]) >> 8);
                int32_t reg_adj = required_tx_multiplier - ctx->TxMultiplier;

                /* Checks if the required adjusted is smaller than the minimum allowed adjustment
                 * in either direction (increase/decrease). */
                if(reg_adj < 0)
                {
                    /**
                     * In case we need to reduce the output power, we increment by one AFTER the
                     * decrease because wewant to avoid an undershoot of the requested powerlevel.
                     */
                    reg_adj++;
                }
                else if(reg_adj > 0)
                {
                    /**
                     * In case we need to increase the output power, we decrement by one AFTER the
                     * increase because we want to avoid an overshoot of the requested powerlevel.
                     */
                    reg_adj--;
                }

                required_tx_multiplier = (ctx->TxMultiplier + reg_adj);
            }
            else if(ListenerType_Proprietary == listenerType)
            {
                /**
                 * The following section contains the regulation calculation for the proprietary power regulation.
                 */
                
                required_tx_multiplier = ctx->TxMultiplier + power_adj;
            }
            else 
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
            }

            /* Check if tx_multiplier is within bounds, defined by customer. */
            const uint8_t min_tx_multiplier = ctx->InitParams.WptPowerLevelMin;
            if(required_tx_multiplier < min_tx_multiplier)
            {
                required_tx_multiplier = min_tx_multiplier;
            }

            /* Check if tx_multiplier is within bounds, defined by customer. */
            const uint8_t max_tx_multiplier = ctx->InitParams.WptPowerLevelMax;
            if(required_tx_multiplier > max_tx_multiplier)
            {
                required_tx_multiplier = max_tx_multiplier;
            }

            /* Set the tx_multiplier to be used for next WPT cycle. */
            ctx->TxMultiplier = (uint8_t) required_tx_multiplier;
        }
    }

    return status;
}

static void ptx_RUL_WLC_AddChargingTimeoutSafetyMargin(uint32_t *timeout)
{
    uint32_t safetyMargin = *timeout / PTX_WLC_CHARGING_TIMEOUT_SAFETY_MARGIN_PERCENT;

    if (safetyMargin < PTX_WLC_CHARGING_TIMEOUT_SAFETY_MARGIN_TOLERANCE)
    {
        safetyMargin = PTX_WLC_CHARGING_TIMEOUT_SAFETY_MARGIN_TOLERANCE;
    }

    *timeout = *timeout + safetyMargin;
}

static ptx_Status_t ptx_RUL_WLC_ProcessWptStopNtf(void *ctx, ptx_NSC_WLC_WptStop_Ntf_t *ntf, uint8_t *ntf_used)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_WLC_t *wlc = ctx;

    if (PTX_COMP_CHECK(wlc, Comp_WLC) && PTX_COMP_CHECK(wlc->InitParams.Rul, Comp_RUL) && (NULL != ntf) && (NULL != ntf_used))
    {
        PTX_LOG_V(wlc->Log, "ptx_RUL_WLC_ProcessWptStopNtf");

        switch (ntf->Reason)
        {
            case WptStopReason_Expired:
                wlc->WptState = WptState_Idle;
                break;

            case WptStopReason_ICD:
                wlc->WptState = WptState_ICD;
                break;

            default:
                wlc->WptState = WptState_Max;
                break;
        }

        status = PTX_STATUS_SUCCESS;

        *ntf_used = PTX_RUL_EXTENSION_NTF_USED;
    }

    return status;
}


static ptx_Status_t ptx_RUL_WLC_PresenceCheck(ptx_RUL_WLC_t *ctx, uint8_t *isAvailable)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL) && (NULL != isAvailable))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_PresenceCheck");

        *isAvailable = 0u;

        ptx_RUL_DiscoverConfig_t disc_config;
        memset(&disc_config, 0, sizeof(disc_config));

        disc_config.EnablePolling = 1u;
        disc_config.PollMode = 1u;
        disc_config.DisableNfcDepProtocol = 1u;
        disc_config.DisablePollIsoDep = 1u;
       
        /* Only poll for the RF-technology of the tag that was previously connected. */
        uint32_t presence_check_duration = 0;

        switch (ctx->Card.TechType)
        {
            case ptx_RUL_CardRfTechType_A:
                disc_config.PollTypeA = 1u;
                disc_config.PollTypeAPollFrequency = 1u;
                presence_check_duration = PTX_WLC_SLEEP_PRESENCE_CHECK_DURATION_TYPE_A;
                break;
            case ptx_RUL_CardRfTechType_B:
                disc_config.PollTypeB = 1u;
                disc_config.PollTypeBPollFrequency = 1u;
                presence_check_duration = PTX_WLC_SLEEP_PRESENCE_CHECK_DURATION_TYPE_B;
                break;
            case ptx_RUL_CardRfTechType_F:
                /**
                 * In the given case, the Poller shall poll for Type-F, but we have to find out 
                 * for what F-Bitrate in particular. --> Use the same setting from the user's provided
                 * InitParams.
                 */
                disc_config.PollTypeF212 = ctx->InitParams.NfcLeDiscoverConfig.PollTypeF212;
                disc_config.PollTypeF424 = ctx->InitParams.NfcLeDiscoverConfig.PollTypeF424;
                disc_config.PollTypeFPollFrequency = 1u;
                presence_check_duration = PTX_WLC_SLEEP_PRESENCE_CHECK_DURATION_TYPE_F;
                break;
            case ptx_RUL_CardRfTechType_V:
                disc_config.PollTypeV = 1u;
                disc_config.PollTypeVPollFrequency = 1u;
                presence_check_duration = PTX_WLC_SLEEP_PRESENCE_CHECK_DURATION_TYPE_V;
                break;
            default:
                break;
        }

        presence_check_duration *= PTX_WLC_SLEEP_PRESENCE_CHECK_TIMEOUT_FACTOR;


        /* Wakeup Poller from potential Standby/Sleep state. */
        status = ptx_RUL_SetPowerMode(ctx->InitParams.Rul, PowerMode_Wakeup);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_InitiateDiscovery(ctx->InitParams.Rul, &disc_config);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_Tmr_Start(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, HAL_DRV_Tmr_Unit_mSec, presence_check_duration);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t is_elapsed = 0u;
            ptx_RUL_Status_Params_t status_params;

            /* Poll until there are cards discovered. */
            do
            {
                status_params.Type = ptx_RUL_StatusType_Discover;

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_RUL_GetStatusInfo(ctx->InitParams.Rul, &status_params);
                }

                /* Check if the timeout is already reached. */
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_HAL_Tmr_IsElapsed(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, &is_elapsed);
                }

            }
            while ( (ptx_RUL_StatusInfoDiscover_CardDetectionOngoing == status_params.Info.Discover) ||
                    ((ptx_RUL_StatusInfoDiscover_NoCard == status_params.Info.Discover) && (0 == is_elapsed)) );
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_Tmr_Stop(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1);

            if (ctx->SleepRemainingDuration >= presence_check_duration)
            {
                ctx->SleepRemainingDuration -= presence_check_duration;
            }
            else
            {
                ctx->SleepRemainingDuration = 0;
            }
        }


        /* Search for the Listener in the card registry. */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_WLC_CheckListenerPresence(ctx, ctx->Card.TechParam, isAvailable);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_WLC_DeactivateListener(ctx);
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_WLC_CheckListenerPresence(ptx_RUL_WLC_t *ctx, ptx_RUL_CardRfTechParam_t cardParams, uint8_t *isAvailable)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL) && (NULL != isAvailable))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_CheckListenerPresence");

        *isAvailable = 0;

        ptx_RUL_CardRegistry_t *card_registry;
        status = ptx_RUL_GetCardRegistry(ctx->InitParams.Rul, &card_registry);


        if (PTX_STATUS_SUCCESS == status)
        {
            /*
             * Iterate through all available card params (--> card params contain the NFC ID).
             */
            for (uint8_t i = 0; i < card_registry->CardsCnt; ++i)
            {
                if (0u == memcmp(&card_registry->Cards[i].TechParam, &cardParams, sizeof(ptx_RUL_CardRfTechParam_t)))
                {
                    *isAvailable = 1;
                    break;
                }
            }
        }
    }

    return status;
}


static ptx_Status_t ptx_RUL_WLC_DeactivateListener(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_DeactivateListener");

        status = ptx_HAL_Tmr_Stop(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Permanently turn off RF-field and switch to Idle State. */
            ptx_RUL_DeactivateParams_t deact_params;
            deact_params.Type = ptx_RUL_DeactivateType_Generic;
            deact_params.State = ptx_RUL_DeactivateState_Idle;
            status = ptx_RUL_Deactivate(ctx->InitParams.Rul, &deact_params);
            ctx->InitParams.Rul->ActiveExtensionEndpoint = NULL;

            /* If the Deactivate command returns an invalid state error, this is accepted as a success.
             * It represents the case, that the chip is already in deactivated state.
             */
            if (PTX_STATUS_E_GEN_INVALID_STATE == status)
            {
                status = PTX_STATUS_SUCCESS;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Switch to Standby State. */
            status = ptx_RUL_SetPowerMode(ctx->InitParams.Rul, PowerMode_Standby);

            /* If the SetPowerMode command returns an invalid state error, this is accepted as a success.
             * It represents the case, that the chip is already in Standby.
             */
            if (PTX_STATUS_E_GEN_INVALID_STATE == status)
            {
                status = PTX_STATUS_SUCCESS;
            }
        }
    }

    return status;
}

static void ptx_RUL_WLC_NtfCallback(ptx_RUL_WLC_t *ctx, ptx_RUL_WLC_Notification_t Notification, void *data, uint32_t dataLen)
{
    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && PTX_COMP_CHECK(ctx->InitParams.Rul, Comp_RUL) && (NULL != ctx->InitParams.NtfCb))
    {
        PTX_LOG_V(ctx->Log, "ptx_RUL_WLC_NtfCallback");

        ctx->InitParams.NtfCb(ctx, Notification, data, dataLen);
    }
}
