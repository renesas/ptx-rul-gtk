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
    File        : ptx_RUL_LLCP_Pdu.c

    Description : Implementation of the Logical Link Control Protocol Data Unit (LLCP PDU).
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL_LLCP_Pdu.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_LLCP_PDU_PAX_MAX_TLV_LEN        (5u)    /**< Maximum number of TLVs for PAX: PAX MIUX, WKS, LTO, OPT */
#define PTX_LLCP_PDU_CONNECT_MAX_TLV_LEN    (3u)    /**< Maximum number of TLVs for CONNECT: MIUX, RW, SN */
#define PTX_LLCP_PDU_CC_MAX_TLV_LEN         (2u)    /**< Maximum number of TLVs for CC: MIUX, RW */
#define PTX_LLCP_PDU_SNL_MAX_TLV_LEN        (2u)    /**< Maximum number of TLVs for SNL: SDREQ, SDRES */
#define PTX_LLCP_PDU_DPS_MAX_TLV_LEN        (2u)    /**< Maximum number of TLVs for DPS: CPK, RN */
/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
ptx_Status_t ptx_RUL_LLCP_PDU_Assemble(ptx_RUL_LLCP_PDU_t *ctx, ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvLength);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_RUL_LLCP_PDU_Sym (ptx_RUL_LLCP_Frame_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Prepare the PDU datastructure */
    ptx_RUL_LLCP_PDU_t pdu;
    memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

    /* SYM PDU is simple, we just need to set the type */
    pdu.Ptype = LLCP_PduType_SYMM;

    /* Write the PDU to the scratchpad */
    status = ptx_RUL_LLCP_Frame_WritePdu(ctx, &pdu);

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_Pax (ptx_RUL_LLCP_Frame_t *ctx, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvsLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != tlvs) && (0 != tlvsLength) && (PTX_LLCP_PDU_PAX_MAX_TLV_LEN >= tlvsLength))
    {
        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /* PAX PDU is simple, we just need to set the type */
        pdu.Ptype = LLCP_PduType_PAX;

        /* Assemble the TLV datastructure(s) */
        ptx_RUL_LLCP_TLV_Handler_t tlv_handler;
        (void) ptx_RUL_LLCP_TLV_Open (&tlv_handler);
        status = ptx_RUL_LLCP_PDU_Assemble(&pdu, &tlv_handler, tlvs, tlvsLength);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Write the PDU to the scratchpad */
            status = ptx_RUL_LLCP_Frame_WritePdu(ctx, &pdu);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_Agf (ptx_RUL_LLCP_Frame_t *ctx, uint8_t *pdus, uint8_t pdusLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != pdus) && (0 != pdusLen))
    {
        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /* Assemble the PDU structure */
        pdu.Ptype              = LLCP_PduType_AGF;
        pdu.Information        = pdus;
        pdu.InformationLen     = pdusLen;

        /* Write the PDU to the scratchpad */
        status = ptx_RUL_LLCP_Frame_WritePdu(ctx, &pdu);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_Ui (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, uint8_t *information, uint8_t informationLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_SAP_CHECK(PTX_LLCP_PDU_DSAP_MASK, dsap) && PTX_SAP_CHECK(PTX_LLCP_PDU_SSAP_MASK, ssap) && (NULL != information) && (0 < informationLen))
    {
        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /* Set up the data structure */
        pdu.Sap.Destination    = dsap;
        pdu.Sap.Source         = ssap;
        pdu.Ptype              = LLCP_PduType_UI;
        pdu.Information        = information;
        pdu.InformationLen     = informationLen;

        /* Write the PDU to the scratchpad */
        status = ptx_RUL_LLCP_Frame_WritePdu(ctx, &pdu);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_Connect (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_SAP_CHECK(PTX_LLCP_PDU_DSAP_MASK, dsap) && PTX_SAP_CHECK(PTX_LLCP_PDU_SSAP_MASK, ssap) && (NULL != tlvs) && (0 != tlvLength) && (PTX_LLCP_PDU_CONNECT_MAX_TLV_LEN >= tlvLength))
    {
        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /* Set up the data structure */
        pdu.Sap.Destination    = dsap;
        pdu.Sap.Source         = ssap;
        pdu.Ptype              = LLCP_PduType_CONNECT;

        /* Assemble the TLV datastructure(s) */
        ptx_RUL_LLCP_TLV_Handler_t tlv_handler;
        (void) ptx_RUL_LLCP_TLV_Open(&tlv_handler);
        status = ptx_RUL_LLCP_PDU_Assemble(&pdu, &tlv_handler, tlvs, tlvLength);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Write the PDU to the scratchpad */
            status = ptx_RUL_LLCP_Frame_WritePdu(ctx, &pdu);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_Disconnect (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_SAP_CHECK(PTX_LLCP_PDU_DSAP_MASK, dsap) && PTX_SAP_CHECK(PTX_LLCP_PDU_SSAP_MASK, ssap))
    {
        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /* Set up the data structure */
        pdu.Sap.Destination    = dsap;
        pdu.Sap.Source         = ssap;
        pdu.Ptype              = LLCP_PduType_DISC;

        /* Write the PDU to the scratchpad */
        status = ptx_RUL_LLCP_Frame_WritePdu(ctx, &pdu);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_ConnectionComplete (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_SAP_CHECK(PTX_LLCP_PDU_DSAP_MASK, dsap) && PTX_SAP_CHECK(PTX_LLCP_PDU_SSAP_MASK, ssap) && (NULL != tlvs) && (0 != tlvLength) && (PTX_LLCP_PDU_CC_MAX_TLV_LEN >= tlvLength))
    {
        /** Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /** Set up the data structure */
        pdu.Sap.Destination    = dsap;
        pdu.Sap.Source         = ssap;
        pdu.Ptype              = LLCP_PduType_CC;

        /** Assemble the TLV datastructure(s) */
        ptx_RUL_LLCP_TLV_Handler_t tlv_handler;
        (void) ptx_RUL_LLCP_TLV_Open (&tlv_handler);

        status = ptx_RUL_LLCP_PDU_Assemble(&pdu, &tlv_handler, tlvs, tlvLength);

        if (PTX_STATUS_SUCCESS == status)
        {
            /** Write the PDU to the scratchpad */
            status = ptx_RUL_LLCP_Frame_WritePdu (ctx, &pdu);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_DisconnectMode (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, ptx_RUL_LLCP_DisconnectReason_t reason)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_SAP_CHECK(PTX_LLCP_PDU_DSAP_MASK, dsap) && PTX_SAP_CHECK(PTX_LLCP_PDU_SSAP_MASK, ssap))
    {
        ptx_RUL_LLCP_DisconnectReason_t temp_reason = reason;

        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /* Set up the data structure */
        pdu.Sap.Destination    = dsap;
        pdu.Sap.Source         = ssap;
        pdu.Ptype              = LLCP_PduType_DM;
        pdu.Information        = (uint8_t*) &temp_reason;
        pdu.InformationLen     = sizeof(ptx_RUL_LLCP_DisconnectReason_t);

        /* Write the PDU to the scratchpad */
        status = ptx_RUL_LLCP_Frame_WritePdu(ctx, &pdu);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_FrameReject (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, ptx_RUL_LLCP_RejectReport_t *rejectReport)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_SAP_CHECK(PTX_LLCP_PDU_DSAP_MASK, dsap) && PTX_SAP_CHECK(PTX_LLCP_PDU_SSAP_MASK, ssap) && (NULL != rejectReport))
    {
        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));


        /* Assemble the bitfields required for the report. */
        uint8_t report_payload[4];
        memset(report_payload, 0, sizeof(report_payload));
        report_payload[0] = (uint8_t) (report_payload[0] | (rejectReport->W ? 0x80 : 0x00));
        report_payload[0] = (uint8_t) (report_payload[0] | (rejectReport->I ? 0x40 : 0x00));
        report_payload[0] = (uint8_t) (report_payload[0] | (rejectReport->R ? 0x20 : 0x00));
        report_payload[0] = (uint8_t) (report_payload[0] | (rejectReport->S ? 0x10 : 0x00));
        report_payload[0] = (uint8_t) (report_payload[0] | rejectReport->Ptype);
        report_payload[1] = (uint8_t) ((rejectReport->SendSequenceNumber << 4) | (rejectReport->ReceiveSequenceNumber << 0));
        report_payload[2] = (uint8_t) ((rejectReport->Vs << 4) | (rejectReport->Vr << 0));
        report_payload[3] = (uint8_t) ((rejectReport->Vsa << 4) | (rejectReport->Vra << 0));

        /* Set up the data structure */
        pdu.Sap.Destination = dsap;
        pdu.Sap.Source      = ssap;
        pdu.Ptype           = LLCP_PduType_FRMR;
        pdu.Information     = report_payload;
        pdu.InformationLen  = sizeof(report_payload);

        /* Write the PDU to the scratchpad */
        status = ptx_RUL_LLCP_Frame_WritePdu(ctx, &pdu);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_Information (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, uint8_t sseq, uint8_t rseq, uint8_t *information, uint8_t informationLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_SAP_CHECK(PTX_LLCP_PDU_DSAP_MASK, dsap) && PTX_SAP_CHECK(PTX_LLCP_PDU_SSAP_MASK, ssap) && (NULL != information))
    {
        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /* Set up the data structure */
        pdu.Sap.Destination         = dsap;
        pdu.Sap.Source              = ssap;
        pdu.SendSequenceNumber      = sseq;
        pdu.ReceiveSequenceNumber   = rseq;
        pdu.Ptype                   = LLCP_PduType_I;
        pdu.Information             = information;
        pdu.InformationLen          = informationLen;

        /* Write the PDU to the scratchpad */
        status = ptx_RUL_LLCP_Frame_WritePdu(ctx, &pdu);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_ReceiveReady (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, uint8_t receiveSequenceNumber)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_SAP_CHECK(PTX_LLCP_PDU_DSAP_MASK, dsap) && PTX_SAP_CHECK(PTX_LLCP_PDU_SSAP_MASK, ssap))
    {
        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /* Set up the data structure */
        pdu.Sap.Destination         = dsap;
        pdu.Sap.Source              = ssap;
        pdu.Ptype                   = LLCP_PduType_RR;
        pdu.ReceiveSequenceNumber   = receiveSequenceNumber;

        /* Write the PDU to the scratchpad */
        status = ptx_RUL_LLCP_Frame_WritePdu(ctx, &pdu);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_ReceiveNotReady (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, uint8_t receiveSequenceNumber)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_SAP_CHECK(PTX_LLCP_PDU_DSAP_MASK, dsap) && PTX_SAP_CHECK(PTX_LLCP_PDU_SSAP_MASK, ssap))
    {
        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /* Set up the data structure */
        pdu.Sap.Destination         = dsap;
        pdu.Sap.Source              = ssap;
        pdu.Ptype                   = LLCP_PduType_RNR;
        pdu.ReceiveSequenceNumber   = receiveSequenceNumber;

        /* Write the PDU to the scratchpad */
        status = ptx_RUL_LLCP_Frame_WritePdu(ctx, &pdu);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_ServiceNameLookup (ptx_RUL_LLCP_Frame_t *ctx, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != tlvs) && (0 != tlvLength) && (PTX_LLCP_PDU_SNL_MAX_TLV_LEN >= tlvLength))
    {
        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /** Set up the data structure */
        pdu.Sap.Destination = 1u;
        pdu.Sap.Source = 1u;
        pdu.Ptype = LLCP_PduType_SNL;

        /* Assemble the TLV datastructure(s) */
        ptx_RUL_LLCP_TLV_Handler_t tlv_handler;
        (void) ptx_RUL_LLCP_TLV_Open (&tlv_handler);
        status = ptx_RUL_LLCP_PDU_Assemble(&pdu, &tlv_handler, tlvs, tlvLength);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Write the PDU to the scratchpad */
            status = ptx_RUL_LLCP_Frame_WritePdu (ctx, &pdu);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_PDU_DataProtectionSetup (ptx_RUL_LLCP_Frame_t *ctx, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != tlvs) && (0 != tlvLength) && (PTX_LLCP_PDU_DPS_MAX_TLV_LEN >= tlvLength))
    {
        /* Prepare the PDU datastructure */
        ptx_RUL_LLCP_PDU_t pdu;
        memset(&pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        /* Set up the data structure */
        pdu.Ptype = LLCP_PduType_DPS;

        /* Assemble the TLV datastructure(s) */
        ptx_RUL_LLCP_TLV_Handler_t tlv_handler;
        (void) ptx_RUL_LLCP_TLV_Open (&tlv_handler);
        status = ptx_RUL_LLCP_PDU_Assemble(&pdu, &tlv_handler, tlvs, tlvLength);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Write the PDU to the scratchpad */
            status = ptx_RUL_LLCP_Frame_WritePdu (ctx, &pdu);
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
ptx_Status_t ptx_RUL_LLCP_PDU_Assemble(ptx_RUL_LLCP_PDU_t *ctx, ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != tlv) && (NULL != tlvs) && (0 != tlvLength))
    {
        /* Assemble the TLVs structures */
        status = ptx_RUL_LLCP_TLV_Write(tlv, tlvs, tlvLength);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* If everything went well, append the TLVs as payload */
            ctx->Information    = tlv->ScratchPad;
            ctx->InformationLen = (uint8_t) tlv->ScratchPadIndex;

            /* Write the PDU to the scratchpad */
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}
