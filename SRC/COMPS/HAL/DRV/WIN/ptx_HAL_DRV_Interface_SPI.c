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
    Module      : Windows SPI Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_SPI.c

    Description : Implementation of the SPI Communication Interface Driver for the Windows Reference Platform using FTDI
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_DRV_Interface_SPI.h"


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_HAL_DRV_INTERFACE_SPI_NUMBER_PORTS        (1u)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 * Default Port Configuration for FT232H module
 * One is supported, extension requires change of MODULE_DESCRIPTOR handling and gDrvSPIPortConfigs.
 */
static ptx_HAL_DRV_Interface_SPI_PortConfig_t gDrvSPIPortConfigs[PTX_HAL_DRV_INTERFACE_SPI_NUMBER_PORTS] =
{
    {
        .Type               = HAL_DRV_Interface_Port_Type_Default,
        .ID                 = 0,
        .IRQPin_Low         = FT_IRQ_LINE_GPIO_BIT_LO,  /* SPI Default IRQ pin low byte, Pin D4 */
        .IRQPin_High        = FT_IRQ_LINE_GPIO_BIT,      /* SPI Default IRQ pin high byte, Pin C0 */
        .ModuleDescriptor   = MODULE_DESCRIPTOR
    }
};

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
            if ((gDrvSPIPortConfigs[config].Type == initParams->PortType) && (gDrvSPIPortConfigs[config].ID == initParams->PortID))
            {
                status = ptx_HAL_FTD2XX_LibOpen((void**)&ctx->Ftd2xx_ctx, gDrvSPIPortConfigs[config].ModuleDescriptor, NULL);
                if (PTX_STATUS_SUCCESS == status)
                {
                    ctx->Port = &gDrvSPIPortConfigs[config];
                    ctx->Speed = (0 == initParams->Speed) ? PTX_HAL_DRV_INTERFACE_SPI_DEFAULT_SPEED : initParams->Speed;
                    ctx->Log = initParams->Log;
                    ctx->OS = (ptx_OSAL_t*)initParams->OS;
                    ctx->Ftd2xx_ctx->IRQPin_Low = ctx->Port->IRQPin_Low;
                    ctx->Ftd2xx_ctx->IRQPin_High = ctx->Port->IRQPin_High;
                    ctx->Ftd2xx_ctx->Log = initParams->Log;
                    ctx->CompId = Comp_HAL;
                }

            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_DeInit (ptx_HAL_DRV_Interface_SPI_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = ptx_HAL_FTD2XX_LibClose(ctx->Ftd2xx_ctx);
        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->Ftd2xx_ctx = NULL;
            memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_SPI_t));
            ctx->CompId = Comp_None;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Open(ptx_HAL_DRV_Interface_SPI_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = ptx_HAL_FTD2XX_SPI_Open(ctx->Ftd2xx_ctx, ctx->Speed);

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I (ctx->Log, "SPI Open OK on FTDI: %s", ctx->Port->ModuleDescriptor);
        }
        else
        {
            PTX_LOG_I (ctx->Log, "SPI Open FAILED");
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Close(ptx_HAL_DRV_Interface_SPI_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = ptx_HAL_FTD2XX_SPI_Close(ctx->Ftd2xx_ctx);

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I (ctx->Log, "SPI Close OK on FTDI: %s", ctx->Port->ModuleDescriptor);
        }
        else
        {
            PTX_LOG_I (ctx->Log, "SPI Close FAILED");
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Tx(ptx_HAL_DRV_Interface_SPI_t* ctx, uint8_t *txBuf, uint32_t txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = ptx_HAL_FTD2XX_SPI_TRx(ctx->Ftd2xx_ctx, txBuf, &txLen, NULL, 0);

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

ptx_Status_t ptx_HAL_DRV_Interface_SPI_Rx(ptx_HAL_DRV_Interface_SPI_t* ctx, uint8_t *rxBuf, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = ptx_HAL_FTD2XX_SPI_TRx(ctx->Ftd2xx_ctx, 0, NULL, rxBuf, &rxLen);

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

ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetNss(ptx_HAL_DRV_Interface_SPI_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = ptx_HAL_FTD2XX_SPICSEnable(ctx->Ftd2xx_ctx);
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_ResetNss(ptx_HAL_DRV_Interface_SPI_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = ptx_HAL_FTD2XX_SPICSDisable(ctx->Ftd2xx_ctx);
    }
    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_ActivateRxIrq(ptx_HAL_DRV_Interface_SPI_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = PTX_STATUS_SUCCESS;
    }
    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_DeactivateRxIrq(ptx_HAL_DRV_Interface_SPI_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = PTX_STATUS_SUCCESS;
    }
    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_RxIrqLevel(ptx_HAL_DRV_Interface_SPI_t* ctx, uint8_t *level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint16_t level_16 = 0;
    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = ptx_HAL_FTD2XX_GetIRQLine(ctx->Ftd2xx_ctx, &level_16);
        *level = (0 == level_16) ? 0 : 1;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_WaitForIRQ(ptx_HAL_DRV_Interface_SPI_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetParameter(ptx_HAL_DRV_Interface_SPI_t* ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
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

ptx_Status_t ptx_HAL_DRV_Interface_SPI_GetParameter(ptx_HAL_DRV_Interface_SPI_t* ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
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

        status = ptx_HAL_DRV_Interface_SPI_Open(ctx);

        if (PTX_STATUS_SUCCESS != status)
        {
            ctx->Speed = oldSpeed;
        }
    }

    return status;
}