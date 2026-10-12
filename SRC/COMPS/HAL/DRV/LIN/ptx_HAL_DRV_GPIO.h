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
    Module      : Linux GPIO Driver
    File        : ptx_HAL_DRV_GPIO.h

    Description : Implementation of the GPIO Driver for the Linux Reference Platform
*/


/**
 * \addtogroup DRV_LIN_IMPL_GPIO
 *
 * @{
 */

#if !defined(PTX_HAL_DRV_GPIO_H)
#define PTX_HAL_DRV_GPIO_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_Log.h"
#include <stddef.h>
#include "ptx_HAL_DRV_GPIO_Int.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

#define PTX_HAL_DRV_GPIO_RESET_PIN              0   /**< GPIO pin used to hard-reset PTX chip */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL_DRV_GPIO
{
    ptx_Comps_t                         CompId;         /**< Component ID */

    int                                 ResetFd;        /**< Reset Pin File Descriptor */
    uint32_t                            ResetPin;       /**< Reset Pin Number of the predefined gpiochip device */
    ptx_Log_t                           *Log;           /**< Log context. */
} ptx_HAL_DRV_GPIO_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_GPIO_InitParams
{
    ptx_Log_t                   *Log;                   /**< Log context. */
} ptx_HAL_DRV_GPIO_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * Initializes the GPIO driver component. Validates input parameters and sets up
 * component references. The Log context must be a valid logger instance.
 *
 * \param[out]      ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_Init (ptx_HAL_DRV_GPIO_t *ctx, ptx_HAL_DRV_GPIO_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * Closes any open GPIO resources and clears all component data.
 * Resets component ID to Comp_None.
 *
 * \param[in,out]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_DeInit (ptx_HAL_DRV_GPIO_t *ctx);

/**
 * \brief Opens a GPIO device.
 *
 * Opens the GPIO chip device (/dev/gpiochip0 by default) and configures the reset pin
 * (pin 21 by default) as an output. Obtains a line handle for the reset pin via ioctl.
 * Sets the reset pin to HIGH (idle state) upon successful opening.
 *
 * \param[in,out]       ctx     Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_Open (void *ctx);

/**
 * \brief Closes a GPIO device.
 *
 * Closes the GPIO line handle file descriptor and releases the associated resources.
 *
 * \param[in,out]       ctx     Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_Close (void *ctx);

/**
 * \brief Reads a GPIO pin level.
 *
 * Reads the current logic level of the reset pin using GPIOHANDLE_GET_LINE_VALUES_IOCTL.
 * Currently supports only the predefined reset pin (pin 21 on gpiochip0).
 * The pin parameter is ignored.
 *
 * \param[in]       ctx     Pointer to component.
 * \param[in]       pin     Pin to read from (currently unused; reset pin is always used).
 * \param[out]      level   Pointer to variable for storing pin level (0 or 1).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_ReadPin (void *ctx, uint16_t pin, uint8_t *level);

/**
 * \brief Writes a GPIO pin level.
 *
 * Sets the logic level of the reset pin using GPIOHANDLE_SET_LINE_VALUES_IOCTL.
 * Any non-zero level value sets the pin HIGH; zero sets it LOW.
 * Currently supports only the predefined reset pin (pin 21 on gpiochip0).
 * The pin parameter is ignored.
 *
 * \param[in]       ctx     Pointer to component.
 * \param[in]       pin     Pin to write to (currently unused; reset pin is always used).
 * \param[in]       level   Level to set the pin to (0 = LOW, non-zero = HIGH).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_WritePin (void *ctx, uint16_t pin, uint8_t level);

/**
 * \brief Reads data from GPIO port.
 *
 * Not implemented.
 *
 * \param[in]       ctx     Pointer to component.
 * \param[in]       port    Port to read from (unused).
 * \param[out]      data    Pointer to variable for storing port data (set to 0).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_ReadPort (void *ctx, uint32_t port, uint16_t *data);

/**
 * \brief Writes data to GPIO port.
 *
 * Not implemented.
 *
 * \param[in]       ctx     Pointer to component.
 * \param[in]       port    Port to write to (unused).
 * \param[in]       data    Port data to write (unused).
 * \param[in]       mask    Mask of pins to write (unused).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_WritePort (void *ctx, uint32_t port, uint16_t data, uint16_t mask);

/**
 * \brief Sets configuration for a GPIO pin.
 *
 * Not implemented.
 *
 * \param[in]       ctx     Pointer to component.
 * \param[in]       cfg     Array of pin configurations (unused).
 * \param[in]       cfgLen  Length of configuration array (must be > 0).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_SetCfgPin (void *ctx, ptx_HAL_DRV_GPIO_Cfg_t *cfg, uint32_t cfgLen);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

