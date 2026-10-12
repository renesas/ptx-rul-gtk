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
    Module      : MCU-RA4M2 GPIO Driver
    File        : ptx_HAL_DRV_GPIO.c

    Description : Implementation of the GPIO Driver for the MCU-RA4M2 Reference Platform
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_DRV_GPIO.h"

#include <stddef.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

static ptx_HAL_DRV_GPIO_PortConfig_t gDrvGpioPortConfig =
{
    .Instance   = &g_ioport,
};

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

ptx_Status_t ptx_HAL_DRV_GPIO_Init (ptx_HAL_DRV_GPIO_t *ctx, ptx_HAL_DRV_GPIO_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams) && (NULL != initParams->Log))
    {        
        PTX_LOG_V(initParams->Log, "ptx_HAL_DRV_GPIO_Init");

        (void)memset(ctx, 0, sizeof(ptx_HAL_DRV_GPIO_t));

        ctx->CompId    = Comp_HAL;
        ctx->Port      = &gDrvGpioPortConfig;
        ctx->Log       = initParams->Log;

        status = PTX_STATUS_SUCCESS;

    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_DeInit (ptx_HAL_DRV_GPIO_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_GPIO_DeInit");

        (void)ptx_HAL_DRV_GPIO_Close (ctx);
        memset(ctx, 0, sizeof(ptx_HAL_DRV_GPIO_t));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_Open (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)(ctx);

    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_Open");

        fsp_err_t r_status = R_IOPORT_Open(gpio->Port->Instance->p_ctrl, gpio->Port->Instance->p_cfg);
        if (FSP_ERR_ALREADY_OPEN == r_status)
        {
            (void)R_IOPORT_Close(gpio->Port->Instance->p_ctrl);
            r_status = R_IOPORT_Open(gpio->Port->Instance->p_ctrl, gpio->Port->Instance->p_cfg);
        }

        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_OPEN;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Default setup on reference board shall have RESET pin set as output-high pin (inactive state) */
            ptx_HAL_DRV_GPIO_Cfg_t  reset_pin_cfg;
            const uint32_t cfg_len = 1;
            reset_pin_cfg.Key = HAL_DRV_GPIO_CfgParam_PinDirection;
            reset_pin_cfg.Pin = PTX_HAL_DRV_GPIO_RESET_PIN;
            reset_pin_cfg.Value = PTX_HAL_DRV_GPIO_PINCFG_OUTPUT_HIGH;

            status = ptx_HAL_DRV_GPIO_SetCfgPin(gpio, &reset_pin_cfg, cfg_len);
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_Close (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)(ctx);

    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_Close");

        fsp_err_t r_status = R_IOPORT_Close(gpio->Port->Instance->p_ctrl);
        if ((FSP_SUCCESS == r_status) || (FSP_ERR_NOT_OPEN == r_status))
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_CLOSE;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_ReadPin (void *ctx, uint16_t pin, uint8_t *level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)(ctx);

    if (PTX_COMP_CHECK(gpio, Comp_HAL) && (NULL != level))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_ReadPin");

        fsp_err_t r_status = R_IOPORT_PinRead(gpio->Port->Instance->p_ctrl, pin, level);
        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_IRQ_READ;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_WritePin (void *ctx, uint16_t pin, uint8_t level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)(ctx);

    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_WritePin");

        fsp_err_t r_status = R_IOPORT_PinWrite(gpio->Port->Instance->p_ctrl, pin, level);
        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_IRQ_VALUE;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_ReadPort (void *ctx, uint32_t port, uint16_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)(ctx);

    if (PTX_COMP_CHECK(gpio, Comp_HAL) && (NULL != data))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_ReadPort");

        fsp_err_t r_status = R_IOPORT_PortRead(gpio->Port->Instance->p_ctrl, port, data);
        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_IRQ_READ;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_WritePort (void *ctx, uint32_t port, uint16_t data, uint16_t mask)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)(ctx);

    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_WritePort");

        fsp_err_t r_status = R_IOPORT_PortWrite(gpio->Port->Instance->p_ctrl, port, data, mask);
        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_IRQ_VALUE;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_SetCfgPin (void *ctx, ptx_HAL_DRV_GPIO_Cfg_t *cfg, uint32_t cfgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)(ctx);

    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_SetCfgPin");

        for (uint32_t i = 0; i < cfgLen; ++i)
        {
            fsp_err_t r_status = FSP_SUCCESS;

            switch (cfg[i].Key)
            {
                case HAL_DRV_GPIO_CfgParam_EnableExtIrq:
                    r_status = R_IOPORT_PinCfg(gpio->Port->Instance->p_ctrl, cfg[i].Pin, cfg[i].Value);
                    break;

                case HAL_DRV_GPIO_CfgParam_PinDirection:
                    r_status = R_IOPORT_PinCfg(gpio->Port->Instance->p_ctrl, cfg[i].Pin, cfg[i].Value);
                    break;

                case HAL_DRV_GPIO_CfgParam_PinPullUpEnable:
                    r_status = R_IOPORT_PinCfg(gpio->Port->Instance->p_ctrl, cfg[i].Pin, cfg[i].Value);
                    break;

                default:
                    r_status = FSP_ERR_NOT_OPEN;
                    break;
            }

            if (FSP_SUCCESS == r_status)
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                /* Abort the loop on any configuration failure */
                status = PTX_STATUS_E_GEN_INTERFACE_IRQ_DIRECTION;
                break;
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

