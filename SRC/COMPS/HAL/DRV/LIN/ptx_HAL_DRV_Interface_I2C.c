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
    Module      : Linux I2C Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_I2C.c

    Description : Implementation of the I2C Communication Interface Driver for the Linux Reference Platform
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <string.h>
#include "ptx_HAL_DRV_Interface_I2C.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <linux/gpio.h>
#include <poll.h>
#include <sys/eventfd.h>


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_HAL_DRV_INTERFACE_I2C_NUMBER_PORTS                             (1u)

#define PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_BUFFER_MAX                      64u

/*
 * Default I2C device to be used: i2cdev0.0.
 * Default speed: 1Mbps
 */
#define PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_I2C_DEV(buffer, no)             snprintf(buffer, PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_BUFFER_MAX, "/dev/i2c-%d", no)

/*
 * GPIO device used on the referent platform. Beside this, the pin number is required to specify a GPIO pin to be used.
 * Also, please note that it might be required also to set up the GPIO pin in the Device Tree.
 * Default setting: gpiochip0, pin 19.
 */
#define	PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_GPIOCHIP_USED                   "/dev/gpiochip0"
#define	PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_IRQ_LINE_NR                     19U
#define PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_SET_GPIOCHIP(buffer, gpiochip)  snprintf(buffer, PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_BUFFER_MAX, gpiochip)

static ptx_HAL_DRV_Interface_I2C_PortConfig_t gDrvI2CConfigs[PTX_HAL_DRV_INTERFACE_I2C_NUMBER_PORTS] =
{
    {
        .Type           = HAL_DRV_Interface_Port_Type_Default,
        .ID             = 0,

        .Dev            = -1,
        .PortNr         = 1,

        .IrqFd          = -1,
        .IrqFdCancel    = -1,

        .IrqPin         = PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_IRQ_LINE_NR,

        .Msgs           = {{0}, {0}},
    }
};

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_HAL_DRV_Interface_I2C_GpioSetInput (ptx_HAL_DRV_Interface_I2C_t *ctx, int *retFd);
static ptx_Status_t ptx_HAL_DRV_Interface_I2C_GpioSetIrqRising (ptx_HAL_DRV_Interface_I2C_t *ctx, int *retFd);
static ptx_Status_t ptx_HAL_DRV_Interface_I2C_IRQ_Clear_Cancel(ptx_HAL_DRV_Interface_I2C_t *ctx);

static ptx_Status_t ptx_HAL_DRV_Interface_I2C_IRQ_Config (ptx_HAL_DRV_Interface_I2C_t *ctx);
static ptx_Status_t ptx_HAL_DRV_Interface_I2C_GpioCheckPendingIRQ(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *irqPending);

static ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetSlaveAddress(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t address);
static ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetTransmissionSpeed(ptx_HAL_DRV_Interface_I2C_t *ctx, uint32_t speed);
static ptx_Status_t ptx_HAL_DRV_Interface_I2C_ValidateAddr(uint8_t busAddr);
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_HAL_DRV_Interface_I2C_Init (ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_I2C_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams))
    {
        (void)memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_I2C_t));
        for (uint8_t config = 0; (PTX_HAL_DRV_INTERFACE_I2C_NUMBER_PORTS > config) && (PTX_STATUS_SUCCESS != status); ++config)
        {
            if ((gDrvI2CConfigs[config].Type == initParams->PortType) && (gDrvI2CConfigs[config].ID == initParams->PortID))
            {
                ctx->Port = &gDrvI2CConfigs[config];
                ctx->Speed = (0 == initParams->Speed) ? PTX_HAL_DRV_INTERFACE_I2C_SPEED_DEFAULT : initParams->Speed;
                ctx->BusAddr = (0 == initParams->Address) ? PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4C : initParams->Address;
                ctx->Log = initParams->Log;
                ctx->OS = (ptx_OSAL_t*)initParams->OS;
                ctx->CompId = Comp_HAL;

                status = ptx_OSAL_MutexInit(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);

                if (PTX_STATUS_SUCCESS == status)
                {
                    ctx->CompId = Comp_HAL;
                }
                else
                {
                    (void)memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_I2C_t));
                    ctx->CompId = Comp_None;
                }

            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_DeInit (ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = ptx_OSAL_MutexDeInit(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);
        if (PTX_STATUS_SUCCESS == status)
        {
            memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_I2C_t));
            ctx->CompId = Comp_None;
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_Open(ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        char dev_name[PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_BUFFER_MAX];
        memset(dev_name, 0, sizeof(char)*PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_BUFFER_MAX);

        PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_I2C_DEV(dev_name, ctx->Port->PortNr);

        ctx->Port->Dev = open(dev_name, O_RDWR);

        if ((0 <= ctx->Port->Dev) && (PTX_STATUS_SUCCESS == ptx_HAL_DRV_Interface_I2C_ValidateAddr(ctx->BusAddr)))
        {
            /* open file successfull now configure and open gpio*/
            status = ptx_HAL_DRV_Interface_I2C_IRQ_Config(ctx);
        }
        else
        {
            /* error opening file */
            (void) close(ctx->Port->Dev);
            status = PTX_STATUS_E_GEN_INTERNAL;
        }


        /* If init failed ensure that Dev contains invalid fd ID */
        if (PTX_STATUS_SUCCESS != status)
        {
            ctx->Port->Dev = -1;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I (ctx->Log, "I2C Open OK");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "I2C Open FAILED");
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_Close(ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        (void)close(ctx->Port->Dev);
        (void)close(ctx->Port->IrqFd);
        (void)close(ctx->Port->IrqFdCancel);
        status = PTX_STATUS_SUCCESS;

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I (ctx->Log, "I2C Close OK");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "I2C Close FAILED");
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_Tx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *txBuf, uint32_t txLen, uint8_t restart)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        uint8_t mutex_locked = 0;

        status = ptx_OSAL_MutexLock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);
        if (PTX_STATUS_SUCCESS == status)
        {
            mutex_locked = 1;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 == restart)
            {
                struct i2c_rdwr_ioctl_data msgset[1];

                ctx->Port->Msgs[0].addr = ctx->BusAddr;
                ctx->Port->Msgs[0].flags = 0;
                ctx->Port->Msgs[0].len = (unsigned short) txLen;
                ctx->Port->Msgs[0].buf = txBuf;
                ctx->Port->Msgs[1].addr = 0x00;

                msgset[0].msgs = ctx->Port->Msgs;
                msgset[0].nmsgs = 1;

                int error_code = ioctl(ctx->Port->Dev, I2C_RDWR, &msgset);
                if ((error_code > 0) && (msgset[0].nmsgs == 1))
                {
                    /* all msgs were sent */
                    if (ctx->Port->Msgs[0].len == txLen)
                    {
                        /* all bytes were sent */
                        status = PTX_STATUS_SUCCESS;
                    }
                    else
                    {
                        /* failed to transmit all bytes */
                        status = PTX_STATUS_E_GEN_INTERNAL;
                    }
                }
                else
                {
                    /* msg */
                    status = PTX_STATUS_E_GEN_INTERNAL;
                }
            }
            else
            {
                /* To avoid sending stop condition (required for Rx) Tx & Rx message must be given to kernel at once.
                Store Tx message, which is then sent with Rx message. */
                ctx->Port->Msgs[0].addr = ctx->BusAddr;
                ctx->Port->Msgs[0].flags = 0;
                ctx->Port->Msgs[0].len = (unsigned short) txLen;
                ctx->Port->Msgs[0].buf = txBuf;
                ctx->Port->Msgs[1].addr = ctx->BusAddr;

                status = PTX_STATUS_SUCCESS;
            }
        }

        if (1 == mutex_locked)
        {
            ptx_Status_t st = ptx_OSAL_MutexUnlock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);

            /* Keep the error status if previously set. */
            status = (PTX_STATUS_SUCCESS == status) ? st : status;
        }

#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I_MEM (ctx->Log, txBuf, txLen, "I2C Write: ");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "I2C Write ERROR Status: %u", status);
        }
#endif
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_Rx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *rxBuf, uint32_t rxLen, uint8_t restart)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    PTX_UNUSED_PARAMETER(restart);

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        if (0 == rxLen)
        {
            /*
             * Although we fell through until here, the result is fine.
             * We were supposed to read nothing and this is just what we have done.
             */
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            if (NULL != rxBuf)
            {
                int32_t error_code = -1;

                uint8_t mutex_locked = 0;

                uint32_t txLen = 0;
                struct i2c_rdwr_ioctl_data msgset[1];

                status = ptx_OSAL_MutexLock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);
                if (PTX_STATUS_SUCCESS == status)
                {
                    mutex_locked = 1;
                }

                if (PTX_STATUS_SUCCESS == status)
                {

                    ctx->Port->Msgs[1].addr = ctx->BusAddr;
                    ctx->Port->Msgs[1].flags = I2C_M_RD | I2C_M_NOSTART;
                    ctx->Port->Msgs[1].len = (unsigned short) rxLen;
                    ctx->Port->Msgs[1].buf = rxBuf;

                    msgset[0].msgs = ctx->Port->Msgs;
                    msgset[0].nmsgs = 2;

                    txLen = ctx->Port->Msgs[0].len;

                    error_code = ioctl(ctx->Port->Dev, I2C_RDWR, &msgset);
                }

                if ((error_code > 0) && (msgset[0].nmsgs == 2))
                {
                    /* all msgs were sent */
                    if ((ctx->Port->Msgs[0].len == txLen) && (ctx->Port->Msgs[1].len == rxLen))
                    {
                        /* all bytes were sent & received */
                        status = PTX_STATUS_SUCCESS;
                    }
                    else
                    {
                        status = PTX_STATUS_E_GEN_INTERNAL;
                    }
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INTERNAL;
                }

                if (1 == mutex_locked)
                {
                    ptx_Status_t st = ptx_OSAL_MutexUnlock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);
                    if (PTX_STATUS_SUCCESS != st)
                    {
                        PTX_LOG_I(ctx->Log, "Error ptx_OSAL_MutexUnlock ST = %d", st);
                    }

                    /* Keep the error status if previously set. */
                    status = (PTX_STATUS_SUCCESS == status) ? st : status;
                }
            }
            else
            {
                /* no Rx buffer was provided */
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }

#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I_MEM (ctx->Log, rxBuf, rxLen, "I2C Read: ");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "I2C Read ERROR Status: %u", status);
        }
#endif
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_ActivateRxIrq(ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_DeactivateRxIrq(ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;


    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_RxIrqLevel(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    PTX_UNUSED_PARAMETER(level);

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        uint8_t gpio_pending = 0;

        /* Check if the IRQ of the GPIO-pin is already pending i.e. set to 1 */
        status = ptx_HAL_DRV_Interface_I2C_GpioCheckPendingIRQ(ctx, &gpio_pending);
        if (PTX_STATUS_SUCCESS == status)
        {
            *level = gpio_pending;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_WaitForIRQ(ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        /*
         * The routine first reads the line status: if the line is set, exit the routine.
         * Otherwise, continue to the loop and poll continuously until the expected events come, or an internal error
         * occurrs. Possible events to expect: line rising edge or gpio cancel.
         * The function returns only if any of those 2 events happens.
         *
         * NOTE: there might be some exceptional events and the referring flags set, like POLLPRI or POLLERR.
         * Such events are disregarded. Nevertheless a fd needs to be read to clear the event.
         */

        int rv;
        int64_t rd;
        struct pollfd pfd[2];
        struct gpioevent_data event;
        uint8_t gpio_pending = 0;
        uint8_t exit_loop = 0;

        /* Check if the IRQ of the GPIO-pin is already pending i.e. set to 1 */
        st = ptx_HAL_DRV_Interface_I2C_GpioCheckPendingIRQ(ctx, &gpio_pending);

        if ((PTX_STATUS_SUCCESS == st) && (0 == gpio_pending))
        {
            while ((0 == exit_loop) && (PTX_STATUS_SUCCESS == st))
            {
                memset(&pfd[0], 0, 2 * sizeof(struct pollfd));

                /* Add file descriptors to the set */
                pfd[0].fd = ctx->Port->IrqFdCancel;
                pfd[0].events = POLLIN | POLLPRI;

                pfd[1].fd = ctx->Port->IrqFd;
                pfd[1].events = POLLIN | POLLPRI;

                gpio_pending = 0;

                /* Wait to infinity and beyond... */
                rv = poll(&pfd[0], 2, -1);
                if (rv > 0)
                {
                    /* There is one or more events available. GpioCancel is higher priority. */

                    if (0 != pfd[0].revents)
                    {
                        /* Cancel Event happened. Read to clear. */
                        (void)ptx_HAL_DRV_Interface_I2C_IRQ_Clear_Cancel(ctx);

                        /* Check also the IrqLine event just to clear it. */
                        if (0 != pfd[1].revents)
                        {
                            rd = read(ctx->Port->IrqFd, &event, sizeof(event));
                        }

                        st = PTX_STATUS_E_HAL_CANCELLED;
                    }
                    else if (0 != pfd[1].revents)
                    {
                        /* Irq line Event happened. Read to get the event data, or just to clear. */

                        rd = read(ctx->Port->IrqFd, &event, sizeof(event));
                        if (0 >= rd)
                        {
                            st = PTX_STATUS_E_GEN_INTERNAL;
                        }
                        else
                        {
                            if ( (0 != (pfd[1].revents & POLLIN)) && (GPIOEVENT_EVENT_RISING_EDGE == event.id))
                            {
                                /* IRQ line trigger. */
                                gpio_pending = 1;
                            }
                        }
                    }
                    else
                    {
                        /* Unexpected error. */
                        st = PTX_STATUS_E_GEN_INTERNAL;
                    }

                    if ((PTX_STATUS_SUCCESS == st) && (0 != gpio_pending))
                    {
                        gpio_pending = 0;

                        /* The line might be set. Please note that there might be some false alarm. So, check the line once more. */
                        st = ptx_HAL_DRV_Interface_I2C_GpioCheckPendingIRQ(ctx, &gpio_pending);
                        if ((PTX_STATUS_SUCCESS == st) && (0 != gpio_pending))
                        {
                            /* IRQ line is set. */
                            exit_loop = 1U;
                        }
                    }

                }
                else
                {
                    /* An error. Cannot be a timeout since we wait for an infinite time. */
                    st = PTX_STATUS_E_GEN_INTERNAL;
                }
            }
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetParameter(ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && NULL != parameter)
    {
        switch (parameter->type)
        {
            case HAL_DRV_Interface_Parameter_Type_Transmission_Speed:
                status = ptx_HAL_DRV_Interface_I2C_SetTransmissionSpeed(ctx, parameter->value.TransmissionSpeed);
                break;
            case HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address:
                status = ptx_HAL_DRV_Interface_I2C_SetSlaveAddress(ctx, parameter->value.I2CSlaveAddress);
                break;
            case HAL_DRV_Interface_Parameter_Type_UART_Enable_Fast_Mode:
                status = PTX_STATUS_E_GEN_NOT_AVAILABLE;
                break;
            default:
                break;
        }
    }

    return status;

}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_GetParameter(ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && NULL != parameter)
    {
        switch (parameter->type)
        {
            case HAL_DRV_Interface_Parameter_Type_Transmission_Speed:
                parameter->value.TransmissionSpeed = ctx->Speed;
                status = PTX_STATUS_SUCCESS;
                break;
            case HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address:
                parameter->value.I2CSlaveAddress = ctx->BusAddr;
                status = PTX_STATUS_SUCCESS;
                break;
            default:
                break;
        }
    }

    return status;

}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetSlaveAddress(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t address)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        ctx->BusAddr = address;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetTransmissionSpeed(ptx_HAL_DRV_Interface_I2C_t *ctx, uint32_t speed)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (0 != speed))
    {
        /*
            Note: There is no way to directly change the clock speed for I2C under Linux through Kernel calls during runtime.
                  To change it update the "config.txt" file in your firmware folder and reboot.
         */
        status = PTX_STATUS_E_GEN_NOT_AVAILABLE;
    }
    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_HAL_DRV_Interface_I2C_ValidateAddr(uint8_t busAddr)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t address = busAddr;

    switch (address)
    {
        case PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4C:
        /* Fall through */
        case PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4D:
        /* Fall through */
        case PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4E:
        /* Fall through */
        case PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4F:
            status = PTX_STATUS_SUCCESS;
            break;
        default:
            /* Nothing, error already set */
            break;
    }

    return status;
}


static ptx_Status_t ptx_HAL_DRV_Interface_I2C_IRQ_Config (ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERFACE_OPEN;

    int fd;

    char buffer[PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_BUFFER_MAX];
    memset(buffer, 0, PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_BUFFER_MAX);
    PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_SET_GPIOCHIP(buffer, PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_GPIOCHIP_USED);

    /*
     * Open a chip fd here. This fd will be used to retrieve line fd and  then closed. A different
     * fd is then used for furter gpio access.
     */
    fd = open(buffer, O_RDWR | O_NONBLOCK);
    if (fd > 0)
    {
        ctx->Port->IrqFd = fd;
        st = ptx_HAL_DRV_Interface_I2C_GpioSetInput(ctx, &fd);
        if (PTX_STATUS_SUCCESS == st)
        {
            /*
             * Opened fd was closed in GpioSetInput and new fd was assigned by the GPIO interface API.
             * This fd is used from now on - for all further ioctl requests.
             */
            ctx->Port->IrqFd = fd;
        }
    }

    if (PTX_STATUS_SUCCESS == st)
    {
        st = ptx_HAL_DRV_Interface_I2C_GpioSetIrqRising(ctx, &fd);
        if (PTX_STATUS_SUCCESS == st)
        {
            /*
             * Fd was modified/closed inside GpioSetIrqRising.
             * This final fd is used to get line state and to poll for line events.
             */
            ctx->Port->IrqFd = fd;
        }
    }

    if (PTX_STATUS_SUCCESS == st)
    {
        /* Fd used to cancel WaitForIrq operation. */
        ctx->Port->IrqFdCancel = eventfd(0, O_NONBLOCK);
        if (ctx->Port->IrqFdCancel <= 0)
        {
            /* In case Cancel Fd could not be properly set, close already open Fd */
            (void)close(ctx->Port->IrqFd);

            st = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return st;
}

static ptx_Status_t ptx_HAL_DRV_Interface_I2C_GpioSetInput (ptx_HAL_DRV_Interface_I2C_t *ctx, int *retFd)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * This actually gets a line handle and changes the FD!!!!!
     * Call this only from ptx_HAL_DRV_Interface_I2C_IRQ_Config!!!
     * Or, even better, modify the code and rename this function...
     */

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (ctx->Port->IrqFd > 0) && (NULL != retFd))
    {
        int fd;
        int rv;
        struct gpiohandle_request req;

        st = PTX_STATUS_SUCCESS;

        fd = ctx->Port->IrqFd;
        req.flags = GPIOHANDLE_REQUEST_INPUT;
        req.lines = 1;
        req.lineoffsets[0] = ctx->Port->IrqPin;
        strcpy(req.consumer_label, "IrqLine_CfgMgr");

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

static ptx_Status_t ptx_HAL_DRV_Interface_I2C_GpioSetIrqRising (ptx_HAL_DRV_Interface_I2C_t *ctx, int *retFd)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * This is called only from GpioOpen!!!
     * File descriptor used by ioctl must be the one of the chip!!!
     * So, not the one currently used by ctx.
     * As a result, this API will also modify the FD!!!
     */

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != retFd))
    {
        int fd, rv;
        struct gpioevent_request req;

        st = PTX_STATUS_SUCCESS;

        /* Close any previously opened file descriptor. */
        if (ctx->Port->IrqFd > 0)
        {
            (void)close(ctx->Port->IrqFd);
            ctx->Port->IrqFd = -1;
        }

        fd = open(PTX_HAL_DRV_INTERFACE_I2C_WCHARDEV_GPIOCHIP_USED, O_RDWR | O_NONBLOCK);
        if (0 < fd)
        {
            req.lineoffset = ctx->Port->IrqPin;
            req.handleflags = GPIOHANDLE_REQUEST_INPUT;
            req.eventflags = GPIOEVENT_REQUEST_RISING_EDGE;
            strcpy(req.consumer_label, "IrqLine_EvtMgr");
            rv = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &req);
            if (rv == -1)
            {
                st = PTX_STATUS_E_GEN_INTERNAL;
            }
            else
            {
                *retFd = req.fd;
            }

            (void)close(fd);
        }
        else
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return st;

}

static ptx_Status_t ptx_HAL_DRV_Interface_I2C_GpioCheckPendingIRQ (ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *irqPending)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (ctx->Port->IrqFd > 0) && (NULL != irqPending))
    {
        int fd;
        int res;
        struct gpiohandle_data data;

        fd = ctx->Port->IrqFd;

        memset(&data, 0, sizeof(data));

        /* Read line state */
        res = ioctl(fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
        if (-1 == res)
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            *irqPending = (data.values[0] != 0) ? 1u : 0u;
            st = PTX_STATUS_SUCCESS;
        }
    }

    return st;
}

static ptx_Status_t ptx_HAL_DRV_Interface_I2C_IRQ_Clear_Cancel (ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        uint64_t rd_num;
        ssize_t br;
        int i = 0;
        size_t bytes_read = 0;
        size_t bytes_to_read = sizeof(rd_num);

        status = PTX_STATUS_SUCCESS;

        do
        {
            br = read(ctx->Port->IrqFdCancel, &rd_num, bytes_to_read - bytes_read); /* Non-blocking */
            if (br >= 0)
            {
                bytes_read += (size_t)br;
            }
            else
            {
                int local_error = errno;

                /*
                 * workaround for over ambitious warning
                 */
                if (EAGAIN == local_error)
                {
                    /* Not yet all.. */
                }
                else if ((EAGAIN != EWOULDBLOCK) && (EWOULDBLOCK == local_error))
                {
                    /*
                     * Workaround for a compile warning/error: error: logical ‘or’ of equal expressions [-Werror=logical-op]
                     *
                     * This happens because EWOULDBLOCK and EAGAIN might be the same value.
                     *
                     * EWOULDBLOCK
                     * Operation would block (may be same value as EAGAIN)
                     * (POSIX.1-2001).
                     */
                }
                else
                {
                    break;
                }
            }
            i++;
        }
        while ((bytes_read < bytes_to_read) && (i < 3));
    }

    return status;
}

