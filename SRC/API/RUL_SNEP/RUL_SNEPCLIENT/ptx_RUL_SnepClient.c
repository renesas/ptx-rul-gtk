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
    File        : ptx_RUL_SnepClient.c

    Description : Implementation of the SNEP client.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL_SnepClient.h"

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
static ptx_Status_t ptx_RUL_SnepClient_InitRequest(ptx_RUL_SnepClient_t *ctx, uint8_t *message, uint32_t messageLength, ptx_RUL_SnepClient_RequestType_t requestType);
static ptx_Status_t ptx_RUL_SnepClient_Start(ptx_RUL_SnepClient_t *ctx);
static ptx_Status_t ptx_RUL_SnepClient_Process(ptx_RUL_SnepClient_t *ctx);
static ptx_Status_t ptx_RUL_SnepClient_ProcessState_PrepareRequest(ptx_RUL_SnepClient_t *ctx);
static ptx_Status_t ptx_RUL_SnepClient_ProcessState_SendFragment(ptx_RUL_SnepClient_t *ctx);
static ptx_Status_t ptx_RUL_SnepClient_ProcessReceiveFragment(ptx_RUL_SnepClient_t *ctx);
static ptx_Status_t ptx_RUL_SnepClient_ProcessResponse(ptx_RUL_SnepClient_t *ctx);
static void ptx_RUL_SnepClient_SetControlMessage(ptx_RUL_SnepClient_t *ctx, ptx_RUL_SnepClient_RequestCode_t code);
static void ptx_RUL_SnepClient_SetMessage(ptx_RUL_SnepClient_t *ctx, ptx_RUL_SnepClient_RequestCode_t code, uint8_t *information, uint32_t informationLength, uint32_t messageLength, uint32_t acceptableLength);
static uint32_t ptx_RUL_SnepClient_GetBytesToProcess(ptx_RUL_SnepClient_t *ctx, uint32_t nrBytesToProcess, uint32_t nrBytesProcessed, uint8_t isGetRequest);
static uint32_t ptx_RUL_SnepClient_RetrieveLength(uint8_t *data);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_RUL_SnepClient_Init(ptx_RUL_SnepClient_t *ctx, ptx_RUL_LLCP_t *llcp, uint8_t enableDTA)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && PTX_COMP_CHECK(llcp, Comp_LLCP))
    {
        ctx->Llcp = llcp;

        status = ptx_RUL_LLCP_EnableSnep(ctx->Llcp);

        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_Init");

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->CompId = Comp_SNEP;

            /* Set all the internal state variables to a defined value. */
            ctx->ConnectedToServer = 0;
            ctx->TxContinuationMode = 0;
            ctx->RxContinuationMode = 0;
            ctx->LastSnepResponseCode = 0;

            /* Check if we need to run the real world scenario or NFC Forum DTA mode. */
            ctx->ExtendedDtaMode = (enableDTA == 0 ? 0 : 1);

            /* Initialize the internal state variables. */
            ctx->State = ClientLinkState_LlcpLinkDown;
            ctx->ClientState = ClientState_Idle;

            /* Set the inital status by default to success. */
            ctx->LastSnepStatus = PTX_STATUS_SUCCESS;

            /* Clear the buffer and set the maximum size. */
            memset(ctx->MsgToRecvBuffer, 0, PTX_SNEPCLIENT_RECVEIVEBUFFER_SIZE);
            ctx->MsgToRecvSize = PTX_SNEPCLIENT_RECVEIVEBUFFER_SIZE;

            /* Initialize the buffer index to zero. */
            ctx->MsgToRecvIndex = 0;

            /* Clear the buffer and set the maximum size. */
            memset(ctx->MsgToSendBuffer, 0, PTX_SNEPCLIENT_SENDBUFFER_SIZE);
            ctx->MsgToSendSize = PTX_SNEPCLIENT_SENDBUFFER_SIZE;

            /* Initialize the buffer index to zero. */
            ctx->MsgToSendIndex = 0;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_SnepClient_Start(ctx);
        }

        if (PTX_STATUS_SUCCESS != status)
        {
            ctx->CompId = Comp_MAX;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_DeInit(ptx_RUL_SnepClient_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_DeInit");

        status = ptx_RUL_LLCP_DisableSnep(ctx->Llcp);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_LLCP_Link_Reset(ctx->Link);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            memset(ctx, 0, sizeof(ptx_RUL_SnepClient_t));
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_StartDefaultDiscovery(ptx_RUL_SnepClient_t *ctx, uint32_t idleTime)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_StartDefaultDiscovery");

        /**
         * A wrapper function that starts the default RF discovery
         * for SNEP and takes care of the general bytes.
         */
        status = ptx_RUL_LLCP_StartDefaultDiscovery(ctx->Llcp, idleTime);
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_WaitForActivation(ptx_RUL_SnepClient_t *ctx, uint8_t *activated, uint32_t timeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_WaitForActivation");

        /**
         * A wrapper function waiting for a defined amount of time
         * until the peer has been activated
         */
        status = ptx_RUL_LLCP_WaitForActivation(ctx->Llcp, activated, timeout);
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_ConnectToServer(ptx_RUL_SnepClient_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_ConnectToServer");

        if (0 != ctx->ExtendedDtaMode)
        {
            /* In case of the DTA mode we connect to the extended SNEP DTA server. */
            status = ptx_RUL_LLCP_ConnectSapByName(ctx->Llcp, ctx->Link, PTX_SNEP_SERVICE_URI_DTA);
        }
        else
        {
            /* In the real world scenario connect to the default server. */
            status = ptx_RUL_LLCP_ConnectSap(ctx->Llcp, ctx->Link, PTX_SNEPCLIENT_DEFAULT_ADDRESS);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Setup all the internal variables required for the statemachine to work. */
            ctx->ConnectedToServer = 1;
            ctx->RxContinuationMode = 0;
            ctx->TxContinuationMode = 0;

            /** Prepare the states for the state machine */
            ctx->State = ClientLinkState_ClientActive;
            ctx->ClientState = ClientState_PrepareRequest;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_PutMessage(ptx_RUL_SnepClient_t *ctx, uint8_t *message, uint32_t messageLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP) && (NULL != message))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_PutMessage");
        status = ptx_RUL_SnepClient_InitRequest(ctx, message, messageLength, PutRequest);
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_GetMessage(ptx_RUL_SnepClient_t *ctx, uint8_t *message, uint32_t messageLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    
    if (PTX_COMP_CHECK(ctx, Comp_SNEP) && (NULL != message))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_GetMessage");

        status =  ptx_RUL_SnepClient_InitRequest(ctx, message, messageLength, GetRequest);
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_Handle(ptx_RUL_SnepClient_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_Handle");

        status = ptx_RUL_LLCP_Handle(ctx->Llcp);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_SnepClient_Process(ctx);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_InitRequest(ptx_RUL_SnepClient_t *ctx, uint8_t *message, uint32_t messageLength, ptx_RUL_SnepClient_RequestType_t requestType)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP) && (NULL != message) && (0 != messageLength))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_InitRequest");

        if (0 != ctx->ConnectedToServer)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Check if the buffer size is large enough. */
            if (messageLength <= PTX_SNEPCLIENT_SENDBUFFER_SIZE)
            {
                memcpy(ctx->MsgToSendBuffer, message, messageLength);
                ctx->MsgToSendIndex = messageLength;
                ctx->TxContinuationMode = 0;
                ctx->RxContinuationMode = 0;
                ctx->ProcessedBytes = 0;
                ctx->RequestType = requestType;
                ctx->ClientState = ClientState_PrepareRequest;
            }
            else
            {
                /* The message does not fit into the buffer. */
                status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_Start(ptx_RUL_SnepClient_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_Start");

        /* Allocate a link from the LLCP component. */
        status = ptx_RUL_LLCP_AllocateLink(ctx->Llcp, &ctx->Link);

        /* Check if we were able to allocate the link. */
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Setup the Link with the required values for SNEP and register it. */
            ptx_RUL_LLCP_Link_InitParams_t init_params;
            memset(&init_params, 0, sizeof(ptx_RUL_LLCP_Link_InitParams_t));

            init_params.Type        = LinkType_ConnectionOriented;
            init_params.Ssap        = PTX_SNEPCLIENT_DEFAULT_ADDRESS;
            init_params.Window      = PTX_SNEPCLIENT_DEFAULT_RW;
            init_params.Miux        = PTX_SNEPCLIENT_DEFAULT_MIUX;

            status = ptx_RUL_LLCP_Link_Register(ctx->Link, &init_params);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_Process(ptx_RUL_SnepClient_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_Process");

        uint8_t is_down = 0;
        status = ptx_RUL_LLCP_IsLinkDown(ctx->Llcp, &is_down);

        if ((PTX_STATUS_SUCCESS == status) && (0 == is_down) && (MAC_Layer_Deactivated != ctx->Llcp->State))
        {
            switch (ctx->ClientState)
            {
                case ClientState_Idle:
                    /** Nothing to do here. */
                    break;

                case ClientState_PrepareRequest:
                    ctx->LastSnepStatus = ptx_RUL_SnepClient_ProcessState_PrepareRequest(ctx);
                    break;

                case ClientState_SendFragment:
                    ctx->LastSnepStatus = ptx_RUL_SnepClient_ProcessState_SendFragment(ctx);
                    break;

                case ClientState_SendRequest:
                    ctx->LastSnepStatus = ptx_RUL_LLCP_SendInfo(ctx->Llcp, ctx->Link, ctx->TxBuffer, (uint16_t) ctx->TxBufferLen, 1);
                    ctx->ClientState = (PTX_STATUS_SUCCESS == ctx->LastSnepStatus) ? ClientState_ReceiveResponse : ClientState_OperationFinished;
                    break;

                case ClientState_SendContinue:
                    ctx->RxContinuationMode = 1;
                    ptx_RUL_SnepClient_SetControlMessage(ctx, ClientRequestCode_Continue);
                    ctx->LastSnepStatus = ptx_RUL_LLCP_SendInfo(ctx->Llcp, ctx->Link, ctx->TxBuffer, (uint16_t) ctx->TxBufferLen, 1);
                    ctx->ClientState = (PTX_STATUS_SUCCESS == ctx->LastSnepStatus) ? ClientState_ReceiveFragment : ClientState_OperationFinished;
                    break;

                case ClientState_SendReject:
                    ptx_RUL_SnepClient_SetControlMessage(ctx, ClientRequestCode_Reject);
                    ctx->LastSnepStatus = ptx_RUL_LLCP_SendInfo(ctx->Llcp, ctx->Link, ctx->TxBuffer, (uint16_t) ctx->TxBufferLen, 1);
                    ctx->ClientState = ClientState_OperationFinished;
                    break;

                case ClientState_ReceiveResponse:
                    ctx->LastSnepStatus = ptx_RUL_SnepClient_ProcessResponse(ctx);
                    break;

                case ClientState_ReceiveFragment:
                    ctx->LastSnepStatus = ptx_RUL_SnepClient_ProcessReceiveFragment(ctx);
                    break;

                case ClientState_OperationFinished:
                    ctx->LastSnepStatus = ptx_RUL_LLCP_DisconnectSap(ctx->Llcp, ctx->Link);
                    ctx->ClientState = ClientState_Idle;
                    break;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_ProcessState_PrepareRequest(ptx_RUL_SnepClient_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_ProcessState_PrepareRequest");

        status = PTX_STATUS_SUCCESS;
        switch (ctx->RequestType)
        {
            case PutRequest:
                ctx->CurrentNrBytes = ptx_RUL_SnepClient_GetBytesToProcess(ctx, ctx->MsgToSendIndex, ctx->ProcessedBytes, 0);
                ptx_RUL_SnepClient_SetMessage(ctx, ClientRequestCode_Put,  &ctx->MsgToSendBuffer[ctx->ProcessedBytes], ctx->CurrentNrBytes, ctx->MsgToSendIndex, 0);
                ctx->ProcessedBytes += ctx->CurrentNrBytes;
                if (ctx->ProcessedBytes < ctx->MsgToSendIndex)
                {
                    ctx->TxContinuationMode = 1u;
                    ctx->ContinueResponseReceived = 0u;
                }
                break;
            case GetRequest:
                ctx->CurrentNrBytes = ptx_RUL_SnepClient_GetBytesToProcess(ctx, ctx->MsgToSendIndex, ctx->ProcessedBytes, 1);
                ptx_RUL_SnepClient_SetMessage(ctx, ClientRequestCode_Get, &ctx->MsgToSendBuffer[ctx->ProcessedBytes], ctx->CurrentNrBytes, 0, ctx->MsgToRecvSize);
                ctx->ProcessedBytes += ctx->CurrentNrBytes;
                ctx->ProcessedBytes = 0;
                ctx->CurrentNrBytes = 0;
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        ctx->ClientState = ClientState_SendRequest;
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_ProcessState_SendFragment(ptx_RUL_SnepClient_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_ProcessState_SendFragment");

        ctx->CurrentNrBytes = ptx_RUL_SnepClient_GetBytesToProcess(ctx, ctx->MsgToSendIndex, ctx->ProcessedBytes, 0);
        ptx_RUL_SnepClient_SetMessage(ctx, ClientRequestCode_Null, &ctx->MsgToSendBuffer[ctx->ProcessedBytes], ctx->CurrentNrBytes, 0, 0);
        ctx->ProcessedBytes += ctx->CurrentNrBytes;

        status = ptx_RUL_LLCP_SendInfo(ctx->Llcp, ctx->Link, ctx->TxBuffer, (uint16_t) ctx->TxBufferLen, 1);
        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t rx_data[PTX_LLCP_FRAME_BUFFER_LEN];
            uint32_t rx_data_len;
            status = ptx_RUL_LLCP_Link_ReadMessageInbox(ctx->Link, rx_data, &rx_data_len);

            if (PTX_STATUS_E_GEN_NOT_FOUND == status)
            {
                /* We are not meant to receive anything in this state, hence this is a success. */
                status = PTX_STATUS_SUCCESS;

                if (ctx->ProcessedBytes == ctx->MsgToSendIndex)
                {
                    ctx->TxContinuationMode = 0;
                }
            }
            else
            {
                ctx->ClientState = ClientState_OperationFinished;
            }
        }
        else
        {
            ctx->ClientState = ClientState_OperationFinished;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_ProcessReceiveFragment(ptx_RUL_SnepClient_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_ProcessReceiveFragment");

        uint8_t rx_data[PTX_LLCP_FRAME_BUFFER_LEN];
        uint32_t rx_data_len = sizeof(rx_data);
        status = ptx_RUL_LLCP_Link_ReadMessageInbox(ctx->Link, rx_data, &rx_data_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            if (rx_data_len <= ctx->Link->Local.Miu)
            {
                /* Store Fragment */
                if ((ctx->MsgToRecvIndex + (rx_data_len - PTX_SNEP_FRAME_HEADER_LEN))  < ctx->MsgToRecvSize)
                {
                    memcpy(&ctx->MsgToRecvBuffer[ctx->MsgToRecvIndex], &rx_data[PTX_SNEP_FRAME_HEADER_LEN], (rx_data_len - PTX_SNEP_FRAME_HEADER_LEN));
                    ctx->MsgToRecvIndex += rx_data_len;
                }

                /* NDEF Message complete? */
                if (ctx->MsgToRecvIndex == ctx->RequestLen)
                {
                    ctx->LastSnepStatus = PTX_STATUS_SUCCESS;
                    ctx->ClientState = ClientState_OperationFinished;
                }
            }
            else
            {
                ctx->LastSnepStatus = PTX_STATUS_SNEP_OPERATION_FAILED;
                ctx->ClientState = ClientState_OperationFinished;
            }
        }
        else
        {
            if (0 == ctx->Llcp->LinkTimeoutOccured)
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_SNEP_TIMEOUT_ERROR;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SnepClient_ProcessResponse(ptx_RUL_SnepClient_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_SNEP))
    {
        PTX_LOG_I(&ctx->Llcp->Rul->Log, "ptx_RUL_SnepClient_ProcessResponse");

        uint8_t rx_data[PTX_LLCP_FRAME_BUFFER_LEN];
        uint32_t rx_data_len = sizeof(rx_data);
        status = ptx_RUL_LLCP_Link_ReadMessageInbox(ctx->Link, rx_data, &rx_data_len);

        if ((PTX_STATUS_SUCCESS == status) && (rx_data_len > 0))
        {
            switch (ctx->LastSnepStatus)
            {
                case PTX_STATUS_SUCCESS:
                    /* Default assumption(s) */
                    ctx->LastSnepStatus = PTX_STATUS_SNEP_OPERATION_FAILED;
                    ctx->ClientState = ClientState_OperationFinished;

                    /* Do we have a valid request? */
                    if (rx_data_len >= PTX_SNEP_FRAME_HEADER_LEN)
                    {
                        /* Local MIU exceeded? */
                        if (rx_data_len <= ctx->Link->Local.Miu)
                        {
                            /* Matching version? */
                            if (PTX_SNEPCLIENT_DEFAULT_VERSION == (rx_data[0] & PTX_SNEPCLIENT_MAJOR_VERSION_MASK))
                            {
                                ctx->RequestLen = ptx_RUL_SnepClient_RetrieveLength(&rx_data[2]);
                                ctx->LastSnepResponseCode = rx_data[1];

                                switch (ctx->LastSnepResponseCode)
                                {
                                    case ClientResponseType_Success:
                                        if (PutRequest == ctx->RequestType)
                                        {
                                            /* PUT-Request => everything OK, operation done. */
                                            ctx->LastSnepStatus = PTX_STATUS_SUCCESS;
                                            ctx->ClientState = ClientState_OperationFinished;
                                        }
                                        else
                                        {
                                            if (ctx->RequestLen <= PTX_SNEPCLIENT_RECVEIVEBUFFER_SIZE)
                                            {
                                                /* Is the NDEF-Message complete? */
                                                if ((rx_data_len - PTX_SNEP_FRAME_HEADER_LEN) == ctx->RequestLen)
                                                {
                                                    if ((ctx->MsgToRecvIndex + (rx_data_len - PTX_SNEP_FRAME_HEADER_LEN))  < ctx->MsgToRecvSize)
                                                    {
                                                        memcpy(&ctx->MsgToRecvBuffer[ctx->MsgToRecvIndex], &rx_data[6], (rx_data_len - PTX_SNEP_FRAME_HEADER_LEN));
                                                        ctx->MsgToRecvIndex += (rx_data_len - PTX_SNEP_FRAME_HEADER_LEN);
                                                    }

                                                    /* GET-Request => everything OK, operation done */
                                                    ctx->LastSnepStatus = PTX_STATUS_SUCCESS;
                                                    ctx->ClientState = ClientState_OperationFinished;
                                                }
                                                else
                                                {
                                                    /* Store Fragment */
                                                    ctx->ProcessedBytes = (rx_data_len - PTX_SNEP_FRAME_HEADER_LEN);
                                                    ctx->ClientState = ClientState_SendContinue;

                                                    if ((ctx->MsgToRecvIndex + ctx->ProcessedBytes)  < ctx->MsgToRecvSize)
                                                    {
                                                        memcpy(&ctx->MsgToRecvBuffer[ctx->MsgToRecvIndex], &rx_data[PTX_SNEP_FRAME_HEADER_LEN], ctx->ProcessedBytes);
                                                        ctx->MsgToRecvIndex += ctx->ProcessedBytes;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                ctx->LastSnepStatus = PTX_STATUS_SUCCESS;
                                                ctx->ClientState = ClientState_OperationFinished;
                                            }
                                        }
                                        break;

                                    case ClientResponseType_Continue:
                                        if ((0 == ctx->ContinueResponseReceived) && (0 != ctx->TxContinuationMode))
                                        {
                                            ctx->ClientState = ClientState_SendFragment;
                                            ctx->ContinueResponseReceived = 1u;
                                        }
                                        else
                                        {
                                            ctx->LastSnepStatus = PTX_STATUS_SNEP_OPERATION_FAILED;
                                            ctx->ClientState = ClientState_OperationFinished;
                                        }
                                        break;

                                    default:
                                        ctx->LastSnepStatus = PTX_STATUS_SNEP_OPERATION_FAILED;
                                        ctx->ClientState = ClientState_OperationFinished;
                                        break;
                                }
                            }
                        }
                        else
                        {
                            ctx->LastSnepStatus = PTX_STATUS_SNEP_OPERATION_FAILED;
                            ctx->ClientState = ClientState_OperationFinished;
                        }
                    }
                    else
                    {
                        ctx->LastSnepStatus = PTX_STATUS_SNEP_OPERATION_FAILED;
                        ctx->ClientState = ClientState_OperationFinished;
                    }
                    break;

                case PTX_STATUS_SNEP_TIMEOUT_ERROR:
                    /* Allowed - can happen - just reset the status */
                    ctx->LastSnepStatus = PTX_STATUS_SUCCESS;

                    if (ctx->TxContinuationMode)
                    {
                        ctx->ClientState = ClientState_SendFragment;
                    }
                    break;

                default:
                    ctx->ClientState = ClientState_OperationFinished;
                    break;
            }
        }
        else
        {
            if (0 == ctx->Llcp->LinkTimeoutOccured)
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_SNEP_TIMEOUT_ERROR;
            }
        }
    }

    return status;
}

void ptx_RUL_SnepClient_SetControlMessage(ptx_RUL_SnepClient_t *ctx, ptx_RUL_SnepClient_RequestCode_t code)
{
    /* Just a wrappper for ptx_RUL_SnepClient_SetMessage() since everything other than the request code is NULL / zero. */
    ptx_RUL_SnepClient_SetMessage(ctx, code, NULL, 0, 0, 0);
}

static void ptx_RUL_SnepClient_SetMessage(ptx_RUL_SnepClient_t *ctx, ptx_RUL_SnepClient_RequestCode_t code, uint8_t *information, uint32_t informationLength, uint32_t messageLength, uint32_t acceptableLength)
{
    ctx->TxBufferLen = 0;
    if (ClientRequestCode_Null != code)
    {
        /* Set the implemented SNEP version. */
        ctx->TxBuffer[ctx->TxBufferLen] = PTX_SNEPCLIENT_DEFAULT_VERSION;
        ++ctx->TxBufferLen;

        /* Set the request code. */
        ctx->TxBuffer[ctx->TxBufferLen] = code;
        ++ctx->TxBufferLen;

        /* Set the message length (4 bytes). */
        ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((messageLength >> 24) & 0xFF);
        ++ctx->TxBufferLen;
        ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((messageLength >> 16) & 0xFF);
        ++ctx->TxBufferLen;
        ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((messageLength >> 8) & 0xFF);
        ++ctx->TxBufferLen;
        ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((messageLength >> 0) & 0xFF);
        ++ctx->TxBufferLen;

        if ((ClientRequestCode_Get == code) && (0 != acceptableLength))
        {
            /* Set the maximum acceptable length (4 bytes). */
            ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((acceptableLength >> 24) & 0xFF);
            ++ctx->TxBufferLen;
            ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((acceptableLength >> 16) & 0xFF);
            ++ctx->TxBufferLen;
            ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((acceptableLength >> 8) & 0xFF);
            ++ctx->TxBufferLen;
            ctx->TxBuffer[ctx->TxBufferLen] = (uint8_t) ((acceptableLength >> 0) & 0xFF);
            ++ctx->TxBufferLen;
        }

    }

    if ((NULL != information) && (0 != informationLength))
    {
        /* Append the actual information / payload. */
        memcpy(&ctx->TxBuffer[ctx->TxBufferLen], information, informationLength);
        ctx->TxBufferLen += informationLength;
    }
}

uint32_t ptx_RUL_SnepClient_GetBytesToProcess(ptx_RUL_SnepClient_t *ctx, uint32_t nrBytesToProcess, uint32_t nrBytesProcessed, uint8_t isGetRequest)
{
    uint32_t new_nr_bytes_to_process = (ctx->Link->Remote.Miu < ctx->Link->Local.Miu ? ctx->Link->Remote.Miu : ctx->Link->Local.Miu);
    uint32_t snep_header_length = 0;

    if (!ctx->TxContinuationMode)
    {
        snep_header_length = (!isGetRequest ? PTX_SNEP_FRAME_HEADER_LEN : (PTX_SNEP_FRAME_HEADER_LEN + PTX_SNEPCLIENT_LENGTH_LENGTH));

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

uint32_t ptx_RUL_SnepClient_RetrieveLength(uint8_t *data)
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