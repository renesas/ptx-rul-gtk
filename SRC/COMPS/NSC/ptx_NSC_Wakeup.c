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
    Module      : NFC Soft Controller (NSC) Interface for System Wake-up
    File        : ptx_NSC_Wakeup.c

    Description : Implementation of NFC Soft Controller Interface for System Wake-up.
                  Contains the function(s) to change the system operation mode from
                  Standby to normal active mode, into RF_IDLE state.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Wakeup.h"
#include "ptx_Components.h"
#include "ptx_NSC_StateMachine.h"
#include "ptx_HAL.h"

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

ptx_Status_t ptx_NSC_Wakeup (ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_Wakeup, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {

        uint8_t buffer[PTX_NSC_WAKEUP_CMD_LENGTH];
        memset(buffer, 0, PTX_NSC_WAKEUP_CMD_LENGTH);

        buffer[0] = (uint8_t)PTX_NSC_WAKEUP_CMD_OPCODE;
        status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, buffer, PTX_NSC_WAKEUP_CMD_LENGTH);



        /*
         * Special handling for I2C. When trying to wakeup the PTX2xxR/W with the NSC_WAKEUP_CMD on I2C, the CMD will fail its transmission.
         * After the I2C Slave Address block is sent, a NACK is received from the chip. This is the intended behavior and the chip will indeed wakeup.
         * Unfortunately, this will cause an error status of the ptx_NSC_CmdProcessor_DispatchMsg function. This error must be ignored and the pending
         * Response must be processed to receive the NSC_WAKEUP_RSP, which indicates a successful wakeup.
         */
#if defined(PTX_ENABLE_I2C)
        if ((PTX_STATUS_SUCCESS == status) || (HAL_Interface_Type_I2C == nsc->HAL->Interface.Type))
#else
        if (PTX_STATUS_SUCCESS == status)
#endif
        {
            uint8_t rsp_buffer[PTX_NSC_CMD_PROCESSOR_WAKEUP_RSPLEN];
            uint32_t rsp_buffer_len = PTX_NSC_CMD_PROCESSOR_WAKEUP_RSPLEN;
            memset(rsp_buffer, 0, sizeof(uint8_t)*PTX_NSC_CMD_PROCESSOR_WAKEUP_RSPLEN);

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_buffer, &rsp_buffer_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_WAKEUP_RSP);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_Wakeup, rsp_buffer, rsp_buffer_len, NULL);
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
