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
    Module      : Windows GPIO Driver
    File        : ptx_HAL_DRV_GPIO.c

    Description : Implementation of the GPIO Driver for the Windows Reference Platform using FTDI
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_HAL_DRV_GPIO.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#if !defined(PTX_ENABLE_UART)
    static const uint16_t FTDI_MAX_GPIO_PINS = 8;

    /*
    * ####################################################################################################################
    * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
    * ####################################################################################################################
    */
    static ptx_Status_t ptx_HAL_MapGPIOPin(uint16_t pinNr, ptx_HAL_FTD2XX_GPIO_t *gpio_pin);
#endif
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_HAL_DRV_GPIO_Init (ptx_HAL_DRV_GPIO_t *ctx, ptx_HAL_DRV_GPIO_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    
    if ((NULL != ctx)&& (NULL != initParams) && (NULL != initParams->Log))
    {
        PTX_LOG_V(initParams->Log, "ptx_HAL_DRV_GPIO_Init");

        ctx->CompId = Comp_HAL;
        ctx->Gpio_only = 0; /* per default GPIO is not used standalone */
        ctx->Log = initParams->Log;
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

        memset(ctx, 0, sizeof(ptx_HAL_DRV_GPIO_t));
        ctx->CompId = Comp_None;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_Open (void *ctx)
{
#if !defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;

    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_Open");
        
        status = ptx_HAL_FTD2XX_LibOpen((void**)&gpio->Ftd2xx_ctx, MODULE_DESCRIPTOR, NULL, gpio->Log);


        if (PTX_STATUS_SUCCESS == status)
        {
            /* GPIO usage requires MPSSE engine therefore I2C or SPI interface can be used for opening the device */
            status = ptx_HAL_FTD2XX_SingleDeviceOpen(gpio->Ftd2xx_ctx, FTDI_DEV_INTF_I2C);

            if (PTX_STATUS_SUCCESS == status && gpio->Gpio_only == 1)
            {
                status = ptx_HAL_FTD2XX_CheckOpening(gpio->Ftd2xx_ctx, FTDI_DEV_INTF_MPSSE_ONLY, 0);
            }
        }
    }
#endif

#if defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_NOT_AVAILABLE; /* Init and open must return SUCCESS */

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;
    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        if (gpio->Gpio_only == 1)
        {
            /* GPIO usage only not available on WIN - UART */
            status = PTX_STATUS_E_GEN_NOT_AVAILABLE;
        }
        else
        {
            status = ptx_HAL_FTD2XX_LibOpen((void**)&gpio->Ftd2xx_ctx, MODULE_DESCRIPTOR, NULL, gpio->Log);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* GPIO usage requires MPSSE engine therefore I2C or SPI interface can be used for opening the device */
            status = ptx_HAL_FTD2XX_SingleDeviceOpen(gpio->Ftd2xx_ctx, FTDI_DEV_INTF_I2C);
        }
    }

#endif

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_Close (void *ctx)
{
#if !defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;

    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_Close");

        if (gpio->Gpio_only)
        {
            status = ptx_HAL_FTD2XX_LibClose(gpio->Ftd2xx_ctx);
        }
        else
        {
            /** NOTHING TODO */
            status = PTX_STATUS_SUCCESS;
        }
    }
#endif

#if defined(PTX_ENABLE_UART)
    /* Nothing todo since gpio->Gpio_only is not supported for WIN UART */
    ptx_Status_t status = PTX_STATUS_SUCCESS; /* Init and open must return SUCCESS */
    PTX_UNUSED_PARAMETER(ctx);
#endif
    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_ReadPin (void *ctx, uint16_t pin, uint8_t *level)
{
#if !defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;

    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_ReadPin");

        ptx_HAL_FTD2XX_GPIO_t gpio_pin;
        status = ptx_HAL_MapGPIOPin(pin, &gpio_pin);
        uint16_t lvl = (uint16_t) * level;
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_FTD2XX_GetGPIOLevel(gpio->Ftd2xx_ctx, gpio_pin, &lvl);
            *level = (uint8_t) lvl;
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }
#endif

#if defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_NOT_AVAILABLE; /* Return error GPIO not available under FTD2XX UART */
    PTX_UNUSED_PARAMETER(ctx);
    PTX_UNUSED_PARAMETER(pin);
    PTX_UNUSED_PARAMETER(level);
#endif
    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_WritePin (void *ctx, uint16_t pin, uint8_t level)
{
#if !defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;
    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        ptx_HAL_FTD2XX_GPIO_t gpio_pin;
        status = ptx_HAL_MapGPIOPin(pin, &gpio_pin);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_FTD2XX_SetGPIOLevel(gpio->Ftd2xx_ctx, gpio_pin, level);
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }
#endif

#if defined(PTX_ENABLE_UART)

    ptx_Status_t status = PTX_STATUS_E_GEN_NOT_AVAILABLE;
    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;

    /*
        Generally GPIO's are not available using UART & FTDI. RTS can be controlled, but
        needs to be mapped to the 2K reset pin on the HW manually!
        Only if GPIO pin is equal to the default reset pin this function shall not fail and set RTS Pin.
        */
    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        if (PTX_HAL_DRV_GPIO_RESET_PIN == pin && 0 == level)
        {
            status = ptx_HAL_FTD2XX_UART_SetRts(gpio->Ftd2xx_ctx);
        }
        else if (PTX_HAL_DRV_GPIO_RESET_PIN == pin && 1 == level)
        {
            status = ptx_HAL_FTD2XX_UART_ClrRts(gpio->Ftd2xx_ctx);
        }
        else
        {
            /* Return error other GPIO's not available under FTD2XX UART only Reset through RTS pin */
        }
    }
#endif
    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_ReadPort (void *ctx, uint32_t port, uint16_t *data)
{
#if !defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    /** There is only one port for all 8 available GPIO pins */
    PTX_UNUSED_PARAMETER(port);

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;
    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_ReadPort");

        uint8_t data8 = 0x00;
        status = ptx_HAL_FTD2XX_GetHighPinsState(gpio->Ftd2xx_ctx, &data8);
        *data = (uint16_t) data8;
    }
#endif

#if defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_NOT_AVAILABLE; /* Return error GPIO not available under FTD2XX UART */
    PTX_UNUSED_PARAMETER(ctx);
    PTX_UNUSED_PARAMETER(port);
    PTX_UNUSED_PARAMETER(data);
#endif
    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_WritePort (void *ctx, uint32_t port, uint16_t data, uint16_t mask)
{
#if !defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    /** There is only one port for all 8 available GPIO pins */
    PTX_UNUSED_PARAMETER(port);

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;

    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_ReadPort");

        status = ptx_HAL_FTD2XX_SetHighPinsState(gpio->Ftd2xx_ctx, data, mask);
    }
#endif

#if defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_NOT_AVAILABLE; /* Return error GPIO not available under FTD2XX UART */
    PTX_UNUSED_PARAMETER(ctx);
    PTX_UNUSED_PARAMETER(port);
    PTX_UNUSED_PARAMETER(data);
    PTX_UNUSED_PARAMETER(mask);
#endif
    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_SetCfgPin (void *ctx, ptx_HAL_DRV_GPIO_Cfg_t cfg[], uint32_t cfgLen)
{
#if !defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    PTX_UNUSED_PARAMETER(cfg);
    PTX_UNUSED_PARAMETER(cfgLen);

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;

    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_SetCfgPin");

        /* Not used. Just return status. */
        status = PTX_STATUS_E_GEN_NOT_AVAILABLE;
    }
#endif

#if defined(PTX_ENABLE_UART)
    ptx_Status_t status = PTX_STATUS_E_GEN_NOT_AVAILABLE; /* Return error GPIO not available under FTD2XX UART */
    PTX_UNUSED_PARAMETER(ctx);
    PTX_UNUSED_PARAMETER(cfg);
    PTX_UNUSED_PARAMETER(cfgLen);
#endif
    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
#if !defined(PTX_ENABLE_UART)
static ptx_Status_t ptx_HAL_MapGPIOPin(uint16_t pinNr, ptx_HAL_FTD2XX_GPIO_t *gpio_pin)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /** Map pin number to pin byte value */
    if (pinNr < FTDI_MAX_GPIO_PINS)
    {
        status = PTX_STATUS_SUCCESS;

        if (0 == pinNr)
        {
            *gpio_pin = GPIOH0;
        }
        else if (1 == pinNr)
        {
            *gpio_pin = GPIOH1;
        }
        else if (2 == pinNr)
        {
            *gpio_pin = GPIOH2;
        }
        else if (3 == pinNr)
        {
            *gpio_pin = GPIOH3;
        }
        else if (4 == pinNr)
        {
            *gpio_pin = GPIOH4;
        }
        else if (5 == pinNr)
        {
            *gpio_pin = GPIOH5;
        }
        else if (6 == pinNr)
        {
            *gpio_pin = GPIOH6;
        }
        else if (7 == pinNr)
        {
            *gpio_pin = GPIOH7;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

#endif