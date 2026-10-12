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
    Module      : Windows Timer Driver
    File        : ptx_HAL_DRV_Timer.c

    Description : Implementation of the Timer Driver for the Windows Reference Platform using FTDI
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_HAL_DRV_Timer.h"
#include "ptx_OSAL.h"

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
static void ptx_HAL_DRV_Tmr_ThdProc(void *timerThdParam);
static ptx_Status_t ptx_HAL_DRV_Tmr_PeriodSet (ptx_HAL_DRV_Tmr_t *tmr, ptx_HAL_DRV_Tmr_Cfg_t *cfg);
static ptx_Status_t ptx_HAL_DRV_Tmr_ModeSet (ptx_HAL_DRV_Tmr_t *tmr, ptx_HAL_DRV_Tmr_Cfg_t *cfg);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Init (ptx_HAL_DRV_Tmr_t *ctx, ptx_HAL_DRV_Tmr_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams) && (NULL != initParams->Log))
    {
        PTX_LOG_V(initParams->Log, "ptx_HAL_DRV_Tmr_Init");

        (void)memset(ctx, 0, sizeof(ptx_HAL_DRV_Tmr_t));

        ptx_OSAL_t *osal = NULL;

        /** OSAL needs to parsed down from upper layers. */
        if (PTX_COMP_CHECK((ptx_OSAL_t *)(initParams->Osal), Comp_OSAL))
        {
            osal = initParams->Osal;
            ctx->Osal       = osal;
            ctx->CompId     = Comp_HAL;
            ctx->Log        = initParams->Log;
            (void)memset(&ctx->Timer, 0, sizeof(ptx_Timer_t));
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_DeInit (ptx_HAL_DRV_Tmr_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Tmr_DeInit");

        ptx_OSAL_t *osal = (ptx_OSAL_t *)ctx->Osal;

        (void)ptx_OSAL_TimerClose(osal, &ctx->Timer);
        (void)memset(ctx, 0, sizeof(ptx_HAL_DRV_Tmr_t));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_Open (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;
    if (PTX_COMP_CHECK(tmr, Comp_HAL))
    {
        PTX_LOG_V(tmr->Log, "ptx_HAL_DRV_Tmr_Open");

        ptx_OSAL_t *osal = (ptx_OSAL_t *)tmr->Osal;
        status = ptx_OSAL_TimerCreate(osal, &tmr->Timer, &ptx_HAL_DRV_Tmr_ThdProc, tmr);
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_Close (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;

    if (PTX_COMP_CHECK(tmr, Comp_HAL))
    {
        PTX_LOG_V(tmr->Log, "ptx_HAL_DRV_Tmr_Close");

        ptx_OSAL_t *osal = (ptx_OSAL_t *)tmr->Osal;
        status = ptx_OSAL_TimerClose(osal, &tmr->Timer);
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_Start (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;

    if (PTX_COMP_CHECK(tmr, Comp_HAL))
    {
        PTX_LOG_V(tmr->Log, "ptx_HAL_DRV_Tmr_Start");

        ptx_OSAL_t *osal = (ptx_OSAL_t *)tmr->Osal;

        tmr->IsElapsed = PTX_HAL_DRV_TMR_IS_ELAPSED_FALSE;
        status = ptx_OSAL_TimerStart(osal, &tmr->Timer, tmr->Period);
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_Stop (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;

    if (PTX_COMP_CHECK(tmr, Comp_HAL))
    {
        PTX_LOG_V(tmr->Log, "ptx_HAL_DRV_Tmr_Stop");

        ptx_OSAL_t *osal = (ptx_OSAL_t *)tmr->Osal;

        if (0 != tmr->Timer.Running)
        {
            status = ptx_OSAL_TimerCancel(osal, &tmr->Timer);
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_IsElapsed (void *ctx, uint8_t *isElapsed)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;

    if (PTX_COMP_CHECK(tmr, Comp_HAL) && (NULL != isElapsed))
    {
        PTX_LOG_V(tmr->Log, "ptx_HAL_DRV_Tmr_IsElapsed");

        *isElapsed = tmr->IsElapsed;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_RegExecCb (void *ctx, ptx_HAL_DRV_Tmr_ExecCb_t cb, ptx_HAL_DRV_Tmr_ExecCbCtx_t cbCtx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;

    if (PTX_COMP_CHECK(tmr, Comp_HAL) && (NULL != cb) && (NULL != cbCtx))
    {
        PTX_LOG_V(tmr->Log, "ptx_HAL_DRV_Tmr_RegExecCb");

        tmr->ExecCb    = cb;
        tmr->ExecCbCtx = cbCtx;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_Cfg (void *ctx, ptx_HAL_DRV_Tmr_Cfg_t cfg[], uint32_t len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;

    if (PTX_COMP_CHECK(tmr, Comp_HAL) && (NULL != cfg) && (0 < len))
    {
        PTX_LOG_V(tmr->Log, "ptx_HAL_DRV_Tmr_Cfg");

        for (uint8_t i = 0; i < (uint8_t)len; i++)
        {
            switch (cfg[i].CfgType)
            {
                case HAL_DRV_Tmr_CfgType_Period:
                    status = ptx_HAL_DRV_Tmr_PeriodSet(tmr, &cfg[i]);
                    break;
                case HAL_DRV_Tmr_CfgType_Mode:
                    status = ptx_HAL_DRV_Tmr_ModeSet(tmr, &cfg[i]);
                    break;
                default:
                    break;
            }

            if (PTX_STATUS_SUCCESS != status)
            {
                break;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_Sleep (void *ctx, uint32_t timeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;

    if (PTX_COMP_CHECK(tmr, Comp_HAL))
    {
        PTX_LOG_V(tmr->Log, "ptx_HAL_DRV_Tmr_Sleep");

        if (0 != timeout)
        {
            ptx_OSAL_t *osal = (ptx_OSAL_t *)tmr->Osal;
            ptx_OSAL_ThreadSleep(osal, timeout);
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static void ptx_HAL_DRV_Tmr_ThdProc (void *timerThdParam)
{
    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)timerThdParam;
    if (PTX_COMP_CHECK(tmr, Comp_HAL))
    {
        PTX_LOG_V(tmr->Log, "ptx_HAL_DRV_Tmr_Sleep");

        tmr->IsElapsed = PTX_HAL_DRV_TMR_IS_ELAPSED_TRUE;

        if ((NULL != tmr->ExecCb) && (NULL != tmr->ExecCbCtx))
        {
            (void)tmr->ExecCb(tmr->ExecCbCtx);
        }
    }
}

static ptx_Status_t ptx_HAL_DRV_Tmr_PeriodSet (ptx_HAL_DRV_Tmr_t *tmr, ptx_HAL_DRV_Tmr_Cfg_t *cfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    if (PTX_COMP_CHECK(tmr, Comp_HAL) && (NULL != cfg))
    {
        PTX_LOG_V(tmr->Log, "ptx_HAL_DRV_Tmr_PeriodSet");

        if (0 != cfg->Period)
        {
            switch (cfg->TimeUnit)
            {
                case HAL_DRV_Tmr_Unit_Sec:
                    tmr->Period = cfg->Period * 1000;
                    status = PTX_STATUS_SUCCESS;
                    break;
                case HAL_DRV_Tmr_Unit_mSec:
                    tmr->Period = cfg->Period;
                    status = PTX_STATUS_SUCCESS;
                    break;
                default:
                    break;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            tmr->Timer.Interval = tmr->Period;
        }
    }
    return status;
}

static ptx_Status_t ptx_HAL_DRV_Tmr_ModeSet (ptx_HAL_DRV_Tmr_t *tmr, ptx_HAL_DRV_Tmr_Cfg_t *cfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    if (PTX_COMP_CHECK(tmr, Comp_HAL) && (NULL != cfg))
    {
        PTX_LOG_V(tmr->Log, "ptx_HAL_DRV_Tmr_ModeSet");

        switch (cfg->Mode)
        {
            case HAL_DRV_Tmr_Mode_Periodic:
                tmr->Mode = cfg->Mode;
                status = PTX_STATUS_SUCCESS;
                break;
            case HAL_DRV_Tmr_Mode_SingleShot:
                tmr->Mode = cfg->Mode;
                status = PTX_STATUS_SUCCESS;
                break;
            default:
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            tmr->Timer.Mode = tmr->Mode;
        }
    }

    return status;
}