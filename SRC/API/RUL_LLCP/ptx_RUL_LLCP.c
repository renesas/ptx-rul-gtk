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
    Module      : LLCP
    File        : ptx_RUL_LLCP.c

    Description : Logical Link Control Protocol (LLCP) implementation.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_RUL_LLCP.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_LLCP_ATRES_LEN_OFFSET (21)
#define PTX_LLCP_ATREQ_LEN_OFFSET (20)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_RUL_LLCP_RegisterExtension(ptx_RUL_LLCP_t *ctx);
static ptx_Status_t ptx_RUL_LLCP_CB_ProcessActivateNtf(void *ctx, ptx_NSC_RfActivate_Ntf_t *ntf, uint8_t *write_card_registry);
static ptx_Status_t ptx_RUL_LLCP_CB_ProcessDeactivateNtf(void *ctx, ptx_NSC_RfDeactivate_Ntf_t *ntf, uint8_t *ntf_used);
static ptx_Status_t ptx_RUL_LLCP_CB_ProcessRfFieldNtf(void *ctx, uint8_t field, uint8_t *ntf_used);
static ptx_Status_t ptx_RUL_LLCP_CB_ProcessDataMsg(void *ctx, ptx_NSC_RfData_Msg_t *msg);
static ptx_Status_t ptx_RUL_LLCP_CB_Timer(ptx_HAL_DRV_Tmr_ExecCbCtx_t ctx);
static ptx_Status_t ptx_RUL_LLCP_ConfigureTimer(ptx_RUL_LLCP_t *ctx);
static ptx_Status_t ptx_RUL_LLCP_IsSapFree(ptx_RUL_LLCP_t *ctx, uint8_t address, uint8_t *free);
static ptx_Status_t ptx_RUL_LLCP_ReserveSap(ptx_RUL_LLCP_t *ctx, uint8_t address);
static ptx_Status_t ptx_RUL_LLCP_FreeSap(ptx_RUL_LLCP_t *ctx, uint8_t address);
static ptx_Status_t ptx_RUL_LLCP_SendMessageBuffer(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_MessageBuffer_t *buffer);
static ptx_Status_t ptx_RUL_LLCP_DisconnectServiceLayer(ptx_RUL_LLCP_t *ctx);
static ptx_Status_t ptx_RUL_LLCP_ValidateServiceName(ptx_RUL_LLCP_t *ctx, char* name, uint32_t nameLength, uint8_t *valid);
static ptx_Status_t ptx_RUL_LLCP_ProcessRxPdu(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_GetLinkFromDestination(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu, ptx_RUL_LLCP_Link_t **link);
static ptx_Status_t ptx_RUL_LLCP_AssembleRejectFrame(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, ptx_RUL_LLCP_PDU_t *pdu, uint8_t w, uint8_t i, uint8_t r, uint8_t s);
static ptx_Status_t ptx_RUL_LLCP_Processor_Symm(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_Ui(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_Connect(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_Disconnect(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_ConnectionComplete(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_DisconnectMode(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_FrameReject(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_ServiceNameLookup(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_Information(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_ReceiveReady(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_ReceiveNotReady(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_RFU(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);
static ptx_Status_t ptx_RUL_LLCP_Processor_HandleInitialCondition(ptx_RUL_LLCP_t *ctx);
static ptx_Status_t ptx_RUL_LLCP_Processor_ValidateLinkTimeout(ptx_RUL_LLCP_t *ctx);
static ptx_Status_t ptx_RUL_LLCP_Processor_CheckRfErrors(ptx_RUL_LLCP_t *ctx);
static ptx_Status_t ptx_RUL_LLCP_Processor_ConnectionTeardown(ptx_RUL_LLCP_t *ctx);
static ptx_Status_t ptx_RUL_LLCP_Processor_HandleStates(ptx_RUL_LLCP_t *ctx);
static ptx_Status_t ptx_RUL_LLCP_Processor(ptx_RUL_LLCP_t *ctx);
static ptx_Status_t ptx_RUL_LLCP_WaitForAcknowledge(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link);
static ptx_Status_t ptx_RUL_LLCP_IsLinkConnected(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, uint8_t *connected);
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_RUL_LLCP_Init(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_InitParameters_t *initParameters)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParameters) && PTX_COMP_CHECK(initParameters->Rul, Comp_RUL))
    {        
        PTX_LOG_I(&initParameters->Rul->Log, "ptx_RUL_LLCP_Init");

        /* Initialize the main message buffer */
        status = ptx_RUL_LLCP_MessageBuffer_Init(&ctx->TxBuffer);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Initialize the frame buffer */
            status = ptx_RUL_LLCP_Frame_Open(&ctx->LlcpFrame);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Pre-Initialize all available (= statically allocated) links */
            for (uint8_t i = 0; (i < PTX_LLCP_MAX_LINKS) && (PTX_STATUS_SUCCESS == status); ++i)
            {
                status = ptx_RUL_LLCP_Link_Init(&ctx->Link[i]);
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* In case general bytes were provided, copy them into the internal data structure.  */
            if ((NULL != initParameters->GeneralBytes) && (0 != initParameters->GeneralBytesLength))
            {
                /* Copy general bytes if they exist. */
                uint8_t bytes_to_copy = 0;
                if (PTX_LLCP_GENERAL_BYTES_MAX_LEN > initParameters->GeneralBytesLength)
                {
                    bytes_to_copy = initParameters->GeneralBytesLength;
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    memcpy(ctx->GeneralBytes, initParameters->GeneralBytes, bytes_to_copy);
                }
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Initialize the rest of the internal variables  */
            ctx->CompId = Comp_LLCP;
            ctx->SentI = 0u;
            ctx->SentUI = 0u;
            ctx->Running = 0u;
            ctx->Initiate = 0u;
            ctx->RxDataIntLen = 0u;
            ctx->SapList = 0x0000;
            ctx->LinkTimeoutOccured = 0u;
            ctx->LastDisconnectReason = 0u;
            ctx->Rul = initParameters->Rul;
            ctx->State = Link_Layer_Activated;
            ctx->ApplicationTimerTimeout = 5000u;
            ctx->TargetLinkTimeout = PTX_LLCP_DEFAULT_GEN_BYTES_LTO;

            /* Set LINK_MGMT & SERVICE_DISC in SAP list to "in use" */
            ptx_RUL_LLCP_ReserveSap(ctx, PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT);
            ptx_RUL_LLCP_ReserveSap(ctx, PTX_LLCP_SAP_ADDRESS_SERVICE_DISCOVERY);

            /* Register LLCP as RUL extension */
            status = ptx_RUL_LLCP_RegisterExtension(ctx);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Preconfigure the HW Timer (required for LinkTimout measurement). */
            status = ptx_RUL_LLCP_ConfigureTimer(ctx);
        }

        if (PTX_STATUS_SUCCESS != status)
        {
            /* In case something went wrong, reset the whole data structure. */
            memset(ctx, 0, sizeof(ptx_RUL_LLCP_t));
        }
    }

    return status;
}


ptx_Status_t ptx_RUL_LLCP_SetApplicationTimeout(ptx_RUL_LLCP_t *ctx, uint32_t applicationTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_SetApplicationTimeout");

        ctx->ApplicationTimerTimeout = applicationTimeout;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_SetCustomGeneralBytes(ptx_RUL_LLCP_t *ctx, uint8_t *generalBytes, uint8_t generalBytesLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != generalBytes) && (PTX_LLCP_GENERAL_BYTES_MAX_LEN >= generalBytesLen))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_SetCustomGeneralBytes");

        /* First, clear the general bytes array. */
        memset(ctx->GeneralBytes, 0, PTX_LLCP_GENERAL_BYTES_MAX_LEN);
        /* Set the general bytes to the desired value. */
        memcpy(ctx->GeneralBytes, generalBytes, generalBytesLen);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_GenerateDefaultGeneralBytes(ptx_RUL_LLCP_t *ctx, uint8_t *generalBytes, uint8_t *generalBytesLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != generalBytes) && (NULL != generalBytesLen) && (PTX_LLCP_GENERAL_BYTES_MAX_LEN == *generalBytesLen))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_GenerateDefaultGeneralBytes");

        /* Clear the general bytes array. */
        memset(generalBytes, 0, *generalBytesLen);

        /* Set the three magic bytes. */
        generalBytes[PTX_LLCP_DEFAULT_MAGIC_BYTE_0_INDEX] = PTX_LLCP_DEFAULT_MAGIC_BYTE_0_VALUE;
        generalBytes[PTX_LLCP_DEFAULT_MAGIC_BYTE_1_INDEX] = PTX_LLCP_DEFAULT_MAGIC_BYTE_1_VALUE;
        generalBytes[PTX_LLCP_DEFAULT_MAGIC_BYTE_2_INDEX] = PTX_LLCP_DEFAULT_MAGIC_BYTE_2_VALUE;
        *generalBytesLen = PTX_LLCP_DEFAULT_TLV_LEN_GEN_BYTES;

        ptx_RUL_LLCP_TLV_Handler_t handler;
        memset(&handler, 0, sizeof(ptx_RUL_LLCP_TLV_Handler_t));
        /* Setup the TLV helper. */
        status = ptx_RUL_LLCP_TLV_Open(&handler);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Write the most essential connection parameters (VERSION, LTO, MIUX). */
            ptx_RUL_LLCP_TLV_Data_t tlvs[PTX_LLCP_DEFAULT_GEN_BYTES_TLVS];

            /* Write the default LLCP version number. */
            tlvs[0].Type = LLCP_TagType_Version;
            tlvs[0].Data.Version.Major = PTX_LLCP_DEFAULT_GEN_BYTES_MAJOR_VERSION;
            tlvs[0].Data.Version.Minor = PTX_LLCP_DEFAULT_GEN_BYTES_MINOR_VERSION;

            /* Write the default link timeout. */
            tlvs[1].Type = LLCP_TagType_LTO;
            tlvs[1].Data.Lto.Timeout = PTX_LLCP_DEFAULT_GEN_BYTES_LTO;

            /* Write the default Maximum Information Unit Extension (MIUX). */
            tlvs[2].Type = LLCP_TagType_MIUX;
            tlvs[2].Data.Miux.Miux = PTX_LLCP_DEFAULT_GEN_BYTES_MIUX;

            /* Serialize the above TLVs. */
            status = ptx_RUL_LLCP_TLV_Write (&handler, tlvs, PTX_LLCP_DEFAULT_GEN_BYTES_TLVS);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Copy the generated & serializes TLVs to the rest of the general bytes. */
            memcpy(&generalBytes[PTX_LLCP_DEFAULT_TLV_LEN_GEN_BYTES], handler.ScratchPad, handler.ScratchPadIndex);
            *generalBytesLen = (uint8_t) (*generalBytesLen + handler.ScratchPadIndex);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_StartDefaultDiscovery(ptx_RUL_LLCP_t *ctx, uint32_t idleTime)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_StartDefaultDiscovery");

        /* Prepare the . */
        ptx_RUL_DiscoverConfig_t config;
        memset(&config, 0, sizeof(ptx_RUL_DiscoverConfig_t));

        if (0 == ctx->GeneralBytesLength)
        {
            /* In this case the user didn't provide any general bytes --> generate the default ones. */
            ctx->GeneralBytesLength = PTX_LLCP_GENERAL_BYTES_MAX_LEN;
            status = ptx_RUL_LLCP_GenerateDefaultGeneralBytes(ctx, ctx->GeneralBytes, &ctx->GeneralBytesLength);
        }
        else
        {
            /* In this case the general bytes were already set by user. */
            status = PTX_STATUS_SUCCESS;
        }

        /* Polling parameters change depending on if we are in Initiator or Target mode. */
        config.EnablePolling           = 1u;
        config.PollMode                = ptx_RUL_PollMode_NfcForum;
        config.IdleTime                = idleTime;
        config.DisablePollIsoDep       = 1u;
        config.PollNfcDepHbr           = ptx_RUL_PollNfcDepHBR_Standard;
        config.PollTypeA               = 1u;
        config.PollTypeAPollFrequency  = 1u;
        config.PollTypeF212            = 1u;
        config.PollTypeFPollFrequency  = 1u;
        config.PollNfcDepAtrReqLen     = ctx->GeneralBytesLength;
        memcpy(config.PollNfcDepAtrReq, ctx->GeneralBytes, config.PollNfcDepAtrReqLen);

#if !defined(PTX_DISABLE_LISTEN_MODE)
        /* Listen mode activated & Rf-Field NTFs enabled. */
        config.ListenMode              = 3u;
        config.ListenTypeA             = 1u;
        config.ListenTypeF212          = 1u;
        config.ListenTypeF424          = 1u;
        config.ListenNfcDep            = 1u;
        config.ListenNfcDepAtrRes[PTX_LLCP_DEFAULT_ATRRES_WT_INDEX] = PTX_LLCP_DEFAULT_ATRRES_WT_VALUE;
        config.ListenNfcDepAtrRes[PTX_LLCP_DEFAULT_ATRRES_PP_INDEX] = PTX_LLCP_DEFAULT_ATRRES_PP_VALUE;
        memcpy(&config.ListenNfcDepAtrRes[PTX_LLCP_DEFAULT_ATRRES_GEN_BYTES_OFFSET], ctx->GeneralBytes, ctx->GeneralBytesLength);
        config.ListenNfcDepAtrResLen = ctx->GeneralBytesLength + PTX_LLCP_DEFAULT_ATRRES_GEN_BYTES_OFFSET;
#endif /* PTX_DISABLE_LISTEN_MODE */

        /* Start the actual RF discovery. */
        status = ptx_RUL_InitiateDiscovery(ctx->Rul, &config);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_DeInit(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_DeInit");

        /* Deinitialize the main message buffer. */
        status = ptx_RUL_LLCP_MessageBuffer_DeInit(&ctx->TxBuffer);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Deinitialize the frame buffer. */
            status = ptx_RUL_LLCP_Frame_Close(&ctx->LlcpFrame);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Deinitialize all available SAPs / logical Links. */
            for (uint8_t i = 0; (i < PTX_LLCP_MAX_LINKS) && (PTX_STATUS_SUCCESS == status); ++i)
            {
                status = ptx_RUL_LLCP_Link_DeInit(&ctx->Link[i]);
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_Tmr_Stop(&(ctx->Rul->Hal), HAL_Tmr_Id_1);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Deregister extension endpoint from RUL. */
            status = ptx_RUL_RemoveExtensionEndpoints(ctx->Rul, Comp_LLCP);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Clear the list with all running SAPs. */
            ctx->SapList = 0x00;
            memset(&ctx->ActivateNtf, 0, sizeof(ptx_NSC_RfActivate_Ntf_t));
            ctx->CompId = Comp_MAX;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_RegisterSap(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t **link, ptx_RUL_LLCP_Link_InitParams_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_RegisterSap");

        uint8_t free = 0;

        /* Check if the SAP is still unused. */
        status = ptx_RUL_LLCP_IsSapFree(ctx, params->Ssap, &free);

        if ((PTX_STATUS_SUCCESS == status) && (0 != free))
        {
            /* Get the pointer to one of the unused datastructures. */
            status = ptx_RUL_LLCP_AllocateLink(ctx, link);
        }

        if ((PTX_STATUS_SUCCESS == status) && (0 != free))
        {
            /* Get the pointer to one of the unused datastructures. */
            status = ptx_RUL_LLCP_Link_Register(*link, params);
        }

        if ((PTX_STATUS_SUCCESS == status) && (0 != free))
        {
            /* Mark the SAP as reserved if everything went well. */
            status = ptx_RUL_LLCP_ReserveSap(ctx, params->Ssap);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_ConnectSap(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, uint8_t dsap)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_ConnectSap");

        /* Check if we are in the correct state to establish a new connection. */
        if (Link_Layer_Activated == ctx->State)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_LLCP_DISCONNECTED_BY_REMOTE;
        }

        /* Prepare the CONNECT PDU together with the correct TLVs. */
        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_LLCP_TLV_Data_t tlvs[LLCP_CONNECT_SAP_BY_ID_TLVS];
            /* Set the MIUX TLV. */
            tlvs[0].Type = LLCP_TagType_MIUX;
            tlvs[0].Data.Miux.Miux = (uint16_t)(link->Local.Miu - PTX_LLCP_DEFAULT_MIU);
            /* Set the Receive Window (RW) TLV. */
            tlvs[1].Type = LLCP_TagType_RW;
            tlvs[1].Data.Rw.WindowSize = link->Local.Window;
            /* Assemble the CONNECT PDU. */
            status = ptx_RUL_LLCP_PDU_Connect(&ctx->LlcpFrame, dsap, link->Local.Address, tlvs, LLCP_CONNECT_SAP_BY_ID_TLVS);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Write/Serialize the assembled PDU to the buffer for outgoing messages. */
            status = ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Set the LinkState to "connection pending". */
            link->LinkState = LinkState_ConnectionPending;
            /* Setup the receive status handler (responsible to handle timeouts). */
            status = ptx_RUL_LLCP_SetupReceiveStatusHandler(ctx, link);
        }

        /* Wait until we are either in the connected state, or until we get a timeout. */
        while ((PTX_STATUS_SUCCESS == status) && (LinkState_Connected != link->LinkState))
        {
            status = ptx_RUL_LLCP_Handle(ctx);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Regularly check the the status handler. */
                status = ptx_RUL_LLCP_CheckReceiveStatusHandler(ctx);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_ConnectSapByName(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, char* name)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_ConnectSapByName");

        /* Check if we are in the correct state to establish a new connection. */
        if (Link_Layer_Activated == ctx->State)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        /* Prepare the CONNECT PDU together with the correct TLVs. */
        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_LLCP_TLV_Data_t tlvs[LLCP_CONNECT_SAP_BY_NAME_TLVS];

            tlvs[0].Type = LLCP_TagType_SN;
            tlvs[0].Data.Sn.Name = name;
            tlvs[0].Data.Sn.Length = (uint32_t) strlen(name);

            tlvs[1].Type = LLCP_TagType_MIUX;
            tlvs[1].Data.Miux.Miux = (uint16_t)(link->Local.Miu - PTX_LLCP_DEFAULT_MIU);

            tlvs[2].Type = LLCP_TagType_RW;
            tlvs[2].Data.Rw.WindowSize = link->Local.Window;

            status = ptx_RUL_LLCP_PDU_Connect(  &ctx->LlcpFrame,
                                                PTX_LLCP_SAP_ADDRESS_SERVICE_DISCOVERY,
                                                link->Local.Address, tlvs,
                                                LLCP_CONNECT_SAP_BY_NAME_TLVS);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Write the assembled PDU to the buffer for outgoing messages. */
            status = ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Set the LinkState to "connection pending". */
            link->LinkState = LinkState_ConnectionPending;
            /* Setup the receive status handler (responsible to handle timeouts). */
            status = ptx_RUL_LLCP_SetupReceiveStatusHandler(ctx, link);
        }

        while ((PTX_STATUS_SUCCESS == status) && (LinkState_Connected != link->LinkState))
        {
            status = ptx_RUL_LLCP_Handle(ctx);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Regularly check the the status handler. */
                status = ptx_RUL_LLCP_CheckReceiveStatusHandler(ctx);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_DisconnectSap(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != link))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_DisconnectSap");

        /* Check if we are in the correct state to terminate the connection. */
        if ((Link_Layer_Activated == ctx->State) && (LinkState_Connected == link->LinkState))
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Clear all pending outgoing messages. */
            status = ptx_RUL_LLCP_MessageBuffer_Init(&link->Remote.MessageBuffer);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Set the LinkState to "disconnect pending". */
            link->LinkState = LinkState_DisconnectionPending;
            /* Setup the disconnect PDU. */
            status = ptx_RUL_LLCP_PDU_Disconnect(&ctx->LlcpFrame, link->Remote.Address, link->Local.Address);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Write the assembled PDU to the buffer for outgoing messages. */
            status = ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Setup the receive status handler (responsible to handle timeouts). */
            status = ptx_RUL_LLCP_SetupReceiveStatusHandler(ctx, link);
        }

        while ((PTX_STATUS_SUCCESS == status) && (LinkState_Disconnected != link->LinkState))
        {
            status = ptx_RUL_LLCP_Handle(ctx);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Regularly check the the status handler. */
                status = ptx_RUL_LLCP_CheckReceiveStatusHandler(ctx);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_SendInfo(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, uint8_t *payload, uint16_t payloadLength, uint8_t waitForAck)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != link) && (NULL != payload))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_SendInfo");

        /* Check if we are in the correct state to send the information PDU. */
        if ((Link_Layer_Activated == ctx->State) && (LinkState_Connected == link->LinkState))
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        /* We can only send Information PDUs across a Connection Oriented Link. */
        if ((PTX_STATUS_SUCCESS == status) && (LinkType_ConnectionOriented != link->Type))
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

        /* Do we have a valid remote Receive Window (RW). */
        if ((PTX_STATUS_SUCCESS == status) && (0 == link->Remote.Window))
        {
            status = PTX_STATUS_E_GEN_INTERFACE_TX;
        }

        /* Flow control check: are we permitted to tranmit further PDUs. */
        if ((PTX_STATUS_SUCCESS == status) && ((link->TrxStates.Vs + 1) % 16) <= ((link->TrxStates.Vsa + link->Remote.Window) % 16))
        {
            /* Is the remote busy? */
            if (0 == link->Remote.Busy)
            {
                status = ptx_RUL_LLCP_PDU_Information(  &ctx->LlcpFrame,
                                                        link->Remote.Address,
                                                        link->Local.Address,
                                                        link->TrxStates.Vs,
                                                        link->TrxStates.Vr,
                                                        payload,
                                                        (uint8_t) payloadLength);
            }
            else
            {
                status = PTX_STATUS_LLCP_REMOTE_PEER_BUSY;
            }
        }
        else
        {
            status = PTX_STATUS_LLCP_REMOTE_RW_FULL;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Buffer the message within the Links Message outbox. */
            status = ptx_RUL_LLCP_Link_WriteMessageOutbox(link, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Write the assembled PDU to the buffer for outgoing messages. */
            status = ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Increment the send state (needed for flow control). */
            status = ptx_RUL_LLCP_TrxState_IncrementSendState(&link->TrxStates);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Update the Receive Acknowledge State. */
            status = ptx_RUL_LLCP_TrxState_SetReceiveAcknowledgeState(&link->TrxStates, link->TrxStates.Vr);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->SentI = 0;
            ctx->ActiveLink = link;
            status = ptx_RUL_LLCP_SetupReceiveStatusHandler(ctx, NULL);
        }

        while ((PTX_STATUS_SUCCESS == status) && (Link_Layer_Activated == ctx->State) && (0 == ctx->SentI))
        {
            status = ptx_RUL_LLCP_Handle(ctx);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_LLCP_CheckReceiveStatusHandler(ctx);
            }
        }

        if ((PTX_STATUS_SUCCESS == status) && (0 != waitForAck))
        {
            status = ptx_RUL_LLCP_WaitForAcknowledge(ctx, link);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_WaitForActivation(ptx_RUL_LLCP_t *ctx, uint8_t *activated, uint32_t timeoutMs)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != activated))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_WaitForActivation");

        uint32_t start_tick = ctx->TimerTicks;
        do
        {
            /* Retrieve the discovery status. */
            ptx_RUL_Status_Params_t status_params;
            status_params.Type = ptx_RUL_StatusType_Discover;
            status = ptx_RUL_GetStatusInfo(ctx->Rul, &status_params);

            /* Check LLCP support by interpreting the general bytes. */
            if ((PTX_STATUS_SUCCESS == status) && (ptx_RUL_StatusInfoDiscover_Active == status_params.Info.Discover))
            {
                status = ptx_RUL_LLCP_IsLlcpSupported(ctx, activated);
            }
        }
        while ((PTX_STATUS_SUCCESS == status) && (0 == *activated) && ((ctx->TimerTicks - start_tick) * PTX_LLCP_DEFAULT_TIMER_TICK_UNIT) < timeoutMs);

        if ((PTX_STATUS_SUCCESS == status) && (0 != *activated))
        {
            status = ptx_RUL_LLCP_Handle(ctx);
        }
    }

    return status;
}


ptx_Status_t ptx_RUL_LLCP_Handle(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_Handle");

        ptx_RUL_Status_Params_t status_params;
        status_params.Type = ptx_RUL_StatusType_System;
        status = ptx_RUL_GetStatusInfo(ctx->Rul, &status_params);

        if ((PTX_STATUS_SUCCESS == status) && (ptx_RUL_StatusInfoSystem_OK == status_params.Info.System))
        {
            status = ptx_RUL_LLCP_Processor(ctx);
        }
    }

    return status;
}


ptx_Status_t ptx_RUL_LLCP_WaitForAnyConnection(ptx_RUL_LLCP_t *ctx, uint32_t timeoutMs)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_WaitForAnyConnection");

        if (Link_Layer_Activated == ctx->State)
        {
            uint8_t timeout = 0;
            uint8_t connected = 0;
            uint32_t start_time = ctx->TimerTicks;

            status = PTX_STATUS_SUCCESS;

            while ((PTX_STATUS_SUCCESS == status) && (0 == connected) && (0 == timeout))
            {
                status = ptx_RUL_LLCP_Handle(ctx);

                if (PTX_STATUS_SUCCESS == status)
                {
                    for (uint8_t i = 0; (i < PTX_LLCP_MAX_LINKS) && (0 == connected); ++i)
                    {
                        ptx_RUL_LLCP_IsLinkConnected(ctx, &ctx->Link[i], &connected);
                    }
                }

                timeout = (((ctx->TimerTicks - start_time) * PTX_LLCP_DEFAULT_TIMER_TICK_UNIT) > timeoutMs);
            }

            if (0 != timeout)
            {
                status = PTX_STATUS_LLCP_TIMEOUT;
            }
        }
        else
        {
            status = PTX_STATUS_LLCP_DISCONNECTED_BY_REMOTE;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_WaitForConnection(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, uint32_t timeoutMs)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_WaitForConnection");

        uint32_t application_timeout = ctx->ApplicationTimerTimeout;
        ctx->ApplicationTimerTimeout = timeoutMs;

        /* Check if we are in the correct state to wait for an incoming connection. */
        if (Link_Layer_Activated == ctx->State)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_LLCP_DISCONNECTED_BY_REMOTE;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (Role_Initiator == ctx->Role)
            {
                link->LinkState = LinkState_ConnectionPending;
            }

            status = ptx_RUL_LLCP_SetupReceiveStatusHandler(ctx, link);
        }

        while ((PTX_STATUS_SUCCESS == status) && (LinkState_Connected != link->LinkState))
        {
            status = ptx_RUL_LLCP_Handle(ctx);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_LLCP_CheckReceiveStatusHandler(ctx);
            }
        }

        ctx->ApplicationTimerTimeout = application_timeout;

        if ((PTX_STATUS_SUCCESS == status) && (LinkState_Connected == link->LinkState))
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = ctx->LastLlcpStatus;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_SendUInfo(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, uint8_t *payload, uint16_t payloadLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != link) && (NULL != payload) && (0 != payloadLength))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_SendUInfo");

        /* Check if we are in the correct state to send the unnumbered information PDU. */
        if (Link_Layer_Activated == ctx->State)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }


        /* Validate if the unnumbered PDU is really being sent from a SAP registered as "unnumbered" SAP. */
        if ((PTX_STATUS_SUCCESS == status) && (LinkType_ConnectionLess != link->Type))
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

        if ((PTX_STATUS_SUCCESS == status) && (payloadLength > link->Remote.Miu))
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Prepare the PDU Unnumbered Information PDU. */
            status = ptx_RUL_LLCP_PDU_Ui(&ctx->LlcpFrame, link->Remote.Address, link->Local.Address, payload, (uint8_t) payloadLength);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Write the serialized PDU to the outgoing message buffer. */
            status = ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->SentUI = 0;
            status = ptx_RUL_LLCP_SetupReceiveStatusHandler(ctx, NULL);
        }

        while ((PTX_STATUS_SUCCESS == status) && (Link_Layer_Activated == ctx->State) && (0 == ctx->SentUI))
        {
            status = ptx_RUL_LLCP_Handle(ctx);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_LLCP_CheckReceiveStatusHandler(ctx);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_DiscoverService(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Parameter_ServiceName_t *service, uint8_t *dSap)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != service) && (NULL != dSap))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_DiscoverService");

        /* Check if we are in the correct state to do a SNL request. */
        if (Link_Layer_Activated == ctx->State)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_LLCP_DISCONNECTED_BY_REMOTE;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Allocate and prepare the SDREQ TLV datastructure*/
            ptx_RUL_LLCP_TLV_Data_t tlv;
            tlv.Type = LLCP_TagType_SDREQ;
            tlv.Data.SdReq.TransactionIdentifier = 0x55;
            tlv.Data.SdReq.ServiceName.Name = service->Name;
            tlv.Data.SdReq.ServiceName.Length = service->Length;

            /* Initialize the SNL PDU with the provided TLV. */
            status = ptx_RUL_LLCP_PDU_ServiceNameLookup (&ctx->LlcpFrame, &tlv, 1u);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Write the serialized PDU to the outgoing message buffer. */
            status = ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Setup the receive status handler. */
            status = ptx_RUL_LLCP_SetupReceiveStatusHandler(ctx, NULL);
        }

        ctx->RemoteServiceId = 0x00;
        /* Execute the LLC protocol until either we got a response or ran into a timeout. */
        while ((PTX_STATUS_SUCCESS == status) && (0x00 == ctx->RemoteServiceId))
        {
            status = ptx_RUL_LLCP_Handle(ctx);

            if (status == PTX_STATUS_SUCCESS)
            {
                status = ptx_RUL_LLCP_CheckReceiveStatusHandler(ctx);
            }
        }

        if (status == PTX_STATUS_SUCCESS)
        {
            /* In case everything went fine, copy over the acquired DSAP address. */
            *dSap = ctx->RemoteServiceId;
        }
        status = ctx->LastLlcpStatus;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_ReceiveFromLink(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != link))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_ReceiveFromLink");

        /* Check if we are in the correct state to receive something. */
        if (Link_Layer_Activated == ctx->State)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_LLCP_DISCONNECTED_BY_REMOTE;
        }

        if (status == PTX_STATUS_SUCCESS)
        {
            /* Setup the receive status handler (responsible to handle timeouts). */
            status = ptx_RUL_LLCP_SetupReceiveStatusHandler(ctx, link);
        }

        /* Allocate a variable to store the amount of received messages. */
        uint32_t size = 0;

        /* Loop until either we received a message/PDU or until a timeout happened. */
        while ((PTX_STATUS_SUCCESS == status) && (0 == size))
        {
            /* Regularly poll for the amount of messages within the input buffer. */
            ptx_RUL_LLCP_MessageBuffer_Size(&link->Local.MessageBuffer, &size);

            status = ptx_RUL_LLCP_Handle(ctx);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_LLCP_CheckReceiveStatusHandler(ctx);
            }
        }

        status = ctx->LastLlcpStatus;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_SetupReceiveStatusHandler(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_SetupReceiveStatusHandler");

        ctx->ReceiveLink = NULL;
        ctx->LastLlcpStatus = PTX_STATUS_SUCCESS;
        /* Get the current timestamp. */
        ctx->ApplicationTimerInitValue = ctx->TimerTicks;

        if (NULL != link)
        {
            ctx->ReceiveLink = (LinkType_ConnectionOriented == link->Type ? link : NULL);
        }
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_CheckReceiveStatusHandler(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_CheckReceiveStatusHandler");

        status = PTX_STATUS_SUCCESS;

        if (Role_Target == ctx->Role)
        {
            /* Is the MAC layer still up (in Target-Mode)? */
            if (0 == ctx->RfFieldPresent)
            {
                ctx->State = MAC_Layer_Deactivated;
                status = PTX_STATUS_E_GEN_INTERNAL;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (ctx->State)
            {
                case MAC_Layer_Deactivated:
                    /* Is the MAC layer still up? */
                    ctx->LastLlcpStatus = PTX_STATUS_LLCP_MAC_DEACTIVATED_BY_REMOTE;
                    status = PTX_STATUS_E_GEN_INTERNAL;
                    break;

                case Link_Layer_Deactivated_Remote:
                    /* Is the Link still up? */
                    if (Role_Initiator == ctx->Role)
                    {
                        ctx->LastLlcpStatus = PTX_STATUS_LLCP_CONNECTION_DISCONNECTED_BY_REMOTE;
                        status = PTX_STATUS_E_GEN_INTERNAL;
                    }
                    break;
                default:
                    /* OK */
                    break;
            }
        }

        if ((PTX_STATUS_SUCCESS == status) && (NULL != ctx->ReceiveLink))
        {
            switch (ctx->ReceiveLink->LinkState)
            {
                case LinkState_Disconnected:
                    /* The connection has been terminated - nothing we can do about it at this point. */
                    ctx->LastLlcpStatus = PTX_STATUS_LLCP_CONNECTION_DISCONNECTED_BY_REMOTE;
                    status = PTX_STATUS_E_GEN_INTERNAL;
                    break;

                case LinkState_FrameRejected:
                    /* There has been an error causing a frame to be rejected. */
                    ctx->LastLlcpStatus = PTX_STATUS_LLCP_FRMR_EXCHANGED;
                    status = PTX_STATUS_E_GEN_INTERNAL;
                    break;

                default:
                    break;
            }
        }

        /* Calculate the amount of time that has passed, depending on the amount of timer ticks. */
        uint32_t passed_time = ((ctx->TimerTicks - ctx->ApplicationTimerInitValue) * PTX_LLCP_DEFAULT_TIMER_TICK_UNIT);
        if (passed_time > ctx->ApplicationTimerTimeout)
        {
            ctx->LastLlcpStatus = PTX_STATUS_LLCP_TIMEOUT;
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_GetFreeSapAddress(ptx_RUL_LLCP_t *ctx, uint8_t wellKnown, uint8_t *foundAddress)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != foundAddress))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_GetFreeSapAddress");

        /* Check in which range of SAPs we have to search. The first 16 SAPs are well known. */
        uint8_t start_address = ((0 == wellKnown) ? PTX_LLCP_WELLKNOWN_SIZE : PTX_LLCP_SAP_ADDRESS_SNEP);
        uint8_t end_address = ((0 == wellKnown) ? PTX_LLCP_SAP_ADDRESS_MAX : PTX_LLCP_WELLKNOWN_SIZE);

        uint8_t address;
        uint8_t free = 0;
        for (address = start_address; (address < end_address) && (0 == free); ++address)
        {
            ptx_RUL_LLCP_IsSapFree(ctx, address, &free);
        }

        if (address < end_address)
        {
            *foundAddress = address;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_EnableSnep(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_EnableSnep");

        /* Sets the bit in the WKS list for SNEP. */
        status = ptx_RUL_LLCP_ReserveSap(ctx, PTX_LLCP_SAP_ADDRESS_SNEP);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_DisableSnep(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_DisableSnep");

        /* Clears the bit in the WKS list for SNEP. */
        status = ptx_RUL_LLCP_FreeSap(ctx, PTX_LLCP_SAP_ADDRESS_SNEP);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_GetDisconnectReason(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_DisconnectReason_t *reason)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != reason))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_GetDisconnectReason");

        *reason = ctx->LastDisconnectReason;
        
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_IsLlcpSupported(ptx_RUL_LLCP_t *ctx, uint8_t *supported)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != supported))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_IsLlcpSupported");

        status = PTX_STATUS_SUCCESS;

        *supported = 0;
        if (0 != (0x10 & ctx->ActivateNtf.RfTech))
        {
            /* PTX2K is in listen mode and just got activated by another reader. */
            if ( ((0x11 == ctx->ActivateNtf.RfTech) || (0x13 == ctx->ActivateNtf.RfTech)) &&
                 (PTX_LLCP_DEFAULT_MAGIC_BYTE_0_VALUE == ctx->ActivateNtf.RfProtParams.NfcDep.ListenAF.AtrReq[PTX_LLCP_ATRREQ_MAGIC_BYTE_0_OFFSET]) &&
                 (PTX_LLCP_DEFAULT_MAGIC_BYTE_1_VALUE == ctx->ActivateNtf.RfProtParams.NfcDep.ListenAF.AtrReq[PTX_LLCP_ATRREQ_MAGIC_BYTE_1_OFFSET]) &&
                 (PTX_LLCP_DEFAULT_MAGIC_BYTE_2_VALUE == ctx->ActivateNtf.RfProtParams.NfcDep.ListenAF.AtrReq[PTX_LLCP_ATRREQ_MAGIC_BYTE_2_OFFSET]))
            {
                ctx->Role = Role_Target;
                ctx->RfFieldPresent = 1;
                *supported = 1;
            }

        }
        else
        {
            /* PTX2K is in poll mode and just activated a tag. */
            if ( ((0x01 == ctx->ActivateNtf.RfTech) || (0x03 == ctx->ActivateNtf.RfTech)) &&
                 (PTX_LLCP_DEFAULT_MAGIC_BYTE_0_VALUE == ctx->ActivateNtf.RfProtParams.NfcDep.PollAF.AtrRes[PTX_LLCP_ATRRES_MAGIC_BYTE_0_OFFSET]) &&
                 (PTX_LLCP_DEFAULT_MAGIC_BYTE_1_VALUE == ctx->ActivateNtf.RfProtParams.NfcDep.PollAF.AtrRes[PTX_LLCP_ATRRES_MAGIC_BYTE_1_OFFSET]) &&
                 (PTX_LLCP_DEFAULT_MAGIC_BYTE_2_VALUE == ctx->ActivateNtf.RfProtParams.NfcDep.PollAF.AtrRes[PTX_LLCP_ATRRES_MAGIC_BYTE_2_OFFSET]))
            {
                ctx->Role = Role_Initiator;
                *supported = 1;
            }
        }

    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_DisconnectLink(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_DisconnectLink");

        /* Triggers the disconnection of the whole physical link. */
        ctx->State = Link_Layer_Deactivation_Pending_Local;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_IsLinkDown(ptx_RUL_LLCP_t *ctx, uint8_t *isDown)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != isDown))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_IsLinkDown");

        /* Check if the state machine is still running. */
        if (0 == ctx->Running)
        {
            *isDown = 1;
        }
        else
        {
            switch (ctx->State)
            {
                case Link_Layer_Deactivated_Remote:         /* Fall through */
                    if (ctx->Role == Role_Target)
                    {
                        *isDown = 0;
                    }
                    else
                    {
                        *isDown = 1;
                    }
                    break;
                case Link_Layer_Deactivated_Local:          /* Fall through */
                case Link_Layer_Deactivation_Pending_Local: /* Fall through */
                case MAC_Layer_Deactivation_Pending:
                    *isDown = 1;
                    break;
                default:
                    *isDown = 0;
                    break;
            }
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_AllocateLink(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t **link)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != link))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_AllocateLink");

        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;

        for (uint8_t i = 0; (i < PTX_LLCP_MAX_LINKS) && (PTX_STATUS_SUCCESS != status); ++i)
        {
            if (0 == ctx->Link[i].InUse)
            {
                *link = &ctx->Link[i];
                status = PTX_STATUS_SUCCESS;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_GetLinkFromUri(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t **link, uint8_t requestBySDP, char* name, uint32_t len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != link) && (NULL != name))
    {
        PTX_LOG_I(&(ctx->Rul->Log), "ptx_RUL_LLCP_GetLinkFromUri");

        *link = NULL;
        for (uint32_t i = 0; (i < PTX_LLCP_MAX_LINKS) && (PTX_STATUS_SUCCESS != status); ++i)
        {
            if ((ctx->Link[i].SdpState != SDP_NotAdvertised) || (!requestBySDP))
            {
                if ((0 == strncmp(name, ctx->Link[i].ServiceUri, len)) && ((uint32_t) strlen(ctx->Link[i].ServiceUri) == len))
                {
                    *link = &ctx->Link[i];
                    status = PTX_STATUS_SUCCESS;
                }
            }
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACKS
 * ####################################################################################################################
 */
ptx_Status_t ptx_RUL_LLCP_RegisterExtension (ptx_RUL_LLCP_t *ctx)
{
    ptx_RUL_Extension_Endpoints_t endpoints;
    memset(&endpoints, 0, sizeof(ptx_RUL_Extension_Endpoints_t));

    PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_RegisterExtension");

    endpoints.Extension = ctx;
    endpoints.CompId = Comp_LLCP;
    endpoints.ProcessRfActivate = ptx_RUL_LLCP_CB_ProcessActivateNtf;
    endpoints.ProcessRfDeactivate = ptx_RUL_LLCP_CB_ProcessDeactivateNtf;
    endpoints.ProcessRfField = ptx_RUL_LLCP_CB_ProcessRfFieldNtf;
    endpoints.ProcessDataMsg = ptx_RUL_LLCP_CB_ProcessDataMsg;

    return ptx_RUL_RegisterExtensionEndpoints(ctx->Rul, &endpoints);
}

ptx_Status_t ptx_RUL_LLCP_CB_ProcessActivateNtf(void *ctx, ptx_NSC_RfActivate_Ntf_t *ntf, uint8_t *ntf_used)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_LLCP_t *llcp = ctx;
    if (PTX_COMP_CHECK(llcp, Comp_LLCP) && (NULL != ntf) && (NULL != ntf_used))
    {
        PTX_LOG_V(&(llcp->Rul->Log), "ptx_RUL_LLCP_RegisterExtension");

        memcpy(&llcp->ActivateNtf, ntf, sizeof(ptx_NSC_RfActivate_Ntf_t));

        uint8_t supported = 0;
        status = ptx_RUL_LLCP_IsLlcpSupported(llcp, &supported);

        if ((PTX_STATUS_SUCCESS == status) && (0 != supported))
        {

            llcp->Initiate = 1u;
            llcp->LastSymmPdu = llcp->TimerTicks;
            *ntf_used = 1;


            ptx_RUL_LLCP_TLV_Handler_t tlv;
            ptx_RUL_LLCP_TLV_Open(&tlv);

            uint8_t *data;
            uint32_t dataLen;
            if (llcp->Role == Role_Initiator)
            {
                data = &llcp->ActivateNtf.RfProtParams.NfcDep.PollAF.AtrRes[PTX_LLCP_ATRES_LEN_OFFSET];
                dataLen = llcp->ActivateNtf.RfProtParams.NfcDep.PollAF.AtrRes[0] - PTX_LLCP_ATRES_LEN_OFFSET;
            }
            else
            {
                data = &llcp->ActivateNtf.RfProtParams.NfcDep.ListenAF.AtrReq[PTX_LLCP_ATREQ_LEN_OFFSET];
                dataLen = llcp->ActivateNtf.RfProtParams.NfcDep.ListenAF.AtrReq[0] - PTX_LLCP_ATREQ_LEN_OFFSET;
            }

            llcp->ActivationTlvsLen = PTX_LLCP_MAX_TLV_PARAMS;
            memset(llcp->ActivationTlvs, 0, sizeof(llcp->ActivationTlvs));
            ptx_RUL_LLCP_TLV_ProcessReceivedData(&tlv, data, dataLen, llcp->ActivationTlvs, &llcp->ActivationTlvsLen);

            for (uint32_t i = 0; i < llcp->ActivationTlvsLen; ++i)
            {
                switch (llcp->ActivationTlvs[i].Type)
                {
                    case LLCP_TagType_Version:
                        llcp->PeerLlcpVerison = llcp->ActivationTlvs[i].Data.Version.Minor;
                        break;
                    case LLCP_TagType_WKS:
                        llcp->PeerLlcpWks = llcp->ActivationTlvs[i].Data.Wks.ServiceList;
                        break;
                    case LLCP_TagType_MIUX:
                        llcp->PeerLlcpMiux = llcp->ActivationTlvs[i].Data.Miux.Miux;
                        break;
                    case LLCP_TagType_LTO:
                        llcp->TargetLinkTimeout = llcp->ActivationTlvs[i].Data.Lto.Timeout;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_CB_ProcessDeactivateNtf(void *ctx, ptx_NSC_RfDeactivate_Ntf_t *ntf, uint8_t *ntf_used)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_LLCP_t *llcp = ctx;

    if (PTX_COMP_CHECK(llcp, Comp_LLCP) && (NULL != ntf) && (NULL != ntf_used))
    {
        PTX_LOG_V(&(llcp->Rul->Log), "ptx_RUL_LLCP_CB_ProcessDeactivateNtf");

        llcp->Initiate = 0u;
        if ((NSC_ErrorCode_NoErr == ntf->Error) && ((ptx_NSC_RfDeactivate_Reason_RF_Field == ntf->Reason) || (ptx_NSC_RfDeactivate_Reason_Deselect == ntf->Reason)))
        {
            llcp->RfFieldPresent = 0;
        }
        memset(&llcp->ActivateNtf, 0, sizeof(ptx_NSC_RfActivate_Ntf_t));
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_CB_ProcessRfFieldNtf(void *ctx, uint8_t field, uint8_t *ntf_used)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_LLCP_t *llcp = ctx;

    if (PTX_COMP_CHECK(llcp, Comp_LLCP) && (NULL != ntf_used))
    {
        
        PTX_LOG_V(&(llcp->Rul->Log), "ptx_RUL_LLCP_CB_ProcessDeactivateNtf");

        llcp->RfFieldPresent = field;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_RUL_LLCP_CB_ProcessDataMsg(void *ctx, ptx_NSC_RfData_Msg_t *msg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_LLCP_t *llcp_ctx = (ptx_RUL_LLCP_t*) ctx;

    if (PTX_COMP_CHECK(llcp_ctx, Comp_LLCP) && (NULL != msg))
    {
        PTX_LOG_V(&(llcp_ctx->Rul->Log), "ptx_RUL_LLCP_CB_ProcessDeactivateNtf");

        if ((llcp_ctx->RxDataIntLen + msg->CtrlLen.DataLen) <= PTX_LLCP_FRAME_BUFFER_LEN)
        {
            memcpy(&llcp_ctx->RxData[llcp_ctx->RxDataIntLen], msg->Data, msg->CtrlLen.DataLen);
            llcp_ctx->RxDataIntLen += msg->CtrlLen.DataLen;
        }

        if (ptx_NSC_RfData_Msg_Chaining_None == msg->Fcb.Chaining)
        {
            llcp_ctx->RxDataLen = llcp_ctx->RxDataIntLen;
            llcp_ctx->RxDataIntLen = 0;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_CB_Timer(ptx_HAL_DRV_Tmr_ExecCbCtx_t ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_LLCP_t *llcp = ctx;

    if (PTX_COMP_CHECK(llcp, Comp_LLCP))
    {
        PTX_LOG_V(&(llcp->Rul->Log), "ptx_RUL_LLCP_CB_ProcessDeactivateNtf");

        ++llcp->TimerTicks;
        status = PTX_STATUS_SUCCESS;
    }
    return status;
}

ptx_Status_t ptx_RUL_LLCP_ConfigureTimer(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status;

    status = ptx_HAL_Tmr_RegExecCb (&(ctx->Rul->Hal), HAL_Tmr_Id_1, ptx_RUL_LLCP_CB_Timer, ctx);

    if ((PTX_STATUS_SUCCESS == status))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_ConfigureTimer");

        ptx_HAL_DRV_Tmr_Cfg_t cfg[1];

        cfg[0].CfgType = HAL_DRV_Tmr_CfgType_Mode;
        cfg[0].Mode = HAL_DRV_Tmr_Mode_Periodic;
        status = ptx_HAL_Tmr_Cfg(&(ctx->Rul->Hal), HAL_Tmr_Id_1, cfg, 1);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        ctx->TimerTicks = 0;
        status = ptx_HAL_Tmr_Start(&(ctx->Rul->Hal), HAL_Tmr_Id_1, HAL_DRV_Tmr_Unit_mSec, PTX_LLCP_DEFAULT_TIMER_TICK_UNIT);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_IsSapFree(ptx_RUL_LLCP_t *ctx, uint8_t address, uint8_t *free)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (PTX_LLCP_SAP_ADDRESS_MAX > address) && (NULL != free))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_IsSapFree");

        *free =  (0 == (ctx->SapList & (1 << address)) ? 1 : 0);
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_ReserveSap(ptx_RUL_LLCP_t *ctx, uint8_t address)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (PTX_LLCP_SAP_ADDRESS_MAX > address))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_ReserveSap");

        ctx->SapList |= 1 << address;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_FreeSap(ptx_RUL_LLCP_t *ctx, uint8_t address)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (PTX_LLCP_SAP_ADDRESS_MAX > address))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_FreeSap");

        ctx->SapList &= ~((uint64_t)(1 << address));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_SendMessageBuffer(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_MessageBuffer_t *buffer)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != buffer))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_SendMessageBuffer");

        uint8_t tx_data[PTX_LLCP_MESSAGEBUFFER_MESSAGELENGTH];
        uint32_t tx_length = sizeof(tx_data);
        status = ptx_RUL_LLCP_MessageBuffer_Read(buffer, tx_data, &tx_length, 1u);

        if (PTX_STATUS_E_GEN_NOT_FOUND == status)
        {
            if (Role_Target == ctx->Role)
            {
                ptx_RUL_Sleep(ctx->Rul, 1);
                ptx_RUL_LLCP_PDU_Sym(&ctx->LlcpFrame);
                status = ptx_RUL_Send(ctx->Rul, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                ctx->LastSymmPdu = ctx->TimerTicks;
                ctx->Turn = Role_Initiator;
            }
            else
            {
                uint32_t now = ctx->TimerTicks;
                if ((now - ctx->LastSymmPdu) >= (ctx->TargetLinkTimeout - 1u))
                {
                    ptx_RUL_LLCP_PDU_Sym(&ctx->LlcpFrame);
                    status = ptx_RUL_Send(ctx->Rul, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                    ctx->LastSymmPdu = ctx->TimerTicks;
                    ctx->Turn = Role_Target;
                }
                else
                {
                    status = PTX_STATUS_SUCCESS;
                }
            }
        }
        else if ((PTX_STATUS_SUCCESS == status) && (0 != tx_length))
        {
            status = ptx_RUL_Send(ctx->Rul, tx_data, tx_length);
            if (Role_Target == ctx->Role)
            {
                ctx->Turn = Role_Initiator;
            }
            else
            {
                ctx->Turn = Role_Target;
            }

            ctx->LastSymmPdu = ctx->TimerTicks;

            ptx_RUL_LLCP_PduType_t type = ((tx_data[0] << PTX_LLCP_PDU_PTYPE_OFFSET_LB) & 0x0C) | ((tx_data[1] >> PTX_LLCP_PDU_PTYPE_OFFSET_HB) & 0x03);
            if (LLCP_PduType_I == type)
            {
                ctx->SentI = 1;
            }
            else if (LLCP_PduType_UI == type)
            {
                ctx->SentUI = 1;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_DisconnectServiceLayer(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_DisconnectServiceLayer");

        status = PTX_STATUS_SUCCESS;

        for (uint8_t i = 0; (i < PTX_LLCP_MAX_LINKS) && (PTX_STATUS_SUCCESS == status); ++i)
        {
            if (0 != ctx->Link[i].InUse)
            {
                status = ptx_RUL_LLCP_Link_Disconnect(&ctx->Link[i]);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_ValidateServiceName(ptx_RUL_LLCP_t *ctx, char* name, uint32_t nameLength, uint8_t *valid)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != name) && (NULL != valid))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_ValidateServiceName");

        const char *prefix_1 = "urn:nfc:sn";
        const char *prefix_2 = "urn:nfc:xsn";
        const char postfix = ':';

        if ((0 == nameLength) || (PTX_LLCP_LINK_DEFAULT_SERVICE_URI_LENGTH < nameLength))
        {
            *valid = 0;
        }
        else if (!((strncmp(prefix_1, name, strlen(prefix_1)) == 0) || (strncmp(prefix_2, name, strlen(prefix_2)) == 0)))
        {
            *valid = 0;
        }
        else if (postfix == name[nameLength - 1])
        {
            *valid = 0;
        }
        else
        {
            *valid = 1;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_ProcessRxPdu(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_ProcessRxPdu");

        if ((Link_Layer_Deactivated_Remote == ctx->State) && (Role_Target == ctx->Role))
        {
            ptx_RUL_LLCP_MessageBuffer_Clear(&ctx->TxBuffer);
            ptx_RUL_LLCP_PDU_Disconnect(&ctx->LlcpFrame, PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT, PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT);
            ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            if (ctx->LinkTimeoutOccured)
            {
                pdu->Ptype = LLCP_PduType_DISC;
            }
            switch (pdu->Ptype)
            {
                case LLCP_PduType_SYMM:
                    ptx_RUL_LLCP_Processor_Symm(ctx, pdu);
                    break;

                case LLCP_PduType_PAX:
                    /* actually forbidded to be sent when NFC is used as MAC-layer...but here we just ignore it  */
                    break;

                case LLCP_PduType_AGF:
                    /* Nothing to do - already handled */
                    break;

                case LLCP_PduType_UI:
                    ptx_RUL_LLCP_Processor_Ui(ctx, pdu);
                    break;

                case LLCP_PduType_CONNECT:
                    ptx_RUL_LLCP_Processor_Connect(ctx, pdu);
                    break;

                case LLCP_PduType_DISC:
                    ptx_RUL_LLCP_Processor_Disconnect(ctx, pdu);
                    break;

                case LLCP_PduType_CC:
                    ptx_RUL_LLCP_Processor_ConnectionComplete(ctx, pdu);
                    break;

                case LLCP_PduType_DM:
                    ptx_RUL_LLCP_Processor_DisconnectMode(ctx, pdu);
                    break;

                case LLCP_PduType_FRMR:
                    ptx_RUL_LLCP_Processor_FrameReject(ctx, pdu);
                    break;

                case LLCP_PduType_SNL:
                    ptx_RUL_LLCP_Processor_ServiceNameLookup(ctx, pdu);
                    break;

                case LLCP_PduType_I:
                    ptx_RUL_LLCP_Processor_Information(ctx, pdu);
                    break;

                case LLCP_PduType_RR:
                    ptx_RUL_LLCP_Processor_ReceiveReady(ctx, pdu);
                    break;

                case LLCP_PduType_RNR:
                    ptx_RUL_LLCP_Processor_ReceiveNotReady(ctx, pdu);
                    break;

                case LLCP_PduType_RFU:
                    ptx_RUL_LLCP_Processor_RFU(ctx, pdu);
                    break;

                default:
                    break;
            }

            if (LLCP_PduType_SYMM != pdu->Ptype)
            {
                ctx->ReceivedPdu = 1u;
            }
        }
    }
    return status;
}

ptx_Status_t ptx_RUL_LLCP_GetLinkFromDestination(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu, ptx_RUL_LLCP_Link_t **link)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu) && (NULL != link))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_GetLinkFromDestination");

        status = PTX_STATUS_E_GEN_NOT_FOUND;

        *link = NULL;
        for (uint8_t i = 0; (i < PTX_LLCP_MAX_LINKS) && (PTX_STATUS_SUCCESS != status); ++i)
        {
            if (ctx->Link[i].Local.Address == pdu->Sap.Destination)
            {
                *link = &ctx->Link[i];
                status = PTX_STATUS_SUCCESS;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_AssembleRejectFrame(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, ptx_RUL_LLCP_PDU_t *pdu, uint8_t w, uint8_t i, uint8_t r, uint8_t s)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != link) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_AssembleRejectFrame");

        ptx_RUL_LLCP_RejectReport_t report;
        report.W = w;
        report.I = i;
        report.R = r;
        report.S = s;
        report.Ptype = pdu->Ptype;
        report.Vs = link->TrxStates.Vs;
        report.Vr = link->TrxStates.Vr;
        report.Vsa = link->TrxStates.Vsa;
        report.Vra = link->TrxStates.Vra;
        report.SendSequenceNumber = pdu->SendSequenceNumber;
        report.ReceiveSequenceNumber = pdu->ReceiveSequenceNumber;

        link->Local.Busy = 0;
        link->LinkState = LinkState_FrameRejected;

        status = ptx_RUL_LLCP_MessageBuffer_Clear(&link->Remote.MessageBuffer);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_LLCP_PDU_FrameReject(&ctx->LlcpFrame, link->Remote.Address, link->Local.Address, &report);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, (uint8_t) ctx->LlcpFrame.ScratchPadIndex);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_Symm(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_Symm");

        if ((NULL != ctx->ActiveLink) && (0 == ctx->ActiveLink->Local.Busy) && (0 == ctx->ActiveLink->BusyReleased))
        {
            uint32_t pending_messages;
            ptx_RUL_LLCP_MessageBuffer_Size(&ctx->ActiveLink->Local.MessageBuffer, &pending_messages);

            if (0 != pending_messages)
            {
                ctx->ActiveLink->BusyReleased = 1;
                ptx_RUL_LLCP_PDU_ReceiveReady(&ctx->LlcpFrame, ctx->ActiveLink->Remote.Address, ctx->ActiveLink->Local.Address, ctx->ActiveLink->TrxStates.Vr);
                ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);

                ptx_RUL_LLCP_Link_t *link = ctx->ActiveLink;
                ptx_RUL_LLCP_TrxState_SetReceiveAcknowledgeState(&link->TrxStates, link->TrxStates.Vr);
                link->Local.Busy = 0;
            }

        }
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}


ptx_Status_t ptx_RUL_LLCP_Processor_Ui(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_Ui");

        ptx_RUL_LLCP_Link_t *link = NULL;
        status = ptx_RUL_LLCP_GetLinkFromDestination(ctx, pdu, &link);

        if (PTX_STATUS_SUCCESS == status)
        {
            if ((pdu->InformationLen <= (uint8_t)link->Local.Miu) && (LinkType_ConnectionLess == link->Type))
            {
                if (pdu->InformationLen > link->Remote.Miu)
                {
                    for (uint8_t i = 0; (i < PTX_LLCP_MAX_LINKS); ++i)
                    {
                        ctx->Link[i].Remote.Miu = pdu->InformationLen;
                    }
                }


                status = ptx_RUL_LLCP_Link_WriteMessageInbox(link, pdu->Information, pdu->InformationLen);
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_Connect(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_Connect");

        uint32_t tlvs_len = PTX_LLCP_MAX_TLV_PARAMS;
        ptx_RUL_LLCP_TLV_Data_t tlvs[PTX_LLCP_MAX_TLV_PARAMS];

        if (0 != pdu->InformationLen)
        {
            ptx_RUL_LLCP_TLV_Handler_t tlv_handler;
            ptx_RUL_LLCP_TLV_Open(&tlv_handler);
            ptx_RUL_LLCP_TLV_ProcessReceivedData(&tlv_handler, pdu->Information, pdu->InformationLen, tlvs, &tlvs_len);
        }
        else
        {
            tlvs_len = 0;
        }

        ptx_RUL_LLCP_Link_t *target_link = NULL;

        if (PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT != pdu->Sap.Destination)
        {
            char sn_value[PTX_LLCP_LINK_DEFAULT_SERVICE_URI_LENGTH];
            memset(sn_value, 0, sizeof(sn_value));

            uint8_t sn_present = 0;
            uint8_t rw_value = 0;
            uint16_t miux_value = 0;

            for (uint8_t i = 0; i < tlvs_len; ++i)
            {
                switch (tlvs[i].Type)
                {
                    case LLCP_TagType_SN:
                        memcpy(sn_value, tlvs[i].Data.Sn.Name, tlvs[i].Data.Sn.Length);
                        sn_present = 1;
                        break;

                    case LLCP_TagType_MIUX:
                        miux_value = tlvs[i].Data.Miux.Miux;
                        break;

                    case LLCP_TagType_RW:
                        rw_value = tlvs[i].Data.Rw.WindowSize;
                        break;

                    default:
                        break;
                }
            }

            uint8_t valid_service_name = 0;
            if (PTX_LLCP_SAP_ADDRESS_SERVICE_DISCOVERY == pdu->Sap.Destination)
            {
                /* CONNECT via SDP (ignore PDU if SN is missing) */
                ptx_RUL_LLCP_ValidateServiceName(ctx, sn_value, (uint32_t) strlen(sn_value), &valid_service_name);
                if (0 != valid_service_name)
                {
                    ptx_RUL_LLCP_GetLinkFromUri(ctx, &target_link, 0, sn_value, (uint32_t) strlen(sn_value));
                }

            }
            else
            {
                /* CONNECT directly */
                ptx_RUL_LLCP_GetLinkFromDestination(ctx, pdu, &target_link);
            }

            if (NULL != target_link)
            {
                if (LinkType_ConnectionOriented == target_link->Type)
                {
                    if (LinkState_Connected != target_link->LinkState)
                    {
                        if (0 != miux_value)
                        {
                            target_link->Remote.Miu += miux_value;
                        }
                        if (0 != rw_value)
                        {
                            target_link->Remote.Window = rw_value;
                        }
                        target_link->Remote.Address = pdu->Sap.Source;
                        target_link->LinkState = LinkState_Connected;

                        ptx_RUL_LLCP_TLV_Data_t cc_tlvs[2];
                        cc_tlvs[0].Type = LLCP_TagType_MIUX;
                        cc_tlvs[0].Data.Miux.Miux = (uint16_t)(target_link->Local.Miu - PTX_LLCP_DEFAULT_MIU);

                        cc_tlvs[1].Type = LLCP_TagType_RW;
                        cc_tlvs[1].Data.Rw.WindowSize = target_link->Local.Window;

                        ptx_RUL_LLCP_PDU_ConnectionComplete(&ctx->LlcpFrame, target_link->Remote.Address, target_link->Local.Address, cc_tlvs, 2);
                        ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                    }
                    else
                    {
                        ptx_RUL_LLCP_AssembleRejectFrame(ctx, target_link, pdu, 1, 0, 0, 0);
                    }
                }
                else
                {
                    ptx_RUL_LLCP_PDU_DisconnectMode(&ctx->LlcpFrame, target_link->Remote.Address, target_link->Local.Address, DisconnectReason_NoServiceBound);
                    ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                }
            }
            else
            {
                if (0 != valid_service_name)
                {
                    if ((PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT == pdu->Sap.Destination) || (PTX_LLCP_SAP_ADDRESS_SERVICE_DISCOVERY == pdu->Sap.Destination))
                    {
                        ptx_RUL_LLCP_PDU_DisconnectMode(&ctx->LlcpFrame, pdu->Sap.Source, pdu->Sap.Destination, DisconnectReason_PermRejectSameSap);
                    }
                    else
                    {
                        ptx_RUL_LLCP_PDU_DisconnectMode(&ctx->LlcpFrame, pdu->Sap.Source, pdu->Sap.Destination, DisconnectReason_NoServiceBound);
                    }
                }
                else
                {
                    if (ctx->Role == Role_Initiator)
                    {
                        ptx_RUL_LLCP_PDU_DisconnectMode(&ctx->LlcpFrame, pdu->Sap.Source, pdu->Sap.Destination, DisconnectReason_NoServiceBound);
                    }
                    else
                    {
                        if (sn_present)
                        {
                            ptx_RUL_LLCP_PDU_DisconnectMode(&ctx->LlcpFrame, pdu->Sap.Source, pdu->Sap.Destination, DisconnectReason_NoServiceBound);
                        }
                        else
                        {
                            ptx_RUL_LLCP_PDU_DisconnectMode(&ctx->LlcpFrame, pdu->Sap.Source, pdu->Sap.Destination, DisconnectReason_ConnectRejected);
                        }
                    }
                }
                ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
            }
        }
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_Disconnect(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_Disconnect");

        ptx_RUL_LLCP_Link_t *link = NULL;
        ptx_RUL_LLCP_GetLinkFromDestination(ctx, pdu, &link);

        if (PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT == pdu->Sap.Destination || PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT == pdu->Sap.Source)
        {
            for (uint8_t i = 0; (i < PTX_LLCP_MAX_LINKS) && (PTX_STATUS_SUCCESS == status); ++i)
            {
                ptx_RUL_LLCP_MessageBuffer_Clear(&ctx->Link[i].Remote.MessageBuffer);
                ctx->Link[i].LinkState = LinkState_Disconnected;
            }
            ctx->State = Link_Layer_Deactivated_Remote;

            if (ctx->Role == Role_Target)
            {
                ptx_RUL_LLCP_MessageBuffer_Clear(&ctx->TxBuffer);
                ptx_RUL_LLCP_PDU_Disconnect(&ctx->LlcpFrame, 0, 0);
                status = ptx_RUL_Send(ctx->Rul, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                ctx->Turn = Role_Initiator;
            }
        }
        else if ((PTX_LLCP_SAP_ADDRESS_SERVICE_DISCOVERY == pdu->Sap.Destination) || (PTX_LLCP_SAP_ADDRESS_SERVICE_DISCOVERY == link->Local.Address))
        {
            /* OK */
        }
        else
        {
            if ((NULL != link) && (LinkType_ConnectionOriented == link->Type))
            {
                if ((LinkState_Connected == link->LinkState) || (LinkState_ConnectionPending == link->LinkState))
                {
                    link->LinkState = LinkState_Disconnected;
                    link->Remote.Busy = 0;
                    ptx_RUL_LLCP_MessageBuffer_Clear(&link->Remote.MessageBuffer);
                    ptx_RUL_LLCP_PDU_DisconnectMode(&ctx->LlcpFrame, link->Remote.Address, link->Local.Address, DisconnectReason_Disconnected);
                    ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                }
                else
                {
                    ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
                }
            }
        }
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_ConnectionComplete(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_ConnectionComplete");

        ptx_RUL_LLCP_Link_t *link = NULL;
        ptx_RUL_LLCP_GetLinkFromDestination(ctx, pdu, &link);

        if ((NULL != link) && (LinkState_ConnectionPending == link->LinkState))
        {
            uint32_t tlvs_len = PTX_LLCP_MAX_TLV_PARAMS;
            ptx_RUL_LLCP_TLV_Data_t tlvs[PTX_LLCP_MAX_TLV_PARAMS];
            ptx_RUL_LLCP_TLV_Handler_t tlv_handler;
            ptx_RUL_LLCP_TLV_Open(&tlv_handler);
            ptx_RUL_LLCP_TLV_ProcessReceivedData(&tlv_handler, pdu->Information, pdu->InformationLen, tlvs, &tlvs_len);

            if (0 == tlvs_len)
            {
                ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 1, 0, 0);
            }
            else
            {
                for (uint32_t i = 0; i < tlvs_len; ++i)
                {
                    switch (tlvs[i].Type)
                    {
                        case LLCP_TagType_MIUX:
                            link->Remote.Miu = PTX_LLCP_DEFAULT_MIU + tlvs[i].Data.Miux.Miux;
                            break;

                        case LLCP_TagType_RW:
                            link->Remote.Window = tlvs[i].Data.Rw.WindowSize;
                            break;

                        default:
                            break;
                    }
                }
                link->Remote.Address = pdu->Sap.Source;
                link->LinkState = LinkState_Connected;
            }
        }
        else
        {
            ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
        }
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_DisconnectMode(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_DisconnectMode");

        ptx_RUL_LLCP_Link_t *link = NULL;
        ptx_RUL_LLCP_GetLinkFromDestination(ctx, pdu, &link);


        if ((PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT == pdu->Sap.Destination) || (PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT == pdu->Sap.Source))
        {
            /* Acknowledge to previous DISC-PDU from local peer ? DM-PDU is unexpected but also not critical to get it ... just ignore it */
        }
        else if ((PTX_LLCP_SAP_ADDRESS_SERVICE_DISCOVERY == pdu->Sap.Destination) || (PTX_LLCP_SAP_ADDRESS_SERVICE_DISCOVERY == pdu->Sap.Source))
        {
            /* This is completely wrong - also just ignore it... */
        }
        else
        {
            /* DM-PDU is only applicable for connection-oriented Service Access Points - ignore it for connection-less types... */
            if (LinkType_ConnectionOriented == link->Type)
            {
                if ((LinkState_ConnectionPending == link->LinkState) || (LinkState_DisconnectionPending == link->LinkState))
                {
                    if(PTX_LLCP_PDU_MINIMUM_LEN == pdu->InformationLen)
                    {
                        ctx->LastDisconnectReason = pdu->Information[0];
                    }
                    /* OK - mark Data Link Connection as Disconnected now */
                    link->LinkState = LinkState_Disconnected;
                }
                else
                {
                    /* Invalid request - reject it */
                    ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
                }
            }
            else
            {
                /* Invalid request - reject it */
                ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
            }
        }
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_FrameReject(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_FrameReject");

        ptx_RUL_LLCP_Link_t *link = NULL;
        ptx_RUL_LLCP_GetLinkFromDestination(ctx, pdu, &link);

        /* FRMR is only allowed on connection-oriented Service Access Points */
        if (LinkType_ConnectionOriented == link->Type)
        {
            /* PDU format valid? */
            if (6u == ctx->LlcpFrame.ScratchPadIndex)
            {
                /* Inform Service Access Point about Frame-Rejection by remote peer */
                link->LinkState = LinkState_FrameRejected;
            }
            else
            {
                /* Reject frame */
                ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
            }
        }
        else
        {
            /* Reject frame */
            ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_ServiceNameLookup(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_ServiceNameLookup");

        if ((PTX_LLCP_SAP_ADDRESS_SERVICE_DISCOVERY == pdu->Sap.Destination) && (PTX_LLCP_SAP_ADDRESS_SERVICE_DISCOVERY == pdu->Sap.Source))
        {
            uint32_t tlvs_len = LLCP_SNL_TLVS;
            ptx_RUL_LLCP_TLV_Data_t tlvs[LLCP_SNL_TLVS];
            ptx_RUL_LLCP_TLV_Handler_t tlv_handler;
            ptx_RUL_LLCP_TLV_Open(&tlv_handler);
            ptx_RUL_LLCP_TLV_ProcessReceivedData(&tlv_handler, pdu->Information, pdu->InformationLen, tlvs, &tlvs_len);

            /* TLV-Params == null indicates invalid i.e. malformed PDU */
            for (uint32_t i = 0; i < tlvs_len; ++i)
            {
                switch (tlvs[i].Type)
                {
                    /* Only SDREQ and SDRES are supported for SNL - others shall be ignored */
                    /* Note: If the received PDU contains any format errors - processing will be cancelled i.e. PDU will be ignored */
                    case LLCP_TagType_SDREQ:
                    {
                        /* Remote Peer requests Service-ID from a given Service Name URI */
                        uint8_t sap_address = 0;
                        ptx_RUL_LLCP_Link_t *target_link = NULL;

                        uint8_t tid = tlvs[i].Data.SdReq.TransactionIdentifier;
                        ptx_RUL_LLCP_GetLinkFromUri(ctx, &target_link, 0u, tlvs[i].Data.SdReq.ServiceName.Name, tlvs[i].Data.SdReq.ServiceName.Length);

                        if (Link_Layer_Deactivated_Remote == ctx->State)
                        {
                            ptx_RUL_LLCP_MessageBuffer_Clear(&ctx->TxBuffer);
                            ptx_RUL_LLCP_PDU_Disconnect(&ctx->LlcpFrame, 0, 0);
                            ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);

                        }
                        else
                        {
                            if (0 != strncmp(PTX_DTA_LLCP_SVC_DTA_PATTERN_NUMBER_NAME, tlvs[i].Data.SdReq.ServiceName.Name, strlen(PTX_DTA_LLCP_SVC_DTA_PATTERN_NUMBER_NAME)))
                            {
                                if (NULL != target_link && 0 != tlvs[i].Data.SdReq.ServiceName.Length)
                                {
                                    sap_address = target_link->Local.Address;
                                }
                                else if (0 == strncmp(PTX_DTA_LLCP_SVC_DISCOVERY_PROT_SERVICE_NAME, tlvs[i].Data.SdReq.ServiceName.Name, strlen(PTX_DTA_LLCP_SVC_DISCOVERY_PROT_SERVICE_NAME)))
                                {
                                    sap_address = PTX_DTA_LLCP_SVC_DISCOVERY_PROT_SAP;
                                }

                                ptx_RUL_LLCP_TLV_Data_t tlv;
                                tlv.Type = LLCP_TagType_SDRES;
                                tlv.Data.SdRes.TransactionIdentifier = tid;
                                tlv.Data.SdRes.ConnectServiceName = 0;
                                tlv.Data.SdRes.ServiceAccessPoint = sap_address;

                                /* Send response SLN with SAP valid SAP value or with value 0 indicating that the Service Access Point is not available or not advertised via SNL */
                                ptx_RUL_LLCP_PDU_ServiceNameLookup(&ctx->LlcpFrame, &tlv, 1);
                                ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                            }
                        }
                    }
                    break;

                    case LLCP_TagType_SDRES:
                        /* Local Peer requested Service-ID from a given Service Name URI */
                        ctx->RemoteServiceId = tlvs[i].Data.SdRes.ServiceAccessPoint;
                        break;

                    default:
                        break;
                }
            }
        }
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_Information(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_Information");

        ptx_RUL_LLCP_Link_t *link = NULL;
        status = ptx_RUL_LLCP_GetLinkFromDestination(ctx, pdu, &link);

        if (PTX_STATUS_SUCCESS == status && (LinkType_ConnectionOriented != link->Type))
        {
            /* Reject frame */
            ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if (PTX_STATUS_SUCCESS == status && (LinkState_Connected != link->LinkState))
        {
            ptx_RUL_LLCP_PDU_DisconnectMode(&ctx->LlcpFrame, link->Remote.Address, link->Local.Address, DisconnectReason_NoActiveConnection);
            ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if (PTX_STATUS_SUCCESS == status && (0 != link->Remote.Busy))
        {
            /* Remote-Busy Condition => Remote is only allowed to send RR-PDU! */
            ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t nr_valid = 0;
            ptx_RUL_LLCP_Link_ValidateNrSequence(link, pdu->ReceiveSequenceNumber, &nr_valid);
            if (0 == nr_valid)
            {
                /* Wrong NR-sequence detected => reject frame */
                ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 0, 0, 1, 0);
                status = PTX_STATUS_E_GEN_INVALID_STATE;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t ns_valid;
            ptx_RUL_LLCP_Link_ValidateNsSequence(link, pdu->SendSequenceNumber, &ns_valid);
            if (0 == ns_valid)
            {
                /* Wrong NS-sequence detected => reject frame */
                ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 0, 0, 0, 1);
                status = PTX_STATUS_E_GEN_INVALID_STATE;
            }
        }

        if ((PTX_STATUS_SUCCESS == status) && (pdu->InformationLen > link->Local.Miu))
        {
            /* Invalid information length detected => reject frame */
            ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 0, 1, 0, 0);
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if ((PTX_STATUS_SUCCESS == status) && (0 != link->Local.Busy))
        {
            /* Local RW exceeded => reject frame */
            ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 0, 0, 0, 1);
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Everything OK */
            ptx_RUL_LLCP_TrxState_SetSendAcknowledgeState(&link->TrxStates, pdu->ReceiveSequenceNumber);
            ptx_RUL_LLCP_TrxState_IncrementReceiveState(&link->TrxStates);
            ptx_RUL_LLCP_Link_WriteMessageInbox(link, pdu->Information, pdu->InformationLen);

            ptx_RUL_LLCP_PDU_ReceiveReady(&ctx->LlcpFrame, link->Remote.Address, link->Local.Address, link->TrxStates.Vr);
            ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);

            ptx_RUL_LLCP_TrxState_SetReceiveAcknowledgeState(&link->TrxStates, link->TrxStates.Vr);
            link->Local.Busy = 0;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_ReceiveReady(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_ReceiveReady");

        ptx_RUL_LLCP_Link_t *link = NULL;
        status = ptx_RUL_LLCP_GetLinkFromDestination(ctx, pdu, &link);

        if ((PTX_STATUS_SUCCESS == status) && (LinkType_ConnectionOriented != link->Type))
        {
            /* Reject frame */
            ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if ((PTX_STATUS_SUCCESS == status) && (link->LinkState != LinkState_Connected))
        {
            ptx_RUL_LLCP_PDU_DisconnectMode(&ctx->LlcpFrame, link->Remote.Address, link->Local.Address, DisconnectReason_NoActiveConnection);
            ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t nr_valid;
            ptx_RUL_LLCP_Link_ValidateNrSequence(link, pdu->ReceiveSequenceNumber, &nr_valid);
            if (0 == nr_valid)
            {
                /* Wrong NR-sequence detected => reject frame */
                ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 0, 0, 1, 0);
                status = PTX_STATUS_E_GEN_INVALID_STATE;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Everything OK */
            ptx_RUL_LLCP_TrxState_SetSendAcknowledgeState(&link->TrxStates, pdu->ReceiveSequenceNumber);
            /* Remote Peer is in any case ready to receive new I-PDUs */
            link->Remote.Busy = 0;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_ReceiveNotReady(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_ReceiveNotReady");

        ptx_RUL_LLCP_Link_t *link = NULL;
        status = ptx_RUL_LLCP_GetLinkFromDestination(ctx, pdu, &link);

        if ((PTX_STATUS_SUCCESS == status) && (LinkType_ConnectionOriented != link->Type))
        {
            ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if ((PTX_STATUS_SUCCESS == status) && (link->LinkState != LinkState_Connected))
        {
            ptx_RUL_LLCP_PDU_DisconnectMode(&ctx->LlcpFrame, link->Remote.Address, link->Local.Address, DisconnectReason_NoActiveConnection);
            ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if ((PTX_STATUS_SUCCESS == status) && (0 != link->Remote.Busy))
        {
            /* Remote-Busy Condition => Remote is only allowed to send RR-PDU! */
            ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t nr_valid;
            ptx_RUL_LLCP_Link_ValidateNrSequence(link, pdu->ReceiveSequenceNumber, &nr_valid);
            /* NR value sent by remote has to match local NS value! */
            if (0 == nr_valid)
            {
                ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 0, 0, 1, 0);
                status = PTX_STATUS_E_GEN_INVALID_STATE;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Everything OK */
            ptx_RUL_LLCP_TrxState_SetSendAcknowledgeState(&link->TrxStates, pdu->ReceiveSequenceNumber);
            /* Set Busy-flag of remote Peer */
            link->Remote.Busy = 1u;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_RFU(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != pdu))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_RFU");

        ptx_RUL_LLCP_Link_t *link = NULL;
        ptx_RUL_LLCP_GetLinkFromDestination(ctx, pdu, &link);

        if (LinkType_ConnectionOriented == link->Type)
        {
            if (LinkState_Connected == link->LinkState)
            {
                ptx_RUL_LLCP_AssembleRejectFrame(ctx, link, pdu, 1, 0, 0, 0);
            }
            else
            {
                ptx_RUL_LLCP_PDU_DisconnectMode(&ctx->LlcpFrame, link->Remote.Address, link->Local.Address, DisconnectReason_NoActiveConnection);
                ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
            }
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_HandleInitialCondition(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_HandleInitialCondition");

        if (Role_Initiator == ctx->Role)
        {
            ptx_RUL_LLCP_PDU_Sym(&ctx->LlcpFrame);
            ptx_RUL_Send(ctx->Rul, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);

            ctx->LastSymmPdu = ctx->TimerTicks;
            ctx->Running = 1;
            ctx->Initiate = 0;
            ctx->Turn = Role_Target;
        }
        else
        {
            if (0 != ctx->RxDataLen)
            {
                ctx->Running = 1;
                ctx->Initiate = 0;
                ctx->Turn = Role_Target;
            }
        }
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_ValidateLinkTimeout(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_ValidateLinkTimeout");

        if (Role_Target == ctx->Role)
        {
            if (0 == ctx->RfFieldPresent)
            {
                ctx->State = MAC_Layer_Deactivated;
            }

            if ((ctx->TimerTicks - ctx->LastSymmPdu) > ctx->TargetLinkTimeout)
            {
                ctx->LinkTimeoutOccured = 1;
            }
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_CheckRfErrors(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_CheckRfErrors");

        if (Role_Initiator == ctx->Role)
        {
            if (ptx_RUL_StatusInfoRfError_ErrorNone != ctx->Rul->RfErrorStatus)
            {
                ctx->State = MAC_Layer_Deactivation_Pending;
            }
        }
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_ConnectionTeardown(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_ConnectionTeardown");

        ctx->Running = 0;

        ptx_RUL_DeactivateParams_t deact_params;
        deact_params.Type = ptx_RUL_DeactivateType_Specific;
        deact_params.State = (ctx->Role != Role_Target ? ptx_RUL_DeactivateState_Poll : ptx_RUL_DeactivateState_ListenSleep);
        status = ptx_RUL_Deactivate(ctx->Rul, &deact_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            deact_params.Type = ptx_RUL_DeactivateType_Generic;
            deact_params.State = (ctx->Role != Role_Target ? ptx_RUL_DeactivateState_Poll : ptx_RUL_DeactivateState_ListenIdle);
            status = ptx_RUL_Deactivate(ctx->Rul, &deact_params);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor_HandleStates(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor_HandleStates");

        switch (ctx->State)
        {
            case MAC_Layer_Activated:
            {
                ctx->Running = 0;
            }
            break;

            case Link_Layer_Deactivated_Remote:
            {
                /* Inform complete Servce Layer about Link disconnection */
                ptx_RUL_LLCP_DisconnectServiceLayer(ctx);
                /* Clear any pending data. */
                ptx_RUL_LLCP_MessageBuffer_Clear(&ctx->TxBuffer);

                if (Role_Initiator == ctx->Role)
                {
                    ctx->Running = 0;
                }
                else
                {
                    /*
                        * NFC Forum Special Case:
                        * If the following conditions are met:
                        * a) MAC-Mapping == NFC-DEP,
                        * b) the local Peer is in Target-Mode
                        * c) the local Peer has received a DISC-PDU
                        * then, the local peer continues processing LLCP-packets, but replies to incoming packets with a DISC-PDU ...
                        */
                    ptx_RUL_LLCP_PDU_Disconnect(&ctx->LlcpFrame, PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT, PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT);
                    ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                    ptx_RUL_LLCP_SendMessageBuffer(ctx, &ctx->TxBuffer);
                }
            }
            break;

            case MAC_Layer_Deactivated:
            case Link_Layer_Deactivated_Local:
            case MAC_Layer_Deactivation_Pending:
            {
                ptx_RUL_LLCP_DisconnectServiceLayer(ctx);
                ptx_RUL_LLCP_MessageBuffer_Clear(&ctx->TxBuffer);
                ctx->Running = 0;
            }
            break;

            case Link_Layer_Activated:
            {
                /* All good */
            }
            break;

            case Link_Layer_Deactivation_Pending_Local:
            {
                ptx_RUL_LLCP_MessageBuffer_Clear(&ctx->TxBuffer);
                ptx_RUL_LLCP_PDU_Disconnect(&ctx->LlcpFrame, PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT, PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT);
                ptx_RUL_LLCP_MessageBuffer_Write(&ctx->TxBuffer, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                ptx_RUL_LLCP_SendMessageBuffer(ctx, &ctx->TxBuffer);
                ctx->State = Link_Layer_Deactivated_Local;
            }
            break;
        }
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Processor(ptx_RUL_LLCP_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_Processor");

        if (0 != ctx->Initiate)
        {
            status = ptx_RUL_LLCP_Processor_HandleInitialCondition(ctx);
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }

        if (0 != ctx->Running)
        {
            if (status == PTX_STATUS_SUCCESS)
            {
                status = ptx_RUL_LLCP_Processor_ValidateLinkTimeout(ctx);
            }

            if (status == PTX_STATUS_SUCCESS)
            {
                status = ptx_RUL_LLCP_Processor_HandleStates(ctx);
            }

            if ((status == PTX_STATUS_SUCCESS) && (0 != ctx->RxDataLen))
            {
                if (ctx->State == Link_Layer_Deactivated_Remote && ctx->Role == Role_Initiator)
                {
                    ptx_RUL_LLCP_MessageBuffer_Clear(&ctx->TxBuffer);
                    ptx_RUL_LLCP_PDU_Disconnect(&ctx->LlcpFrame, 0, 0);
                    status = ptx_RUL_Send(ctx->Rul, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                    ctx->Turn = Role_Initiator;
                }
                else
                {
                    ctx->Turn = ctx->Role;

                    /* Prepare all the contexts */
                    ptx_RUL_LLCP_Frame_t frame_ctx;
                    ptx_RUL_LLCP_PDU_t rx_pdus[PTX_LLCP_MAX_AGGREGATED_PDUS];
                    uint32_t rx_pdus_len = PTX_LLCP_MAX_AGGREGATED_PDUS;

                    /* Open Frame */
                    ptx_RUL_LLCP_Frame_Open(&frame_ctx);
                    /* Copy all the received data into the Scratchpad for further interpretation/parsing. */
                    frame_ctx.ScratchPadIndex = ctx->RxDataLen;
                    memcpy(frame_ctx.ScratchPad, ctx->RxData, frame_ctx.ScratchPadIndex);
                    /* Reset the index of the receive buffer to be ready for receiving again. */
                    ctx->RxDataLen = 0;

                    /* Parse all the received PDUs. */
                    ptx_RUL_LLCP_Frame_Parse(&frame_ctx, rx_pdus, &rx_pdus_len);

                    /* Did we run into a link timeout? */
                    if (0 == ctx->LinkTimeoutOccured)
                    {
                        /* Process the received data PDU by PDU. */
                        for (uint8_t i = 0; i < rx_pdus_len; ++i)
                        {
                            ptx_RUL_LLCP_ProcessRxPdu(ctx, &rx_pdus[i]);
                        }
                    }
                    else
                    {
                        /* In case of a link timeout clear all the data we previously received. */
                        ptx_RUL_LLCP_MessageBuffer_Clear(&ctx->TxBuffer);
                        ptx_RUL_LLCP_PDU_Disconnect(&ctx->LlcpFrame, 0, 0);
                        ptx_RUL_Send(ctx->Rul, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                    }
                }
            }
            else
            {
                status = ptx_RUL_LLCP_Processor_CheckRfErrors(ctx);
            }

            uint8_t is_down = 0;
            ptx_RUL_LLCP_IsLinkDown(ctx, &is_down);
            if ((PTX_STATUS_SUCCESS == status) && (0 == is_down))
            {
                if (ctx->Role == ctx->Turn)
                {
                    if (0 == ctx->LinkTimeoutOccured)
                        status = ptx_RUL_LLCP_SendMessageBuffer(ctx, &ctx->TxBuffer);
                    else
                    {
                        ptx_RUL_LLCP_PDU_Disconnect(&ctx->LlcpFrame, 0, 0);
                        ptx_RUL_Send(ctx->Rul, ctx->LlcpFrame.ScratchPad, ctx->LlcpFrame.ScratchPadIndex);
                    }
                }
            }
            else
            {
                status = ptx_RUL_LLCP_Processor_ConnectionTeardown(ctx);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_WaitForAcknowledge(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != link))
    {
        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_WaitForAcknowledge");

        if (Link_Layer_Activated == ctx->State)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_LLCP_DISCONNECTED_BY_REMOTE;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (LinkType_ConnectionOriented == link->Type)
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_LLCP_INVALID_PARAMETER;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (LinkState_Connected == link->LinkState)
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_LLCP_CONNECTION_DISCONNECTED_BY_REMOTE;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Wait for Acknolwedgement  */
            status = ptx_RUL_LLCP_SetupReceiveStatusHandler(ctx, link);

            while (  (PTX_STATUS_SUCCESS == status) &&
                     (LinkState_Connected == link->LinkState) &&
                     (link->TrxStates.Vs != link->TrxStates.Vsa))
            {
                status = ptx_RUL_LLCP_Handle(ctx);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_RUL_LLCP_CheckReceiveStatusHandler(ctx);
                }
            }
        }
    }

    return status;
}


ptx_Status_t ptx_RUL_LLCP_IsLinkConnected(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, uint8_t *connected)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP) && (NULL != link) && (NULL != connected))
    {
        *connected = 0;

        PTX_LOG_V(&(ctx->Rul->Log), "ptx_RUL_LLCP_IsLinkConnected");

        if (LinkType_ConnectionOriented == link->Type)
        {
            /* In case of a connection oriented SAP, we simply evaluate the link state. */
            if ((LinkState_Connected == link->LinkState))
            {
                *connected = 1u;
            }
        }
        else
        {
            /*
             * In case of a connection less SAP, we consider is as "connected" if there is a message in the inbox.
             * This behavior is first and foremost needed for NFC Forum DTA testing.
             */
            uint32_t messages = 0;
            ptx_RUL_LLCP_MessageBuffer_Size(&link->Local.MessageBuffer, &messages);

            if (0 != messages)
            {
                *connected = 1u;
            }
        }
    }

    return status;
}
