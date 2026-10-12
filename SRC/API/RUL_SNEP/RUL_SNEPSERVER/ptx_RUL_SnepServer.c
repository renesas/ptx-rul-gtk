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
    Module      : SNEP
    File        : ptx_RUL_SnepServer.c

    Description : Implementation of the SNEP server.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL_SnepServer.h"

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

static ptx_Status_t ptx_RUL_SnepServer_Start(ptx_RUL_SnepServer_t *ctx);
static ptx_Status_t ptx_RUL_SnepServer_Process(ptx_RUL_SnepServer_t *ctx);
static ptx_Status_t ptx_RUL_SnepServer_ProcessReceiveRequest(ptx_RUL_SnepServer_t *ctx);
static ptx_Status_t ptx_RUL_SnepServer_ProcessReceiveFragment(ptx_RUL_SnepServer_t *ctx, uint8_t *data, uint32_t *dataLength);
static ptx_Status_t ptx_RUL_SnepServer_ProcessSendMessage(ptx_RUL_SnepServer_t *ctx);
static ptx_Status_t ptx_RUL_SnepServer_ProcessSendFragment(ptx_RUL_SnepServer_t *ctx);
static ptx_Status_t ptx_RUL_SnepServer_ProcessRequest(ptx_RUL_SnepServer_t *ctx);
static ptx_Status_t ptx_RUL_SnepServer_MapLlcpStatus(ptx_Status_t llcpStatus);
static void ptx_RUL_SnepServer_SetMessage(ptx_RUL_SnepServer_t *ctx, ptx_RUL_SnepServer_ResponseCode_t code, uint8_t *information, uint32_t informationLength, uint32_t totalLength);
static void ptx_RUL_SnepServer_SetControlMessage(ptx_RUL_SnepServer_t *ctx, ptx_RUL_SnepServer_ResponseCode_t code);
static uint32_t ptx_RUL_SnepServer_GetBytesToProcess(ptx_RUL_SnepServer_t *ctx, uint32_t nrBytesToProcess, uint32_t nrBytesProcessed, uint8_t isGetRequest);
static uint32_t ptx_RUL_SnepServer_RetrieveLength(uint8_t *data);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_RUL_SnepServer_Init(ptx_RUL_SnepServer_t *ctx, ptx_RUL_LLCP_t *llcp, uint8_t enableDTA)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && PTX_COMP_CHECK(llcp, Comp_LLCP))
    {
        ctx->Llcp = llcp;
        status = ptx_RUL_LLCP_EnableSnep(ctx->Llcp);
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_Init");


        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->CompId = Comp_SNEP;

            /* Initialize the internal state variables. */
            ctx->TxContinuationMode = 0;
            ctx->RxContinuationMode = 0;
            ctx->LastSnepStatus = PTX_STATUS_SUCCESS;

            /* Check if we need to run the real world scenario or NFC Forum DTA mode. */
            ctx->ExtendedDtaMode = (enableDTA == 0 ? 0 : 1);

            /* Clear the buffer and set the maximum size. */
            memset(ctx->MsgBuffer, 0, PTX_SNEPSERVER_SENDBUFFER_SIZE);
            ctx->MsgBufferLen = PTX_SNEPSERVER_SENDBUFFER_SIZE;

            /* Initialize the buffer index to zero. */
            ctx->MsgBufferIndex = 0;

            status = ptx_RUL_SnepServer_Start(ctx);
        }

        if (PTX_STATUS_SUCCESS != status)
        {
            ctx->CompId = Comp_MAX;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_Start(ptx_RUL_SnepServer_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_Start");

        /* Allocate a link from the LLCP component. */
        status = ptx_RUL_LLCP_AllocateLink(ctx->Llcp, &ctx->Link);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Setup the Link with the required values for SNEP and register it. */
            ptx_RUL_LLCP_Link_InitParams_t init_params;
            memset(&init_params, 0, sizeof(ptx_RUL_LLCP_Link_InitParams_t));

            init_params.Type        = LinkType_ConnectionOriented;
            init_params.Ssap        = PTX_SNEPSERVER_DEFAULT_ADDRESS;
            init_params.Window      = PTX_SNEPSERVER_DEFAULT_RW;
            init_params.Miux        = PTX_SNEPSERVER_DEFAULT_MIUX;
            if (0 != ctx->ExtendedDtaMode)
            {
                init_params.ServiceUri = PTX_SNEP_SERVICE_URI_DTA;
                init_params.ServiceUriLen = strlen(PTX_SNEP_SERVICE_URI_DTA);
            }
            else
            {
                init_params.ServiceUri = PTX_SNEP_SERVICE_URI;
                init_params.ServiceUriLen = strlen(PTX_SNEP_SERVICE_URI);
            }

            status = ptx_RUL_LLCP_Link_Register(ctx->Link, &init_params);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->Running = 1;
            ctx->ServerState = ServerState_ReceiveRequest;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_Stop(ptx_RUL_SnepServer_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_Stop");

        ctx->Running = 0;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_DeInit(ptx_RUL_SnepServer_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_DeInit");

        status = ptx_RUL_LLCP_DisableSnep(ctx->Llcp);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_LLCP_Link_Reset(ctx->Link);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            memset(ctx, 0, sizeof(ptx_RUL_SnepServer_t));
            status = PTX_STATUS_SUCCESS;
        }

    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_StartDefaultDiscovery(ptx_RUL_SnepServer_t *ctx, uint32_t idleTime)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_StartDefaultDiscovery");

        /**
         * A wrapper function that starts the default RF discovery
         * for SNEP and takes care of the general bytes.
         */
        status = ptx_RUL_LLCP_StartDefaultDiscovery(ctx->Llcp, idleTime);
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_WaitForActivation(ptx_RUL_SnepServer_t *ctx, uint8_t *activated, uint32_t timeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_WaitForActivation");

        /**
         * A wrapper function waiting for a defined amount of time
         * until the peer has been activated
         */
        status = ptx_RUL_LLCP_WaitForActivation(ctx->Llcp, activated, timeout);
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_WaitForClient(ptx_RUL_SnepServer_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_WaitForClient");

        status = ptx_RUL_LLCP_WaitForConnection(ctx->Llcp, ctx->Link, ctx->Llcp->ApplicationTimerTimeout);

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->RxContinuationMode = 0;
            ctx->TxContinuationMode = 0;
            ctx->DisconnectPending = 0;
            ctx->ContinueReqHandled = 0;
            ctx->ServerState = ServerState_ReceiveRequest;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_PutMessage(ptx_RUL_SnepServer_t *ctx, uint8_t *message, uint32_t messageLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP) && (NULL != message) && (0 != messageLength))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_PutMessage");

        if (messageLength <= PTX_SNEPSERVER_SENDBUFFER_SIZE)
        {
            memcpy(ctx->MsgBuffer, message, messageLength);
            ctx->MsgBufferIndex = messageLength;
            ctx->TxContinuationMode = 0;
            ctx->RxContinuationMode = 0;
            ctx->ServerState = ServerState_SendMessage;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_GetMessage(ptx_RUL_SnepServer_t *ctx, uint8_t *message, uint32_t *messageLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP) && (NULL != message) && (NULL != messageLength))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_GetMessage");

        if (*messageLength >= ctx->MsgBufferIndex)
        {
            *messageLength = ctx->MsgBufferIndex;
            memcpy(message, ctx->MsgBuffer, *messageLength);
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            *messageLength = 0;
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_Handle(ptx_RUL_SnepServer_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_Handle");

        status = ptx_RUL_LLCP_Handle(ctx->Llcp);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_SnepServer_Process(ctx);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_Process(ptx_RUL_SnepServer_t *ctx)
{
    uint8_t rx_data[PTX_LLCP_FRAME_BUFFER_LEN];
    uint32_t rx_data_len = 0;

    uint8_t is_llcp_down = 0;
    ptx_Status_t status = ptx_RUL_LLCP_IsLinkDown(ctx->Llcp, &is_llcp_down);
    PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_Process");

    if (is_llcp_down || ctx->Link->LinkState == LinkState_Disconnected)
    {
        ctx->ServerState = ServerState_ServerDown;
    }

    if (0 != ctx->Running)
    {
        if (PTX_STATUS_SNEP_CONNECTION_LOSS_ERROR == ctx->LastSnepStatus)
        {
            /* It's allowed by the remote Peer to close the connection to the SNEP-Server - just reset the status and wait for the next incoming Connection-request */
            ctx->LastSnepStatus = PTX_STATUS_SUCCESS;
            ctx->ServerState = ServerState_ServerDown;
        }

        switch (ctx->ServerState)
        {
            case ServerState_ReceiveRequest:
                ctx->LastSnepStatus = ptx_RUL_SnepServer_ProcessReceiveRequest(ctx);
                break;

            case ServerState_ReceiveFragment:
                rx_data_len = sizeof(rx_data);
                ctx->LastSnepStatus = ptx_RUL_SnepServer_ProcessReceiveFragment(ctx, rx_data, &rx_data_len);
                break;

            case ServerState_SendMessage:
                ctx->LastSnepStatus = ptx_RUL_SnepServer_ProcessSendMessage(ctx);
                break;

            case ServerState_SendFragment:
                ctx->LastSnepStatus = ptx_RUL_SnepServer_ProcessSendFragment(ctx);
                break;

            case ServerState_SendDisc:
                ctx->LastSnepStatus = ptx_RUL_LLCP_DisconnectSap(ctx->Llcp, ctx->Link);
                ctx->ServerState = ServerState_ServerDown;
                break;

            default:
                ctx->ServerState = ServerState_ServerDown;
                break;
        }

        /* Mapping of LLCP error codes to dedicated SNEP error codes. */
        ctx->LastSnepStatus = ptx_RUL_SnepServer_MapLlcpStatus(ctx->LastSnepStatus);
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_ProcessReceiveRequest(ptx_RUL_SnepServer_t *ctx)
{
    ptx_Status_t status = ptx_RUL_SnepServer_ProcessRequest(ctx);

    if (PTX_STATUS_LLCP_TIMEOUT == status)
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_ProcessReceiveRequest");

        status = PTX_STATUS_SUCCESS;
        if (0 != ctx->TxContinuationMode)
        {
            ctx->ServerState = ServerState_SendFragment;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_ProcessReceiveFragment(ptx_RUL_SnepServer_t *ctx, uint8_t *data, uint32_t *dataLength)
{
    ptx_Status_t status = ptx_RUL_LLCP_Link_ReadMessageInbox(ctx->Link, data, dataLength);
    PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_ProcessReceiveFragment");
    if (PTX_STATUS_SUCCESS == status)
    {
        if (*dataLength <= ctx->Link->Local.Miu)
        {
            /* Store Fragment */
            memcpy(&ctx->MsgBuffer[ctx->MsgBufferIndex], data, *dataLength);
            ctx->MsgBufferIndex += *dataLength;

            /* NDEF Message complete? */
            if (ctx->MsgBufferIndex == ctx->RequestLen)
            {
                ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_Success);

                ctx->RxContinuationMode = 0;
                ctx->ServerState = ServerState_SendMessage;
            }
        }
        else
        {
            ctx->RxContinuationMode = 0;
            ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_Reject);
            ctx->ServerState = ServerState_SendMessage;
        }
    }
    else
    {
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_ProcessSendMessage(ptx_RUL_SnepServer_t *ctx)
{
    ptx_Status_t status = ptx_RUL_LLCP_SendInfo(ctx->Llcp, ctx->Link, ctx->TxBuffer, (uint16_t) ctx->TxBufferLen, 1);
    PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_ProcessSendMessage");

    switch (status)
    {
        case PTX_STATUS_SUCCESS:
            ctx->ServerState = (!ctx->RxContinuationMode ? ServerState_ReceiveRequest : ServerState_ReceiveFragment);

            if (ctx->DisconnectPending)
            {
                ctx->ServerState = ServerState_SendDisc;
            }
            break;

        case PTX_STATUS_LLCP_CONNECTION_DISCONNECTED_BY_REMOTE:
            ctx->ServerState = ServerState_ServerDown;
            break;

        default:
            status = ptx_RUL_LLCP_DisconnectLink(ctx->Llcp);
            ctx->ServerState = ServerState_ServerDown;
            break;
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_ProcessSendFragment(ptx_RUL_SnepServer_t *ctx)
{
    ptx_Status_t status;

    uint8_t rx_data[PTX_LLCP_FRAME_BUFFER_LEN];
    uint32_t rx_data_len = sizeof(rx_data);
    status = ptx_RUL_LLCP_Link_ReadMessageInbox(ctx->Link, rx_data, &rx_data_len);
    PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_ProcessSendFragment");

    if (status != PTX_STATUS_SUCCESS)
    {
        
        ctx->CurrentNrBytes = ptx_RUL_SnepServer_GetBytesToProcess(ctx, ctx->MsgBufferIndex, ctx->ProcessedBytes, 0);
        ptx_RUL_SnepServer_SetMessage(ctx, ServerResponseCode_Null, &ctx->MsgBuffer[ctx->ProcessedBytes], ctx->CurrentNrBytes, 0);
        ctx->ProcessedBytes += ctx->CurrentNrBytes;
        status = ptx_RUL_LLCP_SendInfo(ctx->Llcp, ctx->Link, ctx->TxBuffer, (uint16_t) ctx->TxBufferLen, 1);

        if (status == PTX_STATUS_SUCCESS)
        {
            if (ctx->ProcessedBytes == ctx->MsgBufferIndex)
            {
                ctx->TxContinuationMode = 0;
            }
        }
        else
        {
            status = ptx_RUL_LLCP_DisconnectLink(ctx->Llcp);
        }

        if (Role_Initiator == ctx->Llcp->Role)
        {
            ctx->ServerState = ServerState_ReceiveRequest;
        }
        else
        {
            ctx->ServerState = ServerState_SendFragment;
        }
    }
    else
    {
        ctx->ServerState = ServerState_SendDisc;
    }
    ctx->StateTick = ctx->Llcp->TimerTicks;

    return status;
}


ptx_Status_t ptx_RUL_SnepServer_ProcessRequest(ptx_RUL_SnepServer_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_ProcessRequest");

        uint8_t rx_data[PTX_LLCP_FRAME_BUFFER_LEN];
        uint32_t rx_data_len = sizeof(rx_data);

        status = ptx_RUL_LLCP_Link_ReadMessageInbox(ctx->Link, rx_data, &rx_data_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->ServerState = ServerState_SendMessage;
            /* Valid request? */
            if (rx_data_len >= PTX_SNEP_FRAME_HEADER_LEN)
            {
                /* Local MIU exceeded? */
                if (rx_data_len <= ctx->Link->Local.Miu)
                {
                    /* Matching version? */
                    if (PTX_SNEPSERVER_DEFAULT_VERSION == (rx_data[0] & PTX_SNEPSERVER_MAJOR_VERSION_MASK))
                    {
                        ctx->RequestLen = ptx_RUL_SnepServer_RetrieveLength(&rx_data[2]);

                        /* rx_data[1] contains the request type. */
                        switch (rx_data[1])
                        {
                            case ServerRequestType_RequestPut:
                                if (ctx->RequestLen <= ctx->MsgBufferLen)
                                {
                                    uint32_t information_len = rx_data_len - PTX_SNEP_FRAME_HEADER_LEN;

                                    /* NDEF Message complete? */
                                    if (information_len == ctx->RequestLen)
                                    {
                                        /* In case of YES, confirm successful operation. */
                                        ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_Success);

                                        memcpy(&ctx->MsgBuffer[0], &rx_data[PTX_SNEP_FRAME_HEADER_LEN], information_len);
                                        ctx->MsgBufferIndex = information_len;
                                    }
                                    else if (information_len > ctx->RequestLen)
                                    {
                                        ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_ExcessData);
                                        ctx->DisconnectPending = 1;
                                    }
                                    else
                                    {
                                        ctx->RxContinuationMode = 1;
                                        memcpy(&ctx->MsgBuffer[0], &rx_data[PTX_SNEP_FRAME_HEADER_LEN], information_len);
                                        ctx->MsgBufferIndex = information_len;
                                        ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_Continue);
                                    }
                                }
                                else
                                {
                                    ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_ExcessData);
                                }
                                break;

                            case ServerRequestType_RequestReject:
                                /* OK ... just receive the next request */
                                ctx->ServerState = ServerState_ReceiveRequest;
                                break;

                            case ServerRequestType_RequestGet:
                                /* GET requested supported only if "Extended DTA Functionality" is enabled. */
                                if (0 != ctx->ExtendedDtaMode)
                                {
                                    /**
                                     * Note: A GET-request from a Client contains a NDEF-Message which defines how the response looks like.
                                     *       But as we're using the GET-request only in the context of NFC Forum certification i.e. extended DTA-functionality,
                                     *       we just pick the first item from the Item-container (if there is any) - this fits to all NFC Forum SNEP tests.
                                     */
                                    if (0 != ctx->MsgBufferIndex)
                                    {
                                        uint32_t acceptable_length = ptx_RUL_SnepServer_RetrieveLength(&rx_data[6]);

                                        /* Does the message fit into the acceptable length of the client? */
                                        if (ctx->MsgBufferIndex <= acceptable_length)
                                        {
                                            ctx->ProcessedBytes = 0;
                                            /** Note: Actually it's possible that the NDEF-message sent via the GET-REQUEST is also separated into fragments, but ...
                                             *        as we're using this again in the context of NFC Forum certification i.e. extended DTA-functionality, this scenario
                                             *        can be omitted because all test cases use an incoming length of 0x29 bytes.
                                             */
                                            if ((ctx->MsgBufferIndex <= (ctx->Link->Remote.Miu - PTX_SNEP_FRAME_HEADER_LEN)))
                                            {
                                                ctx->TxContinuationMode = 0;
                                                ctx->CurrentNrBytes = ctx->MsgBufferIndex;
                                            }
                                            else
                                            {
                                                //ctx->DtaFragmentation = 1;
                                                ctx->TxContinuationMode = 1;
                                                ctx->CurrentNrBytes = ctx->Link->Remote.Miu - PTX_SNEP_FRAME_HEADER_LEN;
                                            }

                                            ptx_RUL_SnepServer_SetMessage(ctx, ServerResponseCode_Success, &ctx->MsgBuffer[0], ctx->CurrentNrBytes, ctx->MsgBufferIndex);
                                            ctx->ProcessedBytes += ctx->CurrentNrBytes;
                                        }
                                        else
                                        {
                                            ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_ExcessData);
                                        }
                                    }
                                    else
                                    {
                                        ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_NotFound);
                                    }
                                }
                                else
                                {
                                    ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_NotImplemented);
                                }
                                break;

                            case ServerRequestType_RequestContinue:
                                ctx->ServerState = ServerState_SendDisc;
                                if ((0 != ctx->ExtendedDtaMode) && (0 != ctx->TxContinuationMode) && (0 == ctx->ContinueReqHandled))
                                {
                                    ctx->ContinueReqHandled = 1;
                                    ctx->ServerState = ServerState_SendFragment;
                                }
                                break;

                            case ServerRequestType_RequestRFU:
                                ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_NotImplemented);
                                ctx->DisconnectPending = 1;
                                break;

                            default:
                                /* Invalid Request received - send Disconnect */
                                ctx->ServerState = ServerState_SendDisc;
                                break;
                        }
                    }
                    else
                    {
                        ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_UnsupportedVersion);
                        ctx->DisconnectPending = 1;
                    }
                }
                else
                {
                    ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_Reject);
                    ctx->DisconnectPending = 1;
                }
            }
            else
            {
                ptx_RUL_SnepServer_SetControlMessage(ctx, ServerResponseCode_BadRequest);
                ctx->DisconnectPending = 1;
            }
        }
        else
        {
            if ((ctx->Llcp->TimerTicks - ctx->StateTick) < ctx->Llcp->TargetLinkTimeout)
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_LLCP_TIMEOUT;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepServer_MapLlcpStatus(ptx_Status_t llcpStatus)
{
    ptx_Status_t status;

    switch (llcpStatus)
    {
        case PTX_STATUS_SUCCESS:
            status = PTX_STATUS_SUCCESS;
            break;

        case PTX_STATUS_LLCP_INVALID_PARAMETER:
            status = PTX_STATUS_SNEP_INVALID_PARAMETER;
            break;

        case PTX_STATUS_LLCP_VERSION_AGREEMENT:
            status =  PTX_STATUS_SNEP_LLCP_NOT_SUPPORTED;
            break;

        case PTX_STATUS_LLCP_DISCONNECTED_BY_REMOTE:
            status = PTX_STATUS_SNEP_LINK_LOSS_ERROR;
            break;

        case PTX_STATUS_LLCP_MAC_DEACTIVATED_BY_REMOTE:
            status = PTX_STATUS_SNEP_LINK_LOSS_ERROR;
            break;

        case PTX_STATUS_LLCP_TIMEOUT:
            status = PTX_STATUS_SNEP_TIMEOUT_ERROR;
            break;

        case PTX_STATUS_LLCP_CONNECTION_DISCONNECTED_BY_REMOTE:
            status = PTX_STATUS_SNEP_CONNECTION_LOSS_ERROR;
            break;

        case PTX_STATUS_LLCP_REMOTE_PEER_BUSY:
        case PTX_STATUS_LLCP_FRMR_EXCHANGED:
            status = PTX_STATUS_SNEP_LLCP_PROTOCOL_ERROR;
            break;

        default:
            status = PTX_STATUS_SNEP_LLCP_UNKNOWN_ERROR;
            break;
    }

    return status;
}

void ptx_RUL_SnepServer_SetMessage(ptx_RUL_SnepServer_t *ctx, ptx_RUL_SnepServer_ResponseCode_t code, uint8_t *information, uint32_t informationLength, uint32_t totalLength)
{
    ctx->TxBufferLen = 0;
    PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_SetMessage");

    if (ServerResponseCode_Null != code)
    {
        /* Set the implemented SNEP version. */
        ctx->TxBuffer[ctx->TxBufferLen] = PTX_SNEPSERVER_DEFAULT_VERSION;
        ++ctx->TxBufferLen;

        /* Set the request code. */
        ctx->TxBuffer[ctx->TxBufferLen] = code;
        ++ctx->TxBufferLen;

        /* Set the maximum acceptable length (4 bytes). */
        ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((totalLength >> 24) & 0xFF);
        ++ctx->TxBufferLen;
        ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((totalLength >> 16) & 0xFF);
        ++ctx->TxBufferLen;
        ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((totalLength >> 8) & 0xFF);
        ++ctx->TxBufferLen;
        ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((totalLength >> 0) & 0xFF);
        ++ctx->TxBufferLen;
    }

    if ((NULL != information) && (0 != informationLength))
    {
        /* Append the actual information / payload. */
        memcpy(&ctx->TxBuffer[ctx->TxBufferLen], information, informationLength);
        ctx->TxBufferLen += informationLength;
    }
}

void ptx_RUL_SnepServer_SetControlMessage(ptx_RUL_SnepServer_t *ctx, ptx_RUL_SnepServer_ResponseCode_t code)
{    
    PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_SetControlMessage");

    /* Just a wrappper for ptx_RUL_SnepClient_SetMessage() since everything other than the request code is NULL / zero. */
    ptx_RUL_SnepServer_SetMessage(ctx, code, NULL, 0, 0);
}

uint32_t ptx_RUL_SnepServer_GetBytesToProcess(ptx_RUL_SnepServer_t *ctx, uint32_t nrBytesToProcess, uint32_t nrBytesProcessed, uint8_t isGetRequest)
{
    uint32_t new_nr_bytes_to_process = (ctx->Link->Remote.Miu < ctx->Link->Local.Miu ? ctx->Link->Remote.Miu : ctx->Link->Local.Miu);

    uint32_t snep_header_length = 0;
    PTX_LOG_I(ctx->Log, "ptx_RUL_SnepServer_GetBytesToProcess");

    if (!ctx->TxContinuationMode)
    {
        snep_header_length = (!isGetRequest ? PTX_SNEP_FRAME_HEADER_LEN : (PTX_SNEP_FRAME_HEADER_LEN + PTX_SNEPSERVER_LENGTH_LENGTH));

        if ((nrBytesToProcess - nrBytesProcessed - snep_header_length) <= (uint32_t)(PTX_LLCP_DEFAULT_MIU + ctx->Link->Remote.Miu))
        {
            new_nr_bytes_to_process = nrBytesToProcess - nrBytesProcessed;
        }
    }
    else
    {
        if ((nrBytesToProcess - nrBytesProcessed) <=  (uint32_t)(ctx->Link->Remote.Miu))
        {
            new_nr_bytes_to_process = nrBytesToProcess - nrBytesProcessed;
        }
    }

    return new_nr_bytes_to_process;
}

uint32_t ptx_RUL_SnepServer_RetrieveLength(uint8_t *data)
{
    /* Helper function to parse the length information from a received message*/
    uint8_t index = 0;
    uint32_t length = 0;

    /* The length information consists of 4 bytes (MSB first)*/
    length |= data[index] << 24;
    ++index;
    length |= data[index] << 16;
    ++index;
    length |= data[index] << 8;
    ++index;
    length |= data[index] << 0;

    return length;
}
