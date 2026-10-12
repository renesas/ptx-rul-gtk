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
    Module      : NFC Soft Controller (NSC) Interface for System Configuration
    File        : ptx_NSC_SystemConfig.c

    Description : Implementation of NFC Soft Controller Interface for System Configuration.
                  Contains the function(s) to set specific parameters for hardware initialization.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_SystemConfig.h"
#include "ptx_NSC_Base.h"
#include "ptx_NSC_CmdProcessor.h"
#include "ptx_NSC_Firmware.h"
#include "ptx_NSC_InitHw.h"


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_NSC_INIT_CLOCK_SELECT_DEFAULT           (0x00)          /* NSC INIT default clock selection (= crystal). */
#define PTX_NSC_INIT_EXT_CLOCK_GUARD_TIME           (0x00)          /* NSC INIT default external clock guard time. */
#define PTX_NSC_INIT_EXT_CLOCK_REQUEST_ENAB         (0x00)          /* NSC INIT default external clock request selection (= disabled). */
#define PTX_NSC_INIT_XTAL_TRIM_DEFAULT              (12)            /* NSC INIT default XTAL cap selection */
#define PTX_NSC_INIT_REFERENCE_FREQUENCY_DEFAULT    (27.12E6)       /* NSC INIT default crystal frequency (27.12MHz). */
#define PTX_NSC_INIT_TARGET_FREQUENCY_DEFAULT       (13.56E6)       /* NSC INIT default target frequency (13.56MHz). */
#define PTX_NSC_INIT_PRNG_SEED_DEFAULT              (0x44444444)    /* NSC INIT PRNG seed default. */
#define PTX_NSC_INIT_NALMMAX_DEFAULT                (0xFFFFFFFF)    /* NSC INIT NALM_MAX default. */
#define PTX_NSC_INIT_NALMMIN_DEFAULT                (0x00000000)    /* NSC INIT NALM_MIN default. */
#define PTX_NSC_INIT_NALMOFFSET_DEFAULT             (0x000B237E)    /* NSC INIT NHOST_ALM offset (div_offset) default (fits vco_offset 4720000Hz). */
#define PTX_NSC_INIT_TEMPERRTRESH_DEFAULT           85              /* NSC INIT Over-temperature protection default */
#define PTX_NSC_INIT_TXCURLIMGAIN_DEFAULT           0               /* NSC INIT Over-current protection disabled */


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NSC_System_SendSystemConfig(ptx_NSC_t* nsc);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_System_LoadDefaultParam(ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_System_LoadDefaultParam");
        ptx_NSC_SystemConfig_t *config = &(nsc->SysCfg);

        config->ClkSel          = PTX_NSC_INIT_CLOCK_SELECT_DEFAULT;
        config->ClkReqEn        = PTX_NSC_INIT_EXT_CLOCK_REQUEST_ENAB;
        config->ClkReqGt        = PTX_NSC_INIT_EXT_CLOCK_GUARD_TIME;
        config->XtalTrim        = PTX_NSC_INIT_XTAL_TRIM_DEFAULT;
        config->RefFreq         = PTX_NSC_INIT_REFERENCE_FREQUENCY_DEFAULT;
        config->TargetFreq      = PTX_NSC_INIT_TARGET_FREQUENCY_DEFAULT;
        config->PrngSeed        = PTX_NSC_INIT_PRNG_SEED_DEFAULT;
        config->NalmMax         = PTX_NSC_INIT_NALMMAX_DEFAULT;
        config->NalmMin         = PTX_NSC_INIT_NALMMIN_DEFAULT;
        config->NAlmOffset      = PTX_NSC_INIT_NALMOFFSET_DEFAULT;
        config->TempErrTreshold = PTX_NSC_INIT_TEMPERRTRESH_DEFAULT;
        config->TxCurlimGain    = PTX_NSC_INIT_TXCURLIMGAIN_DEFAULT;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NSC_System_UpdateConfiguration(ptx_NSC_t *nsc, ptx_NSC_SystemConfig_Param_t *params, uint8_t cnt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_System_UpdateConfiguration");

        if ( (NULL != params) && (0 < cnt) && (PTX_NSC_SYSTEM_CONFIG_PARAM_ID_MAX_PARAMID >= cnt) )
        {
            /* Input parameters are valid and the list is not empty. Load the parameters into NSC internal SysCfg structure. */
            status = PTX_STATUS_SUCCESS;

            for (uint8_t i = 0; i < cnt; ++i)
            {
                switch (params[i].Config)
                {
                    case NSC_SystemConfig_Config_ClkSel:
                        nsc->SysCfg.ClkSel = params[i].Value.ClkSel;
                        break;

                    case NSC_SystemConfig_Config_ClkReqEn:
                        nsc->SysCfg.ClkReqEn = params[i].Value.ClkReqEn;
                        break;

                    case NSC_SystemConfig_Config_ClkReqGt:
                        nsc->SysCfg.ClkReqGt = params[i].Value.ClkReqGt;
                        break;

                    case NSC_SystemConfig_Config_RefFreq:
                        nsc->SysCfg.RefFreq = params[i].Value.RefFreq;
                        break;

                    case NSC_SystemConfig_Config_TargetFreq:
                        nsc->SysCfg.TargetFreq = params[i].Value.TargetFreq;
                        break;

                    case NSC_SystemConfig_Config_PrngSeed:
                        nsc->SysCfg.PrngSeed = params[i].Value.PrngSeed;
                        break;

                    case NSC_SystemConfig_Config_NalmMax:
                        nsc->SysCfg.NalmMax = params[i].Value.NalmMax;
                        break;

                    case NSC_SystemConfig_Config_NalmMin:
                        nsc->SysCfg.NalmMin = params[i].Value.NalmMin;
                        break;

                    case NSC_SystemConfig_Config_NAlmOffset:
                        nsc->SysCfg.NAlmOffset = params[i].Value.NAlmOffset;
                        break;

                    case NSC_SystemConfig_Config_XtalTrim:
                        nsc->SysCfg.XtalTrim = params[i].Value.XtalTrim;
                        break;

                    case NSC_SystemConfig_Config_TempErrTresh:
                        nsc->SysCfg.TempErrTreshold = params[i].Value.TempErrTresh;
                        break;

                    case NSC_SystemConfig_Config_TxCurlimGain:
                        nsc->SysCfg.TxCurlimGain = params[i].Value.TxCurlimGain;
                        break;

                    default:
                        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                        break;
                }

                if (PTX_STATUS_SUCCESS != status)
                {
                    /* Invalid parameter provided. Abort. */
                    break;
                }
            }
        }
        else if (NULL != nsc->SysCfg.LoadSystemParam)
        {
            /*
             * Input parameters are not valid, but CB has been registered to load system configuration.
             * Call the CB to load the parameters into NSC internal SysCfg structure.
             */
            status = nsc->SysCfg.LoadSystemParam(nsc);
        }
        else
        {
            /* Invalid parameters provided. Keep the error. */
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_System_SendSystemConfig(nsc);
        }

    }


    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_System_SendSystemConfig(ptx_NSC_t* nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_System_SendSystemConfig");

        if (nsc->State == ptx_NSC_State_Init)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            /* System is not in INIT state. Must be turned into INIT state now. */

            /* Activate the FW. This shall first reset the chip, and then retrieve and check the reset notification. */
            status = ptx_NSC_Firmware_Activate(nsc, NULL);

            /*
            * If activation was successful, NSC_RESET_NTF has arrived and has been handled
            * in ptx_NSC_Firmware_Activate API.
            * System turned into ptx_NSC_State_Init state in command processor notification handler.
            */
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /*
            * All successful up to now. NSC is in ptx_NSC_State_Init state.
            * The system (stack and PTX chip) is synchronized and may be initialised.
            */
            status = ptx_NSC_InitHw(nsc);
        }
    }
    return status;

}
