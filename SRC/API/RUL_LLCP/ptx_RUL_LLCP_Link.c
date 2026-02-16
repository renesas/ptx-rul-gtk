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
    File        : ptx_RUL_LLCP_Link.c

    Description : Implementation handling the LLCP link.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL_LLCP_Link.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_LLCP_DEFAULT_MIU (128u)
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
ptx_Status_t ptx_RUL_LLCP_Link_Init(ptx_RUL_LLCP_Link_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ctx != NULL)
    {
        ctx->CompId = Comp_LLCP_Link;

        status = ptx_RUL_LLCP_Link_Reset(ctx);

        if (PTX_STATUS_SUCCESS != status)
        {
            ctx->CompId = Comp_MAX;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Link_ValidateNrSequence(ptx_RUL_LLCP_Link_t *ctx, uint8_t receivedNr, uint8_t *valid)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Link) && (NULL != valid))
    {
        status = PTX_STATUS_SUCCESS;

        *valid = 0;

        if (1u == ctx->Remote.Window)
        {
            if (receivedNr == ctx->TrxStates.Vs)
            {
                *valid = 1;
            }
        }

        if (1u < ctx->Remote.Window)
        {
            if (receivedNr <= ctx->TrxStates.Vs)
            {
                *valid = 1;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Link_ValidateNsSequence(ptx_RUL_LLCP_Link_t *ctx, uint8_t receivedNs, uint8_t *valid)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Link) && (NULL != valid))
    {
        status = PTX_STATUS_SUCCESS;

        *valid = 0;
        if (receivedNs == ctx->TrxStates.Vr)
        {
            *valid = 1;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Link_WriteMessageInbox(ptx_RUL_LLCP_Link_t *ctx, uint8_t *data, uint32_t dataLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Link))
    {
        status = ptx_RUL_LLCP_ServiceAccessPoint_WriteMessageBuffer(&ctx->Local, data, dataLength);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Link_ReadMessageInbox(ptx_RUL_LLCP_Link_t *ctx, uint8_t *data, uint32_t *dataLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Link))
    {
        status = ptx_RUL_LLCP_ServiceAccessPoint_ReadMessageBuffer(&ctx->Local, data, dataLength);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Link_WriteMessageOutbox(ptx_RUL_LLCP_Link_t *ctx, uint8_t *data, uint32_t dataLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Link))
    {
        status = ptx_RUL_LLCP_ServiceAccessPoint_WriteMessageBuffer(&ctx->Remote, data, dataLength);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Link_Register(ptx_RUL_LLCP_Link_t *ctx, ptx_RUL_LLCP_Link_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Link))
    {
        if (0 == ctx->InUse)
        {
            /* Setup the local SAP first */
            ptx_RUL_LLCP_ServiceAccessPoint_InitParams_t localParams;
            localParams.Address = initParams->Ssap;
            localParams.Window  = initParams->Window;
            localParams.Miu     = PTX_LLCP_DEFAULT_MIU + initParams->Miux;
            status = ptx_RUL_LLCP_ServiceAccessPoint_Register(&ctx->Local, &localParams);

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->Type = initParams->Type;
                ctx->LinkState = LinkState_Registered;
                ctx->Remote.Window = 1;
                ctx->Remote.Miu = PTX_LLCP_DEFAULT_MIU;
                if ((NULL != initParams->ServiceUri) && (0 != initParams->ServiceUriLen))
                {
                    strncpy(ctx->ServiceUri, initParams->ServiceUri, initParams->ServiceUriLen);
                }

                ctx->InUse = 1u;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Link_Disconnect(ptx_RUL_LLCP_Link_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Link))
    {
        /* Set State accordingly. */
        ctx->LinkState = LinkState_Disconnected;

        /* Clear any pending outgoing messages. */
        (void) ptx_RUL_LLCP_ServiceAccessPoint_Init(&ctx->Local);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Link_Reset(ptx_RUL_LLCP_Link_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Link))
    {
        status = ptx_RUL_LLCP_ServiceAccessPoint_Init(&ctx->Local);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_LLCP_ServiceAccessPoint_Init(&ctx->Remote);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_LLCP_TrxState_Init(&ctx->TrxStates);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->LinkState = LinkState_Unregistered;
            ctx->BusyReleased = 0;
            memset(ctx->ServiceUri, 0, PTX_LLCP_LINK_DEFAULT_SERVICE_URI_LENGTH);

            ctx->InUse = 0u;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_Link_DeInit(ptx_RUL_LLCP_Link_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_Link))
    {
        status = ptx_RUL_LLCP_Link_Reset(ctx);

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->CompId = Comp_MAX;
        }
    }

    return status;
}
