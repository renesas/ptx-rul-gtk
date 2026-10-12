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
    Module      : MCU-RA4M2 UART Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_UART.c

    Description : Implementation of the UART Communication Interface Driver for the MCU-RA4M2 Reference Platform
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <limits.h>

#include "ptx_HAL_DRV_Interface_UART.h"
#include "ptx_HAL_GPIO.h"
#include "ptx_HAL_DRV_MCU.h"



/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_HAL_DRV_INTERFACE_UART_RESET_VALUE                    ((int32_t)0u)
#define PTX_HAL_DRV_INTERFACE_UART_DEFAULT_CHANNEL                (0u)
#define PTX_HAL_DRV_INTERFACE_UART_DEBUG_CHANNEL                  (2u)
#define PTX_HAL_DRV_INTERFACE_UART_TRANSFER_COMPLETE              (1u)
#define PTX_HAL_DRV_INTERFACE_UART_NUMBER_PORTS                   (2u)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_HAL_DRV_Interface_UART_PortConfig_t gDrvUartPortConfigs[PTX_HAL_DRV_INTERFACE_UART_NUMBER_PORTS] =
{
    {
        .Type                   = HAL_DRV_Interface_Port_Type_Default,
        .ID                     = 0,
        .Instance               = &ptx_uart_default,
        .Irq                    = BSP_IO_PORT_04_PIN_14,
        .RxRingBuffer           = {0},

        .Channel                = PTX_HAL_DRV_INTERFACE_UART_DEFAULT_CHANNEL,
        .TransferState          = PTX_HAL_DRV_INTERFACE_UART_RESET_VALUE,
        .TransferWaitCnt        = 0u,
        .ExternalIrqInstance    = &ptx_pmod2_irq,
    },
    {
        .Type                   = HAL_DRV_Interface_Port_Type_Debug,
        .ID                     = 0,
        .Instance               = &ptx_uart_dbg,
        .Irq                    = BSP_IO_PORT_04_PIN_14,
        .RxRingBuffer           = {0},

        .Channel                = PTX_HAL_DRV_INTERFACE_UART_DEBUG_CHANNEL,
        .TransferState          = PTX_HAL_DRV_INTERFACE_UART_RESET_VALUE,
        .TransferWaitCnt        = 0u,
        .ExternalIrqInstance    = &ptx_pmod2_irq,
    }
};

extern void ptx_HAL_DRV_Interface_UART_TransferCallback(uart_callback_args_t *p_args);
void ptx_HAL_DRV_Interface_UART_IrqCallback(external_irq_callback_args_t *p_args);

static ptx_Status_t ptx_HAL_DRV_Interface_UART_Wait4TransferDone(ptx_HAL_DRV_Interface_UART_t *ctx);
static ptx_Status_t ptx_HAL_DRV_Interface_UART_SetTransmissionSpeed(ptx_HAL_DRV_Interface_UART_t *ctx, uint32_t speed);
static ptx_Status_t ptx_HAL_DRV_Interface_UART_SetBaudRate(ptx_HAL_DRV_Interface_UART_t *ctx, uint32_t baudrate);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Init(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_UART_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams) && (NULL != initParams->Log))
    {
        PTX_LOG_V(initParams->Log, "ptx_HAL_DRV_Interface_UART_Init");

        memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_UART_t));

        for (uint8_t config = 0; (PTX_HAL_DRV_INTERFACE_UART_NUMBER_PORTS > config) && (PTX_STATUS_SUCCESS != status); ++config)
        {
            if ((gDrvUartPortConfigs[config].Type == initParams->PortType) && (gDrvUartPortConfigs[config].ID == initParams->PortID))
            {

                ctx->Port = &gDrvUartPortConfigs[config];
                ctx->Speed = (0 == initParams->Speed) ? PTX_HAL_DRV_INTERFACE_UART_DEFAULT_BAUDRATE : initParams->Speed;
                ctx->Log = initParams->Log;
                ctx->Gpio = initParams->Gpio;
                ctx->CompId = Comp_HAL;
                status = PTX_STATUS_SUCCESS;


            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_DeInit(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_UART_DeInit");

        memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_UART_t));
        ctx->CompId = Comp_None;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_Open(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_UART_Open");

        const uart_instance_t *uart = ctx->Port->Instance;
        fsp_err_t r_status = R_SCI_UART_Open(uart->p_ctrl, uart->p_cfg);

        if (FSP_ERR_ALREADY_OPEN == r_status)
        {
            /* Close and re-open the interface. */
            (void)R_SCI_UART_Close(uart->p_ctrl);
            if (NULL != ctx->Port->ExternalIrqInstance)
            {
                const external_irq_instance_t *irq = ctx->Port->ExternalIrqInstance;
                (void)R_ICU_ExternalIrqClose(irq->p_ctrl);
            }

            r_status = R_SCI_UART_Open(uart->p_ctrl, uart->p_cfg);
        }


        if (FSP_SUCCESS == r_status)
        {

            if (HAL_DRV_Interface_Port_Type_Debug != ctx->Port->Type)
            {
                const external_irq_instance_t *irq = ctx->Port->ExternalIrqInstance;

                if (NULL != irq)
                {
                    r_status = R_ICU_ExternalIrqOpen(irq->p_ctrl, irq->p_cfg);

                    if (FSP_SUCCESS == r_status)
                    {
                        r_status = R_ICU_ExternalIrqEnable(irq->p_ctrl);
                    }

                    if (FSP_SUCCESS == r_status)
                    {
                        r_status = R_ICU_ExternalIrqCallbackSet(ctx->Port->ExternalIrqInstance->p_ctrl, ptx_HAL_DRV_Interface_UART_IrqCallback, ctx, NULL);
                    }
                }

            }

            if (FSP_SUCCESS == r_status)
            {
                r_status = R_SCI_UART_CallbackSet(uart->p_ctrl, ptx_HAL_DRV_Interface_UART_TransferCallback, ctx, NULL);
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
                status = ptx_HAL_DRV_Interface_UART_SetBaudRate(ctx, ctx->Speed);
            }

            if ((PTX_STATUS_SUCCESS == status) && (HAL_DRV_Interface_Port_Type_Debug == ctx->Port->Type))
            {
                (void)ptx_HAL_DRV_MCU_DisableIrq();
                ptx_RingBuffer_Init(&ctx->Port->RxRingBuffer, ctx->RxRingBuffer, PTX_HAL_DRV_INTERFACE_UART_RX_RING_BUF_SIZE);
                (void)ptx_HAL_DRV_MCU_EnableIrq();
            }
        }
    }
    return status;
    
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_Close(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;


    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_UART_Close");

        const uart_instance_t *uart = ctx->Port->Instance;

        fsp_err_t r_status = R_SCI_UART_Close(uart->p_ctrl);

        if ((FSP_SUCCESS == r_status) || (FSP_ERR_NOT_OPEN == r_status))
        {
            if (NULL != ctx->Port->ExternalIrqInstance)
            {
                const external_irq_instance_t *irq = ctx->Port->ExternalIrqInstance;
                r_status = R_ICU_ExternalIrqClose(irq->p_ctrl);
            }
        }

        if ((FSP_SUCCESS == r_status) || (FSP_ERR_NOT_OPEN == r_status))
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_CLOSE;
        }

        if ((PTX_STATUS_SUCCESS == status) && (HAL_DRV_Interface_Port_Type_Debug == ctx->Port->Type))
        {
            (void)ptx_HAL_DRV_MCU_DisableIrq();
            ptx_RingBuffer_Reset(&ctx->Port->RxRingBuffer);
            (void)ptx_HAL_DRV_MCU_EnableIrq();
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_Tx(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *txBuf, uint32_t txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != txBuf))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_UART_Tx");

        const uart_instance_t *uart = ctx->Port->Instance;

        ctx->Port->TransferState = PTX_HAL_DRV_INTERFACE_UART_RESET_VALUE;
        ctx->Port->TransferWaitCnt = INT_MAX;

        fsp_err_t r_status = R_SCI_UART_Write(uart->p_ctrl, txBuf, txLen);

        if (FSP_SUCCESS == r_status)
        {
            status = ptx_HAL_DRV_Interface_UART_Wait4TransferDone(ctx);
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_TX;
        }

#if !defined(PTX_ENABLE_TFCI)
        if (HAL_DRV_Interface_Port_Type_Debug != ctx->Port->Type)
        {
            if (PTX_STATUS_SUCCESS == status)
            {
                PTX_LOG_I_MEM(ctx->Log, &txBuf[0], txLen, "TX: ");
            }
        }
#endif
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_Rx(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *rxBuf, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != rxBuf))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_UART_Rx");

        if (HAL_DRV_Interface_Port_Type_Debug == ctx->Port->Type)
        {
            (void)ptx_HAL_DRV_MCU_DisableIrq();
            ptx_RingBuffer_Read(&ctx->Port->RxRingBuffer, rxBuf, (uint16_t*) &rxLen);
            (void)ptx_HAL_DRV_MCU_EnableIrq();
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            const uart_instance_t *uart = ctx->Port->Instance;

            ctx->Port->TransferState = PTX_HAL_DRV_INTERFACE_UART_RESET_VALUE;
            ctx->Port->TransferWaitCnt = INT_MAX;

            fsp_err_t r_status = R_SCI_UART_Read(uart->p_ctrl, &rxBuf[0], rxLen);

            if (FSP_SUCCESS == r_status)
            {
                status = ptx_HAL_DRV_Interface_UART_Wait4TransferDone(ctx);
            }
            else
            {
                status = PTX_STATUS_E_GEN_INTERFACE_RX;
            }
        

#if !defined(PTX_ENABLE_TFCI)
                if (PTX_STATUS_SUCCESS == status)
                {
                    PTX_LOG_I_MEM(ctx->Log, &rxBuf[0], rxLen, "RX: ");
                }
#endif
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_RxIrqLevel(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_UART_RxIrqLevel");

        if (HAL_DRV_Interface_Port_Type_Debug == ctx->Port->Type)
        {
            (void)ptx_HAL_DRV_MCU_DisableIrq();
            uint16_t size = ptx_RingBuffer_Size(&ctx->Port->RxRingBuffer);
            (void)ptx_HAL_DRV_MCU_EnableIrq();

            *level = (0 < size) ? 1 : 0;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            bsp_io_level_t value;
            memset(&value, 0, sizeof(value));

            fsp_err_t r_status = R_IOPORT_PinRead(ctx->Gpio->Drv.Port->Instance->p_ctrl, ctx->Port->Irq, &value);
            if (FSP_SUCCESS == r_status)
            {
                *level = value;
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INTERFACE_IRQ_READ;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_SetParameter(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != parameter))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_UART_SetParameter");

        switch (parameter->type)
        {
            case HAL_DRV_Interface_Parameter_Type_Transmission_Speed:
                status = ptx_HAL_DRV_Interface_UART_SetTransmissionSpeed(ctx, parameter->value.TransmissionSpeed);
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

ptx_Status_t ptx_HAL_DRV_Interface_UART_GetParameter(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != parameter))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_UART_GetParameter");

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
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

ptx_Status_t ptx_HAL_DRV_Interface_UART_SetTransmissionSpeed(ptx_HAL_DRV_Interface_UART_t *ctx, uint32_t speed)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (0 != speed))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_UART_SetTransmissionSpeed");

        status = ptx_HAL_DRV_Interface_UART_SetBaudRate(ctx, speed);
        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->Speed = speed;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_UART_SetBaudRate(ptx_HAL_DRV_Interface_UART_t *ctx, uint32_t baudrate)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Tmr_PeriodSet");

        baud_setting_t baud_setting;
        fsp_err_t r_status = R_SCI_UART_BaudCalculate(baudrate, PTX_HAL_DRV_INTERFACE_UART_BAUDRATE_MODULATION, PTX_HAL_DRV_INTERFACE_UART_BAUDRATE_ERROR_PERCENT_X_1000, &baud_setting);
        
        if (FSP_SUCCESS == r_status)
        {
            PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_UART_SetBaudRate");

            const uart_instance_t *uart = ctx->Port->Instance;
            r_status = R_SCI_UART_BaudSet(uart->p_ctrl, (void *) &baud_setting);
            if (FSP_SUCCESS == r_status)
            {
                status = PTX_STATUS_SUCCESS;
            }
        }
    }
    return status;
}

void ptx_HAL_DRV_Interface_UART_TransferCallback(uart_callback_args_t * p_args)
{
    if (NULL != p_args)
    {
        ptx_HAL_DRV_Interface_UART_PortConfig_t *config = ((ptx_HAL_DRV_Interface_UART_t *)p_args->p_context)->Port;
        if (config->Channel == p_args->channel)
        {
            switch (p_args->event)
            {
                case UART_EVENT_RX_CHAR:
                {
                    /* Data is received asynchronously via Rx ISR (read has not been called), store data char by char in RingBuffer */
                    if (HAL_DRV_Interface_Port_Type_Debug == config->Type)
                    {
                        ptx_RingBuffer_Write(&config->RxRingBuffer, (uint8_t*) & (p_args->data), 1);
                    }
                    break;
                }

                case UART_EVENT_RX_COMPLETE:
                {
                    /* The number of data which has been read reaches to the number specified in R_SCI_UART_Read() */
                    config->TransferState = PTX_HAL_DRV_INTERFACE_UART_TRANSFER_COMPLETE;
                    break;
                }

                case UART_EVENT_TX_DATA_EMPTY:
                {
                    /* Do nothing, next callback will be UART_EVENT_TX_COMPLETE */
                    break;
                }

                case UART_EVENT_TX_COMPLETE:
                {
                    /* Last byte from TxBuffer was transferred to the physical interface */
                    config->TransferState = PTX_HAL_DRV_INTERFACE_UART_TRANSFER_COMPLETE;
                    break;
                }

                case UART_EVENT_ERR_PARITY:
                /* Parity error event,          FALL_THROUGH */
                case UART_EVENT_ERR_FRAMING:
                /* Mode fault error event,      FALL_THROUGH */
                case UART_EVENT_ERR_OVERFLOW:
                /* FIFO Overflow error event,   FALL_THROUGH */
                case UART_EVENT_BREAK_DETECT:
                /* Break detect error event,    FALL_THROUGH */
                default:
                {
                    break;
                }
            }
        }

    }
}

void ptx_HAL_DRV_Interface_UART_IrqCallback(external_irq_callback_args_t *p_args)
{
    /*
     * Callback is left empty intentionally!
     * All actions regarding the interrupt and the pin level are handled by the device
     * implementation! The ISR callback is just used to wake up the MCU from sleep states
     */
    (void)p_args;
}


static ptx_Status_t ptx_HAL_DRV_Interface_UART_Wait4TransferDone(ptx_HAL_DRV_Interface_UART_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INTERNAL;
    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_UART_Wait4TransferDone");

        while (PTX_HAL_DRV_INTERFACE_UART_TRANSFER_COMPLETE != ctx->Port->TransferState)
        {
            --ctx->Port->TransferWaitCnt;

            if (PTX_HAL_DRV_INTERFACE_UART_RESET_VALUE >= ctx->Port->TransferWaitCnt)
            {
                status = PTX_STATUS_E_GEN_INTERFACE_TIMEOUT;
                break;
            }
            else
            {
                /* Do nothing. Error already set. */
            }
        }

        if (PTX_HAL_DRV_INTERFACE_UART_TRANSFER_COMPLETE == ctx->Port->TransferState)
        {
            status = PTX_STATUS_SUCCESS;
        }
    }
    return status;
}
