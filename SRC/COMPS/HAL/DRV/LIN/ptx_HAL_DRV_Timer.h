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
    Module      : Linux Timer Driver
    File        : ptx_HAL_DRV_Timer.h

    Description : Implementation of the Timer Driver for the Linux Reference Platform
*/


/**
 * \addtogroup DRV_LIN_IMPL_TMR
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

#include "ptx_Components.h"
#include "ptx_HAL_DRV_Timer_Int.h"
#include "ptx_OSAL.h"
#include "ptx_Log.h"
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
* \brief HAL Timer specific definitions.
 * @{
*/
#define PTX_HAL_DRV_TMR_IS_ELAPSED_FALSE                (0)     /**< Internal Timer elapsed flag (0). */
#define PTX_HAL_DRV_TMR_IS_ELAPSED_TRUE                 (1)     /**< Internal Timer elapsed flag (1). */
#define PTX_HAL_DRV_TMR_MAX_PORTS                       (2u)    /**< Max. timer ports. */
/** @} */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL_DRV_Tmr
{
    ptx_Comps_t                         CompId;             /**< Component id. */
    void                                *Osal;              /**< OSAL context - providing access to underlying system features. */
    ptx_Timer_t                         Timer;             /**< Timer context - reference to timer abstraction structure instance. */
    uint32_t                            Period;             /**< Period of timer in ms. */
    ptx_HAL_DRV_Tmr_Mode_t              Mode;               /**< Timer mode. */
    volatile uint8_t                    IsElapsed;          /**< Is elapsed flag, set in Internal Timer callback routine. */
    ptx_HAL_DRV_Tmr_ExecCb_t            ExecCb;             /**< Timer callback registered by the timer user (upper layer routine).
                                                                 Called within Internal timer callback. */
    ptx_HAL_DRV_Tmr_ExecCbCtx_t         ExecCbCtx;          /**< Timer callback ctx used in the registered timer callback. */
    ptx_Log_t                           *Log;               /**< Pointer to logger context. */
} ptx_HAL_DRV_Tmr_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Tmr_InitParams
{
    uint8_t     Id;             /**< Timer ID. */
    void        *Osal;          /**< Pointer to initialised Osal context. */
    ptx_Log_t   *Log;           /**< Pointer to logger context. */

} ptx_HAL_DRV_Tmr_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * Initializes the Timer driver component. Validates OSAL context and sets up
 * component references. The OSAL context must be a valid, initialized OSAL instance.
 *
 * \param[out]      ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Init (ptx_HAL_DRV_Tmr_t *ctx, ptx_HAL_DRV_Tmr_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * Closes any open timer resources and clears all component data.
 * Resets component ID to Comp_None.
 *
 * \param[in,out]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_DeInit (ptx_HAL_DRV_Tmr_t *ctx);

/**
 * \brief Timer Open.
 *
 * Creates and initializes the underlying OSAL timer with the internal thread procedure
 * as callback. Must be called before starting the timer.
 *
 * \param[in,out]       ctx   Pointer to function context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Open (void *ctx);

/**
 * \brief Timer Close.
 *
 * Closes and releases the underlying OSAL timer resource.
 *
 * \param[in,out]       ctx   Pointer to function context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Close (void *ctx);

/**
 * \brief Timer Start.
 *
 * Starts the timer with the configured period. Resets the IsElapsed flag to false
 * before starting. Timer mode (periodic or single-shot) must be configured
 * before calling this function.
 *
 * \param[in]       ctx   Pointer to function context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Start (void *ctx);

/**
 * \brief Timer Stop.
 *
 * Cancels/stops the running timer. Does not reset the IsElapsed flag.
 *
 * \param[in]       ctx   Pointer to function context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Stop (void *ctx);

/**
 * \brief Timer IsElapsed request.
 *
 * Checks if the timer has elapsed. For single-shot timers, provides special handling:
 * if the timer is not already marked as elapsed, not in periodic mode, and not running,
 * the IsElapsed flag is set to true (synchronization mechanism).
 *
 * \param[in]       ctx         Pointer to function context.
 * \param[out]      isElapsed   Pointer to variable for storing elapsed status (0 or 1).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_IsElapsed (void *ctx, uint8_t *isElapsed);

/**
 * \brief Timer Execution callback registration.
 *
 * Registers a callback function and its context to be invoked when the timer elapses.
 * The callback is called from within the internal timer thread procedure.
 * Both cb and cbCtx must be non-NULL.
 *
 * \param[in]       ctx   Pointer to function context.
 * \param[in]       cb    Callback function to be registered (must be non-NULL).
 * \param[in]       cbCtx Context for callback function (must be non-NULL).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_RegExecCb (void *ctx, ptx_HAL_DRV_Tmr_ExecCb_t cb, ptx_HAL_DRV_Tmr_ExecCbCtx_t cbCtx);

/**
 * \brief Timer Configuration.
 *
 * Applies timer configuration parameters. Supports the following configuration types:
 * - HAL_DRV_Tmr_CfgType_Period: Sets timer period with TimeUnit conversion
 *   - HAL_DRV_Tmr_Unit_Sec: Period is multiplied by 1000 to convert to milliseconds
 *   - HAL_DRV_Tmr_Unit_mSec: Period is used as-is
 *   - Period must be non-zero
 * - HAL_DRV_Tmr_CfgType_Mode: Sets timer mode
 *   - HAL_DRV_Tmr_Mode_Periodic: Timer repeats after each interval
 *   - HAL_DRV_Tmr_Mode_SingleShot: Timer fires once and stops
 *
 * Configuration is applied to both the driver context and the underlying OSAL timer.
 * Processing stops on first error.
 *
 * \param[in]       ctx   Pointer to function context.
 * \param[in]       cfg   Array of configurations to be set (must be non-NULL).
 * \param[in]       len   Length of configuration array (must be > 0).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Cfg (void *ctx, ptx_HAL_DRV_Tmr_Cfg_t cfg[], uint32_t len);

/**
 * \brief Sleep/Delay functionality.
 *
 * Blocks the calling thread for the specified timeout duration using OSAL thread sleep.
 * If timeout is 0, returns immediately without sleeping.
 *
 * \param[in]       ctx         Pointer to function context.
 * \param[in]       timeout     Time to sleep, in milliseconds. If 0, no sleep occurs.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Sleep (void *ctx, uint32_t timeout);

#if defined(__cplusplus)
}
#endif


#endif /* Guard */

/** @} */

