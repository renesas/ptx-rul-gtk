/** \file
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
    Module      : EVK Peripherals - DC/DC Converter
    File        : ptx_EVK_Peripherals_DCDC.h

    Description : Submodule of EVK Peripherals, which allows the control of the DC/DC Converter
                  component on the RRQ35230-POLLER_EVB Board.
*/
/**
 * \addtogroup EVK_PERIPHERALS_DCDC
 * @{
 */

#if !defined(PTX_RULPERIPHERALSDCDC_H_)
#define PTX_RULPERIPHERALSDCDC_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Status.h"
#include <stddef.h>
#include "ptx_HAL.h"


#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name Generic DC/DC Converter defines.
 * @{
 */
#define PTX_EVK_PERIPHERALS_DCDC_I2C_ADDRESS    (0x69)   /**< Default I2C address of DC/DC converter. */
/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * DC/DC Converter structure.
 */
typedef struct ptx_EVK_Peripherals_DCDC
{
    ptx_Comps_t                     CompId;           /**< Component ID */
    ptx_HAL_t                       *i2cInterface;    /**< I2C Communication Interface */
    uint8_t                         BusAddress;       /**< I2C address of the DC/DC converter. */
} ptx_EVK_Peripherals_DCDC_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initialize the DC/DC converter with the default configuration.
 *
 * \note The caller has to provide an allocated and initialized I2C interface instance. I2C interface initialization is not done here.
 *
 * \param[in]       ctx                 Pointer to an uninitialized context.
 * \param[in]       i2cAddress          I2C address of the DC/DC converter.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_DCDC_Init(ptx_EVK_Peripherals_DCDC_t *ctx, uint8_t i2cAddress);

/**
 * \brief De-Initialize DC/DC converter.
 *
 * \param[in]       ctx                 Pointer to an initialized context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_DCDC_Deinit(ptx_EVK_Peripherals_DCDC_t *ctx);

/**
 * \brief Sets the DC/DC converter to the desired voltage.
 *
 * \param[in]       ctx                 Pointer to an initialized context..
 * \param[in]       voltage             Desired output voltage in 10mV steps. (e.g. a value of 330 corresponds to 3.3V)
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_DCDC_SetVoltage(ptx_EVK_Peripherals_DCDC_t *ctx, uint16_t voltage);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

