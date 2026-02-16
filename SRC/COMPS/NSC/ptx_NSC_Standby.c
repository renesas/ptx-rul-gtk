/** \file
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
    Module      : NFC Soft Controller (NSC) Interface for Standby
    File        : ptx_NSC_Standby.c

    Description : Implementation of NFC Soft Controller Interface for Standby mode.
                  Contains the function(s) to change the system operation mode from
                  normal active mode (RF_IDLE state) into Standby mode (low power operating mode).
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Components.h"
#include "ptx_NSC_Int.h"
#include "ptx_NSC_Standby.h"
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
ptx_Status_t ptx_NSC_Standby (ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_Standby, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t buffer[PTX_NSC_STANDBY_CMD_LENGTH];
        memset(buffer, 0, PTX_NSC_STANDBY_CMD_LENGTH);

        buffer[0] = (uint8_t)PTX_NSC_STANDBY_CMD_OPCODE;
        status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, buffer, PTX_NSC_STANDBY_CMD_LENGTH);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t rsp_buffer[PTX_NSC_CMD_PROCESSOR_STANDBY_RSPLEN];
        uint32_t rsp_buffer_len = PTX_NSC_CMD_PROCESSOR_STANDBY_RSPLEN;
        memset(rsp_buffer, 0, sizeof(uint8_t)*PTX_NSC_CMD_PROCESSOR_STANDBY_RSPLEN);

        status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_buffer, &rsp_buffer_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_STANDBY_RSP);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_Standby, rsp_buffer, rsp_buffer_len, NULL);
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
