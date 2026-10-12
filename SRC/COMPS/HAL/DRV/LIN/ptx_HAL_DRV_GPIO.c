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
    Module      : Linux GPIO Driver
    File        : ptx_HAL_DRV_GPIO.c

    Description : Implementation of the GPIO Driver for the Linux Reference Platform
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_HAL_DRV_GPIO.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_HAL_DRV_GPIO_CHIP_BUFFER_MAX             64U
#define PTX_HAL_DRV_GPIO_PIN_LEVEL_HIGH              1U
#define PTX_HAL_DRV_GPIO_PIN_LEVEL_LOW               0

/*
 * GPIO device and pin on the referent platform, used to reset PTX chip.
 * Please note that it might be required also to set up the GPIO pin in the Device Tree.
 * Default setting: gpiochip0, pin 21.
 */
#define PTX_HAL_RESETPIN_GPIOCHIP_USED                  "/dev/gpiochip0"
#define PTX_HAL_RESETPIN_LINE_NR                        21U
#define PTX_HAL_RESETPIN_SET_GPIOCHIP(buffer, gpiochip) snprintf(buffer, PTX_HAL_DRV_GPIO_CHIP_BUFFER_MAX, gpiochip)


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_HAL_DRV_GPIO_GpioSetOutput (ptx_HAL_DRV_GPIO_t *ctx, int *retFd);

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


        memset(ctx, 0, sizeof(ptx_HAL_DRV_GPIO_t));

        ctx->CompId = Comp_HAL;
        ctx->ResetPin = PTX_HAL_RESETPIN_LINE_NR;
        ctx->ResetFd = -1;
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

        status = ptx_HAL_DRV_GPIO_Close(ctx);
        if (PTX_STATUS_SUCCESS == status)
        {
            memset(ctx, 0, sizeof(ptx_HAL_DRV_GPIO_t));
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_Open (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*) ctx;
    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_Open");

        int fd;

        char buffer[PTX_HAL_DRV_GPIO_CHIP_BUFFER_MAX];
        memset(buffer, 0, PTX_HAL_DRV_GPIO_CHIP_BUFFER_MAX);
        PTX_HAL_RESETPIN_SET_GPIOCHIP(buffer, PTX_HAL_RESETPIN_GPIOCHIP_USED);

        /*
         * Open a chip fd here. This fd will be used to retrieve line fd and  then closed. A different
         * fd is then used for further gpio access.
         */
        fd = open(buffer, O_RDWR | O_NONBLOCK);
        if (fd > 0)
        {
            gpio->ResetFd = fd;
            status = ptx_HAL_DRV_GPIO_GpioSetOutput(gpio, &fd);
            if (PTX_STATUS_SUCCESS == status)
            {
                /*
                 * Opened fd was closed in GpioSetOutput and new fd was assigned by the GPIO interface API.
                 * This fd is used from now on - for all further ioctl requests.
                 */
                gpio->ResetFd = fd;
            }
            else
            {
                close(fd);
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Set ResetPin high i.e. idle state */
            status = ptx_HAL_DRV_GPIO_WritePin (gpio, 0, PTX_HAL_DRV_GPIO_PIN_LEVEL_HIGH);
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_Close (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;

    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_Close");

        (void)close(gpio->ResetFd);
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_ReadPin (void *ctx, uint16_t pin, uint8_t *level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Currently GPIO implements support only for the RESET pin, which is already predefined. */
    PTX_UNUSED_PARAMETER(pin);

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*) ctx;
    if (PTX_COMP_CHECK(gpio, Comp_HAL) && (NULL != level))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_ReadPin");


        struct gpiohandle_data data;

        int ret = ioctl(gpio->ResetFd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
        if (ret == -1)
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            *level = (uint8_t)data.values[0];
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_WritePin (void *ctx, uint16_t pin, uint8_t level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Currently GPIO implements support only for the RESET pin, which is already predefined. */
    PTX_UNUSED_PARAMETER(pin);

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*) ctx;
    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_WritePin");

        struct gpiohandle_data data;
        data.values[0] = (level > 0) ? PTX_HAL_DRV_GPIO_PIN_LEVEL_HIGH : PTX_HAL_DRV_GPIO_PIN_LEVEL_LOW;

        int ret = ioctl(gpio->ResetFd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
        if (ret == -1)
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_ReadPort (void *ctx, uint32_t port, uint16_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    PTX_UNUSED_PARAMETER(port);

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*) ctx;
    if (PTX_COMP_CHECK(gpio, Comp_HAL) && (NULL != data))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_ReadPort");

        /* Not used. Just return status. */
        *data = 0;
        status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_WritePort (void *ctx, uint32_t port, uint16_t data, uint16_t mask)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    PTX_UNUSED_PARAMETER(port);
    PTX_UNUSED_PARAMETER(data);
    PTX_UNUSED_PARAMETER(mask);

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;
    if (PTX_COMP_CHECK(gpio, Comp_HAL))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_WritePort");
        /* Not used. Just return status. */
        status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_GPIO_SetCfgPin (void *ctx, ptx_HAL_DRV_GPIO_Cfg_t cfg[], uint32_t cfgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_DRV_GPIO_t *gpio = (ptx_HAL_DRV_GPIO_t*)ctx;
    if (PTX_COMP_CHECK(gpio, Comp_HAL) && (NULL != cfg) && (0 != cfgLen))
    {
        PTX_LOG_V(gpio->Log, "ptx_HAL_DRV_GPIO_SetCfgPin");

        /* Not used. Just return status. */
        status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
static ptx_Status_t ptx_HAL_DRV_GPIO_GpioSetOutput (ptx_HAL_DRV_GPIO_t *ctx, int *retFd)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * This actually gets a line handle and changes the FD!!!!!
     * Call this only from ptx_HAL_DRV_Interface_SPI_IRQ_Config!!!
     * Or, even better, modify the code and rename this function...
     */

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (ctx->ResetFd > 0) && (NULL != retFd))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_GPIO_GpioSetOutput");

        int fd;
        int rv;
        struct gpiohandle_request req;

        st = PTX_STATUS_SUCCESS;

        fd = ctx->ResetFd;
        req.flags = GPIOHANDLE_REQUEST_OUTPUT;
        req.lines = 1;
        req.lineoffsets[0] = ctx->ResetPin;
        strcpy(req.consumer_label, "ResetLine_CfgMgr");

        *retFd = -1;
        rv = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
        if (rv == -1)
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            *retFd = req.fd;
        }

        rv = close(fd);
        if (rv == -1)
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return st;
}


