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
    Module      : EVK Peripherals
    File        : ptx_EVK_Peripherals.c

    Description : EVK Peripherals is a set of modules which controls various external hardware component on the RRQ35230-POLLER_EVB board.
                  The component is only needed for the RRQ35230-POLLER_EVB board, but the implementation may be re-used for own
                  application-specific purposes, but can be excluded otherwise.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Status.h"
#include "ptx_HAL.h"
#include "ptx_HAL_DRV_GPIO.h"
#include "ptx_EVK_Peripherals.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_EVK_PERIPHERALS_POWERSENSOR_VDTX_ADDRESS    (0x40)  /**< Default I2C address of the VDTX power sensor. */
#define PTX_EVK_PERIPHERALS_POWERSENSOR_VBAT_ADDRESS    (0x44)  /**< Default I2C address of the VBAT power sensor. */
#define PTX_EVK_PERIPHERALS_POWERSENSOR_DEFAULT_SHUNT   (100)   /**< Default shunt resistor [in mOhm]. */
#define PTX_EVK_PERIPHERALS_POWERSENSOR_VDTX_MAXCURRENT (1200)  /**< Maximum expected current on VDTX rail (includes headroom). */
#define PTX_EVK_PERIPHERALS_POWERSENSOR_VBAT_MAXCURRENT (250)   /**< Maximum expected current on VBAT rail (includes headroom). */

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

ptx_Status_t ptx_EVK_Peripherals_Init(ptx_EVK_Peripherals_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {
        ptx_HAL_InitParams_t i2cInit;
        memset(&i2cInit, 0, sizeof(ptx_HAL_InitParams_t));
        i2cInit.InterfaceType = HAL_Interface_Type_I2C;
        i2cInit.InterfacePortType = HAL_DRV_Interface_Port_Type_Evk;
        i2cInit.InterfacePortID = 0;
        i2cInit.NumberOfTimers = 0;
        i2cInit.Log = ctx->Log;
        status = ptx_HAL_Init(&ctx->i2cInterface, &i2cInit);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_Interface_Open(&ctx->i2cInterface);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_HAL_DRV_Interface_Parameter_t parameter = {};
            parameter.type = HAL_DRV_Interface_Parameter_Type_Transmission_Speed;
            parameter.value.TransmissionSpeed = PTX_HAL_DRV_INTERFACE_I2C_SPEED_DEFAULT;
            status = ptx_HAL_Interface_SetParameter(&ctx->i2cInterface, &parameter);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->Dcdc.i2cInterface = &ctx->i2cInterface;
            status = ptx_EVK_Peripherals_DCDC_Init(&ctx->Dcdc, PTX_EVK_PERIPHERALS_DCDC_I2C_ADDRESS);
        }

        /** The following code section initializes sensors which are optional on some of the boards. */

        /** Setup of the temperature sensor. */
        memset(&ctx->TempSensor, 0, sizeof(ptx_EVK_Peripherals_TemperatureSensor_t));
        ctx->TempSensor.i2cInterface = &ctx->i2cInterface;
        (void) ptx_EVK_Peripherals_TemperatureSensor_Init(&ctx->TempSensor, PTX_EVK_PERIPHERALS_TEMP_SENSOR_ADDRESS);

        /** Setup of the power sensor for VDTX rail. */
        memset(&ctx->PowerSensorVdTx, 0, sizeof(ptx_EVK_Peripherals_PowerSensor_t));
        ctx->PowerSensorVdTx.i2cInterface     = &ctx->i2cInterface;
        ctx->PowerSensorVdTx.BusAddress = PTX_EVK_PERIPHERALS_POWERSENSOR_VDTX_ADDRESS;
        ctx->PowerSensorVdTx.Shunt      = PTX_EVK_PERIPHERALS_POWERSENSOR_DEFAULT_SHUNT;
        ctx->PowerSensorVdTx.MaxCurrent = PTX_EVK_PERIPHERALS_POWERSENSOR_VDTX_MAXCURRENT;
        ctx->PowerSensorVdTx.OpMode     = ALL;

        /** Initialize the sensor. */
        (void) ptx_EVK_Peripherals_PowerSensor_Init(&ctx->PowerSensorVdTx);

        /** Configure the sensor to do automatic sampling. */
        (void) ptx_EVK_Peripherals_PowerSensor_StartConversion(&ctx->PowerSensorVdTx,
                                                               AVG_128_SAMPLES, AVG_128_SAMPLES, AVG_128_SAMPLES,
                                                               ADC_TIME_256_US, ADC_TIME_256_US, ADC_TIME_256_US,
                                                               CONTINUOUS_MODE);

        /** Setup of the power sensor for VBAT rail. */
        memset(&ctx->PowerSensorVbat, 0, sizeof(ptx_EVK_Peripherals_PowerSensor_t));
        ctx->PowerSensorVbat.i2cInterface     = &ctx->i2cInterface;
        ctx->PowerSensorVbat.BusAddress = PTX_EVK_PERIPHERALS_POWERSENSOR_VBAT_ADDRESS;
        ctx->PowerSensorVbat.Shunt      = PTX_EVK_PERIPHERALS_POWERSENSOR_DEFAULT_SHUNT;
        ctx->PowerSensorVbat.MaxCurrent = PTX_EVK_PERIPHERALS_POWERSENSOR_VBAT_MAXCURRENT;
        ctx->PowerSensorVbat.OpMode     = ALL;

        /** Initialize the sensor. */
        (void) ptx_EVK_Peripherals_PowerSensor_Init(&ctx->PowerSensorVbat);

        /** Configure the sensor to do automatic sampling. */
        (void) ptx_EVK_Peripherals_PowerSensor_StartConversion(&ctx->PowerSensorVbat,
                                                               AVG_128_SAMPLES, AVG_128_SAMPLES, AVG_128_SAMPLES,
                                                               ADC_TIME_256_US, ADC_TIME_256_US, ADC_TIME_256_US,
                                                               CONTINUOUS_MODE);

        if (PTX_STATUS_SUCCESS == status)
        {
            ctx->CompId = Comp_WLC_PERIPHERALS;
        }
    }

    return status;
}

ptx_Status_t ptx_EVK_Peripherals_Deinit(ptx_EVK_Peripherals_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC_PERIPHERALS))
    {
        ptx_EVK_Peripherals_TemperatureSensor_DeInit(&ctx->TempSensor);
        ptx_EVK_Peripherals_PowerSensor_DeInit(&ctx->PowerSensorVbat);
        ptx_EVK_Peripherals_PowerSensor_DeInit(&ctx->PowerSensorVdTx);
        ptx_EVK_Peripherals_DCDC_Deinit(&ctx->Dcdc);
        ptx_HAL_DeInit(&ctx->i2cInterface);
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}
