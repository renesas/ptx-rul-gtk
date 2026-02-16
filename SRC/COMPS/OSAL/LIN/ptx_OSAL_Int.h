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
    Module      : Operating System Abstraction Layer (OSAL) Linux Interface
    File        : ptx_OSAL_Int.h

    Description : Contains operating system specific declarations.
*/

/**
 * \addtogroup OSAL_LIN
 *
 * @{
 */

#if !defined(PTX_OSAL_INT_H)
#define PTX_OSAL_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>
#include <semaphore.h>


#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

#define PTX_SEM_BASE_NAME       "/PTX_NSC_SEM_"         /**< Semaphore name prefix as seen by the system */
#define PTX_SEM_UID_LEN         8                       /**< Semaphore uid length */
#define PTX_SEM_MAX_NAME_LEN    (20+PTX_SEM_UID_LEN+1)  /**< Semaphore max. name length must hold base name + UID + \0 */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Semaphore Wrapper Structure
 */
typedef struct ptx_Semaphore
{
    sem_t       *Sem;                                    /**< Posix semaphore. */
    uint8_t     Name[PTX_SEM_MAX_NAME_LEN];              /**< Sempahore name. */
} ptx_Semaphore_t;

/**
 * Mutex Wrapper Structure
 */
typedef struct ptx_Mutex
{
    pthread_mutex_t Mx;                                 /**< Posix mutex. */
} ptx_Mutex_t;


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
