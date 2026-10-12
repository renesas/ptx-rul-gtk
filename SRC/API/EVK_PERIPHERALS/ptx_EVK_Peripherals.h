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
    Module      : EVK Peripherals
    File        : ptx_EVK_Peripherals.h

    Description : EVK Peripherals is a set of modules which controls various external hardware component on the RRQ35230-POLLER_EVB board.
                  The component is only needed for the RRQ35230-POLLER_EVB board, but the implementation may be re-used for own
                  application-specific purposes, but can be excluded otherwise.
*/
/**
 * \addtogroup EVK_PERIPHERALS
 * @{
 */

#if !defined(PTX_RULPERIPHERALS_H_)
#define PTX_RULPERIPHERALS_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Status.h"
#include "ptx_HAL.h"
#include "ptx_EVK_Peripherals_DCDC.h"
#include "ptx_EVK_Peripherals_PowerSensor.h"
#include "ptx_EVK_Peripherals_TemperatureSensor.h"
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief EVK Peripherals component.
 */
typedef struct ptx_EVK_Peripherals
{
    ptx_Comps_t                              CompId;         /**< Component Id. */
    ptx_HAL_t                                i2cInterface;   /**< I2C Interface for Peripherals. */
    ptx_EVK_Peripherals_DCDC_t               Dcdc;           /**< DC/DC converter. */
    ptx_EVK_Peripherals_TemperatureSensor_t  TempSensor;     /**< Temperature Sensor. */
    ptx_EVK_Peripherals_PowerSensor_t        PowerSensorVdTx;/**< Power sensor for VdTx. */
    ptx_EVK_Peripherals_PowerSensor_t        PowerSensorVbat;/**< Power sensor for Vbat. */
    ptx_Log_t                               *Log;            /**< LOG context. */
} ptx_EVK_Peripherals_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initialize HW and SW needed to operate the EVK PCB.
 *
 * \param[in]       ctx             Pointer to not initialized instance of RUL peripherals.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_Init(ptx_EVK_Peripherals_t *ctx);

/**
 * \brief De-initialize HW and SW components.
 *
 * \param[in]       ctx             Pointer to initialized instance of RUL peripherals.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_EVK_Peripherals_Deinit(ptx_EVK_Peripherals_t *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

