/* \file
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
    Module      : EVK Peripherals - Power Sensor
    File        : ptx_EVK_PowerSensor.c

    Description : Submodule of EVK Peripherals, which allows the control of the Power Sensor
                  component on the RRQ35230-POLLER_EVB Board.
 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_EVK_Peripherals_PowerSensor.h"
#include <cmsis_gcc.h>
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
/* IC Device Details */
#define ISL28025_CAPABILITY             (uint8_t)0x19
#define ISL28025_VOUT_MODE              (uint8_t)0x20
#define ISL28025_PMBUS_REV              (uint8_t)0x99
#define ISL28025_IC_DEVICE_ID           (uint8_t)0xAD
#define ISL28025_IC_DEVICE_REV          (uint8_t)0xAE

/* Global IC Controls */
#define ISL28025_RESTORE_DEFAULT_ALL    (uint8_t)0x12
#define ISL28025_OPERATION              (uint8_t)0x01

/* Primary and Auxiliary Channel Controls */
#define ISL28025_SET_DPM_MODE           (uint8_t)0xD2
#define ISL28025_DPM_CONV_STATUS        (uint8_t)0xD3
#define ISL28025_CONFIG_ICHANNEL        (uint8_t)0xD4

/* Calibration and Configuration Registers */
#define ISL28025_IOUT_CAL_GAIN          (uint8_t)0x38
#define ISL28025_CONFIG_VCHANNEL        (uint8_t)0xD5
#define ISL28025_CONFIG_PEAK_DET        (uint8_t)0xD7

/* Measurement Registers */
#define ISL28025_READ_VSHUNT_OUT        (uint8_t)0xD6
#define ISL28025_READ_VOUT              (uint8_t)0x8B
#define ISL28025_READ_IOUT              (uint8_t)0x8C
#define ISL28025_READ_PEAK_MIN_IOUT     (uint8_t)0xD8
#define ISL28025_READ_PEAK_MAX_IOUT     (uint8_t)0xD9
#define ISL28025_READ_POUT              (uint8_t)0x96
#define ISL28025_READ_VOUT_AUX          (uint8_t)0xE1
#define ISL28025_READ_TEMPERATURE_1     (uint8_t)0x8D

/* Threshold Detectors */
#define ISL28025_VOUT_OV_THRESHOLD_SET  (uint8_t)0xDA
#define ISL28025_VOUT_UV_THRESHOLD_SET  (uint8_t)0xDB
#define ISL28025_IOUT_OC_THRESHOLD_SET  (uint8_t)0xDC

/* SMB Alert Registers */
#define ISL28025_CONFIG_INTR            (uint8_t)0xDD
#define ISL28025_FORCE_FEEDTHR_ALERT    (uint8_t)0xDE
#define ISL28025_SMBALERT_MASK          (uint8_t)0x1B
#define ISL28025_SMBALERT2_MASK         (uint8_t)0xDF
#define ISL28025_CLEAR_FAULTS           (uint8_t)0x03
#define ISL28025_STATUS_VOUT            (uint8_t)0x7A
#define ISL28025_STATUS_IOUT            (uint8_t)0x7B
#define ISL28025_STATUS_TEMPERATURE     (uint8_t)0x7D
#define ISL28025_STATUS_CML             (uint8_t)0x7E
#define ISL28025_STATUS_BYTE            (uint8_t)0x78
#define ISL28025_STATUS_WORD            (uint8_t)0x79

/* Voltage Margin - DAC Out (20 Pin QFN) */
#define ISL28025_CONFIG_VOL_MARGIN      (uint8_t)0xE4
#define ISL28025_SET_VOL_MARGIN         (uint8_t)0xE3

/* External Clock Control (16 Pin WLCSP) */
#define ISL28025_CONFIG_EXT_CLK         (uint8_t)0xE5

#define ISL28025_ADC_ENABLE             (uint8_t)0x0
#define ISL28025_ADC_DISABLE            (uint8_t)0x1

#define ISL28025_ADCENABLE_POS          (uint8_t)6
#define ISL28025_ADCMODE_POS            (uint8_t)3
#define ISL28025_BUSAVERAGING_POS       (uint8_t)3
#define ISL28025_AVGMODE_POS            (uint8_t)3
#define ISL28025_AUXAVERAGING_POS       (uint8_t)10
#define ISL28025_AUXTIME_POS            (uint8_t)7
#define ISL28025_NA1_POS                (uint8_t)8
#define ISL28025_NA2_POS                (uint8_t)7
#define ISL28025_MODE_MASK              (uint8_t)0x07

static const uint16_t sVoltageDivider = 1000U;                                   /* 1mV */
static const int32_t sCurrentDivider = 1000U;                                    /* 1uA */
static const int32_t sPowerDivider = (1000000000000U / ISL28025_P_MAGNIFIER);    /* 1uW */


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static uint16_t ptx_EVK_Peripherals_PowerSensor_CalculateCalibrationGain(
    uint32_t shuntResistorMohm,
    uint32_t maxCurrentMa,
    uint32_t *currentLsbOut);

static uint16_t ptx_EVK_Peripherals_PowerSensor_ConstructIChannelConfig(
    ptx_EVK_Peripherals_PowerSensor_AdcTime_t convTime,
    ptx_EVK_Peripherals_PowerSensor_AveragingMode_t avgMode);

static uint16_t ptx_EVK_Peripherals_PowerSensor_ConstructVChannelConfig(ptx_EVK_Peripherals_PowerSensor_AdcTime_t vauxAdcTime,
                                                                        ptx_EVK_Peripherals_PowerSensor_AdcTime_t vbusAdcTime,
                                                                        ptx_EVK_Peripherals_PowerSensor_AveragingMode_t vauxAveraging,
                                                                        ptx_EVK_Peripherals_PowerSensor_AveragingMode_t vbusAveraging);

static uint8_t ptx_EVK_Peripherals_PowerSensor_ConstructDpmModeConfig(uint8_t enableAdc, ptx_EVK_Peripherals_PowerSensor_AdcMode_t adcMode,
                                                                      ptx_EVK_Peripherals_PowerSensor_OperatingMode_t operatingMode);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_EVK_Peripherals_PowerSensor_Init(ptx_EVK_Peripherals_PowerSensor_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {
        uint32_t regValue = 0U;

        /* Set I2C Slave Address */
        ptx_HAL_DRV_Interface_Parameter_t addressParameter;
        addressParameter.type = HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address;
        addressParameter.value.I2CSlaveAddress = ctx->BusAddress;
        status = ptx_HAL_Interface_SetParameter((void *)ctx->i2cInterface, &addressParameter);

        /* TODO: FLY-437 Check if the chip shall be set in Power Up/Down trough the OPERATION register
        * or if ADC Enable/Disable is enough.*/
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Calculate Gains */
            regValue = ptx_EVK_Peripherals_PowerSensor_CalculateCalibrationGain(ctx->Shunt, ctx->MaxCurrent, &ctx->CurrentLsb);

            uint8_t tx_data[] = { ISL28025_IOUT_CAL_GAIN, (uint8_t)(regValue >> 8), (uint8_t)(regValue & 0xFFU)};
            uint8_t *tx_buf[1] = { tx_data };
            uint32_t tx_len[1] = { sizeof(tx_data) };
            status = ptx_HAL_Interface_TRx(ctx->i2cInterface, tx_buf, tx_len, 1, NULL,  NULL, 0);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Calculate DPM Mode Configuration */
            regValue = ptx_EVK_Peripherals_PowerSensor_ConstructDpmModeConfig(ISL28025_ADC_DISABLE, CONTINUOUS_MODE, ctx->OpMode);

            /* Write Configuration */
            uint8_t tx_data[] = {ISL28025_SET_DPM_MODE, (uint8_t)(regValue)};
            uint8_t *tx_buf[1] = { tx_data };
            uint32_t tx_len[1] = { sizeof(tx_data) };
            status = ptx_HAL_Interface_TRx(ctx->i2cInterface, tx_buf, tx_len, 1, NULL,  NULL, 0);
        }
    }

    return status;
}

ptx_Status_t ptx_EVK_Peripherals_PowerSensor_StartConversion(ptx_EVK_Peripherals_PowerSensor_t *ctx,
                                                             ptx_EVK_Peripherals_PowerSensor_AveragingMode_t avgBus, ptx_EVK_Peripherals_PowerSensor_AveragingMode_t avgShunt,
                                                             ptx_EVK_Peripherals_PowerSensor_AveragingMode_t avgAux, ptx_EVK_Peripherals_PowerSensor_AdcTime_t convTimeBus,
                                                             ptx_EVK_Peripherals_PowerSensor_AdcTime_t convTimeShunt, ptx_EVK_Peripherals_PowerSensor_AdcTime_t convTimeAux,
                                                             ptx_EVK_Peripherals_PowerSensor_AdcMode_t continuous)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {
        uint32_t regValue = 0U;

        ptx_HAL_DRV_Interface_Parameter_t addressParameter;
        addressParameter.type = HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address;
        addressParameter.value.I2CSlaveAddress = ctx->BusAddress;
        status = ptx_HAL_Interface_SetParameter((void *)ctx->i2cInterface, &addressParameter);

        /* Set requested DPM ADC Mode Type */
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Calculate DPM Mode Configuration */
            regValue = ptx_EVK_Peripherals_PowerSensor_ConstructDpmModeConfig(ISL28025_ADC_ENABLE, continuous, ctx->OpMode);

            /* Write Configuration */
            uint8_t tx_data[] = {ISL28025_SET_DPM_MODE, (uint8_t)(regValue)};
            uint8_t *tx_buf[1] = { tx_data };
            uint32_t tx_len[1] = { sizeof(tx_data) };
            status = ptx_HAL_Interface_TRx(ctx->i2cInterface, tx_buf, tx_len, 1, NULL,  NULL, 0);
        }

        /* Set IChannel Mode */
        if (PTX_STATUS_SUCCESS == status)
        {
            regValue = ptx_EVK_Peripherals_PowerSensor_ConstructIChannelConfig(convTimeShunt, avgShunt);

            /* Write Configuration */
            uint8_t tx_data[] = {ISL28025_CONFIG_ICHANNEL, (uint8_t)(0x03U), (uint8_t)(regValue & 0xFF)};
            uint8_t *tx_buf[1] = { tx_data };
            uint32_t tx_len[1] = { sizeof(tx_data) };
            status = ptx_HAL_Interface_TRx(ctx->i2cInterface, tx_buf, tx_len, 1, NULL,  NULL, 0);
        }

        /* Set VChannel Mode */
        if (PTX_STATUS_SUCCESS == status)
        {
            regValue = ptx_EVK_Peripherals_PowerSensor_ConstructVChannelConfig(convTimeAux, convTimeBus, avgAux, avgBus);

            /* Write Configuration */
            uint8_t tx_data[] = {ISL28025_CONFIG_VCHANNEL, (uint8_t)(regValue >> 8), (uint8_t)(regValue & 0xFFU)};
            uint8_t *tx_buf[1] = { tx_data };
            uint32_t tx_len[1] = { sizeof(tx_data) };
            status = ptx_HAL_Interface_TRx(ctx->i2cInterface, tx_buf, tx_len, 1, NULL,  NULL, 0);
        }
    }

    return status;
}

ptx_Status_t ptx_EVK_Peripherals_PowerSensor_StopConversion(ptx_EVK_Peripherals_PowerSensor_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {
        uint32_t regValue = 0U;
        ptx_HAL_DRV_Interface_Parameter_t addressParameter;
        addressParameter.type = HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address;
        addressParameter.value.I2CSlaveAddress = ctx->BusAddress;
        status = ptx_HAL_Interface_SetParameter((void *)ctx->i2cInterface, &addressParameter);

        /* Set requested DPM ADC Mode Type */
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Calculate DPM Mode Configuration */
            regValue = ptx_EVK_Peripherals_PowerSensor_ConstructDpmModeConfig(ISL28025_ADC_DISABLE, CONTINUOUS_MODE, ctx->OpMode);

            /* Write Register */
            uint8_t tx_data[] = {ISL28025_SET_DPM_MODE, (uint8_t)(regValue)};
            uint8_t *tx_buf[1] = { tx_data };
            uint32_t tx_len[1] = { sizeof(tx_data) };
            status = ptx_HAL_Interface_TRx(ctx->i2cInterface, tx_buf, tx_len, 1, NULL,  NULL, 0);
        }
    }

    return status;
}

ptx_Status_t ptx_EVK_Peripherals_PowerSensor_ReadData(ptx_EVK_Peripherals_PowerSensor_t *ctx, ptx_EVK_Peripherals_PowerSensor_MeasurementValue_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {
        ptx_HAL_DRV_Interface_Parameter_t addressParameter;
        addressParameter.type = HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address;
        addressParameter.value.I2CSlaveAddress = ctx->BusAddress;
        status = ptx_HAL_Interface_SetParameter((void *)ctx->i2cInterface, &addressParameter);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint16_t tmp;
            uint8_t *rx[1] = { (uint8_t *) &tmp };
            uint32_t rx_len[1] = { sizeof(tmp) };

            uint8_t tx_data[] = {ISL28025_READ_VOUT};
            uint8_t *tx_buf[1] = { tx_data };
            uint32_t tx_len[1] = { sizeof(tx_data) };
            status = ptx_HAL_Interface_TRx(ctx->i2cInterface, tx_buf, tx_len, 1, rx,  rx_len, 1);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* compute bus voltage and read power */
                tmp = (uint16_t) __REVSH((int16_t)tmp);
                data->BusVoltage = (uint32_t)((ISL28025_V_BUS_LSB * tmp) / sVoltageDivider); /* mV */

                tmp = 0U; /* Reset tmp */

                *tx_data = ISL28025_READ_POUT;
                status = ptx_HAL_Interface_TRx(ctx->i2cInterface, tx_buf, tx_len, 1, rx,  rx_len, 1);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* compute power and read current */
                tmp = (uint16_t) __REVSH((int16_t)tmp);
                /* TODO: FLY-437 Find a better way to handle big numbers and signed as well. Not sure if
                * it works for negative power correct.*/
                int64_t regVal = (int64_t)tmp * ISL28025_P_LSB(ctx->CurrentLsb);

                data->Power = (int32_t)(regVal / (int64_t)sPowerDivider); /* uW */

                tmp = 0U; /* Reset tmp */

                *tx_data = ISL28025_READ_IOUT;
                status = ptx_HAL_Interface_TRx(ctx->i2cInterface, tx_buf, tx_len, 1, rx,  rx_len, 1);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* compute current */
                int16_t stmp = (int16_t) __REVSH((int16_t)tmp);
                int32_t regVal = (int32_t) (stmp * ((int32_t)ctx->CurrentLsb));
                data->Current = (int32_t)(regVal / sCurrentDivider); /* uA */

                if (data->Current < 0)
                {
                    data->Power *= -1;
                }

                *tx_data = ISL28025_READ_VOUT_AUX;
                status = ptx_HAL_Interface_TRx(ctx->i2cInterface, tx_buf, tx_len, 1, rx,  rx_len, 1);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                tmp = (uint16_t) __REVSH((int16_t)tmp);
                data->AuxiliaryVoltage = (uint16_t)((ISL28025_V_AUX_LSB * tmp) / sVoltageDivider) *
                                         ISL28025_VOLTAGE_DIVIDER; /* mV */
            }
        }
    }

    return status;
}

ptx_Status_t ptx_EVK_Peripherals_PowerSensor_DeInit(ptx_EVK_Peripherals_PowerSensor_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {
        status = ptx_EVK_Peripherals_PowerSensor_StopConversion(ctx);
        if (PTX_STATUS_SUCCESS == status)
        {
            memset(ctx, 0, sizeof(ptx_EVK_Peripherals_PowerSensor_t));
        }
    }

    return status;
}

/**
 * \brief Calculate and set the calibration gain register value for the ISL28025F.
 *
 * This function computes the calibration register value based on the specified
 * shunt resistor and maximum measurable current. It also outputs the calculated
 * current LSB (Least Significant Bit) value.
 *
 * \param shuntResistorMilliOhm The value of the shunt resistor in milliohms.
 * \param maxCurrentMa The maximum measurable current in milliamperes.
 * \param currentLsbOut Pointer to a variable where the computed current LSB value will be stored.
 * \return The calculated calibration register value.
 */
static uint16_t ptx_EVK_Peripherals_PowerSensor_CalculateCalibrationGain(
    uint32_t shuntResistorMohm, uint32_t maxCurrentMa, uint32_t *currentLsbOut)
{
    /* Step 1: Calculate Current LSB in nA/bit */
    uint32_t currentLsbNanoAmps = (maxCurrentMa * 1000000u) / ISL28025_ADC_RES;

    /* Step 2: Calculate Calibration Register Value */
    uint32_t calRegVal = (ISL28025_MATH_RES * ISL28025_V_SHUNT_LSB) /
                         ((currentLsbNanoAmps * shuntResistorMohm) / 1000u);

    /* Output the calculated LSB for storing in the structure */
    if (currentLsbOut)
    {
        *currentLsbOut = currentLsbNanoAmps;
    }

    /* Return the integer value of the calibration register */
    return (uint16_t)calRegVal;
}

/**
 * \brief Construct the 16-bit register value for the ISL28025_CONFIG_ICHANNEL register.
 *
 * This function combines the configuration parameters for conversion time and
 * averaging mode into a single 16-bit value, which is used to configure the
 * ICHANNEL settings of the ISL28025F.
 *
 * \param convTime The conversion time descriptor specifying the ADC's sampling duration.
 * \param avgMode The averaging mode descriptor specifying the number of samples to average.
 * \return A 16-bit register value encoding the provided configuration settings.
 */
static uint16_t ptx_EVK_Peripherals_PowerSensor_ConstructIChannelConfig(
    ptx_EVK_Peripherals_PowerSensor_AdcTime_t convTime, ptx_EVK_Peripherals_PowerSensor_AveragingMode_t avgMode)
{
    uint16_t regValue = 0u;

    /* Bitfield: [15:10] N/A -> 0, [9:7] N/A -> 1, [6:3] AVG, [2:0] ADC Time */
    regValue = (uint16_t) (regValue | (3 << ISL28025_NA1_POS));
    regValue = (uint16_t) (regValue | (1 << ISL28025_NA2_POS));
    regValue = (uint16_t) (regValue | (avgMode << ISL28025_AVGMODE_POS));
    regValue = (uint16_t) (regValue | convTime);

    return regValue;
}

/**
 * \brief Construct the 16-bit register value for the ISL28025_CONFIG_VCHANNEL register.
 *
 * This function generates a 16-bit configuration value for the VCHANNEL settings
 * by combining the averaging modes and ADC times for both the VBUS and VAUX channels.
 *
 * \param vauxAdcTime The ADC sampling duration descriptor for the VAUX channel.
 * \param vbusAdcTime The ADC sampling duration descriptor for the VBUS channel.
 * \param vauxAveraging The averaging mode descriptor for the VAUX channel, specifying the number
 * of samples to average.
 * \param vbusAveraging The averaging mode descriptor for the VBUS channel, specifying the number
 * of samples to average.
 * \return A 16-bit register value encoding the combined configuration for VBUS and VAUX channels.
 */
static uint16_t ptx_EVK_Peripherals_PowerSensor_ConstructVChannelConfig(ptx_EVK_Peripherals_PowerSensor_AdcTime_t vauxAdcTime,
                                                                        ptx_EVK_Peripherals_PowerSensor_AdcTime_t vbusAdcTime, ptx_EVK_Peripherals_PowerSensor_AveragingMode_t vauxAveraging,
                                                                        ptx_EVK_Peripherals_PowerSensor_AveragingMode_t vbusAveraging)
{
    uint16_t regValue = 0u;

    /* Bitfield: [15:13] VAUX AVG, [12:10] VAUX ADC Time, [9:7] VBUS AVG, [6:4] VBUS ADC Time */
    regValue = (uint16_t) (regValue | (vauxAveraging << ISL28025_AUXAVERAGING_POS));
    regValue = (uint16_t) (regValue | (vauxAdcTime << ISL28025_AUXTIME_POS));
    regValue = (uint16_t) (regValue | (vbusAveraging << ISL28025_BUSAVERAGING_POS));
    regValue = (uint16_t) (regValue | vbusAdcTime);

    return regValue;
}

/**
 * \brief Construct the register value for the DPM_MODE configuration.
 *
 * This function generates an 8-bit configuration value for the DPM_MODE register
 * based on the specified ADC state, ADC mode, and operating mode.
 *
 * \param enableAdc Specifies whether to enable or disable the ADC (0 to enable, 1 to disable).
 * \param adcMode The descriptor specifying the ADC operating mode, such as continuous or
 * triggered.
 * \param operatingMode The descriptor specifying the device's operating mode.
 * \return An 8-bit register value encoding the DPM_MODE configuration settings.
 */
static uint8_t ptx_EVK_Peripherals_PowerSensor_ConstructDpmModeConfig(uint8_t enableAdc, ptx_EVK_Peripherals_PowerSensor_AdcMode_t adcMode,
                                                                      ptx_EVK_Peripherals_PowerSensor_OperatingMode_t operatingMode)
{
    uint8_t regValue = 0u;

    /* Bitfield: [6] Enable ADC, [3] ADC Mode Type, [2-0] Operating Mode */
    regValue = (uint8_t) (regValue | (enableAdc << ISL28025_ADCENABLE_POS));         /* Set ADC enable bit */
    regValue = (uint8_t) (regValue | (operatingMode & ISL28025_MODE_MASK));   /* Set operating mode bit */
    regValue = (uint8_t) (regValue | (adcMode << ISL28025_ADCMODE_POS));           /* Set ADC mode bit */

    return regValue;
}
