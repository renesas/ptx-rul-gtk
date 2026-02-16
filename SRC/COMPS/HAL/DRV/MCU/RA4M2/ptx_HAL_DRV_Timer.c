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
    Module      : MCU-RA4M2 Timer Driver
    File        : ptx_HAL_DRV_Timer.c

    Description : Implementation of the Timer Driver for the MCU-RA4M2 Reference Platform


*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_DRV_Timer.h"
#include "ptx_HAL_DRV_MCU.h"

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

static ptx_HAL_DRV_Tmr_Port_t gDrvTimerPortConfigs[PTX_HAL_DRV_TMR_MAX_PORTS] =
{
    {
        .Instance       = &ptx_tmr0,
        .IsElapsed      = PTX_HAL_DRV_TMR_IS_ELAPSED_FALSE,
        .ExecCb         = NULL,
        .ExecCbCtx      = NULL,
        .Mode           = HAL_DRV_Tmr_Mode_Periodic,
        .TimerAssigned  = 0
    },
    {
        .Instance       = &ptx_tmr1,
        .IsElapsed      = PTX_HAL_DRV_TMR_IS_ELAPSED_FALSE,
        .ExecCb         = NULL,
        .ExecCbCtx      = NULL,
        .Mode           = HAL_DRV_Tmr_Mode_Periodic,
        .TimerAssigned  = 0
    }
};

extern void ptx_HAL_DRV_Tmr_ISR(timer_callback_args_t *p_args);

static fsp_err_t ptx_HAL_DRV_Tmr_PeriodSet (ptx_HAL_DRV_Tmr_t *tmr, ptx_HAL_DRV_Tmr_Cfg_t *cfg);
static fsp_err_t ptx_HAL_DRV_Tmr_ModeSet (ptx_HAL_DRV_Tmr_t *tmr, ptx_HAL_DRV_Tmr_Cfg_t *cfg);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_HAL_DRV_Tmr_Init (ptx_HAL_DRV_Tmr_t *ctx, ptx_HAL_DRV_Tmr_InitParams_t *initParams )
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams))
    {
        ctx->Port = NULL;

        for (uint8_t portID = 0; (PTX_HAL_DRV_TMR_MAX_PORTS > portID) && (NULL == ctx->Port); ++portID)
        {
            if (0 == gDrvTimerPortConfigs[portID].TimerAssigned)
            {
                ctx->Port = &gDrvTimerPortConfigs[portID];

                ctx->Id = portID;

                ctx->Port->TimerAssigned = 1;

                ctx->CompId = Comp_HAL;

                status = PTX_STATUS_SUCCESS;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_DeInit (ptx_HAL_DRV_Tmr_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        ctx->Port->ExecCb    = NULL;
        ctx->Port->ExecCbCtx = NULL;
        ctx->Port->TimerAssigned = 0;

        ctx->Port = NULL;
        ctx->Id = 0;
        ctx->CompId = Comp_None;

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
        fsp_err_t r_status = R_GPT_Open(tmr->Port->Instance->p_ctrl, tmr->Port->Instance->p_cfg);
        if (FSP_ERR_ALREADY_OPEN == r_status)
        {
            (void) R_GPT_Close(tmr->Port->Instance->p_ctrl);
            r_status = R_GPT_Open(tmr->Port->Instance->p_ctrl, tmr->Port->Instance->p_cfg);
        }

        if (FSP_SUCCESS == r_status)
        {
            r_status = R_GPT_CallbackSet(tmr->Port->Instance->p_ctrl, ptx_HAL_DRV_Tmr_ISR, tmr, NULL);
        }

        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_Close (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;

    if (PTX_COMP_CHECK(tmr, Comp_HAL))
    {
        fsp_err_t r_status = R_GPT_Close(tmr->Port->Instance->p_ctrl);
        if ((FSP_SUCCESS == r_status) || (FSP_ERR_NOT_OPEN == r_status))
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_HALTIMER_INTERNAL_CLOSE;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_Start (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;

    if (PTX_COMP_CHECK(tmr, Comp_HAL))
    {
        tmr->Port->IsElapsed = PTX_HAL_DRV_TMR_IS_ELAPSED_FALSE;

        fsp_err_t r_status = R_GPT_Start(tmr->Port->Instance->p_ctrl);
        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_HALTIMER_INTERNAL_START;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_Stop (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;

    if (PTX_COMP_CHECK(tmr, Comp_HAL))
    {
        fsp_err_t r_status = R_GPT_Stop(tmr->Port->Instance->p_ctrl);
        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_HALTIMER_INTERNAL_STOP;
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
        *isElapsed = tmr->Port->IsElapsed;

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
        tmr->Port->ExecCb    = cb;
        tmr->Port->ExecCbCtx = cbCtx;

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
        fsp_err_t err = FSP_ERR_INVALID_ARGUMENT;
        uint8_t idx = 0;
        do
        {
            switch (cfg[idx].CfgType)
            {
                case HAL_DRV_Tmr_CfgType_Period:
                    err = ptx_HAL_DRV_Tmr_PeriodSet(tmr, &cfg[idx]);
                    break;
                case HAL_DRV_Tmr_CfgType_Mode:
                    err = ptx_HAL_DRV_Tmr_ModeSet(tmr, &cfg[idx]);
                    break;
                default:
                    err = FSP_ERR_INVALID_ARGUMENT;
                    break;
            }
            ++idx;
        }
        while ((FSP_SUCCESS == err) && ((uint8_t)len > idx));

        if (err != FSP_SUCCESS)
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Tmr_Sleep (void *ctx, uint32_t timeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_Tmr_t *tmr = (ptx_HAL_DRV_Tmr_t*)ctx;

    if (PTX_COMP_CHECK(tmr, Comp_HAL) && (0 != timeout))
    {
        status = ptx_HAL_DRV_Tmr_Stop(tmr);
        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_HAL_DRV_Tmr_Cfg_t cfg;
            uint32_t len = 1;
            memset(&cfg, 0, sizeof(ptx_HAL_DRV_Tmr_Cfg_t));

            cfg.CfgType = HAL_DRV_Tmr_CfgType_Period;
            cfg.TimeUnit = HAL_DRV_Tmr_Unit_mSec;
            cfg.Period = timeout;
            status = ptx_HAL_DRV_Tmr_Cfg(tmr, &cfg, len);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* The timer was successfully stopped and configured. Now start it. */
            status = ptx_HAL_DRV_Tmr_Start(tmr);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            while (PTX_HAL_DRV_TMR_IS_ELAPSED_TRUE != tmr->Port->IsElapsed)
            {
                (void)ptx_HAL_DRV_MCU_DisableIrq();
                if (PTX_HAL_DRV_TMR_IS_ELAPSED_TRUE == tmr->Port->IsElapsed)
                {
                    ptx_HAL_DRV_MCU_EnableIrq();
                    break;
                }
                else
                {
                    ptx_HAL_DRV_MCU_WaitForIrq();
                    ptx_HAL_DRV_MCU_EnableIrq();
                }
            }

            (void)ptx_HAL_DRV_Tmr_Stop(tmr);
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

void ptx_HAL_DRV_Tmr_ISR(timer_callback_args_t *p_args)
{
    if (NULL != p_args)
    {
        ptx_HAL_DRV_Tmr_Port_t *config = ((ptx_HAL_DRV_Tmr_t *)p_args->p_context)->Port;

        if ((NULL != config->ExecCb) && (NULL != config->ExecCbCtx))
        {
            if ((HAL_DRV_Tmr_Mode_Periodic == config->Mode) ||
                ((HAL_DRV_Tmr_Mode_SingleShot == config->Mode) && (PTX_HAL_DRV_TMR_IS_ELAPSED_FALSE == config->IsElapsed)))
            {
                config->ExecCb(config->ExecCbCtx);
            }
        }

        config->IsElapsed = PTX_HAL_DRV_TMR_IS_ELAPSED_TRUE;

    }
    (void)p_args;
}

static fsp_err_t ptx_HAL_DRV_Tmr_PeriodSet (ptx_HAL_DRV_Tmr_t *tmr, ptx_HAL_DRV_Tmr_Cfg_t *cfg)
{
    fsp_err_t err = FSP_ERR_INVALID_ARGUMENT;
    uint32_t period = 0;

    uint32_t pclkd_freq_hz = R_FSP_SystemClockHzGet(FSP_PRIV_CLOCK_PCLKD) >> ((timer_cfg_t*)tmr->Port->Instance->p_cfg)->source_div;

    switch (cfg->TimeUnit)
    {
        case HAL_DRV_Tmr_Unit_mSec:
            period = (uint32_t)(((uint64_t)pclkd_freq_hz * cfg->Period) / PTX_HAL_DRV_TMR_MILLISEC_PER_SEC);
            err = FSP_SUCCESS;
            break;
        case HAL_DRV_Tmr_Unit_Sec:
            period = (uint32_t)((uint64_t)pclkd_freq_hz * cfg->Period);
            err = FSP_SUCCESS;
            break;
        default:
            err = FSP_ERR_INVALID_ARGUMENT;
    }

    if (0 == period)
    {
        err = FSP_ERR_INVALID_TIMING_SETTING;
    }

    if (FSP_SUCCESS == err)
    {
        err = R_GPT_PeriodSet(tmr->Port->Instance->p_ctrl, period);
    }

    return err;
}

static fsp_err_t ptx_HAL_DRV_Tmr_ModeSet (ptx_HAL_DRV_Tmr_t *tmr, ptx_HAL_DRV_Tmr_Cfg_t *cfg)
{
    fsp_err_t err = FSP_ERR_INVALID_ARGUMENT;

    switch (cfg->Mode)
    {
        case HAL_DRV_Tmr_Mode_Periodic:
            tmr->Port->Mode = cfg->Mode;
            err = FSP_SUCCESS;
            break;
        case HAL_DRV_Tmr_Mode_SingleShot:
            tmr->Port->Mode = cfg->Mode;
            err = FSP_SUCCESS;
            break;
        default:
            err = FSP_ERR_INVALID_ARGUMENT;
    }

    return err;
}

