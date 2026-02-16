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
    Module      : HAL Timer Interface
    File        : ptx_HAL_Tmr.c

    Description : HAL Timer Interface
                  Implementation of an abstract interface to access timers.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stddef.h>
#include <memory.h>

#include "ptx_HAL_Timer.h"

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
ptx_Status_t ptx_HAL_Tmr_Init (ptx_HAL_Tmr_t *ctx, ptx_HAL_Tmr_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams))
    {
        memset(&ctx->Drv, 0, sizeof(ptx_HAL_DRV_Tmr_t));

        ptx_HAL_DRV_Tmr_InitParams_t tmr_init_params;
        memset(&tmr_init_params, 0, sizeof(ptx_HAL_DRV_Tmr_InitParams_t));

#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
        /* ExtCtx only available for Win / Lin Tmr InitParams*/
        tmr_init_params.Osal = initParams->Osal;
#endif

        status = ptx_HAL_DRV_Tmr_Init(&ctx->Drv, &tmr_init_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->Open       = ptx_HAL_DRV_Tmr_Open;
            ctx->Close      = ptx_HAL_DRV_Tmr_Close;
            ctx->Start      = ptx_HAL_DRV_Tmr_Start;
            ctx->Stop       = ptx_HAL_DRV_Tmr_Stop;
            ctx->IsElapsed  = ptx_HAL_DRV_Tmr_IsElapsed;
            ctx->RegExecCb  = ptx_HAL_DRV_Tmr_RegExecCb;
            ctx->Cfg        = ptx_HAL_DRV_Tmr_Cfg;
            ctx->Sleep      = ptx_HAL_DRV_Tmr_Sleep;

            ctx->CompId     = Comp_HAL;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_Tmr_DeInit (ptx_HAL_Tmr_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = ptx_HAL_DRV_Tmr_DeInit(&ctx->Drv);

        if (PTX_STATUS_SUCCESS == status)
        {
            memset(ctx, 0, sizeof(ptx_HAL_Tmr_t));
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

