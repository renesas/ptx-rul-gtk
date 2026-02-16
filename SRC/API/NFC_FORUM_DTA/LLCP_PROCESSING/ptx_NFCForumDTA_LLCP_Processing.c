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
    Module      : NFC Forum Device Test Application (DTA) LLCP Processing
    File        : ptx_NFCForumDTA_LLCP_Processing.c

    Description : Contains the implementation to handle LLCP in the NFC Forum DTA. It controls the processing if the
                  NFC Forum DTA is running in LLCP Mode for P2P communication.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NFCForumDTA_LLCP_Processing.h"

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NFCForumDTA_LLCP_ProcessCOScenario(ptx_RUL_LLCP_DTA_t *dta);
static ptx_Status_t ptx_NFCForumDTA_LLCP_ProcessCLScenario(ptx_RUL_LLCP_DTA_t *dta);
static ptx_Status_t ptx_NFCForumDTA_LLCP_ProcessLlcpOperation(ptx_RUL_LLCP_DTA_t *dta);

ptx_Status_t ptx_NFCForumDTA_LLCP_ProcessCOScenario(ptx_RUL_LLCP_DTA_t *dta)
{
    dta->QuitScenario = 0;

    if (dta->SetupBySap)
    {
        dta->CoScenarioState = CO_SCENARIO_STATE_CONNECT_VIA_SAP;
    }
    else if (dta->SetupByName)
    {
        dta->CoScenarioState = CO_SCENARIO_STATE_CONNECT_VIA_NAME;
    }
    else if (dta->SetupBySnl)
    {
        dta->CoScenarioState = CO_SCENARIO_STATE_CONNECT_VIA_SNL;
    }
    else
    {
        dta->LlcpStatus = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    ptx_RUL_LLCP_MessageBuffer_t *message_buffer = &(dta->CoInbound->Local.MessageBuffer);

    while ((0 == dta->QuitScenario) && (PTX_STATUS_SUCCESS == dta->LlcpStatus) && (0 == dta->Dta->quitDTA))
    {
        dta->Dta->applicationCallback(dta->Dta);

        uint8_t is_down = 0;
        ptx_RUL_LLCP_IsLinkDown(dta->Llcp, &is_down);

        if (0 != is_down)
        {
            /* If the link is down - just quit scenario - remaining tasks (if any) done by caller (e.g. MAC-Link deactivation) */
            dta->LlcpStatus = PTX_STATUS_E_GEN_INTERNAL;
            continue;
        }

        /* Has any frame on Tx- or Rx-direction been rejected? */
        if ((LinkState_FrameRejected == dta->CoInbound->LinkState) ||
            (LinkState_FrameRejected == dta->CoOutbound->LinkState))
        {
            dta->LlcpStatus = PTX_STATUS_LLCP_FRMR_EXCHANGED;
            continue;
        }

        switch (dta->CoScenarioState)
        {
            case CO_SCENARIO_STATE_CONNECT_VIA_SAP:
                dta->LlcpStatus = ptx_RUL_LLCP_ConnectSap(dta->Llcp, dta->CoOutbound, PTX_NFCFORUMDTA_LLCP_INBOUND_CO_SAP_ADDRESS);
                if (PTX_STATUS_SUCCESS == dta->LlcpStatus)
                {
                    dta->CoScenarioState = CO_SCENARIO_STATE_RECEIVE_I_PDU;
                }
                else
                {
                    /* Reset status and trigger CONNECT-attempt error handling. */
                    dta->LlcpStatus = PTX_STATUS_SUCCESS;
                    dta->CoScenarioState = CO_SCENARIO_STATE_CONNECT_FAIL;
                }
                break;

            case CO_SCENARIO_STATE_CONNECT_VIA_NAME:
                dta->LlcpStatus = ptx_RUL_LLCP_ConnectSapByName(dta->Llcp, dta->CoOutbound, PTX_NFCFORUMDTA_LLCP_OUTBOUND_CO_ECHO_SERVICE_NAME);
                if (PTX_STATUS_SUCCESS == dta->LlcpStatus)
                {
                    dta->CoScenarioState = CO_SCENARIO_STATE_RECEIVE_I_PDU;
                }
                else
                {
                    /* Reset status and trigger CONNECT-attempt error handling. */
                    dta->LlcpStatus = PTX_STATUS_SUCCESS;
                    dta->CoScenarioState = CO_SCENARIO_STATE_RECEIVE_I_PDU;
                }
                break;

            case CO_SCENARIO_STATE_CONNECT_VIA_SNL:
            {
                ptx_RUL_LLCP_Parameter_ServiceName_t service_name;
                service_name.Name = PTX_NFCFORUMDTA_LLCP_OUTBOUND_CO_ECHO_SERVICE_NAME;
                service_name.Length = strlen(PTX_NFCFORUMDTA_LLCP_OUTBOUND_CO_ECHO_SERVICE_NAME);

                uint8_t destination_sap = 0;
                dta->LlcpStatus = ptx_RUL_LLCP_DiscoverService(dta->Llcp, &service_name, &destination_sap);

                if (PTX_STATUS_SUCCESS == dta->LlcpStatus)
                {

                    dta->LlcpStatus = ptx_RUL_LLCP_ConnectSap(dta->Llcp, dta->CoOutbound, destination_sap);
                    if (PTX_STATUS_SUCCESS == dta->LlcpStatus)
                    {
                        dta->CoScenarioState = CO_SCENARIO_STATE_RECEIVE_I_PDU;
                    }
                    else
                    {
                        /* Reset status and trigger CONNECT-attempt error handling */
                        dta->LlcpStatus = PTX_STATUS_SUCCESS;
                        dta->CoScenarioState = CO_SCENARIO_STATE_CONNECT_FAIL;
                    }
                }
                break;
            }
            case CO_SCENARIO_STATE_CONNECT_FAIL:
                if (LinkState_Connected == dta->CoInbound->LinkState)
                {
                    /* Send DM to disconnect InBound-Application. */
                    dta->LlcpStatus = ptx_RUL_LLCP_DisconnectSap(dta->Llcp, dta->CoInbound);
                    dta->QuitScenario = 1;
                }
                break;

            case CO_SCENARIO_STATE_RECEIVE_I_PDU:
                dta->LlcpStatus = ptx_RUL_LLCP_ReceiveFromLink(dta->Llcp, dta->CoInbound);

                /* Valid case - may happen that nothing gets received - just reset the status and continue processing. */
                if (dta->LlcpStatus == PTX_STATUS_LLCP_TIMEOUT)
                {
                    dta->LlcpStatus = PTX_STATUS_SUCCESS;
                }

                if (PTX_STATUS_SUCCESS == dta->LlcpStatus)
                {
                    uint32_t size = 0;
                    ptx_RUL_LLCP_MessageBuffer_Size(&dta->CoInbound->Local.MessageBuffer, &size);
                    /* Any I-PDU received in the InBound Message-Box? */
                    if (0 != size)
                    {
                        dta->CoScenarioState = CO_SCENARIO_STATE_SEND_I_PDU;
                    }
                }

                break;

            case CO_SCENARIO_STATE_SEND_I_PDU:
            {
                uint32_t start_time = dta->Llcp->TimerTicks;
                uint32_t timeout = PTX_NFCFORUMDTA_LLCP_T_DELAY_CO;
                while ((((dta->Llcp->TimerTicks - start_time) * PTX_LLCP_DEFAULT_TIMER_TICK_UNIT) < timeout) && (0 == dta->Dta->quitDTA))
                {
                    dta->LlcpStatus = ptx_RUL_LLCP_Handle(dta->Llcp);
                    dta->Dta->applicationCallback(dta->Dta);

                    if ((LinkState_FrameRejected == dta->CoInbound->LinkState) ||
                        (LinkState_FrameRejected == dta->CoOutbound->LinkState))
                    {
                        dta->LlcpStatus = PTX_STATUS_LLCP_FRMR_EXCHANGED;
                        break;
                    }
                }

                if (PTX_STATUS_LLCP_FRMR_EXCHANGED != dta->LlcpStatus)
                {
                    uint32_t messages = 0;
                    ptx_RUL_LLCP_MessageBuffer_Size(message_buffer, &messages);

                    /* Send all I-PDUs in the Inbound Message-Box via the OutBound Message-Box */
                    while ((0 != messages)  && (0 == dta->Dta->quitDTA))
                    {
                        dta->Dta->applicationCallback(dta->Dta);

                        uint8_t payload[PTX_LLCP_MESSAGEBUFFER_MESSAGELENGTH];
                        uint32_t payload_length = sizeof(payload);

                        ptx_RUL_LLCP_MessageBuffer_Read(message_buffer, payload, &payload_length, 0);

                        dta->LlcpStatus = ptx_RUL_LLCP_SendInfo(dta->Llcp, dta->CoOutbound, payload, (uint16_t) payload_length, 0);

                        if (PTX_STATUS_SUCCESS == dta->LlcpStatus)
                        {
                            payload_length = sizeof(payload);
                            /* I-PDU sent - remove the item */
                            ptx_RUL_LLCP_MessageBuffer_Read(message_buffer, payload, &payload_length, 1);
                            ptx_RUL_LLCP_MessageBuffer_Size(message_buffer, &messages);
                        }
                        else
                        {
                            /* Cancel processing */
                            break;
                        }
                    }
                }

                switch (dta->LlcpStatus)
                {
                    /* REMOTE-BUSY-Error is returned when remote is busy - just keep keep on receiving until the condition is cleared */
                    case PTX_STATUS_LLCP_REMOTE_PEER_BUSY: /* Fall through */
                    /* RW of peer filled up - just give it more time to send a RR */
                    case PTX_STATUS_LLCP_REMOTE_RW_FULL:
                        dta->LlcpStatus = PTX_STATUS_SUCCESS;
                        dta->CoScenarioState = CO_SCENARIO_STATE_RECEIVE_I_PDU;
                        break;

                    default:
                        /* Let the while-loop conditions handle it (in case necessary) */
                        dta->CoScenarioState = CO_SCENARIO_STATE_RECEIVE_I_PDU;
                        break;
                }
                break;
            }
            default:
                dta->LlcpStatus = PTX_STATUS_E_GEN_INTERNAL;
                break;
        }
    }

    /* Post-processing */
    switch (dta->LlcpStatus)
    {
        case PTX_STATUS_LLCP_FRMR_EXCHANGED:
        {
            uint32_t start_time = dta->Llcp->TimerTicks;
            uint32_t timeout = 300;
            while ((((dta->Llcp->TimerTicks - start_time) * PTX_LLCP_DEFAULT_TIMER_TICK_UNIT) < timeout) && (0 == dta->Dta->quitDTA))
            {
                dta->LlcpStatus = ptx_RUL_LLCP_Handle(dta->Llcp);
                dta->Dta->applicationCallback(dta->Dta);
            }
            dta->LlcpStatus = ptx_RUL_LLCP_DisconnectSap(dta->Llcp, dta->CoOutbound);
        }
        break;

        case PTX_STATUS_SUCCESS: /* Fall through */
        case PTX_STATUS_LLCP_CONNECTION_DISCONNECTED_BY_REMOTE:
            /*
             * DM received on InBound-Application?
             */
            switch (dta->CoInbound->LinkState)
            {
                case LinkState_FrameRejected: /* Fall through */
                    dta->LlcpStatus = ptx_RUL_LLCP_DisconnectSap(dta->Llcp, dta->CoOutbound);
                    break;
                case LinkState_Disconnected:
                    /*
                     * Send DM to disconnect Out-Application.
                     */
                {
                    dta->LlcpStatus = ptx_RUL_LLCP_DisconnectSap(dta->Llcp, dta->CoOutbound);

                    uint32_t start_time = dta->Llcp->TimerTicks;
                    uint32_t timeout = (dta->Llcp->Role == Role_Initiator ? 1000 : 300);
                    while ((((dta->Llcp->TimerTicks - start_time) * PTX_LLCP_DEFAULT_TIMER_TICK_UNIT) < timeout) && (0 == dta->Dta->quitDTA))
                    {
                        dta->LlcpStatus = ptx_RUL_LLCP_Handle(dta->Llcp);
                        dta->Dta->applicationCallback(dta->Dta);
                    }
                }
                break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return dta->LlcpStatus;
}


ptx_Status_t ptx_NFCForumDTA_LLCP_ProcessCLScenario(ptx_RUL_LLCP_DTA_t *dta)
{
    dta->QuitScenario = 0;
    dta->ClScenarioState = CL_SCENARIO_STATE_CHECK_TEST_START_CMD;

    ptx_RUL_LLCP_MessageBuffer_t *message_buffer = NULL;
    uint8_t dsap = 0;

    while ((0 == dta->QuitScenario) && (PTX_STATUS_SUCCESS == dta->LlcpStatus)  && (0 == dta->Dta->quitDTA))
    {
        dta->Dta->applicationCallback(dta->Dta);

        uint8_t is_down;
        ptx_RUL_LLCP_IsLinkDown(dta->Llcp, &is_down);

        if (0 != is_down)
        {
            /* If the link is down - just quit scenario - remaining tasks (if any) done by caller (e.g. MAC-Link deactivation). */
            dta->LlcpStatus = PTX_STATUS_E_GEN_INTERNAL;
            continue;
        }

        switch (dta->ClScenarioState)
        {

            case CL_SCENARIO_STATE_CHECK_TEST_START_CMD:
                message_buffer = &(dta->ClInbound->Local.MessageBuffer);

                uint8_t data[PTX_LLCP_MESSAGEBUFFER_MESSAGELENGTH];
                uint32_t data_len = sizeof(data);
                dta->LlcpStatus = ptx_RUL_LLCP_MessageBuffer_Read(message_buffer, data, &data_len, 1u);

                uint8_t dta_echo_test_cl_sot[] = {0x53, 0x4F, 0x54};

                if (0 == memcmp(data, dta_echo_test_cl_sot, data_len))
                {
                    dta->ClScenarioState = CL_SCENARIO_STATE_SEND_SNL_REQUEST;
                }
                else
                {
                    /* Error - First UI-PDU received is not SOT. */
                    dta->LlcpStatus = PTX_STATUS_E_GEN_INTERNAL;
                }
                break;

            case CL_SCENARIO_STATE_SEND_SNL_REQUEST:
            {
                /* Send SNL to peer. */
                ptx_RUL_LLCP_Parameter_ServiceName_t service_name;
                service_name.Name = PTX_NFCFORUMDTA_LLCP_OUTBOUND_CL_ECHO_SERVICE_NAME;
                service_name.Length = strlen(PTX_NFCFORUMDTA_LLCP_OUTBOUND_CL_ECHO_SERVICE_NAME);

                dta->LlcpStatus = ptx_RUL_LLCP_DiscoverService(dta->Llcp, &service_name, &dsap);

                if (dta->LlcpStatus == PTX_STATUS_SUCCESS)
                {
                    /* Set DSAP of OutBound-Application. */
                    dta->ClScenarioState = CL_SCENARIO_STATE_RECEIVE_UI_PDU;
                }
                break;
            }
            case CL_SCENARIO_STATE_RECEIVE_UI_PDU:
                ptx_RUL_LLCP_ReceiveFromLink(dta->Llcp, dta->ClInbound);

                uint32_t r_messages = 0;
                ptx_RUL_LLCP_MessageBuffer_Size(message_buffer, &r_messages);
                /* Any I-PDU received in the InBound Message-Box? */
                if (0 != r_messages)
                {
                    dta->ClScenarioState = CL_SCENARIO_STATE_SEND_UI_PDU;
                }

                break;

            case CL_SCENARIO_STATE_SEND_UI_PDU:
            {
                uint32_t start_time = dta->Llcp->TimerTicks;
                uint32_t timeout = PTX_NFCFORUMDTA_LLCP_T_DELAY_CO;
                while ((((dta->Llcp->TimerTicks - start_time) * PTX_LLCP_DEFAULT_TIMER_TICK_UNIT) < timeout) && (0 == dta->Dta->quitDTA))
                {
                    ptx_RUL_LLCP_Handle(dta->Llcp);
                    dta->Dta->applicationCallback(dta->Dta);
                }

                /* Send all UI-PDUs in the Inbound Message-Box via the OutBound Message-Box. */
                uint32_t messages = 0;
                ptx_RUL_LLCP_MessageBuffer_Size(message_buffer, &messages);

                while ((0 != messages) && (0 == dta->Dta->quitDTA))
                {
                    uint8_t in_data[PTX_LLCP_MESSAGEBUFFER_MESSAGELENGTH];
                    uint32_t in_data_len = sizeof(in_data);
                    ptx_RUL_LLCP_MessageBuffer_Read(message_buffer, in_data, &in_data_len, 0);
                    dta->ClOutbound->Remote.Address = dsap;
                    dta->LlcpStatus = ptx_RUL_LLCP_SendUInfo(dta->Llcp, dta->ClOutbound, in_data, (uint16_t) in_data_len);

                    dta->Dta->applicationCallback(dta->Dta);

                    if (PTX_STATUS_SUCCESS == dta->LlcpStatus)
                    {
                        /* UI-PDU sent - remove the item. */
                        ptx_RUL_LLCP_MessageBuffer_Read(message_buffer, in_data, &in_data_len, 1u);
                        /* Evaluate how many messages are still left. */
                        ptx_RUL_LLCP_MessageBuffer_Size(message_buffer, &messages);
                    }
                    else
                    {
                        /* Cancel processing */
                        break;
                    }
                }

                /* Start next round */
                dta->ClScenarioState = CL_SCENARIO_STATE_RECEIVE_UI_PDU;
                break;
            }
            default:
                dta->LlcpStatus = PTX_STATUS_E_GEN_INTERNAL;
                break;
        }
    }

    /* Post processing */
    switch (dta->LlcpStatus)
    {
        case PTX_STATUS_LLCP_TIMEOUT:
            /* Timeout happened in Target-Mode ? - Just receive again */
            if (Role_Target == dta->Llcp->Role)
            {
                ptx_RUL_LLCP_MessageBuffer_Clear(message_buffer);
                uint32_t start_time = dta->Llcp->TimerTicks;
                while ((((dta->Llcp->TimerTicks - start_time) * PTX_LLCP_DEFAULT_TIMER_TICK_UNIT) < PTX_NFCFORUMDTA_LLCP_POST_PROCESSING_TO) &&
                       (0 == dta->Dta->quitDTA))
                {
                    ptx_RUL_LLCP_Handle(dta->Llcp);
                    dta->Dta->applicationCallback(dta->Dta);
                }
            }
            break;

        default:
            /* Ignore */
            break;
    }

    return dta->LlcpStatus;
}


ptx_Status_t ptx_NFCForumDTA_LLCP_ProcessLlcpOperation(ptx_RUL_LLCP_DTA_t *dta)
{
    ptx_Status_t status = ptx_RUL_LLCP_WaitForAnyConnection(dta->Llcp, PTX_NFCFORUMDTA_LLCP_INITIAL_CONNECT_TO);

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Determine the active Service Access Point */
        if (dta->CoInbound->LinkState == LinkState_Connected)
        {
            /* Connection to Inbound Service Access Point established */
            dta->LlcpStatus = ptx_NFCForumDTA_LLCP_ProcessCOScenario(dta);
        }
        else
        {
            ptx_RUL_LLCP_MessageBuffer_t *message_buffer = &(dta->ClInbound->Local.MessageBuffer);

            uint32_t messages = 0;
            status = ptx_RUL_LLCP_MessageBuffer_Size(message_buffer, &messages);

            if (1u == messages)
            {
                /* UI PDU received on Connection-Less Service Access Point */
                status = ptx_NFCForumDTA_LLCP_ProcessCLScenario(dta);
            }
            else
            {
                /* Shall never happen */
                status = PTX_STATUS_E_GEN_TIMEOUT;
            }
        }
    }
    else if ((PTX_STATUS_LLCP_TIMEOUT == status) || (PTX_STATUS_LLCP_DISCONNECTED_BY_REMOTE == status))
    {
        status = PTX_STATUS_SUCCESS;

        /* Retrieve the current timer tick to properly measure the timeout. */
        uint32_t start_time = dta->Llcp->TimerTicks;
        while (  (PTX_STATUS_SUCCESS == status) &&
                 (((dta->Llcp->TimerTicks - start_time) * PTX_LLCP_DEFAULT_TIMER_TICK_UNIT) < PTX_NFCFORUMDTA_LLCP_POST_PROCESSING_TO) &&
                 (0 == dta->Dta->quitDTA))
        {
            ptx_RUL_LLCP_Handle(dta->Llcp);

            dta->Dta->applicationCallback(dta->Dta);
        }
    }

    return status;
}


ptx_Status_t ptx_NFCForumDTA_RunLlcpTestOperation(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Allocate the LLCP datastructure */
    ptx_RUL_LLCP_DTA_t llcp_dta;
    memset(&llcp_dta, 0, sizeof(ptx_RUL_LLCP_DTA_t));

    /* Setup the datastructure */
    llcp_dta.Llcp = &dta->llcp;
    llcp_dta.Dta = dta;

    /* Retrieve the pointers to the SAPs/Links to be used for DTA test purposes. */
    ptx_RUL_LLCP_GetLinkFromUri(llcp_dta.Llcp, &llcp_dta.ClInbound, 0, PTX_NFCFORUMDTA_LLCP_INBOUND_CL_ECHO_SERVICE_NAME, strlen(PTX_NFCFORUMDTA_LLCP_INBOUND_CL_ECHO_SERVICE_NAME));
    ptx_RUL_LLCP_GetLinkFromUri(llcp_dta.Llcp, &llcp_dta.CoInbound, 0, PTX_NFCFORUMDTA_LLCP_INBOUND_CO_ECHO_SERVICE_NAME, strlen(PTX_NFCFORUMDTA_LLCP_INBOUND_CO_ECHO_SERVICE_NAME));
    ptx_RUL_LLCP_GetLinkFromUri(llcp_dta.Llcp, &llcp_dta.ClOutbound, 0, PTX_NFCFORUMDTA_LLCP_OUTBOUND_CL_ECHO_SERVICE_NAME, strlen(PTX_NFCFORUMDTA_LLCP_OUTBOUND_CL_ECHO_SERVICE_NAME));
    ptx_RUL_LLCP_GetLinkFromUri(llcp_dta.Llcp, &llcp_dta.CoOutbound, 0, PTX_NFCFORUMDTA_LLCP_OUTBOUND_CO_ECHO_SERVICE_NAME, strlen(PTX_NFCFORUMDTA_LLCP_OUTBOUND_CO_ECHO_SERVICE_NAME));

    /* Distinguish between the test patterns. */
    switch (testCfg->PATTERN_NUMBER)
    {
        /* LLCP Connection Oriented Functionality setting up by SAP or Connection-less Functionality. */
        case 0x1200: /* Fall through */
        case 0x1201:
            llcp_dta.SetupBySap  = 1;
            llcp_dta.SetupByName = 0;
            llcp_dta.SetupBySnl  = 0;
            status = ptx_NFCForumDTA_LLCP_ProcessLlcpOperation(&llcp_dta);
            break;

        /* LLCP Connection Oriented Functionality setting-up by name or Connection-less Functionality. */
        case 0x1240: /* Fall through */
        case 0x1241:
            llcp_dta.SetupBySap  = 0;
            llcp_dta.SetupByName = 1;
            llcp_dta.SetupBySnl  = 0;
            status = ptx_NFCForumDTA_LLCP_ProcessLlcpOperation(&llcp_dta);
            break;

        /* LLCP Connection Oriented Functionality setting up by using SNL or Connection-less Functionality. */
        case 0x1280: /* Fall through */
        case 0x1281:
            llcp_dta.SetupBySap  = 0;
            llcp_dta.SetupByName = 0;
            llcp_dta.SetupBySnl  = 1;
            status = ptx_NFCForumDTA_LLCP_ProcessLlcpOperation(&llcp_dta);
            break;

        default:
            break;
    }

    dta->State = ptx_NFCForumDTA_State_FINISHED;

    return status;
}
