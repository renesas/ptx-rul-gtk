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
    Module      : Windows GPIO Driver Common Types and Definitions
    File        : ptx_HAL_DRV_GPIO_Int.h

    Description : Implementation of the Common GPIO Driver Definitions and Types for the Windows Reference Platform
*/

/**
 * \addtogroup DRV_WIN_IMPL_GPIO_INT
 *
 * @{
 */

#if !defined(PTX_HAL_DRV_GPIO_INT_H)
#define PTX_HAL_DRV_GPIO_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Status.h"
#include "ptx_HAL_Ftd2xx.h"

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
 * \brief Forward declaration for GPIO IRQ/ISR callback context.
 */
typedef void *ptx_HAL_DRV_GPIO_IrqCbCtx_t;

/**
 * \brief Forward declaration for GPIO IRQ/ISR callback function.
 */
typedef ptx_Status_t (*ptx_HAL_DRV_GPIO_IrqCb_tx_t) (ptx_HAL_DRV_GPIO_IrqCbCtx_t ctx);

/**
 * \brief HAL GPIO Configuration Structure
 */
typedef enum ptx_HAL_DRV_GPIO_CfgParam
{
    HAL_DRV_GPIO_CfgParam_PinDirection,
    HAL_DRV_GPIO_CfgParam_EnableExtIrq,
    HAL_DRV_GPIO_CfgParam_PinPullUpEnable,
} ptx_HAL_DRV_GPIO_CfgParam_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL_DRV_GPIO_Cfg
{
    ptx_HAL_DRV_GPIO_CfgParam_t Key;        /**< GPIO Configuration. */
    ptx_HAL_FTD2XX_GPIO_t       Pin;        /**< GPIO pin assignment. */
    uint32_t                    Value;      /**< GPIO pin value. */
} ptx_HAL_DRV_GPIO_Cfg_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

