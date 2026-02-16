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
    Module      : MCU-RA4M2 SPI Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_SPI.c

    Description : Implementation of the SPI Communication Interface Driver for the MCU-RA4M2 Reference Platform
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <limits.h>

#include "ptx_HAL_DRV_Interface_SPI.h"
#include "ptx_HAL_GPIO.h"
#include "hal_data.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_HAL_DRV_INTERFACE_SPI_RESET_VALUE                 ((int32_t)0u)
#define PTX_HAL_DRV_INTERFACE_SPI_DEFAULT_CHANNEL             (0u)
#define PTX_HAL_DRV_INTERFACE_SPI_EVK_CHANNEL                 (0u)
#define PTX_HAL_DRV_INTERFACE_SPI_NUMBER_PORTS                (1u)

#define PTX_HAL_DRV_INTERFACE_SPI_GPIO_MUX_SCI0_CONTROLLED    (IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_SCI0_2_4_6_8)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_HAL_DRV_Interface_SPI_PortConfig_t gDrvSpiPortConfigs[PTX_HAL_DRV_INTERFACE_SPI_NUMBER_PORTS] =
{
#if !defined(PTX_ENABLE_BOARD_EVK)
    {
        .Type                   = HAL_DRV_Interface_Port_Type_Default,
        .ID                     = 0,
        .Instance               = &ptx_pmod2_spi,
        .Mosi                   = BSP_IO_PORT_04_PIN_11,
        .Clock                  = BSP_IO_PORT_04_PIN_12,
        .Nss                    = BSP_IO_PORT_04_PIN_13,
        .Irq                    = BSP_IO_PORT_04_PIN_14,

        .Channel                = PTX_HAL_DRV_INTERFACE_SPI_DEFAULT_CHANNEL,
        .TransferState          = PTX_HAL_DRV_INTERFACE_SPI_RESET_VALUE,
        .TransferWaitCnt        = 0u,
        .ExternalIrqInstance    = &ptx_pmod2_irq,
    },
#else
    {
        .Type                   = HAL_DRV_Interface_Port_Type_Evk,
        .ID                     = 0,
        .Instance               = &ptx_pmod2_spi,
        .Mosi                   = BSP_IO_PORT_01_PIN_01,
        .Clock                  = BSP_IO_PORT_01_PIN_02,
        .Nss                    = BSP_IO_PORT_01_PIN_03,
        .Irq                    = BSP_IO_PORT_00_PIN_15,

        .Channel                = PTX_HAL_DRV_INTERFACE_SPI_EVK_CHANNEL,
        .TransferState          = PTX_HAL_DRV_INTERFACE_SPI_RESET_VALUE,
        .TransferWaitCnt        = 0u,
        .ExternalIrqInstance    = &ptx_pmod2_irq,
    }
#endif
};

extern void ptx_HAL_DRV_Interface_SPI_TransferCallback(spi_callback_args_t *p_args);
void ptx_HAL_DRV_Interface_SPI_IrqCallback(external_irq_callback_args_t *p_args);

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetTransmissionSpeed(ptx_HAL_DRV_Interface_SPI_t *ctx, uint32_t speed);
static ptx_Status_t ptx_HAL_DRV_Interface_SPI_Wait4TransferDone (ptx_HAL_DRV_Interface_SPI_t *ctx);

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_PreConfigurePins(ptx_HAL_DRV_Interface_SPI_t *ctx);
static ptx_Status_t ptx_HAL_DRV_Interface_SPI_DeConfigurePins(ptx_HAL_DRV_Interface_SPI_t *ctx);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Init (ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_SPI_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != ctx) && (NULL != initParams))
    {
        memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_SPI_t));

        for (uint8_t config = 0; (PTX_HAL_DRV_INTERFACE_SPI_NUMBER_PORTS > config) && (PTX_STATUS_SUCCESS != status); ++config)
        {
            if ((gDrvSpiPortConfigs[config].Type == initParams->PortType) && (gDrvSpiPortConfigs[config].ID == initParams->PortID))
            {
                ctx->Port = &gDrvSpiPortConfigs[config];
                ctx->Speed = (0 == initParams->Speed) ? PTX_HAL_DRV_INTERFACE_SPI_DEFAULT_SPEED : initParams->Speed;
                ctx->Logger = initParams->Log;
                ctx->Gpio = initParams->Gpio;
                ctx->CompId = Comp_HAL;
                status = PTX_STATUS_SUCCESS;
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
        memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_SPI_t));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Open(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (0 != ctx->Speed))
    {
        status = ptx_HAL_DRV_Interface_SPI_PreConfigurePins(ctx);

        if (PTX_STATUS_SUCCESS == status)
        {
            fsp_err_t r_status = R_SCI_SPI_Open(ctx->Port->Instance->p_ctrl, ctx->Port->Instance->p_cfg);

            if ((FSP_ERR_ALREADY_OPEN == r_status) || (FSP_ERR_IN_USE == r_status))
            {
                /* If the interface is already opened, close it first and then try to re-open. */
                (void) R_SCI_SPI_Close(ctx->Port->Instance->p_ctrl);
                if (NULL != ctx->Port->ExternalIrqInstance)
                {
                    (void) R_ICU_ExternalIrqClose(ctx->Port->ExternalIrqInstance->p_ctrl);
                }

                /* Now try to re-open it. The only acceptable return value now is FSP_SUCCESS.  */
                r_status = R_SCI_SPI_Open(ctx->Port->Instance->p_ctrl, ctx->Port->Instance->p_cfg);
            }

            if (FSP_SUCCESS == r_status)
            {
                /* Interface successfully opened. */
                if (0 != ctx->Speed)
                {
                    sci_spi_div_setting_t baudrate_setting;
                    memset(&baudrate_setting, 0, sizeof(sci_spi_div_setting_t));

                    r_status = R_SCI_SPI_CalculateBitrate(ctx->Speed, &baudrate_setting, false);
                    /*
                    * Set the SFRs, which are responsible for generating the desired clock frequency AFTER R_SCI_SPI_Open().
                    */
                    sci_spi_instance_ctrl_t *sci_reg_intfc = ((sci_spi_instance_ctrl_t *) ctx->Port->Instance->p_ctrl);
                    sci_reg_intfc->p_reg->SMR = (uint8_t) (R_SCI0_SMR_CM_Msk | (uint8_t) (baudrate_setting.cks << R_SCI0_SMR_CKS_Pos));
                    sci_reg_intfc->p_reg->BRR = baudrate_setting.brr;
                    sci_reg_intfc->p_reg->MDDR = baudrate_setting.mddr;
                }

                if (FSP_SUCCESS == r_status)
                {

                    if (NULL != ctx->Port->ExternalIrqInstance)
                    {
                        r_status = R_ICU_ExternalIrqOpen(ctx->Port->ExternalIrqInstance->p_ctrl, ctx->Port->ExternalIrqInstance->p_cfg);

                        if (FSP_SUCCESS == r_status)
                        {
                            r_status = R_SCI_SPI_CallbackSet(ctx->Port->Instance->p_ctrl, ptx_HAL_DRV_Interface_SPI_TransferCallback, ctx, NULL);
                        }

                        if (FSP_SUCCESS == r_status)
                        {
                            r_status = R_ICU_ExternalIrqCallbackSet(ctx->Port->ExternalIrqInstance->p_ctrl, ptx_HAL_DRV_Interface_SPI_IrqCallback, ctx, NULL);
                        }

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
            else
            {
                status = PTX_STATUS_E_GEN_INTERFACE_OPEN;
            }

        }

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_SPI_Open OK");
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_SPI_Open FAILED");
        }
    }
    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Close(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        fsp_err_t r_status = R_SCI_SPI_Close(ctx->Port->Instance->p_ctrl);

        if (((FSP_SUCCESS == r_status) || (FSP_ERR_NOT_OPEN == r_status)) && (NULL != ctx->Port->ExternalIrqInstance))
        {
            r_status = R_ICU_ExternalIrqClose(ctx->Port->ExternalIrqInstance->p_ctrl);
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
            status = ptx_HAL_DRV_Interface_SPI_DeConfigurePins(ctx);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_SPI_Close OK");
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_SPI_Close FAILED");
        }

    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Tx(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *txBuf, uint32_t txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != txBuf))
    {
        ctx->Port->TransferState = PTX_HAL_DRV_INTERFACE_SPI_RESET_VALUE;
        ctx->Port->TransferWaitCnt = INT_MAX;

        fsp_err_t r_status = R_SCI_SPI_Write(ctx->Port->Instance->p_ctrl, &txBuf[0], txLen, SPI_BIT_WIDTH_8_BITS);

        if (FSP_SUCCESS == r_status)
        {
            status = ptx_HAL_DRV_Interface_SPI_Wait4TransferDone(ctx);
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_TX;
        }

#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I_MEM(ctx->Logger, &txBuf[0], txLen, "R_SCI_SPI_Write: ");
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_SPI_Write ERROR STATUS: %d", r_status);
        }
#endif
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Rx(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *rxBuf, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != rxBuf))
    {
        ctx->Port->TransferState = PTX_HAL_DRV_INTERFACE_SPI_RESET_VALUE;
        ctx->Port->TransferWaitCnt = INT_MAX;

        fsp_err_t r_status = R_SCI_SPI_Read(ctx->Port->Instance->p_ctrl, &rxBuf[0], rxLen, SPI_BIT_WIDTH_8_BITS);

        if (FSP_SUCCESS == r_status)
        {
            status = ptx_HAL_DRV_Interface_SPI_Wait4TransferDone(ctx);
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_RX;
        }

#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I_MEM(ctx->Logger, &rxBuf[0], rxLen, "R_SCI_SPI_Read: ");
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "R_SCI_SPI_Read ERROR STATUS: %d", r_status);
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
        fsp_err_t r_status = R_IOPORT_PinWrite(ctx->Gpio->Drv.Port->Instance->p_ctrl, ctx->Port->Nss, BSP_IO_LEVEL_LOW);
        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_NSS;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_ResetNss(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        fsp_err_t r_status = R_IOPORT_PinWrite(ctx->Gpio->Drv.Port->Instance->p_ctrl, ctx->Port->Nss, BSP_IO_LEVEL_HIGH);
        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_NSS;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_ActivateRxIrq(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        fsp_err_t r_status = R_ICU_ExternalIrqEnable(ctx->Port->ExternalIrqInstance->p_ctrl);
        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_RX_IRQ;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_DeactivateRxIrq(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        fsp_err_t r_status = R_ICU_ExternalIrqDisable(ctx->Port->ExternalIrqInstance->p_ctrl);
        if (FSP_SUCCESS == r_status)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_RX_IRQ;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_RxIrqLevel(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
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

ptx_Status_t ptx_HAL_DRV_Interface_SPI_WaitForIRQ(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;
    }

    return status;
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
        uint32_t oldSpeed = ctx->Speed;
        ctx->Speed = speed;
        /* The call to Open first closes the port if it has already been opened.
         * Then, it re-opens the port and configures the new transmission speed.
         */
        status = ptx_HAL_DRV_Interface_SPI_Open(ctx);

        if (PTX_STATUS_SUCCESS != status)
        {
            ctx->Speed = oldSpeed;
        }
    }

    return status;
}

void ptx_HAL_DRV_Interface_SPI_TransferCallback(spi_callback_args_t *p_args)
{
    if (NULL != p_args)
    {
        ptx_HAL_DRV_Interface_SPI_PortConfig_t *config = ((ptx_HAL_DRV_Interface_SPI_t *)p_args->p_context)->Port;
        if (config->Channel == p_args->channel)
        {
            switch (p_args->event)
            {
                case SPI_EVENT_TRANSFER_COMPLETE:
                    config->TransferState = SPI_EVENT_TRANSFER_COMPLETE;
                    break;

                default:
                    /* Transfer Error */
                    config->TransferState = SPI_EVENT_TRANSFER_ABORTED;
                    break;
            }

        }
        else
        {
            /* Transfer Error */
            config->TransferState = SPI_EVENT_TRANSFER_ABORTED;
        }
    }
}

void ptx_HAL_DRV_Interface_SPI_IrqCallback(external_irq_callback_args_t *p_args)
{
    /*
     * Callback is left empty intentionally!
     * All actions regarding the interrupt and the pin level are handled by the device
     * implementation! The ISR callback is just used to wake up the MCU from sleep states
     */
    (void)p_args;
}

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_Wait4TransferDone (ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INTERNAL;

    while (SPI_EVENT_TRANSFER_COMPLETE != ctx->Port->TransferState)
    {
        --ctx->Port->TransferWaitCnt;

        if (PTX_HAL_DRV_INTERFACE_SPI_RESET_VALUE >= ctx->Port->TransferWaitCnt)
        {
            status = PTX_STATUS_E_GEN_INTERFACE_TIMEOUT;
            break;
        }
        else if (SPI_EVENT_TRANSFER_ABORTED == ctx->Port->TransferState)
        {
            status = PTX_STATUS_E_GEN_TRANSFER_ABORTED;
            break;
        }
        else
        {
            /* Do nothing. Error already set. */
        }
    }

    if (SPI_EVENT_TRANSFER_COMPLETE == ctx->Port->TransferState)
    {
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_PreConfigurePins(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {

        fsp_err_t r_status = R_IOPORT_PinCfg(ctx->Gpio->Drv.Port->Instance->p_ctrl, ctx->Port->Mosi, PTX_HAL_DRV_INTERFACE_SPI_GPIO_MUX_SCI0_CONTROLLED);

        if (FSP_SUCCESS == r_status)
        {
            r_status = R_IOPORT_PinCfg(ctx->Gpio->Drv.Port->Instance->p_ctrl, ctx->Port->Clock, PTX_HAL_DRV_INTERFACE_SPI_GPIO_MUX_SCI0_CONTROLLED);
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

    return status;
}

static ptx_Status_t ptx_HAL_DRV_Interface_SPI_DeConfigurePins(ptx_HAL_DRV_Interface_SPI_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {

        fsp_err_t r_status = R_IOPORT_PinCfg(ctx->Gpio->Drv.Port->Instance->p_ctrl, ctx->Port->Mosi, IOPORT_CFG_PORT_DIRECTION_INPUT);

        if (FSP_SUCCESS == r_status)
        {
            r_status = R_IOPORT_PinCfg(ctx->Gpio->Drv.Port->Instance->p_ctrl, ctx->Port->Clock, IOPORT_CFG_PORT_DIRECTION_INPUT);
        }

        if (FSP_SUCCESS == r_status)
        {
            r_status = R_IOPORT_PinCfg(ctx->Gpio->Drv.Port->Instance->p_ctrl, ctx->Port->Nss, IOPORT_CFG_PORT_DIRECTION_INPUT);
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

    return status;
}
