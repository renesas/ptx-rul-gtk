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
    Module      : MCU-RA4M2 I2C Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_I2C.c

    Description : Implementation of the I2C Communication Interface Driver for the MCU-RA4M2 Reference Platform
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <limits.h>

#include "ptx_HAL_DRV_Interface_I2C.h"
#include "ptx_HAL_GPIO.h"

#include "hal_data.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_HAL_DRV_I2C_RESET_VALUE                   ((int32_t)0u)
#define PTX_HAL_DRV_I2C_DEFAULT_CHANNEL               (0u)
#define PTX_HAL_DRV_I2C_EVK_CHANNEL                   (4u)
#define PTX_HAL_DRV_INTERFACE_I2C_NUMBER_PORTS        (1u)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_HAL_DRV_Interface_I2C_PortConfig_t gDrvI2CPortConfigs[PTX_HAL_DRV_INTERFACE_I2C_NUMBER_PORTS] =
{
#if !defined(PTX_ENABLE_BOARD_EVK)
    {
        .Type                       = HAL_DRV_Interface_Port_Type_Default,
        .ID                         = 0,
        .Instance                   = &g_i2c0,
        .Irq                        = BSP_IO_PORT_04_PIN_14,
        .Channel                    = PTX_HAL_DRV_I2C_DEFAULT_CHANNEL,
        .TransferState              = PTX_HAL_DRV_I2C_RESET_VALUE,
        .TransferWaitCnt            = 0u,
        .ExternalIrqInstance        = &ptx_pmod2_irq,
    },
#else
    {
        .Type                       = HAL_DRV_Interface_Port_Type_Evk,
        .ID                         = 0,
        .Instance                   = &g_i2c0,
        .Irq                        = BSP_IO_PORT_00_PIN_15,
        .Channel                    = PTX_HAL_DRV_I2C_EVK_CHANNEL,
        .TransferState              = PTX_HAL_DRV_I2C_RESET_VALUE,
        .TransferWaitCnt            = 0u,
        .ExternalIrqInstance        = &ptx_pmod2_irq,
    },
#endif
};

extern void ptx_HAL_DRV_Interface_I2C_TransferCallback(i2c_master_callback_args_t *p_args);
void ptx_HAL_DRV_Interface_I2C_IrqCallback(external_irq_callback_args_t *p_args);

static ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetSlaveAddress(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t address);
static ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetTransmissionSpeed(ptx_HAL_DRV_Interface_I2C_t *ctx, uint32_t speed);
static ptx_Status_t ptx_HAL_DRV_Interface_I2C_Wait4TransferDone (ptx_HAL_DRV_Interface_I2C_t *ctx);


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
        memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_I2C_t));

        for (uint8_t config = 0; (PTX_HAL_DRV_INTERFACE_I2C_NUMBER_PORTS > config) && (PTX_STATUS_SUCCESS != status); ++config)
        {
            if ((gDrvI2CPortConfigs[config].Type == initParams->PortType) && (gDrvI2CPortConfigs[config].ID == initParams->PortID))
            {
                ctx->Port = &gDrvI2CPortConfigs[config];
                ctx->Speed = (0 == initParams->Speed) ? PTX_HAL_DRV_INTERFACE_I2C_SPEED_DEFAULT : initParams->Speed;
                ctx->Address = (0 == initParams->Address) ? PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4C : initParams->Address;
                ctx->Logger = initParams->Log;
                ctx->Gpio = initParams->Gpio;
                ctx->CompId = Comp_HAL;
                status = PTX_STATUS_SUCCESS;
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
        memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_I2C_t));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_Open(ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        const i2c_master_instance_t *i2c = ctx->Port->Instance;
        fsp_err_t r_status = R_SCI_I2C_Open(i2c->p_ctrl, i2c->p_cfg);

        if (FSP_ERR_ALREADY_OPEN == r_status)
        {
            /* Try to close the interface. */
            (void)R_SCI_I2C_Close(i2c->p_ctrl);
            if (NULL != ctx->Port->ExternalIrqInstance)
            {
                const external_irq_instance_t *irq = ctx->Port->ExternalIrqInstance;
                (void)R_ICU_ExternalIrqClose(irq->p_ctrl);
            }

            r_status = R_SCI_I2C_Open(i2c->p_ctrl, i2c->p_cfg);
        }

        if (FSP_SUCCESS == r_status)
        {
            /* Update I2C communication speed as FSP does not provide any api to change it
               unless it's manually specified in e2studio */
            sci_i2c_instance_ctrl_t *p_ctrl = (sci_i2c_instance_ctrl_t *) i2c->p_ctrl;

            switch (ctx->Speed)
            {
                case PTX_HAL_DRV_INTERFACE_I2C_SPEED_DEFAULT:
                {
                    /* default FSP configuration, do nothing */
                    break;
                }

                case PTX_HAL_DRV_INTERFACE_I2C_SPEED_FAST:
                {
                    /* Update specific register to set the desired rate to 400kHz
                     * Values are taken from FSP generated code */
                    p_ctrl->p_reg->MDDR = 131u;
                    p_ctrl->p_reg->BRR = 3u;
                    break;
                }

                case PTX_HAL_DRV_INTERFACE_I2C_SPEED_FASTPLUS:
                /* FALL THROUGH */
                case PTX_HAL_DRV_INTERFACE_I2C_SPEED_HIGHSPEED:
                /* FALL THROUGH */
                default:
                {
                    /* any other requested speed currently not supported by SCI Simple IIC Mode */
                    status = PTX_STATUS_E_GEN_INVALID_BITRATE;
                    break;
                }
            }

            if (status != PTX_STATUS_E_GEN_INVALID_BITRATE)
            {
                if (NULL != ctx->Port->ExternalIrqInstance)
                {
                    const external_irq_instance_t *irq = ctx->Port->ExternalIrqInstance;
                    r_status = R_ICU_ExternalIrqOpen(irq->p_ctrl, irq->p_cfg);

                    if (FSP_SUCCESS == r_status)
                    {
                        r_status = R_ICU_ExternalIrqEnable(irq->p_ctrl);
                    }

                    if (FSP_SUCCESS == r_status)
                    {
                        r_status = R_SCI_I2C_CallbackSet(i2c->p_ctrl, ptx_HAL_DRV_Interface_I2C_TransferCallback, ctx, NULL);
                    }

                    if (FSP_SUCCESS == r_status)
                    {
                        r_status = R_ICU_ExternalIrqCallbackSet(ctx->Port->ExternalIrqInstance->p_ctrl, ptx_HAL_DRV_Interface_I2C_IrqCallback, ctx, NULL);
                    }

                    if (FSP_SUCCESS == r_status)
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    else
                    {
                        status = PTX_STATUS_E_GEN_INTERFACE_OPEN;
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_OPEN;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_I2C_Open OK");
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_I2C_Open FAILED");
        }


    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_Close(ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        const i2c_master_instance_t *i2c = ctx->Port->Instance;

        fsp_err_t r_status = R_SCI_I2C_Close(i2c->p_ctrl);

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

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_I2C_Close OK");
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_I2C_Close FAILED");
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_Tx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *txBuf, uint32_t txLen, uint8_t restart)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != txBuf))
    {
        const i2c_master_instance_t *i2c = ctx->Port->Instance;

        ctx->Port->TransferState = TRX_TX_INPROGRESS;
        ctx->Port->TransferWaitCnt = INT_MAX;

        fsp_err_t r_status = R_SCI_I2C_Write(i2c->p_ctrl, txBuf, txLen, restart);

        if (FSP_SUCCESS == r_status)
        {
            status = ptx_HAL_DRV_Interface_I2C_Wait4TransferDone(ctx);
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_TX;
        }

#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I_MEM(ctx->Logger, &txBuf[0], txLen, "R_SCI_I2C_Write: ");
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_I2C_Write ERROR STATUS: %d", r_status);
        }
#endif
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_Rx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *rxBuf, uint32_t rxLen, uint8_t restart)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != rxBuf))
    {
        const i2c_master_instance_t *i2c = ctx->Port->Instance;

        ctx->Port->TransferState = TRX_RX_INPROGRESS;
        ctx->Port->TransferWaitCnt = INT_MAX;

        fsp_err_t r_status = R_SCI_I2C_Read(i2c->p_ctrl, rxBuf, rxLen, restart);

        if (FSP_SUCCESS == r_status)
        {
            status = ptx_HAL_DRV_Interface_I2C_Wait4TransferDone(ctx);
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_TX;
        }

#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I_MEM(ctx->Logger, &rxBuf[0], rxLen, "R_SCI_I2C_Read: ");
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_I2C_Read ERROR STATUS: %d", r_status);
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
        if (NULL != ctx->Port->ExternalIrqInstance)
        {
            const external_irq_instance_t *irq = ctx->Port->ExternalIrqInstance;
            fsp_err_t r_status = R_ICU_ExternalIrqEnable(irq->p_ctrl);

            if (FSP_SUCCESS == r_status)
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INTERFACE_RX_IRQ;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_DeactivateRxIrq(ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        if (NULL != ctx->Port->ExternalIrqInstance)
        {
            const external_irq_instance_t *irq = ctx->Port->ExternalIrqInstance;
            fsp_err_t r_status = R_ICU_ExternalIrqDisable(irq->p_ctrl);

            if (FSP_SUCCESS == r_status)
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INTERFACE_RX_IRQ;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_RxIrqLevel(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != level))
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

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_WaitForIRQ(ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;
    }

    return status;
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
                parameter->value.I2CSlaveAddress = ctx->Address;
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

    if ((NULL != ctx))
    {
        i2c_master_instance_t *i2c = (i2c_master_instance_t *)ctx->Port->Instance;

        fsp_err_t r_status = R_SCI_I2C_SlaveAddressSet(i2c->p_ctrl, address, I2C_MASTER_ADDR_MODE_7BIT);

        if (FSP_SUCCESS == r_status)
        {
            ctx->Address = address;
            status = PTX_STATUS_SUCCESS;
        }

        else
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetTransmissionSpeed(ptx_HAL_DRV_Interface_I2C_t *ctx, uint32_t speed)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (0 != speed))
    {
        uint32_t oldSpeed = ctx->Speed;
        ctx->Speed = speed;
        /* The call to Open first closes the port if it has already been opened.
         * Then, it re-opens the port and configures the new transmission speed.
         */
        status = ptx_HAL_DRV_Interface_I2C_Open(ctx);

        if (PTX_STATUS_SUCCESS != status)
        {
            ctx->Speed = oldSpeed;
        }
    }
    return status;
}

void ptx_HAL_DRV_Interface_I2C_TransferCallback(i2c_master_callback_args_t *p_args)
{
    if (NULL != p_args)
    {
        ptx_HAL_DRV_Interface_I2C_PortConfig_t *config = ((ptx_HAL_DRV_Interface_I2C_t *)p_args->p_context)->Port;
        switch (p_args->event)
        {
            case I2C_MASTER_EVENT_TX_COMPLETE:
            {
                /* Transfer Error */
                config->TransferState = TRX_TX_DONE;
                break;
            }
            case I2C_MASTER_EVENT_RX_COMPLETE:
            {
                /* Transfer Error */
                config->TransferState = TRX_RX_DONE;
                break;
            }
            default:
            {
                /* Transfer Error */
                config->TransferState = TRX_ERROR;
                break;
            }
        }
    }
}

void ptx_HAL_DRV_Interface_I2C_IrqCallback(external_irq_callback_args_t *p_args)
{
    /*
     * Callback is left empty intentionally!
     * All actions regarding the interrupt and the pin level are handled by the device
     * implementation! The ISR callback is just used to wake up the MCU from sleep states
     */
    (void)p_args;
}


static ptx_Status_t ptx_HAL_DRV_Interface_I2C_Wait4TransferDone (ptx_HAL_DRV_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        while ( (TRX_TX_DONE != ctx->Port->TransferState) && (TRX_RX_DONE != ctx->Port->TransferState) )
        {
            --ctx->Port->TransferWaitCnt;

            if (PTX_HAL_DRV_I2C_RESET_VALUE >= ctx->Port->TransferWaitCnt)
            {
                status = PTX_STATUS_E_GEN_INTERFACE_TIMEOUT;
                break;
            }
            else if (TRX_ERROR == ctx->Port->TransferState)
            {
                status = PTX_STATUS_E_GEN_TRANSFER_ABORTED;
                break;
            }
            else
            {
                /* Do nothing. Error already set. */
            }
        }

        if ( (TRX_TX_DONE == ctx->Port->TransferState) || (TRX_RX_DONE == ctx->Port->TransferState) )
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}
