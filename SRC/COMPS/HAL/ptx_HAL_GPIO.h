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
    Module      : HAL GPIO Interface
    File        : ptx_HAL_GPIO.h

    Description : HAL GPIO Interface
                  Implementation of an abstract HAL GPIO interface.
*/

/**
 * \addtogroup HAL_IF_GPIO
 *
 * @{
 */

#if !defined(PTX_HAL_GPIO_H)
#define PTX_HAL_GPIO_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_DRV_GPIO.h"
#include "ptx_Log.h"


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
 * \brief HAL GPIO Open Datatype
 *
 * Callback used by a GPIO to open the GPIO IP
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_GPIO_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_GPIO_CbOpen_t) (void *ctx);

/**
 * \brief HAL GPIO Close Datatype
 *
 * Callback used by a GPIO to Close the GPIO IP
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_GPIO_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_GPIO_CbClose_t) (void *ctx);

/**
 * \brief HAL GPIO Read Pin Datatype
 *
 * Callback used to read the given pin from the GPIO
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_GPIO_t.
 * \param[in]       pin             Pin to read from
 * \param[out]      level           Value of the pin, LOW = 0, HIGH = !LOW
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_GPIO_CbReadPin_t) (void *ctx, uint16_t pin, uint8_t *level);

/**
 * \brief HAL GPIO Write Pin Datatype
 *
 * Callback used to write the given pin from the GPIO
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_GPIO_t.
 * \param[in]       pin             Pin to read from
 * \param[in]       level           Value of the pin, LOW = 0, HIGH = !LOW
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_GPIO_CbWritePin_t) (void *ctx, uint16_t pin, uint8_t level);

/**
 * \brief HAL GPIO Read Port Datatype
 *
 * Callback used to read the given port from the GPIO
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_GPIO_t.
 * \param[in]       port            Port to read from
 * \param[out]      data            Value of the port, LOW = 0, HIGH = !LOW
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_GPIO_CbReadPort_t) (void *ctx, uint32_t port, uint16_t *data);

/**
 * \brief HAL GPIO Write Port Datatype
 *
 * Callback used to write the given port from the GPIO
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_GPIO_t.
 * \param[in]       port            Port to read from
 * \param[in]       data            Value of the port, LOW = 0, HIGH = !LOW
 * \param[in]       mask            Mask for the pins on the port
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_GPIO_CbWritePort_t) (void *ctx, uint32_t port, uint16_t data, uint16_t mask);

/**
 * \brief HAL GPIO to configure a pin Datatype
 *
 * Callback used to wait actively for an IRQ
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_GPIO_t.
 * \param[in]       cfg             Array with port configurations
 * \param[in]       cfgLen          Length/size of the configuration array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_GPIO_CbSetCfgPin_t) (void *ctx, ptx_HAL_DRV_GPIO_Cfg_t cfg[], uint32_t cfgLen);

/**
 * \brief Main Component Struture
 */
typedef struct ptx_HAL_GPIO
{
    ptx_Comps_t                 CompId;       /**< Component ID. */

    ptx_HAL_DRV_GPIO_t          Drv;          /**< GPIO Driver. */

    ptx_HAL_GPIO_CbOpen_t       Open;         /**< GPIO Open Callback. */
    ptx_HAL_GPIO_CbClose_t      Close;        /**< GPIO Close Callback. */
    ptx_HAL_GPIO_CbReadPin_t    ReadPin;      /**< GPIO Read Pin Callback. */
    ptx_HAL_GPIO_CbWritePin_t   WritePin;     /**< GPIO Write Pin Callback. */
    ptx_HAL_GPIO_CbReadPort_t   ReadPort;     /**< GPIO Read Port Callback. */
    ptx_HAL_GPIO_CbWritePort_t  WritePort;    /**< GPIO Write Port Callback. */
    ptx_HAL_GPIO_CbSetCfgPin_t  SetCfg;       /**< GPIO Set Configuration Callback. */

} ptx_HAL_GPIO_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_GPIO_InitParams
{
    ptx_Log_t                   *Log;                   /**< Logger context. */
} ptx_HAL_GPIO_InitParams_t;

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
ptx_Status_t ptx_HAL_GPIO_Init (ptx_HAL_GPIO_t *ctx, ptx_HAL_GPIO_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       ctx            Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_GPIO_DeInit (ptx_HAL_GPIO_t *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

