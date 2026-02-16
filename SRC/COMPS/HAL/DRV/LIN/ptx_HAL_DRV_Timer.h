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
 * \brief Main Component Struture
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
} ptx_HAL_DRV_Tmr_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Tmr_InitParams
{
    uint8_t     Id;             /**< Timer ID. */
    void        *Osal;          /**< Pointer to initialised Osal context. */
} ptx_HAL_DRV_Tmr_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Tmr_Init (ptx_HAL_DRV_Tmr_t *ctx, ptx_HAL_DRV_Tmr_InitParams_t *initParams);

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

