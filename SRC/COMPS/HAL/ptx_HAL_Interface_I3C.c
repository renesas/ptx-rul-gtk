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
    Module      : HAL Communication Interface for I3C
    File        : ptx_HAL_Interface_I3C.c

    Description : Default Implementation of I3C Device
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_Interface_I3C.h"
#include "ptx_HAL_Interface.h"

#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_HAL_INTERFACE_I3C_IRQ_LEVEL_HIGH  (1u)

#define PTX_DEVI3C_FP_CHECK(ptx_devI3C_fp) ((NULL != i3cInterface->Drv) && (NULL != i3cInterface->Drv->ptx_devI3C_fp))

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

ptx_Status_t ptx_HAL_Interface_I3C_Init(ptx_HAL_Interface_I3C_t *ctx, ptx_HAL_Interface_I3C_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams) && (NULL != initParams->Log))
    {        
        PTX_LOG_V(initParams->Log, "ptx_HAL_Interface_I3C_Init");

        memset(&ctx->Drv, 0, sizeof(ptx_HAL_DRV_Interface_I3C_t));

        ptx_HAL_DRV_Interface_I3C_InitParams_t drv_init_params;
        memset(&drv_init_params, 0, sizeof(ptx_HAL_DRV_Interface_I3C_InitParams_t));
        drv_init_params.PortType = initParams->PortType;
        drv_init_params.PortID = initParams->PortID;
        drv_init_params.Speed = initParams->Speed;
        drv_init_params.Log = initParams->Log;
        drv_init_params.Gpio = initParams->Gpio;
        ctx->Log = initParams->Log;
#if defined(PTX_APP_DIVERSITY_WINDOWS) || defined(PTX_APP_DIVERSITY_LINUX)
        drv_init_params.OS = initParams->OS;
#endif

        status = ptx_HAL_DRV_Interface_I3C_Init(&ctx->Drv, &drv_init_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->CompId = Comp_HAL;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I3C_DeInit(ptx_HAL_Interface_I3C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {      
        PTX_LOG_V(ctx->Log, "ptx_HAL_Interface_I3C_DeInit");

        status = ptx_HAL_DRV_Interface_I3C_DeInit(&ctx->Drv);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I3C_Open(void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_I3C_t *i3cInterface = &((ptx_HAL_Interface_t*)ctx)->I3cInterface;

    if (PTX_COMP_CHECK(i3cInterface, Comp_HAL))
    {  
        PTX_LOG_V(i3cInterface->Log, "ptx_HAL_Interface_I3C_Open");

        status = ptx_HAL_DRV_Interface_I3C_Open(&i3cInterface->Drv);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I3C_Close(void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_I3C_t *i3cInterface = &((ptx_HAL_Interface_t*)ctx)->I3cInterface;

    if (PTX_COMP_CHECK(i3cInterface, Comp_HAL))
    { 
        PTX_LOG_V(i3cInterface->Log, "ptx_HAL_Interface_I3C_Close");

        status = ptx_HAL_DRV_Interface_I3C_Close(&i3cInterface->Drv);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I3C_TRx(void *ctx, uint8_t *txBuf[], uint32_t txLen[], uint32_t txBufferCnt,
                                        uint8_t *rxBuf[], uint32_t rxLen[], uint32_t rxBufferCnt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    bool txReq = true;
    bool rxReq = true;

    ptx_HAL_Interface_I3C_t *i3cInterface = &((ptx_HAL_Interface_t*)ctx)->I3cInterface;

    if (PTX_COMP_CHECK(i3cInterface, Comp_HAL))
    {
        PTX_LOG_V(i3cInterface->Log, "ptx_HAL_Interface_I3C_TRx");

        /* Check if TX Request was not wanted (eg all parameters are NULL / 0) */
        if ((NULL == txBuf) && (NULL == txLen) && (0 == txBufferCnt))
        {
            txReq = false;
        }

        /* Check if RX Request was not wanted (eg all parameters are NULL / 0) */
        if ((NULL == rxBuf) && (NULL == rxLen) && (0 == rxBufferCnt))
        {
            rxReq = false;
        }

        /* TX Requested and all parameters are valid */
        if ((true == txReq) && (NULL != txBuf) && (NULL != txLen) && (0 != txBufferCnt))
        {
            status = PTX_STATUS_SUCCESS;
            /* Do Tx operation: all parameters provided valid. */

            uint32_t txIdx = 0;
            while ((PTX_STATUS_SUCCESS == status) && (txIdx < txBufferCnt))
            {
                /* send repeated start after last tx buffer if there is data to read */
                uint8_t restart = ((rxBufferCnt > 0) && (txIdx == txBufferCnt - 1)) ? 1 : 0;

                if (NULL != txBuf[txIdx])
                {
                    status = ptx_HAL_DRV_Interface_I3C_Tx(&i3cInterface->Drv, txBuf[txIdx], txLen[txIdx], restart);
                }
                else
                {
                    /* Buffer is not valid */
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
                }
                
                ++txIdx;
            }
        }

        /* If TX was requested, but one of the parameters was not valid, abort the RX as well */
        if ((true == txReq) && (PTX_STATUS_SUCCESS != status))
        {
            rxReq = false;
        }

        /* Let us check for Rx operation. */
        if ((true == rxReq) && (NULL != rxBuf) && (NULL != rxLen) && (0 != rxBufferCnt))
        {
            status = PTX_STATUS_SUCCESS;
            /* Rx operation requested and parameters are valid. */
            uint32_t rxIdx = 0;
            while ((PTX_STATUS_SUCCESS == status) && (rxIdx < rxBufferCnt))
            {
                if (NULL != rxBuf[rxIdx])
                {
                    status = ptx_HAL_DRV_Interface_I3C_Rx(&i3cInterface->Drv, rxBuf[rxIdx], rxLen[rxIdx], 0);  /* Per default no restart condition */
                }
                else
                {
                    /* Buffer is not valid */
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
                }

                ++rxIdx;
            }
        }
    }
    
    return status;
}

ptx_Status_t ptx_HAL_Interface_I3C_RxPending(void *ctx, uint8_t *rxPending)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_I3C_t *i3cInterface = &((ptx_HAL_Interface_t*)ctx)->I3cInterface;

    if (PTX_COMP_CHECK(i3cInterface, Comp_HAL) && (NULL != rxPending))
    {
        PTX_LOG_V(i3cInterface->Log, "ptx_HAL_Interface_I3C_RxPending");

        status = ptx_HAL_DRV_Interface_I3C_RxIrqLevel(&i3cInterface->Drv, rxPending);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I3C_SetParameter(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_I3C_t *i3cInterface = &((ptx_HAL_Interface_t*)ctx)->I3cInterface;

    if (PTX_COMP_CHECK(i3cInterface, Comp_HAL))
    {
        PTX_LOG_V(i3cInterface->Log, "ptx_HAL_Interface_I3C_SetParameter");

        status = ptx_HAL_DRV_Interface_I3C_SetParameter(&i3cInterface->Drv, parameter);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I3C_GetParameter(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_I3C_t *i3cInterface = &((ptx_HAL_Interface_t*)ctx)->I3cInterface;

    if (PTX_COMP_CHECK(i3cInterface, Comp_HAL))
    {
        PTX_LOG_V(i3cInterface->Log, "ptx_HAL_Interface_I3C_GetParameter");

        status = ptx_HAL_DRV_Interface_I3C_GetParameter(&i3cInterface->Drv, parameter);
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
