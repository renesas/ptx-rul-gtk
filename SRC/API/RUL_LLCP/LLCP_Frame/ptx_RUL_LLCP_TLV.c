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
    Module      : LLCP
    File        : ptx_RUL_LLCP_TLV.c

    Description : Implementation of the Logical Link Control Protocol TLVs.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL_LLCP_TLV.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_LLCP_TLV_BUFFER_LEN                 (255u)      /**< Length of the TLV buffer */
#define PTX_LLCP_TLV_HEADER_LEN                 (0x02)      /**< Length of the TLV header */
#define PTX_LLCP_TLV_VERSION_PAYLOAD_LEN        (0x01)      /**< Payload length of VERSION type */
#define PTX_LLCP_TLV_MIUX_PAYLOAD_LEN           (0x02)      /**< Payload length of MIUX type */
#define PTX_LLCP_TLV_WKS_PAYLOAD_LEN            (0x02)      /**< Payload length of WKS type */
#define PTX_LLCP_TLV_LTO_PAYLOAD_LEN            (0x01)      /**< Payload length of LTO type */
#define PTX_LLCP_TLV_RW_PAYLOAD_LEN             (0x01)      /**< Payload length of RW type */
#define PTX_LLCP_TLV_OPT_PAYLOAD_LEN            (0x01)      /**< Payload length of OPT type */
#define PTX_LLCP_TLV_SDREQ_STATIC_PAYLOAD_LEN   (0x01)      /**< Static part of the payload length of SDREQ type */
#define PTX_LLCP_TLV_SDRES_PAYLOAD_LEN          (0x02)      /**< Payload length of SDREQ type */

#define PTX_LLCP_TLV_MIUX_MASK                  (0x07FF)    /**< Bit mask for the value of MIUX */
#define PTX_LLCP_TLV_MIUX_OFFSET                (0)         /**< Offset for the value of MIUX */
#define PTX_LLCP_TLV_WKS_DEFAULT_MASK           (0x0001)    /**< Bit mask for the value of WKS */
#define PTX_LLCP_TLV_DPC_OFFSET                 (2)         /**< Offset for the value of DPC */
#define PTX_LLCP_TLV_DPC_MASK                   (0x01)      /**< Bit mask for the value of DPC */
#define PTX_LLCP_TLV_CSN_MASK                   (0x01)      /**< Bit mask for the value of CSN */
#define PTX_LLCP_TLV_CSN_OFFSET                 (6)         /**< Offset for the value of CSN */
#define PTX_LLCP_TLV_SAP_MASK                   (0x3F)      /**< Bit mask for the value of SAP */
#define PTX_LLCP_TLV_SAP_OFFSET                 (0)         /**< Offset for the value of SAP */

#define PTX_LLCP_GENERIC_NIBBLE_MASK            (0x0F)      /**< Generic mask for a nibble (4 Bit) */
#define PTX_LLCP_GENERIC_HNIBBLE_OFFSET         (0x04)      /**< Generic offset for the high nibble */
#define PTX_LLCP_GENERIC_LNIBBLE_OFFSET         (0x00)      /**< Generic offset for the low nibble */
#define PTX_LLCP_GENERIC_UINT8_MASK             (0xFF)      /**< Payload length of SDREQ type */
/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static void ptx_RUL_LLCP_TLV_WriteUint8 (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint8_t data);
static void ptx_RUL_LLCP_TLV_WriteUint16 (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint16_t data);
static void ptx_RUL_LLCP_TLV_WriteArray (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint8_t *data, uint32_t dataLen);
static ptx_Status_t ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint32_t dataToWrite);
static ptx_Status_t ptx_RUL_LLCP_TLV_ParseBuffer (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint8_t *data, uint32_t dataLen, ptx_RUL_LLCP_TLV_t *tlvList, uint32_t *tlvListLen);

static uint8_t sTlvScratchPad[PTX_LLCP_TLV_BUFFER_LEN];
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_RUL_LLCP_TLV_Open (ptx_RUL_LLCP_TLV_Handler_t *tlv)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != tlv)
    {
        tlv->CompId             = Comp_LLCP_TLV;
        tlv->ScratchPad         = sTlvScratchPad;
        tlv->ScratchPadSize     = PTX_LLCP_TLV_BUFFER_LEN;
        tlv->ScratchPadIndex    = 0;

        memset(tlv->ScratchPad, 0, PTX_LLCP_TLV_BUFFER_LEN);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_Close (ptx_RUL_LLCP_TLV_Handler_t *tlv)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV))
    {
        memset(tlv, 0, sizeof(ptx_RUL_LLCP_TLV_Handler_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_ClearBuffer (ptx_RUL_LLCP_TLV_Handler_t *tlv)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV))
    {
        tlv->ScratchPadIndex = 0;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_WriteVersion (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Version_t *version)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV))
    {
        status = ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace(tlv, PTX_LLCP_TLV_HEADER_LEN + PTX_LLCP_TLV_VERSION_PAYLOAD_LEN);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, LLCP_TagType_Version);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, PTX_LLCP_TLV_VERSION_PAYLOAD_LEN);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, (uint8_t) (((version->Major & PTX_LLCP_GENERIC_NIBBLE_MASK) << PTX_LLCP_GENERIC_HNIBBLE_OFFSET) |
                                                        ((version->Minor & PTX_LLCP_GENERIC_NIBBLE_MASK) << PTX_LLCP_GENERIC_LNIBBLE_OFFSET)));
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_WriteMiux (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Miux_t *miux)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV))
    {
        status = ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace(tlv, PTX_LLCP_TLV_HEADER_LEN + PTX_LLCP_TLV_MIUX_PAYLOAD_LEN);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, LLCP_TagType_MIUX);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, PTX_LLCP_TLV_MIUX_PAYLOAD_LEN);
            ptx_RUL_LLCP_TLV_WriteUint16(tlv, miux->Miux & PTX_LLCP_TLV_MIUX_MASK);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_WriteWks (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Wks_t *wks)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV))
    {
        status = ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace(tlv, PTX_LLCP_TLV_HEADER_LEN + PTX_LLCP_TLV_WKS_PAYLOAD_LEN);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, LLCP_TagType_WKS);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, PTX_LLCP_TLV_WKS_PAYLOAD_LEN);
            ptx_RUL_LLCP_TLV_WriteUint16(tlv, wks->ServiceList | PTX_LLCP_TLV_WKS_DEFAULT_MASK);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_WriteLto (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Lto_t *lto)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV))
    {
        status = ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace(tlv, PTX_LLCP_TLV_HEADER_LEN + PTX_LLCP_TLV_LTO_PAYLOAD_LEN);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, LLCP_TagType_LTO);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, PTX_LLCP_TLV_LTO_PAYLOAD_LEN);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, lto->Timeout);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_WriteReceiveWindowSize (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Rw_t *rw)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV))
    {
        status = ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace(tlv, PTX_LLCP_TLV_HEADER_LEN + PTX_LLCP_TLV_RW_PAYLOAD_LEN);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, LLCP_TagType_RW);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, PTX_LLCP_TLV_RW_PAYLOAD_LEN);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, rw->WindowSize & PTX_LLCP_GENERIC_NIBBLE_MASK);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_WriteServiceName (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_ServiceName_t *serviceName)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV) && (NULL != serviceName->Name) && (0 < serviceName->Length) && (255 > serviceName->Length))
    {
        status = ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace(tlv, PTX_LLCP_TLV_HEADER_LEN + serviceName->Length);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, LLCP_TagType_SN);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, (uint8_t) serviceName->Length);
            ptx_RUL_LLCP_TLV_WriteArray(tlv, (uint8_t*) serviceName->Name, serviceName->Length);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_WriteOption (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Option_t *option)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV))
    {
        status = ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace(tlv, PTX_LLCP_TLV_HEADER_LEN + PTX_LLCP_TLV_OPT_PAYLOAD_LEN);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t dpc_bit = (option->DataProtectionClass == 0 ? 0 : (PTX_LLCP_TLV_DPC_MASK << PTX_LLCP_TLV_DPC_OFFSET));

            ptx_RUL_LLCP_TLV_WriteUint8(tlv, LLCP_TagType_OPT);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, PTX_LLCP_TLV_OPT_PAYLOAD_LEN);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, (uint8_t) (dpc_bit | (option->ServiceClass & LLCP_LSC_Class3)));
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_WriteServiceDiscoveryRequest (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Sdreq_t *sdReq)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV) && (NULL != sdReq->ServiceName.Name) && (0 < sdReq->ServiceName.Length) && (255 > sdReq->ServiceName.Length))
    {
        status = ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace(tlv, PTX_LLCP_TLV_HEADER_LEN + PTX_LLCP_TLV_SDREQ_STATIC_PAYLOAD_LEN + sdReq->ServiceName.Length);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, LLCP_TagType_SDREQ);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, (uint8_t) (PTX_LLCP_TLV_SDREQ_STATIC_PAYLOAD_LEN + sdReq->ServiceName.Length));
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, sdReq->TransactionIdentifier);
            ptx_RUL_LLCP_TLV_WriteArray(tlv, (uint8_t*) sdReq->ServiceName.Name, sdReq->ServiceName.Length);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_WriteServiceDiscoveryResponse (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Sdres_t *sdRes)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV))
    {
        status = ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace(tlv, PTX_LLCP_TLV_HEADER_LEN + PTX_LLCP_TLV_SDRES_PAYLOAD_LEN);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t csn_bit = (sdRes->ConnectServiceName == 0 ? 0 : (PTX_LLCP_TLV_CSN_MASK << PTX_LLCP_TLV_CSN_OFFSET));

            ptx_RUL_LLCP_TLV_WriteUint8(tlv, LLCP_TagType_SDRES);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, PTX_LLCP_TLV_SDRES_PAYLOAD_LEN);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, sdRes->TransactionIdentifier);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, (uint8_t) (csn_bit | ((sdRes->ServiceAccessPoint << PTX_LLCP_TLV_SAP_OFFSET) & PTX_LLCP_TLV_SAP_MASK)));
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_WriteECPublicKey (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Ecpk_t *ecpk)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV) && (NULL != ecpk) && (NULL != ecpk->CoordinateX) && (0 < ecpk->Length))
    {
        status = ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace(tlv, PTX_LLCP_TLV_HEADER_LEN + (2u * ecpk->Length));

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, LLCP_TagType_ECPK);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, (uint8_t) (2u * ecpk->Length));
            ptx_RUL_LLCP_TLV_WriteArray(tlv, ecpk->CoordinateX, ecpk->Length);
            ptx_RUL_LLCP_TLV_WriteArray(tlv, ecpk->CoordinateY, ecpk->Length);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_WriteNonce (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Nonce_t *nonce)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV) && (NULL != nonce) && (NULL != nonce->Nonce) && (0 < nonce->Length))
    {
        status = ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace(tlv, PTX_LLCP_TLV_HEADER_LEN + nonce->Length);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, LLCP_TagType_RN);
            ptx_RUL_LLCP_TLV_WriteUint8(tlv, (uint8_t) nonce->Length);
            ptx_RUL_LLCP_TLV_WriteArray(tlv, nonce->Nonce, nonce->Length);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_Write (ptx_RUL_LLCP_TLV_Handler_t *tlv, ptx_RUL_LLCP_TLV_Data_t *tlvList, uint32_t tlvListLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV) && (NULL != tlvList) && (0 < tlvListLen))
    {
        status = PTX_STATUS_SUCCESS;
        for (uint32_t i = 0; (i < tlvListLen) && (PTX_STATUS_SUCCESS == status); ++i)
        {
            switch (tlvList[i].Type)
            {
                case LLCP_TagType_Version:
                    status = ptx_RUL_LLCP_TLV_WriteVersion(tlv, &tlvList[i].Data.Version);
                    break;

                case LLCP_TagType_MIUX:
                    status = ptx_RUL_LLCP_TLV_WriteMiux(tlv, &tlvList[i].Data.Miux);
                    break;

                case LLCP_TagType_WKS:
                    status = ptx_RUL_LLCP_TLV_WriteWks(tlv, &tlvList[i].Data.Wks);
                    break;

                case LLCP_TagType_LTO:
                    status = ptx_RUL_LLCP_TLV_WriteLto(tlv, &tlvList[i].Data.Lto);
                    break;

                case LLCP_TagType_RW:
                    status = ptx_RUL_LLCP_TLV_WriteReceiveWindowSize(tlv, &tlvList[i].Data.Rw);
                    break;

                case LLCP_TagType_SN:
                    status = ptx_RUL_LLCP_TLV_WriteServiceName(tlv, &tlvList[i].Data.Sn);
                    break;

                case LLCP_TagType_OPT:
                    status = ptx_RUL_LLCP_TLV_WriteOption(tlv, &tlvList[i].Data.Option);
                    break;

                case LLCP_TagType_SDREQ:
                    status = ptx_RUL_LLCP_TLV_WriteServiceDiscoveryRequest(tlv, &tlvList[i].Data.SdReq);
                    break;

                case LLCP_TagType_SDRES:
                    status = ptx_RUL_LLCP_TLV_WriteServiceDiscoveryResponse(tlv, &tlvList[i].Data.SdRes);
                    break;

                case LLCP_TagType_ECPK:
                    status = ptx_RUL_LLCP_TLV_WriteECPublicKey(tlv, &tlvList[i].Data.Ecpk);
                    break;

                case LLCP_TagType_RN:
                    status = ptx_RUL_LLCP_TLV_WriteNonce(tlv, &tlvList[i].Data.Nonce);
                    break;

                default:
                    break;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_ProcessReceivedData (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint8_t *data, uint32_t dataLen, ptx_RUL_LLCP_TLV_Data_t *tlvList, uint32_t *tlvListLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV) && (NULL != tlvList) && (NULL != tlvListLen) && (0 != *tlvListLen))
    {
        uint32_t tlvsLen = LLCP_TLV_MAX_PARAMS;

        ptx_RUL_LLCP_TLV_t tlvs[LLCP_TLV_MAX_PARAMS];
        memset(tlvs, 0, sizeof(ptx_RUL_LLCP_TLV_t) * LLCP_TLV_MAX_PARAMS);

        status = ptx_RUL_LLCP_TLV_ParseBuffer(tlv, data, dataLen, tlvs, &tlvsLen);

        if ((PTX_STATUS_SUCCESS == status) && (*tlvListLen < tlvsLen))
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            *tlvListLen = tlvsLen;
            for (uint32_t i = 0; i < tlvsLen; ++i)
            {
                tlvList[i].Type = tlvs[i].Type;

                switch (tlvList[i].Type)
                {
                    case LLCP_TagType_Version:
                        tlvList[i].Data.Version.Major = (tlvs[i].Data[0] >> PTX_LLCP_GENERIC_HNIBBLE_OFFSET) & PTX_LLCP_GENERIC_NIBBLE_MASK;
                        tlvList[i].Data.Version.Minor = (tlvs[i].Data[0] >> PTX_LLCP_GENERIC_LNIBBLE_OFFSET) & PTX_LLCP_GENERIC_NIBBLE_MASK;
                        break;
                    case LLCP_TagType_MIUX:
                        tlvList[i].Data.Miux.Miux = ((tlvs[i].Data[0] << 8) | tlvs[i].Data[1]) & PTX_LLCP_TLV_MIUX_MASK;
                        break;
                    case LLCP_TagType_WKS:
                        tlvList[i].Data.Wks.ServiceList = (uint16_t) (((tlvs[i].Data[0]) << 8) | tlvs[i].Data[1] );
                        break;
                    case LLCP_TagType_LTO:
                        tlvList[i].Data.Lto.Timeout = tlvs[i].Data[0];
                        break;
                    case LLCP_TagType_RW:
                        tlvList[i].Data.Rw.WindowSize = tlvs[i].Data[0];
                        break;
                    case LLCP_TagType_SN:
                        tlvList[i].Data.Sn.Name = (char*) &tlvs[i].Data[0];
                        tlvList[i].Data.Sn.Length = tlvs[i].Length;
                        break;
                    case LLCP_TagType_OPT:
                        tlvList[i].Data.Option.ServiceClass = (tlvs[i].Data[0] & LLCP_LSC_Class3);
                        tlvList[i].Data.Option.DataProtectionClass = ((tlvs[i].Data[0] & (PTX_LLCP_TLV_DPC_MASK << PTX_LLCP_TLV_DPC_OFFSET)) == 0 ? 0 : 1);
                        break;
                    case LLCP_TagType_SDREQ:
                        tlvList[i].Data.SdReq.TransactionIdentifier = tlvs[i].Data[0];
                        tlvList[i].Data.SdReq.ServiceName.Name = (char*) &tlvs[i].Data[1];
                        tlvList[i].Data.SdReq.ServiceName.Length = tlvs[i].Length - PTX_LLCP_TLV_SDREQ_STATIC_PAYLOAD_LEN;
                        break;
                    case LLCP_TagType_SDRES:
                        tlvList[i].Data.SdRes.TransactionIdentifier = tlvs[i].Data[0];
                        tlvList[i].Data.SdRes.ConnectServiceName = ((tlvs[i].Data[1] & (PTX_LLCP_TLV_CSN_MASK << PTX_LLCP_TLV_CSN_OFFSET)) == 0 ? 0 : 1);
                        tlvList[i].Data.SdRes.ServiceAccessPoint = tlvs[i].Data[1] & (PTX_LLCP_TLV_SAP_MASK << PTX_LLCP_TLV_SAP_OFFSET);
                        break;
                    case LLCP_TagType_ECPK:
                        tlvList[i].Data.Ecpk.Length = tlvs[i].Length / 2;
                        tlvList[i].Data.Ecpk.CoordinateX = &tlvs[i].Data[0];
                        tlvList[i].Data.Ecpk.CoordinateY = &tlvs[i].Data[tlvList[i].Data.Ecpk.Length];
                        break;
                    case LLCP_TagType_RN:
                        tlvList[i].Data.Nonce.Length = tlvs[i].Length;
                        tlvList[i].Data.Nonce.Nonce = &tlvs[i].Data[0];
                        break;
                    default:
                        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                        break;
                }
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
void ptx_RUL_LLCP_TLV_WriteUint8 (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint8_t data)
{
    tlv->ScratchPad[tlv->ScratchPadIndex] = data;
    ++tlv->ScratchPadIndex;
}

void ptx_RUL_LLCP_TLV_WriteUint16 (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint16_t data)
{
    tlv->ScratchPad[tlv->ScratchPadIndex] = (uint8_t) ((data >> 8) & PTX_LLCP_GENERIC_UINT8_MASK);
    ++tlv->ScratchPadIndex;

    tlv->ScratchPad[tlv->ScratchPadIndex] = (uint8_t) ((data >> 0) & PTX_LLCP_GENERIC_UINT8_MASK);
    ++tlv->ScratchPadIndex;
}

void ptx_RUL_LLCP_TLV_WriteArray (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint8_t *data, uint32_t dataLen)
{
    memcpy(&tlv->ScratchPad[tlv->ScratchPadIndex], data, dataLen);
    tlv->ScratchPadIndex += dataLen;
}

ptx_Status_t ptx_RUL_LLCP_TLV_CheckAvailableBufferSpace (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint32_t dataToWrite)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;

    if ((tlv->ScratchPadSize - tlv->ScratchPadIndex) >= dataToWrite)
    {
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TLV_ParseBuffer (ptx_RUL_LLCP_TLV_Handler_t *tlv, uint8_t *data, uint32_t dataLen, ptx_RUL_LLCP_TLV_t *tlvList, uint32_t *tlvListLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tlv, Comp_LLCP_TLV) && (NULL != data) && (0 != dataLen) && (NULL != tlvList) && (NULL != tlvListLen) && (0 < *tlvListLen))
    {
        uint32_t tlvIndex = 0;
        uint32_t bufferIndex = 0;

        while ((bufferIndex < dataLen) && (tlvIndex < *tlvListLen))
        {
            tlvList[tlvIndex].Type = data[bufferIndex];
            ++bufferIndex;

            tlvList[tlvIndex].Length = data[bufferIndex];
            ++bufferIndex;

            tlvList[tlvIndex].Data = &data[bufferIndex];
            bufferIndex += tlvList[tlvIndex].Length;

            ++tlvIndex;
        }

        if (tlvIndex <= *tlvListLen)
        {
            *tlvListLen = tlvIndex;
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}
