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
    Module      : HAL Communication Interface
    File        : ptx_HAL_Interface.c

    Description : Implementation of the HAL Communication Interface for physical interfaces.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <string.h>
#include "ptx_HAL_Interface.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

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

ptx_Status_t ptx_HAL_Interface_Init(ptx_HAL_Interface_t *ctx, ptx_HAL_Interface_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams))
    {
#if defined(PTX_ENABLE_SPI)

        if (HAL_Interface_Type_SPI == initParams->Type)
        {
            memset(&ctx->SpiInterface, 0, sizeof(ptx_HAL_Interface_SPI_t));

            ptx_HAL_Interface_SPI_InitParams_t interface_init_params;
            memset(&interface_init_params, 0, sizeof(ptx_HAL_Interface_SPI_InitParams_t));
            interface_init_params.PortType = initParams->PortType;
            interface_init_params.PortID = initParams->PortID;
            interface_init_params.Speed = initParams->Speed;
            interface_init_params.Log = initParams->Log;
            interface_init_params.Gpio = initParams->Gpio;
#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
            interface_init_params.OS = initParams->Osal;
#endif

            status = ptx_HAL_Interface_SPI_Init(&ctx->SpiInterface, &interface_init_params);

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->Type = HAL_Interface_Type_SPI;

                ctx->Open               = ptx_HAL_Interface_SPI_Open;
                ctx->Close              = ptx_HAL_Interface_SPI_Close;
                ctx->TRx                = ptx_HAL_Interface_SPI_TRx;
                ctx->Tx                 = NULL;
                ctx->Rx                 = NULL;
                ctx->EnableAsyncRx      = ptx_HAL_Interface_SPI_EnableAsyncRx;
                ctx->DisableAsyncRx     = ptx_HAL_Interface_SPI_DisableAsyncRx;
                ctx->RegAsyncRxCb       = ptx_HAL_Interface_SPI_RegAsyncRxCb;
                ctx->AsyncRxNoNBlocking = ptx_HAL_Interface_SPI_AsyncRxNoNBlocking;
                ctx->AsyncRxBlocking    = ptx_HAL_Interface_SPI_AsyncRxBlocking;
                ctx->RxPending          = ptx_HAL_Interface_SPI_RxPending;
                ctx->SetParameter       = ptx_HAL_Interface_SPI_SetParameter;
                ctx->GetParameter       = ptx_HAL_Interface_SPI_GetParameter;
            }

        }
#endif // PTX_ENABLE_SPI


#if defined(PTX_ENABLE_UART)

        if (HAL_Interface_Type_UART == initParams->Type)
        {

            memset(&ctx->UartInterface, 0, sizeof(ptx_HAL_Interface_UART_t));

            ptx_HAL_Interface_UART_InitParams_t interface_init_params;
            memset(&interface_init_params, 0, sizeof(ptx_HAL_Interface_UART_InitParams_t));
            interface_init_params.InterfaceName = initParams->Name;
            interface_init_params.InterfaceNameLen = initParams->NameLen;
            interface_init_params.PortType = initParams->PortType;
            interface_init_params.PortID = initParams->PortID;
            interface_init_params.Log = initParams->Log;
            interface_init_params.Gpio = initParams->Gpio;
            interface_init_params.Speed = initParams->Speed;
#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
            interface_init_params.OS = initParams->Osal;
#endif
            status = ptx_HAL_Interface_UART_Init(&ctx->UartInterface, &interface_init_params);

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->Type = HAL_Interface_Type_UART;

                ctx->Open               = ptx_HAL_Interface_UART_Open;
                ctx->Close              = ptx_HAL_Interface_UART_Close;
                ctx->TRx                = ptx_HAL_Interface_UART_TRx;
                ctx->Tx                 = ptx_HAL_Interface_UART_Tx;
                ctx->Rx                 = ptx_HAL_Interface_UART_Rx;
                ctx->EnableAsyncRx      = ptx_HAL_Interface_UART_EnableAsyncRx;
                ctx->DisableAsyncRx     = ptx_HAL_Interface_UART_DisableAsyncRx;
                ctx->RegAsyncRxCb       = ptx_HAL_Interface_UART_RegAsyncRxCb;
                ctx->AsyncRxNoNBlocking = ptx_HAL_Interface_UART_AsyncRxNonBlocking;
                ctx->AsyncRxBlocking    = ptx_HAL_Interface_UART_AsyncRxBlocking;
                ctx->RxPending          = ptx_HAL_Interface_UART_RxPending;
                ctx->SetParameter       = ptx_HAL_Interface_UART_SetParameter;
                ctx->GetParameter       = ptx_HAL_Interface_UART_GetParameter;
            }
        }


#endif // PTX_ENABLE_UART
#if defined(PTX_ENABLE_I2C)
        if (HAL_Interface_Type_I2C == initParams->Type)
        {
            memset(&ctx->I2cInterface, 0, sizeof(ptx_HAL_Interface_I2C_t));

            ptx_HAL_Interface_I2C_InitParams_t interface_init_params;
            memset(&interface_init_params, 0, sizeof(ptx_HAL_Interface_I2C_InitParams_t));
            interface_init_params.PortType = initParams->PortType;
            interface_init_params.PortID = initParams->PortID;
            interface_init_params.Log = initParams->Log;
            interface_init_params.Gpio = initParams->Gpio;
            interface_init_params.Speed = initParams->Speed;
#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
            interface_init_params.OS = initParams->Osal;
#endif

            status = ptx_HAL_Interface_I2C_Init(&ctx->I2cInterface, &interface_init_params);

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->Type = HAL_Interface_Type_I2C;

                ctx->Open               = ptx_HAL_Interface_I2C_Open;
                ctx->Close              = ptx_HAL_Interface_I2C_Close;
                ctx->TRx                = ptx_HAL_Interface_I2C_TRx;
                ctx->Tx                 = NULL;
                ctx->Rx                 = NULL;
                ctx->EnableAsyncRx      = ptx_HAL_Interface_I2C_EnableAsyncRx;
                ctx->DisableAsyncRx     = ptx_HAL_Interface_I2C_DisableAsyncRx;
                ctx->RegAsyncRxCb       = ptx_HAL_Interface_I2C_RegAsyncRxCb;
                ctx->AsyncRxNoNBlocking = ptx_HAL_Interface_I2C_AsyncRxNonBlocking;
                ctx->AsyncRxBlocking    = ptx_HAL_Interface_I2C_AsyncRxBlocking;
                ctx->RxPending          = ptx_HAL_Interface_I2C_RxPending;
                ctx->SetParameter       = ptx_HAL_Interface_I2C_SetParameter;
                ctx->GetParameter       = ptx_HAL_Interface_I2C_GetParameter;
            }
        }
#endif // PTX_ENABLE_I2C
#if defined(PTX_ENABLE_I3C)
        if (HAL_Interface_Type_I3C == initParams->Type)
        {
            memset(&ctx->I3cInterface, 0, sizeof(ptx_HAL_Interface_I3C_t));

            ptx_HAL_Interface_I3C_InitParams_t interface_init_params;
            memset(&interface_init_params, 0, sizeof(ptx_HAL_Interface_I3C_InitParams_t));
            interface_init_params.PortType = initParams->PortType;
            interface_init_params.PortID = initParams->PortID;
            interface_init_params.Log = initParams->Log;
            interface_init_params.Gpio = initParams->Gpio;
            interface_init_params.Speed = initParams->Speed;
#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX)
            interface_init_params.OS = initParams->Osal;
#endif

            status = ptx_HAL_Interface_I3C_Init(&ctx->I3cInterface, &interface_init_params);

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->Type = HAL_Interface_Type_I3C;

                ctx->Open               = ptx_HAL_Interface_I3C_Open;
                ctx->Close              = ptx_HAL_Interface_I3C_Close;
                ctx->TRx                = ptx_HAL_Interface_I3C_TRx;
                ctx->Tx                 = NULL;
                ctx->Rx                 = NULL;
                ctx->EnableAsyncRx      = ptx_HAL_Interface_I3C_EnableAsyncRx;
                ctx->DisableAsyncRx     = ptx_HAL_Interface_I3C_DisableAsyncRx;
                ctx->RegAsyncRxCb       = ptx_HAL_Interface_I3C_RegAsyncRxCb;
                ctx->AsyncRxNoNBlocking = ptx_HAL_Interface_I3C_AsyncRxNonBlocking;
                ctx->AsyncRxBlocking    = ptx_HAL_Interface_I3C_AsyncRxBlocking;
                ctx->RxPending          = ptx_HAL_Interface_I3C_RxPending;
                ctx->SetParameter       = ptx_HAL_Interface_I3C_SetParameter;
                ctx->GetParameter       = ptx_HAL_Interface_I3C_GetParameter;
            }
        }
#endif // PTX_ENABLE_I3C

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->CompId = Comp_HAL;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_DeInit (ptx_HAL_Interface_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
#if defined(PTX_ENABLE_SPI)
        if (HAL_Interface_Type_SPI == ctx->Type)
        {
            status = ptx_HAL_Interface_SPI_DeInit(&ctx->SpiInterface);
        }
#endif

#if defined(PTX_ENABLE_UART)
        if (HAL_Interface_Type_UART == ctx->Type)
        {
            status = ptx_HAL_Interface_UART_DeInit(&ctx->UartInterface);
        }
#endif

#if defined(PTX_ENABLE_I2C)
        if (HAL_Interface_Type_I2C == ctx->Type)
        {
            status = ptx_HAL_Interface_I2C_DeInit(&ctx->I2cInterface);
        }
#endif

#if defined(PTX_ENABLE_I3C)
        if (HAL_Interface_Type_I3C == ctx->Type)
        {
            status = ptx_HAL_Interface_I3C_DeInit(&ctx->I3cInterface);
        }
#endif

        if (PTX_STATUS_SUCCESS == status)
        {
            memset(ctx, 0, sizeof(ptx_HAL_Interface_t));
            ctx->CompId = Comp_None;
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
