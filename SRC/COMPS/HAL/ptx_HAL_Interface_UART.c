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
    Module      : HAL Communication Interface for UART
    File        : ptx_HAL_Interface_UART.c

    Description : Implementation of the UART Communication Interface
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_Interface_UART.h"
#include "ptx_HAL_Interface.h"

#include <string.h>


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/* Macro to check CB function pointer */
#define PTX_DEVUART_FP_CHECK(ptx_devUart_fp)    ((NULL != uartInterface->Drv) && (NULL != uartInterface->Drv->ptx_devUart_fp))

#define PTX_HAL_INTERFACE_UART_IRQ_LEVEL_HIGH  (1u)

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

ptx_Status_t ptx_HAL_Interface_UART_Init (ptx_HAL_Interface_UART_t *ctx, ptx_HAL_Interface_UART_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams))
    {
        memset(&ctx->Drv, 0, sizeof(ptx_HAL_DRV_Interface_UART_t));

        ptx_HAL_DRV_Interface_UART_InitParams_t    drv_init_params;
        memset(&drv_init_params, 0, sizeof(ptx_HAL_DRV_Interface_UART_InitParams_t));
        drv_init_params.InterfaceName = initParams->InterfaceName;
        drv_init_params.InterfaceNameLen = initParams->InterfaceNameLen;
        drv_init_params.PortType = initParams->PortType;
        drv_init_params.PortID = initParams->PortID;
        drv_init_params.Speed = initParams->Speed;
        drv_init_params.Log = initParams->Log;
        drv_init_params.Gpio = initParams->Gpio;
#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
        drv_init_params.OS = initParams->OS;
#endif

        status = ptx_HAL_DRV_Interface_UART_Init(&ctx->Drv, &drv_init_params);
        if (PTX_STATUS_SUCCESS == status)
        {
            /** Interface speed to be used */
            ctx->CfgBaudrate = drv_init_params.Speed; /* speed is set inside of DrvUartInit */
            ctx->CompId = Comp_HAL;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_DeInit (ptx_HAL_Interface_UART_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        status = ptx_HAL_DRV_Interface_UART_DeInit(&ctx->Drv);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_Open (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL))
    {
        status = ptx_HAL_DRV_Interface_UART_Open(&uartInterface->Drv);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_Close (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL))
    {
        status = ptx_HAL_DRV_Interface_UART_Close(&uartInterface->Drv);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_TRx (void *ctx, uint8_t *txBuf[], uint32_t txLen[], uint32_t txBufferCnt,
                                         uint8_t *rxBuf[], uint32_t rxLen[], uint32_t rxBufferCnt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL))
    {
        status = PTX_STATUS_SUCCESS;

        if ((NULL != txBuf) && (NULL != txLen) && (0 != txBufferCnt))
        {
            /* TX operation is required and parameters are valid. */
            uint32_t txIdx = 0;

            while ((PTX_STATUS_SUCCESS == status) && (txIdx < txBufferCnt))
            {
                if (NULL != txBuf[txIdx])
                {
                    status = ptx_HAL_DRV_Interface_UART_Tx(&uartInterface->Drv, txBuf[txIdx], txLen[txIdx]);
                }
                else
                {
                    /* Buffer is not valid */
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
                }

                txIdx++;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if ((NULL != rxBuf) && (NULL != rxLen) && (0 != rxBufferCnt))
            {
                /* RX operation is required and parameters are valid. */
                uint32_t rxIdx = 0;

                while ((PTX_STATUS_SUCCESS == status) && (rxIdx < rxBufferCnt))
                {
                    if (NULL != rxBuf[rxIdx])
                    {
                        status = ptx_HAL_DRV_Interface_UART_Rx(&uartInterface->Drv, rxBuf[rxIdx], rxLen[rxIdx]);
                    }
                    else
                    {
                        /* Buffer is not valid */
                        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                        break;
                    }

                    rxIdx++;
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_Tx (void *ctx, uint8_t *txBuf, uint32_t txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL) && (NULL != txBuf))
    {
        status = ptx_HAL_DRV_Interface_UART_Tx(&uartInterface->Drv, txBuf, txLen);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_Rx (void *ctx, uint8_t *rxBuf, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL) && (NULL != rxBuf))
    {
        status = ptx_HAL_DRV_Interface_UART_Rx(&uartInterface->Drv, &rxBuf[0], rxLen);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_EnableAsyncRx (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL))
    {
        status = ptx_HAL_DRV_Interface_UART_ActivateRxIrq(&uartInterface->Drv);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_DisableAsyncRx (void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL))
    {
        status = ptx_HAL_DRV_Interface_UART_DeactivateRxIrq(&uartInterface->Drv);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_RegAsyncRxCb(void *ctx, ptx_HAL_Interface_UART_AsyncRxCb_t rxCb, ptx_HAL_Interface_UART_AsyncRxCbCtx_t rxCbCtx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL) && (NULL != rxCb) && (NULL != rxCbCtx))
    {
        uartInterface->AsyncRxCb       = rxCb;
        uartInterface->AsyncRxCbCtx    = rxCbCtx;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_RxPending(void* ctx, uint8_t *rxPending)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL) && (NULL != rxPending))
    {
        status = ptx_HAL_DRV_Interface_UART_RxIrqLevel(&uartInterface->Drv, rxPending);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_AsyncRxNonBlocking(void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL))
    {
        uint8_t level = 0;

        status = ptx_HAL_DRV_Interface_UART_RxIrqLevel(&uartInterface->Drv, &level);

        if ((PTX_STATUS_SUCCESS == status) && (PTX_HAL_INTERFACE_UART_IRQ_LEVEL_HIGH == level))
        {
            if ((NULL != uartInterface->AsyncRxCb) && (NULL != uartInterface->AsyncRxCbCtx))
            {
                uartInterface->AsyncRxCb(uartInterface->AsyncRxCbCtx);
            }
            else
            {
                /* No AsyncRx callback registered */
                status = PTX_STATUS_E_GEN_INTERNAL;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_AsyncRxBlocking(void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL))
    {
        status = ptx_HAL_DRV_Interface_UART_WaitForIRQ(&uartInterface->Drv);

        if (PTX_STATUS_SUCCESS == status)
        {
            if ((NULL != uartInterface->AsyncRxCb) && (NULL != uartInterface->AsyncRxCbCtx))
            {
                uartInterface->AsyncRxCb(uartInterface->AsyncRxCbCtx);
            }
            else
            {
                /* No AsyncRx callback registered */
                status = PTX_STATUS_E_GEN_INTERNAL;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_SetParameter(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL))
    {
        status = ptx_HAL_DRV_Interface_UART_SetParameter(&uartInterface->Drv, parameter);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_UART_GetParameter(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_UART_t *uartInterface = &((ptx_HAL_Interface_t*)ctx)->UartInterface;

    if (PTX_COMP_CHECK(uartInterface, Comp_HAL))
    {
        status = ptx_HAL_DRV_Interface_UART_GetParameter(&uartInterface->Drv, parameter);
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
