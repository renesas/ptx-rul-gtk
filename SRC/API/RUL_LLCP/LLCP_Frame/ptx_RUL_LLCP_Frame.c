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
    File        : ptx_RUL_LLCP_Frame.c

    Description : Implementation to assemble and parse LLCP frames.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL_LLCP_Frame.h"
#include <string.h>

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
static ptx_Status_t ptx_RUL_LLCP_Frame_ParsePdu (ptx_RUL_LLCP_Frame_t *ctx, uint32_t startIdx, ptx_RUL_LLCP_PDU_t *pdu, uint32_t *processedHeaderBytes);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_RUL_LLCP_Frame_Open (ptx_RUL_LLCP_Frame_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {
        ctx->CompId            = Comp_LLCP_Frame;
        ctx->ScratchPadSize    = PTX_LLCP_FRAME_BUFFER_LEN;
        ctx->ScratchPadIndex   = 0;
        memset(ctx->ScratchPad, 0, PTX_LLCP_FRAME_BUFFER_LEN);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Frame_Close (ptx_RUL_LLCP_Frame_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Frame))
    {
        memset(ctx, 0, sizeof(ptx_RUL_LLCP_Frame_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Frame_WritePdu (ptx_RUL_LLCP_Frame_t *ctx, ptx_RUL_LLCP_PDU_t *pdu)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Frame) && (NULL != pdu))
    {
        uint32_t index = 0;
        ctx->ScratchPad[index] = (uint8_t) ((pdu->Sap.Destination & PTX_LLCP_PDU_DSAP_MASK) << PTX_LLCP_PDU_DSAP_OFFSET);
        ctx->ScratchPad[index] |= (uint8_t) ((pdu->Ptype & PTX_LLCP_PDU_PTYPE_MASK) >> PTX_LLCP_PDU_PTYPE_OFFSET_LB);
        index++;

        ctx->ScratchPad[index] = (uint8_t) ((pdu->Sap.Source & PTX_LLCP_PDU_SSAP_MASK) << PTX_LLCP_PDU_SSAP_OFFSET);
        ctx->ScratchPad[index] |= (uint8_t) ((pdu->Ptype & PTX_LLCP_PDU_PTYPE_MASK) << PTX_LLCP_PDU_PTYPE_OFFSET_HB);
        index++;

        if ( (LLCP_PduType_I == pdu->Ptype) || (LLCP_PduType_RR == pdu->Ptype) || (LLCP_PduType_RNR == pdu->Ptype) )
        {
            ctx->ScratchPad[index] = (uint8_t) ((pdu->ReceiveSequenceNumber & PTX_LLCP_PDU_SEQNR_MASK) << PTX_LLCP_PDU_SEQNR_OFFSET);
            ctx->ScratchPad[index] |= (uint8_t) ((pdu->SendSequenceNumber & PTX_LLCP_PDU_SEQNS_MASK) << PTX_LLCP_PDU_SEQNS_OFFSET);
            index++;
        }

        if ((0 != pdu->InformationLen) && (NULL != pdu->Information))
        {
            memcpy(&ctx->ScratchPad[index], pdu->Information,  pdu->InformationLen);
            index += pdu->InformationLen;
        }

        if (0 != pdu->IcvLen)
        {
            memcpy(&ctx->ScratchPad[index], pdu->Icv,  pdu->IcvLen);
            index += pdu->IcvLen;
        }

        ctx->ScratchPadIndex = index;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Frame_Parse (ptx_RUL_LLCP_Frame_t *ctx, ptx_RUL_LLCP_PDU_t *pdus, uint32_t *pdusLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Frame) && (NULL != pdus) && (NULL != pdusLength) && (0 != *pdusLength))
    {
        /* Clear all PDU structs */
        memset(pdus, 0, sizeof(ptx_RUL_LLCP_PDU_t) * *pdusLength);

        uint32_t pdu_index = 0;
        uint32_t parse_index = 0;
        uint32_t processed_header_bytes = 0;

        /* Parse the PDU. */
        status = ptx_RUL_LLCP_Frame_ParsePdu(ctx, parse_index, &pdus[pdu_index], &processed_header_bytes);

        if (PTX_STATUS_SUCCESS == status)
        {
            pdus[pdu_index].InformationLen = (uint8_t) (ctx->ScratchPadIndex - processed_header_bytes);
            pdu_index++;

            parse_index += processed_header_bytes;

            /* In case of an Aggregated PDU, we need to iterate over the nested PDUs. */
            if (LLCP_PduType_AGF == pdus[pdu_index].Ptype)
            {
                /* Iterate over the content as long as there are unparsed bytes left and as long as there are PDU structs to store the parsed data. */
                while (ctx->ScratchPadIndex > parse_index && pdu_index < *pdusLength)
                {
                    /* Nested PDUs contains a 16-Bit length information. */
                    uint16_t nested_pdu_len = (uint16_t) ((ctx->ScratchPad[parse_index] << 8) | ctx->ScratchPad[parse_index + 1]);
                    parse_index += 2;

                    /* Parse PDU by PDU. */
                    status = ptx_RUL_LLCP_Frame_ParsePdu (ctx, parse_index, &pdus[pdu_index], &processed_header_bytes);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* For nested PDUs, the Information length must be recalculated. */
                        pdus[pdu_index].InformationLen = (uint8_t) (nested_pdu_len - processed_header_bytes);
                        pdu_index += 1;
                        parse_index += nested_pdu_len;
                    }
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Store the number of parsed/processed PDUs. */
                    *pdusLength = pdu_index;
                }
                else
                {
                    /* Something went wrong. */
                    *pdusLength = 0;
                }
            }
            else
            {
                *pdusLength = 1;
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
ptx_Status_t ptx_RUL_LLCP_Frame_ParsePdu (ptx_RUL_LLCP_Frame_t *ctx, uint32_t startIdx, ptx_RUL_LLCP_PDU_t *pdu, uint32_t *processedHeaderBytes)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Frame) && (NULL != pdu) && (NULL != processedHeaderBytes))
    {
        memset(pdu, 0, sizeof(ptx_RUL_LLCP_PDU_t));

        *processedHeaderBytes = 0;

        pdu->Sap.Destination = (ctx->ScratchPad[startIdx + *processedHeaderBytes] >> PTX_LLCP_PDU_DSAP_OFFSET) & PTX_LLCP_PDU_DSAP_MASK;
        pdu->Ptype = (ctx->ScratchPad[startIdx + *processedHeaderBytes] << PTX_LLCP_PDU_PTYPE_OFFSET_LB) & 0x0C;
        (*processedHeaderBytes)++;

        pdu->Sap.Source = (ctx->ScratchPad[startIdx + *processedHeaderBytes] >> PTX_LLCP_PDU_SSAP_OFFSET) & PTX_LLCP_PDU_SSAP_MASK;
        pdu->Ptype |= (ctx->ScratchPad[startIdx + *processedHeaderBytes] >> PTX_LLCP_PDU_PTYPE_OFFSET_HB) & 0x03;
        (*processedHeaderBytes)++;

        if ((LLCP_PduType_I == pdu->Ptype) || (LLCP_PduType_RR == pdu->Ptype) || (LLCP_PduType_RNR == pdu->Ptype))
        {
            pdu->ReceiveSequenceNumber = (ctx->ScratchPad[startIdx + *processedHeaderBytes] >> PTX_LLCP_PDU_SEQNR_OFFSET) & PTX_LLCP_PDU_SEQNR_MASK;
            pdu->SendSequenceNumber = (ctx->ScratchPad[startIdx + *processedHeaderBytes] >> PTX_LLCP_PDU_SEQNS_OFFSET) & PTX_LLCP_PDU_SEQNS_MASK;
            (*processedHeaderBytes)++;
        }

        if ( (LLCP_PduType_PAX == pdu->Ptype) || (LLCP_PduType_AGF == pdu->Ptype) || (LLCP_PduType_UI == pdu->Ptype)
             || (LLCP_PduType_CONNECT == pdu->Ptype) || (LLCP_PduType_CC == pdu->Ptype) || (LLCP_PduType_DM == pdu->Ptype)
             || (LLCP_PduType_FRMR == pdu->Ptype) || (LLCP_PduType_I == pdu->Ptype) || (LLCP_PduType_SNL == pdu->Ptype)
             || (LLCP_PduType_DPS == pdu->Ptype))
        {
            pdu->Information = &ctx->ScratchPad[startIdx + *processedHeaderBytes];
            pdu->InformationLen = (uint8_t) (ctx->ScratchPadIndex - startIdx - *processedHeaderBytes);
        }

        status = PTX_STATUS_SUCCESS;
    }
    return status;
}
