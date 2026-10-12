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
    Module      : HAL Main Interface
    File        : ptx_HAL.c

    Description : Hardware Abstraction Layer (HAL) main interface.
                  Contains functions of top-level HAL interface functions
                  to interact with NFC HW.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL.h"
#include "ptx_HAL_DRV_MCU.h"
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

ptx_Status_t ptx_HAL_Init (ptx_HAL_t *ctx, ptx_HAL_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams) && (NULL != initParams->Log))
    {
        PTX_LOG_V(initParams->Log, "ptx_HAL_Init");

        const uint8_t max_timer_count = (HAL_Tmr_Id_MAX > PTX_HAL_DRV_TMR_MAX_PORTS) ? PTX_HAL_DRV_TMR_MAX_PORTS : HAL_Tmr_Id_MAX;
        const uint8_t timer_count = (max_timer_count > initParams->NumberOfTimers) ? initParams->NumberOfTimers : max_timer_count;

        /* Clean up HAL structure before we start initialising */
        (void)memset(ctx, 0, sizeof(ptx_HAL_t));

        /* Temporarily set valid CompId which is required for the following Open APIs */
        ctx->CompId = Comp_HAL;
        ctx->Log = initParams->Log;


        /* Initialise GPIO module */
        ptx_HAL_GPIO_InitParams_t gpio_init_params;
        memset(&gpio_init_params, 0, sizeof(ptx_HAL_GPIO_InitParams_t));
        gpio_init_params.Log = initParams->Log;

        status = ptx_HAL_GPIO_Init(&ctx->Gpio, &gpio_init_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_GPIO_Open(ctx);
        }

        if (PTX_STATUS_SUCCESS == status)
        {


            /* Initialise and open communication device */
            ptx_HAL_Interface_InitParams_t interface_init_params;
            (void)memset(&interface_init_params, 0, sizeof(ptx_HAL_Interface_InitParams_t));
            interface_init_params.Name = initParams->InterfaceName;
            interface_init_params.NameLen = initParams->InterfaceNameLen;
            interface_init_params.Log = initParams->Log;
            interface_init_params.Speed = initParams->InterfaceSpeed;
            interface_init_params.Type = initParams->InterfaceType;
            interface_init_params.PortID = initParams->InterfacePortID;
            interface_init_params.PortType = initParams->InterfacePortType;
            interface_init_params.Gpio = &ctx->Gpio;
            interface_init_params.Log = initParams->Log;

#if defined(PTX_APP_DIVERSITY_WINDOWS) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
            interface_init_params.Osal = initParams->Osal;
#endif
            status = ptx_HAL_Interface_Init(&ctx->Interface, &interface_init_params);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_Interface_Open(ctx);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Initialise and open timers. The number of timers is defined in DRV implementation. */
            for (uint8_t i = 0; i < timer_count; i++)
            {
                ptx_HAL_Tmr_InitParams_t tmr_init_params;
                memset(&tmr_init_params, 0, sizeof(ptx_HAL_Tmr_InitParams_t));

                tmr_init_params.Log = initParams->Log;

#if defined(PTX_APP_DIVERSITY_WINDOWS) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
                tmr_init_params.Osal = initParams->Osal;
#endif

                status = ptx_HAL_Tmr_Init(&ctx->Tmr[i], &tmr_init_params);
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_HAL_Tmr_Open(ctx, (ptx_HAL_Tmr_Id_t)i);
                }

                if (PTX_STATUS_SUCCESS != status)
                {
                    /* Stop the loop on the first failure */
                    break;
                }

                ++ctx->NumberOfTimers;
            }
        }

        if (PTX_STATUS_SUCCESS != status)
        {
            /* If any of the initialisation steps failed, do the clean up */

            /* Device cleanup */
            (void)ptx_HAL_Interface_Close(ctx);
            (void)ptx_HAL_Interface_DeInit(&ctx->Interface);

            /* GPIO Cleanup */
            (void)ptx_HAL_GPIO_Close(ctx);
            (void)ptx_HAL_GPIO_DeInit(&ctx->Gpio);

            /* Timer cleanup */
            for (uint8_t i = 0; i < timer_count; i++)
            {
                (void)ptx_HAL_Tmr_Close (ctx, (ptx_HAL_Tmr_Id_t)i);
                (void)ptx_HAL_Tmr_DeInit(&ctx->Tmr[i]);
            }

            /* Finally, clean main HAL component */
            (void)memset(ctx, 0, sizeof(ptx_HAL_t));
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DeInit (ptx_HAL_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DeInit");

        status = ptx_HAL_Interface_Close(ctx);
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_Interface_DeInit(&ctx->Interface);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_GPIO_Close(ctx);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_GPIO_DeInit(&ctx->Gpio);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            const uint8_t max_timer_count = (HAL_Tmr_Id_MAX > PTX_HAL_DRV_TMR_MAX_PORTS) ? PTX_HAL_DRV_TMR_MAX_PORTS : HAL_Tmr_Id_MAX;
            const uint8_t timer_count = (max_timer_count > ctx->NumberOfTimers) ? ctx->NumberOfTimers : max_timer_count;

            for (uint8_t i = 0; i < timer_count; i++)
            {
                status = ptx_HAL_Tmr_Close (ctx, (ptx_HAL_Tmr_Id_t)i);
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_HAL_Tmr_DeInit(&ctx->Tmr[i]);
                }

                if (PTX_STATUS_SUCCESS != status)
                {
                    /* Timer DeInit failed. Abort. */
                    break;
                }
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Only if all went well i.e. all parts successfully de-initialized, the structure shall be clean and made invalid.
             * Otherwise, keep the component valid.
             * Each sub component DeInit shall clean up its structure. Here just set CompId to an invalid value.
             */
            ctx->CompId = Comp_None;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_Open (ptx_HAL_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Interface.Open))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Interface_Open");

        status = ctx->Interface.Open((void*)(&ctx->Interface));
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_Close (ptx_HAL_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Interface.Close))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Interface_Close");

        status = ctx->Interface.Close((void*)(&ctx->Interface));
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_TRx (ptx_HAL_t *ctx, uint8_t *txBuf[], uint32_t txLen[], uint32_t txBufferCnt,
                                    uint8_t *rxBuf[], uint32_t rxLen[], uint32_t rxBufferCnt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Interface.TRx))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Interface_TRx");

        status = ctx->Interface.TRx((void*)(&ctx->Interface), txBuf, txLen, txBufferCnt, rxBuf, rxLen, rxBufferCnt);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_Tx (ptx_HAL_t *ctx, uint8_t *txBuf, uint32_t txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Interface.Tx))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Interface_Tx");

        status = ctx->Interface.Tx((void*)(&ctx->Interface), txBuf, txLen);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_Rx (ptx_HAL_t *ctx, uint8_t *rxBuf, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Interface.Rx))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Interface_Rx");

        status = ctx->Interface.Rx((void*)(&ctx->Interface), rxBuf, rxLen);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_RxPending(ptx_HAL_t * ctx, uint8_t* rxPending)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Interface.RxPending))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Interface_RxPending");

        status = ctx->Interface.RxPending((void*)(&ctx->Interface), rxPending);

        if ((PTX_STATUS_SUCCESS == status) && (0 != *rxPending))
        {
            PTX_LOG_I(ctx->Log, "Detected IRQ");
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_SetParameter(ptx_HAL_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx) && (NULL != ctx->Interface.SetParameter))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Interface_SetParameter");

        status = ctx->Interface.SetParameter((void*)(&ctx->Interface), parameter);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_GetParameter(ptx_HAL_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx) && (NULL != ctx->Interface.GetParameter))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Interface_GetParameter");

        status = ctx->Interface.GetParameter((void*)(&ctx->Interface), parameter);
    }

    return status;
}

ptx_Status_t ptx_HAL_GPIO_Open (ptx_HAL_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Gpio.Open))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_GPIO_Open");

        status = ctx->Gpio.Open((void*)(&ctx->Gpio.Drv));
    }

    return status;
}

ptx_Status_t ptx_HAL_GPIO_Close (ptx_HAL_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Gpio.Close))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_GPIO_Close");

        status = ctx->Gpio.Close((void*)(&ctx->Gpio.Drv));
    }

    return status;
}

ptx_Status_t ptx_HAL_GPIO_ReadPin (ptx_HAL_t *ctx, uint16_t pin, uint8_t *level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Gpio.ReadPin))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_GPIO_ReadPin");

        status = ctx->Gpio.ReadPin((void*)(&ctx->Gpio.Drv), pin, level);
    }

    return status;
}

ptx_Status_t ptx_HAL_GPIO_WritePin (ptx_HAL_t *ctx, uint16_t pin, uint8_t level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Gpio.WritePin))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_GPIO_WritePin");

        status = ctx->Gpio.WritePin((void*)(&ctx->Gpio.Drv), pin, level);
    }

    return status;
}

ptx_Status_t ptx_HAL_GPIO_ReadPort (ptx_HAL_t *ctx, uint32_t port, uint16_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Gpio.ReadPort))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_GPIO_ReadPort");

        status = ctx->Gpio.ReadPort((void*)(&ctx->Gpio.Drv), port, data);
    }

    return status;
}

ptx_Status_t ptx_HAL_GPIO_WritePort (ptx_HAL_t *ctx, uint32_t port, uint16_t data, uint16_t mask)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Gpio.WritePort))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_GPIO_WritePort");

        status = ctx->Gpio.WritePort((void*)(&ctx->Gpio.Drv), port, data, mask);
    }

    return status;
}

ptx_Status_t ptx_HAL_GPIO_SetCfgPin (ptx_HAL_t *ctx, ptx_HAL_DRV_GPIO_Cfg_t cfg[], uint32_t cfgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != ctx->Gpio.SetCfg))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_GPIO_SetCfgPin");

        status = ctx->Gpio.SetCfg((void*)(&ctx->Gpio.Drv), cfg, cfgLen);
    }

    return status;
}

ptx_Status_t ptx_HAL_EnableIrq (ptx_HAL_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_EnableIrq");
        status = ptx_HAL_DRV_MCU_EnableIrq();
    }

    return status;
}

ptx_Status_t ptx_HAL_DisableIrq (ptx_HAL_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DisableIrq");

        status = ptx_HAL_DRV_MCU_DisableIrq();
    }
    return status;
}

ptx_Status_t ptx_HAL_WaitForIrq (ptx_HAL_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_WaitForIrq");

        status = ptx_HAL_DRV_MCU_WaitForIrq();
    }
    return status;
}

ptx_Status_t ptx_HAL_Tmr_Open (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (HAL_Tmr_Id_MAX > id) && (NULL != ctx->Tmr[id].Open))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Tmr_Open");

        status = ctx->Tmr[id].Open((void*)(&ctx->Tmr[id].Drv));
    }

    return status;
}

ptx_Status_t ptx_HAL_Tmr_Close (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (HAL_Tmr_Id_MAX > id) && (NULL != ctx->Tmr[id].Close))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Tmr_Close");

        status = ctx->Tmr[id].Close((void*)(&ctx->Tmr[id].Drv));
    }

    return status;
}

ptx_Status_t ptx_HAL_Tmr_Start (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id, ptx_HAL_DRV_Tmr_Unit_t unit, uint32_t ticks)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (HAL_Tmr_Id_MAX > id) && (NULL != ctx->Tmr[id].Start) && (NULL != ctx->Tmr[id].Cfg) && (NULL != ctx->Tmr[id].Stop))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Tmr_Start");

        uint32_t len = 1;

        status = ctx->Tmr[id].Stop((void*)(&ctx->Tmr[id].Drv));

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_HAL_DRV_Tmr_Cfg_t cfg;
            memset(&cfg, 0, sizeof(ptx_HAL_DRV_Tmr_Cfg_t));

            cfg.CfgType = HAL_DRV_Tmr_CfgType_Period;
            cfg.TimeUnit = unit;
            cfg.Period = ticks;
            

            status = ctx->Tmr[id].Cfg((void*)(&ctx->Tmr[id].Drv), &cfg, len);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ctx->Tmr[id].Start((void*)(&ctx->Tmr[id].Drv));
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_Tmr_Stop (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (HAL_Tmr_Id_MAX > id) && (NULL != ctx->Tmr[id].Stop))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Tmr_Stop");

        status = ctx->Tmr[id].Stop((void*)(&ctx->Tmr[id].Drv));
    }

    return status;
}

ptx_Status_t ptx_HAL_Tmr_IsElapsed (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id, uint8_t *isElapsed)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (HAL_Tmr_Id_MAX > id) && (NULL != ctx->Tmr[id].IsElapsed))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Tmr_IsElapsed");

        status = ctx->Tmr[id].IsElapsed((void*)(&ctx->Tmr[id].Drv), isElapsed);
    }

    return status;
}

ptx_Status_t ptx_HAL_Tmr_RegExecCb (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id, ptx_HAL_DRV_Tmr_ExecCb_t cb, ptx_HAL_DRV_Tmr_ExecCbCtx_t cbCtx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (HAL_Tmr_Id_MAX > id) && (NULL != ctx->Tmr[id].RegExecCb))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Tmr_RegExecCb");

        status = ctx->Tmr[id].RegExecCb((void*)(&ctx->Tmr[id].Drv), cb, cbCtx);
    }

    return status;
}

ptx_Status_t ptx_HAL_Tmr_Cfg (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id, ptx_HAL_DRV_Tmr_Cfg_t cfg[], uint32_t len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (HAL_Tmr_Id_MAX > id) && (NULL != ctx->Tmr[id].Cfg))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Tmr_Cfg");

        status = ctx->Tmr[id].Cfg((void*)(&ctx->Tmr[id].Drv), cfg, len);
    }

    return status;
}

ptx_Status_t ptx_HAL_Sleep (ptx_HAL_t *ctx, uint32_t msTimeout )
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * This API shall use the default HAL timer (ID 0) to generate delays. This needs to be taken into account
     * if 2 timed operations are supposed to run simultaneously. If the default timer has already been started,
     * it shall be stopped and restarted.
     */

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (0 != msTimeout) && (NULL != ctx->Tmr[HAL_Tmr_Id_0].Sleep))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Sleep");

        status = ctx->Tmr[HAL_Tmr_Id_0].Sleep((void*)(&ctx->Tmr[HAL_Tmr_Id_0].Drv), msTimeout);
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

