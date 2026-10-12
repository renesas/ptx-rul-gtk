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
    Module      : NFC Soft Controller (NSC) Interface for Command Processing
    File        : ptx_NSC_CmdProcessor.c

    Description : Implementation of NFC Soft Controller Interface for Command Processing.
                  Contains function(s) for basic NSC protocol message processing and dispatching.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HIP.h"
#include "ptx_HAL.h"
#include "ptx_NSC_CmdProcessor.h"
#include "ptx_NSC_Error.h"
#include "ptx_NSC_RfActivate.h"
#include "ptx_NSC_RfDeactivate.h"
#include "ptx_NSC_RfDiscover.h"
#if !defined(PTX_DISABLE_WLC)
    #include "ptx_NSC_Wlc.h"
#endif
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * NSC Command Processor specific RSP and NTF message definitions.
 */
#define PTX_NSC_CMD_PROCESSOR_RSP_ID_OFFSET                 (0u)            /* CmdProcessor response opcode field offset */
#define PTX_NSC_CMD_PROCESSOR_RSP_STATUS_OFFSET             (1u)            /* CmdProcessor status field offset */
#define PTX_NSC_CMD_PROCESSOR_RSP_DATA_PAYLOAD_OFFSET       (2u)            /* CmdProcessor data payload offset */
#define PTX_NSC_CMD_PROCESSOR_RSP_PPS1_BRS_OFFSET           (2u)            /* CmdProcessor PPS1/BRS field offset */
#define PTX_NSC_CMD_PROCESSOR_MIN_RSP_LEN                   (2u)            /* Minimum response length: Opcode + ErrorCode */

#define PTX_NSC_CMD_PROCESSOR_RFFIELD_OFF                   (0u)            /* Rf-Field notification value OFF. */
#define PTX_NSC_CMD_PROCESSOR_RFFIELD_ON                    (1u)            /* Rf-Field notification value ON. */

#define PTX_NSC_CMD_PROCESSOR_ACTNTF_MINLEN                 (3u)            /* Minimum length of NSC_RF_ACTIVATE_NTF: Opcode + RfTech + RfProt */

/*
 * FSCI MTU Size
 */
#define PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_0               (13u)           /* FSCI 0 */
#define PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_1               (21u)           /* FSCI 1 */
#define PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_2               (29u)           /* FSCI 2 */
#define PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_3               (37u)           /* FSCI 3 */
#define PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_4               (45u)           /* FSCI 4 */
#define PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_5               (61u)           /* FSCI 5 */
#define PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_6               (93u)           /* FSCI 6 */
#define PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_7               (125u)          /* FSCI 7 */
#define PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_8               (253u)          /* FSCI 8 */
#define PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_MAX             (253u)          /* FSCI MAX */

/*
 * FSDI MTU Size
 */
#define PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_0               (13u)           /* FSDI 0 */
#define PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_1               (21u)           /* FSDI 1 */
#define PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_2               (29u)           /* FSDI 2 */
#define PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_3               (37u)           /* FSDI 3 */
#define PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_4               (45u)           /* FSDI 4 */
#define PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_5               (61u)           /* FSDI 5 */
#define PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_6               (93u)           /* FSDI 6 */
#define PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_7               (125u)          /* FSDI 7 */
#define PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_8               (253u)          /* FSDI 8 */
#define PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_MAX             (253u)          /* FSDI MAX */

/*
 * FSCI Values
 */
#define PTX_NSC_RFPROT_PARAMS_FSCI_VAL_0                    (0u)            /* FSCI Value 0 */
#define PTX_NSC_RFPROT_PARAMS_FSCI_VAL_1                    (1u)            /* FSCI Value 1 */
#define PTX_NSC_RFPROT_PARAMS_FSCI_VAL_2                    (2u)            /* FSCI Value 2 */
#define PTX_NSC_RFPROT_PARAMS_FSCI_VAL_3                    (3u)            /* FSCI Value 3 */
#define PTX_NSC_RFPROT_PARAMS_FSCI_VAL_4                    (4u)            /* FSCI Value 4 */
#define PTX_NSC_RFPROT_PARAMS_FSCI_VAL_5                    (5u)            /* FSCI Value 5 */
#define PTX_NSC_RFPROT_PARAMS_FSCI_VAL_6                    (6u)            /* FSCI Value 6 */
#define PTX_NSC_RFPROT_PARAMS_FSCI_VAL_7                    (7u)            /* FSCI Value 7 */
#define PTX_NSC_RFPROT_PARAMS_FSCI_VAL_8                    (8u)            /* FSCI Value 8 */

/*
 * FSDI Values
 */
#define PTX_NSC_RFPROT_PARAMS_FSDI_VAL_0                    (0u)            /* FSDI Value 0 */
#define PTX_NSC_RFPROT_PARAMS_FSDI_VAL_1                    (1u)            /* FSDI Value 1 */
#define PTX_NSC_RFPROT_PARAMS_FSDI_VAL_2                    (2u)            /* FSDI Value 2 */
#define PTX_NSC_RFPROT_PARAMS_FSDI_VAL_3                    (3u)            /* FSDI Value 3 */
#define PTX_NSC_RFPROT_PARAMS_FSDI_VAL_4                    (4u)            /* FSDI Value 4 */
#define PTX_NSC_RFPROT_PARAMS_FSDI_VAL_5                    (5u)            /* FSDI Value 5 */
#define PTX_NSC_RFPROT_PARAMS_FSDI_VAL_6                    (6u)            /* FSDI Value 6 */
#define PTX_NSC_RFPROT_PARAMS_FSDI_VAL_7                    (7u)            /* FSDI Value 7 */
#define PTX_NSC_RFPROT_PARAMS_FSDI_VAL_8                    (8u)            /* FSDI Value 8 */

/*
 * LR MTU Size
 */
#define PTX_NSC_CMD_PROCESSOR_LR_SIZE_64    (64u)                           /* MTU size of 64 byte for NFC-DEP */
#define PTX_NSC_CMD_PROCESSOR_LR_SIZE_128   (128u)                          /* MTU size of 128 byte for NFC-DEP */
#define PTX_NSC_CMD_PROCESSOR_LR_SIZE_192   (192)                           /* MTU size of 192 byte for NFC-DEP */
#define PTX_NSC_CMD_PROCESSOR_LR_SIZE_MAX   (254u)                          /* Maximum MTU size of 254 byte for NFC-DEP */

/* Calculate the maximum usable payload bytes for NFC-DEP */
#define PTX_NSC_CMD_PROCESSOR_CALC_MTU_SIZE(x)                (uint8_t)(x-3)

/*
 * LR Value
 */
#define PTX_NSC_CMD_PROCESSOR_LR_VAL_0  (0x00)
#define PTX_NSC_CMD_PROCESSOR_LR_VAL_1  (0x10)
#define PTX_NSC_CMD_PROCESSOR_LR_VAL_2  (0x20)
#define PTX_NSC_CMD_PROCESSOR_LR_VAL_3  (0x30)

/*
 * Maximum number of NSC messages to be fetched in a single call to ptx_NSC_CmdProcessor_FetchMsg.
 */
#define PTX_NSC_FETCHMSG_COUNT_MAX                          (10u)           /* Max. 10 messages */


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NSC_CmdProcessor_ProcessNtfMsg(ptx_NSC_t *nsc, uint8_t *msg, uint32_t msgLen);
static ptx_Status_t ptx_NSC_CmdProcessor_ProcessDataMsg(ptx_NSC_t *nsc, uint8_t *msg, uint32_t msgLen);
static void ptx_NSC_ParseRfActivateNtf(ptx_NSC_t *nsc, ptx_NSC_RfActivate_Ntf_t *ntf);
static void ptx_NSC_ParseRfDeactivateNtf(ptx_NSC_t *nsc, ptx_NSC_RfDeactivate_Ntf_t *ntf, ptx_Status_t *st);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_CmdProcessor_Config (ptx_NSC_t *nsc, ptx_NSC_CmdProcessor_Cfg_t *cfgParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != cfgParams))
    {       
        PTX_LOG_I(nsc->Log, "ptx_NSC_CmdProcessor_Config");


        /* All callback functions have to be registered, so all the input parameters must be valid.
         * Note: FetchMsg CB is used and set by NSC, and so it is not configured within this API.
         */

        const uint8_t params_count_to_be_configured = 10U;
        uint8_t params_configured = 0;

        if (NULL != cfgParams->Ctx)
        {
            nsc->CmdProcessor.Ctx = cfgParams->Ctx;
            params_configured++;
        }

        if (NULL != cfgParams->ProcessResetNtf)
        {
            nsc->CmdProcessor.ProcessResetNtf = cfgParams->ProcessResetNtf;
            params_configured++;
        }

        if (NULL != cfgParams->ProcessRfDiscoverNtf)
        {
            nsc->CmdProcessor.ProcessRfDiscoverNtf = cfgParams->ProcessRfDiscoverNtf;
            params_configured++;
        }

        if (NULL != cfgParams->ProcessRfLpcdNtf)
        {
            nsc->CmdProcessor.ProcessRfLpcdNtf = cfgParams->ProcessRfLpcdNtf;
            params_configured++;
        }

        if (NULL != cfgParams->ProcessRfActivateNtf)
        {
            nsc->CmdProcessor.ProcessRfActivateNtf = cfgParams->ProcessRfActivateNtf;
            params_configured++;
        }

        if (NULL != cfgParams->ProcessRfDeactivateNtf)
        {
            nsc->CmdProcessor.ProcessRfDeactivateNtf = cfgParams->ProcessRfDeactivateNtf;
            params_configured++;
        }

        if (NULL != cfgParams->ProcessErrorNtf)
        {
            nsc->CmdProcessor.ProcessErrorNtf = cfgParams->ProcessErrorNtf;
            params_configured++;
        }

        if (NULL != cfgParams->ProcessDataMsg)
        {
            nsc->CmdProcessor.ProcessDataMsg = cfgParams->ProcessDataMsg;
            params_configured++;
        }

        if (NULL != cfgParams->ProcessRfFieldNtf)
        {
            nsc->CmdProcessor.ProcessRfFieldNtf = cfgParams->ProcessRfFieldNtf;
            params_configured++;
        }

        if (NULL != cfgParams->ProcessWptStopNtf)
        {
            nsc->CmdProcessor.ProcessWptStopNtf = cfgParams->ProcessWptStopNtf;
            params_configured++;
        }

        if (params_count_to_be_configured == params_configured)
        {
            /* All parameters shall be configured. Otherwise, the API is not used properly. */
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_CmdProcessor_DispatchMsg(ptx_NSC_t *nsc, uint16_t writeBuffer, uint8_t *msg, uint32_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_CmdProcessor_DispatchMsg");

        status = ptx_NSC_CmdProcessor_GetPendingMsg(nsc);

        if (PTX_STATUS_SUCCESS == status)
        {
            if (msgLen <= PTX_NSC_LENGTH_MAX)
            {
                status = ptx_HIP_WrMsg(nsc->HIP, (uint8_t) writeBuffer, msg, msgLen);
            }
            else
            {
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_CmdProcessor_FetchMsg(ptx_NSC_t *nsc, uint8_t *msg, uint32_t *msgLen, uint32_t timeout, uint8_t msgOpCode)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != nsc->CmdProcessor.FetchMsg) && (NULL != msg))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_CmdProcessor_FetchMsg");

        uint8_t msg_type_received;
        uint8_t num_retries = 0;

        /* If expected msgOpCode is 0, then only one message will be read. */
        uint8_t expected_msg_type = msgOpCode;

        /* Store the expected message length. */
        uint32_t expected_msg_length = *msgLen;

        do
        {
            /* If there are more than one messages coming, expected message length needs to be restored,
             * because msgLen gets overwritten within FetchMsg callback.
             */
            *msgLen = expected_msg_length;

            status = nsc->CmdProcessor.FetchMsg(nsc, msg, msgLen, timeout);

            if (PTX_STATUS_SUCCESS == status)
            {
                msg_type_received = msg[PTX_NSC_CMD_PROCESSOR_FRAMETYPE_OFFSET];
            }
            else
            {
                msg_type_received = 0;
            }

            if ((((PTX_STATUS_SUCCESS == status) || (PTX_STATUS_E_NSC_RX_ERROR_LENGTH == status))) && (msg_type_received != expected_msg_type))
            {
                if (ptx_NSC_State_Error == nsc->State)
                {
                    /* If we got a notification and it was not the expected type of message, check if there was critical error in PTX2xxR/W.
                     * Abort in such a case.
                     */
                    status = PTX_STATUS_E_NSC_SYSTEM;
                }
                else if ((PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_RSP == expected_msg_type) &&
                         (PTX_NSC_CMD_PROCESSOR_ERROR_NTF == msg_type_received) &&
                         (ptx_NSC_State_Idle == nsc->State))
                {
                    /* Special case if an error notification has been received after we sent DeactivateCmd, and NSC turned to IDLE.
                     * This shall happen only during Discovery, if NSC was in Standby mode.
                     * Break the loop here. Status needs to stay SUCCESS, since the message gets additionally processed in the calling API.
                     */
                    num_retries = PTX_NSC_FETCHMSG_COUNT_MAX;
                }
                else
                {
                    /* Just proceed. */
                }
            }

            num_retries++;
        }
        while (((PTX_STATUS_SUCCESS == status) || (PTX_STATUS_E_NSC_RX_ERROR_LENGTH == status)) && (0 != expected_msg_type) &&
               (msg_type_received != expected_msg_type) && (num_retries < PTX_NSC_FETCHMSG_COUNT_MAX));
    }

    return status;
}

ptx_Status_t ptx_NSC_CmdProcessor_ProcessRsp(ptx_NSC_t *nsc, ptx_NSC_CmdProcessor_RspId_t rspId, uint8_t *msg, uint32_t msgLen, uint8_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != msg) && (PTX_NSC_CMD_PROCESSOR_MIN_RSP_LEN <= msgLen))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_CmdProcessor_ProcessRsp");

        if (PTX_NSC_CMD_PROCESSOR_LOOPBACK_RSP == msg[PTX_NSC_CMD_PROCESSOR_RSP_ID_OFFSET])
        {
            /* Special handling required for LOOPBACK_RSP message. ErrorCode is not the first byte.
             * Just continue processing.
             */
            status = PTX_STATUS_SUCCESS;
        }
        else if (PTX_NSC_CMD_PROCESSOR_RFACTIVATE_RSP == msg[PTX_NSC_CMD_PROCESSOR_RSP_ID_OFFSET])
        {
            /* Special handling required for ACTIVATE_RSP message. RF-Errors are possible here (e.g. card movement during selection).
             * RF-Errors to be propagated to upper layers.
             */
            switch (msg[PTX_NSC_CMD_PROCESSOR_RSP_STATUS_OFFSET])
            {
                case NSC_ErrorCode_NoErr:
                    status = PTX_STATUS_SUCCESS;
                    break;

                case NSC_ErrorCode_RfTimeoutError:
                case NSC_ErrorCode_RfTransmissionError:
                case NSC_ErrorCode_RfProtocolError:
                    status = PTX_STATUS_E_NSC_RFERROR;
                    break;

                default:
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                    break;
            }
        }
        else if (NSC_ErrorCode_NoErr != msg[PTX_NSC_CMD_PROCESSOR_RSP_STATUS_OFFSET])
        {
            /* For all other responses check the ErrorCode at the first position in the message payload.
             * If an error is set, command failed. Stop processing RSP message here.
             */
            status = PTX_STATUS_E_NSC_PROTOCOL;
        }
        else
        {
            /* Response message ErrorCode is NO_ERROR value. Keep on processing. */
            status = PTX_STATUS_SUCCESS;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t rsp_opcode = msg[PTX_NSC_CMD_PROCESSOR_RSP_ID_OFFSET];

            status = PTX_STATUS_E_NSC_PROTOCOL;

            /* Parse RSP message. */
            switch (rspId)
            {
                case ptx_NSC_CmdProcessor_RspId_RfDiscover:
                    if ((PTX_NSC_CMD_PROCESSOR_RFDISCOVER_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_RFDISCOVER_RSPLEN == msgLen))
                    {
                        /* State transition occurs after successful NSC_RF_DISCOVER_RSP. By default, next state is RF_DISCOVERY.
                         *  However, there might be a specific transition required. This is handled in ptx_NSC_RfDiscover API.
                         */
                        nsc->State = ptx_NSC_State_Discovery;
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;
                case ptx_NSC_CmdProcessor_RspId_Init:
                    if ((PTX_NSC_CMD_PROCESSOR_INIT_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_INIT_RSPLEN == msgLen))
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;
                case ptx_NSC_CmdProcessor_RspId_Read:
                    if ((PTX_NSC_CMD_PROCESSOR_READ_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_READ_RSPLEN <= msgLen))
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;
                case ptx_NSC_CmdProcessor_RspId_Write:
                    if ((PTX_NSC_CMD_PROCESSOR_WRITE_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_WRITE_RSPLEN == msgLen))
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;
                case ptx_NSC_CmdProcessor_RspId_Loopback:
                    if ((PTX_NSC_CMD_PROCESSOR_LOOPBACK_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_LOOPBACK_RSPLEN <= msgLen))
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;
                case ptx_NSC_CmdProcessor_RspId_GetStatus:
                    if ((PTX_NSC_CMD_PROCESSOR_GETSTATUS_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_GETSTATUS_RSPLEN == msgLen))
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;
                case ptx_NSC_CmdProcessor_RspId_RfConfig:
                    if ((PTX_NSC_CMD_PROCESSOR_RFCONFIG_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_RFCONFIG_RSPLEN == msgLen))
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;
                case ptx_NSC_CmdProcessor_RspId_RfActivate:
                    if ((PTX_NSC_CMD_PROCESSOR_RFACTIVATE_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_RFACTIVATE_RSPLEN <= msgLen))
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;
                case ptx_NSC_CmdProcessor_RspId_RfDeactivate:
                    if ((PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_RSPLEN == msgLen))
                    {
                        if (NULL != data)
                        {
                            *data = msg[PTX_NSC_CMD_PROCESSOR_RSP_DATA_PAYLOAD_OFFSET];
                            status = PTX_STATUS_SUCCESS;
                        }
                        else
                        {
                            /* Invalid parameter provided. */
                            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                        }
                    }
                    break;
                case ptx_NSC_CmdProcessor_RspId_RfSetParam:
                    if ((PTX_NSC_CMD_PROCESSOR_RFSETPARAM_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_RFSETPARAM_RSPLEN == msgLen))
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;

#if !defined(PTX_DISABLE_WLC)
                case ptx_NSC_CmdProcessor_RspId_WptStart:
                    if ((PTX_NSC_CMD_PROCESSOR_WPT_START_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_WPT_START_RSPLEN == msgLen))
                    {
                        if (ptx_NSC_State_Poll_Dep == nsc->State)
                        {
                            nsc->State = ptx_NSC_State_Wpt_Dep;
                            status = PTX_STATUS_SUCCESS;
                        }
                        else if (ptx_NSC_State_Poll == nsc->State)
                        {
                            nsc->State = ptx_NSC_State_Wpt;
                            status = PTX_STATUS_SUCCESS;
                        }
                        else
                        {
                            status = PTX_STATUS_E_NSC_SYSTEM;
                        }
                    }
                    break;

                case ptx_NSC_CmdProcessor_RspId_WptStop:
                    if ((PTX_NSC_CMD_PROCESSOR_WPT_STOP_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_WPT_STOP_RSPLEN == msgLen))
                    {
                        if (ptx_NSC_State_Wpt_Dep == nsc->State)
                        {
                            nsc->State = ptx_NSC_State_Poll_Dep;
                            status = PTX_STATUS_SUCCESS;
                        }
                        else if (ptx_NSC_State_Wpt == nsc->State)
                        {
                            nsc->State = ptx_NSC_State_Poll;
                            status = PTX_STATUS_SUCCESS;
                        }
                        else
                        {
                            status = PTX_STATUS_E_NSC_SYSTEM;
                        }
                    }
                    break;
#endif
                case ptx_NSC_CmdProcessor_RspId_Standby:
                    if ((PTX_NSC_CMD_PROCESSOR_STANDBY_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_STANDBY_RSPLEN == msgLen))
                    {
                        nsc->State = ptx_NSC_State_Standby;
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;

                case ptx_NSC_CmdProcessor_RspId_Wakeup:
                    if ((PTX_NSC_CMD_PROCESSOR_WAKEUP_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_WAKEUP_RSPLEN == msgLen))
                    {
                        nsc->State = ptx_NSC_State_Idle;
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;

                case ptx_NSC_CmdProcessor_RspId_RfSetRoutingTbl:
                    if ((PTX_NSC_CMD_PROCESSOR_RFSETROUTINGTABLE_RSP == rsp_opcode) && (PTX_NSC_CMD_PROCESSOR_RFSETROUTINGTABLE_RSPLEN == msgLen))
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;

                default:
                    /* No action required. Protocol error. */
                    break;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_CmdProcessor_GetPendingMsg(ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_CmdProcessor_GetPendingMsg");

        uint8_t isRxPending = 0;
        status = ptx_HAL_Interface_RxPending(nsc->HAL, &isRxPending);

        if ((PTX_STATUS_SUCCESS == status) && (1 == isRxPending))
        {
            uint16_t msg_len = 0;

            status = ptx_HIP_RdMsgLen(nsc->HIP, &msg_len);
            if ((PTX_STATUS_SUCCESS == status) && (0 != msg_len) && (msg_len <= PTX_NSC_LENGTH_MAX))
            {
                /* HIP read message length succeeded. Valid message length. */
                uint8_t msg[PTX_NSC_LENGTH_MAX];

                status = ptx_HIP_RdMsg(nsc->HIP, &msg[0], msg_len);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NSC_CmdProcessor_ProcessMsg(nsc, msg, msg_len);
                }
            }
            else if (PTX_STATUS_SUCCESS != status)
            {
                /* HIP read message length failed. Forward the error up. */
            }
            else
            {
                /* HIP read message length succeeded. However, message length is invalid. */
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_CmdProcessor_ProcessMsg(ptx_NSC_t *nsc, uint8_t *msg, uint32_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != msg) && (0 != msgLen))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_CmdProcessor_ProcessMsg");

        uint8_t msg_type = (msg[PTX_NSC_CMD_PROCESSOR_FRAMETYPE_OFFSET] & PTX_NSC_CMD_PROCESSOR_FRAMETYPE_MASK);

        nsc->CmdProcessor.LastMsg.IsNewMsg = 1;
        nsc->CmdProcessor.LastMsg.Type = msg_type;
        nsc->CmdProcessor.LastMsg.Length = msgLen;
        memcpy(nsc->CmdProcessor.LastMsg.Payload, msg, msgLen);

        switch (msg_type)
        {
            case PTX_NSC_CMD_PROCESSOR_FRAMETYPE_RSP:
                /* No pre-processing, just leave the message as is. */
                status = PTX_STATUS_SUCCESS;
                break;

            case PTX_NSC_CMD_PROCESSOR_FRAMETYPE_NTF:
                status = ptx_NSC_CmdProcessor_ProcessNtfMsg(nsc, msg, msgLen);
                break;

            case PTX_NSC_CMD_PROCESSOR_FRAMETYPE_DATA:
                status = ptx_NSC_CmdProcessor_ProcessDataMsg(nsc, msg, msgLen);
                break;

            default:
                status = PTX_STATUS_E_NSC_PROTOCOL;
                break;
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_CmdProcessor_ProcessNtfMsg(ptx_NSC_t *nsc, uint8_t *msg, uint32_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != msg) && (0 != msgLen))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_CmdProcessor_ProcessNtfMsg");

        switch (msg[0])
        {
            case PTX_NSC_CMD_PROCESSOR_RESET_NTF:
            {
                if (PTX_NSC_CMD_PROCESSOR_RESET_NTFLEN_MIN <= msgLen)
                {
                    nsc->State = ptx_NSC_State_Init;
                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }

                if ((PTX_STATUS_SUCCESS == status) && (NULL != nsc->CmdProcessor.ProcessResetNtf))
                {
                    status = nsc->CmdProcessor.ProcessResetNtf(nsc->CmdProcessor.Ctx);
                }
                break;
            }

            case PTX_NSC_CMD_PROCESSOR_RFFIELD_NTF:
            {
                if (PTX_NSC_CMD_PROCESSOR_RFFIELD_NTFLEN == msgLen)
                {
                    if (PTX_NSC_CMD_PROCESSOR_RFFIELD_ON == msg[1])
                    {
                        nsc->State = ptx_NSC_State_Listen_Idle;
                    }
                    else if (PTX_NSC_CMD_PROCESSOR_RFFIELD_OFF == msg[1])
                    {
                        nsc->State = ptx_NSC_State_Discovery;
                    }
                    else
                    {
                        /* RFU. Keep the state. */
                    }

                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }

                if ((PTX_STATUS_SUCCESS == status) && (NULL != nsc->CmdProcessor.ProcessRfFieldNtf))
                {
                    status = nsc->CmdProcessor.ProcessRfFieldNtf(nsc->CmdProcessor.Ctx, msg[1]);
                }

                break;
            }

            case PTX_NSC_CMD_PROCESSOR_RFDISCOVER_NTF:
            {
                ptx_NSC_RfDiscover_Ntf_t ntf;
                memset(&ntf, 0, sizeof(ptx_NSC_RfDiscover_Ntf_t));

                status = ptx_NSC_ProcessRfDiscoverNtf(&msg[0], msgLen, &ntf);

                if (PTX_STATUS_SUCCESS == status)
                {
                    if (1 == ntf.IsLastNtf)
                    {
                        nsc->State = ptx_NSC_State_Poll;
                    }

                    if (NULL != nsc->CmdProcessor.ProcessRfDiscoverNtf)
                    {
                        status = nsc->CmdProcessor.ProcessRfDiscoverNtf(nsc->CmdProcessor.Ctx, &ntf);
                    }
                }
                break;
            }

            case PTX_NSC_CMD_PROCESSOR_RFLPCD_NTF:
            {
                if (PTX_NSC_CMD_PROCESSOR_RFLPCD_NTFLEN == msgLen)
                {
                    status = PTX_STATUS_SUCCESS;

                    /* If poll mode is LPCD, NSC turns into RF_IDLE. */
                    if ((uint8_t)NSC_RfDiscover_PollMode_Lpcd == nsc->PollMode)
                    {
                        nsc->State = ptx_NSC_State_Idle;
                    }
                }
                else
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }

                if ((PTX_STATUS_SUCCESS == status) && (NULL != nsc->CmdProcessor.ProcessRfLpcdNtf))
                {
                    status = nsc->CmdProcessor.ProcessRfLpcdNtf(nsc->CmdProcessor.Ctx);
                }
                break;
            }

            case PTX_NSC_CMD_PROCESSOR_RFACTIVATE_NTF:
            {
                ptx_NSC_RfActivate_Ntf_t ntf;
                memset(&ntf, 0, sizeof(ptx_NSC_RfActivate_Ntf_t));

                if (PTX_NSC_CMD_PROCESSOR_ACTNTF_MINLEN <= msgLen)
                {
                    status = ptx_NSC_ProcessRfActivateNtf(&msg[1], msgLen - 1, &ntf);
                }
                else
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Do state transitions and additional data parsing. */
                    ptx_NSC_ParseRfActivateNtf(nsc, &ntf);

                    if (NULL != nsc->CmdProcessor.ProcessRfActivateNtf)
                    {
                        status = nsc->CmdProcessor.ProcessRfActivateNtf(nsc->CmdProcessor.Ctx, &ntf);
                    }
                }
                break;
            }

            case PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_NTF:
            {
                ptx_NSC_RfDeactivate_Ntf_t ntf;
                memset(&ntf, 0, sizeof(ptx_NSC_RfDeactivate_Ntf_t));

                if (PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_NTFLEN == msgLen)
                {
                    status = ptx_NSC_ProcessRfDeactivateNtf(&msg[1], msgLen - 1, &ntf);
                }
                else
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }

                /* Do state transitions and additional data parsing. */
                ptx_NSC_ParseRfDeactivateNtf(nsc, &ntf, &status);

                break;
            }

            case PTX_NSC_CMD_PROCESSOR_ERROR_NTF:
            {
                ptx_NSC_Error_Ntf_t ntf;
                memset(&ntf, 0, sizeof(ptx_NSC_Error_Ntf_t));

                if (PTX_NSC_CMD_PROCESSOR_ERROR_NTFLEN == msgLen)
                {
                    status = ptx_NSC_ProcessErrorNtf(&msg[1], msgLen - 1, &ntf);
                }
                else
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    switch (ntf.Error)
                    {
                        case NSC_ErrorCode_OverTemperature:
                        /* FALL THROUGH */
                        case NSC_ErrorCode_OverCurrent:
                        /* FALL THROUGH */
                        case NSC_ErrorCode_VddTx:
                        /* FALL THROUGH */
                        case NSC_ErrorCode_OTPConfig:
                            /* In case of critical system errors, NSC turns the state into ERROR. */
                            nsc->State = ptx_NSC_State_Error;
                            status = PTX_STATUS_E_NSC_SYSTEM;
                            break;

                        case NSC_ErrorCode_StandbyMode:
                            /* This is sent when NSC turns from RF_DISCOVERY (Standby mode) to RF_IDLE state. */
                            nsc->State = ptx_NSC_State_Idle;
                            break;
                        default:
                            /* Other errors are just forwarded up. */
                            break;
                    }

                    if (NULL != nsc->CmdProcessor.ProcessErrorNtf)
                    {
                        status = nsc->CmdProcessor.ProcessErrorNtf(nsc->CmdProcessor.Ctx, &ntf);
                    }
                }
                break;
            }
#if !defined(PTX_DISABLE_WLC)
            case PTX_NSC_CMD_PROCESSOR_WPT_STOP_NTF:
            {
                ptx_NSC_WLC_WptStop_Ntf_t ntf;
                memset(&ntf, 0, sizeof(ptx_NSC_WLC_WptStop_Ntf_t));

                if (PTX_NSC_CMD_PROCESSOR_WPT_STOP_NTFLEN == msgLen)
                {
                    status = ptx_NSC_WLC_ProcessWptStopNtf(&msg[1], msgLen - 1, &ntf);
                }
                else
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    if (ptx_NSC_State_Wpt_Dep == nsc->State)
                    {
                        nsc->State = ptx_NSC_State_Poll_Dep;
                        status = PTX_STATUS_SUCCESS;
                    }
                    else if (ptx_NSC_State_Wpt == nsc->State)
                    {
                        nsc->State = ptx_NSC_State_Poll;
                        status = PTX_STATUS_SUCCESS;
                    }
                    else
                    {
                        status = PTX_STATUS_E_NSC_SYSTEM;
                    }

                    if (NULL != nsc->CmdProcessor.ProcessWptStopNtf)
                    {
                        if (PTX_STATUS_SUCCESS == status)
                        {
                            status = nsc->CmdProcessor.ProcessWptStopNtf(nsc->CmdProcessor.Ctx, &ntf);
                        }
                        else
                        {
                            /*
                                If status is not success here it was caused by invalid nsc->State.
                                We still call ProcessWptStopNtf for the application, as a valid one was received,
                                but return the first error that occurred.
                            */
                            (void) nsc->CmdProcessor.ProcessWptStopNtf(nsc->CmdProcessor.Ctx, &ntf);
                        }
                    }
                }
                break;
            }
#endif
            case PTX_NSC_CMD_PROCESSOR_DBG_NTF:
            {
                if ((PTX_NSC_LENGTH_MAX >= msgLen) && (PTX_NSC_CMD_OPCODE_LEN <= msgLen))
                {
                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }
                break;
            }
            default:
            {
                status = PTX_STATUS_E_NSC_PROTOCOL;
                break;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_CmdProcessor_ProcessDataMsg(ptx_NSC_t *nsc, uint8_t *msg, uint32_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != msg) && (PTX_NSC_LENGTH_DATAMSG_MIN <= msgLen))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_CmdProcessor_ProcessDataMsg");

        status = PTX_STATUS_SUCCESS;

        /* Basic pre-processing of Data message */
        ptx_NSC_RfData_Msg_t data_msg;

        memset(&data_msg, 0, sizeof(ptx_NSC_RfData_Msg_t));

        /* FCB */
        if (PTX_NSC_DATA_MSG_FCB_BIT_CTRL == (PTX_NSC_DATA_MSG_FCB_BIT_CTRL & msg[PTX_NSC_DATA_MSG_IDX_FCB]))
        {
            /* It is a Control message */
            data_msg.Fcb.MsgType = ptx_NSC_RfData_Msg_Type_Control;
            data_msg.CtrlLen.CtrlData = msg[PTX_NSC_DATA_MSG_IDX_DATALEN];
        }
        else
        {
            /* It is a regular data message. Still, it does not have to contain any payload. Check this also. */
            data_msg.Fcb.MsgType = ptx_NSC_RfData_Msg_Type_Data;
            data_msg.CtrlLen.DataLen = msg[PTX_NSC_DATA_MSG_IDX_DATALEN];

            if (0 != data_msg.CtrlLen.DataLen)
            {
                data_msg.Data = &msg[PTX_NSC_DATA_MSG_IDX_DATA];
            }
        }

        /* Check chaining bit. Default value is None (0). */
        if (PTX_NSC_DATA_MSG_FCB_BIT_CHAIN == (PTX_NSC_DATA_MSG_FCB_BIT_CHAIN & msg[PTX_NSC_DATA_MSG_IDX_FCB]))
        {
            data_msg.Fcb.Chaining = ptx_NSC_RfData_Msg_Chaining_Used;
        }
        else
        {
            data_msg.Fcb.Chaining = ptx_NSC_RfData_Msg_Chaining_None;
        }

        /* Check if status byte is present. Default value is Not_Present (0). */
        if (PTX_NSC_DATA_MSG_FCB_BIT_STATUS == (PTX_NSC_DATA_MSG_FCB_BIT_STATUS & msg[PTX_NSC_DATA_MSG_IDX_FCB]))
        {
            uint16_t status_byte_index = PTX_NSC_DATA_MSG_IDX_DATALEN + 1;
            data_msg.Fcb.StatusPresent = ptx_NSC_RfData_Msg_StatusBy_Present;

            if (ptx_NSC_RfData_Msg_Type_Control == data_msg.Fcb.MsgType)
            {
                data_msg.Status = msg[status_byte_index];
            }
            else
            {
                /* For data message, check the total length also. */
                status_byte_index = (uint16_t)(status_byte_index + (data_msg.CtrlLen.DataLen + 1));
                if (status_byte_index == msgLen)
                {
                    data_msg.Status = msg[status_byte_index - 1];
                }
                else
                {
                    status = PTX_STATUS_E_NSC_INVALID_DATA_LENGTH;
                }
            }
        }
        else
        {
            data_msg.Fcb.StatusPresent = ptx_NSC_RfData_Msg_StatusBy_NotPresent;
        }

        if ((PTX_STATUS_SUCCESS == status) && (NULL != nsc->CmdProcessor.ProcessDataMsg))
        {
            status = nsc->CmdProcessor.ProcessDataMsg(nsc->CmdProcessor.Ctx, &data_msg);
        }
    }

    return status;
}

static void ptx_NSC_ParseRfActivateNtf(ptx_NSC_t *nsc, ptx_NSC_RfActivate_Ntf_t *ntf)
{
    PTX_LOG_V(nsc->Log, "ptx_NSC_ParseRfActivateNtf");

    /* No additional parameter checking here. The function is called only within NTF message handler,
     * after all the parameter checking has already been done.
     */

    /* Check state transitions for Listen mode. */
    
    if ((ptx_NSC_RfTech_Passive_Listen_A <= ntf->RfTech) && (ptx_NSC_RfTech_Passive_Listen_F >= ntf->RfTech))
    {
        if ((ptx_NSC_RfProtocol_IsoDep == ntf->RfProt) || (ptx_NSC_RfProtocol_NfcDep == ntf->RfProt))
        {
            /* It is ISO-DEP or NFC-DEP activation. */
            nsc->State = ptx_NSC_State_Listen_Dep;
        }
        else
        {
            /* It is T2T or T3T activation. */
            nsc->State = ptx_NSC_State_Listen;
        }
    }
    else
    {
        /* Otherwise, we are in Poll mode. */
        if ((ptx_NSC_RfProtocol_IsoDep == ntf->RfProt) || (ptx_NSC_RfProtocol_NfcDep == ntf->RfProt))
        {
            nsc->State = ptx_NSC_State_Poll_Dep;
        }
        else
        {
            nsc->State = ptx_NSC_State_Poll;
        }
    }

    if ((ptx_NSC_RfProtocol_IsoDep == ntf->RfProt) && (ptx_NSC_RfTech_Passive_Poll_A == ntf->RfTech))
    {
        switch (ntf->RfProtParams.IsoDep.PollA.FSCI)
        {
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_0:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_0;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_1:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_1;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_2:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_2;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_3:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_3;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_4:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_4;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_5:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_5;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_6:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_6;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_7:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_7;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_8:
            /* FALLTHROUGH */
            default:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_MAX;
                break;
        }
    }
    else if ((ptx_NSC_RfProtocol_IsoDep == ntf->RfProt) && (ptx_NSC_RfTech_Passive_Poll_B == ntf->RfTech))
    {
        switch ((ntf->RfTechParams.PollB.SensBRes[PTX_NSC_RF_DISCOVER_PROTINFO1_OFFSET] & PTX_NSC_RF_DISCOVER_PINFO1_FSCI_MASK) >> 4)
        {
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_0:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_0;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_1:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_1;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_2:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_2;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_3:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_3;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_4:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_4;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_5:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_5;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_6:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_6;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_7:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_7;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSCI_VAL_8:
            /* FALLTHROUGH */
            default:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_MAX;
                break;
        }
    }
    else if ((ptx_NSC_RfProtocol_IsoDep == ntf->RfProt) && (ptx_NSC_RfTech_Passive_Listen_A == ntf->RfTech))
    {
        switch ((ntf->RfProtParams.IsoDep.ListenA.Rats & PTX_NSC_RF_DISCOVER_RATS_FSDI_MASK) >> 4)
        {
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_0:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_0;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_1:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_1;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_2:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_2;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_3:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_3;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_4:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_4;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_5:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_5;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_6:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_6;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_7:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_7;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_8:
            /* FALLTHROUGH */
            default:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_MAX;
                break;
        }
    }

    else if ((ptx_NSC_RfProtocol_IsoDep == ntf->RfProt) && (ptx_NSC_RfTech_Passive_Listen_B == ntf->RfTech))
    {
        switch (ntf->RfProtParams.IsoDep.ListenB.Attrib[PTX_NSC_RF_DISCOVER_PARAM2_OFFSET] & PTX_NSC_RF_DISCOVER_ATRRIB_FSDI_MASK)
        {
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_0:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_0;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_1:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_1;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_2:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_2;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_3:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_3;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_4:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_4;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_5:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_5;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_6:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_6;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_7:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_7;
                break;
            case PTX_NSC_RFPROT_PARAMS_FSDI_VAL_8:
            /* FALLTHROUGH */
            default:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSDI_MTU_SIZE_MAX;
                break;
        }
    }
    else if (ptx_NSC_RfProtocol_NfcDep == ntf->RfProt)
    {
        /*Setting MTU size depending on the RF tech*/

        uint8_t mtu_size = 0x00;
        if (ptx_NSC_RfTech_Active_Listen == ntf->RfTech)
        {
            mtu_size = ntf->RfProtParams.NfcDep.PollAF.AtrRes[16];
        }
        else if (ptx_NSC_RfTech_Active_Poll == ntf->RfTech || ptx_NSC_RfTech_Passive_Listen_F == ntf->RfTech || ptx_NSC_RfTech_Passive_Listen_A == ntf->RfTech)
        {
            mtu_size = ntf->RfProtParams.NfcDep.ListenAF.Lr;
        }
        else
        {
            mtu_size = 0xFF;
        }
        switch ((mtu_size & PTX_NSC_RF_DISCOVER_LISTEN_LR_MASK))
        {
            case PTX_NSC_CMD_PROCESSOR_LR_VAL_0:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_CALC_MTU_SIZE(PTX_NSC_CMD_PROCESSOR_LR_SIZE_64);
                break;

            case PTX_NSC_CMD_PROCESSOR_LR_VAL_1:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_CALC_MTU_SIZE(PTX_NSC_CMD_PROCESSOR_LR_SIZE_128);
                break;

            case PTX_NSC_CMD_PROCESSOR_LR_VAL_2:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_CALC_MTU_SIZE(PTX_NSC_CMD_PROCESSOR_LR_SIZE_192);
                break;

            case PTX_NSC_CMD_PROCESSOR_LR_VAL_3:
            /* FALLTHROUGH */
            default:
                nsc->MtU = PTX_NSC_CMD_PROCESSOR_CALC_MTU_SIZE(PTX_NSC_CMD_PROCESSOR_LR_SIZE_MAX);
                break;
        }
    }
    else
    {
        nsc->MtU = PTX_NSC_MAX_TRANSFER_UNIT_SIZE;
    }
}

static void ptx_NSC_ParseRfDeactivateNtf(ptx_NSC_t *nsc, ptx_NSC_RfDeactivate_Ntf_t *ntf, ptx_Status_t *st)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    
    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != ntf) && (NULL !=  st))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_ParseRfDeactivateNtf");

        status = *st;

        if (PTX_STATUS_SUCCESS == status)
        {

            if ((ptx_NSC_State_Listen == nsc->State) || (ptx_NSC_State_Listen_Dep == nsc->State))
            {
                if (ptx_NSC_RfDeactivate_Reason_RF_Field == ntf->Reason)
                {
                    nsc->State = ptx_NSC_State_Discovery;
                }
                else if (ptx_NSC_RfDeactivate_Reason_Release == ntf->Reason)
                {
                    nsc->State = ptx_NSC_State_Listen_Idle;
                }
                else if (ptx_NSC_RfDeactivate_Reason_Deselect == ntf->Reason)
                {
                    nsc->State = ptx_NSC_State_Listen_Sleep;
                }
                else
                {
                    nsc->State = nsc->PendingState;
                }
            }
            else
            {
                nsc->State = nsc->PendingState;
            }

            nsc->MtU = PTX_NSC_CMD_PROCESSOR_FSCI_MTU_SIZE_MAX;

            if (NULL != nsc->CmdProcessor.ProcessRfDeactivateNtf)
            {
                status = nsc->CmdProcessor.ProcessRfDeactivateNtf(nsc->CmdProcessor.Ctx, ntf);
            }
        }
        else if (PTX_STATUS_E_NSC_RFERROR == status)
        {
            /* NTF ErrorCode is an error value, but NTF message is a proper one.
            *  Notify upper layer about it.
            */

            /* We SHALL not overwrite the previously set error status. Just make a call to CB handler */
            if (NULL != nsc->CmdProcessor.ProcessRfDeactivateNtf)
            {
                (void)nsc->CmdProcessor.ProcessRfDeactivateNtf(nsc->CmdProcessor.Ctx, ntf);
            }
        }
        else
        {
            /* Nothing to do. Error is propagated up. */
        }

        *st = status;
    }
}
