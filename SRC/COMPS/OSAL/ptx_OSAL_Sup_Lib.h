/** \file
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
    Module      : Operating System Abstraction Layer (OSAL) Support Library
    File        : ptx_OSAL_Sup_Lib.h

    Description : Contains utility functions to get metadata about OSAL items.
                  Operating system specific implementations are available for
                  Windows and Linux.
*/

/**
 * \addtogroup OSAL
 *
 * @{
 */

#if !defined(PTX_OSAL_SUP_LIB_H)
#define PTX_OSAL_SUP_LIB_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_OSAL.h"

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
 *  \brief Timer procedure function pointer declaration.
 *
 *  \param      timerParam Pointer to timer-specific ("context") storage.
 */
typedef void (*pptx_OSAL_TimerProc_t)(void *timerParam);

/**
 * \brief Object types
 *
 * Used in API \ref ptx_OSAL_GetUsage to get current number of corresponding Osal items used.
 * If used in \ref ptx_OSAL_GetPeakUsage, it corresponds to the Osal item peak usage number.
 */
typedef enum ptx_OSAL_ItemType
{
    ptx_OSAL_ItemType_Sem,                    /**< Get item: Number of semaphores allocated. */
    ptx_OSAL_ItemType_Mutex,                  /**< Get item: How many mutexes are currently in use. */
    ptx_OSAL_ItemType_Thread,                 /**< Get item: Present thread count. */
    ptx_OSAL_ItemType_Timer,                  /**< Get item: Number of timers currently in use. */
    ptx_OSAL_ItemType_Count,                  /**< Number of members of this enum type. */
} ptx_OSAL_ItemType_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Get OSAL Item value.
 *
 * \param[in]       osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]       itemType        Type of the OSAL Item to get.
 * \param[out]      itemUsage       The value of the OSAL Item.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_OSAL_GetUsage(ptx_OSAL_t *osalCtx, ptx_OSAL_ItemType_t itemType, int32_t *itemUsage);

/**
 * \brief Get OSAL Item peak value.
 *
 * \param[in]       osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in]       itemType        Type of the OSAL Item to get.
 * \param[out]      itemUsage       The value of the OSAL Item.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_OSAL_GetPeakUsage(ptx_OSAL_t *osalCtx, ptx_OSAL_ItemType_t itemType, int32_t *itemUsage);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
