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
    Module      : MCU-RA4M2 Timer Driver
    File        : ptx_HAL_DRV_Timer.h

    Description : Implementation of the Timer Driver for the MCU-RA4M2 Reference Platform

*/

/**
 * \addtogroup DRV_RA4M2_IMPL_TMR
 *
 * @{
 */

#if !defined(PTX_HAL_DRV_TMR_H)
#define PTX_HAL_DRV_TMR_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stddef.h>
#include "ptx_Components.h"
#include "ptx_HAL_DRV_Timer_Int.h"
#include "hal_data.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
* \brief Specific GPIO definitions for Renesas RA4M2 MCU.
 * @{
*/
#define PTX_HAL_DRV_TMR_IS_ELAPSED_FALSE                (0)  /**< Timer elapsed false. */
#define PTX_HAL_DRV_TMR_IS_ELAPSED_TRUE                 (1)  /**< Timer elapsed true. */

#define PTX_HAL_DRV_TMR_MAX_PORTS                       (2u) /**< Max. timer ports. */

/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */




/**
 * \brief Port Configuration
 *
 * This structure contains all variables required to operate the Timer driver from the Renesas RA4M2 MCU.
 *
 * \note Integrators must initialise this structure according to their needs! A working example can be found in the
 * corresponding c file.
 */
typedef struct ptx_HAL_DRV_Tmr_Port
{
    const timer_instance_t              *Instance;      /**< Pointer to HW instance. */

    volatile uint8_t                    IsElapsed;      /**< Is elapsed flag, set by IRQ. */

    ptx_HAL_DRV_Tmr_Mode_t              Mode;           /**< Timer mode. */
    ptx_HAL_DRV_Tmr_ExecCb_t            ExecCb;         /**< Timer callback. */
    ptx_HAL_DRV_Tmr_ExecCbCtx_t         ExecCbCtx;      /**< Timer callback ctx. */

    uint8_t                             TimerAssigned;  /**< Flag which indicates if the Timer is already assigned. */
} ptx_HAL_DRV_Tmr_Port_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL_DRV_Tmr
{
    ptx_Comps_t                 CompId;     /**< Component ID. */

    uint8_t                     Id;         /**< Timer Port Id.*/

    ptx_HAL_DRV_Tmr_Port_t      *Port;      /**< Pointer to initialised timer. */
} ptx_HAL_DRV_Tmr_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Tmr_InitParams
{
#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX)
    void        *Osal;      /**< Pointer to OSAL context. */
#endif
} ptx_HAL_DRV_Tmr_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[in]       ctx   Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Init (ptx_HAL_DRV_Tmr_t *ctx, ptx_HAL_DRV_Tmr_InitParams_t *initParams );

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_DeInit (ptx_HAL_DRV_Tmr_t *ctx);

/**
 * \brief Timer Open.
 *
 * \param[in]       ctx   Pointer to function context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Open (void *ctx);

/**
 * \brief Timer Close.
 *
 * \param[in]       ctx   Pointer to function context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Close (void *ctx);

/**
 * \brief Timer Start.
 *
 * \param[in]       ctx   Pointer to function context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Start (void *ctx);

/**
 * \brief Timer Stop.
 *
 * \param[in]       ctx   Pointer to function context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Stop (void *ctx);

/**
 * \brief Timer IsElapsed request.
 *
 * \param[in]       ctx         Pointer to function context.
 * \param[in]       isElapsed   Pointer to variable for storing status information.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_IsElapsed (void *ctx, uint8_t *isElapsed);

/**
 * \brief Timer Execution callback registration.
 *
 * \param[in]       ctx   Pointer to function context.
 * \param[in]       cb    Callback function to be registered.
 * \param[in]       cbCtx Context for Callback function.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_RegExecCb (void *ctx, ptx_HAL_DRV_Tmr_ExecCb_t cb, ptx_HAL_DRV_Tmr_ExecCbCtx_t cbCtx);

/**
 * \brief Timer Configuration.
 *
 * \param[in]       ctx   Pointer to function context.
 * \param[in]       cfg   Array of configurations to be set.
 * \param[in]       len   Length of configuration array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Cfg (void *ctx, ptx_HAL_DRV_Tmr_Cfg_t cfg[], uint32_t len);

/**
 * \brief Sleep/Delay functionality.
 *
 * \param[in]       ctx         Pointer to function context.
 * \param[in]       timeout     Time to sleep, in milliseconds.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Sleep (void *ctx, uint32_t timeout);

#if defined(__cplusplus)
}
#endif


#endif /* Guard */

/** @} */

