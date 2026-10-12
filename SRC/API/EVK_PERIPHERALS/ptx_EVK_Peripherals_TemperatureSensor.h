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
    Module      : EVK Peripherals - Temperature Sensor
    File        : ptx_EVK_Peripherals_TemperatureSensor.h

    Description : Submodule of EVK Peripherals, which allows the control of the Temperature Sensor
                  component on the RRQ35230-POLLER_EVB Board.
*/
/**
 * \addtogroup EVK_PERIPHERALS_TEMPERATURESENSOR
 * @{
 */
#if !defined(PTX_RULPERIPHERALSTEMPERATURESENSOR_H_)
#define PTX_RULPERIPHERALSTEMPERATURESENSOR_H_


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

/**
 * \name Temperature sensor specific adresses.
 * @{
 */
#define PTX_EVK_PERIPHERALS_TEMP_SENSOR_ADDRESS          (0x4B)   /**< Default I2C address of the temperature sensor. */
#define PTX_EVK_PERIPHERALS_TEMP_SENSOR_TEMP_REGISTER    (0x00)   /**< Address of the register holding the temperature reading. */
#define PTX_EVK_PERIPHERALS_SENSOR_CONF_REGISTER         (0x01)   /**< Address of the register to configure the sensor. */
/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Temperature sensor component.
 */
typedef struct ptx_EVK_Peripherals_TemperatureSensor
{
    ptx_Comps_t                     CompId;           /**< Component Id. */
    ptx_HAL_t                       *i2cInterface;    /**< I2C Communication Interface. */
    uint8_t                         BusAddress;       /**< I2C address of temperature sensor. */
} ptx_EVK_Peripherals_TemperatureSensor_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initialize HW and SW needed for temperature sensor on poller board.
 *
 * \note The caller has to provide an allocated and initialized I2C interface instance. I2C interface initialization is not done here.
 *
 * \param[in]       ctx             Pointer to not initialized instance of Temperature Sensor.
 * \param[in]       i2cAddress         I2C address of the temperature sensor.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_TemperatureSensor_Init(ptx_EVK_Peripherals_TemperatureSensor_t *ctx, uint8_t i2cAddress);

/**
 * \brief De-initialize HW and SW needed for temperature sensor on poller board.
 *
 * \param[in]       ctx             Pointer to initialized instance of Temperature Sensor.
 * \param[out]      tempValue       Pointer where the measured temperature value will be stored.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_TemperatureSensor_ReadTemp(ptx_EVK_Peripherals_TemperatureSensor_t *ctx, int16_t *tempValue);

/**
 * \brief De-initialize HW and SW needed for temperature sensor on poller board.
 *
 * \param[in]       ctx          Pointer to initialized instance of Temperature Sensor.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_TemperatureSensor_DeInit(ptx_EVK_Peripherals_TemperatureSensor_t *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

