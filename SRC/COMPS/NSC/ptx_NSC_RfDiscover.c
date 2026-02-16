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
    Module      : NFC Soft Controller (NSC) Interface for RF Discovery
    File        : ptx_NSC_RfDiscover.c

    Description : Implementation of NFC Soft Controller Interface for RF Discovery.
                  Contains functions to start RF Discovery process and to parse the information gathered
                  during discovery.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_RfDiscover.h"
#include "ptx_NSC_StateMachine.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_NSC_RF_DISCOVER_EMPTYNTF_LEN        (1U)    /* Empty Discover NTF Length (NSC_RF_DISCOVER_NTF without parameters). */


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_RfDiscover_Dispatch(ptx_NSC_t *nsc, uint8_t *txBuffer, uint32_t txBufferLen);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NSC_RfDiscover(ptx_NSC_t *nsc, ptx_NSC_RfDiscover_Param_t *discoverParams, uint32_t paramCnt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) )
    {
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_RfDiscover, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Allocate buffer for the RF-Discover command. */
        uint8_t nsc_buf[PTX_NSC_LENGTH_MAX];
        uint8_t *nsc_buf_pos = nsc_buf;

        uint32_t nsc_buf_free_len = PTX_NSC_LENGTH_MAX - PTX_NSC_CMD_OPCODE_LEN;

        /* Depending on the command configuration parameters, NSC might transition to a state other than RF_DISCOVERY, i.e. RF_POLL state.
         *  By default, next state after successful NSC_RF_DISCOVER_CMD will be RF_DISCOVERY.
         *  Use this variable to get the next specific pending state if required by specific poll parameters.
         */
        ptx_NSC_State_t disc_next_state = ptx_NSC_State_Discovery;

        /* NSC RF state after the last successful NSC_RF_DISCOVER_CMD.
         *  This is used in the case when no configuration parameters are provided, if there was an earlier successful Discovery.
         *  In such a case, the last used discovery configuration will be used by NSC and the state transition will follow that configuration.
         *
         *  NOTE: DO NOT OVERWRITE LastDiscState VARIABLE OUTSIDE THIS API!
         */
        ptx_NSC_State_t last_disc_state = nsc->LastDiscState;

        /* Store poll mode and update it after successful command transfer. */
        uint8_t poll_mode = nsc->PollMode;

        /* Append RF-Discover-Command opcode. */
        *nsc_buf_pos = PTX_NSC_RF_DISCOVER_CMD_OPCODE;
        nsc_buf_pos++;

        if ((NULL != discoverParams) && (0 < paramCnt) && (paramCnt <= PTX_NSC_RF_DISCOVER_MAX_PARAMCOUNT))
        {
            /* Parameters are provided and their number is within the limits. */

            uint32_t param_idx = 0;

            if ((NULL != nsc->ExtendDiscoveryParametersCb) && (NULL != nsc->ExtendDiscoveryParametersCtx))
            {
                status = nsc->ExtendDiscoveryParametersCb(nsc->ExtendDiscoveryParametersCtx, &nsc_buf_pos, &nsc_buf_free_len);
            }

            /* Iterate over specified parameters. */
            while ( (param_idx < paramCnt) && (PTX_STATUS_SUCCESS == status) )
            {
                switch (discoverParams[param_idx].Type)
                {
                    /*
                     * POLL mode parameters
                     */
                    case NSC_RfDiscover_Param_Type_Poll:
                        status = ptx_NSC_ParamParser_Poll(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        if (PTX_STATUS_SUCCESS == status)
                        {
                            poll_mode = (uint8_t)discoverParams[param_idx].Value.Poll.PollMode;
                        }
                        break;
                    case NSC_RfDiscover_Param_Type_PollAcm:
                        status = ptx_NSC_ParamParser_PollAcm(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
                    case NSC_RfDiscover_Param_Type_PollA:
                        status = ptx_NSC_ParamParser_PollA(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
                    case NSC_RfDiscover_Param_Type_PollB:
                        status = ptx_NSC_ParamParser_PollB(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
                    case NSC_RfDiscover_Param_Type_PollF:
                        status = ptx_NSC_ParamParser_PollF(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
                    case NSC_RfDiscover_Param_Type_PollV:
                        status = ptx_NSC_ParamParser_PollV(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
                    case NSC_RfDiscover_Param_Type_PollIsoDep:
                        status = ptx_NSC_ParamParser_PollIsoDep(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
                    case NSC_RfDiscover_Param_Type_PollNfcDep:
                        status = ptx_NSC_ParamParser_PollNfcDep(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;

                    /*
                     * LISTEN mode parameters
                     */
                    case NSC_RfDiscover_Param_Type_Listen:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                        status = ptx_NSC_ParamParser_Listen(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
#endif /* PTX_DISABLE_LISTEN_MODE*/
                    case NSC_RfDiscover_Param_Type_ListenA:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                        status = ptx_NSC_ParamParser_ListenA(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
#endif /* PTX_DISABLE_LISTEN_MODE*/
                    case NSC_RfDiscover_Param_Type_ListenB:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                        status = ptx_NSC_ParamParser_ListenB(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
#endif /* PTX_DISABLE_LISTEN_MODE*/
                    case NSC_RfDiscover_Param_Type_ListenF:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                        status = ptx_NSC_ParamParser_ListenF(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
#endif /* PTX_DISABLE_LISTEN_MODE*/
                    case NSC_RfDiscover_Param_Type_ListenIsoDep:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                        status = ptx_NSC_ParamParser_ListenIsoDep(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
#endif /* PTX_DISABLE_LISTEN_MODE*/
                    case NSC_RfDiscover_Param_Type_ListenNfcDep:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                        status = ptx_NSC_ParamParser_ListenNfcDep(&nsc_buf_pos, &nsc_buf_free_len, &discoverParams[param_idx].Value, &disc_next_state);
                        break;
#endif /* PTX_DISABLE_LISTEN_MODE*/

                    default:
                        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                        break;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Go to the next parameter */
                    param_idx++;
                }

            }
        }
        else if ((NULL == discoverParams) && (0 == paramCnt))
        {
            /*
             * No parameters are provided. NSC will use last used discovery configuration.
             *
             * If there was no successful RfDiscovery before this call, this is not a valid state for RfDiscovery execution.
             * NSC_RF_DISCOVER_CMD SHALL fail. However, we prevent the call here.
             */
            if ((ptx_NSC_State_Poll != last_disc_state) && (ptx_NSC_State_Discovery != last_disc_state))
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
            }
            else
            {
                /* Last state after discovery was RF_POLL or RF_DISCOVERY. Use it for the following transition. */
                disc_next_state = last_disc_state;
            }
        }
        else
        {
            /* Invalid parameters are provided to the API. */
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 != nsc_buf_free_len)
            {
                /* Everything fine up to now. */

                /* Append 'End-of-Command'. */
                *nsc_buf_pos = NSC_RfDiscover_Param_Type_EoC;
                nsc_buf_pos++;

                /* Send the remaining buffer to the chip. */
                status = ptx_NSC_RfDiscover_Dispatch(nsc, nsc_buf, (uint32_t)(nsc_buf_pos - nsc_buf));
            }
            else
            {
                /* Improper command structure. Do not continue. */
                status = PTX_STATUS_E_GEN_INTERNAL;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* NSC State is already set in RfDiscover Response handler to the RF_DISCOVERY state.
             *  By design it is ensured that ptx_NSC_RfDiscover_Dispatch will have the State properly set in case of successful return.
             *  Here we set the specific state transition.
             *
             *  NOTE: acceptable transitions after NSC_RF_DISCOVER_CMD are to RF_DISCOVERY or RF_POLL state.
             */
            if (ptx_NSC_State_Poll == disc_next_state)
            {
                nsc->State = disc_next_state;
            }

            nsc->LastDiscState = nsc->State;

            nsc->PollMode = poll_mode;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_ProcessRfDiscoverNtf(uint8_t *buffer, uint32_t bufferLen, ptx_NSC_RfDiscover_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != buffer) && (NULL != ntf) && (0 != bufferLen))
    {
        if (PTX_NSC_RF_DISCOVER_EMPTYNTF_LEN < bufferLen)
        {
            /* Not an empty NTF, contains ID and the payload. Start index at the payload offset. */
            uint8_t idx = 1U;

            /* Get RF-Tech ID. */
            ntf->RfTech = buffer[idx];
            idx++;

            if (ptx_NSC_RfTech_Passive_Poll_A == ntf->RfTech)
            {
                ntf->PollParam.PollA.SensRes[0] = buffer[idx];
                idx++;

                ntf->PollParam.PollA.SensRes[1] = buffer[idx];
                idx++;

                uint8_t nfc_id1_len = buffer[idx];
                idx++;

                /* Check for NFC-ID length. */
                if ((PTX_NSC_NFCID1_LEN_1 == nfc_id1_len) || (PTX_NSC_NFCID1_LEN_2 == nfc_id1_len) ||
                    (PTX_NSC_NFCID1_LEN_3 == nfc_id1_len) )
                {
                    /* NFC-ID length is 4, 7 or 10. */
                    ntf->PollParam.PollA.NfcId1Len = nfc_id1_len;

                    (void) memcpy(ntf->PollParam.PollA.NfcId1, &buffer[idx], nfc_id1_len);
                    idx = (uint8_t) (idx + nfc_id1_len);

                    ntf->PollParam.PollA.SelRes = buffer[idx];
                    idx++;

                    status = PTX_STATUS_SUCCESS;
                }
                else if (PTX_NSC_NFCID1_LEN_0 == nfc_id1_len)
                {
                    /* NFC-ID length is 0. No more data to extract. */
                    ntf->PollParam.PollA.NfcId1Len = nfc_id1_len;
                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    /* Invalid NFC ID length */
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }
            }
            else if (ptx_NSC_RfTech_Passive_Poll_B == ntf->RfTech)
            {
                uint8_t param_poll_b_size = PTX_NSC_RF_DISCOVER_SENSBRES_LEN;

                (void) memcpy(ntf->PollParam.PollB.SensBRes, &buffer[idx], param_poll_b_size);
                idx = (uint8_t) (idx + param_poll_b_size);

                status = PTX_STATUS_SUCCESS;
            }
            else if (ptx_NSC_RfTech_Passive_Poll_F == ntf->RfTech)
            {
                /*
                 * SENSF_RES length might be 18 or 20 By, it is not known in advance.
                 * So, we need to check the length parameter - whether it matches these values.
                 */
                uint8_t param_poll_f_size = buffer[idx];

                if ((PTX_NSC_RF_DISCOVER_SENSFRES_MAXLEN == param_poll_f_size) ||
                    (PTX_NSC_RF_DISCOVER_SENSFRES_MINLEN == param_poll_f_size))
                {
                    (void) memcpy(ntf->PollParam.PollF.SensFRes, &buffer[idx], param_poll_f_size);
                    idx = (uint8_t) (idx + param_poll_f_size);

                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    /* Invalid SENSF_RES length */
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }
            }
            else if (ptx_NSC_RfTech_Passive_Poll_V == ntf->RfTech)
            {
                uint8_t param_poll_v_size = PTX_NSC_RF_DISCOVER_INVENTORY_RES_LEN;
                (void) memcpy(ntf->PollParam.PollV.InventoryRes, &buffer[idx], param_poll_v_size);
                idx = (uint8_t) (idx + param_poll_v_size);

                status = PTX_STATUS_SUCCESS;
            }
            else if (ptx_NSC_RfTech_Active_Poll == ntf->RfTech)
            {
                /* There are no NFC-ACM notification parameters. */
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* The message seems to be fine up to this point. */
                ntf->RfState = (ptx_NSC_RfDiscover_Ntf_State_t) buffer[idx];
                idx++;

                ntf->CollPend = buffer[idx];
                idx++;

                /* Verify if received length matches amount of processed data. */
                if (idx != bufferLen)
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }
            }
        }
        else
        {
            /* Empty NTF, contains only ID. This is the last NTF. */
            ntf->IsLastNtf = 1;
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_GetRfDiscoveryParam(ptx_NSC_t *nsc, ptx_NSC_RfDiscover_Param_Type_t type, ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != cfg) && (NULL != params))
    {
        if (NSC_RfDiscover_Param_Type_Listen > type)
        {
            status = ptx_NSC_GetRfDiscoveryParam_Poll(nsc, type, cfg, params);
        }
        else
        {
#if !defined(PTX_DISABLE_LISTEN_MODE)
            status = ptx_NSC_GetRfDiscoveryParam_Listen(nsc, type, cfg, params);
#endif /* PTX_DISABLE_LISTEN_MODE */
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_RfDiscover_Dispatch(ptx_NSC_t *nsc, uint8_t *txBuffer, uint32_t txBufferLen)
{
    ptx_Status_t status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, txBuffer, txBufferLen);

    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t nsc_rsp[PTX_NSC_CMD_PROCESSOR_RFDISCOVER_RSPLEN];
        uint32_t nsc_rsp_len = PTX_NSC_CMD_PROCESSOR_RFDISCOVER_RSPLEN;

        status = ptx_NSC_CmdProcessor_FetchMsg(nsc, nsc_rsp, &nsc_rsp_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_RFDISCOVER_RSP);

        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 != nsc_rsp_len)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_RfDiscover, nsc_rsp, nsc_rsp_len, NULL);
            }
            else
            {
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }
        }
    }

    return status;
}
