/*
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
    Module      : MCU-RA2E2 GPIO Driver
    File        : ptx_HAL_DRV_GPIO.h

    Description : Implementation of the GPIO Driver for the MCU-RA2E2 Reference Platform
*/

/**
 * \addtogroup DRV_RA2E2_IMPL_GPIO
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
#include "ptx_Log.h"
#include "ptx_HAL_DRV_GPIO_Int.h"
#include "hal_data.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

#define PTX_HAL_DRV_GPIO_RESET_PIN          BSP_IO_PORT_01_PIN_10   /**< MCU GPIO pin used to hard-reset PTX chip. */
#define PTX_HAL_GPIO_PINCFG_OUTPUT_LOW      ((uint32_t) IOPORT_CFG_PORT_DIRECTION_OUTPUT | (uint32_t) IOPORT_CFG_PORT_OUTPUT_LOW)   /**< GPIO set to output-low */
#define PTX_HAL_GPIO_PINCFG_INPUT           ((uint32_t) IOPORT_CFG_PORT_DIRECTION_INPUT)                                            /**< GPIO set to input */
#define PTX_HAL_GPIO_PINCFG_OUTPUT_HIGH     ((uint32_t) IOPORT_CFG_PORT_DIRECTION_OUTPUT | (uint32_t) IOPORT_CFG_PORT_OUTPUT_HIGH)  /**< GPIO set to output-high */
#define PTX_HAL_GPIO_PINCFG_EXT_IRQ         ((uint32_t) IOPORT_CFG_IRQ_ENABLE | (uint32_t) IOPORT_CFG_PORT_DIRECTION_INPUT)         /**< GPIO set to input-interrupt */
#define PTX_HAL_GPIO_PINCFG_INPUT_PULLUP    ((uint32_t) IOPORT_CFG_PULLUP_ENABLE | (uint32_t) IOPORT_CFG_PORT_DIRECTION_INPUT)      /**< GPIO set to input-pullup */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Port Configuration
 *
 * This structure contains all variables required to operate the GPIO driver from the Renesas RA2E2 MCU.
 *
 * \note Integrators must initialise this structure according to their needs! A working example can be found in the
 * corresponding c file.
 */
typedef struct ptx_HAL_DRV_GPIO_PortConfig
{
    const ioport_instance_t             *Instance;  /**< Instance */
} ptx_HAL_DRV_GPIO_PortConfig_t;


/**
 * \brief Main Component Struture
 */
typedef struct ptx_HAL_DRV_GPIO
{
    ptx_Comps_t                         CompId;     /**< Component ID */
    ptx_HAL_DRV_GPIO_PortConfig_t       *Port;      /**< Pointer to port */

    ptx_Log_t                           *Log;       /**< Logger context. */
} ptx_HAL_DRV_GPIO_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_GPIO_InitParams
{
    ptx_Log_t                   *Log;                   /**< Logger context. */
} ptx_HAL_DRV_GPIO_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_Init (ptx_HAL_DRV_GPIO_t *ctx, ptx_HAL_DRV_GPIO_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_DeInit (ptx_HAL_DRV_GPIO_t *ctx);

/**
 * \brief Opens a GPIO device
 *
 * \param[in]       ctx     Pointer to component
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_Open (void *ctx);

/**
 * \brief Closes a GPIO device
 *
 * \param[in]       ctx     Pointer to component
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_Close (void *ctx);

/**
 * \brief Reads a pin
 *
 * \param[in]       ctx     Pointer to component.
 * \param[in]       pin     Pin to read from.
 * \param[out]      level   Level of the current pin.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_ReadPin (void *ctx, uint16_t pin, uint8_t *level);

/**
 * \brief Write a pin
 *
 * \param[in]       ctx     Pointer to component.
 * \param[in]       pin     Pin to read from.
 * \param[in]       level   Level to set the pin to.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_WritePin (void *ctx, uint16_t pin, uint8_t level);

/**
 * \brief Reads data from port
 *
 * \param[in]       ctx     Pointer to component.
 * \param[in]       port    Port to read from
 * \param[out]      data    Port data
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_ReadPort (void *ctx, uint32_t port, uint16_t *data);

/**
 * \brief Writes data port
 *
 * \param[in]       ctx     Pointer to component.
 * \param[in]       port    Port to read from
 * \param[in]       data    Port data
 * \param[in]       mask    Mask of pins to read from
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_WritePort (void *ctx, uint32_t port, uint16_t data, uint16_t mask);

/**
 * \brief Sets configuration for a pin
 *
 * \param[in]       ctx     Pointer to component.
 * \param[in]       cfg     Array of configurations.
 * \param[in]       cfgLen  Length of configuration array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_GPIO_SetCfgPin (void *ctx, ptx_HAL_DRV_GPIO_Cfg_t *cfg, uint32_t cfgLen);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
