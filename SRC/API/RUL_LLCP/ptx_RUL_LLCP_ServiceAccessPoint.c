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
    File        : ptx_RUL_LLCP_ServiceAccessPoint.c

    Description : Service Access Point (SAP) implementation.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL_LLCP_ServiceAccessPoint.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_LLCP_SERVICEACCESSPOINT_DEFAULT_MIU         (128u)
#define PTX_LLCP_SERVICEACCESSPOINT_WELLKNOWN_BOUNDARY  (16u)
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
ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_Init(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ctx != NULL)
    {
        status = ptx_RUL_LLCP_MessageBuffer_Init(&ctx->MessageBuffer);

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->CompId     = Comp_LLCP_Sap;
            ctx->Window     = 0u;
            ctx->Busy       = 0u;
            ctx->Address    = 0xFF;
            ctx->Miu        = 0;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_IsWellKnown(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx, uint8_t *isWellKnown)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Sap) && (NULL != isWellKnown))
    {
        if (ctx->Address < PTX_LLCP_SERVICEACCESSPOINT_WELLKNOWN_BOUNDARY)
        {
            *isWellKnown = 1u;
        }
        else
        {
            *isWellKnown = 0u;
        }
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_Register(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx,  ptx_RUL_LLCP_ServiceAccessPoint_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Sap) && (NULL != initParams))
    {
        ctx->Address = initParams->Address;
        ctx->Miu = initParams->Miu;
        ctx->Window = initParams->Window;
        ctx->Busy = 0u;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_WriteMessageBuffer(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx, uint8_t *data, uint32_t dataLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Sap))
    {
        status = ptx_RUL_LLCP_MessageBuffer_Write(&ctx->MessageBuffer, data, dataLength);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_ReadMessageBuffer(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx, uint8_t *data, uint32_t *dataLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Sap))
    {
        status = ptx_RUL_LLCP_MessageBuffer_Read(&ctx->MessageBuffer, data, dataLength, 1u);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_DeInit(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Sap))
    {
        status = ptx_RUL_LLCP_MessageBuffer_DeInit(&ctx->MessageBuffer);

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->Window = 0u;
            ctx->Busy = 0u;
            ctx->Address = 0xFF;
            ctx->Miu = 0;
            ctx->CompId = Comp_MAX;
        }
    }

    return status;
}