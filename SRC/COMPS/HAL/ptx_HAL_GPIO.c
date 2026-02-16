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
    Module      : HAL GPIO Interface
    File        : ptx_HAL_GPIO.c

    Description : HAL GPIO Interface
                  Implementation of an abstract HAL GPIO interface.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_HAL_GPIO.h"
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


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_HAL_GPIO_Init (ptx_HAL_GPIO_t *ctx, ptx_HAL_GPIO_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams))
    {
        memset(ctx, 0, sizeof(ptx_HAL_GPIO_t));

        /* Currently, only one GPIO type supported. */
        ptx_HAL_DRV_GPIO_InitParams_t init_params;
        memset(&init_params, 0, sizeof(ptx_HAL_DRV_GPIO_InitParams_t));

        init_params.Log = initParams->Log;

        /* Allocate GPIO Driver context. */
        memset(&ctx->Drv, 0, sizeof(ptx_HAL_DRV_GPIO_t));

        status = ptx_HAL_DRV_GPIO_Init(&ctx->Drv, &init_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->Open       = ptx_HAL_DRV_GPIO_Open;
            ctx->Close      = ptx_HAL_DRV_GPIO_Close;
            ctx->ReadPin    = ptx_HAL_DRV_GPIO_ReadPin;
            ctx->WritePin   = ptx_HAL_DRV_GPIO_WritePin;
            ctx->ReadPort   = ptx_HAL_DRV_GPIO_ReadPort;
            ctx->WritePort  = ptx_HAL_DRV_GPIO_WritePort;
            ctx->SetCfg     = ptx_HAL_DRV_GPIO_SetCfgPin;

            ctx->CompId     = Comp_HAL;
        }


    }

    return status;
}

ptx_Status_t ptx_HAL_GPIO_DeInit (ptx_HAL_GPIO_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        memset(ctx, 0, sizeof(ptx_HAL_GPIO_t));
        ctx->CompId = Comp_None;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

