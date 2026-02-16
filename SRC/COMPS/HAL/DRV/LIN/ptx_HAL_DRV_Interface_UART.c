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
    Module      : Linux UART Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_UART.c

    Description : Implementation of the UART Communication Interface Driver for the Linux Reference Platform
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_DRV_Interface_UART.h"
#include "ptx_OSAL.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/gpio.h>
#include <poll.h>
#include <sys/eventfd.h>

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
#define PTX_HAL_DRV_INTERFACE_UART_NUMBER_PORTS                             (1u)

#define PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_BUFFER_MAX                      64u
#define PTX_HAL_DRV_INTERFACE_UART_WCHAR_DEFAULT_DEV_NAME                   "/dev/serial0"
#define PTX_HAL_DRV_INTERFACE_UART_WCHAR_DEFAULT_DEV_NAME_LEN               13u
#define PTX_HAL_DRV_INTERFACE_UART_MAX_ALLOWED_FRAME_LENGTH                 265u /* Equals max HIP Frame length + 1 byte SOF */
#define PTX_HAL_DRV_INTERFACE_UART_MAX_WRRD_RETRIES                         PTX_HAL_DRV_INTERFACE_UART_MAX_ALLOWED_FRAME_LENGTH
#define PTX_HAL_DRV_INTERFACE_UART_MIN_BYTES_PER_READ                       1u

/*
 * GPIO device used on the reference platform. Beside this, the pin number is required to specify a GPIO pin to be used.
 * Also, please note that it might be required also to set up the GPIO pin in the Device Tree.
 * Default setting: gpiochip0, pin 19.
 */
#define	PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_GPIOCHIP_USED                   "/dev/gpiochip0"
#define	PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_IRQ_LINE_NR                     19U
#define PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_SET_GPIOCHIP(buffer, gpiochip)  snprintf(buffer, PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_BUFFER_MAX, gpiochip)

/*
 * ####################################################################################################################
 * INTERNALS
 * ####################################################################################################################
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_OpenAltBaudrate(ptx_HAL_DRV_Interface_UART_t *ctx, uint32_t altRate);

static ptx_Status_t ptx_HAL_DRV_Interface_UART_InitComPort(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_UART_InitParams_t *initParams);

static ptx_Status_t ptx_HAL_DRV_Interface_UART_GpioSetInput (ptx_HAL_DRV_Interface_UART_t *ctx, int *retFd);
static ptx_Status_t ptx_HAL_DRV_Interface_UART_GpioSetIrqRising (ptx_HAL_DRV_Interface_UART_t *ctx, int *retFd);
static ptx_Status_t ptx_HAL_DRV_Interface_UART_IRQ_Clear_Cancel(ptx_HAL_DRV_Interface_UART_t *ctx);

static ptx_Status_t ptx_HAL_DRV_Interface_UART_IRQ_Config (ptx_HAL_DRV_Interface_UART_t *ctx);
static ptx_Status_t ptx_HAL_DRV_Interface_UART_GpioCheckPendingIRQ(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *irqPending);

static ptx_Status_t ptx_HAL_DRV_Interface_UART_SetStopBitCountInternal(ptx_HAL_DRV_Interface_UART_t* ctx, uint8_t enableTwoStopBits);
static ptx_Status_t ptx_HAL_DRV_Interface_UART_SetStopBitCount(ptx_HAL_DRV_Interface_UART_t* ctx, uint8_t enableTwoStopBits);

static ptx_HAL_DRV_Interface_UART_PortConfig_t gDrvUartConfigs[PTX_HAL_DRV_INTERFACE_UART_NUMBER_PORTS] =
{
    {
        .Type           = HAL_DRV_Interface_Port_Type_Default,
        .ID             = 0,

        .Dev            = -1,
        .Options        = {0},

        .IrqFd          = -1,
        .IrqFdCancel    = -1,

        .IrqPin         = PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_IRQ_LINE_NR,
    }
};

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_HAL_DRV_Interface_UART_Init(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_UART_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams))
    {
        (void)memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_UART_t));

        for (uint8_t config = 0; (PTX_HAL_DRV_INTERFACE_UART_NUMBER_PORTS > config) && (PTX_STATUS_SUCCESS != status); ++config)
        {
            if ((gDrvUartConfigs[config].Type == initParams->PortType) && (gDrvUartConfigs[config].ID == initParams->PortID))
            {

                ctx->Port = &gDrvUartConfigs[config];
                ctx->Speed = (0 == initParams->Speed) ? PTX_HAL_DRV_INTERFACE_UART_DEFAULT_BAUDRATE : initParams->Speed;
                ctx->Log = initParams->Log;
                ctx->OS = (ptx_OSAL_t*)initParams->OS;
                ctx->EnableFastMode = 0;

                status = ptx_OSAL_MutexInit(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_HAL_DRV_Interface_UART_InitComPort(ctx, initParams);

                    if (PTX_STATUS_SUCCESS != status)
                    {
                        /* Mutex was properly initialized, but Uart port not properly opened. Clean-up mutex. */
                        (void) ptx_OSAL_MutexDeInit(ctx->OS, &ctx->Mutex);
                    }
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    ctx->CompId = Comp_HAL;
                }
                else
                {
                    (void)memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_UART_t));
                    ctx->CompId = Comp_None;
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_DeInit(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        if (0 <= ctx->Port->Dev)
        {
            uint8_t mutex_locked = 0;

            st = ptx_OSAL_MutexLock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);
            if (PTX_STATUS_SUCCESS == st)
            {
                mutex_locked = 1;
                (void) close(ctx->Port->Dev);
            }

            if (1 == mutex_locked)
            {
                ptx_Status_t status = ptx_OSAL_MutexUnlock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);

                /* Keep the error status if it was previously set. */
                st = (PTX_STATUS_SUCCESS == st) ? status : st;
            }
        }
        else
        {
            st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

        if (PTX_STATUS_SUCCESS == st)
        {
            st = ptx_OSAL_MutexDeInit(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);
        }

        if (PTX_STATUS_SUCCESS == st)
        {
            (void)memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_UART_t));
            ctx->CompId = Comp_None;
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_Open(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        st = ptx_HAL_DRV_Interface_UART_DiscardData(ctx);

        if (PTX_STATUS_SUCCESS == st)
        {
            st = ptx_HAL_DRV_Interface_UART_IRQ_Config(ctx);
        }

        if (PTX_STATUS_SUCCESS == st)
        {
            PTX_LOG_I (ctx->Log, "UART Open OK");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "UART Open FAILED");
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_Close(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        (void)close(ctx->Port->IrqFd);
        (void)close(ctx->Port->IrqFdCancel);
        st = PTX_STATUS_SUCCESS;

        if (PTX_STATUS_SUCCESS == st)
        {
            PTX_LOG_I (ctx->Log, "UART Close OK");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "UART Close FAILED");
        }
    }

    return st;
}


ptx_Status_t ptx_HAL_DRV_Interface_UART_Tx(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *txBuf, uint32_t txLen)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != txBuf) && (txLen > 0))
    {
        uint32_t length = txLen;
        int bytes_written = 0;
        uint32_t write_index = 0;
        int error_count = 0;

        do
        {
            uint8_t mutex_locked = 0;

            st = ptx_OSAL_MutexLock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);
            if (PTX_STATUS_SUCCESS == st)
            {
                mutex_locked = 1;
            }

            /*
             * write() will not always write all: Check this and repeat operation if needed!
             */
            if (PTX_STATUS_SUCCESS == st)
            {
                bytes_written = (int) write(ctx->Port->Dev, &(txBuf[write_index]), length - write_index);
            }

            if (1 == mutex_locked)
            {
                ptx_Status_t status = ptx_OSAL_MutexUnlock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);

                /* Keep the error status if it was previously set. */
                st = (PTX_STATUS_SUCCESS == st) ? status : st;
            }

            if (0 < bytes_written)
            {
                write_index += (uint32_t) bytes_written;
            }
            else
            {
                error_count++;
            }
        }
        while ((0 == error_count) && (PTX_STATUS_SUCCESS == st) && (write_index < length));

        if ((0 < bytes_written) && (write_index == length))
        {
            /* OK */
            st = PTX_STATUS_SUCCESS;
        }
        else
        {
            /*
             * Write length mismatch
             */
            st = PTX_STATUS_E_GEN_INTERNAL;
        }

        /*
         * PTX2K needs a minimum of 1 bit delay between HIP frames. The tcdrain() function blocks until all data has been written,
         * effectively making sure this delay is kept. However, the function consistently blocks for 8-12ms more than necessary.
         * Therefore, tcdrain() is only used when fast mode is disabled which is before the NSC init command.
         * For performance reasons, fast mode can be enabled using ptx_HAL_Interface_SetParameter afterwards. This enables 2 stop bit
         * communication which automatically introduces a second bit after each byte, making sure the delay is kept.
         * This cannot be enabled before the NSC init command, as the command and all previous HIP resets reset this flag on the PTX2K.
         */
        if (false == ctx->EnableFastMode)
        {
            tcdrain(ctx->Port->Dev);
        }

#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == st)
        {
            PTX_LOG_I_MEM (ctx->Log, txBuf, txLen, "UART Write: ");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "UART Write ERROR Status: %u", st);
        }
#endif
    }

    return st;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_Rx(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *rxBuf, uint32_t rxLen)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((PTX_COMP_CHECK(ctx, Comp_HAL)) && (NULL != rxBuf))
    {
        st = PTX_STATUS_SUCCESS;
        if (0 == rxLen)
        {
            /*
             * Although we fell through until here, the result is fine.
             * We were supposed to read nothing and this is just what we have done.
             * Status already set to success.
             */
        }
        else
        {
            uint32_t num_read = 0;
            uint32_t nr_to_read = rxLen;
            ssize_t rd_status = 0;
            uint32_t retries = PTX_HAL_DRV_INTERFACE_UART_MAX_WRRD_RETRIES; /* In the worst case one byte at a time is read out from the file buffer */
            do
            {
                rd_status = read(ctx->Port->Dev, &(rxBuf[num_read]), PTX_HAL_DRV_INTERFACE_UART_MAX_ALLOWED_FRAME_LENGTH);
                retries --;
                if (rd_status >= 0)
                {
                    num_read += (uint32_t)rd_status;
                }
                else
                {
                    /* read() failed and errno was set */
                    st = PTX_STATUS_E_GEN_INTERNAL;
                }

                if (retries == 0)
                {
                    st = PTX_STATUS_E_GEN_TIMEOUT;
                }
            }
            while ((num_read < nr_to_read) && (PTX_STATUS_SUCCESS == st));
        }

#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == st)
        {
            PTX_LOG_I_MEM (ctx->Log, rxBuf, rxLen, "UART Read: ");
        }
        else
        {
            PTX_LOG_I (ctx->Log, "UART Read ERROR Status: %u", st);
        }
#endif
    }
    else
    {
        st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return st;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_ActivateRxIrq(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_DeactivateRxIrq(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_RxIrqLevel(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != level))
    {
        uint8_t gpio_state = 0;

        /* Check if the IRQ of the GPIO-pin is already pending i.e. set to 1 */
        status = ptx_HAL_DRV_Interface_UART_GpioCheckPendingIRQ(ctx, &gpio_state);
        if (PTX_STATUS_SUCCESS == status)
        {
            *level = gpio_state;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_WaitForIRQ(ptx_HAL_DRV_Interface_UART_t* ctx)
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
        st = ptx_HAL_DRV_Interface_UART_GpioCheckPendingIRQ(ctx, &gpio_pending);

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
                        (void)ptx_HAL_DRV_Interface_UART_IRQ_Clear_Cancel(ctx);

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
                        st = ptx_HAL_DRV_Interface_UART_GpioCheckPendingIRQ(ctx, &gpio_pending);
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

ptx_Status_t ptx_HAL_DRV_Interface_UART_SetParameter(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && NULL != parameter)
    {
        switch (parameter->type)
        {
            case HAL_DRV_Interface_Parameter_Type_Transmission_Speed:
            {
                status = ptx_HAL_DRV_Interface_UART_OpenAltBaudrate(ctx, parameter->value.TransmissionSpeed);
                break;
            }
            case HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address:
            {
                status = PTX_STATUS_E_GEN_NOT_AVAILABLE;
                break;
            }
            case HAL_DRV_Interface_Parameter_Type_UART_Enable_Fast_Mode:
            {
                // Enable two stop bits for fast mode, one stop bit otherwise.
                uint8_t stop_bit_count = parameter->value.LinuxUARTEnableFastMode == true ? 2 : 1;

                // Set stop bit count on PTX2K and internally
                status = ptx_HAL_DRV_Interface_UART_SetStopBitCount(ctx, stop_bit_count);
                ctx->EnableFastMode = parameter->value.LinuxUARTEnableFastMode;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return status;
}


ptx_Status_t ptx_HAL_DRV_Interface_UART_GetParameter(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
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
 * INTERNAL FUNCTIONS / HELPERS
 * ####################################################################################################################
 */

ptx_Status_t ptx_HAL_DRV_Interface_UART_DiscardData(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        uint8_t mutex_locked = 0;

        st = ptx_OSAL_MutexLock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);
        if (PTX_STATUS_SUCCESS == st)
        {
            mutex_locked = 1;
        }

        if ((PTX_STATUS_SUCCESS == st) && (0 == tcflush(ctx->Port->Dev, TCIOFLUSH)))
        {
            st = PTX_STATUS_SUCCESS;
        }
        else
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }

        if (1 == mutex_locked)
        {
            ptx_Status_t status = ptx_OSAL_MutexUnlock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);

            /* Keep the error status if it was previously set. */
            st = (PTX_STATUS_SUCCESS == st) ? status : st;
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_Cancel(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        uint32_t i = 0;
        uint64_t nr = 1;
        uint32_t bytes_written = 0;
        uint32_t bytes_to_write = sizeof(nr);
        int bw = -1;

        uint8_t mutex_locked = 0;

        st = ptx_OSAL_MutexLock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);
        if (PTX_STATUS_SUCCESS == st)
        {
            mutex_locked = 1;
        }

        do
        {
            if (PTX_STATUS_SUCCESS == st)
            {
                bw = (int) write(ctx->Port->Dev, &nr, bytes_to_write - bytes_written); /* Signal <nr> times by writing 8 bytes, representing <nr> into the efd. */
            }

            if (bw >= 0)
            {
                bytes_written += (uint32_t) bw;
            }
            else
            {
                st = PTX_STATUS_E_GEN_INTERNAL;
            }

            i++;
        }
        while ((PTX_STATUS_SUCCESS == st) && (bytes_written < bytes_to_write) && (i < PTX_HAL_DRV_INTERFACE_UART_MAX_WRRD_RETRIES));

        if (1 == mutex_locked)
        {
            ptx_Status_t status = ptx_OSAL_MutexUnlock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);

            /* Keep the error status if it was previously set. */
            st = (PTX_STATUS_SUCCESS == st) ? status : st;
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_ClearCancel(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        uint64_t rd_num;
        ssize_t br;
        uint32_t i = 0;
        size_t bytes_read = 0;
        size_t bytes_to_read = sizeof(rd_num);

        uint8_t mutex_locked = 0;

        st = ptx_OSAL_MutexLock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);
        if (PTX_STATUS_SUCCESS == st)
        {
            mutex_locked = 1;
        }

        do
        {
            br = (int) read(ctx->Port->Dev, &rd_num, bytes_to_read - bytes_read); /* Non-blocking */
            if (br >= 0)
            {
                bytes_read += (size_t) br;
            }
            else
            {
                int local_error = errno;
                if (EAGAIN == local_error)
                {
                    /*
                     * Not yet all..
                     */
                }
                else
                {
                    st = PTX_STATUS_E_GEN_INTERNAL;
                    break;
                }

                if (EWOULDBLOCK == local_error)
                {
                    /*
                     * Not yet all..
                     */
                }
                else
                {
                    st = PTX_STATUS_E_GEN_INTERNAL;
                    break;
                }
            }
            i++;
        }
        while ((PTX_STATUS_SUCCESS == st) && (bytes_read < bytes_to_read) && (i < PTX_HAL_DRV_INTERFACE_UART_MAX_WRRD_RETRIES));

        if (1 == mutex_locked)
        {
            ptx_Status_t status = ptx_OSAL_MutexUnlock(ctx->OS, (ptx_Mutex_t*)&ctx->Mutex);

            /* Keep the error status if it was previously set. */
            st = (PTX_STATUS_SUCCESS == st) ? status : st;
        }
    }

    return st;
}


static ptx_Status_t ptx_HAL_DRV_Interface_UART_InitComPort(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_UART_InitParams_t *initParams)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams))
    {
        ptx_HAL_DRV_Interface_UART_PortConfig_t *uartConfig = ctx->Port;

        char dev_name[PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_BUFFER_MAX];
        (void)memset(dev_name, 0, sizeof(char)*PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_BUFFER_MAX);

        if (initParams->InterfaceName == NULL)
        {
            (void)memcpy(dev_name, PTX_HAL_DRV_INTERFACE_UART_WCHAR_DEFAULT_DEV_NAME, sizeof(char)*PTX_HAL_DRV_INTERFACE_UART_WCHAR_DEFAULT_DEV_NAME_LEN);
        }
        else
        {
            (void)memcpy(dev_name, initParams->InterfaceName, initParams->InterfaceNameLen);
        }

        uartConfig->Dev = open(dev_name, O_RDWR | O_NOCTTY);

        if (0 <= uartConfig->Dev)
        {
            if (0 != tcgetattr(uartConfig->Dev, &uartConfig->Options))
            {
                st = PTX_STATUS_E_GEN_INTERNAL;
            }
            else
            {
                if (0 != fcntl(uartConfig->Dev, F_SETFL, 0))
                {
                    st = PTX_STATUS_E_GEN_INTERNAL;
                }
                else
                {
                    st = PTX_STATUS_SUCCESS;
                }
            }

            if (PTX_STATUS_SUCCESS == st)
            {
                uint32_t baudrate = 0;
                st = ptx_HAL_DRV_Interface_UART_UartBaudrateConv(ctx->Speed, &baudrate);

                if (PTX_STATUS_SUCCESS == st)
                {
                    int cfset_ret = 0;
                    cfset_ret = cfsetispeed(&uartConfig->Options, baudrate);

                    if (0 == cfset_ret)
                    {
                        /* input speed setting successful */
                        cfset_ret = cfsetospeed(&uartConfig->Options, baudrate);
                        if (0 != cfset_ret)
                        {
                            /* if we get here reason is something else than baudrate, otherwise cfsetispeed would have already failed (check errno) */
                            (void) close(uartConfig->Dev);
                            st = PTX_STATUS_E_GEN_INTERNAL;
                        }
                    }
                    else
                    {
                        /* input speed setting failed */
                        if (EINVAL == errno)
                        {
                            st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                        }
                        else
                        {
                            st = PTX_STATUS_E_GEN_INTERNAL;
                        }
                    }

                    uartConfig->Options.c_cc[VMIN]    = PTX_HAL_DRV_INTERFACE_UART_MIN_BYTES_PER_READ;      /* Min bytes to be received to return from read system call */
                    uartConfig->Options.c_cc[VTIME]   = (cc_t)initParams->Timeout;       /* Set receive timeout between bytes */
                    uartConfig->Options.c_cflag      &= (tcflag_t)~CSTOPB;               /* 1 stop bit used per default (fast mode disabled) */
                    uartConfig->Options.c_cflag      |= (tcflag_t)(CREAD | CLOCAL);      /* Turn on READ & ignore ctrl lines (CLOCAL = 1) */
                    uartConfig->Options.c_cflag      &= (tcflag_t)~(CSIZE | PARENB);     /* 8 bits per byte */
                    uartConfig->Options.c_cflag      |= CS8;
                    uartConfig->Options.c_cflag      &= ~CRTSCTS;
                    uartConfig->Options.c_lflag      &= ~(tcflag_t)(ICANON | ECHO | ECHONL | ISIG | IEXTEN); /* Set Raw mode */
                    uartConfig->Options.c_oflag      &= ~(tcflag_t) (OPOST);             /* Clear post process output to set Raw output processing */

                    uartConfig->Options.c_iflag      &= ~(tcflag_t)(IXON | IXOFF | IXANY); /* Turn off s/w flow ctrl */
                    uartConfig->Options.c_iflag      &= ~(tcflag_t)(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); /* Disable any special handling of received bytes */
                }
            }

            if (PTX_STATUS_SUCCESS == st)
            {
                if (tcsetattr(uartConfig->Dev, TCSANOW, &uartConfig->Options) != 0)
                {
                    (void) close(uartConfig->Dev);
                    st = PTX_STATUS_E_GEN_INTERNAL;
                }
            }
        }
        else
        {
            (void) close(uartConfig->Dev);
            st = PTX_STATUS_E_GEN_INTERNAL;
        }

        /* If init failed ensure that Dev contains invalid fd ID */
        if (PTX_STATUS_SUCCESS != st)
        {
            uartConfig->Dev = -1;
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_UartBaudrateConv(uint32_t rawBaud, uint32_t* convBaud)
{
    ptx_Status_t status = PTX_STATUS_SUCCESS;

    switch (rawBaud)
    {
        case 9600:
            *convBaud = (uint32_t)B9600;
            break;

        case 19200:
            *convBaud = (uint32_t)B19200;
            break;

        case 38400:
            *convBaud = (uint32_t)B38400;
            break;

        case 57600:
            *convBaud = (uint32_t)B57600;
            break;

        case 115200:
            *convBaud = (uint32_t)B115200;
            break;

        case 230400:
            *convBaud = (uint32_t)B230400;
            break;

        case 460800:
            *convBaud = (uint32_t)B460800;
            break;

        case 500000:
            *convBaud = (uint32_t)B500000;
            break;

        case 921600:
            *convBaud = (uint32_t)B921600;
            break;

        case 1000000:
            *convBaud = (uint32_t)B1000000;
            break;

        case 1500000:
            *convBaud = (uint32_t)B1500000;
            break;

        case 2000000:
            *convBaud = (uint32_t)B2000000;
            break;

        case 2500000:
            *convBaud = (uint32_t)B2500000;
            break;

        case 3000000:
            *convBaud = (uint32_t)B3000000;
            break;

        case 3500000:
            *convBaud = (uint32_t)B3500000;
            break;

        case 4000000:
            *convBaud = (uint32_t)B4000000;
            break;

        default:
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            break;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_OpenAltBaudrate(ptx_HAL_DRV_Interface_UART_t *ctx, uint32_t altRate)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        uint32_t baudrate = 0;
        status = ptx_HAL_DRV_Interface_UART_UartBaudrateConv(altRate, &baudrate);
        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 == cfsetispeed(&ctx->Port->Options, baudrate))
            {
                /* changing input speed successful */
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                (void) close(ctx->Port->Dev);
                status = PTX_STATUS_E_GEN_INTERNAL;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* change output speed */
            if (0 != cfsetospeed(&ctx->Port->Options, baudrate))
            {
                /* if we get here reason is something else than baudrate, otherwise cfsetispeed would have already failed (check errno) */
                (void) close(ctx->Port->Dev);
                status = PTX_STATUS_E_GEN_INTERNAL;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* apply port settings */
            if (0 != tcsetattr(ctx->Port->Dev, TCSANOW, &ctx->Port->Options))
            {
                (void) close(ctx->Port->Dev);
                status = PTX_STATUS_E_GEN_INTERNAL;
            }
            else
            {
                /* status already set to success, update Speed */
                ctx->Speed = altRate;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_HAL_DRV_Interface_UART_IRQ_Config (ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERFACE_OPEN;

    int fd;

    char buffer[PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_BUFFER_MAX];
    memset(buffer, 0, PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_BUFFER_MAX);
    PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_SET_GPIOCHIP(buffer, PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_GPIOCHIP_USED);

    /*
     * Open a chip fd here. This fd will be used to retrieve line fd and  then closed. A different
     * fd is then used for further gpio access.
     */
    fd = open(buffer, O_RDWR | O_NONBLOCK);
    if (fd > 0)
    {
        ctx->Port->IrqFd = fd;
        st = ptx_HAL_DRV_Interface_UART_GpioSetInput(ctx, &fd);
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
        st = ptx_HAL_DRV_Interface_UART_GpioSetIrqRising(ctx, &fd);
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

static ptx_Status_t ptx_HAL_DRV_Interface_UART_GpioSetInput (ptx_HAL_DRV_Interface_UART_t *ctx, int *retFd)
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

static ptx_Status_t ptx_HAL_DRV_Interface_UART_GpioSetIrqRising (ptx_HAL_DRV_Interface_UART_t *ctx, int *retFd)
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

        fd = open(PTX_HAL_DRV_INTERFACE_UART_WCHARDEV_GPIOCHIP_USED, O_RDWR | O_NONBLOCK);
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

static ptx_Status_t ptx_HAL_DRV_Interface_UART_GpioCheckPendingIRQ (ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *irqPending)
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

static ptx_Status_t ptx_HAL_DRV_Interface_UART_SetStopBitCountInternal(ptx_HAL_DRV_Interface_UART_t* ctx, uint8_t enableTwoStopBits)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        if (tcgetattr(ctx->Port->Dev, &ctx->Port->Options) != 0)
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            st = PTX_STATUS_SUCCESS;
        }
        if (PTX_STATUS_SUCCESS == st)
        {
            if (enableTwoStopBits != 0)
            {
                ctx->Port->Options.c_cflag |= (tcflag_t)CSTOPB;
            }
            else
            {
                ctx->Port->Options.c_cflag &= (tcflag_t)~CSTOPB;
            }

            if (tcsetattr(ctx->Port->Dev, TCSANOW, &ctx->Port->Options) != 0)
            {
                st = PTX_STATUS_E_GEN_INTERNAL;
            }
        }
    }

    return st;
}

static ptx_Status_t ptx_HAL_DRV_Interface_UART_SetStopBitCount(ptx_HAL_DRV_Interface_UART_t* ctx, uint8_t enableTwoStopBits)
{
    /* configures the 2K to send 1 or 2 stop bits through HIF_UART_CONFIG0_REG */
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        const uint8_t HIP_CMD_START_LEN = 3;
        const uint16_t HIF_UART_CONFIG0_REG = 0x018;

        uint8_t tx_buffer[50];
        uint32_t tx_len = HIP_CMD_START_LEN;

        /* RDM cmd */
        tx_buffer[tx_len] = 0xB0;
        tx_len++;

        /* register addr */
        tx_buffer[tx_len] = (uint8_t)(HIF_UART_CONFIG0_REG >> 8);
        tx_len++;
        tx_buffer[tx_len] = (uint8_t)(HIF_UART_CONFIG0_REG & 0xFF);
        tx_len++;

        /* nr of bytes to read payload + 2 len bytes + 1 op code byte */
        uint16_t b_to_read = (uint16_t)(1);
        tx_buffer[tx_len] = (uint8_t)(b_to_read >> 8);
        tx_len++;
        tx_buffer[tx_len] = (uint8_t)(b_to_read & 0xFF);
        tx_len++;

        /* cmd length (excluding payload len bytes)*/
        tx_buffer[0] = 0x55; // SOF
        tx_buffer[1] = (uint8_t) ((tx_len - HIP_CMD_START_LEN) >> 8);
        tx_buffer[2] = (uint8_t) (tx_len - HIP_CMD_START_LEN) & 0xFF;

        uint8_t rx_buffer[300];
        uint32_t rx_len = 4;

        status = ptx_HAL_DRV_Interface_UART_Tx(ctx, tx_buffer, tx_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_DRV_Interface_UART_Rx(ctx, rx_buffer, rx_len);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            tx_len = HIP_CMD_START_LEN;

            /* WDM cmd */
            tx_buffer[tx_len] = 0xA0;
            tx_len++;

            /* register addr */
            tx_buffer[tx_len] = (uint8_t)(HIF_UART_CONFIG0_REG >> 8);
            tx_len++;
            tx_buffer[tx_len] = (uint8_t)(HIF_UART_CONFIG0_REG & 0xFF);
            tx_len++;

            if (0 == enableTwoStopBits)
            {
                /* set 1 stop bit */
                tx_buffer[tx_len] = (uint8_t)(rx_buffer [3] & ~0x10);
                tx_len++;
            }
            else
            {
                /* set 2 stop bits */
                tx_buffer[tx_len] = (uint8_t)(rx_buffer [3] | 0x10);
                tx_len++;
            }

            /* cmd length (excluding payload len bytes)*/
            tx_buffer[0] = 0x55; // SOF
            tx_buffer[1] = (uint8_t) ((tx_len - HIP_CMD_START_LEN) >> 8);
            tx_buffer[2] = (uint8_t) (tx_len - HIP_CMD_START_LEN) & 0xFF;

            status = ptx_HAL_DRV_Interface_UART_Tx(ctx, tx_buffer, tx_len);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            // Update internal stop bit count used in HAL driver.
            status = ptx_HAL_DRV_Interface_UART_SetStopBitCountInternal(ctx, enableTwoStopBits);
        }
    }

    return status;
}

static ptx_Status_t ptx_HAL_DRV_Interface_UART_IRQ_Clear_Cancel (ptx_HAL_DRV_Interface_UART_t *ctx)
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


