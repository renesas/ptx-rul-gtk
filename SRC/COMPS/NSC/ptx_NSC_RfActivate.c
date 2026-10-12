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
    Module      : NFC Soft Controller (NSC) Interface for RF Activation
    File        : ptx_NSC_RfActivate.c

    Description : Implementation of NFC Soft Controller Interface for RF Activation.
                  Contains function(s) to start RF Activation and to gather the information
                  during the process.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Int.h"
#include "ptx_NSC_RfActivate.h"
#include "ptx_NSC_StateMachine.h"
#include "ptx_Status.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * Definitions for NSC_RF_ACTIVATE_CMD/RSP.
 */
#define PTX_NSC_RF_ACTIVATE_ATS_FSCI_MASK                   (0x0F)          /* NSC ATS FSCI */

#define PTX_NSC_RF_ACTIVATE_ATRREQ_BS_IDX                   (14u)           /* NSC ATR_REQ BS byte index */
#define PTX_NSC_RF_ACTIVATE_ATRREQ_BR_IDX                   (15u)           /* NSC ATR_REQ BR byte index */
#define PTX_NSC_RF_ACTIVATE_ATRREQ_PP_IDX                   (16u)           /* NSC ATR_REQ PP byte index */
#define PTX_NSC_RF_ACTIVATE_ATRREQ_BR_OFFSET                (4u)            /* NSC ATR_REQ BR offset for BRS */
#define PTX_NSC_RF_ACTIVATE_ATRREQ_BR_MASK                  (0xF0)          /* Mask for BR values in BRS byte*/
#define PTX_NSC_RF_ACTIVATE_ATRREQ_BS_MASK                  (0x0F)          /* Mask for BS values in BRS byte */

#define PTX_NSC_RF_ACTIVATE_SENSBRES_LEN                    (13U)           /* NSC Type B SENSRES length */
#define PTX_NSC_RF_ACTIVATE_SENSFRES_LEN_1                  (18U)           /* NSC Type F SENSRES NTF parameter length */
#define PTX_NSC_RF_ACTIVATE_SENSFRES_LEN_2                  (20U)           /* NSC Type F SENSRES NTF parameter length */

#define PTX_NSC_RF_ACTIVATE_INVENTORY_RES_LEN               (10U)           /* NSC Type V INVENTORY_RES length */

/* NSC RF Activate RSP maximum length is defined by protocol activation parameters size */
#define PTX_NSC_RF_ACTIVATE_RSP_MAXLEN                      (PTX_NSC_CMD_PROCESSOR_RFACTIVATE_RSPLEN + sizeof(ptx_NSC_RfAct_ProtParam_t))
#define PTX_NSC_RF_ACTIVATE_RSP_PAYLOAD_IDX                 2U

#define PTX_NSC_RF_ACTIVATE_RSP_ISODEPA_MINLEN              4U              /* Minimum length of RSP with NFC-A ISO-DEP parameters */
#define PTX_NSC_RF_ACTIVATE_RSP_ISODEPA_MAXLEN              24U             /* Maximum length of RSP with NFC-A ISO-DEP parameters */

#define PTX_NSC_RF_ACTIVATE_RSP_ISODEPB_MINLEN              6U              /* Minimum length of RSP with NFC-B ISO-DEP parameters */
#define PTX_NSC_RF_ACTIVATE_RSP_ISODEPB_MAXLEN              21U             /* Maximum length of RSP with NFC-B ISO-DEP parameters */

#define PTX_NSC_RF_ACTIVATE_RSP_NFCDEP_MINLEN               21U             /* Minimum length of RSP with NDC-DEP parameters */
#define PTX_NSC_RF_ACTIVATE_RSP_NFCDEP_MAXLEN               68U             /* Maximum length of RSP with NDC-DEP parameters */

/*  */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_RfActivateNtf_CheckProtLen (uint8_t *buffer, uint32_t bufferLen, uint8_t curIdx, ptx_NSC_RfActivate_Ntf_t *ntf);
static ptx_Status_t ptx_NSC_RfActivateNtf_ParseProtParams_IsoDep (uint8_t *buffer, uint32_t bufferLen, uint8_t *curIdx, ptx_NSC_RfActivate_Ntf_t *ntf);
static ptx_Status_t ptx_NSC_RfActivateNtf_ParseProtParams_NfcDep (uint8_t *buffer, uint32_t bufferLen, uint8_t *curIdx, ptx_NSC_RfActivate_Ntf_t *ntf);
static ptx_Status_t ptx_NSC_RfActivateNtf_ParseProtParams (uint8_t *buffer, uint32_t bufferLen, uint8_t *curIdx, ptx_NSC_RfActivate_Ntf_t *ntf);
static ptx_Status_t ptx_NSC_RfActivateNtf_ParseTechParams (uint8_t *buffer, uint8_t *curIdx, ptx_NSC_RfActivate_Ntf_t *ntf);

static ptx_Status_t ptx_NSC_RfActivate_ParseRsp(uint8_t *buffer, uint32_t bufferLen, uint8_t tech, ptx_NSC_RfProtocol_t RfProt, ptx_NSC_RfAct_ProtParam_t *data);
static ptx_Status_t ptx_NSC_RfActivate_ParseRsp_IsoDepA (uint8_t *buffer, uint32_t bufferLen, ptx_NSC_RfAct_ProtParam_t *data);
static ptx_Status_t ptx_NSC_RfActivate_ParseRsp_IsoDepB (uint8_t *buffer, uint32_t bufferLen, ptx_NSC_RfAct_ProtParam_t *data);
static ptx_Status_t ptx_NSC_RfActivate_ParseRsp_NfcDep (uint8_t *buffer, uint32_t bufferLen, ptx_NSC_RfAct_ProtParam_t *data);

static ptx_Status_t ptx_NSC_RfActivate_Process(ptx_NSC_t *nsc, ptx_NSC_RfAct_Param_t *param, ptx_NSC_RfAct_ProtParam_t *data);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_RfActivate(ptx_NSC_t *nsc, ptx_NSC_RfAct_Param_t *param, ptx_NSC_RfAct_ProtParam_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != param) && (NULL != data))
    {
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_RfActivate, NULL);
    }

    if ( PTX_STATUS_SUCCESS == status )
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_RfActivate");

        /* Special Case: For T2T, always activate from RF_STATE SLEEP, this is required
         * to handle Multi-card issues for constellations containing T2T and T5T cards. In these scenarios, the FW 
         * reports the T2T as active even though it is deactivated. An activation from the RF_STATE ACTIVE will 
         * result in a deactivated T2T card without any NSC error. To solve this, the T2T needs to be 
         * activated from scratch by forcing the RF_STATE to SLEEP.
         */
        if((param->RfTech == ptx_NSC_RfTech_Passive_Poll_A) && (param->RfProt == ptx_NSC_RfProtocol_T2T))
        {
            param->RfTechParams.RfAct_A.RfState = ptx_NSC_RfAct_PollMode_State_A_Sleep;
        }

        status = ptx_NSC_RfActivate_Process(nsc, param, data);

        /* Special Case: If the activation fails, always activate once again from RF_STATE SLEEP, this is required
         * to handle Multi-card issues for multiple constellations. In these scenarios the FW reports active 
         * cards even though they are deactivated. An activation from the RF_STATE ACTIVE will 
         * result in a NSC Error. To solve this, the cards need to be 
         * activated from scratch by forcing the RF_STATE to SLEEP.
         */
        if(PTX_STATUS_SUCCESS != status)
        {
            status = PTX_STATUS_SUCCESS;

            switch (param->RfTech)
            {
                case ptx_NSC_RfTech_Passive_Poll_A:
                {
                    param->RfTechParams.RfAct_A.RfState = ptx_NSC_RfAct_PollMode_State_A_Sleep;
                    break;
                }
                case ptx_NSC_RfTech_Passive_Poll_B:
                {
                    param->RfTechParams.RfAct_B.RfState = ptx_NSC_RfAct_PollMode_State_B_Sleep;
                    break;
                }
                case ptx_NSC_RfTech_Passive_Poll_F:
                {
                    param->RfTechParams.RfAct_F.RfState = ptx_NSC_RfAct_PollMode_State_F_Sleep_Af;
                    break;
                }
                case ptx_NSC_RfTech_Passive_Poll_V:
                {
                    /* No special adjustments required for Type V. */
                    break;
                }
                default:
                {
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
                }
            }

            if(PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_RfActivate_Process(nsc, param, data);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_ProcessRfActivateNtf (uint8_t *buffer, uint32_t bufferLen, ptx_NSC_RfActivate_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != buffer) && (0 != bufferLen) && (NULL != ntf) )
    {

        uint8_t idx = 0;

        /* Get RF-Tech ID. */
        ntf->RfTech = buffer[idx];
        idx++;
        status = ptx_NSC_RfActivateNtf_ParseTechParams (buffer, &idx, ntf);

        /* RF Protocol ID */
        if (PTX_STATUS_SUCCESS == status)
        {
            ntf->RfProt = (ptx_NSC_RfProtocol_t) buffer[idx];
            idx++;
            status = ptx_NSC_RfActivateNtf_ParseProtParams (buffer, bufferLen, &idx, ntf);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Get Routing-Action (Listen-Mode only!). */
            switch (ntf->RfTech)
            {
                case ptx_NSC_RfTech_Passive_Listen_A:
                /* Fall through. */
                case ptx_NSC_RfTech_Passive_Listen_B:
                /* Fall through. */
                case ptx_NSC_RfTech_Passive_Listen_F:
                /* Fall through */
                case ptx_NSC_RfTech_Active_Listen:
                    ntf->RfRoutingAction = buffer[idx];
                    idx++;
                    break;

                default:
                    /* Poll-Mode - ignore */
                    break;
            }
        }

        /* Verify if received length matches amount of processed data. */
        if (PTX_STATUS_SUCCESS == status)
        {
            if (bufferLen != idx)
            {
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_RfActivate_Process(ptx_NSC_t *nsc, ptx_NSC_RfAct_Param_t *param, ptx_NSC_RfAct_ProtParam_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* For IsoDep and NfcDep protocols, there shall be NSC state transition from RF_POLL to RF_POLL_DEP.
     * Use this variable to track pending state and assign new state at the end if all went well.
     */
    ptx_NSC_State_t pending_state = ptx_NSC_State_Undefined;

    /* Allocate buffer for the RF-Activate command. */
    uint8_t nsc_buf[PTX_NSC_RF_ACTIVATE_BUFFER_SIZE];
    uint8_t *nsc_buf_pos = nsc_buf;

    /* Append RF-Activate command opcode. */
    *nsc_buf_pos = PTX_NSC_RF_ACTIVATE_CMD_OPCODE;
    nsc_buf_pos++;

    /* Acceptable RF technologies and mode are NFC-A/B/F/V poll mode. Others are interpreted as INVALID_PARAMETER. */
    *nsc_buf_pos = param->RfTech;
    nsc_buf_pos++;

    switch (param->RfTech)
    {
        /* Add RF technology specific parameters. */
        case ptx_NSC_RfTech_Passive_Poll_A:
        {
            *nsc_buf_pos = param->RfTechParams.RfAct_A.RfState;
            nsc_buf_pos++;

            uint8_t nfc_id1_len = param->RfTechParams.RfAct_A.NfcId1Len;

            if (PTX_NSC_NFCID1_LEN_3 >= nfc_id1_len)
            {
                *nsc_buf_pos = nfc_id1_len;
                nsc_buf_pos++;

                (void) memcpy(nsc_buf_pos, param->RfTechParams.RfAct_A.NfcId1, nfc_id1_len);
                nsc_buf_pos += nfc_id1_len;
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }

            break;
        }

        case ptx_NSC_RfTech_Passive_Poll_B:
        {
            *nsc_buf_pos = param->RfTechParams.RfAct_B.RfState;
            nsc_buf_pos++;

            const uint8_t rf_tech_params_len = PTX_NSC_RF_DISCOVER_SENSBRES_LEN;
            (void) memcpy(nsc_buf_pos, param->RfTechParams.RfAct_B.SensBRes, rf_tech_params_len);
            nsc_buf_pos += rf_tech_params_len;
            status = PTX_STATUS_SUCCESS;

            break;
        }

        case ptx_NSC_RfTech_Passive_Poll_F:
        {
            *nsc_buf_pos = (uint8_t)param->RfTechParams.RfAct_F.BitRate;
            nsc_buf_pos++;

            *nsc_buf_pos = param->RfTechParams.RfAct_F.RfState;
            nsc_buf_pos++;

            const uint8_t rf_tech_params_len = PTX_NSC_NFCID2_LEN;
            (void) memcpy(nsc_buf_pos, param->RfTechParams.RfAct_F.NfcId2, rf_tech_params_len);
            nsc_buf_pos += rf_tech_params_len;
            status = PTX_STATUS_SUCCESS;

            break;
        }

        case ptx_NSC_RfTech_Passive_Poll_V:
        {
            /* No special parameters required for Type V. */
            status = PTX_STATUS_SUCCESS;
            break;
        }

        default:
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            break;
        }
    }

    if ( PTX_STATUS_SUCCESS == status )
    {
        *nsc_buf_pos = param->RfProt;
        nsc_buf_pos++;

        switch (param->RfProt)
        {
            case ptx_NSC_RfProtocol_IsoDep:
            {
                *nsc_buf_pos = param->RfProtParams.IsoDep.HbrEnable;
                nsc_buf_pos++;

                uint8_t attrib_inf_len = param->RfProtParams.IsoDep.AttribInfLen;

                if (PTX_NSC_RFACTIVATE_ATTRIBINF_LEN >= attrib_inf_len)
                {
                    *nsc_buf_pos = attrib_inf_len;
                    nsc_buf_pos++;

                    (void) memcpy(nsc_buf_pos, param->RfProtParams.IsoDep.AttribInf, attrib_inf_len);
                    nsc_buf_pos += attrib_inf_len;
                }
                else
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }

                pending_state = ptx_NSC_State_Poll_Dep;
                break;
            }

            case ptx_NSC_RfProtocol_NfcDep:
            {
                *nsc_buf_pos = param->RfProtParams.NfcDep.HbrEnable;
                nsc_buf_pos++;

                uint8_t general_bytes_len = param->RfProtParams.NfcDep.GeneralBytesLen;

                if (PTX_NSC_RFACTIVATE_GENBYTES_LEN >= general_bytes_len)
                {
                    *nsc_buf_pos = general_bytes_len;
                    nsc_buf_pos++;

                    (void) memcpy(nsc_buf_pos, param->RfProtParams.NfcDep.GeneralBytes, general_bytes_len);
                    nsc_buf_pos += general_bytes_len;
                }
                else
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }

                pending_state = ptx_NSC_State_Poll_Dep;
                break;
            }

            default:
            {
                /* The RF_PROTOCOL specific parameters shall be omitted in case of a T2T, T3T, T5T or an undefined protocol. */
                break;
            }
        }
    }

    if ( PTX_STATUS_SUCCESS == status )
    {
        /* Valid parameters provided. Send the command. */
        status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, nsc_buf, (uint32_t)(nsc_buf_pos - nsc_buf));
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t nsc_rsp[PTX_NSC_RF_ACTIVATE_RSP_MAXLEN];
        uint32_t nsc_rsp_len = PTX_NSC_RF_ACTIVATE_RSP_MAXLEN;

        status = ptx_NSC_CmdProcessor_FetchMsg(nsc, nsc_rsp, &nsc_rsp_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_RFACTIVATE_RSP);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_RfActivate, nsc_rsp, nsc_rsp_len, NULL);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* RF Activate RSP successfully received. Check protocol specific parameters, if any. */
            status = ptx_NSC_RfActivate_ParseRsp(nsc_rsp, nsc_rsp_len, param->RfTech, param->RfProt, data);
        }

        if ((PTX_STATUS_SUCCESS == status) && (ptx_NSC_State_Poll_Dep == pending_state) &&
            (ptx_NSC_State_Poll == nsc->State))
        {
            /* State changed from  RF_POLL to RF_POLL_DEP. */
            nsc->State = ptx_NSC_State_Poll_Dep;
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_RfActivate_ParseRsp_NfcDep (uint8_t *buffer, uint32_t bufferLen, ptx_NSC_RfAct_ProtParam_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_NSC_PROTOCOL;

    if ( (NULL != buffer) && (0 != bufferLen) && (NULL != data) )
    {
        const uint8_t brs_size = 1;
        uint8_t atr_res_len = 0;

        if ((PTX_NSC_RF_ACTIVATE_RSP_NFCDEP_MINLEN <= bufferLen) &&
            (PTX_NSC_RF_ACTIVATE_RSP_NFCDEP_MAXLEN >= bufferLen))
        {
            uint32_t res_len = PTX_NSC_CMD_PROCESSOR_RFACTIVATE_RSPLEN + buffer[PTX_NSC_RF_ACTIVATE_RSP_PAYLOAD_IDX] + brs_size;
            if (bufferLen == res_len)
            {
                atr_res_len = buffer[PTX_NSC_RF_ACTIVATE_RSP_PAYLOAD_IDX];
                status = PTX_STATUS_SUCCESS;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t idx = PTX_NSC_RF_ACTIVATE_RSP_PAYLOAD_IDX;
            memcpy(&data->NfcDep.PollAF.AtrRes[0], &buffer[idx], atr_res_len);
            idx = (uint8_t)(idx + atr_res_len);
            data->NfcDep.PollAF.Brs = buffer[idx];
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_RfActivate_ParseRsp_IsoDepA (uint8_t *buffer, uint32_t bufferLen, ptx_NSC_RfAct_ProtParam_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_NSC_PROTOCOL;

    if ( (NULL != buffer) && (0 != bufferLen) && (NULL != data) )
    {
        const uint8_t pps1_size = 1;

        if ((PTX_NSC_RF_ACTIVATE_RSP_ISODEPA_MINLEN <= bufferLen) &&
            (PTX_NSC_RF_ACTIVATE_RSP_ISODEPA_MAXLEN >= bufferLen))
        {
            uint32_t res_len = PTX_NSC_CMD_PROCESSOR_RFACTIVATE_RSPLEN + buffer[PTX_NSC_RF_ACTIVATE_RSP_PAYLOAD_IDX] + pps1_size;
            if (bufferLen == res_len)
            {
                status = PTX_STATUS_SUCCESS;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t ats_len = buffer[PTX_NSC_RF_ACTIVATE_RSP_PAYLOAD_IDX];

            if (PTX_NSC_RFACTIVATE_ATS_MIN_LEN == ats_len)
            {
                uint32_t idx = PTX_NSC_RF_ACTIVATE_RSP_PAYLOAD_IDX + ats_len;
                /* Set structure members to a defined state. */
                (void)memset(data->IsoDep.PollA.ATS, 0, PTX_NSC_RFACTIVATE_POLLA_ATS_LEN);
                data->IsoDep.PollA.PPS1 = buffer[idx];

                /* 'Empty' ATS, use default values. */
                data->IsoDep.PollA.FSCI = PTX_NSC_RFACTIVATE_ATS_FSCI_DEFAULT;
            }
            else if ( (PTX_NSC_RFACTIVATE_ATS_MIN_LEN < ats_len)
                    && (PTX_NSC_RFACTIVATE_POLLA_ATS_LEN >= ats_len) )
            {
                uint32_t idx = PTX_NSC_RF_ACTIVATE_RSP_PAYLOAD_IDX;

                /* Copy data into structure members. */
                (void)memcpy(data->IsoDep.PollA.ATS, &buffer[idx], ats_len);
                idx = (uint8_t)(idx + ats_len);

                data->IsoDep.PollA.PPS1 = buffer[idx];
                idx++;

                data->IsoDep.PollA.FSCI = data->IsoDep.PollA.ATS[PTX_NSC_RFACTIVATE_ATS_T0_IDX] & PTX_NSC_RF_ACTIVATE_ATS_FSCI_MASK;
            }
            else
            {
                /* Wrong format */
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }
        }
    }
    return status;
}

static ptx_Status_t ptx_NSC_RfActivate_ParseRsp_IsoDepB (uint8_t *buffer, uint32_t bufferLen, ptx_NSC_RfAct_ProtParam_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_NSC_PROTOCOL;

    if ( (NULL != buffer) && (0 != bufferLen) && (NULL != data) )
    {
        /* Header (2By) + ATTRIB_PARAM1 (1By) + ATTRIB_PARAM2 (1By) + ATTRIB_RES_LEN (1By) */
        const uint8_t isodep_b_minlen = PTX_NSC_CMD_PROCESSOR_RFACTIVATE_RSPLEN + 3;
        const uint8_t attrib_res_len_idx = 4;

        uint8_t attrib_res_len = 0;

        if ((PTX_NSC_RF_ACTIVATE_RSP_ISODEPB_MINLEN <= bufferLen) &&
            (PTX_NSC_RF_ACTIVATE_RSP_ISODEPB_MAXLEN >= bufferLen))
        {
            /* Index of ATTRIB_RES_LENGTH */
            attrib_res_len = buffer[attrib_res_len_idx];
            if (((uint32_t)(isodep_b_minlen + attrib_res_len) == bufferLen) &&
                (PTX_NSC_RFACTIVATE_POLLB_ATTRIBRES_LEN >= attrib_res_len))
            {
                status = PTX_STATUS_SUCCESS;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint32_t idx = PTX_NSC_RF_ACTIVATE_RSP_PAYLOAD_IDX;
            data->IsoDep.PollB.AttribParam1 = buffer[idx];
            idx++;
            data->IsoDep.PollB.AttribParam2 = buffer[idx];
            idx++;
            data->IsoDep.PollB.AttribResLen = attrib_res_len;
            idx++;
            (void)memcpy(&data->IsoDep.PollB.AttribRes[0], &buffer[idx], attrib_res_len);
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_RfActivate_ParseRsp(uint8_t *buffer, uint32_t bufferLen, uint8_t tech, ptx_NSC_RfProtocol_t RfProt, ptx_NSC_RfAct_ProtParam_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_NSC_PROTOCOL;

    if ( (NULL != buffer) && (0 != bufferLen) && (NULL != data) )
    {

        switch (RfProt)
        {
            case ptx_NSC_RfProtocol_IsoDep:
                if (ptx_NSC_RfTech_Passive_Poll_A == tech)
                {
                    status = ptx_NSC_RfActivate_ParseRsp_IsoDepA(buffer, bufferLen, data);
                }
                else if (ptx_NSC_RfTech_Passive_Poll_B == tech)
                {
                    status = ptx_NSC_RfActivate_ParseRsp_IsoDepB(buffer, bufferLen, data);
                }
                else
                {
                    /* Wrong technology. Error already set. */
                }
                break;
            case ptx_NSC_RfProtocol_NfcDep:
                status = ptx_NSC_RfActivate_ParseRsp_NfcDep(buffer, bufferLen, data);
                break;
            default:
                /* For all other protocols, protocol specific parameters shall be omitted. */
                if (PTX_NSC_CMD_PROCESSOR_RFACTIVATE_RSPLEN == bufferLen)
                {
                    status = PTX_STATUS_SUCCESS;
                }
                break;
        }
    }
    return status;
}

static ptx_Status_t ptx_NSC_RfActivateNtf_CheckProtLen (uint8_t *buffer, uint32_t bufferLen, uint8_t curIdx, ptx_NSC_RfActivate_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_NSC_PROTOCOL;

    if ( (NULL != buffer) && (0 != bufferLen) && (NULL != ntf) )
    {
        uint8_t idx = curIdx;
        uint32_t total_len = curIdx;
        if (ptx_NSC_RfProtocol_IsoDep == ntf->RfProt)
        {
            if (ptx_NSC_RfTech_Passive_Poll_A == ntf->RfTech)
            {
                uint8_t ats_len = buffer[idx];
                /* ATS len */
                total_len += ats_len;
                /* PPS1 */
                total_len += 1;
                if (total_len <= bufferLen)
                {
                    status = PTX_STATUS_SUCCESS;
                }
            }
            else if (ptx_NSC_RfTech_Passive_Poll_B == ntf->RfTech)
            {
                uint8_t attrib_res_len = 0;
                /* Count in ATTRIB_PARAM1 (1By) + ATTRIB_PARAM2 (1By) + ATTRIB_RES_LEN (1By) */
                total_len += 3u;
                if (total_len <= bufferLen)
                {
                    /* Index of ATTRIB_RES_LENGTH */
                    attrib_res_len = buffer[idx + 2];
                    total_len += attrib_res_len;
                    if (total_len <= bufferLen)
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                }
            }
            else if ((ptx_NSC_RfTech_Passive_Listen_A == ntf->RfTech) || (ptx_NSC_RfTech_Passive_Listen_B == ntf->RfTech))
            {
                if (total_len <= bufferLen)
                {
                    status = PTX_STATUS_SUCCESS;
                }
            }
            else
            {
                /* No action required: protocol error. */
            }
        }
        else if (ptx_NSC_RfProtocol_NfcDep == ntf->RfProt)
        {
            if ( (ptx_NSC_RfTech_Passive_Poll_A == ntf->RfTech) ||
                (ptx_NSC_RfTech_Passive_Poll_F == ntf->RfTech) ||
                (ptx_NSC_RfTech_Active_Poll == ntf->RfTech) )
            {
                uint8_t atr_res_len = buffer[idx];
                /* ATR_RES len */
                total_len += atr_res_len;
                /* BRS */
                total_len += 1;
                if (total_len <= bufferLen)
                {
                    status = PTX_STATUS_SUCCESS;
                }
            }
            else if ((ptx_NSC_RfTech_Passive_Listen_A == ntf->RfTech) ||
                    (ptx_NSC_RfTech_Passive_Listen_F == ntf->RfTech) ||
                    (ptx_NSC_RfTech_Active_Listen == ntf->RfTech) )
            {
                uint8_t atr_req_len = buffer[idx];
                /* ATR_REQ len */
                total_len += atr_req_len;
                /* BRS and LR */
                total_len += 2;
                if (total_len <= bufferLen)
                {
                    status = PTX_STATUS_SUCCESS;
                }
            }
            else
            {
                /* No action required: protocol error. */
            }
        }
        else
        {
            /* Allow any length, protocol bytes will not be checked. Propagate further. */
            status = PTX_STATUS_SUCCESS;
        }
    }
    return status;
}

static ptx_Status_t ptx_NSC_RfActivateNtf_ParseProtParams_IsoDep (uint8_t *buffer, uint32_t bufferLen, uint8_t *curIdx, ptx_NSC_RfActivate_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_NSC_PROTOCOL;

    if ((NULL != buffer) && (0 != bufferLen)&& (0 != curIdx) && (NULL != ntf) )
    {
        uint8_t idx = *curIdx;

        status = ptx_NSC_RfActivateNtf_CheckProtLen (buffer, bufferLen, idx, ntf);
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Parsing is not fully completed. Set status error until the message gets completely parsed. */
            status = PTX_STATUS_E_NSC_PROTOCOL;

            switch (ntf->RfTech)
            {
                case ptx_NSC_RfTech_Passive_Poll_A:
                {
                    uint8_t ats_len = buffer[idx];

                    if (PTX_NSC_RFACTIVATE_ATS_MIN_LEN == ats_len)
                    {
                        /* Set structure members to a defined state. */
                        (void)memset(ntf->RfProtParams.IsoDep.PollA.ATS, 0, PTX_NSC_RFACTIVATE_POLLA_ATS_LEN);

                        idx = (uint8_t)(idx + ats_len);

                        ntf->RfProtParams.IsoDep.PollA.PPS1 = buffer[idx];
                        idx++;

                        /* 'Empty' ATS, use default values. */
                        ntf->RfProtParams.IsoDep.PollA.FSCI = PTX_NSC_RFACTIVATE_ATS_FSCI_DEFAULT;

                        status = PTX_STATUS_SUCCESS;
                    }
                    else if ( (PTX_NSC_RFACTIVATE_ATS_MIN_LEN < ats_len)
                            && (PTX_NSC_RFACTIVATE_POLLA_ATS_LEN >= ats_len) )
                    {
                        /* Copy data into structure members. */
                        (void)memcpy(ntf->RfProtParams.IsoDep.PollA.ATS, &buffer[idx], ats_len);
                        idx = (uint8_t)(idx + ats_len);

                        ntf->RfProtParams.IsoDep.PollA.PPS1 = buffer[idx];
                        idx++;

                        ntf->RfProtParams.IsoDep.PollA.FSCI = ntf->RfProtParams.IsoDep.PollA.ATS[PTX_NSC_RFACTIVATE_ATS_T0_IDX] & PTX_NSC_RF_ACTIVATE_ATS_FSCI_MASK;

                        status = PTX_STATUS_SUCCESS;
                    }
                    else
                    {
                        /* Unexpected length. Protocol error already set. */
                    }

                    break;
                }

                case ptx_NSC_RfTech_Passive_Poll_B:
                {
                    ntf->RfProtParams.IsoDep.PollB.AttribParam1 = buffer[idx];
                    idx++;

                    ntf->RfProtParams.IsoDep.PollB.AttribParam2 = buffer[idx];
                    idx++;

                    uint8_t attrib_res_len = buffer[idx];
                    idx++;

                    if ((0 < attrib_res_len) && (PTX_NSC_RFACTIVATE_POLLB_ATTRIBRES_LEN >= attrib_res_len))
                    {
                        ntf->RfProtParams.IsoDep.PollB.AttribResLen = attrib_res_len;

                        (void)memcpy(ntf->RfProtParams.IsoDep.PollB.AttribRes, &buffer[idx], attrib_res_len);
                        idx = (uint8_t)(idx + attrib_res_len);

                        status = PTX_STATUS_SUCCESS;
                    }

                    break;
                }

                case ptx_NSC_RfTech_Passive_Listen_A:
                {
                    ntf->RfProtParams.IsoDep.ListenA.Rats = buffer[idx];
                    idx++;

                    ntf->RfProtParams.IsoDep.ListenA.Pps1 = buffer[idx];
                    idx++;

                    status = PTX_STATUS_SUCCESS;
                    break;
                }

                case ptx_NSC_RfTech_Passive_Listen_B:
                {
                    uint8_t attrib_len = buffer[idx];
                    idx++;

                    if ((0 < attrib_len) && (PTX_NSC_RFACTIVATE_LISTENB_ATTRIB_LEN >= attrib_len))
                    {
                        ntf->RfProtParams.IsoDep.ListenB.AttribLen = attrib_len;

                        (void)memcpy(ntf->RfProtParams.IsoDep.ListenB.Attrib, &buffer[idx], attrib_len);
                        idx = idx + attrib_len;
                        status = PTX_STATUS_SUCCESS;
                    }

                    break;
                }

                default:
                    /* Undefined. Protocol error already set.*/
                    break;
            }
        }

        *curIdx = idx;
    }
    return status;
}

static ptx_Status_t ptx_NSC_RfActivateNtf_ParseProtParams_NfcDep (uint8_t *buffer, uint32_t bufferLen, uint8_t *curIdx, ptx_NSC_RfActivate_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_NSC_PROTOCOL;

    if ((NULL != buffer) && (0 != bufferLen)&& (0 != curIdx) && (NULL != ntf) )
    {
        uint8_t idx = *curIdx;

        status = ptx_NSC_RfActivateNtf_CheckProtLen (buffer, bufferLen, idx, ntf);
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Parsing is not fully completed. Set status error until the message gets completely parsed. */
            status = PTX_STATUS_E_NSC_PROTOCOL;

            switch (ntf->RfTech)
            {
                case ptx_NSC_RfTech_Passive_Poll_A:
                /* FALL THROUGH */
                case ptx_NSC_RfTech_Passive_Poll_F:
                /* FALL THROUGH */
                case ptx_NSC_RfTech_Active_Poll:
                {
                    uint8_t atr_res_len = buffer[idx];

                    if (PTX_NSC_RFACTIVATE_ATRRES_LEN >= atr_res_len)
                    {
                        (void)memcpy(ntf->RfProtParams.NfcDep.PollAF.AtrRes, &buffer[idx], atr_res_len);
                        idx = (uint8_t)(idx + atr_res_len);

                        ntf->RfProtParams.NfcDep.PollAF.Brs = buffer[idx];
                        idx++;

                        status = PTX_STATUS_SUCCESS;

                    }

                    break;
                }

                case ptx_NSC_RfTech_Passive_Listen_A:
                /* FALL THROUGH */
                case ptx_NSC_RfTech_Passive_Listen_F:
                /* FALL THROUGH */
                case ptx_NSC_RfTech_Active_Listen:
                {
                    uint8_t atr_req_len = buffer[idx];

                    if (PTX_NSC_RFACTIVATE_ATRREQ_LEN >= atr_req_len)
                    {
                        (void)memcpy(ntf->RfProtParams.NfcDep.ListenAF.AtrReq, &buffer[idx], atr_req_len);
                        idx = (uint8_t)(idx + (atr_req_len));

                        ntf->RfProtParams.NfcDep.ListenAF.Brs = buffer[idx];
                        idx++;

                        ntf->RfProtParams.NfcDep.ListenAF.Lr = buffer[idx];
                        idx++;

                        if (0xFF == ntf->RfProtParams.NfcDep.ListenAF.Lr)
                        {
                            ntf->RfProtParams.NfcDep.ListenAF.Lr = ntf->RfProtParams.NfcDep.ListenAF.AtrReq[PTX_NSC_RF_ACTIVATE_ATRREQ_PP_IDX];
                        }

                        status = PTX_STATUS_SUCCESS;

                    }

                    break;
                }

                default:
                    /* Undefined. Protocol error already set.*/
                    break;
            }
        }

        *curIdx = idx;
    }

    return status;
}

static ptx_Status_t ptx_NSC_RfActivateNtf_ParseProtParams (uint8_t *buffer, uint32_t bufferLen, uint8_t *curIdx, ptx_NSC_RfActivate_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_NSC_PROTOCOL;

    if ((NULL != buffer) && (0 != bufferLen)&& (0 != curIdx) && (NULL != ntf) )
    {
        uint8_t idx = *curIdx;

        switch (ntf->RfProt)
        {
            case ptx_NSC_RfProtocol_IsoDep:
                status = ptx_NSC_RfActivateNtf_ParseProtParams_IsoDep(buffer, bufferLen, &idx, ntf);
                break;

            case ptx_NSC_RfProtocol_NfcDep:
                status = ptx_NSC_RfActivateNtf_ParseProtParams_NfcDep(buffer, bufferLen, &idx, ntf);
                break;

            default:
                /* Undefined. Propagate further.*/
                status = PTX_STATUS_SUCCESS;
                break;
        }

        *curIdx = idx;
    }

    return status;
}

static ptx_Status_t ptx_NSC_RfActivateNtf_ParseTechParams (uint8_t *buffer, uint8_t *curIdx, ptx_NSC_RfActivate_Ntf_t *ntf)
{
    ptx_Status_t status = PTX_STATUS_E_NSC_PROTOCOL;

    if ((NULL != buffer) && (0 != curIdx) && (NULL != ntf) )
    {        
        uint8_t idx = *curIdx;

        switch (ntf->RfTech)
        {
            case ptx_NSC_RfTech_Passive_Poll_A:
            {
                ntf->RfTechParams.PollA.SensRes[0] = buffer[idx];
                idx++;

                ntf->RfTechParams.PollA.SensRes[1] = buffer[idx];
                idx++;

                uint8_t nfc_id1_len = buffer[idx];
                idx++;

                /* Check for NFC-ID length. Must be 4, 7 or 10. */
                if ( (PTX_NSC_NFCID1_LEN_1 == nfc_id1_len) || (PTX_NSC_NFCID1_LEN_2 == nfc_id1_len) ||
                    (PTX_NSC_NFCID1_LEN_3 == nfc_id1_len) )
                {

                    ntf->RfTechParams.PollA.NfcId1Len = nfc_id1_len;
                    (void) memcpy(ntf->RfTechParams.PollA.NfcId1, &buffer[idx], nfc_id1_len);
                    idx = (uint8_t) (idx + nfc_id1_len);

                    ntf->RfTechParams.PollA.SelRes = buffer[idx];
                    idx++;

                    status = PTX_STATUS_SUCCESS;
                }
                break;
            }

            case ptx_NSC_RfTech_Passive_Poll_B:
            {
                uint8_t param_poll_b_size = PTX_NSC_RF_ACTIVATE_SENSBRES_LEN;

                (void) memcpy(ntf->RfTechParams.PollB.SensBRes, &buffer[idx], param_poll_b_size);
                idx = (uint8_t) (idx + param_poll_b_size);

                status = PTX_STATUS_SUCCESS;
                break;
            }

            case ptx_NSC_RfTech_Passive_Poll_F:
            {
                uint8_t sensf_len = buffer[idx];
                if ((PTX_NSC_RF_ACTIVATE_SENSFRES_LEN_1 == sensf_len) || (PTX_NSC_RF_ACTIVATE_SENSFRES_LEN_2 == sensf_len))
                {
                    /* NTF poll F parameters include also the length byte i.e. we get LEN (1By) + SENSF_RES (17 or 19 By). Copy all. */
                    (void) memcpy(ntf->RfTechParams.PollF.SensFRes, &buffer[idx], sensf_len);
                    idx = (uint8_t) (idx + sensf_len);

                    status = PTX_STATUS_SUCCESS;
                }

                break;
            }

            case ptx_NSC_RfTech_Passive_Poll_V:
            {
                uint8_t param_poll_v_size = PTX_NSC_RF_ACTIVATE_INVENTORY_RES_LEN;

                (void) memcpy(ntf->RfTechParams.PollV.InventoryRes, &buffer[idx], param_poll_v_size);
                idx = (uint8_t) (idx + param_poll_v_size);

                status = PTX_STATUS_SUCCESS;
                break;
            }

            case ptx_NSC_RfTech_Active_Poll:
            /* FALL THROUGH */
            case ptx_NSC_RfTech_Active_Listen:
            /* FALL THROUGH */
            case ptx_NSC_RfTech_Passive_Listen_A:
            /* FALL THROUGH */
            case ptx_NSC_RfTech_Passive_Listen_B:
            /* FALL THROUGH */
            case ptx_NSC_RfTech_Passive_Listen_F:
                /* No Parameters */
                status = PTX_STATUS_SUCCESS;
                break;

            default:
                /* Protocol error. Undefined technology. */
                break;
        }

        *curIdx = idx;
    }
    return status;
}
