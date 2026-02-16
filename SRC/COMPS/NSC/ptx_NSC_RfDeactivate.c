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
    Module      : NFC Soft Controller (NSC) Interface for RF Deactivation
    File        : ptx_NSC_RfDeactivate.c

    Description : Implementation of NFC Soft Controller Interface for RF Deactivation.
                  Contains functions(s) to start RF deactivation and to gather information
                  during the process.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_RfDeactivate.h"
#include "ptx_NSC_CmdProcessor.h"
#include "ptx_NSC_Error.h"
#include "ptx_NSC_StateMachine.h"
#include "ptx_HAL.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NSC_RF_DEACTIVATE_BUFFER_SIZE       (3u)    /* Maximum length of the message buffer.  */
#define PTX_NSC_RF_DEACTIVATE_NTF_LENGTH        (3u)    /* NSC RF Deactivate notification length. */
#define PTX_NSC_RF_DEACTIVATE_NTF_PARAM_LENGTH  (PTX_NSC_RF_DEACTIVATE_NTF_LENGTH - 1)    /* NSC RF Deactivate notification Parameters-field length. */

#define PTX_NSC_RF_DEACTIVATE_NTF_REASON_INDEX  (0u)    /* Notification reason index in the Parameters field of NSC notification message. */
#define PTX_NSC_RF_DEACTIVATE_NTF_ERROR_INDEX   (1u)    /* Notification error index in the Parameters field of NSC notification message. */

#define PTX_NSC_RFERR_NTF_IDX_OPCODE            (0u)    /* Error NTF Opcode index */
#define PTX_NSC_RFERR_NTF_IDX_ERRCODE           (1u)    /* Error Code index */

#define PTX_NSC_RFLPCD_NTF_IDX_OPCODE           (0u)    /* LPCD NTF Opcode index */

#define PTX_NSC_RF_DEACTIVATE_RSP_OPCODE_IDX    (0u)    /* NSC RF Deactivate response opcode index */
#define PTX_NSC_RF_DEACTIVATE_RSP_STATUS_IDX    (1u)    /* NSC RF Deactivate response status index */


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_RfDeactivate_CheckErr(ptx_NSC_t *nsc, uint8_t *msg, ptx_Status_t inStatus, ptx_NSC_RfDeact_Param_t *param, uint8_t *ntfToFollow);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_RfDeactivate(ptx_NSC_t *nsc, ptx_NSC_RfDeact_Param_t *param, uint8_t* ntfToFollow)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != param) && (NULL != ntfToFollow) )
    {
        uint32_t cmd_params[PTX_NSC_SM_DEACT_PARAMLEN];
        cmd_params[PTX_NSC_SM_DEACT_LEN_IDX]    = PTX_NSC_SM_DEACT_PARAMLEN;
        cmd_params[PTX_NSC_SM_DEACT_TYPE_IDX]   = param->Type;
        cmd_params[PTX_NSC_SM_DEACT_STATE_IDX]  = param->State;
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_RfDeactivate, &cmd_params[0]);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        switch (param->State)
        {
            case NSC_RfDeact_State_Idle:
                nsc->PendingState = ptx_NSC_State_Idle;
                break;

            case NSC_RfDeact_State_Poll:
                nsc->PendingState = ptx_NSC_State_Poll;
                break;

            case NSC_RfDeact_State_Discovery:
            /* FALL THROUGH */
            case NSC_RfDeact_State_DiscoveryWithRfReset:
                nsc->PendingState = ptx_NSC_State_Discovery;
                break;

            case NSC_RfDeact_State_ListenSleep:
                nsc->PendingState = ptx_NSC_State_Listen_Sleep;
                break;

            case NSC_RfDeact_State_ListenIdle:
                nsc->PendingState = ptx_NSC_State_Listen_Idle;
                break;

            default:
                /* RFU values. Interpreted as 0 (RF_IDLE). */
                nsc->PendingState = ptx_NSC_State_Idle;
                break;
        }

        /* Allocate buffer for the RF-Deactivate command. */
        uint8_t nsc_buf[PTX_NSC_RF_DEACTIVATE_BUFFER_SIZE];
        uint8_t *nsc_buf_pos = nsc_buf;

        /* Append RF-Deactivate command opcode. */
        *nsc_buf_pos = PTX_NSC_RF_DEACTIVATE_CMD_OPCODE;
        nsc_buf_pos++;

        *nsc_buf_pos = param->Type;
        nsc_buf_pos++;

        *nsc_buf_pos = param->State;
        nsc_buf_pos++;

        status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, nsc_buf, (uint32_t)(nsc_buf_pos - nsc_buf));

        /*
         * Special handling for I2C. When trying to wakeup the PTX2xxR/W on I2C while in RF_LISTEN_SLEEP state through a HIF source wakeup, the CMD will fail its transmission.
         * After the I2C Slave Address block is sent, a NACK is received from the chip. This is the intended behavior and the chip will indeed wakeup.
         * Unfortunately, this will cause an error status of the ptx_NSC_CmdProcessor_DispatchMsg function. This error must be ignored and the pending
         * Notification must be processed to receive a NSC_ERROR_NTF with a STDBY_ERR set, which indicates a successful wakeup.
         */
#if defined(PTX_ENABLE_I2C)
        if ((PTX_STATUS_SUCCESS == status) || (HAL_Interface_Type_I2C == nsc->HAL->Interface.Type))
#else
        if (PTX_STATUS_SUCCESS == status)
#endif
        {
            uint8_t nsc_rsp[PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_RSPLEN];
            uint32_t nsc_rsp_len = PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_RSPLEN;

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, nsc_rsp, &nsc_rsp_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_RSP);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_RfDeactivate, nsc_rsp, nsc_rsp_len, ntfToFollow);

                if (PTX_STATUS_SUCCESS != status)
                {
                    status = ptx_NSC_RfDeactivate_CheckErr(nsc, nsc_rsp, status, param, ntfToFollow);
                }
                else
                {
                    if (0 == *ntfToFollow)
                    {
                        nsc->State = nsc->PendingState;
                    }
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_ProcessRfDeactivateNtf (uint8_t *buffer, uint32_t bufferLen, ptx_NSC_RfDeactivate_Ntf_t* ntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != buffer) && (0 != bufferLen) && (NULL != ntf) )
    {
        if (PTX_NSC_RF_DEACTIVATE_NTF_PARAM_LENGTH == bufferLen)
        {
            /* Store the reason of the deactivation notification. */
            ntf->Reason = (ptx_NSC_RfDeactivate_Reason_t) buffer[PTX_NSC_RF_DEACTIVATE_NTF_REASON_INDEX];

            /* Check received error code. */
            ntf->Error = (ptx_NSC_ErrorCodes_t) buffer[PTX_NSC_RF_DEACTIVATE_NTF_ERROR_INDEX];
            if (NSC_ErrorCode_NoErr == ntf->Error)
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_NSC_RFERROR;
            }
        }
        else
        {
            status = PTX_STATUS_E_NSC_PROTOCOL;
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_RfDeactivate_CheckErr(ptx_NSC_t *nsc, uint8_t *msg, ptx_Status_t inStatus, ptx_NSC_RfDeact_Param_t *param, uint8_t *ntfToFollow)
{
    ptx_Status_t status = inStatus;

    /**
     * NOTE: special cases handling is required only for the RfDeactivate API. It is due to the fact that NSC
     * might unexpectedly change its state in the middle of the API execution. The state change is caused by
     * the reception of certain asynchronous messages (notifications). In the cases described below, this unexpected
     * state change might even be treated as successful execution.
     *
     * Special cases handling:
     * -----------------------
     * Scenario 1
     * Handle special case when NSC_ERROR_NTF(Standby Err) is received instead of NSC_RF_DEACTIVATE_RSP.
     * In such a case previous call to ptx_NSC_CmdProcessor_ProcessRsp shall fail with PTX_STATUS_E_NSC_PROTOCOL.
     * Special status error code shall be set here in order to facilitate the decision making process for the upper layers -
     * how to handle this special error case.
     * If the caller requested RF_Deactivate to IDLE, and the above mentioned error occurred,
     * this shall be treated as successful deactivation, since NSC turned into IDLE.
     *
     * Scenario 2
     * If PTX sends RF_LPCD_NTF while in LPCD mode, the state shall automatically change to IDLE. If this happened
     * while execution is already within RfDeactivate API when the command is being dispatched, NSC RSP message shall
     * contain status INVALID_CMD since RfDeactivate command is not allowed in IDLE state.
     * However, if the caller issued RfDeactivate to IDLE this shall be treated as successful transition, i.e. successful API operation.
     * Since this is the expected transition, API response needs to be set to SUCCESS.
     *
     * Scenario 3
     * One more fail case is when we get NSC_RF_LPCD_NTF instead of NSC_RFDEACTIVATE_RSP. Again, if the caller issued
     * deactivation to IDLE state and NSC has already transitioned to IDLE, this shall be treated as a successful scenario.
     *
     * Scenario 4
     * The user attempts to deactivate an ISO-DEP capable Tag with an S(DESELCT) command, however the Tag does not respond with an S(DESLECT) response.
     * In this case we receive an NSC Deactivate response with a timeout error.
     * This is valid and according to the specification we may ignore the PICC.
     * However,due to the error state returned, the internal NSC state was not updated and therefore we are required to send a generic deactivate command
     * to the requested state in order to change the internal state according to the requested state
     */

    if (PTX_STATUS_E_NSC_PROTOCOL == status)
    {
        if ( (PTX_NSC_CMD_PROCESSOR_ERROR_NTF == msg[PTX_NSC_RFERR_NTF_IDX_OPCODE]) &&
             (NSC_ErrorCode_StandbyMode == (ptx_NSC_ErrorCodes_t)msg[PTX_NSC_RFERR_NTF_IDX_ERRCODE]) )
        {
            /* Scenario 1 */

            /* Standby error received. NSC turned to IDLE. */
            status = PTX_STATUS_E_NSC_ERR_STANDBY;

            if ( (ptx_NSC_State_Idle == nsc->State) && (ptx_NSC_State_Idle == nsc->PendingState) )
            {
                /* The caller requested to deactivate to IDLE and the state eventually turned to IDLE after
                 * NSC_ERROR_NTF. This is a valid transition.
                 */
                status = PTX_STATUS_SUCCESS;
            }
        }
        else if ( (PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_RSP == msg[PTX_NSC_RF_DEACTIVATE_RSP_OPCODE_IDX]) &&
                  ((uint8_t)NSC_ErrorCode_InvalidCommand == msg[PTX_NSC_RF_DEACTIVATE_RSP_STATUS_IDX]) )
        {
            /* Scenario 2 */

            if ( (ptx_NSC_State_Idle == nsc->State) && (ptx_NSC_State_Idle == nsc->PendingState) )
            {
                /* The caller requested to deactivate to IDLE and the state eventually turned to IDLE after
                 * NSC_RF_LPCD_NTF. This is a valid transition.
                 */
                status = PTX_STATUS_SUCCESS;
            }
        }
        else if ( PTX_NSC_CMD_PROCESSOR_RFLPCD_NTF == msg[PTX_NSC_RFLPCD_NTF_IDX_OPCODE] )
        {
            /* Scenario 3 */

            if ( (ptx_NSC_State_Idle == nsc->State) && (ptx_NSC_State_Idle == nsc->PendingState) )
            {
                /* The caller requested to deactivate to IDLE and the state eventually turned to IDLE after
                 * NSC_RF_LPCD_NTF. This is a valid transition.
                 */
                status = PTX_STATUS_SUCCESS;
            }
        }
        else if ( (PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_RSP == msg[PTX_NSC_RF_DEACTIVATE_RSP_OPCODE_IDX]) &&
                  ((uint8_t)NSC_ErrorCode_RfTimeoutError == msg[PTX_NSC_RF_DEACTIVATE_RSP_STATUS_IDX]) &&
                  (NSC_RfDeact_Type_ProtSpecific == param->Type))
        {
            /* Scenario 4 */

            if (ptx_NSC_State_Poll_Dep == nsc->State)
            {
                /* The caller sent an S(DESELECT) command, but the Tag did not respond with an S(DESLECT) response
                 * According to the specification we may ignore the PICC.
                 * However, as the deactivate command was answered as a failure, the internal NSC state was not trasitioned
                 * This generic Deactivation with the same State shall assure that we complete the deactivation in the state the user requested*/

                param->Type = NSC_RfDeact_Type_Generic;
                status = ptx_NSC_RfDeactivate(nsc, param, ntfToFollow);
            }

        }
        else
        {
            /* Keep the error */
        }
    }

    return status;
}

