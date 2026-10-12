/** \file
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
    Module      : EVK Peripherals - Power Sensor
    File        : ptx_EVK_Peripherals_PowerSensor.h

    Description : Submodule of EVK Peripherals, which allows the control of the Power Sensor
                  component on the RRQ35230-POLLER_EVB Board.
*/
/**
 * \addtogroup PTX_PERIPHERALS_POWERSENSOR
 * @{
 */
#if !defined(PTX_RULPERIPHERALSPOWERSENSOR_H_)
#define PTX_RULPERIPHERALSPOWERSENSOR_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Status.h"
#include "ptx_HAL.h"
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
#define ISL28025_V_BUS_LSB          250U                        /**< 250uV */
#define ISL28025_V_AUX_LSB          100U                        /**< 100uV */
#define ISL28025_V_SHUNT_LSB        2500U                       /**< 2.5uV */
#define ISL28025_P_LSB(x)           ((x) * ISL28025_V_BUS_LSB)  /**< Power LSB calculated with formula Current LSB * Vbus LSB */
#define ISL28025_P_MAGNIFIER        40000000U                   /**< Integer used in calculating the power output */
#define ISL28025_MATH_RES           2048U                       /**< 11 bits of resolution */
#define ISL28025_ADC_RES            32768U                      /**< 15 bits of resolution */
#define ISL28025_VOLTAGE_DIVIDER    2U                          /**< Ratio resulted from the HW implemented voltage divider */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Enums and Structures for ISL280255 Registers.
 */
typedef enum ptx_EVK_Peripherals_PowerSensor_Configuration
{
    /* Sensors Configurations */
    BAT_ISL_SENSOR = 0U,
    MCU_ISL_SENSOR
} ptx_EVK_Peripherals_PowerSensor_Configuration_t;

/**
 * \brief ADC Conversion Time Options
 */
typedef enum ptx_EVK_Peripherals_PowerSensor_AdcTime
{
    ADC_TIME_64_US    = 0x0,
    ADC_TIME_128_US   = 0x1,
    ADC_TIME_256_US   = 0x2,
    ADC_TIME_512_US   = 0x3,
    ADC_TIME_1024_US  = 0x4,
    ADC_TIME_2048_US  = 0x6
} ptx_EVK_Peripherals_PowerSensor_AdcTime_t;

/**
 * \brief DPM Operating Modes
 */
typedef enum ptx_EVK_Peripherals_PowerSensor_OperatingMode
{
    PRIMARY_SHUNT  = 0x0,
    PRIMARY_VBUS   = 0x1,
    PRIMARY_BOTH   = 0x2,
    AUX_VBUS       = 0x4,
    INTERNAL_TEMP  = 0x6,
    ALL            = 0x7
} ptx_EVK_Peripherals_PowerSensor_OperatingMode_t;

/**
 * \brief ADC Modes
 */
typedef enum ptx_EVK_Peripherals_PowerSensor_AdcMode
{
    TRIGGER_MODE      = 0x0,
    CONTINUOUS_MODE   = 0x1
} ptx_EVK_Peripherals_PowerSensor_AdcMode_t;

/**
 * \brief ptx_EVK_Peripherals_PowerSensor_AveragingMode Mode Options
 */
typedef enum ptx_EVK_Peripherals_PowerSensor_AveragingMode
{
    AVG_1_SAMPLE      = 0x0,
    AVG_2_SAMPLES     = 0x1,
    AVG_4_SAMPLES     = 0x2,
    AVG_8_SAMPLES     = 0x3,
    AVG_16_SAMPLES    = 0x4,
    AVG_32_SAMPLES    = 0x5,
    AVG_64_SAMPLES    = 0x6,
    AVG_128_SAMPLES   = 0x7,
    AVG_256_SAMPLES   = 0x8,
    AVG_512_SAMPLES   = 0x9,
    AVG_1024_SAMPLES  = 0xA,
    AVG_2048_SAMPLES  = 0xB,
    AVG_4096_SAMPLES  = 0xC
} ptx_EVK_Peripherals_PowerSensor_AveragingMode_t;

/**
 * \brief ptx_EVK_Peripherals_PowerSensor_t Power Sensor data structure
 */
typedef struct ptx_EVK_Peripherals_PowerSensor
{
    ptx_HAL_t                                           *i2cInterface;      /**< I2C Interface for Peripherals. */
    uint32_t                                            CurrentLsb;         /**< Unit of an LSB in [nA] */
    uint8_t                                             BusAddress;         /**< Sensor slave address */
    uint16_t                                            Shunt;              /**< Shunt value [mOhm] */
    uint16_t                                            MaxCurrent;         /**< Maximum expected current [mA] */
    ptx_EVK_Peripherals_PowerSensor_OperatingMode_t     OpMode;             /**< Operating Mode */
} ptx_EVK_Peripherals_PowerSensor_t;

/**
 * \brief ptx_EVK_Peripherals_PowerSensor_MeasurementValue_t Returned measurement data from the sensor
 */
typedef struct ptx_EVK_Peripherals_PowerSensor_MeasurementValue
{
    uint32_t  BusVoltage;               /**< Measured bus voltage [mV] */
    int32_t   Power;                    /**< Measured power [mW] */
    int32_t   Current;                  /**< Measured current [uA] */
    uint32_t  AuxiliaryVoltage;         /**< Measured current [mV] */
} ptx_EVK_Peripherals_PowerSensor_MeasurementValue_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initialize the ISL28025 sensor.
 *
 * This function initializes the ISL28025 sensor by setting the I2C slave address,
 * calculating and writing the calibration gain, and configuring the DPM mode.
 * It ensures the sensor is ready for operation by writing the appropriate values
 * to its registers.
 *
 * \param ctx Pointer to the ISL28025 sensor structure containing configuration
 *               and runtime data.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_PowerSensor_Init(ptx_EVK_Peripherals_PowerSensor_t *ctx);

/**
 * \brief Start a conversion on the ISL28025 sensor.
 *
 * This function configures and starts a conversion on the ISL28025 sensor by setting
 * the averaging mode and ADC conversion time for the VBUS, VSHUNT, and VAUX channels.
 * It also allows the user to select the ADC mode (e.g., continuous or triggered).
 *
 * \param ctx Pointer to the ISL28025 sensor structure containing configuration
 *               and runtime data.
 * \param avgBus Averaging mode descriptor for the VBUS channel.
 * \param avgShunt Averaging mode descriptor for the VSHUNT channel.
 * \param avgAux Averaging mode descriptor for the VAUX channel.
 * \param convTimeBus ADC conversion time descriptor for the VBUS channel.
 * \param convTimeShunt ADC conversion time descriptor for the VSHUNT channel.
 * \param convTimeAux ADC conversion time descriptor for the VAUX channel.
 * \param continuous ADC mode descriptor specifying whether the conversion is continuous
 *                   or single-shot.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_PowerSensor_StartConversion(ptx_EVK_Peripherals_PowerSensor_t *ctx,
                                                             ptx_EVK_Peripherals_PowerSensor_AveragingMode_t avgBus, ptx_EVK_Peripherals_PowerSensor_AveragingMode_t avgShunt,
                                                             ptx_EVK_Peripherals_PowerSensor_AveragingMode_t avgAux, ptx_EVK_Peripherals_PowerSensor_AdcTime_t convTimeBus,
                                                             ptx_EVK_Peripherals_PowerSensor_AdcTime_t convTimeShunt, ptx_EVK_Peripherals_PowerSensor_AdcTime_t convTimeAux,
                                                             ptx_EVK_Peripherals_PowerSensor_AdcMode_t continuous);

/**
 * \brief Stop an ongoing conversion on the ISL28025 sensor.
 *
 * This function disables the ADC on the ISL28025 sensor, effectively stopping any
 * ongoing conversions. It updates the DPM_MODE register to disable the ADC functionality.
 *
 * \param ctx Pointer to the ISL28025 sensor structure containing configuration
 *               and runtime data.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_PowerSensor_StopConversion(ptx_EVK_Peripherals_PowerSensor_t *ctx);

/**
 * \brief Read measured data from the ISL28025 sensor.
 *
 * This function reads the measured data from the ISL28025 sensor, including voltage,
 * current, and power. It retrieves data from the relevant registers
 * and populates the provided `ptx_PowerSensor_MeasurementValue_t` structure.
 *
 * \param ctx Pointer to the ISL28025 sensor structure containing configuration
 *               and runtime data.
 * \param data Pointer to the Measurement Value data.
 * 
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_PowerSensor_ReadData(ptx_EVK_Peripherals_PowerSensor_t *ctx, ptx_EVK_Peripherals_PowerSensor_MeasurementValue_t *data);

/**
 * \brief Deinit the ISL28025 sensor.
 *
 * This function performs the necessary steps to deinitialize the ISL28025 sensor.
 * It disables the ADC and clears any configuration or runtime states associated with
 * the provided sensor handle.
 *
 * \param ctx Pointer to the ISL28025 sensor structure containing configuration
 *               and runtime data.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_PowerSensor_DeInit(ptx_EVK_Peripherals_PowerSensor_t *ctx);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

