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
    Module      : HAL Windows Utils
    File        : ptx_HAL_WinUtils.h

    Description : Windows utility methods for dll handling, semaphore, mutex and threads
*/

/**
 * \addtogroup HAL_WIN_UTIL
 *
 * @{
 */

#if !defined(PTX_HAL_WIN_UTILS_H)
#define PTX_HAL_WIN_UTILS_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "ptx_Status.h"
#include "ptx_Components.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/**
 * \brief Library Wrapper Structure.
 */
typedef struct ptx_Win_Lib
{
    void               *LibH;               /**< Loaded library handle. Assigned in \ref ptx_LibOpen. */
    uint8_t             IsLoaded;           /**< Library loaded successfully (1) or not (0). Assigned in \ref ptx_LibOpen. */
    uint32_t            LastError;          /**< If library load in \ref ptx_LibOpen fails, holds resulting Error code. */
} ptx_Win_Lib_t;

/**
 * \brief Mutex handle wrapper structure.
 */
typedef struct ptx_Win_Mutex
{
    HANDLE Mx;                              /**< Handle to windows mutex. */
} ptx_Win_Mutex_t;

/**
 *  \brief Thread procedure function pointer declaration.
 *
 *  \param      threadParam Pointer to thread-specific ("context") storage.
 */
typedef void (*pptx_Win_ThreadProc_t)(void *threadParam);

/**
 * \brief Thread structure.
 */
typedef struct ptx_Win_Thread
{
    pptx_Win_ThreadProc_t   ThdFn;              /**< Pointer to thread procedure function. */
    void                   *ThdParam;           /**< Pointer to a parameter passed to \ref ptx_ThreadCreate. */
    void                   *Thread;             /**< Thread handle, assigned in \ref ptx_ThreadCreate. */
} ptx_Win_Thread_t;

/**
 * \brief Semaphore handle wrapper structure.
 */
struct ptx_Win_Semaphore
{
    HANDLE Sem;                                 /**< Handle to windows semaphore. */
};
/**
 * \brief Semaphore handle wrapper structure.
 */
typedef struct ptx_Win_Semaphore *ptx_Win_Semaphore_t;

/**
 * \brief Initialize an already allocated but non-initialized object.
 *
 * \param[out]  hSem            Pointer to an allocated but non-initialized semaphore object.
 * \param[in]   initialCount    Initial value.
 * \param[in]   maxCount        Maximum count.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_SemInit(ptx_Win_Semaphore_t *hSem, uint32_t initialCount, uint32_t maxCount);

/**
 * \brief Close an initialized semaphore object.
 *
 * \param[in]   hSem            Pointer to an initialized semaphore object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_SemClose(ptx_Win_Semaphore_t *hSem);

/**
 * \brief Signal a semaphore, unblock a waiting thread.
 *
 * \param[in]   hSem            Pointer to an initialized semaphore object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_SemPost(ptx_Win_Semaphore_t *hSem);

/**
 * \brief Wait on the semaphore.
 *
 * \param[in]   hSem            Pointer to an initialized semaphore object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_SemWait(ptx_Win_Semaphore_t *hSem);

/**
 * \brief Wait on the semaphore, with timeout.
 *
 * \param[in]   hSem            Pointer to an initialized semaphore object.
 * \param[in]   msTimeout       Timeout period, in ms.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_SemTimedWait(ptx_Win_Semaphore_t *hSem, uint32_t msTimeout);

/**
 * \brief Initialize a Mutex.
 *
 * \param[out]  hMutex          Pointer to a location that will be initialized/receive the Mutex object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_MutexInit(ptx_Win_Mutex_t *hMutex);

/**
 * \brief Remove a Mutex.
 *
 * \param[out]  hMutex          Pointer to the Mutex object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_MutexDestroy(ptx_Win_Mutex_t *hMutex);

/**
 * \brief Lock/acquire a Mutex.
 *
 * \param[in]   hMutex          Pointer to the initialized Mutex object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_MutexLock(ptx_Win_Mutex_t *hMutex);

/**
 * \brief Unlock/release a Mutex.
 *
 * \param[in]   hMutex          Pointer to the initialized Mutex object.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_MutexUnlock(ptx_Win_Mutex_t *hMutex);

/**
 * \brief Open a dynamic load library.
 *
 * \param[in]   fileNameID      File name or ID, depending on the system.
 * \param[out]  libH            Pointer to a location that will receive the address of an allocated library structure.
 *
 * \return Status, indicating whether the operation was successful.
 *
 */
ptx_Status_t ptx_LibOpen(const char *fileNameID, ptx_Win_Lib_t **libH);

/**
 * \brief Close the dynamically loaded library.
 *
 * \param[out]  libH            Pointer to the location of (\ref ptx_LibOpen) library structure which will be freed.
 *
 * \return Status, indicating whether the operation was successful.
 *
 */
ptx_Status_t ptx_LibClose(ptx_Win_Lib_t **libH);

/**
 * \brief Get the address of a library function.
 *
 * \param[out]  libH            Pointer to a an allocated (\ref ptx_LibOpen) library structure.
 * \param[out]  fnName          Name of the function to load.
 * \param[out]  fnAddr          Pointer to a location that will receive the address of the requested function.
 *
 * \return Status, indicating whether the operation was successful.
 *
 */
ptx_Status_t ptx_LibFn(ptx_Win_Lib_t *libH, const char *fnName, void **fnAddr);

/**
 * \brief Create a thread.
 *
 * \param[out]  hThd            Pointer to a platform-specific thread descriptor/structure/handle.
 * \param[in]   thdFn           Pointer to the thread procedure.
 * \param[in]   thdParam        Parameter that the thread function will receive when it is started. May be NULL
 *                              if there is no data to share with or to hand over to the thread respectively.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_ThreadCreate(ptx_Win_Thread_t *hThd, pptx_Win_ThreadProc_t thdFn, void *thdParam);

/**
 * \brief Join on a thread, wait until it has exited.
 *
 * \param[out]  hThd            Pointer to the thread, created by \ref ptx_ThreadCreate.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_ThreadJoin(ptx_Win_Thread_t *hThd);

/**
 * \brief Close a thread, once it has exited.
 *
 * \param[out]  hThd            Pointer to the thread, created by \ref ptx_ThreadCreate.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_ThreadClose(ptx_Win_Thread_t *hThd);


#endif /* Guard */

/** @} */

