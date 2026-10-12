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
    Module      : RUL
    File        : ptx_RUL_DataProcessor.c

    Description : Reader Universal Library (RUL) data processing implementation.
                  Contains functions to process notifications and commands related to RUL functionality.
 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_RUL_Cfg.h"
#include "ptx_RUL.h"
#include "ptx_RUL_Int.h"
#include <limits.h>

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


static ptx_Status_t ptx_RUL_SetEmvCollision (ptx_RUL_t *rul);
static ptx_Status_t ptx_RUL_ReaderProcessRfDiscoverNtf (ptx_RUL_t *rul, ptx_NSC_RfDiscover_Ntf_t *ntf);
static ptx_Status_t ptx_RUL_ReaderProcessRfActivateNtf (ptx_RUL_t *rul, ptx_NSC_RfActivate_Ntf_t *ntf);
static ptx_Status_t ptx_RUL_SetActiveMode (ptx_RUL_t *rul, ptx_NSC_RfActivate_Ntf_t *ntf);
static ptx_Status_t ptx_RUL_SetIdleMode (ptx_RUL_t *rul, ptx_NSC_RfDeactivate_Ntf_t *ntf);
#if defined(ENABLE_EXTENDED_CARD_REGISTER)
static ptx_Status_t ptx_RUL_ParseSensBRes (ptx_RUL_CardParams_t *discovered_card, uint8_t *sensBRes);
static ptx_Status_t ptx_RUL_ParseSensFRes (ptx_RUL_CardParams_t *discovered_card, uint8_t *sensFRes);
static ptx_Status_t ptx_RUL_ParseInventoryRes (ptx_RUL_CardParams_t *discovered_card, uint8_t *inventoryRes);
static ptx_Status_t ptx_RUL_ParseP2PAcMAtrRes (ptx_RUL_CardParams_t *discovered_card, uint8_t *atrRes);
#endif

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_RUL_ProcessRfDiscoverNtf (void *ctx, ptx_NSC_RfDiscover_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_t *rul = (ptx_RUL_t*)(ctx);

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != ntf))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_ProcessRfDiscoverNtf");

        status = PTX_STATUS_SUCCESS;

        for (uint8_t i = 0; (i < PTX_RUL_MAX_EXTENSIONS) && (PTX_STATUS_SUCCESS == status); i++)
        {
            if ((Comp_None != rul->ExtensionEndpoints[i].CompId) && (NULL != rul->ExtensionEndpoints[i].ProcessRfDiscover))
            {
                status = rul->ExtensionEndpoints[i].ProcessRfDiscover(rul->ExtensionEndpoints[i].Extension, ntf);
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_ReaderProcessRfDiscoverNtf(rul, ntf);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_ProcessRfLpcdNtf (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_t *rul = (ptx_RUL_t*)(ctx);

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_ProcessRfLpcdNtf");

        status = PTX_STATUS_SUCCESS;

        uint8_t ntf_used = PTX_RUL_EXTENSION_NTF_UNUSED;
        ptx_RUL_Extension_Endpoints_t *active_endpoint = rul->ActiveExtensionEndpoint;

        if ((NULL != active_endpoint) && (NULL != active_endpoint->ProcessRfLpcd))
        {
            status = active_endpoint->ProcessRfLpcd(active_endpoint->Extension, &ntf_used);
        }

        if ((PTX_STATUS_SUCCESS == status) && (PTX_RUL_EXTENSION_NTF_UNUSED == ntf_used))
        {
            if (rul->LpcdStatus.LpcdNtfCounter < UCHAR_MAX)
            {
                ++rul->LpcdStatus.LpcdNtfCounter;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_ProcessResetNtf (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_t *rul = (ptx_RUL_t*)(ctx);

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_ProcessResetNtf");

        status = PTX_STATUS_SUCCESS;

        /* Last received NSC message is the Reset Notification (ResetNtf) in this case. */
        uint32_t rst_ntf_len = rul->Nsc.CmdProcessor.LastMsg.Length;
        uint8_t *rst_ntf_payload = rul->Nsc.CmdProcessor.LastMsg.Payload;
        /*
         * ROM version field is optional depending on silicon revision.
         * Some devices send a shorter ResetNtf that does not include the ROM version byte.
         */
        if(PTX_NSC_CMD_PROCESSOR_RESET_NTFLEN_MAX == rst_ntf_len)
        {
            rul->Nsc.RomVersion = rst_ntf_payload[PTX_NSC_RESET_NTF_ROMVERSION_IDX];
        }
        else 
        {
            /* ROM version not present in this ResetNtf. */
            rul->Nsc.RomVersion = 0;
        }

        for (uint8_t i = 0; (i < PTX_RUL_MAX_EXTENSIONS) && (PTX_STATUS_SUCCESS == status); i++)
        {
            if ((Comp_None != rul->ExtensionEndpoints[i].CompId) && (NULL != rul->ExtensionEndpoints[i].ProcessReset))
            {
                status = rul->ExtensionEndpoints[i].ProcessReset(rul->ExtensionEndpoints[i].Extension);
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_ClearCardRegistry(rul);
            rul->SystemStatus = ptx_RUL_StatusInfoSystem_NSCReset;
        }

    }

    return status;
}

ptx_Status_t ptx_RUL_ProcessRfActivateNtf (void *ctx, ptx_NSC_RfActivate_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_t *rul = (ptx_RUL_t*)(ctx);

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != ntf))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_ProcessRfActivateNtf");

        status = ptx_RUL_SetActiveMode (rul, ntf);

        uint8_t ntf_used = PTX_RUL_EXTENSION_NTF_UNUSED;
        for (uint8_t i = 0; (i < PTX_RUL_MAX_EXTENSIONS) && (PTX_STATUS_SUCCESS == status) && (PTX_RUL_EXTENSION_NTF_UNUSED == ntf_used); i++)
        {
            if ((Comp_None != rul->ExtensionEndpoints[i].CompId) && (NULL != rul->ExtensionEndpoints[i].ProcessRfActivate))
            {
                status = rul->ExtensionEndpoints[i].ProcessRfActivate(rul->ExtensionEndpoints[i].Extension, ntf, &ntf_used);

                if (ntf_used)
                {
                    rul->ActiveExtensionEndpoint = &rul->ExtensionEndpoints[i];
                }
            }
        }

        if ((PTX_STATUS_SUCCESS == status) && (PTX_RUL_EXTENSION_NTF_UNUSED == ntf_used))
        {
            rul->ActiveExtensionEndpoint = NULL;
            status = ptx_RUL_ReaderProcessRfActivateNtf(rul, ntf);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_ProcessRfDeactivateNtf (void *ctx, ptx_NSC_RfDeactivate_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_t *rul = (ptx_RUL_t*)(ctx);

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != ntf))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_ProcessRfDeactivateNtf");


        status = ptx_RUL_SetIdleMode (rul, ntf);

        uint8_t ntf_used = PTX_RUL_EXTENSION_NTF_UNUSED;
        ptx_RUL_Extension_Endpoints_t *active_endpoint = rul->ActiveExtensionEndpoint;

        if ((PTX_STATUS_SUCCESS == status) && (NULL != active_endpoint) && (NULL != active_endpoint->ProcessRfDeactivate))
        {
            status = active_endpoint->ProcessRfDeactivate(active_endpoint->Extension, ntf, &ntf_used);
        }

        if ((PTX_STATUS_SUCCESS == status) && (PTX_RUL_EXTENSION_NTF_UNUSED == ntf_used))
        {
            if (NSC_ErrorCode_NoErr == ntf->Error)
            {
                if (ptx_NSC_RfDeactivate_Reason_Finished == ntf->Reason)
                {
                    /* Deactivation in Poll mode */
                    (void)ptx_RUL_ClearCardRegistry(rul);

                    rul->EmvRemovalStatus = ptx_RUL_StatusInfoEmvRemoval_Complete;
                    rul->DeactivateStatus = ptx_RUL_StatusInfoDeactivate_Complete;
                    rul->RfErrorStatus = ptx_RUL_StatusInfoRfError_ErrorNone;

                    rul->CardRegistry.ActiveCard = NULL;
                    rul->CardRegistry.ActiveProtocol = ptx_RUL_CardProtocolType_Undefined;
                    rul->CardRegistry.ActiveCardId = 0;
                }
                else if ((ptx_NSC_RfDeactivate_Reason_Deselect == ntf->Reason) ||
                         (ptx_NSC_RfDeactivate_Reason_Release == ntf->Reason) ||
                         (ptx_NSC_RfDeactivate_Reason_RF_Field == ntf->Reason))
                {
                    /* Deactivation in Listen mode */
                }
                else
                {
                    /* RFU */
                    rul->EmvRemovalStatus = ptx_RUL_StatusInfoEmvRemoval_Error;
                    rul->DeactivateStatus = ptx_RUL_StatusInfoDeactivate_Error;
                }

                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_ProcessErrorNtf (void *ctx, ptx_NSC_Error_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_t *rul = (ptx_RUL_t*)(ctx);
    uint8_t is_rf_error = 0;

    ptx_RUL_StatusInfoRfError_t     tmp_rferror_stat = ptx_RUL_StatusInfoRfError_ErrorNone;
    ptx_RUL_StatusInfoDiscover_t    tmp_disc_stat = ptx_RUL_StatusInfoDiscover_NoCard;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != ntf))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_ProcessErrorNtf");

        status = PTX_STATUS_SUCCESS;

        uint8_t ntf_used = PTX_RUL_EXTENSION_NTF_UNUSED;
        ptx_RUL_Extension_Endpoints_t *active_endpoint = rul->ActiveExtensionEndpoint;

        if ((NULL != active_endpoint) && (NULL != active_endpoint->ProcessError))
        {
            status = active_endpoint->ProcessError(active_endpoint->Extension, ntf, &ntf_used);
        }

        if ((PTX_STATUS_SUCCESS == status) && (PTX_RUL_EXTENSION_NTF_UNUSED == ntf_used))
        {
            switch (ntf->Error)
            {
                case NSC_ErrorCode_RfCollisionError:
                    status = ptx_RUL_SetEmvCollision(rul);
                    /* Temporarily store current error and discovery status. */
                    tmp_rferror_stat = rul->RfErrorStatus;
                    tmp_disc_stat = rul->DiscoverStatus;
                    is_rf_error = 1u;
                    break;
                case NSC_ErrorCode_RfTimeoutError:
                    rul->RfErrorStatus = ptx_RUL_StatusInfoRfError_Timeout;
                    is_rf_error = 1u;
                    break;
                case NSC_ErrorCode_RfTransmissionError:
                    rul->RfErrorStatus = ptx_RUL_StatusInfoRfError_Transmission;
                    is_rf_error = 1u;
                    break;
                case NSC_ErrorCode_RfProtocolError:
                    rul->RfErrorStatus = ptx_RUL_StatusInfoRfError_Protocol;
                    is_rf_error = 1u;
                    break;
                case NSC_ErrorCode_OTPConfig:
                    rul->SystemStatus = ptx_RUL_StatusInfoSystem_ErrorOTP;
                    break;
                case NSC_ErrorCode_VddTx:
                    rul->SystemStatus = ptx_RUL_StatusInfoSystem_ErrorVddTx;
                    break;
                case NSC_ErrorCode_OverCurrent:
                    rul->SystemStatus = ptx_RUL_StatusInfoSystem_ErrorOvercurrent;
                    break;
                case NSC_ErrorCode_OverTemperature:
                    rul->SystemStatus = ptx_RUL_StatusInfoSystem_ErrorTemperature;
                    break;
                case NSC_ErrorCode_OutOfMemory:
                    rul->SystemStatus = ptx_RUL_StatusInfoSystem_ErrorOutOfMemory;
                    break;
                case NSC_ErrorCode_StandbyMode:
                    rul->SystemStatus = ptx_RUL_StatusInfoSystem_ErrorStandby;
                    break;

                default:
                    rul->SystemStatus = ptx_RUL_StatusInfoSystem_Max;
                    break;
            }

            if (0 != is_rf_error)
            {
                switch (rul->Nsc.State)
                {
                    case ptx_NSC_State_Discovery:
                        (void) ptx_RUL_ClearCardRegistry(rul);

                        if (ptx_RUL_StatusInfoDiscover_EmvCollision == tmp_disc_stat)
                        {
                            /* Recover the information that is overwritten in ptx_RUL_ClearCardRegistry. */
                            rul->RfErrorStatus = tmp_rferror_stat;
                            rul->DiscoverStatus = tmp_disc_stat;
                        }
                        break;

                    default:
                        // RFU - do nothing
                        break;
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_ProcessRfFieldNtf (void *ctx, uint8_t field)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_t *rul = (ptx_RUL_t*)(ctx);

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_ProcessRfFieldNtf");
        status = PTX_STATUS_SUCCESS;

        uint8_t ntf_used = PTX_RUL_EXTENSION_NTF_UNUSED;

        uint8_t extension_index = 0;
        while ((extension_index < PTX_RUL_MAX_EXTENSIONS) && (PTX_STATUS_SUCCESS == status))
        {
            if ((Comp_None != rul->ExtensionEndpoints[extension_index].CompId) && (NULL != rul->ExtensionEndpoints[extension_index].ProcessRfActivate))
            {
                status = rul->ExtensionEndpoints[extension_index].ProcessRfField(rul->ExtensionEndpoints[extension_index].Extension, field, &ntf_used);
            }
            extension_index++;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_ProcessWlcWptStopNtf (void *ctx, ptx_NSC_WLC_WptStop_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_t *rul = (ptx_RUL_t*)(ctx);

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != ntf))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_ProcessWlcWptStopNtf");
        status = PTX_STATUS_SUCCESS;

        uint8_t ntf_used = PTX_RUL_EXTENSION_NTF_UNUSED;
        ptx_RUL_Extension_Endpoints_t *active_endpoint = rul->ActiveExtensionEndpoint;

        if ((NULL != active_endpoint) && (NULL != active_endpoint->ProcessWptStop))
        {
            status = active_endpoint->ProcessWptStop(active_endpoint->Extension, ntf, &ntf_used);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_ProcessDataMsg (void *ctx, ptx_NSC_RfData_Msg_t *msg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_t *rul = (ptx_RUL_t*)(ctx);

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_ProcessDataMsg");
        status = PTX_STATUS_SUCCESS;

        ptx_RUL_Extension_Endpoints_t *active_endpoint = rul->ActiveExtensionEndpoint;

        if ((NULL != active_endpoint) && (NULL != active_endpoint->ProcessDataMsg))
        {
            status = active_endpoint->ProcessDataMsg(active_endpoint->Extension, msg);
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_RUL_SetEmvCollision (ptx_RUL_t *rul)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_V(&rul->Log, "ptx_RUL_SetEmvCollision");
        rul->EmvCollisionFilter.Counter++;

        if (rul->EmvCollisionFilter.Counter >= rul->EmvCollisionFilter.Limit)
        {
            rul->RfErrorStatus = ptx_RUL_StatusInfoRfError_Collision;
            rul->DiscoverStatus = ptx_RUL_StatusInfoDiscover_EmvCollision;
            rul->EmvCollisionFilter.Counter = 0;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_RUL_ReaderProcessRfDiscoverNtf (ptx_RUL_t *rul, ptx_NSC_RfDiscover_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != ntf))
    {
        PTX_LOG_V(&rul->Log, "ptx_RUL_ReaderProcessRfDiscoverNtf");

        ptx_RUL_CardParams_t discovered_card;
        memset(&discovered_card, 0, sizeof(discovered_card));

        status = PTX_STATUS_SUCCESS;

        switch (ntf->RfTech)
        {
            case ptx_NSC_RfTech_Passive_Poll_A:
                discovered_card.TechType = ptx_RUL_CardRfTechType_A;

                discovered_card.State = (ptx_RUL_CardRfState_t) ntf->RfState;

                memcpy(discovered_card.TechParam.TypeA.SensRes, &ntf->PollParam.PollA.SensRes, PTX_RUL_SENSRES_LEN);

                discovered_card.TechParam.TypeA.NfcId1Len = ntf->PollParam.PollA.NfcId1Len;
                memcpy(discovered_card.TechParam.TypeA.NfcId1, &ntf->PollParam.PollA.NfcId1, ntf->PollParam.PollA.NfcId1Len);

                discovered_card.TechParam.TypeA.SelRes = ntf->PollParam.PollA.SelRes;

                break;

            case ptx_NSC_RfTech_Passive_Poll_B:
                discovered_card.TechType = ptx_RUL_CardRfTechType_B;
                discovered_card.State = (ptx_RUL_CardRfState_t) ntf->RfState;
                #if defined(ENABLE_EXTENDED_CARD_REGISTER)
                ptx_RUL_ParseSensBRes(&discovered_card, ntf->PollParam.PollB.SensBRes);
                #else  
                memcpy(discovered_card.TechParam.TypeB.SensBRes, ntf->PollParam.PollB.SensBRes, PTX_NSC_TYPES_SENSBRES_LEN);
                #endif
                break;

            case ptx_NSC_RfTech_Passive_Poll_F:
                discovered_card.TechType = ptx_RUL_CardRfTechType_F;
                #if defined(ENABLE_EXTENDED_CARD_REGISTER)
                ptx_RUL_ParseSensFRes(&discovered_card, ntf->PollParam.PollF.SensFRes);
                #else
                memcpy(discovered_card.TechParam.TypeF.SensFRes, ntf->PollParam.PollF.SensFRes, PTX_RUL_SENSFRES_LEN);
                #endif
                break;

            case ptx_NSC_RfTech_Passive_Poll_V:
                discovered_card.TechType = ptx_RUL_CardRfTechType_V;
                #if defined(ENABLE_EXTENDED_CARD_REGISTER)
                ptx_RUL_ParseInventoryRes(&discovered_card, ntf->PollParam.PollV.InventoryRes); 
                #else
                memcpy(discovered_card.TechParam.TypeV.InventoryRes, ntf->PollParam.PollV.InventoryRes, PTX_NSC_TYPES_INVENTORYRES_LEN);
                #endif
                break;

            default:
                /* This path shall only be reached in case of the last discover Ntf. */
                status = ntf->IsLastNtf ? PTX_STATUS_SUCCESS : PTX_STATUS_E_GEN_INVALID_STATE;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 == ntf->IsLastNtf)
            {
                (void) ptx_RUL_DetermineProtocol(rul, &discovered_card);

                status = ptx_RUL_AddToCardRegistry(rul, &discovered_card);

                rul->DiscoverStatus = ptx_RUL_StatusInfoDiscover_CardDetectionOngoing;
            }
            else
            {
                rul->CardRegistry.ActiveCard = NULL;

                /* If only one card was found, but support multiple-protocols - set Multi-Protocol-state, otherwise multiple cards are available. */
                if (1u == rul->CardRegistry.CardsCnt)
                {
                    rul->DiscoverStatus = ptx_RUL_StatusInfoDiscover_MultiProtocol;
                }
                else
                {
                    rul->DiscoverStatus = ptx_RUL_StatusInfoDiscover_CardsAvailable;

                    /* Special case for EMV Mode - report any additional cards (also non EMV-compliant cards) as EMV-Collision.
                    * The status and the error have to be reset here.
                    */
                    if (ptx_RUL_PollMode_Emvco == rul->DiscoverConfig.PollMode)
                    {
                        rul->DiscoverStatus = ptx_RUL_StatusInfoDiscover_EmvCollision;
                    }
                }
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_ReaderProcessRfActivateNtf (ptx_RUL_t *rul, ptx_NSC_RfActivate_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != ntf))
    {
        PTX_LOG_V(&rul->Log, "ptx_RUL_ReaderProcessRfActivateNtf");

        uint8_t write_to_card_registry = 1;
        ptx_RUL_CardParams_t discovered_card;
        memset(&discovered_card, 0, sizeof(discovered_card));

        status = PTX_STATUS_SUCCESS;

        discovered_card.State = ptx_RUL_CardRfState_Active;

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (ntf->RfTech)
            {
                case ptx_NSC_RfTech_Passive_Poll_A:
                    discovered_card.TechType = ptx_RUL_CardRfTechType_A;

                    memcpy(discovered_card.TechParam.TypeA.SensRes, &ntf->RfTechParams.PollA.SensRes, PTX_RUL_SENSRES_LEN);

                    discovered_card.TechParam.TypeA.NfcId1Len = ntf->RfTechParams.PollA.NfcId1Len;
                    memcpy(discovered_card.TechParam.TypeA.NfcId1, &ntf->RfTechParams.PollA.NfcId1, ntf->RfTechParams.PollA.NfcId1Len);

                    discovered_card.TechParam.TypeA.SelRes = ntf->RfTechParams.PollA.SelRes;

                    /* Default Bitrate - may be overwritten later. */
                    discovered_card.TechParam.TypeA.TxBitRate = ptx_RUL_RfBitRate_106;
                    discovered_card.TechParam.TypeA.RxBitRate = ptx_RUL_RfBitRate_106;
                    break;

                case ptx_NSC_RfTech_Passive_Poll_B:
                    discovered_card.TechType = ptx_RUL_CardRfTechType_B;
                    #if defined(ENABLE_EXTENDED_CARD_REGISTER)
                    ptx_RUL_ParseSensBRes(&discovered_card, ntf->RfTechParams.PollB.SensBRes);
                    #else
                    memcpy(discovered_card.TechParam.TypeB.SensBRes, ntf->RfTechParams.PollB.SensBRes, PTX_NSC_TYPES_SENSBRES_LEN);
                    /* Default Bitrate - may be overwritten later. */
                    discovered_card.TechParam.TypeB.TxBitRate = ptx_RUL_RfBitRate_106;
                    discovered_card.TechParam.TypeB.RxBitRate = ptx_RUL_RfBitRate_106;
                    #endif
                    break;

                case ptx_NSC_RfTech_Passive_Poll_F:
                    discovered_card.TechType = ptx_RUL_CardRfTechType_F;
                    #if defined(ENABLE_EXTENDED_CARD_REGISTER)
                    ptx_RUL_ParseSensFRes(&discovered_card, ntf->RfTechParams.PollF.SensFRes); 
                    #else
                    memcpy(discovered_card.TechParam.TypeF.SensFRes, ntf->RfTechParams.PollF.SensFRes, PTX_NSC_TYPES_SENSFRES_LEN);
                    /* Default Bitrate - may be overwritten later. */
                    discovered_card.TechParam.TypeF.BitRate = ptx_RUL_RfBitRate_212;
                    #endif
                    break;

                case ptx_NSC_RfTech_Passive_Poll_V:
                    discovered_card.TechType = ptx_RUL_CardRfTechType_V;
                    #if defined(ENABLE_EXTENDED_CARD_REGISTER)
                    ptx_RUL_ParseInventoryRes(&discovered_card, ntf->RfTechParams.PollV.InventoryRes);  
                    #else
                    memcpy(discovered_card.TechParam.TypeV.InventoryRes, ntf->RfTechParams.PollV.InventoryRes, PTX_NSC_TYPES_INVENTORYRES_LEN);
                    /* Default and only Bitrate. */
                    discovered_card.TechParam.TypeV.BitRate = ptx_RUL_RfBitRate_26;
                    #endif
                    break;

                case PTX_NSC_TYPES_TECH_POLL_ACM:
                    #if defined(ENABLE_EXTENDED_CARD_REGISTER)
                    discovered_card.TechType = ptx_RUL_CardRfTechType_ACM;
                    ptx_RUL_ParseP2PAcMAtrRes(&discovered_card,ntf->RfProtParams.NfcDep.PollAF.AtrRes); 
                    #else   
                    discovered_card.TechType = ptx_RUL_CardRfTechType_ACM;
                    write_to_card_registry = 1;
                    #endif
                    break;
                    
                case ptx_NSC_RfTech_Passive_Listen_A:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                    /* No parameters available */
                    write_to_card_registry = 0;
                    break;
#endif /* PTX_DISABLE_LISTEN_MODE */

                case ptx_NSC_RfTech_Passive_Listen_B:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                    /* No parameters available */
                    write_to_card_registry = 0;
                    break;
#endif /* PTX_DISABLE_LISTEN_MODE */

                case ptx_NSC_RfTech_Passive_Listen_F:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                    /* No parameters available */
                    write_to_card_registry = 0;
                    break;
#endif /* PTX_DISABLE_LISTEN_MODE */

                default:
                    write_to_card_registry = 0;
                    break;
            }

            switch (ntf->RfProt)
            {
                case ptx_NSC_RfProtocol_T2T:
                    rul->CardRegistry.ActiveProtocol = ptx_RUL_CardProtocolType_T2T;
                    break;

                case ptx_NSC_RfProtocol_T3T:
                    rul->CardRegistry.ActiveProtocol = ptx_RUL_CardProtocolType_T3T;
                    break;

                case ptx_NSC_RfProtocol_IsoDep:
                    rul->CardRegistry.ActiveProtocol = ptx_RUL_CardProtocolType_T4T;
                    if (ptx_NSC_RfTech_Passive_Poll_A == ntf->RfTech)
                    {
                        (void)memcpy(rul->CardRegistry.ActiveProtParam.PollAIsoDep.ATS, ntf->RfProtParams.IsoDep.PollA.ATS, PTX_RUL_CARD_RFPROTOCOL_ATS_LEN);
                        rul->CardRegistry.ActiveProtParam.PollAIsoDep.PPS1 = ntf->RfProtParams.IsoDep.PollA.PPS1;

                        discovered_card.TechParam.TypeA.TxBitRate = (ntf->RfProtParams.IsoDep.PollA.PPS1 >> 0) & (uint8_t)0x03;
                        discovered_card.TechParam.TypeA.RxBitRate = (ntf->RfProtParams.IsoDep.PollA.PPS1 >> 2) & (uint8_t)0x03;
                    }
                    else if (ptx_NSC_RfTech_Passive_Poll_B == ntf->RfTech)
                    {
                        rul->CardRegistry.ActiveProtParam.PollBIsoDep.AttribParam1 = ntf->RfProtParams.IsoDep.PollB.AttribParam1;
                        rul->CardRegistry.ActiveProtParam.PollBIsoDep.AttribParam2 = ntf->RfProtParams.IsoDep.PollB.AttribParam2;
                        rul->CardRegistry.ActiveProtParam.PollBIsoDep.AttribResLength = ntf->RfProtParams.IsoDep.PollB.AttribResLen;
                        (void)memcpy(rul->CardRegistry.ActiveProtParam.PollBIsoDep.AttribRes, ntf->RfProtParams.IsoDep.PollB.AttribRes, PTX_RUL_CARD_RFPROTOCOL_ATTRIBRES_LEN);

                        discovered_card.TechParam.TypeB.TxBitRate = (ntf->RfProtParams.IsoDep.PollB.AttribParam2 >> 4) & (uint8_t)0x03;
                        discovered_card.TechParam.TypeB.RxBitRate = (ntf->RfProtParams.IsoDep.PollB.AttribParam2 >> 6) & (uint8_t)0x03;
                    }
                    else if (ptx_NSC_RfTech_Passive_Listen_A == ntf->RfTech)
                    {
                        rul->CardRegistry.ActiveProtParam.ListenAIsoDep.Pps1 = ntf->RfProtParams.IsoDep.ListenA.Pps1;
                        rul->CardRegistry.ActiveProtParam.ListenAIsoDep.Rats = ntf->RfProtParams.IsoDep.ListenA.Rats;

                        discovered_card.TechParam.TypeA.TxBitRate = (ntf->RfProtParams.IsoDep.ListenA.Pps1 >> 0) & (uint8_t)0x03;
                        discovered_card.TechParam.TypeA.RxBitRate = (ntf->RfProtParams.IsoDep.ListenA.Pps1 >> 2) & (uint8_t)0x03;
                    }
                    else if (ptx_NSC_RfTech_Passive_Listen_B == ntf->RfTech)
                    {
                        rul->CardRegistry.ActiveProtParam.ListenBIsoDep.AttribLen = ntf->RfProtParams.IsoDep.ListenB.AttribLen;
                        (void)memcpy(rul->CardRegistry.ActiveProtParam.ListenBIsoDep.Attrib, ntf->RfProtParams.IsoDep.ListenB.Attrib, PTX_RUL_CARD_RFPROTOCOL_ATTRIB_LEN);
                    }
                    break;

                case ptx_NSC_RfProtocol_NfcDep:
                    rul->CardRegistry.ActiveProtocol = ptx_RUL_CardProtocolType_NfcDep;
                    if (ptx_NSC_RfTech_Passive_Poll_A == ntf->RfTech)
                    {
                        rul->CardRegistry.ActiveProtParam.PollAFACMNfcDep.Brs = ntf->RfProtParams.NfcDep.PollAF.Brs;
                        (void)memcpy(rul->CardRegistry.ActiveProtParam.PollAFACMNfcDep.AtrRes, ntf->RfProtParams.NfcDep.PollAF.AtrRes, PTX_RUL_CARD_RFPROTOCOL_ATR_RES_LEN);

                        if (PTX_RUL_NFCDEP_BRS_UNUSED != ntf->RfProtParams.NfcDep.PollAF.Brs)
                        {
                            uint8_t dri = 0x00;
                            uint8_t dsi = 0x00;
                            dri = (ntf->RfProtParams.NfcDep.PollAF.Brs >> 0) & (uint8_t)0x07;
                            dsi = (ntf->RfProtParams.NfcDep.PollAF.Brs >> 3) & (uint8_t)0x07;
                            if ((dri == dsi) && (dri <= PTX_RUL_NFCDEP_BRS_MAX_SPEED_ALLOWED))
                            {
                                discovered_card.TechParam.TypeA.RxBitRate = dri;
                                discovered_card.TechParam.TypeA.TxBitRate = dsi;

                            }
                            else
                            {
                                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                            }
                        }
                        else
                        {
                            discovered_card.TechParam.TypeA.TxBitRate = ptx_RUL_RfBitRate_106;
                            discovered_card.TechParam.TypeA.RxBitRate = ptx_RUL_RfBitRate_106;
                        }
                    }
                    if (ptx_NSC_RfTech_Passive_Poll_F == ntf->RfTech)
                    {
                        rul->CardRegistry.ActiveProtParam.PollAFACMNfcDep.Brs = ntf->RfProtParams.NfcDep.PollAF.Brs;
                        (void)memcpy(rul->CardRegistry.ActiveProtParam.PollAFACMNfcDep.AtrRes, ntf->RfProtParams.NfcDep.PollAF.AtrRes, PTX_RUL_CARD_RFPROTOCOL_ATR_RES_LEN);

                        if (PTX_RUL_NFCDEP_BRS_UNUSED != ntf->RfProtParams.NfcDep.PollAF.Brs)
                        {
                            uint8_t dri = 0x00;
                            uint8_t dsi = 0x00;
                            dri = (ntf->RfProtParams.NfcDep.PollAF.Brs >> 0) & (uint8_t)0x07;
                            dsi = (ntf->RfProtParams.NfcDep.PollAF.Brs >> 3) & (uint8_t)0x07;
                            if ((dri == dsi) && (dri <= PTX_RUL_NFCDEP_BRS_MAX_SPEED_ALLOWED))
                            {
                                discovered_card.TechParam.TypeF.BitRate = dri;
                            }
                            else
                            {
                                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                            }
                        }
                        else
                        {
                            discovered_card.TechParam.TypeF.BitRate = ptx_RUL_RfBitRate_212;
                        }
                    }
                    if (ptx_NSC_RfTech_Active_Poll == ntf->RfTech)
                    {
                        rul->CardRegistry.ActiveProtParam.PollAFACMNfcDep.Brs = ntf->RfProtParams.NfcDep.PollAF.Brs;
                        (void)memcpy(rul->CardRegistry.ActiveProtParam.PollAFACMNfcDep.AtrRes, ntf->RfProtParams.NfcDep.PollAF.AtrRes, PTX_RUL_CARD_RFPROTOCOL_ATR_RES_LEN);
                    }
#if !defined(PTX_DISABLE_LISTEN_MODE)
                    else if (ptx_NSC_RfTech_Passive_Listen_F == ntf->RfTech)
                    {
                        rul->CardRegistry.ActiveProtParam.ListenAFNfcDep.Brs = ntf->RfProtParams.NfcDep.ListenAF.Brs;
                        rul->CardRegistry.ActiveProtParam.ListenAFNfcDep.Lr = ntf->RfProtParams.NfcDep.ListenAF.Lr;
                        (void)memcpy(rul->CardRegistry.ActiveProtParam.ListenAFNfcDep.AtrReq, ntf->RfProtParams.NfcDep.ListenAF.AtrReq, PTX_RUL_CARD_RFPROTOCOL_ATR_REQ_LEN);
                    }
#endif /* PTX_DISABLE_LISTEN_MODE */
                    break;

                case ptx_NSC_RfProtocol_T5T:
                    rul->CardRegistry.ActiveProtocol = ptx_RUL_CardProtocolType_T5T;
                    break;

                default:
                    rul->CardRegistry.ActiveProtocol = ptx_RUL_CardProtocolType_Undefined;
                    break;
            }

            if (1 == write_to_card_registry)
            {
                (void) ptx_RUL_DetermineProtocol(rul, &discovered_card);
                (void) ptx_RUL_AddToCardRegistry(rul, &discovered_card);
                rul->CardRegistry.ActiveCard = &rul->CardRegistry.Cards[0];
                rul->CardRegistry.ActiveCardId = 0;
            }

            rul->DiscoverStatus = ptx_RUL_StatusInfoDiscover_Active;
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_SetActiveMode (ptx_RUL_t *rul, ptx_NSC_RfActivate_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != ntf))
    {
        PTX_LOG_V(&rul->Log, "ptx_RUL_SetActiveMode");
        status = PTX_STATUS_SUCCESS;

        switch (ntf->RfTech)
        {
            case ptx_NSC_RfTech_Passive_Poll_A: /* FALL THROUGH */
            case ptx_NSC_RfTech_Passive_Poll_B: /* FALL THROUGH */
            case ptx_NSC_RfTech_Passive_Poll_F: /* FALL THROUGH */
            case ptx_NSC_RfTech_Passive_Poll_V:
                if ( (ptx_NSC_RfProtocol_NfcDep == ntf->RfProt) &&
                     ((ptx_NSC_RfTech_Passive_Poll_A == ntf->RfTech) || (ptx_NSC_RfTech_Passive_Poll_F == ntf->RfTech)) )
                {
                    rul->NfcMode = ptx_RUL_StatusInfoMode_P2P_Initiator;
                }
                else
                {
                    rul->NfcMode = ptx_RUL_StatusInfoMode_Poller;
                }
                break;
            case ptx_NSC_RfTech_Passive_Listen_A: /* FALL THROUGH */
            case ptx_NSC_RfTech_Passive_Listen_B: /* FALL THROUGH */
            case ptx_NSC_RfTech_Passive_Listen_F:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                if ( (ptx_NSC_RfProtocol_NfcDep == ntf->RfProt) &&
                     ((ptx_NSC_RfTech_Passive_Listen_A == ntf->RfTech) || (ptx_NSC_RfTech_Passive_Listen_F == ntf->RfTech)) )
                {
                    rul->NfcMode = ptx_RUL_StatusInfoMode_P2P_Target;
                }
                else
                {
                    rul->NfcMode = ptx_RUL_StatusInfoMode_Listener;
                }
#endif /* PTX_DISABLE_LISTEN_MODE */
                break;
            case ptx_NSC_RfTech_Active_Poll:
                rul->NfcMode = ptx_RUL_StatusInfoMode_P2P_Initiator;
                break;
            case ptx_NSC_RfTech_Active_Listen:
                rul->NfcMode = ptx_RUL_StatusInfoMode_P2P_Target;
                break;
            default:
                /* RFU */
                break;
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_SetIdleMode (ptx_RUL_t *rul, ptx_NSC_RfDeactivate_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != ntf))
    {
        PTX_LOG_V(&rul->Log, "ptx_RUL_SetIdleMode");
        if (NSC_ErrorCode_NoErr == ntf->Error)
        {
            /* Deactivation was successful. */
            rul->NfcMode = ptx_RUL_StatusInfoMode_Idle;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

#if defined(ENABLE_EXTENDED_CARD_REGISTER)

static ptx_Status_t ptx_RUL_ParseSensBRes (ptx_RUL_CardParams_t *discovered_card, uint8_t *sensBRes)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != sensBRes && discovered_card != NULL))
    {
        //Copy the raw SENSB_RES
        memcpy(discovered_card->TechParam.TypeB.SensBRes, sensBRes, PTX_NSC_TYPES_SENSBRES_LEN);

        //Set Type B SENSB_RES NFCID0 fields
        memcpy(discovered_card->TechParam.TypeB.Nfcid0, &sensBRes[PTX_RUL_SENSB_RES_NFCID0_OFFSET], PTX_RUL_NFCID0_LEN);
        
        //Set Type B SENSB_RES Application Data fields parameters
        ptx_RUL_SensBResponseAppData_t sensBResponseAppData;
        memset(&sensBResponseAppData, 0, sizeof(sensBResponseAppData));
        sensBResponseAppData.Afi =  sensBRes[PTX_RUL_SENSB_RES_AFI_OFFSET];
        sensBResponseAppData.NmbrOfApplications =  sensBRes[PTX_RUL_SENSB_RES_NUMBER_APPLICATIONS_OFFSET];
        memcpy(sensBResponseAppData.CrcBAid, &sensBRes[PTX_RUL_SENSB_RES_CRC_B_OFFSET], PTX_RUL_CRC_B_AID_LEN);
        discovered_card->TechParam.TypeB.appData= sensBResponseAppData;

        //Set Type B SENSB_RES Protocol Info Fields
        ptx_RUL_SensBResponseProtocolInfo_t sensBResProtocolInfo;
        memset(&sensBResProtocolInfo, 0, sizeof(sensBResProtocolInfo));
        ptx_RUL_BProtocolType_t protocolType;
        memset(&protocolType, 0, sizeof(protocolType));
        ptx_RUL_MinTr2B_t minTrb2; 
        ptx_RUL_BPlatformType_t platformType; 
        ptx_RUL_FsciToFsc_t fsci;
        ptx_RUL_BAdcCoding_t adcCoding; 
        ptx_RUL_BFoCoding_t foCoding;
        fsci=(ptx_RUL_FsciToFsc_t)(sensBRes[PTX_RUL_SENSB_FSCI_PROTOCOL_TYPE_OFFSET] & PTX_RUL_SENSB_FSCI_MASK ) >> 4 ;
        minTrb2= (ptx_RUL_MinTr2B_t)( ( (sensBRes[PTX_RUL_SENSB_FSCI_PROTOCOL_TYPE_OFFSET] & PTX_RUL_SENSB_PROTOCOL_TYPE_MASK) >> 1) & (uint8_t)0x03 ) ; 
        platformType= (ptx_RUL_BPlatformType_t) ( (sensBRes[PTX_RUL_SENSB_FSCI_PROTOCOL_TYPE_OFFSET] & PTX_RUL_SENSB_PROTOCOL_TYPE_MASK) & (uint8_t)0x01 ) ;
        adcCoding = (ptx_RUL_BAdcCoding_t) ((sensBRes[PTX_RUL_SENSB_FWI_ADC_FO_OFFSET] & PTX_RUL_SENSB_ADC_FO_MASK) >> 2); 
        foCoding = (ptx_RUL_BFoCoding_t) ((sensBRes[PTX_RUL_SENSB_FWI_ADC_FO_OFFSET] & PTX_RUL_SENSB_ADC_FO_MASK) & (uint8_t)0x03 );                    
        protocolType.minTr2B = minTrb2; 
        protocolType.platformType = platformType;
        sensBResProtocolInfo.BitRateCapability= sensBRes[PTX_RUL_SENSB_BIT_RATE_CAPABILITY_OFFSET];
        sensBResProtocolInfo.ProtocolType = protocolType;
        sensBResProtocolInfo.Fwi  = (sensBRes[PTX_RUL_SENSB_FWI_ADC_FO_OFFSET] & PTX_RUL_SENSB_FWI_MASK) >> 4 ;
        sensBResProtocolInfo.Sfgi = (sensBRes[PTX_RUL_SENSB_SFGI_OFFSET]) >> 4;
        sensBResProtocolInfo.Adc = adcCoding; 
        sensBResProtocolInfo.Fo = foCoding; 
        sensBResProtocolInfo.Fsci = fsci;
        discovered_card->TechParam.TypeB.protocolInfo = sensBResProtocolInfo;
        
        //Set RF Technology Type
        discovered_card->TechType = ptx_RUL_CardRfTechType_B;

        //Set Full Raw SENSB_RES 
        memcpy(discovered_card->TechParam.TypeB.SensBRes,sensBRes, PTX_NSC_TYPES_SENSBRES_LEN);

        /* Default Bitrate - may be overwritten later. */
        discovered_card->TechParam.TypeB.TxBitRate = ptx_RUL_RfBitRate_106;
        discovered_card->TechParam.TypeB.RxBitRate = ptx_RUL_RfBitRate_106;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_RUL_ParseSensFRes (ptx_RUL_CardParams_t *discovered_card, uint8_t *sensFRes)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != sensFRes && discovered_card != NULL))
    {
        //Copy the raw SENSF_RES
        memcpy(discovered_card->TechParam.TypeF.SensFRes, sensFRes, PTX_RUL_SENSFRES_LEN);

        //Set Type F SENSF_RES NFCID2 fields
        memcpy(discovered_card->TechParam.TypeF.Nfcid2, &sensFRes[PTX_RUL_SENSF_RES_NFCID2_OFFSET], PTX_RUL_NFCID2_LEN);

        //Set Type F SENSF_RES PAD0 fields
        memcpy(discovered_card->TechParam.TypeF.Pad0, &sensFRes[PTX_RUL_SENSF_RES_PAD0_OFFSET], PTX_RUL_SENSF_PAD0_LEN);

        //Set Type F SENSF_RES PAD1 fields
        memcpy(discovered_card->TechParam.TypeF.Pad1, &sensFRes[PTX_RUL_SENSF_RES_PAD1_OFFSET], PTX_RUL_SENSF_PAD1_LEN);

        //Set Type F SENSF_RES RD fields
        memcpy(discovered_card->TechParam.TypeF.RD, &sensFRes[PTX_RUL_SENSF_RES_RD_OFFSET], PTX_RUL_RD_LEN);

        //Set Type F SENSF_RES MRTI_CHECK field
        discovered_card->TechParam.TypeF.MrtiCheck = sensFRes[PTX_RUL_SENSF_RES_MRTI_CHECK_OFFSET]; 

        //Set Type F SENSF_RES MRTI_UPDATE field
        discovered_card->TechParam.TypeF.MrtiUpdate = sensFRes[PTX_RUL_SENSF_RES_MRTI_UPDATE_OFFSET];  

        //Set Type F SENSF_RES PAD2 field
        discovered_card->TechParam.TypeF.Pad2 = sensFRes[PTX_RUL_SENSF_RES_PAD2_OFFSET]; 
        
        //Set Type F SENSF_RES RD automatic bit rate selection feature
        ptx_RUL_FAutomaticSelectableBr_t automaticBrSelection;
        automaticBrSelection = (ptx_RUL_FAutomaticSelectableBr_t)( (sensFRes[PTX_RUL_SENSF_RES_RD_OFFSET + 1 ] & PTX_RUL_SENSF_RES_AUTO_BR_SELECT_MASK) >> 7 ); 
        discovered_card->TechParam.TypeF.automaticBrSel= automaticBrSelection; 
        
        //Set Type F SENS_RES RD advanced protocol info supported bit rate 
        ptx_RUL_FBitRateSupported_t supportedBrFeature;  
        supportedBrFeature= (ptx_RUL_FBitRateSupported_t) (sensFRes[PTX_RUL_SENSF_RES_RD_OFFSET +1] & PTX_RUL_SENSF_RES_SUPPORTED_BR_MASK);  
        discovered_card->TechParam.TypeF.supportedBrFeature = supportedBrFeature; 
        
        /* Default Bitrate - may be overwritten later. */
        discovered_card->TechParam.TypeF.BitRate = ptx_RUL_RfBitRate_212;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_RUL_ParseInventoryRes (ptx_RUL_CardParams_t *discovered_card, uint8_t *inventoryRes)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != inventoryRes && discovered_card != NULL))
    {
        //Copy the raw INVENTORY_RES
        memcpy(discovered_card->TechParam.TypeV.InventoryRes, inventoryRes, PTX_RUL_INVENTORYRES_LEN);

        //Set Type V INVENTORY_RES UID fields
        memcpy(discovered_card->TechParam.TypeV.Uid, &inventoryRes[PTX_RUL_INVENTORYRES_UID_OFFSET], PTX_RUL_INVENTORYRES_UID_LEN);

        //Set Type V INVENTORY_RES RED_FLAG
        discovered_card->TechParam.TypeV.RedFlag= inventoryRes[PTX_RUL_INVENTORYRES_REDFLAG_OFFSET];

        //Set Type V INVENTORY_RES DSFID 
        discovered_card->TechParam.TypeV.Dsfid= inventoryRes[PTX_RUL_INVENTORYRES_DSFID_OFFSET];
        
        /* Default and only Bitrate. */
        discovered_card->TechParam.TypeV.BitRate = ptx_RUL_RfBitRate_26;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_RUL_ParseP2PAcMAtrRes (ptx_RUL_CardParams_t *discovered_card, uint8_t *atrRes)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != atrRes && discovered_card != NULL))
    {
        //Copy the raw ATR_RES
        memcpy(discovered_card->TechParam.TypeP2PACM.AtrRes, atrRes, PTX_RUL_CARD_RFPROTOCOL_ATR_RES_LEN);

        //Set ACM P2P ATR_RES NFCID3
        memcpy(discovered_card->TechParam.TypeP2PACM.Nfcid3, &atrRes[PTX_RUL_ATR_RES_NFCID3_OFFSET], PTX_RUL_ATR_RES_NFCID3_LEN);

        //Set P2P ACM ATR_RES General Bytes of ATR_RES
        memcpy(discovered_card->TechParam.TypeP2PACM.GeneralBytes, &atrRes[PTX_RUL_ATR_RES_GB_OFFSET], PTX_RUL_ACM_GENERALBYTES_LEN);

        //Set P2P ACM ATR_RES DiD byte 
        discovered_card->TechParam.TypeP2PACM.Did= atrRes[PTX_RUL_ATR_RES_DID_OFFSET];

        //Set P2P ACM ATR_RES Br byte : Supported bitrate supported by target in receiving direction
        discovered_card->TechParam.TypeP2PACM.Br= atrRes[PTX_RUL_ATR_RES_BR_OFFSET];

        //Set P2P ACM ATR_RES Br byte : Supported bitrate supported by target in sending direction
        discovered_card->TechParam.TypeP2PACM.Bs= atrRes[PTX_RUL_ATR_RES_BS_OFFSET];

        //Set P2P ACM ATR_RES To byte
        discovered_card->TechParam.TypeP2PACM.To = atrRes[PTX_RUL_ATR_RES_TO_OFFSET]; 

        //Set P2P ACM ATR_RES PP byte
        discovered_card->TechParam.TypeP2PACM.PP = atrRes[PTX_RUL_ATR_RES_PP_OFFSET];

        //Set P2P ACM ATR_RES WT 
        discovered_card->TechParam.TypeP2PACM.Wt = atrRes[PTX_RUL_ATR_RES_TO_OFFSET] & PTX_RUL_ATR_RES_WT_MASK;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}
#endif
