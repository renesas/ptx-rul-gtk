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
    Module      : NFC Soft Controller (NSC) Interface for RF Configuration
    File        : ptx_NSC_RfConfig.c

    Description : Implementation of NFC Soft Controller Interface for RF Configuration.
                  Contains function(s) to perform the system RF configuration before discovery process
                  and to gather information about the currently used RF configuration parameters.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Int.h"
#include "ptx_NSC_StateMachine.h"
#include <string.h>


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * Definitions for NSC_RFCONFIG_CMD.
 */
#define PTX_NSC_RFCONFIG_CMD_TAG_LENGTH             (0x01u)     /* NSC RF Config tag length */
#define PTX_NSC_RFCONFIG_CMD_PARAM_LAST             (0x00u)     /* NSC RF Config command param last */
#define PTX_NSC_RFCONFIG_CMD_PARAM_NOONE_LENGTH     (0x01u)     /* NSC RF Config command param NOONE */

#define PTX_NSC_RFCONFIG_BUFFER_MAX_LEN             PTX_NSC_LENGTH_MAX      /* RF Config Buffer maximum size */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NSC_RfConfig_Send(ptx_NSC_t *nsc, ptx_NSC_RfConfig_Param_t *params, uint8_t paramCnt);
static ptx_Status_t ptx_NSC_RfConfig_AddToCmd(uint8_t *buffer, uint32_t *buffer_offset, ptx_NSC_RfConfig_Param_t *params);
static uint8_t ptx_NSC_RfConfig_CheckId(ptx_NSC_RfConfig_ParamListId_t paramId);
static ptx_Status_t ptx_NSC_RfConfig_Dispatch(ptx_NSC_t *nsc, uint8_t *buffer, uint32_t *bufferOffset);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_RfConfig (ptx_NSC_t *nsc, ptx_NSC_RfConfig_Param_t *rfConfigParams, uint32_t rfConfigParamCnt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) )
    {
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_RfConfig, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Checks up to now were successful. However, config has still not taken place. So, set error status. */
        status = PTX_STATUS_E_GEN_INTERNAL;

        if ((NULL == rfConfigParams) && (0 == rfConfigParamCnt))
        {
            /* No parameters provided. Try to load parameters via LoadCfg callback. */
            if (NULL != nsc->RfCfg.LoadCfgParam)
            {
                status = nsc->RfCfg.LoadCfgParam(nsc);
            }
            else
            {
                /* Invalid setup. LoadCfg is not set. */
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Have the parameters been properly loaded? */
                if ((0 != nsc->RfCfg.RfCfgParamLen) &&
                    (PTX_NSC_RFCONFIG_PARAMID_MAX_PARAMID >= nsc->RfCfg.RfCfgParamLen))
                {
                    status = ptx_NSC_RfConfig_Send(nsc, nsc->RfCfg.RfCfgParam, (uint8_t)nsc->RfCfg.RfCfgParamLen);
                }
                else
                {
                    /* No. */
                    status = PTX_STATUS_E_GEN_INTERNAL;
                }
            }
        }
        else if ((NULL != rfConfigParams) && (0 < rfConfigParamCnt) && (PTX_NSC_RFCONFIG_PARAMID_MAX_PARAMID >= rfConfigParamCnt))
        {
            status = ptx_NSC_RfConfig_Send(nsc, rfConfigParams, (uint8_t)rfConfigParamCnt);
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static uint8_t ptx_NSC_RfConfig_CheckId(ptx_NSC_RfConfig_ParamListId_t paramId)
{
    uint8_t ret = 0;

    ptx_NSC_RfConfig_ParamListId_t min_poll_id = NSC_RfConfig_ParamId_EoC;
    ptx_NSC_RfConfig_ParamListId_t max_poll_id = NSC_RfConfig_ParamId_EoC;
#if !defined(PTX_DISABLE_LISTEN_MODE)
    ptx_NSC_RfConfig_ParamListId_t min_listen_id = NSC_RfConfig_ParamId_EoC;
    ptx_NSC_RfConfig_ParamListId_t max_listen_id = NSC_RfConfig_ParamId_EoC;
#endif /* PTX_DISABLE_LISTEN_MODE */

    min_poll_id = NSC_RfConfig_ParamId_Poll;
    max_poll_id = NSC_RfConfig_ParamId_PollV106;
#if !defined(PTX_DISABLE_LISTEN_MODE)
    min_listen_id = NSC_RfConfig_ParamId_ListenA212;
    max_listen_id = NSC_RfConfig_ParamId_Listen;
#endif /* PTX_DISABLE_LISTEN_MODE */

    /* Return 1 if ID is valid. Otherwise, return 0. */
    if ((paramId >= min_poll_id) && (paramId <= max_poll_id))
    {
        /* One of POLL parameters. */
        ret = 1;
    }
#if !defined(PTX_DISABLE_LISTEN_MODE)
    else if ((paramId >= min_listen_id) && (paramId <= max_listen_id))
    {
        /* One of LISTEN parameters. */
        ret = 1;

        /* For B20 we have gaps between defined and undefined values in the parameter list. */
        const uint8_t numof_unsupported = 5U;
        const uint8_t unsupported_values[] = {0x15, 0x19, 0x1A, 0x1B, 0x1C};

        for (uint8_t i = 0; i < numof_unsupported; i++)
        {
            if (unsupported_values[i] == paramId)
            {
                ret = 0;
                break;
            }
        }
    }
#endif /* PTX_DISABLE_LISTEN_MODE */
    else if (NSC_RfConfig_ParamId_WaveGenPat == paramId)
    {
        ret = 1;
    }
    else
    {
        /* Other undefined values. */
    }

    return ret;
}

static ptx_Status_t ptx_NSC_RfConfig_AddToCmd(uint8_t *buffer, uint32_t *buffer_offset, ptx_NSC_RfConfig_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INTERNAL;

    uint32_t idx = *buffer_offset;

    if (0 == idx)
    {
        buffer[idx] = PTX_NSC_RF_CONFIG_CMD_OPCODE;
        ++idx;
    }

    if (idx + params->Length
        + PTX_NSC_RFCONFIG_CMD_TAG_LENGTH
        + PTX_NSC_RFCONFIG_CMD_PARAM_NOONE_LENGTH
        <= PTX_NSC_RFCONFIG_BUFFER_MAX_LEN)
    {
        buffer[idx] = (uint8_t)params->Id;
        ++idx;

        (void)memcpy(&buffer[idx], params->Value, params->Length);
        idx += params->Length;

        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        if (params->Length
            + PTX_NSC_CMD_OPCODE_LEN
            + PTX_NSC_RFCONFIG_CMD_TAG_LENGTH
            + PTX_NSC_RFCONFIG_CMD_PARAM_NOONE_LENGTH
            <= PTX_NSC_RFCONFIG_BUFFER_MAX_LEN)
        {
            status = PTX_STATUS_E_NSC_NO_FIT_TAG;
        }
        else
        {
            status = PTX_STATUS_E_NSC_TAG_NEVER_FITS;
        }
    }

    *buffer_offset = idx;

    return status;
}

static ptx_Status_t ptx_NSC_RfConfig_Send(ptx_NSC_t *nsc, ptx_NSC_RfConfig_Param_t *params, uint8_t paramCnt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t ready_to_send = 0;
    uint8_t buffer_full = 0;
    uint32_t buffer_offset = 0;

    uint8_t buffer[PTX_NSC_RFCONFIG_BUFFER_MAX_LEN];
    memset(buffer, 0, PTX_NSC_RFCONFIG_BUFFER_MAX_LEN);

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != params) && (0 != paramCnt))
    {
        for (uint8_t i = 0; i < paramCnt; i++)
        {
            ready_to_send = 0;
            buffer_full = 0;

            /* Default: provided ID is not valid until it is proven differently.
             * If ID is invalid, the loop shall be exited.
             */
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

            /* Check if ID is valid. */
            if (0 != ptx_NSC_RfConfig_CheckId(params[i].Id))
            {
                /* Parameter ID is valid. Try to load the data into the message buffer. */
                status = ptx_NSC_RfConfig_AddToCmd(buffer, &buffer_offset, &params[i]);
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Data has been added. Just check if this is the last parameter in the input argument list.
                     * In that case, set the flag to send the command.
                     */
                    if ((i + 1) >= paramCnt)
                    {
                        ready_to_send = 1U;
                    }
                }
                else
                {
                    /* Buffer is full: parameter data does not fit into the message buffer.
                     * Set the flag to send the command with the pre-filled data.
                     */
                    buffer_full = 1;
                    ready_to_send = 1;
                    status = PTX_STATUS_SUCCESS;
                }

                if ((PTX_STATUS_SUCCESS == status) && (0 != ready_to_send))
                {
                    /* Send the command and receive the response. */
                    status = ptx_NSC_RfConfig_Dispatch(nsc, buffer, &buffer_offset);
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        memset(buffer, 0, PTX_NSC_RFCONFIG_BUFFER_MAX_LEN);
                        buffer_offset = 0;
                    }

                    if ((PTX_STATUS_SUCCESS == status) && (0 != buffer_full))
                    {
                        /* Since current parameter hasn't been loaded, do it now. */
                        status = ptx_NSC_RfConfig_AddToCmd(buffer, &buffer_offset, &params[i]);

                        if ((PTX_STATUS_SUCCESS == status) && ((i + 1) >= paramCnt))
                        {
                            /* This is the last parameter in the input argument list. Send the command. */
                            status = ptx_NSC_RfConfig_Dispatch(nsc, buffer, &buffer_offset);
                        }
                    }
                }
            }

            if (PTX_STATUS_SUCCESS != status)
            {
                /* Error occurred. Exit the loop. */
                break;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_RfConfig_Dispatch(ptx_NSC_t *nsc, uint8_t *buffer, uint32_t *bufferOffset)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t buffer_offset = *bufferOffset;

    if (0 != buffer_offset)
    {
        /* We shall only send a non-empty buffer. */

        /* Mark EndOfCommand. */
        buffer[buffer_offset] = PTX_NSC_RFCONFIG_CMD_PARAM_LAST;
        ++buffer_offset;

        /* Send the command. */
        status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, buffer, buffer_offset);
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Now get the response. */
            uint8_t rsp_data[PTX_NSC_CMD_PROCESSOR_RFCONFIG_RSPLEN];
            uint32_t rsp_data_len = PTX_NSC_CMD_PROCESSOR_RFCONFIG_RSPLEN;
            uint8_t ntf_to_follow = 0;

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_data, &rsp_data_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_RFCONFIG_RSP);
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_RfConfig, rsp_data, rsp_data_len, &ntf_to_follow);
            }
            else
            {
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            *bufferOffset = buffer_offset;
        }
    }

    return status;
}

