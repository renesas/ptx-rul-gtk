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
    Module      : Windows Timer Driver Common Types and Definitions
    File        : ptx_HAL_DRV_Timer_Int.h

    Description : Implementation of the Common Timer Driver Definitions and Types for the Windows Reference Platform
*/

/**
 * \addtogroup DRV_WIN_IMPL_TMR_INT
 *
 * @{
 */

#if !defined(PTX_HAL_DRV_TMR_INT_H)
#define PTX_HAL_DRV_TMR_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Status.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
#define PTX_HAL_DRV_TMR_MICROSEC_PER_SEC         (1000000)   /**< Microseconds per second. */
#define PTX_HAL_DRV_TMR_MILLISEC_PER_SEC         (1000)      /**< Milliseconds per second. */
#define PTX_HAL_DRV_TMR_DUTYCYCLE_MAXPERCENT     (100)       /**< Max duty cycle. */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Enum of Cfg Type
 */
typedef enum ptx_HAL_DRV_Tmr_CfgType
{
    HAL_DRV_Tmr_CfgType_Period,         /**< Set Period and Time Unit. */
    HAL_DRV_Tmr_CfgType_Mode,           /**< Set Timer Mode: Single shot or Periodic. */

    HAL_DRV_Tmr_CfgType_Max,            /**< Max Count. */
} ptx_HAL_DRV_Tmr_CfgType_t;

/**
 * \brief Enum of Timer Mode
 */
typedef enum ptx_HAL_DRV_Tmr_Mode
{
    HAL_DRV_Tmr_Mode_SingleShot,        /**< Single shot: Timer callback is called only once.
                                         Timer Stops automatically after the time elapses. */
    HAL_DRV_Tmr_Mode_Periodic,          /**< Periodic: Timer callback is called periodically with a preset period.
                                         Timer needs to be stopped with the call to ptx_HAL_DRV_Tmr_Stop. */

    HAL_DRV_Tmr_Mode_Max,               /**< Max Count. */
} ptx_HAL_DRV_Tmr_Mode_t;

/**
 * \brief Enum used for setting Time Units for the Timer Cfg.
 */
typedef enum ptx_HAL_DRV_Tmr_Unit
{
    HAL_DRV_Tmr_Unit_mSec,   /**< Milliseconds. */
    HAL_DRV_Tmr_Unit_Sec,    /**< Seconds. */

    HAL_DRV_Tmr_Unit_Max,    /**< Max Count. */
} ptx_HAL_DRV_Tmr_Unit_t;

/**
 * \brief HAL Timer Cfg Structure
 */
typedef struct ptx_HAL_DRV_Tmr_Cfg
{
    ptx_HAL_DRV_Tmr_CfgType_t    CfgType;        /**< Configuration Type to be performed. */

    /* set via HAL_DRV_Tmr_CfgType_Period */
    uint32_t                     Period;         /**< Period Length in Time Units. */
    ptx_HAL_DRV_Tmr_Unit_t       TimeUnit;       /**< Time Unit used. Default: milliseconds. */
    ptx_HAL_DRV_Tmr_Mode_t       Mode;           /**< Timer mode. */

} ptx_HAL_DRV_Tmr_Cfg_t;

/**
 * \brief Timer Execution Context
 *
 * This context must is needed for the \ref ptx_HAL_DRV_Tmr_ExecCb_t callback!
 *
 * \note This callback must be at least \ref ptx_HAL_Tmr_t!
 */
typedef void *ptx_HAL_DRV_Tmr_ExecCbCtx_t;

/**
 * \brief HAL Timer Execution Datatype for Timer events
 *
 * Callback used by a timer when special events, like timer-elapsed, are reached.
 *
 * \param[in]       ctx             Context handed to the callback.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_DRV_Tmr_ExecCb_t) (ptx_HAL_DRV_Tmr_ExecCbCtx_t ctx);

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

