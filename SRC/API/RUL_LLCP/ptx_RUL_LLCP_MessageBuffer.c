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
    File        : ptx_RUL_LLCP_MessageBuffer.c

    Description : Implementation of the LLCP message buffer required for potential retransmission.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL_LLCP_MessageBuffer.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_LLCP_MESSAGEBUFFER_UNINITIALIZED        (0xFFFFFFFF)    /**< Messagebuffer uninitializes/unused value. */

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
ptx_Status_t ptx_RUL_LLCP_MessageBuffer_Init(ptx_RUL_LLCP_MessageBuffer_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {
        /* Set the component ID. */
        ctx->CompId = Comp_LLCP_MessageBuffer;

        /* Initialize the buffer by clearing everything. */
        status = ptx_RUL_LLCP_MessageBuffer_Clear(ctx);
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_MessageBuffer_Write(ptx_RUL_LLCP_MessageBuffer_t *ctx, uint8_t *data, uint32_t dataLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_MessageBuffer) && (NULL != data) && (PTX_LLCP_MESSAGEBUFFER_MESSAGELENGTH >= dataLength))
    {
        /* Copy the length information. */
        ctx->Messages[ctx->WriteIndex].Length = dataLength;

        /* Copy the data. */
        memcpy(ctx->Messages[ctx->WriteIndex].Data, data, ctx->Messages[ctx->WriteIndex].Length);

        /* Increment write index. */
        ctx->WriteIndex = (ctx->WriteIndex + 1) % PTX_LLCP_MESSAGEBUFFER_MESSAGES;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_MessageBuffer_Read(ptx_RUL_LLCP_MessageBuffer_t *ctx, uint8_t *data, uint32_t *dataLength, uint8_t removeFromBuffer)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_MessageBuffer) && (NULL != data) && (NULL != dataLength))
    {
        /* Check if the entry stores some data. */
        if (PTX_LLCP_MESSAGEBUFFER_UNINITIALIZED != ctx->Messages[ctx->ReadIndex].Length)
        {
            /* Check if the destination size is large enough. */
            if (*dataLength >= ctx->Messages[ctx->ReadIndex].Length)
            {
                /* Copy the data. */
                memcpy(data, ctx->Messages[ctx->ReadIndex].Data, ctx->Messages[ctx->ReadIndex].Length);

                /* Copy the length information. */
                *dataLength = ctx->Messages[ctx->ReadIndex].Length;

                if (0 != removeFromBuffer)
                {
                    /* Set the length information to zero (= ready to be filled with data). */
                    ctx->Messages[ctx->ReadIndex].Length = PTX_LLCP_MESSAGEBUFFER_UNINITIALIZED;

                    /* Increment the read index */
                    ctx->ReadIndex = (ctx->ReadIndex + 1) % PTX_LLCP_MESSAGEBUFFER_MESSAGES;
                }

                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                /* Length of the destination buffer too small. */
                status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            }
        }
        else
        {
            /* Nothing to read. */
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_MessageBuffer_Clear(ptx_RUL_LLCP_MessageBuffer_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_MessageBuffer))
    {
        /* Clear all the buffers. */
        memset(&ctx->Messages, 0, sizeof(ptx_RUL_LLCP_Message_t) * PTX_LLCP_MESSAGEBUFFER_MESSAGES);

        for (uint32_t i = 0; i < PTX_LLCP_MESSAGEBUFFER_MESSAGES; ++i)
        {
            ctx->Messages[i].Length = PTX_LLCP_MESSAGEBUFFER_UNINITIALIZED;
        }

        /* Reset the indices. */
        ctx->WriteIndex = 0;
        ctx->ReadIndex = 0;

        /* Set the status to success. */
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_MessageBuffer_Size(ptx_RUL_LLCP_MessageBuffer_t *ctx, uint32_t *size)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_MessageBuffer) && (NULL != size))
    {
        /* If WriteIndex matches ReadIndex, the MessageBuffer is empty. */
        if (ctx->WriteIndex == ctx->ReadIndex)
        {
            *size = 0;
        }
        else
        {
            *size = ctx->WriteIndex - ctx->ReadIndex;

            if (ctx->WriteIndex < ctx->ReadIndex)
            {
                /* If the index wraps around we have to add the max. possible capacity. */
                *size = *size + PTX_LLCP_MESSAGEBUFFER_MESSAGES;
            }
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_LLCP_MessageBuffer_DeInit(ptx_RUL_LLCP_MessageBuffer_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_LLCP_MessageBuffer))
    {
        /* Delete everything what's left iwhtin the buffers. */
        status = ptx_RUL_LLCP_MessageBuffer_Clear(ctx);

        /* In case of success, also clear the component ID. */
        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->CompId = Comp_MAX;
        }
    }

    return status;
}
