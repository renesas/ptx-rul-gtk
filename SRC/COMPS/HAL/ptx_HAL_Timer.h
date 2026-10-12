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
    Module      : HAL Timer Interface
    File        : ptx_HAL_Tmr.h

    Description : HAL Timer Interface
                  Implementation of an abstract interface to access timers.
*/

/**
 * \addtogroup HAL_IF_TMR
 *
 * @{
 */

#if !defined(PTX_HAL_TMR_H)
#define PTX_HAL_TMR_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Components.h"
#include "ptx_Status.h"

#include "ptx_HAL_DRV_Timer_Int.h"
#include "ptx_HAL_DRV_Timer.h"

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
 * \brief HAL Timer Open Datatype
 *
 * Callback used by HAL to open a timer instance.
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_Tmr_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Tmr_OpenCb_t) (void *ctx);

/**
 * \brief HAL Timer Close Datatype
 *
 * Callback used by HAL to close a timer instance.
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_Tmr_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Tmr_CloseCb_t) (void *ctx);

/**
 * \brief HAL Timer Start Datatype
 *
 * Callback used by HAL to start timer counter.
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_Tmr_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Tmr_StartCb_t) (void *ctx);

/**
 * \brief HAL Timer Stop Datatype
 *
 * Callback used by HAL to stop timer counter.
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_Tmr_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Tmr_StopCb_t) (void *ctx);

/**
 * \brief HAL Timer IsElapsed Datatype
 *
 * Callback used by HAL when special events, like timer-elapsed, are reached.
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_Tmr_t.
 * \param[in]       isElapsed       Pointer to status information storage.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Tmr_IsElapsed_t) (void *ctx, uint8_t *isElapsed);

/**
 * \brief HAL Timer Register Execution Callback Datatype
 *
 * Callback used by HAL to register higher layer callback function within timer module.
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_Tmr_t.
 * \param[in]       cb              Callback function to be registered.
 * \param[in]       cbCtx           Context for the Execution Callback function.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Tmr_RegExecCb_t) (void *ctx, ptx_HAL_DRV_Tmr_ExecCb_t cb, ptx_HAL_DRV_Tmr_ExecCbCtx_t cbCtx);

/**
 * \brief HAL Timer Configuration Datatype
 *
 * Callback used by HAL to configure a timer.
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_Tmr_t.
 * \param[in]       cfg             Array of \ref ptx_HAL_DRV_Tmr_Cfg_t configuration parameters.
 * \param[in]       len             Length of the Configuration Array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_DRV_Tmr_CfgCb_t) (void *ctx, ptx_HAL_DRV_Tmr_Cfg_t cfg[], uint32_t len);

/**
 * \brief HAL Timer Sleep
 *
 * Callback used by HAL to add a delay/sleep time.
 *
 * \param[in]       ctx             Context handed to the callback. \note Must be at least \ref ptx_HAL_DRV_Tmr_t.
 * \param[in]       timeout         Time to sleep, in milliseconds.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Tmr_SleepCb_t) (void *ctx, uint32_t timeout);


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Enum of Timers Ids
 */
typedef enum ptx_HAL_Tmr_Id
{
    HAL_Tmr_Id_0   = 0, /**< Timer 0. */
    HAL_Tmr_Id_1   = 1, /**< Timer 1. */
    HAL_Tmr_Id_MAX = 2  /**< Max number of timers. */
} ptx_HAL_Tmr_Id_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL_Tmr
{

    ptx_Comps_t                  CompId;         /**< Component ID. */

    ptx_HAL_DRV_Tmr_t            Drv;            /**< Timer Driver. */

    ptx_HAL_Tmr_OpenCb_t         Open;           /**< Callback function to Open timer. */
    ptx_HAL_Tmr_CloseCb_t        Close;          /**< Callback function to Close timer. */
    ptx_HAL_Tmr_StartCb_t        Start;          /**< Callback function to Start the timer. */
    ptx_HAL_Tmr_StopCb_t         Stop;           /**< Callback function to Stop timer. */
    ptx_HAL_Tmr_IsElapsed_t      IsElapsed;      /**< Callback function to check if timer is elapsed. */
    ptx_HAL_Tmr_RegExecCb_t      RegExecCb;      /**< Callback function to register an callback for timer execution. */
    ptx_HAL_DRV_Tmr_CfgCb_t      Cfg;            /**< Callback function to configure the timer setting. */
    ptx_HAL_Tmr_SleepCb_t        Sleep;          /**< Callback function for Sleep/Delay functionality. */
    ptx_Log_t                    *Log;           /**< Initialised Log Context */


} ptx_HAL_Tmr_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_Tmr_InitParams
{

#if defined(PTX_APP_DIVERSITY_WINDOWS) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
    void                        *Osal;          /**< Osal context - used by specific implementations. */
#endif
    void 			            *RFU;           /**< Empty struct not allowed in clang */
    ptx_Log_t                   *Log;           /**< Initialised Log Context */

} ptx_HAL_Tmr_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[out]      ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Tmr_Init (ptx_HAL_Tmr_t *ctx, ptx_HAL_Tmr_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in,out]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Tmr_DeInit (ptx_HAL_Tmr_t *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

