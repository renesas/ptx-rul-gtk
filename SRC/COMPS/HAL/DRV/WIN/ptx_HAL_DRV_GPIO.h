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
    Module      : Windows GPIO Driver
    File        : ptx_HAL_DRV_GPIO.h

    Description : Implementation of the GPIO Driver for the Windows Reference Platform using FTDI
*/


/**
 * \addtogroup DRV_WIN_IMPL_GPIO
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
#include "ptx_HAL_Ftd2xx.h"
#include "ptx_HAL_WinUtils.h"

#include "ptx_HAL_DRV_GPIO_Int.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
#define PTX_HAL_DRV_GPIO_RESET_PIN              1   /**< GPIO pin used to hard-reset PTX chip */

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
    ptx_HAL_FTD2XXCtx_t                 *Ftd2xx_ctx;    /**< FTDI context */
    ptx_Log_t                           *Log;           /**< Pointer to an initialized logger. */
    uint8_t                             Gpio_only;      /**< Flag to indicate if GPIO interface is used standalone or in combination with communication driver (SPI/ I2C) */
}ptx_HAL_DRV_GPIO_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_GPIO_InitParams
{
    ptx_HAL_DRV_GPIO_Cfg_t  *Cfg;           /**< GPIO configuration */
    uint32_t                CfgLen;         /**< GPIO configuration size */
    ptx_HAL_FTD2XXCtx_t     *Ftd2xx_ctx;    /**< FTDI context */
    ptx_Log_t               *Log;           /**< Pointer to an initialized logger. */
    uint8_t                 Gpio_only;      /**< Flag to indicate if GPIO interface is used standalone or in combination with communication driver (SPI/ I2C) */
}ptx_HAL_DRV_GPIO_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * Initializes the GPIO driver component. Sets up the component ID, logger reference,
 * and GPIO usage mode (standalone or combined with communication driver).
 *
 * \param[out]      ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_Init (ptx_HAL_DRV_GPIO_t *ctx, ptx_HAL_DRV_GPIO_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * Clears all component data and resets component ID to Comp_None.
 *
 * \param[in,out]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_DeInit (ptx_HAL_DRV_GPIO_t *ctx);

/**
 * \brief Opens the GPIO device.
 *
 * Opens the FTDI device for GPIO operations. Behavior depends on platform and Gpio_only flag:
 * - On non-UART platforms: Opens FTDI library and MPSSE engine
 * - On UART platforms: Only succeeds if Gpio_only == 0 (GPIO-only mode not supported)
 *
 * \param[in,out]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_Open (void *ctx);

/**
 * \brief Closes the GPIO device.
 *
 * Closes the FTDI device. On non-UART platforms, only closes if Gpio_only flag is set.
 * On UART platforms, this is a no-op.
 *
 * \param[in,out]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_Close (void *ctx);

/**
 * \brief Reads the value of a given GPIO pin.
 *
 * Reads the current level of a specified GPIO pin (0-7).
 * Not available on UART platform.
 *
 * \param[in]       ctx   Pointer to component.
 * \param[in]       pin   Pin number to read (0-7).
 * \param[out]      level Read GPIO pin value (0 or 1).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_ReadPin (void *ctx, uint16_t pin, uint8_t *level);

/**
 * \brief Writes a value to a given GPIO pin.
 *
 * Sets the output level of a specified GPIO pin (0-7).
 * On UART platform, only pin 1 (reset pin) is supported via RTS control:
 * - level 0: Sets RTS (reset active)
 * - level 1: Clears RTS (reset inactive)
 *
 * \param[in]       ctx   Pointer to component.
 * \param[in]       pin   Pin number to write (0-7, or 1 for reset on UART).
 * \param[in]       level GPIO pin value to write (0 or 1).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_WritePin (void *ctx, uint16_t pin, uint8_t level);

/**
 * \brief Reads a complete GPIO port.
 *
 * Reads all GPIO pins in a single port operation. Note: There is only one port
 * containing all 8 available GPIO pins (port parameter is ignored).
 * Not available on UART platform.
 *
 * \param[in]       ctx   Pointer to component.
 * \param[in]       port  Port number to read (ignored, only one port available).
 * \param[out]      data  Read port data (8-bit GPIO state).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_ReadPort (void *ctx, uint32_t port, uint16_t *data);

/**
 * \brief Writes to a complete GPIO port.
 *
 * Writes to all GPIO pins in a single port operation with optional mask.
 * Note: There is only one port containing all 8 available GPIO pins (port parameter is ignored).
 * Not available on UART platform.
 *
 * \param[in]       ctx   Pointer to component.
 * \param[in]       port  Port number to write (ignored, only one port available).
 * \param[in]       data  Port data to write (8-bit GPIO values).
 * \param[in]       mask  Write mask (selects which pins to update).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_WritePort (void *ctx, uint32_t port, uint16_t data, uint16_t mask);

/**
 * \brief Apply GPIO pin configuration.
 *
 * Applies GPIO pin configuration. Currently not used and always returns
 * PTX_STATUS_E_GEN_NOT_AVAILABLE on all platforms.
 *
 * \param[in]       ctx     Pointer to component.
 * \param[in]       cfg     GPIO configuration array (currently unused).
 * \param[in]       cfgLen  Size of GPIO configuration array (currently unused).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_SetCfgPin (void *ctx, ptx_HAL_DRV_GPIO_Cfg_t cfg[], uint32_t cfgLen);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

