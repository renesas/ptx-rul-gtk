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
    Module      : Linux SPI Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_SPI.c

    Description : Implementation of the SPI Communication Interface Driver for the Linux Reference Platform
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/eventfd.h>
#include <linux/spi/spidev.h>
#include <errno.h>
#include <unistd.h>
#include <linux/gpio.h>
#include <poll.h>

#include "ptx_HAL_DRV_Interface_SPI.h"


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_HAL_DRV_INTERFACE_SPI_NUMBER_PORTS                             (1u)

#define PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_BUFFER_MAX                      64u

/*
 * Default SPI device to be used: spiInterface0.0.
 * Default speed: 1Mbps
 */
#define PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_SPI_DEV(buffer, no)             snprintf(buffer, PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_BUFFER_MAX, "/dev/spidev0.%d", no)
#define PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_SPI_MODE_0                      (SPI_MODE_0)
#define PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_BITS_PER_WORD                   (8u)
#define PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_IOC_WR_MODE32                   (SPI_IOC_WR_MODE32)
#define PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_IOC_WR_BITS_PER_WORD            (SPI_IOC_WR_BITS_PER_WORD)
#define PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_IOC_WR_MAX_SPEED_HZ             (SPI_IOC_WR_MAX_SPEED_HZ)


/*
 * GPIO device used on the referent platform. Beside this, the pin number is required to specify a GPIO pin to be used.
 * Also, please note that it might be required also to set up the GPIO pin in the Device Tree.
 * Default setting: gpiochip0, pin 19.
 */
#define	PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_GPIOCHIP_USED                   "/dev/gpiochip0"
#define	PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_IRQ_LINE_NR                     19U
#define PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_SET_GPIOCHIP(buffer, gpiochip)  snprintf(buffer, PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_BUFFER_MAX, gpiochip)


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_HAL_DRV_Interface_SPI_PortConfig_t gDrvSpiConfigs[PTX_HAL_DRV_INTERFACE_SPI_NUMBER_PORTS] =
{
    {
        .Type           = HAL_DRV_Interface_Port_Type_Default,
        .ID             = 0,

        .PortNr         = 0,
        .PortFd         = -1,

        .IrqFd          = -1,
        .IrqFdCancel    = -1,

        .IrqPin         = PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_IRQ_LINE_NR,

        .TxData         = NULL,
        .TxDataLen      = 0,

        .RxData         = NULL,
        .RxDataLen      = 0,
    }
};
static ptx_Status_t ptx_HAL_DRV_Interface_SPI_GpioSetInput (ptx_HAL_DRV_Interface_SPI_t *ctx, int *retFd);
static ptx_Status_t ptx_HAL_DRV_Interface_SPI_GpioSetIrqRising (ptx_HAL_DRV_Interface_SPI_t *ctx, int *retFd);
static ptx_Status_t ptx_HAL_DRV_Interface_SPI_IRQ_Clear_Cancel(ptx_HAL_DRV_Interface_SPI_t *ctx);

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_IRQ_Config (ptx_HAL_DRV_Interface_SPI_t *ctx);
static ptx_Status_t ptx_HAL_DRV_Interface_SPI_GpioCheckPendingIRQ(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *irqPending);

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetTransmissionSpeed(ptx_HAL_DRV_Interface_SPI_t *ctx, uint32_t speed);
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Init (ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_SPI_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams))
    {
        memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_SPI_t));

        for (uint8_t config = 0; (PTX_HAL_DRV_INTERFACE_SPI_NUMBER_PORTS > config) && (PTX_STATUS_SUCCESS != status); ++config)
        {
            if ((gDrvSpiConfigs[config].Type == initParams->PortType) && (gDrvSpiConfigs[config].ID == initParams->PortID))
            {
                ctx->Port = &gDrvSpiConfigs[config];
                ctx->Speed = (0 == initParams->Speed) ? PTX_HAL_DRV_INTERFACE_SPI_DEFAULT_SPEED : initParams->Speed;
                ctx->Log = initParams->Log;
                ctx->OS = (ptx_OSAL_t*)initParams->OS;

                status = PTX_STATUS_SUCCESS;
                ctx->CompId = Comp_HAL;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_DeInit (ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        /* First close any opened resources and then clean them up */
        status = ptx_HAL_DRV_Interface_SPI_Close(ctx);
        if (PTX_STATUS_SUCCESS == status)
        {
            memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_SPI_t));
            ctx->CompId = Comp_None;
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Open(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        char buffer[PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_BUFFER_MAX];
        memset(buffer, 0, sizeof(char)*PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_BUFFER_MAX);

        PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_SPI_DEV(buffer, ctx->Port->PortNr);
        ctx->Port->PortFd = open(buffer, (int)(((unsigned int) O_RDWR)
                                               | ((unsigned int) O_NOCTTY)
                                               | ((unsigned int) O_NDELAY)
                                               | ((unsigned int) O_NONBLOCK)
                                              ));
        if (ctx->Port->PortFd >= 0)
        {
            const uint32_t mode = PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_SPI_MODE_0;
            int ret = ioctl(ctx->Port->PortFd, PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_IOC_WR_MODE32, &mode);

            if (-1 != ret)
            {
                const uint32_t bits_per_word = PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_BITS_PER_WORD;
                ret = ioctl(ctx->Port->PortFd, PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_IOC_WR_BITS_PER_WORD, &bits_per_word);
            }

            if (-1 != ret)
            {
                const uint32_t speed_default = PTX_HAL_DRV_INTERFACE_SPI_DEFAULT_SPEED;
                uint32_t speed = (ctx->Speed < speed_default) ? speed_default : ctx->Speed;
                ret = ioctl(ctx->Port->PortFd, PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_IOC_WR_MAX_SPEED_HZ, &speed);
            }

            if (-1 != ret)
            {
                /* spiInterface opened and configured. Now open and configure IRQ GPIO. */
                status = ptx_HAL_DRV_Interface_SPI_IRQ_Config(ctx);
            }
            else
            {
                status = PTX_STATUS_E_GEN_INTERFACE_OPEN;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_OPEN;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I (ctx->Log, "SPI Open OK");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "SPI Open FAILED");
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Close(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        (void)close(ctx->Port->IrqFd);
        (void)close(ctx->Port->IrqFdCancel);
        (void)close(ctx->Port->PortFd);
        status = PTX_STATUS_SUCCESS;

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I (ctx->Log, "SPI Close OK");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "SPI Close FAILED");
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Tx(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *txBuf, uint32_t txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (ctx->Port->PortFd > 0))
    {
        if (NULL == ctx->Port->TxData)
        {
            if (txLen > 0)
            {
                ctx->Port->TxData = txBuf;
                ctx->Port->TxDataLen = txLen;
            }

            status = PTX_STATUS_SUCCESS;
        }

#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I_MEM (ctx->Log, txBuf, txLen, "SPI Write: ");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "SPI Write ERROR Status: %u", status);
        }
#endif
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Rx(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *rxBuf, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        if (rxLen > 0)
        {
            ctx->Port->RxData = rxBuf;
            ctx->Port->RxDataLen = rxLen;
        }

        status = PTX_STATUS_SUCCESS;

#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I_MEM (ctx->Log, rxBuf, rxLen, "SPI Read: ");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "SPI Read ERROR Status: %u", status);
        }
#endif
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetNss(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_ResetNss(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        if (NULL != ctx->Port->TxData)
        {
            struct spi_ioc_transfer data[2] =
            {
                {
                    .tx_buf = (uintptr_t)ctx->Port->TxData,
                    .len = (uint32_t)ctx->Port->TxDataLen,
                },
                {
                    .rx_buf = (uintptr_t)ctx->Port->RxData,
                    .len = (uint32_t)ctx->Port->RxDataLen,
                }
            };

            int ret = ioctl(ctx->Port->PortFd, SPI_IOC_MESSAGE(2), &data);
            if (1 < ret)
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INTERFACE_NSS;
            }


            if (NULL != ctx->Port->TxData)
            {
                ctx->Port->TxData = NULL;
                ctx->Port->TxDataLen = 0;
            }

            if (NULL != ctx->Port->RxData)
            {
                ctx->Port->RxData = NULL;
                ctx->Port->RxDataLen = 0;
            }
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }
    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_ActivateRxIrq(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_DeactivateRxIrq(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        int i = 0;
        uint64_t nr = 1;
        uint64_t bytes_written = 0;
        uint64_t bytes_to_write = sizeof(nr);
        int64_t bw;
        do
        {
            bw = write(ctx->Port->IrqFdCancel, &nr, bytes_to_write - bytes_written); /* Signal <nr> times by writing 8 bytes, representing <nr>  into the efd. */

            if (bw >= 0)
            {
                bytes_written += (uint32_t)bw;
            }
            else
            {
                /* No need to handle the error here. */
            }
            i++;
        }
        while ((bytes_written < bytes_to_write) && (i < 3));

        if (0 != bytes_written)
        {
            /* Something has been written */
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_RxIrqLevel(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((PTX_COMP_CHECK(ctx, Comp_HAL)) && (NULL != level))
    {
        uint8_t gpio_state = 0;

        /* Check if the IRQ of the GPIO-pin is already pending i.e. set to 1 */
        status = ptx_HAL_DRV_Interface_SPI_GpioCheckPendingIRQ(ctx, &gpio_state);
        if (PTX_STATUS_SUCCESS == status)
        {
            *level = gpio_state;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_WaitForIRQ(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        /*
         * The routine first reads the line status: if the line is set, exit the routine.
         * Otherwise, continue to the loop and poll continuously until the expected events come, or an internal error
         * occurs. Possible events to expect: line rising edge or gpio cancel.
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
        st = ptx_HAL_DRV_Interface_SPI_GpioCheckPendingIRQ(ctx, &gpio_pending);

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
                        (void)ptx_HAL_DRV_Interface_SPI_IRQ_Clear_Cancel(ctx);

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
                        st = ptx_HAL_DRV_Interface_SPI_GpioCheckPendingIRQ(ctx, &gpio_pending);
                        if ((PTX_STATUS_SUCCESS == st) && (0 != gpio_pending))
                        {
                            /* The line is not high. */
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

ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetParameter(ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && NULL != parameter)
    {
        switch (parameter->type)
        {
            case HAL_DRV_Interface_Parameter_Type_Transmission_Speed:
                status = ptx_HAL_DRV_Interface_SPI_SetTransmissionSpeed(ctx, parameter->value.TransmissionSpeed);
                break;
            case HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address:
                status = PTX_STATUS_E_GEN_NOT_AVAILABLE;
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

ptx_Status_t ptx_HAL_DRV_Interface_SPI_GetParameter(ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
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
                status = PTX_STATUS_E_GEN_NOT_AVAILABLE;
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

ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetTransmissionSpeed(ptx_HAL_DRV_Interface_SPI_t *ctx, uint32_t speed)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (0 != speed))
    {
        uint32_t speed_applied = speed;

        int error_code = ioctl(ctx->Port->PortFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_applied);
        if (error_code == 0)
        {
            ctx->Speed = speed;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            /* perror("ioctl(SPI speed)"); */
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return status;
}

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_IRQ_Config (ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERFACE_OPEN;

    int fd;

    char buffer[PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_BUFFER_MAX];
    memset(buffer, 0, PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_BUFFER_MAX);
    PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_SET_GPIOCHIP(buffer, PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_GPIOCHIP_USED);

    /*
     * Open a chip fd here. This fd will be used to retrieve line fd and  then closed. A different
     * fd is then used for further gpio access.
     */
    fd = open(buffer, O_RDWR | O_NONBLOCK);
    if (fd > 0)
    {
        ctx->Port->IrqFd = fd;
        
        st = ptx_HAL_DRV_Interface_SPI_GpioSetInput(ctx, &fd);
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
        st = ptx_HAL_DRV_Interface_SPI_GpioSetIrqRising(ctx, &fd);
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

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_GpioSetInput (ptx_HAL_DRV_Interface_SPI_t *ctx, int *retFd)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * This actually gets a line handle and changes the FD!!!!!
     * Call this only from ptx_HAL_DRV_Interface_SPI_IRQ_Config!!!
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

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_GpioSetIrqRising (ptx_HAL_DRV_Interface_SPI_t *ctx, int *retFd)
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

        fd = open(PTX_HAL_DRV_INTERFACE_SPI_WCHARDEV_GPIOCHIP_USED, O_RDWR | O_NONBLOCK);
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

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_GpioCheckPendingIRQ (ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *irqPending)
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

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_IRQ_Clear_Cancel (ptx_HAL_DRV_Interface_SPI_t *ctx)
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

