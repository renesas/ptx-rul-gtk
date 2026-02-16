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
    Module      : NFC Soft Controller (NSC) Interface for RF Discovery Poll Mode
    File        : ptx_NSC_RfDiscover_Poll.c

    Description : Implementation of NFC Soft Controller Interface for RF Discovery Poll Mode.
                  Contains function(s) to handle RF Discovery Poll Mode parameters.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_RfDiscover.h"


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */


/*
 * Definitions for NSC Discover Command - Poll Mode.
 *
 *
 */
#define PTX_NSC_RF_DISCOVER_PARAM_POLL_LENGTH               (12u)   /* Length of the General Poll Config parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_POLL_ACM_LENGTH           (2u)    /* Length of the NFC-ACM Poll Config parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_POLL_A_LENGTH             (3u)    /* Length of the NFC-A Poll Config parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_POLL_B_LENGTH             (5u)    /* Length of the NFC-B Poll Config parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_POLL_F_LENGTH             (8u)    /* Length of the NFC-F Poll Config parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_POLL_V_LENGTH             (3u)    /* Length of the NFC-V Poll Config parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_POLL_PA_LENGTH            (22u)   /* Length of the proprietary NFC-A Poll Config parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_POLL_ISODEP_LENGTH        (18u)   /* Length of the Poll ISO_DEP parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_POLL_NFCDEP_LENGTH        (18u)   /* Length of the Poll NFC_DEP parameter. */

#define PTX_NSC_RF_DISCOVER_POLL_ALL_NTF_DISABLE            (0x00u) /* Disable all notifications during discovery process. */
#define PTX_NSC_RF_DISCOVER_POLL_LPCD_NTF_ENABLE            (0x01u) /* Enable LPCD notifications. */
#define PTX_NSC_RF_DISCOVER_POLL_ERROR_NTF_ENABLE           (0x02u) /* Enable Error notifications. */
#define PTX_NSC_RF_DISCOVER_POLL_MIN_GUARD_TIME             (0u)    /* Minimum additional guard time. */

#define PTX_NSC_RF_DISCOVER_POLL_NO_DEVICE_LIMIT            (0x00u) /* No limit for the number of resolved devices. */

#define PTX_NSC_RF_DISCOVER_POLLA_COMMAND_MASK              (0x01u) /* Poll A Configuration byte: Poll command bit. */
#define PTX_NSC_RF_DISCOVER_POLLA_BAILOUT_MASK              (0x02u) /* Poll A Configuration byte: Bail out bit. */
#define PTX_NSC_RF_DISCOVER_POLLA_ANTICOLEN_MASK            (0x04u) /* Poll A Configuration byte: Anti-collision enable bit. */
#define PTX_NSC_RF_DISCOVER_POLLA_STOPONCOL_MASK            (0x08u) /* Poll A Configuration byte: Stop-on-collision bit. */
#define PTX_NSC_RF_DISCOVER_POLLB_BAILOUT_MASK              (0x01u) /* Poll B Configuration byte: Bail out bit. */
#define PTX_NSC_RF_DISCOVER_POLLB_STOPONCOL_MASK            (0x08u) /* Poll B Configuration byte: Stop-on-collision bit. */
#define PTX_NSC_RF_DISCOVER_POLLV_STOPONCOL_MASK            (0x01u) /* Poll V Configuration byte: Stop-on-collision bit. */

#define PTX_NSC_RF_DISCOVER_FEATURE_DISABLED                (0x00u) /* Feature disabled bit setting. */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NSC_ConfigurePoll(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigurePollAcm(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigurePollA(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigurePollB(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigurePollF(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigurePollV(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigurePollIsoDep(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigurePollNfcDep(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_SetDefaultDiscParams(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NSC_GetRfDiscoveryParam_Poll(ptx_NSC_t *nsc, ptx_NSC_RfDiscover_Param_Type_t type, ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != cfg) && (NULL != params))
    {
        switch (type)
        {
            case NSC_RfDiscover_Param_Type_Poll:
                status = ptx_NSC_ConfigurePoll(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_PollAcm:
                status = ptx_NSC_ConfigurePollAcm(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_PollA:
                status = ptx_NSC_ConfigurePollA(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_PollB:
                status = ptx_NSC_ConfigurePollB(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_PollF:
                status = ptx_NSC_ConfigurePollF(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_PollV:
                status = ptx_NSC_ConfigurePollV(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_PollIsoDep:
                status = ptx_NSC_ConfigurePollIsoDep(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_PollNfcDep:
                status = ptx_NSC_ConfigurePollNfcDep(cfg, params);
                break;

            default:
                /* Error already set. Invalid parameter. */
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_ParamParser_Poll(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_POLL_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Poll_t *param_poll = &param->Poll;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_Poll;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param->Poll.PollMode;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll->GuardTime;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll->Ddpc;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll->Lpcd;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll->IdleTime[2];
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll->IdleTime[1];
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll->IdleTime[0];
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll->StdbyWuCfg[1];
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll->StdbyWuCfg[0];
        cmd_idx++;

        (*bufferPos)[cmd_idx] = (uint8_t)param_poll->InitialTech;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll->NtfEnable;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll->DevicesLimit;
        cmd_idx++;


        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        if (NSC_RfDiscover_PollMode_ActivateRf == param->Poll.PollMode)
        {
            /* Activate RF field and move to RF_POLL state. */
            *nextState = ptx_NSC_State_Poll;
        }
        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}

ptx_Status_t ptx_NSC_ParamParser_PollAcm(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    PTX_UNUSED_PARAMETER(nextState);

    if (PTX_NSC_RF_DISCOVER_PARAM_POLL_ACM_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Poll_Acm_t *param_poll_acm = &param->PollAcm;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_PollAcm;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_acm->PollFrequency;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = (uint8_t)param_poll_acm->BitRate;
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}

ptx_Status_t ptx_NSC_ParamParser_PollA(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_POLL_A_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Poll_A_t *param_poll_A = &param->PollA;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_PollA;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_A->PollFrequency;
        cmd_idx++;

        uint8_t tmp;
        (*bufferPos)[cmd_idx] = (uint8_t)(param_poll_A->PollCommand & PTX_NSC_RF_DISCOVER_POLLA_COMMAND_MASK);

        tmp = ((uint8_t)NSC_PollBailOut_Enabled == param_poll_A->BailOut) ? PTX_NSC_RF_DISCOVER_POLLA_BAILOUT_MASK : PTX_NSC_RF_DISCOVER_FEATURE_DISABLED;
        (*bufferPos)[cmd_idx] = (uint8_t)((*bufferPos)[cmd_idx] | tmp);

        tmp = ((uint8_t)NSC_PollA_AntiCol_Enabled == param_poll_A->AnticollisionEnable) ? PTX_NSC_RF_DISCOVER_POLLA_ANTICOLEN_MASK : PTX_NSC_RF_DISCOVER_FEATURE_DISABLED;
        (*bufferPos)[cmd_idx] = (uint8_t)((*bufferPos)[cmd_idx] | tmp);

        tmp = ((uint8_t)NSC_PollStopOnCol_Enabled == param_poll_A->StopOnCollision) ? PTX_NSC_RF_DISCOVER_POLLA_STOPONCOL_MASK : PTX_NSC_RF_DISCOVER_FEATURE_DISABLED;
        (*bufferPos)[cmd_idx] = (uint8_t)((*bufferPos)[cmd_idx] | tmp);
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_A->DevicesLimit;
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}

ptx_Status_t ptx_NSC_ParamParser_PollB(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_POLL_B_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Poll_B_t *param_poll_B = &param->PollB;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_PollB;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_B->PollFrequency;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_B->SensBReqAfi;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_B->SensBReqParam;
        cmd_idx++;

        uint8_t tmp;
        (*bufferPos)[cmd_idx] = ((uint8_t)NSC_PollBailOut_Enabled == param_poll_B->BailOut) ? PTX_NSC_RF_DISCOVER_POLLB_BAILOUT_MASK : PTX_NSC_RF_DISCOVER_FEATURE_DISABLED;

        tmp = ((uint8_t)NSC_PollStopOnCol_Enabled == param_poll_B->StopOnCollision) ? PTX_NSC_RF_DISCOVER_POLLB_STOPONCOL_MASK : PTX_NSC_RF_DISCOVER_FEATURE_DISABLED;
        (*bufferPos)[cmd_idx] = (uint8_t)((*bufferPos)[cmd_idx] | tmp);
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_B->DevicesLimit;
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }
    return status;
}

ptx_Status_t ptx_NSC_ParamParser_PollF(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_POLL_F_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Poll_F_t *param_poll_F = &param->PollF;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_PollF;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_F->PollFrequency;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = (uint8_t)param_poll_F->BitRate;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_F->BailOut;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_F->DevicesLimit;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_F->SystemCode[0];
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_F->SystemCode[1];
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_F->RequestCode;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_F->InitialTimeSlotNr;
        cmd_idx++;


        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }
    return status;
}

ptx_Status_t ptx_NSC_ParamParser_PollV(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_POLL_V_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Poll_V_t *param_poll_V = &param->PollV;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_PollV;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_V->PollFrequency;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = ((uint8_t)NSC_PollStopOnCol_Enabled == param_poll_V->StopOnCollision) ? PTX_NSC_RF_DISCOVER_POLLV_STOPONCOL_MASK : PTX_NSC_RF_DISCOVER_FEATURE_DISABLED;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_V->DevicesLimit;
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }
    return status;
}

ptx_Status_t ptx_NSC_ParamParser_PollIsoDep(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_POLL_ISODEP_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Poll_IsoDep_t *param_poll_isodep = &param->PollIsoDep;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_PollIsoDep;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_isodep->AutoActivation;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_isodep->Hbr;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_isodep->AttribInfLen;
        cmd_idx++;

        (void) memcpy(&((*bufferPos)[cmd_idx]), param_poll_isodep->AttribInf, PTX_NSC_TYPES_ATTRIBINF_LEN);
        cmd_idx = (uint8_t) (cmd_idx + PTX_NSC_TYPES_ATTRIBINF_LEN);

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}

ptx_Status_t ptx_NSC_ParamParser_PollNfcDep(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_POLL_NFCDEP_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Poll_NfcDep_t *param_poll_nfcdep = &param->PollNfcDep;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_PollNfcDep;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_nfcdep->AutoActivation;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_nfcdep->Hbr;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param_poll_nfcdep->AtrReqGeneralLen;
        cmd_idx++;

        (void) memcpy(&((*bufferPos)[cmd_idx]), param_poll_nfcdep->AtrReqGeneral, PTX_NSC_TYPES_ATRREQGEN_LEN);
        cmd_idx = (uint8_t) (cmd_idx + PTX_NSC_TYPES_ATRREQGEN_LEN);

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_ConfigurePoll(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_Poll;
        ptx_NSC_RfDiscover_Param_Poll_t *pollParams = &params->Value.Poll;
        memset(pollParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Poll_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);
        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->PollMode)
            {
                case NSC_RfDiscover_PollMode_Disable:
                    pollParams->PollMode = NSC_RfDiscover_PollMode_Disable;
                    break;
                case NSC_RfDiscover_PollMode_NfcForum:
                    pollParams->PollMode = NSC_RfDiscover_PollMode_NfcForum;
                    pollParams->InitialTech = (ptx_NSC_RfDiscover_InitialTech_t)cfg->InitPollTech;
                    break;
                case NSC_RfDiscover_PollMode_EMVCo:
                    pollParams->PollMode = NSC_RfDiscover_PollMode_EMVCo;
                    break;
                case NSC_RfDiscover_PollMode_Iso:
                    pollParams->PollMode = NSC_RfDiscover_PollMode_Iso;
                    break;
                case NSC_RfDiscover_PollMode_ActivateRf:
                    pollParams->PollMode = NSC_RfDiscover_PollMode_ActivateRf;
                    break;
                case NSC_RfDiscover_PollMode_Lpcd:
                    pollParams->PollMode = NSC_RfDiscover_PollMode_Lpcd;
                    break;
                default:
                    pollParams->PollMode = NSC_RfDiscover_PollMode_Disable;
                    break;
            }

            pollParams->GuardTime = cfg->GuardTime;

            switch (cfg->EnableDDPC)
            {
                case NSC_PollDDPC_Disabled:
                    pollParams->Ddpc = NSC_PollDDPC_Disabled;
                    break;
                case NSC_PollDDPC_Enabled:
                    pollParams->Ddpc = NSC_PollDDPC_Enabled;
                    break;
                default:
                    pollParams->Ddpc = NSC_PollDDPC_Disabled;
                    break;
            }

            switch (cfg->EnableLPCD)
            {
                case NSC_PollLPCD_Disabled:
                    pollParams->Lpcd = NSC_PollLPCD_Disabled;
                    break;
                case NSC_PollLPCD_Enabled:
                    pollParams->Lpcd = NSC_PollLPCD_Enabled;
                    break;
                default:
                    pollParams->Lpcd = cfg->EnableLPCD;
                    break;
            }

            /* Expect exact enabler value - 1. Other values than 0 or 1 are not allowed and are treated as disablers,
             * i.e. they are treated as default value: 0.
             */
            pollParams->NtfEnable = (uint8_t)(((1 == cfg->EnableLPCDNtf) ? PTX_NSC_RF_DISCOVER_POLL_LPCD_NTF_ENABLE : PTX_NSC_RF_DISCOVER_POLL_ALL_NTF_DISABLE) |
                                              ((1 == cfg->EnableErrorNtf) ? PTX_NSC_RF_DISCOVER_POLL_ERROR_NTF_ENABLE : PTX_NSC_RF_DISCOVER_POLL_ALL_NTF_DISABLE));

            pollParams->IdleTime[0] = (uint8_t)((cfg->IdleTime >>  0) & (uint8_t)0xFF);
            pollParams->IdleTime[1] = (uint8_t)((cfg->IdleTime >>  8) & (uint8_t)0xFF);
            pollParams->IdleTime[2] = (uint8_t)((cfg->IdleTime >> 16) & (uint8_t)0xFF);

            pollParams->StdbyWuCfg[0] = (uint8_t)((cfg->StdbyWuCfg >> 0) & (uint8_t)0xFF);
            pollParams->StdbyWuCfg[1] = (uint8_t)((cfg->StdbyWuCfg >> 8) & (uint8_t)0xFF);

            pollParams->DevicesLimit = cfg->PollDevicesLimit;
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return status;

}

static ptx_Status_t ptx_NSC_ConfigurePollAcm(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_PollAcm;
        ptx_NSC_RfDiscover_Param_Poll_Acm_t *pollAcmParams = &params->Value.PollAcm;
        memset(pollAcmParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Poll_Acm_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);
        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->PollTypeAcmPollFrequency)
            {
                case NSC_PollFreq_Disabled:
                    pollAcmParams->PollFrequency = NSC_PollFreq_Disabled;
                    break;
                case NSC_PollFreq_EveryCycle:
                    pollAcmParams->PollFrequency = NSC_PollFreq_EveryCycle;
                    break;
                default:
                    if ((NSC_PollFreq_CycleMin <= cfg->PollTypeAPollFrequency)
                        && (NSC_PollFreq_CycleMax >= cfg->PollTypeAPollFrequency))
                    {
                        pollAcmParams->PollFrequency = cfg->PollTypeAcmPollFrequency;
                    }
                    else
                    {
                        pollAcmParams->PollFrequency = NSC_PollFreq_Disabled;
                    }
                    break;
            }

            if (NSC_PollFreq_Disabled != pollAcmParams->PollFrequency)
            {
                pollAcmParams->BitRate = (ptx_NSC_PollAcm_BitRate_t)cfg->PollTypeAcmBitrate;
            }
        }
    }

    return status;

}

static ptx_Status_t ptx_NSC_ConfigurePollA(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_PollA;
        ptx_NSC_RfDiscover_Param_Poll_A_t *pollAParams = &params->Value.PollA;
        memset(pollAParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Poll_A_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);
        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->PollTypeAPollFrequency)
            {
                case NSC_PollFreq_Disabled:
                    pollAParams->PollFrequency = NSC_PollFreq_Disabled;
                    break;
                case NSC_PollFreq_EveryCycle:
                    pollAParams->PollFrequency = NSC_PollFreq_EveryCycle;
                    break;
                default:
                    if ((NSC_PollFreq_CycleMin <= cfg->PollTypeAPollFrequency)
                        && (NSC_PollFreq_CycleMax >= cfg->PollTypeAPollFrequency))
                    {
                        pollAParams->PollFrequency = cfg->PollTypeAPollFrequency;
                    }
                    else
                    {
                        pollAParams->PollFrequency = NSC_PollFreq_Disabled;
                    }
                    break;
            }

            if (0 != cfg->PollTypeADeviceLimit)
            {
                pollAParams->DevicesLimit = cfg->PollTypeADeviceLimit;
            }

            if (0 != cfg->PollTypeABailOut)
            {
                pollAParams->BailOut = cfg->PollTypeABailOut;
            }
        }
    }

    return status;

}

static ptx_Status_t ptx_NSC_ConfigurePollB(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_PollB;
        ptx_NSC_RfDiscover_Param_Poll_B_t *pollBParams = &params->Value.PollB;
        memset(pollBParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Poll_B_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);
        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->PollTypeBPollFrequency)
            {
                case NSC_PollFreq_Disabled:
                    pollBParams->PollFrequency = NSC_PollFreq_Disabled;
                    break;
                case NSC_PollFreq_EveryCycle:
                    pollBParams->PollFrequency = NSC_PollFreq_EveryCycle;
                    break;
                default:
                    if ((NSC_PollFreq_CycleMin <= cfg->PollTypeBPollFrequency)
                        && (NSC_PollFreq_CycleMax >= cfg->PollTypeBPollFrequency))
                    {
                        pollBParams->PollFrequency = cfg->PollTypeBPollFrequency;
                    }
                    else
                    {
                        pollBParams->PollFrequency = NSC_PollFreq_Disabled;
                    }
                    break;
            }

            if (0 != cfg->PollTypeBDeviceLimit)
            {
                pollBParams->DevicesLimit = cfg->PollTypeBDeviceLimit;
            }

            if (0 != cfg->PollTypeBBailOut)
            {
                pollBParams->BailOut = cfg->PollTypeBBailOut;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_ConfigurePollF(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_PollF;
        ptx_NSC_RfDiscover_Param_Poll_F_t *pollFParams = &params->Value.PollF;
        memset(pollFParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Poll_F_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);
        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->PollTypeFPollFrequency)
            {
                case NSC_PollFreq_Disabled:
                    pollFParams->PollFrequency = NSC_PollFreq_Disabled;
                    break;
                case NSC_PollFreq_EveryCycle:
                    pollFParams->PollFrequency = NSC_PollFreq_EveryCycle;
                    break;
                default:
                    if ((NSC_PollFreq_CycleMin <= cfg->PollTypeFPollFrequency)
                        && (NSC_PollFreq_CycleMax >= cfg->PollTypeFPollFrequency))
                    {
                        pollFParams->PollFrequency = cfg->PollTypeFPollFrequency;
                    }
                    else
                    {
                        pollFParams->PollFrequency = NSC_PollFreq_Disabled;
                    }
                    break;
            }

            if (0 != cfg->PollTypeFDeviceLimit)
            {
                pollFParams->DevicesLimit = cfg->PollTypeFDeviceLimit;
            }

            if (1 == cfg->PollTypeF212)
            {
                pollFParams->BitRate = NSC_BitRate_212;
            }

            if (1 == cfg->PollTypeF424)
            {
                /* Please note that this setting will overwrite the 212 value if it has been previously set.*/
                pollFParams->BitRate = NSC_BitRate_424;
            }

            if ((1 == cfg->PollTypeF212) || (1 == cfg->PollTypeF424))
            {
                if (0 != cfg->PollTypeFCmdParams.OverwriteDefaultParams)
                {
                    pollFParams->SystemCode[0]     = cfg->PollTypeFCmdParams.SystemCode[0];
                    pollFParams->SystemCode[1]     = cfg->PollTypeFCmdParams.SystemCode[1];
                    pollFParams->RequestCode       = cfg->PollTypeFCmdParams.RequestCode;
                    pollFParams->InitialTimeSlotNr = cfg->PollTypeFCmdParams.InitialTimeSlotNr;
                }
            }


            if (0 != cfg->PollTypeFBailOut)
            {
                pollFParams->BailOut = cfg->PollTypeFBailOut;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_ConfigurePollV(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_PollV;
        ptx_NSC_RfDiscover_Param_Poll_V_t *pollVParams = &params->Value.PollV;
        memset(pollVParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Poll_V_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->PollTypeVPollFrequency)
            {
                case NSC_PollFreq_Disabled:
                    pollVParams->PollFrequency = NSC_PollFreq_Disabled;
                    break;
                case NSC_PollFreq_EveryCycle:
                    pollVParams->PollFrequency = NSC_PollFreq_EveryCycle;
                    break;
                default:
                    if ((NSC_PollFreq_CycleMin <= cfg->PollTypeVPollFrequency)
                        && (NSC_PollFreq_CycleMax >= cfg->PollTypeVPollFrequency))
                    {
                        pollVParams->PollFrequency = cfg->PollTypeVPollFrequency;
                    }
                    else
                    {
                        pollVParams->PollFrequency = NSC_PollFreq_Disabled;
                    }
                    break;
            }

            if (0 != cfg->PollTypeVDeviceLimit)
            {
                pollVParams->DevicesLimit = cfg->PollTypeVDeviceLimit;
            }
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return status;
}

static ptx_Status_t ptx_NSC_ConfigurePollIsoDep(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_PollIsoDep;
        ptx_NSC_RfDiscover_Param_Poll_IsoDep_t *pollIsoDepParams = &params->Value.PollIsoDep;
        memset(pollIsoDepParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Poll_IsoDep_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);
        if (PTX_STATUS_SUCCESS == status)
        {
            if (NSC_PollIsoDep_Disabled == cfg->PollIsoDepEnable)
            {
                /* ISO-DEP is by default enabled. Disable it only if the user requested it. */
                pollIsoDepParams->AutoActivation = NSC_PollIsoDep_Disabled;
            }

            pollIsoDepParams->Hbr = cfg->PollIsoDepHbr;
        }
    }

    return status;

}

static ptx_Status_t ptx_NSC_ConfigurePollNfcDep(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_PollNfcDep;
        ptx_NSC_RfDiscover_Param_Poll_NfcDep_t *pollNfcDepParams = &params->Value.PollNfcDep;
        memset(pollNfcDepParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Poll_NfcDep_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);

        if (PTX_STATUS_SUCCESS == status)
        {
            if (NSC_PollNfcDep_Disabled == cfg->PollNfcDepEnable)
            {
                /* NFC-DEP is by default enabled. Disable it only if the user requested it. */
                pollNfcDepParams->AutoActivation = NSC_PollNfcDep_Disabled;
            }
            else if (0 != cfg->PollNfcDepAtrReqLen)
            {
                pollNfcDepParams->AtrReqGeneralLen = cfg->PollNfcDepAtrReqLen;
                memcpy(pollNfcDepParams->AtrReqGeneral, cfg->PollNfcDepAtrReq, pollNfcDepParams->AtrReqGeneralLen);
            }

            pollNfcDepParams->Hbr = cfg->PollNfcDepHbr;

        }

    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return status;
}

static ptx_Status_t ptx_NSC_SetDefaultDiscParams(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        switch (params->Type)
        {
            case NSC_RfDiscover_Param_Type_Poll:
            {
                ptx_NSC_RfDiscover_Param_Poll_t *pollParams = &params->Value.Poll;

                pollParams->PollMode                = NSC_RfDiscover_PollMode_EMVCo;            /* EMVCo Poll Mode */
                pollParams->GuardTime               = PTX_NSC_RF_DISCOVER_POLL_MIN_GUARD_TIME;  /* Additional Guard Time (= 0ms by default) */
                pollParams->Ddpc                    = NSC_PollDDPC_Enabled;                     /* Digital Dynamic Power Control enabled */
                pollParams->Lpcd                    = NSC_PollLPCD_Disabled;                    /* Low Power Card Detection disabled */

                /* RF-Idle Time (multiple of 32us => 0x0C1C ~ 100ms */
                pollParams->IdleTime[0]             = 0x1Cu;
                pollParams->IdleTime[1]             = 0x0Cu;
                pollParams->IdleTime[2]             = 0u;

                pollParams->DevicesLimit            = PTX_NSC_RF_DISCOVER_POLL_NO_DEVICE_LIMIT;
                pollParams->NtfEnable               = (uint8_t)(PTX_NSC_RF_DISCOVER_POLL_LPCD_NTF_ENABLE +
                                                                PTX_NSC_RF_DISCOVER_POLL_ERROR_NTF_ENABLE);

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_PollAcm:
            {
                ptx_NSC_RfDiscover_Param_Poll_Acm_t *pollAcmParams = &params->Value.PollAcm;

                pollAcmParams->PollFrequency        = NSC_PollFreq_EveryCycle;      /* Poll for NFC-ACM during every discovery cycle */
                pollAcmParams->BitRate              = NSC_PollAcm_BitRate_106;      /* 106kbps NFC-A */

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_PollA:
            {
                ptx_NSC_RfDiscover_Param_Poll_A_t *pollAParams = &params->Value.PollA;

                pollAParams->PollFrequency          = NSC_PollFreq_EveryCycle;      /* Poll for Type-A during every Poll-cycle */
                pollAParams->PollCommand            = NSC_PollCommand_WUPA;         /* Initial Type-A Command */
                pollAParams->BailOut                = NSC_PollBailOut_Disabled;     /* Type-A Bail out */

                pollAParams->AnticollisionEnable    = NSC_PollA_AntiCol_Enabled;    /* Anti-collision enabled */
                pollAParams->StopOnCollision        = NSC_PollStopOnCol_Disabled;   /* Continue resolution in case of collision */

                if (NSC_RfDiscover_PollMode_EMVCo == cfg->PollMode)
                {
                    pollAParams->DevicesLimit       = 1u;                           /* Resolve max. 1 Card in EMVCo */
                }
                else
                {
                    pollAParams->DevicesLimit       = 5u;                           /* Resolve max. 5 Cards in other Poll Modes */
                }

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_PollB:
            {
                ptx_NSC_RfDiscover_Param_Poll_B_t *pollBParams = &params->Value.PollB;

                pollBParams->PollFrequency          = NSC_PollFreq_EveryCycle;          /* Poll for Type-B during every Poll-cycle */
                pollBParams->BailOut                = NSC_PollBailOut_Disabled;         /* Type-B Bail out */
                pollBParams->SensBReqAfi            = 0x00u;                            /* Type-B Init Command AFI Parameter */
                pollBParams->StopOnCollision        = NSC_PollStopOnCol_Disabled;       /* Continue resolution in case of collision */

                if (NSC_RfDiscover_PollMode_Iso == cfg->PollMode)
                {
                    pollBParams->SensBReqParam      = 0x18u;                            /* Type-B Init Command PARAM Parameter with extended SENB_RES support */
                }
                else
                {
                    pollBParams->SensBReqParam      = 0x08u;                            /* Type-B Init Command PARAM Parameter */
                }



                if (NSC_RfDiscover_PollMode_EMVCo == cfg->PollMode)
                {
                    pollBParams->DevicesLimit       = 1u;                               /* Resolve max. 1 Card in EMVCo */
                }
                else
                {
                    pollBParams->DevicesLimit       = 5u;                               /* Resolve max. 5 Cards in other Poll Modes */
                }

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_PollF:
            {
                ptx_NSC_RfDiscover_Param_Poll_F_t *pollFParams = &params->Value.PollF;

                pollFParams->PollFrequency          = NSC_PollFreq_EveryCycle;      /* Poll for Type-F during every Poll-cycle */
                pollFParams->BailOut                = NSC_PollBailOut_Disabled;     /* Type-F Bail out */
                pollFParams->BitRate                = NSC_BitRate_212;              /* Type-F Bit Rate */

                if (NSC_RfDiscover_PollMode_EMVCo == cfg->PollMode)
                {
                    pollFParams->DevicesLimit       = 1u;                           /* Resolve max. 1 Card in EMVCo */
                }
                else
                {
                    pollFParams->DevicesLimit       = 5u;                               /* Resolve max. 5 Cards in other Poll Modes */
                }

                pollFParams->SystemCode[0]          = 0xFFu;
                pollFParams->SystemCode[1]          = 0xFFu;
                pollFParams->RequestCode            = 0x00u;
                pollFParams->InitialTimeSlotNr      = 0x03u;

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_PollV:
            {
                ptx_NSC_RfDiscover_Param_Poll_V_t *pollVParams = &params->Value.PollV;

                pollVParams->PollFrequency          = NSC_PollFreq_EveryCycle;      /* Poll for Type-F during every Poll-cycle */
                pollVParams->StopOnCollision        = NSC_PollStopOnCol_Disabled;   /* Continue resolution in case of collision */

                if (NSC_RfDiscover_PollMode_EMVCo == cfg->PollMode)
                {
                    pollVParams->DevicesLimit       = 1u;                               /* Resolve max. 1 Card in EMVCo */
                }
                else
                {
                    pollVParams->DevicesLimit       = 5u;                               /* Resolve max. 5 Cards in other Poll Modes */
                }

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_PollIsoDep:
            {
                ptx_NSC_RfDiscover_Param_Poll_IsoDep_t *pollIsoDepParams = &params->Value.PollIsoDep;

                /* Enable auto-activation for ISO-DEP protocol. Set maximum bit-rate. */
                pollIsoDepParams->AutoActivation    = (uint8_t)(NSC_PollIsoDep_NfcA_Enabled + NSC_PollIsoDep_NfcB_Enabled);
                pollIsoDepParams->Hbr               = NSC_PollIsoDepHBR_848;   /* Set maximum bit-rate. */
                pollIsoDepParams->AttribInfLen      = 0u;                               /* Empty ATTRIB INF Field */
                memset(pollIsoDepParams->AttribInf, 0x00, sizeof(uint8_t) * PTX_NSC_TYPES_ATTRIBINF_LEN);

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_PollNfcDep:
            {
                ptx_NSC_RfDiscover_Param_Poll_NfcDep_t *pollNfcDepParams = &params->Value.PollNfcDep;

                /* Enable auto-activation for NFC-DEP protocol. Set maximum bit-rate. */
                pollNfcDepParams->AutoActivation    = (uint8_t)(NSC_PollNfcDep_NfcA_Enabled + NSC_PollNfcDep_NfcF_Enabled);
                pollNfcDepParams->Hbr               = NSC_PollNfcDepHBR_424;   /* Set maximum bit-rate. */
                pollNfcDepParams->AtrReqGeneralLen  = 0u;                      /* Empty ATR REQ general bytes */
                memset(pollNfcDepParams->AtrReqGeneral, 0x00, sizeof(uint8_t) * PTX_NSC_TYPES_ATRREQGEN_LEN);

                status = PTX_STATUS_SUCCESS;

                break;
            }

            default:
            {
                /* Invalid parameter. */
                break;
            }
        }
    }

    return status;
}
