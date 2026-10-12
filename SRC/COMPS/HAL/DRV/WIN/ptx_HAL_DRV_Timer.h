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
    Module      : Windows Timer Driver
    File        : ptx_HAL_DRV_Timer.h

    Description : Implementation of the Timer Driver for the Windows Reference Platform using FTDI

*/


/**
 * \addtogroup DRV_WIN_IMPL_TMR
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
#define PTX_HAL_DRV_TMR_IS_ELAPSED_FALSE (0)     /**< Internal Timer elapsed flag (0) */
#define PTX_HAL_DRV_TMR_IS_ELAPSED_TRUE  (1)     /**< Internal Timer elapsed flag (1) */
#define PTX_HAL_DRV_TMR_MAX_PORTS        (2u)    /**< Max. timer ports */
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
    ptx_Log_t                           *Log;               /**< Pointer to an initialized logger. */

} ptx_HAL_DRV_Tmr_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Tmr_InitParams
{
    uint8_t     Id;             /**< Timer ID. */
    void        *Osal;          /**< Pointer to initialized Osal context. */
    ptx_Log_t   *Log;           /**< Pointer to an initialized logger. */

} ptx_HAL_DRV_Tmr_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Component Initialization.
 *
 * Initializes the HAL Timer Driver component with required system resources.
 * Validates initialization parameters and establishes connection to the OSAL layer
 * for underlying timer functionality.
 *
 * \param[out]      ctx             Pointer to uninitialized HAL Timer Driver component.
 * \param[in]       initParams      Pointer to initialization parameters containing:
 *                                 - Id: Timer ID (reserved for future use)
 *                                 - Osal: Pointer to initialized OSAL context (required)
 *                                 - Log: Pointer to initialized logger (required)
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 * \note Component state is zeroed during initialization.
 * \note OSAL context must be initialized before calling this function.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Init (ptx_HAL_DRV_Tmr_t *ctx, ptx_HAL_DRV_Tmr_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * Releases all resources allocated to the HAL Timer Driver component, including
 * any active timers. Closes the underlying OSAL timer and clears component state.
 *
 * \param[in,out]       ctx             Pointer to initialized HAL Timer Driver component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 * \note Component state is zeroed after de-initialization.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_DeInit (ptx_HAL_DRV_Tmr_t *ctx);

/**
 * \brief Timer Open.
 *
 * Creates and initializes a timer resource via the OSAL layer. Must be called
 * before starting the timer. Registers the internal timer callback handler.
 *
 * \param[in,out]       ctx             Pointer to initialized HAL Timer Driver component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 * \note Must be called before ptx_HAL_DRV_Tmr_Start.
 * \note Timer configuration (period, mode) should be set before opening.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Open (void *ctx);

/**
 * \brief Timer Close.
 *
 * Closes and releases the timer resource. Stops any running timer and deallocates
 * associated OSAL resources.
 *
 * \param[in,out]       ctx             Pointer to initialized HAL Timer Driver component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 * \note Safe to call on already-closed timers.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Close (void *ctx);

/**
 * \brief Timer Start.
 *
 * Starts the timer with the configured period and mode. Resets the elapsed flag
 * and initiates the underlying OSAL timer.
 *
 * \param[in]       ctx             Pointer to initialized HAL Timer Driver component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 * \note Timer must be opened via ptx_HAL_DRV_Tmr_Open before starting.
 * \note Period must be configured via ptx_HAL_DRV_Tmr_Cfg before starting.
 * \note IsElapsed flag is cleared when timer starts.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Start (void *ctx);

/**
 * \brief Timer Stop.
 *
 * Stops a running timer and cancels any pending timeout. Has no effect if the
 * timer is not currently running.
 *
 * \param[in]       ctx             Pointer to initialized HAL Timer Driver component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Stop (void *ctx);

/**
 * \brief Timer IsElapsed Query.
 *
 * Queries the current state of the timer elapsed flag. The flag is set to true
 * when the timer expires and is cleared when the timer is started.
 *
 * \param[in]       ctx             Pointer to initialized HAL Timer Driver component.
 * \param[out]      isElapsed       Pointer to variable for storing elapsed status:
 *                                 - PTX_HAL_DRV_TMR_IS_ELAPSED_FALSE (0): Timer has not elapsed
 *                                 - PTX_HAL_DRV_TMR_IS_ELAPSED_TRUE (1): Timer has elapsed
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 * \note Non-destructive read; does not clear the elapsed flag.
 * \note Flag is set in the internal timer callback routine.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_IsElapsed (void *ctx, uint8_t *isElapsed);

/**
 * \brief Timer Execution Callback Registration.
 *
 * Registers a callback function to be invoked when the timer expires. The callback
 * is executed within the internal timer callback routine.
 *
 * \param[in]       ctx             Pointer to initialized HAL Timer Driver component.
 * \param[in]       cb              Callback function pointer to be invoked on timer expiration.
 * \param[in]       cbCtx           Context pointer passed to the callback function.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_RegExecCb (void *ctx, ptx_HAL_DRV_Tmr_ExecCb_t cb, ptx_HAL_DRV_Tmr_ExecCbCtx_t cbCtx);

/**
 * \brief Timer Configuration.
 *
 * Configures timer parameters via an array of configuration structures. Supports
 * setting period (with time unit conversion) and timer mode (periodic or single-shot).
 *
 * \param[in]       ctx             Pointer to initialized HAL Timer Driver component.
 * \param[in]       cfg             Array of configuration structures to be applied.
 * \param[in]       len             Number of configuration entries in the array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *         Returns PTX_STATUS_E_GEN_INVALID_PARAMETER if:
 *         - ctx is NULL or not properly initialized (CompId != Comp_HAL)
 *         - cfg is NULL
 *         - len is 0
 *         - Period is 0 (for period configuration)
 *         - TimeUnit is not HAL_DRV_Tmr_Unit_Sec or HAL_DRV_Tmr_Unit_mSec
 *         - Mode is not HAL_DRV_Tmr_Mode_Periodic or HAL_DRV_Tmr_Mode_SingleShot
 *         Returns PTX_STATUS_SUCCESS if all configurations are applied successfully.
 *
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Cfg (void *ctx, ptx_HAL_DRV_Tmr_Cfg_t cfg[], uint32_t len);

/**
 * \brief Sleep/Delay Functionality.
 *
 * Blocks execution for the specified duration by delegating to the OSAL thread
 * sleep function. Provides a convenient way to implement delays using the timer
 * infrastructure.
 *
 * \param[in]       ctx             Pointer to initialized HAL Timer Driver component.
 * \param[in]       timeout         Time to sleep, in milliseconds. If 0, returns immediately.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Sleep (void *ctx, uint32_t timeout);

#if defined(__cplusplus)
}
#endif


#endif /* Guard */

/** @} */

