/* \file
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
    Module      : Operating System Abstraction Layer (OSAL) Support Library
    File        : ptx_OSAL_Sup_Lib.c

    Description : Contains utility functions to get metadata about OSAL items.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_OSAL_Sup_Lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_OSAL_GetUsage(ptx_OSAL_t *osalCtx, ptx_OSAL_ItemType_t itemType, int32_t *itemUsage)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != itemUsage))
    {
        ret = PTX_STATUS_SUCCESS;

        switch (itemType)
        {
            case ptx_OSAL_ItemType_Sem:
                *itemUsage = osalCtx->SemUsage;
                break;

            case ptx_OSAL_ItemType_Mutex:
                *itemUsage = osalCtx->MutexUsage;
                break;

            case ptx_OSAL_ItemType_Thread:
                *itemUsage = osalCtx->ThreadUsage;
                break;

            case ptx_OSAL_ItemType_Timer:
                *itemUsage = (int32_t)osalCtx->TimerCount;
                break;

            default:
                ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_GetPeakUsage(ptx_OSAL_t *osalCtx, ptx_OSAL_ItemType_t itemType, int32_t *itemUsage)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != itemUsage))
    {
        ret = PTX_STATUS_SUCCESS;

        switch (itemType)
        {
            case ptx_OSAL_ItemType_Sem:
                *itemUsage = osalCtx->PeakSemUsage;
                break;

            case ptx_OSAL_ItemType_Mutex:
                *itemUsage = osalCtx->PeakMutexUsage;
                break;

            case ptx_OSAL_ItemType_Thread:
                *itemUsage = osalCtx->PeakThreadUsage;
                break;

            case ptx_OSAL_ItemType_Timer:
                *itemUsage = (int32_t)osalCtx->PeakTimerCount;
                break;

            default:
                ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }
    }

    return ret;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
