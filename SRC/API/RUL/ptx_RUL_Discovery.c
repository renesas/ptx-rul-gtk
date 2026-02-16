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
    Module      : RUL
    File        : ptx_RUL_Discovery.c
    
    Description : Reader Universal Library (RUL) discovery function implementation.
                  Contains functions to initiate discovery and copy discovery parameters.
 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_RUL.h"

#include "ptx_NSC_RfDiscover.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * 1 Timer-tick in the NFC-HW is 32us
 */
#define RUL_IDLE_TIME_TICK_MS               (uint32_t)(1000)
#define RUL_IDLE_TIME_TICK_DIV              (uint32_t)(32)
#define RUL_IDLE_TIME_MAX_MASK              (uint32_t)(0x000FFFFF)


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 * Copies RUL discovery parameters to NSC discovery parameters
 */
static void ptx_RUL_CopyDiscoverParameter(ptx_RUL_DiscoverConfig_t *src, ptx_NSC_RfDiscovery_Cfg_t *dest);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_RUL_InitiateDiscovery(ptx_RUL_t *rul, ptx_RUL_DiscoverConfig_t *cfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        if (NULL != cfg)
        {
            ptx_NSC_RfDiscovery_Cfg_t nsc_cfg;
            memset(&nsc_cfg, 0, sizeof(ptx_NSC_RfDiscovery_Cfg_t));

            memcpy(&rul->DiscoverConfig, cfg, sizeof(ptx_RUL_DiscoverConfig_t));

            ptx_RUL_CopyDiscoverParameter(cfg, &nsc_cfg);

            ptx_NSC_RfDiscover_Param_t discoverParams[PTX_NSC_RF_DISCOVER_MAX_PARAMCOUNT];
            uint32_t discoverParamsSize = 0;
            memset(&discoverParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_t) * PTX_NSC_RF_DISCOVER_MAX_PARAMCOUNT);

#if !defined(PTX_DISABLE_LISTEN_MODE)
            if ((0 == cfg->EnablePolling) && (0 == cfg->ListenMode))
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
#else
            if (0 == cfg->EnablePolling)
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
#endif /* PTX_DISABLE_LISTEN_MODE */
            else
            {
                status = PTX_STATUS_SUCCESS;
            }

            if ((0 != cfg->EnableLPCD) && (0 == cfg->IdleTime))
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }

            if ((0 != cfg->EnablePolling) && (PTX_STATUS_SUCCESS == status))
            {
                status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_Poll, &nsc_cfg, &discoverParams[discoverParamsSize]);
                discoverParamsSize++;

                if ((0 != cfg->PollTypeA) && (PTX_STATUS_SUCCESS == status))
                {
                    status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_PollA, &nsc_cfg, &discoverParams[discoverParamsSize]);
                    discoverParamsSize++;
                }

                if ((0 != cfg->PollTypeB) && (PTX_STATUS_SUCCESS == status))
                {
                    status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_PollB, &nsc_cfg, &discoverParams[discoverParamsSize]);
                    discoverParamsSize++;
                }

                if (((0 != cfg->PollTypeF212) || (0 != cfg->PollTypeF424)) && (PTX_STATUS_SUCCESS == status))
                {
                    status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_PollF, &nsc_cfg, &discoverParams[discoverParamsSize]);
                    discoverParamsSize++;
                }

                if ((0 != cfg->PollTypeV) && (PTX_STATUS_SUCCESS == status))
                {
                    status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_PollV, &nsc_cfg, &discoverParams[discoverParamsSize]);
                    discoverParamsSize++;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Always load ISO-DEP parameters. */
                    status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_PollIsoDep, &nsc_cfg, &discoverParams[discoverParamsSize]);
                    discoverParamsSize++;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Always load NFC-DEP parameters. */
                    status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_PollNfcDep, &nsc_cfg, &discoverParams[discoverParamsSize]);
                    discoverParamsSize++;
                }
            }

#if !defined(PTX_DISABLE_LISTEN_MODE)
            if ((0 != cfg->ListenMode) && (PTX_STATUS_SUCCESS == status))
            {
                status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_Listen, &nsc_cfg, &discoverParams[discoverParamsSize]);
                discoverParamsSize++;

                if ((0 != cfg->ListenTypeA) && (PTX_STATUS_SUCCESS == status))
                {
                    status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_ListenA, &nsc_cfg, &discoverParams[discoverParamsSize]);
                    discoverParamsSize++;
                }

                if ((0 != cfg->ListenTypeB) && (PTX_STATUS_SUCCESS == status))
                {
                    status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_ListenB, &nsc_cfg, &discoverParams[discoverParamsSize]);
                    discoverParamsSize++;
                }

                if (((0 != cfg->ListenTypeF212) || (0 != cfg->ListenTypeF424)) && (PTX_STATUS_SUCCESS == status))
                {
                    status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_ListenF, &nsc_cfg, &discoverParams[discoverParamsSize]);
                    discoverParamsSize++;
                }

                if ((0 != cfg->ListenIsoDep) && (PTX_STATUS_SUCCESS == status))
                {
                    status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_ListenIsoDep, &nsc_cfg, &discoverParams[discoverParamsSize]);
                    discoverParamsSize++;
                }

                if ((0 != cfg->ListenNfcDep) && (PTX_STATUS_SUCCESS == status))
                {
                    status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_ListenNfcDep, &nsc_cfg, &discoverParams[discoverParamsSize]);
                    discoverParamsSize++;
                }
            }
#endif /* PTX_DISABLE_LISTEN_MODE || PTX_DISABLE_LISTEN_MODE */

            if ((0 != cfg->PollACM) && (PTX_STATUS_SUCCESS == status))
            {
                status = ptx_NSC_GetRfDiscoveryParam(&(rul->Nsc), NSC_RfDiscover_Param_Type_PollAcm, &nsc_cfg, &discoverParams[discoverParamsSize]);
                discoverParamsSize++;
            }


            if (PTX_STATUS_SUCCESS == status)
            {
                (void) ptx_RUL_ClearCardRegistry(rul);

                status = ptx_NSC_RfDiscover(&(rul->Nsc), discoverParams, discoverParamsSize);
            }
        }
        else
        {
            (void) ptx_RUL_ClearCardRegistry(rul);

            status = ptx_NSC_RfDiscover(&(rul->Nsc), NULL, 0);
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static void ptx_RUL_CopyDiscoverParameter(ptx_RUL_DiscoverConfig_t *src, ptx_NSC_RfDiscovery_Cfg_t *dest)
{
    if ((NULL != src) && (NULL != dest))
    {
        /* Map POLL mode settings */

        dest->PollMode = src->PollMode;

        dest->GuardTime = src->GuardTime;
        dest->EnableDDPC = src->EnableDDPC;
        dest->EnableLPCD = src->EnableLPCD;
        dest->EnableLPCDNtf = src->EnableLPCDNtf;

        /* IdleTime is provided in ms. It needs to be transformed into system ticks of 32us. */
        uint32_t idle_time = (uint32_t)(((src->IdleTime * RUL_IDLE_TIME_TICK_MS) / RUL_IDLE_TIME_TICK_DIV) & RUL_IDLE_TIME_MAX_MASK);
        dest->IdleTime = idle_time;
        dest->InitPollTech = src->InitialPollingTech;

        dest->PollTypeADeviceLimit = src->PollTypeADeviceLimit;
        dest->PollTypeAPollFrequency = src->PollTypeAPollFrequency;
        dest->PollTypeABailOut = src->PollTypeABailOut;

        dest->PollTypeBDeviceLimit = src->PollTypeBDeviceLimit;
        dest->PollTypeBPollFrequency = src->PollTypeBPollFrequency;
        dest->PollTypeBBailOut = src->PollTypeBBailOut;

        dest->PollTypeF212 = src->PollTypeF212;
        dest->PollTypeF424 = src->PollTypeF424;
        dest->PollTypeFDeviceLimit = src->PollTypeFDeviceLimit;
        dest->PollTypeFPollFrequency = src->PollTypeFPollFrequency;
        dest->PollTypeFBailOut = src->PollTypeFBailOut;
        dest->PollTypeFCmdParams.OverwriteDefaultParams = src->PollTypeFCmdParams.OverwriteDefaultParams;
        dest->PollTypeFCmdParams.SystemCode[0] = src->PollTypeFCmdParams.SystemCode[0];
        dest->PollTypeFCmdParams.SystemCode[1] = src->PollTypeFCmdParams.SystemCode[1];
        dest->PollTypeFCmdParams.RequestCode = src->PollTypeFCmdParams.RequestCode;
        dest->PollTypeFCmdParams.InitialTimeSlotNr = src->PollTypeFCmdParams.InitialTimeSlotNr;


        dest->PollTypeVDeviceLimit = src->PollTypeVDeviceLimit;
        dest->PollTypeVPollFrequency = src->PollTypeVPollFrequency;

        dest->StdbyWuCfg = 0;

        if (0 != src->StandbyCfg.EnableWuByTimer)
        {
            dest->StdbyWuCfg |= PTX_NSC_POLL_STDBYWU_MASK_WU_TIMER_ENABLE;
        }

        if (0 != src->StandbyCfg.EnableWuByLpfd)
        {
            dest->StdbyWuCfg |= PTX_NSC_POLL_STDBYWU_MASK_WU_LPFD_ENABLE;
        }

        if (0 != src->StandbyCfg.EnableWuByHif)
        {
            dest->StdbyWuCfg |= PTX_NSC_POLL_STDBYWU_MASK_WU_HIF_ENABLE;
        }

        if (0 != src->StandbyCfg.EnableWuByGpio)
        {
            dest->StdbyWuCfg |= PTX_NSC_POLL_STDBYWU_MASK_WU_GPIO_ENABLE;
        }

        if (0 != dest->StdbyWuCfg)
        {
            /* If any of the Standby WakeUp sources has been selected, this means Standby is enabled.
             * In such a case make sure that WakeUpByHIF is always enabled.
             */
            dest->StdbyWuCfg |= PTX_NSC_POLL_STDBYWU_MASK_WU_HIF_ENABLE;
        }

        /* IsoDep is enabled by default. Check if we need to disable it here. */
        dest->PollIsoDepEnable = NSC_PollIsoDep_NfcA_Enabled + NSC_PollIsoDep_NfcB_Enabled;
        if (0 != src->DisablePollIsoDep)
        {
            dest->PollIsoDepEnable = NSC_PollIsoDep_Disabled;
        }

        /* Map Poll mode ISO-DEP HBR settings to NSC */
        switch (src->PollIsoDepHbr)
        {
            case ptx_RUL_PollIsoDepHBR_Maximum:
            /* FALL THROUGH */
            case ptx_RUL_PollIsoDepHBR_848:
                dest->PollIsoDepHbr = NSC_PollIsoDepHBR_848;
                break;
            case ptx_RUL_PollIsoDepHBR_212:
                dest->PollIsoDepHbr = NSC_PollIsoDepHBR_212;
                break;
            case ptx_RUL_PollIsoDepHBR_424:
                dest->PollIsoDepHbr = NSC_PollIsoDepHBR_424;
                break;
            default:
                dest->PollIsoDepHbr = NSC_PollIsoDepHBR_Disabled;
                break;
        }

        /* NfcDep is enabled by default. Check if we need to disable it here. */
        dest->PollNfcDepEnable = NSC_PollNfcDep_NfcA_Enabled + NSC_PollNfcDep_NfcF_Enabled;
        if (0 != src->DisableNfcDepProtocol)
        {
            dest->PollNfcDepEnable = NSC_PollNfcDep_Disabled;
        }

        /* Map Poll mode NFC-DEP HBR settings to NSC */
        switch (src->PollNfcDepHbr)
        {
            case ptx_RUL_PollNfcDepHBR_Maximum:
            /* FALL THROUGH */
            case ptx_RUL_PollNfcDepHBR_424:
                dest->PollNfcDepHbr = NSC_PollNfcDepHBR_424;
                break;
            case ptx_RUL_PollNfcDepHBR_212:
                dest->PollNfcDepHbr = NSC_PollNfcDepHBR_212;
                break;
            default:
                dest->PollNfcDepHbr = NSC_PollNfcDepHBR_Disabled;
                break;
        }

        /* Map ATRREQ settings */
        dest->PollNfcDepAtrReqLen = src->PollNfcDepAtrReqLen;
        memcpy(dest->PollNfcDepAtrReq, src->PollNfcDepAtrReq, dest->PollNfcDepAtrReqLen);


        /* Map ACM settings */
        dest->PollTypeAcmPollFrequency = src->PollACM;
        dest->PollTypeAcmBitrate = src->PollAcmBitRate;

#if !defined(PTX_DISABLE_LISTEN_MODE)
        /* Map LISTEN mode settings */
        dest->ListenMode = src->ListenMode;

        dest->ListenTypeA = src->ListenTypeA;

        dest->ListenTypeANfcIdLen = src->ListenTypeANfcIdLen;
        memcpy(dest->ListenTypeANfcId, src->ListenTypeANfcId, PTX_RUL_NFCID1_MAX_LEN);

        dest->ListenTypeASensResLen = src->ListenTypeASensResLen;
        memcpy(dest->ListenTypeASensRes, src->ListenTypeASensRes, PTX_RUL_SENSRES_LEN);


        dest->ListenTypeB = src->ListenTypeB;
        memcpy(dest->ListenTypeBNfcId, src->ListenTypeBNfcId, PTX_RUL_NFCID0_LEN);
        memcpy(dest->ListenTypeBAppData, src->ListenTypeBAppData, PTX_RUL_APPDATA_LEN);
        memcpy(dest->ListenTypeBProtInfo, src->ListenTypeBProtInfo, PTX_RUL_PROTINFO_LEN);

        dest->ListenTypeBNfcIdLen = src->ListenTypeBNfcIdLen;
        dest->ListenTypeBAppDataLen = src->ListenTypeBAppDataLen;
        dest->ListenTypeBProtInfoLen = src->ListenTypeBProtInfoLen;


        dest->ListenTypeF212 = src->ListenTypeF212;
        dest->ListenTypeF424 = src->ListenTypeF424;

        memcpy(dest->ListenTypeFNfcId, src->ListenTypeFNfcId, PTX_RUL_NFCID2_LEN);
        dest->ListenTypeFNfcIdLen = src->ListenTypeFNfcIdLen;
        memcpy(dest->ListenTypeFRequestData, src->ListenTypeFRequestData, PTX_RUL_RD_LEN);
        dest->ListenTypeFRequestDataLen = src->ListenTypeFRequestDataLen;


        dest->ListenIsoDep = src->ListenIsoDep;
        memcpy(dest->ListenIsoDepATS, src->ListenIsoDepAts, PTX_RUL_ISODEP_ATS_LEN);
        dest->ListenIsoDepATSLen = src->ListenIsoDepAtsLen;
        memcpy(dest->ListenIsoDepAttribRes, src->ListenIsoDepAttribRes, PTX_RUL_ISODEP_ATTRIBRES_LEN);
        dest->ListenIsoDepAttribResLen = src->ListenIsoDepAttribResLen;

        dest->ListenNfcDep = src->ListenNfcDep;
        memcpy(dest->ListenNfcDepAtrRes, src->ListenNfcDepAtrRes, PTX_RUL_NFCDEP_ATRRES_LEN);
        dest->ListenNfcDepAtrResLen = src->ListenNfcDepAtrResLen;
#endif /* PTX_DISABLE_LISTEN_MODE */
    }
}
