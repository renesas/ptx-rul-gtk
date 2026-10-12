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
    Module      : NFC Soft Controller (NSC) Interface for Wireless Charging (WLC)
    File        : ptx_NSC_Wlc.c

    Description : Implementation of NFC Soft Controller Interface for Wireless Charging.
                  Contains the function(s) to start and stop charging and to gather
                  information during the charging process.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Wlc.h"
#include "ptx_NSC_StateMachine.h"


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

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_WLC_WptStart(ptx_NSC_t *nsc, ptx_NSC_WLC_WptOptions_t* options)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != options))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_WLC_WptStart");

        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_WptStart, NULL);
    }

    if ( PTX_STATUS_SUCCESS == status )
    {
        /*
         * Assemble WPT_START_CMD.
         */

        /* Allocate buffer for the command. */
        uint8_t nsc_buf[PTX_NSC_WLC_WPT_START_CMD_LEN];
        uint8_t *nsc_buf_pos = nsc_buf;

        /* Append WPT start command opcode. */
        *nsc_buf_pos = PTX_NSC_WLC_WPT_START_CMD_OPCODE;
        nsc_buf_pos++;

        /* Append amplitude value used for the upcoming WPT cycle. */
        *nsc_buf_pos = options->WptAmp;
        nsc_buf_pos++;

        /* Append amplitude value used for the communication phase. */
        *nsc_buf_pos = options->NfcAmp;
        nsc_buf_pos++;

        /* Append WPT duration. */
        *nsc_buf_pos = (uint8_t) ((options->WptDur >> 16) & 0xFF);
        nsc_buf_pos++;

        *nsc_buf_pos = (uint8_t) ((options->WptDur >> 8) & 0xFF);
        nsc_buf_pos++;

        *nsc_buf_pos = (uint8_t) ((options->WptDur >> 0) & 0xFF);
        nsc_buf_pos++;

        /* Append information if impedance change detection shall be enabled. */
        *nsc_buf_pos = options->IcdEn;
        nsc_buf_pos++;

        /* Append information of ICD guard time. */
        *nsc_buf_pos = options->IcdGt;
        nsc_buf_pos++;

        status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, nsc_buf, PTX_NSC_WLC_WPT_START_CMD_LEN);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t nsc_rsp[PTX_NSC_CMD_PROCESSOR_WPT_START_RSPLEN];
            uint32_t nsc_rsp_len = PTX_NSC_CMD_PROCESSOR_WPT_START_RSPLEN;

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, nsc_rsp, &nsc_rsp_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_WPT_START_RSP);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_WptStart, nsc_rsp, nsc_rsp_len, NULL);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_WLC_WptStop(ptx_NSC_t *nsc, uint8_t wptPostPowerLevel)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_WLC_WptStop");

        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_WptStop, NULL);
    }

    if ( PTX_STATUS_SUCCESS == status )
    {
        /*
         * Assemble WPT_STOP_CMD.
         */

        /* Allocate buffer for the command. */
        uint8_t nsc_buf[PTX_NSC_WLC_WPT_STOP_CMD_LEN];
        uint8_t *nsc_buf_pos = nsc_buf;

        /* Append WPT stop command opcode. */
        *nsc_buf_pos = PTX_NSC_WLC_WPT_STOP_CMD_OPCODE;
        nsc_buf_pos++;

        *nsc_buf_pos = wptPostPowerLevel;
        nsc_buf_pos++;

        status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, nsc_buf, PTX_NSC_WLC_WPT_STOP_CMD_LEN);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t nsc_rsp[PTX_NSC_CMD_PROCESSOR_WPT_STOP_RSPLEN];
            uint32_t nsc_rsp_len = PTX_NSC_CMD_PROCESSOR_WPT_STOP_RSPLEN;

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, nsc_rsp, &nsc_rsp_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_WPT_STOP_RSP);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_WptStop, nsc_rsp, nsc_rsp_len, NULL);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_WLC_ProcessWptStopNtf (uint8_t *buffer, uint32_t bufferLen, ptx_NSC_WLC_WptStop_Ntf_t* ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != buffer) && (0 != bufferLen) && (NULL != ntf) )
    {
        if (PTX_NSC_WLC_WPT_STOP_NTF_PARAM_LENGTH == bufferLen)
        {
            /* Store the reason of the WPT stop notification. */
            ntf->Reason = (ptx_NSC_WLC_WptStopReason_t)buffer[PTX_NSC_WLC_WPT_STOP_NTF_REASON_INDEX];
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_NSC_PROTOCOL;
        }
    }

    return status;
}
