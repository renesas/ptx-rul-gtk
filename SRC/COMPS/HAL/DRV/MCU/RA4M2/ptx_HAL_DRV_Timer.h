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
    
    ptx_Log_t                   *Log;       /**< Pointer to logger context. */

} ptx_HAL_DRV_Tmr_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Tmr_InitParams
{
#if defined(PTX_APP_DIVERSITY_WINDOWS) || defined(PTX_APP_DIVERSITY_LINUX)
    void        *Osal;      /**< Pointer to OSAL context. */

#endif
    ptx_Log_t                   *Log;       /**< Pointer to logger context. */
} ptx_HAL_DRV_Tmr_InitParams_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * Initializes the Timer driver component. Validates input parameters and searches for an available
 * (unassigned) timer port from the pool of configured timer instances. Marks the selected timer port
 * as assigned to prevent reuse. The Log context must be a valid logger instance. Osal (Windows/Linux/AOSP only) pointer to valid OSAL context.
 *
 * \param[out]      ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Init (ptx_HAL_DRV_Tmr_t *ctx, ptx_HAL_DRV_Tmr_InitParams_t *initParams );

/**
 * \brief Component De-Initialization.
 *
 * Clears all component data, releases the assigned timer port, and resets component ID to Comp_None.
 * Clears any registered callback and callback context. The timer port is marked as unassigned and
 * becomes available for reuse by other components.
 *
 * \param[in,out]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_DeInit (ptx_HAL_DRV_Tmr_t *ctx);

/**
 * \brief Opens the timer hardware interface.
 *
 * Opens the GPT (General Purpose Timer) hardware block and configures it according to the port configuration.
 * If the timer is already open (FSP_ERR_ALREADY_OPEN), it will be closed first and then reopened to ensure
 * a clean state. Registers the timer interrupt service routine (ISR) callback for timer events.
 *
 * \param[in,out]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Open (void *ctx);

/**
 * \brief Closes the timer hardware interface.
 *
 * Closes the GPT hardware block and releases associated resources. Returns success if the timer is
 * successfully closed or if it was not open (FSP_ERR_NOT_OPEN).
 *
 * \param[in,out]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Close (void *ctx);
/**
 * \brief Starts the timer.
 *
 * Starts the timer counting. Resets the IsElapsed flag to FALSE before starting. The timer will
 * count according to its configured period and mode (periodic or single-shot). Timer interrupts
 * will be generated when the configured period expires.
 *
 * \param[in]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Start (void *ctx);

/**
 * \brief Stops the timer.
 *
 * Stops the timer from counting. The timer can be restarted by calling ptx_HAL_DRV_Tmr_Start.
 * The IsElapsed flag is not modified by this function.
 *
 * \param[in]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Stop (void *ctx);

/**
 * \brief Checks if the timer has elapsed.
 *
 * Reads the IsElapsed flag which is set by the timer interrupt service routine when the configured
 * period expires. The flag is set to TRUE when the timer elapses and remains TRUE until the timer
 * is started again (which resets it to FALSE).
 *
 * \param[in]       ctx         Pointer to component.
 * \param[out]      isElapsed   Pointer to variable for storing elapsed status (0 = not elapsed, 1 = elapsed).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_IsElapsed (void *ctx, uint8_t *isElapsed);

/**
 * \brief Registers a timer execution callback.
 *
 * Registers a callback function and its context to be executed when the timer elapses. The callback
 * is invoked from the timer interrupt service routine. Both the callback function and context must
 * be non-NULL. The callback behavior depends on the timer mode:
 * - Periodic mode: Callback is executed each time the timer period expires
 * - Single-shot mode: Callback is executed only once when the timer period expires
 *
 * \param[in]       ctx   Pointer to component.
 * \param[in]       cb    Callback function to be registered (must not be NULL).
 * \param[in]       cbCtx Context for callback function (must not be NULL).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_RegExecCb (void *ctx, ptx_HAL_DRV_Tmr_ExecCb_t cb, ptx_HAL_DRV_Tmr_ExecCbCtx_t cbCtx);

/**
 * \brief Configures timer parameters.
 *
 * Applies one or more configuration parameters to the timer. Supported configuration types include:
 * - HAL_DRV_Tmr_CfgType_Period: Sets the timer period with time unit (milliseconds or seconds).
 *   The period is calculated based on the PCLKD clock frequency and the configured clock divider.
 * - HAL_DRV_Tmr_CfgType_Mode: Sets the timer mode (periodic or single-shot).
 *
 * Configuration is applied sequentially; if any configuration fails, the operation aborts and
 * returns an error status.
 *
 * \param[in]       ctx   Pointer to component.
 * \param[in]       cfg   Array of configurations to be applied.
 * \param[in]       len   Length of configuration array (must be > 0).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Cfg (void *ctx, ptx_HAL_DRV_Tmr_Cfg_t cfg[], uint32_t len);

/**
 * \brief Performs a blocking sleep/delay.
 *
 * Blocks execution for the specified duration in milliseconds. Configures the timer with the requested
 * timeout period, starts it, and waits in a low-power loop until the timer elapses. Uses interrupt-driven
 * synchronization with MCU wait-for-interrupt (WFI) to minimize power consumption during the delay.
 *
 * The function performs the following steps:
 * 1. Stops any running timer
 * 2. Configures the timer period to the requested timeout in milliseconds
 * 3. Starts the timer
 * 4. Waits in a loop, disabling/enabling interrupts and using WFI for low-power operation
 * 5. Stops the timer when elapsed
 *
 * \param[in]       ctx         Pointer to component.
 * \param[in]       timeout     Time to sleep, in milliseconds (must be > 0).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Sleep (void *ctx, uint32_t timeout);

#if defined(__cplusplus)
}
#endif


#endif /* Guard */

/** @} */
