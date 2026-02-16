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
    Module      : NFC Soft Controller (NSC) Interface for Reader Universal Library (RUL) RF Configuration
    File        : ptx_NSC_RUL_RfConfig.c

    Description : Implementation of NFC Soft Controller Interface for Reader Universal Library RF Configuration.
                  Contains function(s) for RUL-specific handling of RF configuration parameters.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_RUL_RfConfig.h"
#include "ptx_NSC_RfConfigVal.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/* Register offsets in the general POLL parameters section. */
#define PTX_NSC_RFCONFIG_IDX_POLL_PL0           (0u)
#define PTX_NSC_RFCONFIG_IDX_POLL_PL1           (1u)
#define PTX_NSC_RFCONFIG_IDX_POLL_DDPC_AVG      (2u)
#define PTX_NSC_RFCONFIG_IDX_POLL_PL0THRESH1    (3u)
#define PTX_NSC_RFCONFIG_IDX_POLL_PL0THRESH0    (4u)
#define PTX_NSC_RFCONFIG_IDX_POLL_PL1THRESH1    (5u)
#define PTX_NSC_RFCONFIG_IDX_POLL_PL1THRESH0    (6u)

/* Register offsets in the technology specific POLL/LISTEN MODE parameters sections. */
#define PTX_NSC_RFCONFIG_IDX_ANARXFE_CONFIG     (0u)
#define PTX_NSC_RFCONFIG_IDX_ANARXBB_CONFIG     (1u)
#define PTX_NSC_RFCONFIG_IDX_CLDRXDF_A          (11u)
#define PTX_NSC_RFCONFIG_IDX_CLDRXDF_B          (12u)
#define PTX_NSC_RFCONFIG_IDX_CLDRXDF_F          (10u)
#define PTX_NSC_RFCONFIG_IDX_CLDRXDF_V          (12u)


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NSC_RfConfig_GetParamPointer(ptx_NSC_t *nsc, ptx_NSC_RfConfig_ParamId_t id, uint8_t **value);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_RUL_RfConfig_LoadParam(ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        ptx_NSC_RfConfig_t *config = &(nsc->RfCfg);

        /* Check the version */
        uint8_t file_version_major = ptx_NSC_RfConfig_Version_Major;
        if (PTX_NSC_RFCONFIG_VERSION == file_version_major)
        {
            status = PTX_STATUS_SUCCESS;
            (void)memset(&config->RfCfgParam[0], 0, sizeof(ptx_NSC_RfConfig_Param_t) * (PTX_NSC_RFCONFIG_PARAMID_MAX_PARAMID));

            /* Wavegen pattern shall be loaded before the generic poll configuration. */
            uint8_t idx = 0;

            config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_Wavepattern;
            config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_Wavepattern_len;
            config->RfCfgParam[idx].Id = NSC_RfConfig_ParamId_WaveGenPat;
            ++idx;

            for (int i = NSC_RfConfig_ParamId_Poll; (i < (NSC_RfConfig_ParamId_MAX) && (idx < PTX_NSC_RFCONFIG_PARAMID_MAX_PARAMID)); ++i)
            {
                switch (i)
                {
                    case NSC_RfConfig_ParamId_Poll:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_Poll;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_Poll_len;
                        break;

                    case NSC_RfConfig_ParamId_PollA106:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollA106;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollA106_len;
                        break;

                    case NSC_RfConfig_ParamId_PollA212:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollA212;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollA212_len;
                        break;

                    case NSC_RfConfig_ParamId_PollA424:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollA424;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollA424_len;
                        break;

                    case NSC_RfConfig_ParamId_PollA848:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollA848;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollA848_len;
                        break;

                    case NSC_RfConfig_ParamId_PollB106:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollB106;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollB106_len;
                        break;

                    case NSC_RfConfig_ParamId_PollB212:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollB212;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollB212_len;
                        break;

                    case NSC_RfConfig_ParamId_PollB424:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollB424;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollB424_len;
                        break;

                    case NSC_RfConfig_ParamId_PollB848:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollB848;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollB848_len;
                        break;

                    case NSC_RfConfig_ParamId_PollF212:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollF212;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollF212_len;
                        break;

                    case NSC_RfConfig_ParamId_PollF424:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollF424;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollF424_len;
                        break;

                    case NSC_RfConfig_ParamId_PollV_26K5:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollV26_5;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollV26_5_len;
                        break;

                    case NSC_RfConfig_ParamId_PollV53:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollV53;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollV53_len;
                        break;

                    case NSC_RfConfig_ParamId_PollV106:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_PollV106;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_PollV106_len;
                        break;

#if !defined(PTX_DISABLE_LISTEN_MODE)
                    case NSC_RfConfig_ParamId_Listen:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_Listen;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_Listen_len;
                        break;

                    case NSC_RfConfig_ParamId_ListenA212:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_ListenA212;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_ListenA212_len;
                        break;

                    case NSC_RfConfig_ParamId_ListenA424:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_ListenA424;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_ListenA424_len;
                        break;

                    case NSC_RfConfig_ParamId_ListenA848:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_ListenA848;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_ListenA848_len;
                        break;

                    case NSC_RfConfig_ParamId_ListenB212:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_ListenB212;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_ListenB212_len;
                        break;

                    case NSC_RfConfig_ParamId_ListenB424:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_ListenB424;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_ListenB424_len;
                        break;

                    case NSC_RfConfig_ParamId_ListenB848:
                        config->RfCfgParam[idx].Value = ptx_NSC_RfConfig_ListenB848;
                        config->RfCfgParam[idx].Length = ptx_NSC_RfConfig_ListenB848_len;
                        break;
#endif /* PTX_DISABLE_LISTEN_MODE */

                    default:
                        /* Non-existent Id, skip. */
                        status = PTX_STATUS_E_GEN_NOT_FOUND;
                        break;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Id was known, set Id member and increment array position. */
                    config->RfCfgParam[idx].Id = i;
                    ++idx;
                }
                else
                {
                    /* An unknown Id was found, skip it and continue. */
                    status = PTX_STATUS_SUCCESS;
                }
            }

            /*
             * The loop always exits with status PTX_STATUS_SUCCESS.
             * Check if any parameter has been loaded.
             */
            if (0 != idx)
            {
                /* Finally, assign parameter list to the output parameter if any parameter has been loaded. */
                config->RfCfgParamLen = idx;
            }
            else
            {
                /* No parameter loaded. */
                status = PTX_STATUS_E_GEN_NOT_FOUND;
            }
        }
        else
        {
            /* RF Configuration file version mismatch. */
            status = PTX_STATUS_E_NSC_INVALID_RFCONFIG;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfConfig_SetSection(ptx_NSC_t *nsc, ptx_NSC_RfConfig_Param_t *config)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != config) )
    {
        switch (config->Id)
        {
            case NSC_RfConfig_ParamId_Poll:
                if (ptx_NSC_RfConfig_Poll_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_Poll, config->Value, ptx_NSC_RfConfig_Poll_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollA106:
                if (ptx_NSC_RfConfig_PollA106_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollA106, config->Value, ptx_NSC_RfConfig_PollA106_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollA212:
                if (ptx_NSC_RfConfig_PollA212_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollA212, config->Value, ptx_NSC_RfConfig_PollA212_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollA424:
                if (ptx_NSC_RfConfig_PollA424_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollA424, config->Value, ptx_NSC_RfConfig_PollA424_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollA848:
                if (ptx_NSC_RfConfig_PollA848_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollA848, config->Value, ptx_NSC_RfConfig_PollA848_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollB106:
                if (ptx_NSC_RfConfig_PollB106_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollB106, config->Value, ptx_NSC_RfConfig_PollB106_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollB212:
                if (ptx_NSC_RfConfig_PollB212_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollB212, config->Value, ptx_NSC_RfConfig_PollB212_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollB424:
                if (ptx_NSC_RfConfig_PollB424_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollB424, config->Value, ptx_NSC_RfConfig_PollB424_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollB848:
                if (ptx_NSC_RfConfig_PollB848_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollB848, config->Value, ptx_NSC_RfConfig_PollB848_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollF212:
                if (ptx_NSC_RfConfig_PollF212_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollF212, config->Value, ptx_NSC_RfConfig_PollF212_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollF424:
                if (ptx_NSC_RfConfig_PollF424_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollF424, config->Value, ptx_NSC_RfConfig_PollF424_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollV_26K5:
                if (ptx_NSC_RfConfig_PollV26_5_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollV26_5, config->Value, ptx_NSC_RfConfig_PollV26_5_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollV53:
                if (ptx_NSC_RfConfig_PollV53_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollV53, config->Value, ptx_NSC_RfConfig_PollV53_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_PollV106:
                if (ptx_NSC_RfConfig_PollV106_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_PollV106, config->Value, ptx_NSC_RfConfig_PollV106_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

#if !defined(PTX_DISABLE_LISTEN_MODE)
            case NSC_RfConfig_ParamId_Listen:
                if (ptx_NSC_RfConfig_Listen_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_Listen, config->Value, ptx_NSC_RfConfig_Listen_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_ListenA212:
                if (ptx_NSC_RfConfig_ListenA212_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_ListenA212, config->Value, ptx_NSC_RfConfig_ListenA212_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_ListenA424:
                if (ptx_NSC_RfConfig_ListenA424_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_ListenA424, config->Value, ptx_NSC_RfConfig_ListenA424_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_ListenA848:
                if (ptx_NSC_RfConfig_ListenA848_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_ListenA848, config->Value, ptx_NSC_RfConfig_ListenA848_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_ListenB212:
                if (ptx_NSC_RfConfig_ListenB212_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_ListenB212, config->Value, ptx_NSC_RfConfig_ListenB212_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_ListenB424:
                if (ptx_NSC_RfConfig_ListenB424_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_ListenB424, config->Value, ptx_NSC_RfConfig_ListenB424_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case NSC_RfConfig_ParamId_ListenB848:
                if (ptx_NSC_RfConfig_ListenB848_len == config->Length)
                {
                    memcpy(ptx_NSC_RfConfig_ListenB848, config->Value, ptx_NSC_RfConfig_ListenB848_len);
                    status = PTX_STATUS_SUCCESS;
                }
                break;
#endif /* PTX_DISABLE_LISTEN_MODE */

            default:
                status = PTX_STATUS_E_GEN_NOT_FOUND;
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfConfig_GetSection(ptx_NSC_t *nsc, ptx_NSC_RfConfig_Param_t *config)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != config) )
    {
        status = PTX_STATUS_SUCCESS;
        switch (config->Id)
        {
            case NSC_RfConfig_ParamId_Poll:
                config->Length = ptx_NSC_RfConfig_Poll_len;
                config->Value = ptx_NSC_RfConfig_Poll;
                break;

            case NSC_RfConfig_ParamId_PollA106:
                config->Length = ptx_NSC_RfConfig_PollA106_len;
                config->Value = ptx_NSC_RfConfig_PollA106;
                break;

            case NSC_RfConfig_ParamId_PollB106:
                config->Length = ptx_NSC_RfConfig_PollB106_len;
                config->Value = ptx_NSC_RfConfig_PollB106;
                break;

            case NSC_RfConfig_ParamId_PollF212:
                config->Length = ptx_NSC_RfConfig_PollF212_len;
                config->Value = ptx_NSC_RfConfig_PollF212;
                break;

            case NSC_RfConfig_ParamId_PollF424:
                config->Length = ptx_NSC_RfConfig_PollF424_len;
                config->Value = ptx_NSC_RfConfig_PollF424;
                break;

            case NSC_RfConfig_ParamId_PollV_26K5:
                config->Length = ptx_NSC_RfConfig_PollV26_5_len;
                config->Value = ptx_NSC_RfConfig_PollV26_5;
                break;

            case NSC_RfConfig_ParamId_PollA212:
                config->Length = ptx_NSC_RfConfig_PollA212_len;
                config->Value = ptx_NSC_RfConfig_PollA212;
                break;

            case NSC_RfConfig_ParamId_PollA424:
                config->Length = ptx_NSC_RfConfig_PollA424_len;
                config->Value = ptx_NSC_RfConfig_PollA424;
                break;

            case NSC_RfConfig_ParamId_PollB212:
                config->Length = ptx_NSC_RfConfig_PollB212_len;
                config->Value = ptx_NSC_RfConfig_PollB212;
                break;

            case NSC_RfConfig_ParamId_PollB424:
                config->Length = ptx_NSC_RfConfig_PollB424_len;
                config->Value = ptx_NSC_RfConfig_PollB424;
                break;

            case NSC_RfConfig_ParamId_PollB848:
                config->Length = ptx_NSC_RfConfig_PollB848_len;
                config->Value = ptx_NSC_RfConfig_PollB848;
                break;

#if !defined(PTX_DISABLE_LISTEN_MODE)
            case NSC_RfConfig_ParamId_Listen:
                config->Length = ptx_NSC_RfConfig_Listen_len;
                config->Value = ptx_NSC_RfConfig_Listen;
                break;

            case NSC_RfConfig_ParamId_ListenA212:
                config->Length = ptx_NSC_RfConfig_ListenA212_len;
                config->Value = ptx_NSC_RfConfig_ListenA212;
                break;

            case NSC_RfConfig_ParamId_ListenA424:
                config->Length = ptx_NSC_RfConfig_ListenA424_len;
                config->Value = ptx_NSC_RfConfig_ListenA424;
                break;

            case NSC_RfConfig_ParamId_ListenA848:
                config->Length = ptx_NSC_RfConfig_ListenA848_len;
                config->Value = ptx_NSC_RfConfig_ListenA848;
                break;

            case NSC_RfConfig_ParamId_ListenB212:
                config->Length = ptx_NSC_RfConfig_ListenB212_len;
                config->Value = ptx_NSC_RfConfig_ListenB212;
                break;

            case NSC_RfConfig_ParamId_ListenB424:
                config->Length = ptx_NSC_RfConfig_ListenB424_len;
                config->Value = ptx_NSC_RfConfig_ListenB424;
                break;

            case NSC_RfConfig_ParamId_ListenB848:
                config->Length = ptx_NSC_RfConfig_ListenB848_len;
                config->Value = ptx_NSC_RfConfig_ListenB848;
                break;
#endif /* PTX_DISABLE_LISTEN_MODE */

            default:
                status = PTX_STATUS_E_GEN_NOT_FOUND;
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfConfig_SetParam(ptx_NSC_t *nsc, ptx_NSC_RfConfig_ParamId_t id, uint8_t value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) && (ParamId_Max > id) )
    {
        uint8_t *parameter_ptr;

        status = ptx_NSC_RfConfig_GetParamPointer(nsc, id, &parameter_ptr);

        if (PTX_STATUS_SUCCESS == status)
        {
            *parameter_ptr = value;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfConfig_GetParam(ptx_NSC_t *nsc, ptx_NSC_RfConfig_ParamId_t id, uint8_t *value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) && (ParamId_Max > id) && (NULL != value) )
    {
        uint8_t *parameter_ptr;

        status = ptx_NSC_RfConfig_GetParamPointer(nsc, id, &parameter_ptr);

        if (PTX_STATUS_SUCCESS == status)
        {
            *value = *parameter_ptr;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_GetRfConfigInfo(ptx_NSC_t *nsc, uint8_t *infoData, uint8_t *infoDataLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != infoData) && (NULL != infoDataLen) )
    {
        const uint8_t rfconfig_info_size = 2U;
        infoData[0] = ptx_NSC_RfConfig_Version_Major;
        infoData[1] = ptx_NSC_RfConfig_Version_Minor;
        *infoDataLen = rfconfig_info_size;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_RfConfig_GetParamPointer(ptx_NSC_t *nsc, ptx_NSC_RfConfig_ParamId_t id, uint8_t **value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) && (ParamId_Max > id) && (NULL != value) )
    {
        ptx_NSC_RfConfig_Param_t poll_param_section;
        memset(&poll_param_section, 0, sizeof(ptx_NSC_RfConfig_Param_t));

        status = PTX_STATUS_SUCCESS;
        switch (id)
        {
            case ParamId_PollPwrLvl0:           /* Fall through. */
            case ParamId_PollPwrLvl1:           /* Fall through. */
            case ParamId_PollDdpcAvg:           /* Fall through. */
            case ParamId_PollPwrLvl0_Thr0:      /* Fall through. */
            case ParamId_PollPwrLvl0_Thr1:      /* Fall through. */
            case ParamId_PollPwrLvl1_Thr0:      /* Fall through. */
            case ParamId_PollPwrLvl1_Thr1:
                poll_param_section.Id = NSC_RfConfig_ParamId_Poll;
                break;

            case ParamId_AnaRxFeConfig_A106:    /* Fall through. */
            case ParamId_AnaRxBbConfig_A106:    /* Fall through. */
            case ParamId_ClDrxDf_A106:
                poll_param_section.Id = NSC_RfConfig_ParamId_PollA106;
                break;

            case ParamId_AnaRxFeConfig_A212:    /* Fall through. */
            case ParamId_AnaRxBbConfig_A212:    /* Fall through. */
            case ParamId_ClDrxDf_A212:
                poll_param_section.Id = NSC_RfConfig_ParamId_PollA212;
                break;

            case ParamId_AnaRxFeConfig_A424:    /* Fall through. */
            case ParamId_AnaRxBbConfig_A424:    /* Fall through. */
            case ParamId_ClDrxDf_A424:
                poll_param_section.Id = NSC_RfConfig_ParamId_PollA424;
                break;

            case ParamId_AnaRxFeConfig_A848:    /* Fall through. */
            case ParamId_AnaRxBbConfig_A848:    /* Fall through. */
            case ParamId_ClDrxDf_A848:
                poll_param_section.Id = NSC_RfConfig_ParamId_PollA848;
                break;

            case ParamId_AnaRxFeConfig_B106:    /* Fall through. */
            case ParamId_AnaRxBbConfig_B106:    /* Fall through. */
            case ParamId_ClDrxDf_B106:
                poll_param_section.Id = NSC_RfConfig_ParamId_PollB106;
                break;

            case ParamId_AnaRxFeConfig_B212:    /* Fall through. */
            case ParamId_AnaRxBbConfig_B212:    /* Fall through. */
            case ParamId_ClDrxDf_B212:
                poll_param_section.Id = NSC_RfConfig_ParamId_PollB212;
                break;

            case ParamId_AnaRxFeConfig_B424:    /* Fall through. */
            case ParamId_AnaRxBbConfig_B424:    /* Fall through. */
            case ParamId_ClDrxDf_B424:
                poll_param_section.Id = NSC_RfConfig_ParamId_PollB424;
                break;

            case ParamId_AnaRxFeConfig_B848:    /* Fall through. */
            case ParamId_AnaRxBbConfig_B848:    /* Fall through. */
            case ParamId_ClDrxDf_B848:
                poll_param_section.Id = NSC_RfConfig_ParamId_PollB848;
                break;

            case ParamId_AnaRxFeConfig_F212:    /* Fall through. */
            case ParamId_AnaRxBbConfig_F212:    /* Fall through. */
            case ParamId_ClDrxDf_F212:
                poll_param_section.Id = NSC_RfConfig_ParamId_PollF212;
                break;

            case ParamId_AnaRxFeConfig_F424:    /* Fall through. */
            case ParamId_AnaRxBbConfig_F424:    /* Fall through. */
            case ParamId_ClDrxDf_F424:
                poll_param_section.Id = NSC_RfConfig_ParamId_PollF424;
                break;

            case ParamId_AnaRxFeConfig_V26:    /* Fall through. */
            case ParamId_AnaRxBbConfig_V26:    /* Fall through. */
            case ParamId_ClDrxDf_V26:
                poll_param_section.Id = NSC_RfConfig_ParamId_PollV_26K5;
                break;

            case ParamId_Poll_Max:              /* Fall through. */
            case ParamId_AnaRxFeConfig_Max:     /* Fall through. */
            case ParamId_AnaRxBbConfig_Max:     /* Fall through. */
            case ParamId_Max:                   /* Fall through. */
            default:
                status = PTX_STATUS_E_GEN_NOT_FOUND;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RfConfig_GetSection(nsc, &poll_param_section);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (id)
            {
                case ParamId_PollPwrLvl0:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_POLL_PL0];
                    break;

                case ParamId_PollPwrLvl1:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_POLL_PL1];
                    break;

                case ParamId_PollDdpcAvg:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_POLL_DDPC_AVG];
                    break;

                case ParamId_PollPwrLvl0_Thr1:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_POLL_PL0THRESH1];
                    break;

                case ParamId_PollPwrLvl0_Thr0:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_POLL_PL0THRESH0];
                    break;

                case ParamId_PollPwrLvl1_Thr1:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_POLL_PL1THRESH1];
                    break;

                case ParamId_PollPwrLvl1_Thr0:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_POLL_PL1THRESH0];
                    break;

                case ParamId_AnaRxFeConfig_A106:    /* Fall through. */
                case ParamId_AnaRxFeConfig_A212:    /* Fall through. */
                case ParamId_AnaRxFeConfig_A424:    /* Fall through. */
                case ParamId_AnaRxFeConfig_A848:    /* Fall through. */
                case ParamId_AnaRxFeConfig_B106:    /* Fall through. */
                case ParamId_AnaRxFeConfig_B212:    /* Fall through. */
                case ParamId_AnaRxFeConfig_B424:    /* Fall through. */
                case ParamId_AnaRxFeConfig_B848:    /* Fall through. */
                case ParamId_AnaRxFeConfig_F212:    /* Fall through. */
                case ParamId_AnaRxFeConfig_F424:    /* Fall through. */
                case ParamId_AnaRxFeConfig_V26:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_ANARXFE_CONFIG];
                    break;

                case ParamId_AnaRxBbConfig_A106:    /* Fall through. */
                case ParamId_AnaRxBbConfig_A212:    /* Fall through. */
                case ParamId_AnaRxBbConfig_A424:    /* Fall through. */
                case ParamId_AnaRxBbConfig_A848:    /* Fall through. */
                case ParamId_AnaRxBbConfig_B106:    /* Fall through. */
                case ParamId_AnaRxBbConfig_B212:    /* Fall through. */
                case ParamId_AnaRxBbConfig_B424:    /* Fall through. */
                case ParamId_AnaRxBbConfig_B848:    /* Fall through. */
                case ParamId_AnaRxBbConfig_F212:    /* Fall through. */
                case ParamId_AnaRxBbConfig_F424:    /* Fall through. */
                case ParamId_AnaRxBbConfig_V26:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_ANARXBB_CONFIG];
                    break;

                case ParamId_ClDrxDf_A106:
                case ParamId_ClDrxDf_A212:
                case ParamId_ClDrxDf_A424:
                case ParamId_ClDrxDf_A848:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_CLDRXDF_A];
                    break;

                case ParamId_ClDrxDf_B106:
                case ParamId_ClDrxDf_B212:
                case ParamId_ClDrxDf_B424:
                case ParamId_ClDrxDf_B848:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_CLDRXDF_B];
                    break;

                case ParamId_ClDrxDf_F212:
                case ParamId_ClDrxDf_F424:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_CLDRXDF_F];
                    break;

                case ParamId_ClDrxDf_V26:
                    *value = &poll_param_section.Value[PTX_NSC_RFCONFIG_IDX_CLDRXDF_V];
                    break;

                default:
                    status = PTX_STATUS_E_GEN_NOT_FOUND;
                    break;
            }
        }
    }

    return status;
}
