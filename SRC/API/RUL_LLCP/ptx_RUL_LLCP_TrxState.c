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
    File        : ptx_RUL_LLCP_TrxState.c

    Description : Implementation handling the state variables of an LLCP connection.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL_LLCP_TrxState.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_LLCP_CONNECTION_STATE_RW (16u)
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
ptx_Status_t ptx_RUL_LLCP_TrxState_Init(ptx_RUL_LLCP_TrxState_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {
        /* Set the component ID. */
        ctx->CompId = Comp_LLCP_TrxState;

        /* Initialize the buffer by clearing everything. */
        status = ptx_RUL_LLCP_TrxState_Reset(ctx);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TrxState_Reset(ptx_RUL_LLCP_TrxState_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_TrxState))
    {
        /* Reset all internal state variables to zero. */
        ctx->Vr     = 0;
        ctx->Vra    = 0;
        ctx->Vs     = 0;
        ctx->Vsa    = 0;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TrxState_IncrementSendState(ptx_RUL_LLCP_TrxState_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_TrxState))
    {
        ctx->Vs = (ctx->Vs + 1) % PTX_LLCP_CONNECTION_STATE_RW;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TrxState_IncrementReceiveState(ptx_RUL_LLCP_TrxState_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_TrxState))
    {
        ctx->Vr = (ctx->Vr + 1) % PTX_LLCP_CONNECTION_STATE_RW;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TrxState_SetSendAcknowledgeState(ptx_RUL_LLCP_TrxState_t *ctx, uint8_t vsa)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_TrxState))
    {
        ctx->Vsa = vsa % PTX_LLCP_CONNECTION_STATE_RW;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TrxState_SetReceiveAcknowledgeState(ptx_RUL_LLCP_TrxState_t *ctx, uint8_t vra)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_TrxState))
    {
        ctx->Vra = vra % PTX_LLCP_CONNECTION_STATE_RW;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_TrxState_DeInit(ptx_RUL_LLCP_TrxState_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_TrxState))
    {
        /* Delete everything what's left iwhtin the buffers. */
        status = ptx_RUL_LLCP_TrxState_Reset(ctx);

        /* In case of success, also clear the component ID. */
        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->CompId = Comp_MAX;
        }
    }

    return status;
}