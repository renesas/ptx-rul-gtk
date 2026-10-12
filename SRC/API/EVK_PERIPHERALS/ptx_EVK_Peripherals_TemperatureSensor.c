/* \file
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
    Module      : EVK Peripherals - Temperature Sensor
    File        : ptx_EVK_Peripherals_TemperatureSensor.c

    Description : Submodule of EVK Peripherals, which allows the control of the Temperature Sensor
                  component on the RRQ35230-POLLER_EVB Board.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_EVK_Peripherals_TemperatureSensor.h"
#include <string.h>
#include <stdbool.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_EVK_PERIPHERALS_TEMP_BUFFER_SIZE        (2u)    /**< Size of the Message from the temperature sensor to measure the temperature. */
#define PTX_EVK_PERIPHERALS_TEMP_DEGREE_STEP_SIZE   (2u)    /**< Step size in degrees of the measured the temperature. */
#define PTX_EVK_PERIPHERALS_TEMP_SIGN_MASK          (0x100) /**< Mask to get the sign of the measured the temperature. */

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

ptx_Status_t ptx_EVK_Peripherals_TemperatureSensor_Init(ptx_EVK_Peripherals_TemperatureSensor_t *ctx, uint8_t i2cAddress)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {
        ptx_HAL_DRV_Interface_Parameter_t addressParameter;
        addressParameter.type = HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address;
        addressParameter.value.I2CSlaveAddress = i2cAddress;
        status = ptx_HAL_Interface_SetParameter((void *)ctx->i2cInterface, &addressParameter);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Set the pointer register to temperature register.  */
            uint8_t temp_register = (uint8_t)PTX_EVK_PERIPHERALS_TEMP_SENSOR_TEMP_REGISTER;

            uint8_t *tx[1] = { &temp_register };
            uint32_t tx_len[1] = { sizeof(temp_register) };

            status = ptx_HAL_Interface_TRx((void*)ctx->i2cInterface, tx, tx_len, 1u, false, NULL, 0);

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->CompId = Comp_WLC_PERIPHERALS;
                ctx->BusAddress = i2cAddress;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_EVK_Peripherals_TemperatureSensor_ReadTemp(ptx_EVK_Peripherals_TemperatureSensor_t *ctx, int16_t *tempValue)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC_PERIPHERALS) && (NULL != tempValue) )
    {
        /* Let's read the temperature register.  */
        uint8_t temp_value[PTX_EVK_PERIPHERALS_TEMP_BUFFER_SIZE];

        ptx_HAL_DRV_Interface_Parameter_t addressParameter;
        addressParameter.type = HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address;
        addressParameter.value.I2CSlaveAddress = ctx->BusAddress;
        status = ptx_HAL_Interface_SetParameter((void *)ctx->i2cInterface, &addressParameter);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t *rx[1] = { temp_value };
            uint32_t rx_len[1] = { sizeof(temp_value) };

            status = ptx_HAL_Interface_TRx ((void*)ctx->i2cInterface, NULL, 0, false, rx, rx_len, 1);

            if (PTX_STATUS_SUCCESS == status)
            {
                uint16_t temp_value_long = (uint16_t) (((uint16_t)(temp_value[0] << 8u)) + (uint16_t)temp_value[1]);
                temp_value_long = temp_value_long >> 7u;

                if (temp_value_long >= PTX_EVK_PERIPHERALS_TEMP_SIGN_MASK)
                {
                    /* Negative value.
                     * Let's remove the sign bit, and get complement two value.
                     */
                    temp_value_long = temp_value_long | PTX_EVK_PERIPHERALS_TEMP_SIGN_MASK;
                    temp_value_long = (uint16_t) (PTX_EVK_PERIPHERALS_TEMP_SIGN_MASK - temp_value_long);
                    *tempValue = (int16_t) ((-1) * (int16_t)(temp_value_long / PTX_EVK_PERIPHERALS_TEMP_DEGREE_STEP_SIZE));
                }
                else
                {
                    /* Positive value */
                    *tempValue = (int16_t)(temp_value_long / PTX_EVK_PERIPHERALS_TEMP_DEGREE_STEP_SIZE);
                }

            }
        }

    }

    return status;
}

ptx_Status_t ptx_EVK_Peripherals_TemperatureSensor_DeInit(ptx_EVK_Peripherals_TemperatureSensor_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC_PERIPHERALS))
    {
        (void) memset(ctx, 0, sizeof(ptx_EVK_Peripherals_TemperatureSensor_t));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}
