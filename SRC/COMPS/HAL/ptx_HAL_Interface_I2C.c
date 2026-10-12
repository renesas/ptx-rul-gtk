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
    Module      : HAL Communication Interface for I2C
    File        : ptx_HAL_Interface_I2C.c

    Description : Implementation of the I2C Communication Interface
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_Interface_I2C.h"
#include "ptx_HAL_Interface.h"

#include <string.h>

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

ptx_Status_t ptx_HAL_Interface_I2C_Init(ptx_HAL_Interface_I2C_t *ctx, ptx_HAL_Interface_I2C_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams) && (NULL != initParams->Log))
    {
        PTX_LOG_V(initParams->Log, "ptx_HAL_Interface_I2C_Init");

        memset(&ctx->Drv, 0, sizeof(ptx_HAL_DRV_Interface_I2C_t));

        ptx_HAL_DRV_Interface_I2C_InitParams_t drv_init_params;
        memset(&drv_init_params, 0, sizeof(ptx_HAL_DRV_Interface_I2C_InitParams_t));
        drv_init_params.PortType = initParams->PortType;
        drv_init_params.PortID = initParams->PortID;
        drv_init_params.Speed = initParams->Speed;
        drv_init_params.Log = initParams->Log;
        drv_init_params.Gpio = initParams->Gpio;

        ctx->Log = initParams->Log;

#if defined(PTX_APP_DIVERSITY_WINDOWS) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
        drv_init_params.OS = initParams->OS;
#endif

        status = ptx_HAL_DRV_Interface_I2C_Init(&ctx->Drv, &drv_init_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->CompId = Comp_HAL;
        }

    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I2C_DeInit(ptx_HAL_Interface_I2C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_Interface_I2C_DeInit");

        status = ptx_HAL_DRV_Interface_I2C_DeInit(&ctx->Drv);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I2C_Open(void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_I2C_t *i2cInterface = &((ptx_HAL_Interface_t*)ctx)->I2cInterface;

    if (PTX_COMP_CHECK(i2cInterface, Comp_HAL))
    {
        PTX_LOG_V(i2cInterface->Log, "ptx_HAL_Interface_I2C_Open");

        status = ptx_HAL_DRV_Interface_I2C_Open(&i2cInterface->Drv);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I2C_Close(void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_I2C_t *i2cInterface = &((ptx_HAL_Interface_t*)ctx)->I2cInterface;

    if (PTX_COMP_CHECK(i2cInterface, Comp_HAL))
    {
        PTX_LOG_V(i2cInterface->Log, "ptx_HAL_Interface_I2C_Close");

        status = ptx_HAL_DRV_Interface_I2C_Close(&i2cInterface->Drv);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I2C_TRx(void *ctx, uint8_t *txBuf[], uint32_t txLen[], uint32_t txBufferCnt,
                                        uint8_t *rxBuf[], uint32_t rxLen[], uint32_t rxBufferCnt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_I2C_t *i2cInterface = &((ptx_HAL_Interface_t*)ctx)->I2cInterface;

    if (PTX_COMP_CHECK(i2cInterface, Comp_HAL))
    {
        PTX_LOG_V(i2cInterface->Log, "ptx_HAL_Interface_I2C_TRx");

        status = PTX_STATUS_SUCCESS;

        if ((NULL != txBuf) && (NULL != txLen) && (0 != txBufferCnt))
        {
            /* Do Tx operation: all parameters provided valid. */

            uint32_t txIdx = 0;
            while((PTX_STATUS_SUCCESS == status) && (txIdx < txBufferCnt))
            {
                /* send repeated start after last tx buffer if there is data to read */
                uint8_t restart =((rxBufferCnt > 0) && (txIdx == txBufferCnt - 1)) ? 1 : 0;

                if (NULL != txBuf[txIdx])
                {
                    status = ptx_HAL_DRV_Interface_I2C_Tx(&i2cInterface->Drv, txBuf[txIdx], txLen[txIdx], restart);
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

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Let us check for Rx operation. */
            if ((NULL != rxBuf) && (NULL != rxLen) && (0 != rxBufferCnt))
            {
                /* Rx operation requested and parameters are valid. */
                uint32_t rxIdx = 0;
                while((PTX_STATUS_SUCCESS == status) && (rxIdx < rxBufferCnt))
                {
                    if (NULL != rxBuf[rxIdx])
                    {
                        status = ptx_HAL_DRV_Interface_I2C_Rx(&i2cInterface->Drv, rxBuf[rxIdx], rxLen[rxIdx], 0); /* Per default no restart condition */
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
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I2C_RxPending(void* ctx, uint8_t *rxPending)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_I2C_t *i2cInterface = &((ptx_HAL_Interface_t*)ctx)->I2cInterface;

    if (PTX_COMP_CHECK(i2cInterface, Comp_HAL) && (NULL != rxPending))
    {
        PTX_LOG_V(i2cInterface->Log, "ptx_HAL_Interface_I2C_RxPending");

        status = ptx_HAL_DRV_Interface_I2C_RxIrqLevel(&i2cInterface->Drv, rxPending);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I2C_SetParameter(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_I2C_t *i2cInterface = &((ptx_HAL_Interface_t*)ctx)->I2cInterface;

    if (PTX_COMP_CHECK(i2cInterface, Comp_HAL))
    {
        PTX_LOG_V(i2cInterface->Log, "ptx_HAL_Interface_I2C_SetParameter");

        status = ptx_HAL_DRV_Interface_I2C_SetParameter(&i2cInterface->Drv, parameter);
    }

    return status;
}

ptx_Status_t ptx_HAL_Interface_I2C_GetParameter(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HAL_Interface_I2C_t *i2cInterface = &((ptx_HAL_Interface_t*)ctx)->I2cInterface;

    if (PTX_COMP_CHECK(i2cInterface, Comp_HAL))
    {
        PTX_LOG_V(i2cInterface->Log, "ptx_HAL_Interface_I2C_GetParameter");

        status = ptx_HAL_DRV_Interface_I2C_GetParameter(&i2cInterface->Drv, parameter);
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
