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
    Module      : Operating System Abstraction Layer (OSAL)
    File        : ptx_OSAL.c

    Description : Contains the implementation of the OSAL interface for Linux.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_OSAL.h"
#include "ptx_OSAL_Int.h"

#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <errno.h>

#if __STDC_VERSION__ >= 199901L
    #define _XOPEN_SOURCE 600
#else
    #define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#include <time.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES / INTERNAL
 * ####################################################################################################################
 */

#define PTX_OSAL_FILE_MAX_MODE_LEN            5U        /* Maximum size of the Mode String for File handling. */

/*
 * File Handle wrapper structure.
 */
typedef struct ptx_File_Handle
{
    void               *Handle;              /* Pointer to a file stream. */
} ptx_File_Handle_t;

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */

/*
 * Internal linux variant function for calling the thread.
 */
void *ptx_OSAL_ThdInternalFn(void *thdParam);

/*
* Unique Id (random number) generator. Used by ptx_Osal_CreateSemName.
*/
void ptx_OSAL_UniqueId(ptx_OSAL_t *osalCtx, uint8_t *idBuf, uint32_t idBufLen, uint32_t idSeed);

/*
* Define the name for used semaphore.
*/
void ptx_OSAL_CreateSemName(ptx_OSAL_t *osalCtx, char *semName, uint32_t maxLen);

/*
* Return current time stamp in usec.
*/
static ptx_TimeDiff_t ptx_OSAL_GetTimeStamp(ptx_OSAL_t *osalCtx);

/*
* Timer thread procedure function.
*/
static void ptx_OSAL_TimerThdProc(void *timerThdParam);

/*
 * Get complete path of the file.
 */
static ptx_Status_t ptx_OSAL_GetCompleteFilePath(ptx_OSAL_t *osalCtx, ptx_File_Config_t config, char *completePathOut, uint32_t completePathOut_len);

/*
 * Convert file handling system errors into stack supported \ref ptx_Status_t error values.
 */
static ptx_Status_t ptx_OSAL_FileErrorConv(int errNo);

/*
 * Opens the file with the given config.
 */
static ptx_Status_t ptx_OSAL_FileHandleOpen(ptx_OSAL_t *osalCtx, ptx_File_Config_t config, ptx_File_Handle_t *fileHandle);

/*
 * Closes the file stream and breaks all connections to a file.
 */
static ptx_Status_t ptx_OSAL_FileHandleClose(ptx_OSAL_t *osalCtx, ptx_File_Handle_t *fileHandle);

/*
 * Writes data to a file.
 */
static ptx_Status_t ptx_OSAL_FileHandleWrite(ptx_OSAL_t *osalCtx, ptx_File_Handle_t *fileHandle, ptx_File_Config_t config,  uint8_t *dataBlock, uint32_t dataBlockSize);

/*
 * Reads data from a file.
 */
static ptx_Status_t ptx_OSAL_FileHandleRead(ptx_OSAL_t *osalCtx, ptx_File_Handle_t *fileHandle, ptx_File_Config_t config, uint8_t *dataBlock, uint32_t *dataBlockSize);

/*
 * ####################################################################################################################
 * API FUNCTIONS / ADMINISTRATIVE
 * ####################################################################################################################
 */

ptx_Status_t ptx_OSAL_Init(ptx_OSAL_t *osalCtx, void *sysCtx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != osalCtx)
    {
        ptx_OSAL_t *osal = osalCtx;
        (void)memset(osal, 0, sizeof(ptx_OSAL_t));

        osal->SysCtx = sysCtx;
        osal->CompId = Comp_OSAL;

        status = ptx_OSAL_MutexInit(osal, &osal->Mutex);

        if (PTX_STATUS_SUCCESS != status)
        {
            (void)memset(osal, 0, sizeof(ptx_OSAL_t));
        }
    }

    return status;
}

ptx_Status_t ptx_OSAL_DeInit(ptx_OSAL_t *osalCtx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL))
    {
        status = PTX_STATUS_E_GEN_INTERNAL;
        /*
         * Free/De-Initialize anything that OSAL itself has allocated/initialized.
         *
         * NOTE: Mutex member cannot be destroyed with ptx_OSAL_MutexDeInit since this API relies on this member being
         * valid i.e. not destroyed. So, pthread_mutex_destroy is directly used.
         */
        ptx_Mutex_t *mx = &osalCtx->Mutex;

        int error = pthread_mutex_destroy(&mx->Mx);
        if (0 == error)
        {
            (void)memset(osalCtx, 0, sizeof(ptx_OSAL_t));
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * API FUNCTIONS / SEMAPHORES
 * ####################################################################################################################
 */

ptx_Status_t ptx_OSAL_SemInit(ptx_OSAL_t *osalCtx, ptx_Semaphore_t *hSem, uint32_t initialCount, uint32_t maxCount)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    (void) maxCount;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hSem) && (0 != maxCount) && (initialCount <= maxCount))
    {
        ptx_Semaphore_t *sem = hSem;

        (void)memset(sem, 0, sizeof(ptx_Semaphore_t));

        /*
         * Init string so it has a NULL terminating character.
         */
        (void)memset(&sem->Name[0], 0, PTX_SEM_MAX_NAME_LEN);

        ret = ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
        if (PTX_STATUS_SUCCESS == ret)
        {
            ptx_OSAL_CreateSemName(osalCtx, (char*)sem->Name, PTX_SEM_MAX_NAME_LEN - 1);

            sem->Sem = sem_open((const char*)sem->Name, O_CREAT, S_IRUSR | S_IWUSR, (int)initialCount);
            if (SEM_FAILED == sem->Sem)
            {
                /* Semaphore creation failed. */
                (void)memset(sem, 0, sizeof(ptx_Semaphore_t));

                ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            }
            else
            {
                /* Semaphore creation succeeded. */
                errno = 0;

                osalCtx->SemUsage++;
                osalCtx->PeakSemUsage = (osalCtx->SemUsage > osalCtx->PeakSemUsage) ?
                                        osalCtx->SemUsage : osalCtx->PeakSemUsage;

                ret = PTX_STATUS_SUCCESS;
            }

            (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_SemClose(ptx_OSAL_t *osalCtx, ptx_Semaphore_t *hSem)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != osalCtx) && (NULL != hSem))
    {
        ptx_Semaphore_t *sem = hSem;

        ret = ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
        if (PTX_STATUS_SUCCESS == ret)
        {
            (void)sem_unlink((const char*)sem->Name);
            int st = sem_close(sem->Sem);
            if (0 == st)
            {
                osalCtx->SemUsage--;
                (void)memset(sem, 0, sizeof(ptx_Semaphore_t));
                ret = PTX_STATUS_SUCCESS;
            }
            else
            {
                ret = PTX_STATUS_E_GEN_INTERNAL;
            }

            (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_SemPost(ptx_OSAL_t *osalCtx, ptx_Semaphore_t *hSem)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hSem))
    {
        ptx_Semaphore_t *sem = hSem;

        ret = PTX_STATUS_E_GEN_INTERNAL;

        int error = sem_post(sem->Sem);
        if (0 == error)
        {
            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_SemWait(ptx_OSAL_t *osalCtx, ptx_Semaphore_t *hSem)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hSem))
    {
        ptx_Semaphore_t *sem = hSem;

        ret = PTX_STATUS_E_GEN_INTERNAL;

        int st = sem_wait(sem->Sem);
        if (0 == st)
        {
            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_SemTimedWait(ptx_OSAL_t *osalCtx, ptx_Semaphore_t *hSem, uint32_t msTimeout)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hSem))
    {
        ptx_Semaphore_t *sem = hSem;

        struct timespec ts;
        long ns;
        int sem_ret;

        (void)clock_gettime(CLOCK_REALTIME, &ts);

        /*
         * Convert ms to ns
         */
        ns = (((long)msTimeout % 1000) * 1000000) + ts.tv_nsec;
        ts.tv_nsec = ( ns % (long)1000000000ul);
        ts.tv_sec +=  ((__time_t)msTimeout / 1000) + (__time_t)((uint64_t)ns / 1000000000uL);

        sem_ret = sem_timedwait(sem->Sem, &ts);

        if (-1 == sem_ret)
        {
            int err = errno;
            switch (err)
            {
                case ETIMEDOUT:
                    ret = PTX_STATUS_E_GEN_TIMEOUT;

                    break;

                default:
                    ret = PTX_STATUS_E_GEN_INTERNAL;
                    break;
            }
        }
        else
        {
            /*
             * OK
             */

            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}


/*
 * ####################################################################################################################
 * API FUNCTIONS / MUTEXES
 * ####################################################################################################################
 */

ptx_Status_t ptx_OSAL_MutexInit(ptx_OSAL_t *osalCtx, ptx_Mutex_t *hMutex)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hMutex))
    {
        ptx_Mutex_t *mx = hMutex;
        uint32_t t = sizeof(ptx_Mutex_t);
        (void)t;
        (void)memset(mx, 0, sizeof(ptx_Mutex_t));

        int error = pthread_mutex_init(&mx->Mx, NULL);

        if (0 == error)
        {
            ret = ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
            if (PTX_STATUS_SUCCESS == ret)
            {
                osalCtx->MutexUsage++;
                osalCtx->PeakMutexUsage = (osalCtx->MutexUsage > osalCtx->PeakMutexUsage) ?
                                          osalCtx->MutexUsage : osalCtx->PeakMutexUsage;

                (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);
            }
            else
            {
                /* In case of internal error, destroy the mutex immediately. */
                (void)pthread_mutex_destroy(&mx->Mx);
            }
        }
        else
        {
            ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_MutexDeInit(ptx_OSAL_t *osalCtx, ptx_Mutex_t *hMutex)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hMutex))
    {
        ptx_Mutex_t *mx = hMutex;

        int error = pthread_mutex_destroy(&mx->Mx);

        if (0 == error)
        {
            ptx_Status_t st = ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
            if (PTX_STATUS_SUCCESS == st)
            {
                osalCtx->MutexUsage--;
                (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);
            }

            /* Mutex was successfully destroyed, so clean the structure and return SUCCESS. */
            (void)memset(mx, 0, sizeof(ptx_Mutex_t));
            ret = PTX_STATUS_SUCCESS;
        }
        else
        {
            ret = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_MutexLock(ptx_OSAL_t *osalCtx, ptx_Mutex_t *hMutex)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hMutex))
    {
        ptx_Mutex_t *mx = hMutex;
        ret = PTX_STATUS_E_GEN_INTERNAL;

        int error = pthread_mutex_lock(&mx->Mx);
        if (0 == error)
        {
            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_MutexUnlock(ptx_OSAL_t *osalCtx, ptx_Mutex_t *hMutex)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hMutex))
    {
        ptx_Mutex_t *mx = hMutex;
        ret = PTX_STATUS_E_GEN_INTERNAL;

        int error = pthread_mutex_unlock(&mx->Mx);
        if (0 == error)
        {
            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}


/*
 * ####################################################################################################################
 * API FUNCTIONS / THREADS
 * ####################################################################################################################
 */

ptx_Status_t ptx_OSAL_ThreadCreate(ptx_OSAL_t *osalCtx, ptx_Thread_t *hThd, pptx_OSAL_ThreadProc_t thdFn, void *thdParam)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    int error = 0;

    /*
     * thdParam may be NULL
     */
    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hThd) && (NULL != thdFn) && (ptx_Thread_State_INITIAL == hThd->ThdState))
    {
        hThd->ThdState = ptx_Thread_State_CREATING;
        hThd->ThdFn = thdFn;
        hThd->ThdParam = thdParam;

        /* polyspace<MISRA-C3:11.3:not a defect:no action planned> ptx_Thread_t Thread field is exactly a (pthread*) */
        error = pthread_create((pthread_t*)&hThd->Thread, NULL, &ptx_OSAL_ThdInternalFn, (void*)hThd);

        if (0 == error)
        {
            ptx_Status_t st = ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
            if (PTX_STATUS_SUCCESS == st)
            {
                osalCtx->ThreadUsage++;
                osalCtx->PeakThreadUsage = (osalCtx->ThreadUsage > osalCtx->PeakThreadUsage) ?
                                           osalCtx->ThreadUsage : osalCtx->PeakThreadUsage;

                (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);
            }

            /* Thread was successfully created, so return SUCCESS even if mutex locking might have failed.
             * It shall be terminated by the appropriate caller action (typically in a DeInit routine).
             */
            ret = PTX_STATUS_SUCCESS;
        }
        else
        {
            /* Clean up on failure */
            hThd->Thread = 0;
            hThd->ThdFn = NULL;
            hThd->ThdParam = NULL;
            hThd->ThdState = ptx_Thread_State_ERROR;
            ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_ThreadJoin(ptx_OSAL_t *osalCtx, ptx_Thread_t *hThd)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hThd))
    {
        if ((hThd->ThdState == ptx_Thread_State_CREATING) || 
            (hThd->ThdState == ptx_Thread_State_RUN))
        {
            /* Join was called in valid state */
            ret = PTX_STATUS_E_GEN_INTERNAL;
            hThd->ThdState = ptx_Thread_State_JOIN;
            
            int error = pthread_join((pthread_t)hThd->Thread, NULL);
            
            if (0 == error)
            {
                ret = PTX_STATUS_SUCCESS;
            }
            else
            {
                /* Error joining, set error state, error code already set */
                /* Note: pthread_join failure is rare, thread handle may be invalid now
                 * We cannot safely call pthread_detach here as the thread state is unknown
                 */
                hThd->ThdState = ptx_Thread_State_ERROR;
            }
        }
        else if (hThd->ThdState == ptx_Thread_State_JOIN)
        {
            /* OK Thread was already joined */
            ret = PTX_STATUS_SUCCESS;
        }
        else if ((0 == hThd->Thread) && (hThd->ThdState == ptx_Thread_State_INITIAL))
        {
            /* OK Thread was already correctly closed and is in INITIAL state */
            ret = PTX_STATUS_SUCCESS;
        }
        else
        {
            /* ERROR or other invalid states */
            ret = PTX_STATUS_E_GEN_INVALID_STATE;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_ThreadClose(ptx_OSAL_t *osalCtx, ptx_Thread_t *hThd)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hThd))
    {
        if (ptx_Thread_State_JOIN == hThd->ThdState)
        {
            /* Thread is in correct state to be closed */
            ret = ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
            if (PTX_STATUS_SUCCESS == ret)
            {         
                osalCtx->ThreadUsage--;
                /* Clean up all fields */
                hThd->ThdState = ptx_Thread_State_INITIAL;
                hThd->Thread = 0;
                hThd->ThdFn = NULL;
                hThd->ThdParam = NULL;
            }
            (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);
        }
        else if ((0 == hThd->Thread) && (ptx_Thread_State_INITIAL == hThd->ThdState))
        {
            /* Ok Thread was already closed correctly */
            ret = PTX_STATUS_SUCCESS;
        }
        else
        {
            /* Any other state is invalid for close */
            ret = PTX_STATUS_E_GEN_INVALID_STATE;
        }
    }

    return ret;
}

void ptx_OSAL_ThreadSleep(ptx_OSAL_t *osalCtx, uint32_t msSleep)
{
    (void)osalCtx;

    struct timespec ts;
    ts.tv_sec = msSleep / 1000;
    ts.tv_nsec = (msSleep % 1000) * 1000000;

    nanosleep(&ts, &ts);
}


/*
 * ####################################################################################################################
 * API FUNCTIONS / SYSTEM CONTEXT
 * ####################################################################################################################
 */

void *ptx_OSAL_GetSysCtx(ptx_OSAL_t *osalCtx)
{
    void *sc = NULL;
    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL))
    {
        sc = osalCtx->SysCtx;
    }
    return sc;
}

/*
 * ####################################################################################################################
 * FUNCTIONS / INTEGRATION SPECIFIC
 * ####################################################################################################################
 */

void ptx_OSAL_UniqueId(ptx_OSAL_t *osalCtx, uint8_t *idBuf, uint32_t idBufLen, uint32_t idSeed)
{
    const char *lut = "0123456789ABCDEF";
    unsigned int rv = 0;
    ptx_TimeDiff_t td = 0;
    uint32_t i;

    /*
     * Fill -> Terminate with zero(-es)
     */
    (void)memset(idBuf, 0, idBufLen);
    td = ptx_OSAL_GetTimeStamp(osalCtx);
    rv = (idSeed + 1u) * (uint32_t)td;
    (void)srand(rv);
    for (i = 0; i < ((idBufLen / 2) - 1); i++)
    {
        rv += (unsigned int)rand();
        idBuf[i * 2    ] = (uint8_t)lut[(rv >> 4) & 0x0F];
        idBuf[1 + (i * 2)] = (uint8_t)lut[ rv       & 0x0F];
    }
}

void ptx_OSAL_CreateSemName(ptx_OSAL_t *osalCtx, char *semName, uint32_t maxLen)
{
    const char base_name[] = PTX_SEM_BASE_NAME;
    char unique_id[PTX_SEM_UID_LEN + 1];

    const uint32_t base_name_length = (uint32_t)sizeof(base_name);
    const uint32_t unique_id_length = (uint32_t)PTX_SEM_UID_LEN;

    (void)memset(unique_id, 0, PTX_SEM_UID_LEN + 1);

    if ((base_name_length + unique_id_length) <= maxLen)
    {
        ptx_OSAL_UniqueId(osalCtx, (uint8_t*)unique_id, PTX_SEM_UID_LEN, 0);
        strncpy(semName, base_name, maxLen);
        strncat(semName, unique_id, maxLen - base_name_length);
    }
}

/*
 * ####################################################################################################################
 * STOP WATCH
 * ####################################################################################################################
 */

ptx_Status_t ptx_OSAL_StopwatchStart(ptx_OSAL_t *osalCtx, ptx_TimeDiff_t *startVal)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != startVal))
    {
        ret = PTX_STATUS_SUCCESS;

        struct timespec ts;
        clockid_t clkid = CLOCK_MONOTONIC;
        (void)clock_gettime(clkid, &ts);
        /*
         * Convert to usec
         */
        *startVal = (ptx_TimeDiff_t)((ts.tv_sec * (time_t)1000000) + (ts.tv_nsec / (long)1000));
    }

    return ret;
}

ptx_Status_t ptx_OSAL_StopwatchStop(ptx_OSAL_t *osalCtx, ptx_TimeDiff_t *startStopVal)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != startStopVal))
    {
        struct timespec ts;
        clockid_t clkid = CLOCK_MONOTONIC;
        (void)clock_gettime(clkid, &ts);
        /*
         * Convert to usec
         */
        ptx_TimeDiff_t ct = (ptx_TimeDiff_t)((ts.tv_sec * (time_t)1000000) + (ts.tv_nsec / (long)1000));
        *startStopVal = ct - *startStopVal;

        ret = PTX_STATUS_SUCCESS;
    }

    return ret;
}


/*
 * ####################################################################################################################
 * TIMERS
 * ####################################################################################################################
 */

ptx_Status_t ptx_OSAL_TimerCreate(ptx_OSAL_t *osalCtx, ptx_Timer_t *hTimer, pptx_OSAL_TimerProc_t timerProc, void *timerParam)
{
    /*
     * Set max_count to arbitrary value
     */
    const uint32_t max_count = 1234;

    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * timerParam may be NULL
     */
    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hTimer) && (NULL != timerProc))
    {
        (void)memset(hTimer, 0, sizeof(ptx_Timer_t));
        ret = ptx_OSAL_SemInit(osalCtx, &hTimer->StartSem, 0, max_count);
        if (PTX_STATUS_SUCCESS == ret)
        {
            ret = ptx_OSAL_SemInit(osalCtx, &hTimer->WaitCancelSem, 0, max_count);
            if (PTX_STATUS_SUCCESS == ret)
            {
                hTimer->OS = osalCtx;
                hTimer->Param = timerParam;
                hTimer->Proc = timerProc;
                hTimer->Running = 0;
                hTimer->Run = 1;
                hTimer->Mode = PTX_TIMER_PERIODIC;
                hTimer->Thd.ThdState = ptx_Thread_State_INITIAL;
                ret = ptx_OSAL_ThreadCreate(osalCtx, &hTimer->Thd, &ptx_OSAL_TimerThdProc, hTimer);
                if (PTX_STATUS_SUCCESS == ret)
                {
                    /*
                     * All OK
                     */
                    hTimer->Valid = 1;

                    ret = ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
                    if (PTX_STATUS_SUCCESS == ret)
                    {
                        osalCtx->TimerCount++;
                        osalCtx->PeakTimerCount = (osalCtx->TimerCount > osalCtx->PeakTimerCount) ?
                                                  osalCtx->TimerCount : osalCtx->PeakTimerCount;
                        (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);
                    }
                }
                else
                {
                    /*
                     * Threading init error.
                     */
                    (void)ptx_OSAL_SemClose(osalCtx, &hTimer->WaitCancelSem);
                    (void)ptx_OSAL_SemClose(osalCtx, &hTimer->StartSem);
                }
            }
            else
            {
                (void)ptx_OSAL_SemClose(osalCtx, &hTimer->StartSem);
            }
        }
        else
        {
            /*
             * Not even the first Sem. could be initialized.
             */
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_TimerStart(ptx_OSAL_t *osalCtx, ptx_Timer_t *hTimer, uint32_t msInterval)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hTimer))
    {
        if (0 != hTimer->Valid)
        {
            (void) ptx_OSAL_TimerCancel(osalCtx, hTimer);
            hTimer->Interval = msInterval;
            hTimer->Running = 1;
            ret = ptx_OSAL_SemPost(osalCtx, &hTimer->StartSem);
        }
        else
        {
            ret = PTX_STATUS_E_GEN_INVALID_STATE;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_TimerCancel(ptx_OSAL_t *osalCtx, ptx_Timer_t *hTimer)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hTimer) && (0 != hTimer->Valid))
    {
        if (0 != hTimer->Running)
        {
            ret = ptx_OSAL_SemPost(osalCtx, &hTimer->WaitCancelSem);

            if (PTX_STATUS_SUCCESS == ret)
            {
                /*
                * Give some time to timer thread to clear the Running flag.
                * But, do not wait indefinitely.
                */
                ptx_TimeDiff_t time_to_wait_ms = 10;
                ptx_TimeDiff_t start_time_us;
                ptx_TimeDiff_t elapsed_time_us = 0;
                (void) ptx_OSAL_StopwatchStart(osalCtx, &start_time_us);

                do
                {
                    elapsed_time_us = start_time_us;
                    (void) ptx_OSAL_StopwatchStop(osalCtx, &elapsed_time_us);
                }
                while ((0 != hTimer->Running) && ((elapsed_time_us / 1000u) < time_to_wait_ms));

                if (1 == hTimer->Running)
                {
                    /* Timer flag was not cleared in time */
                    ret = PTX_STATUS_E_GEN_TIMEOUT;
                }
            }
        }
        else
        {
            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_TimerClose(ptx_OSAL_t *osalCtx, ptx_Timer_t *hTimer)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hTimer) && (0 != hTimer->Valid))
    {
        hTimer->Interval = 0;
        hTimer->Run = 0;

        (void)ptx_OSAL_SemPost(osalCtx, &hTimer->StartSem);
        (void)ptx_OSAL_SemPost(osalCtx, &hTimer->WaitCancelSem);
        (void)ptx_OSAL_ThreadJoin(osalCtx, &hTimer->Thd);
        (void)ptx_OSAL_ThreadClose(osalCtx, &hTimer->Thd);
        (void)ptx_OSAL_SemClose(osalCtx, &hTimer->StartSem);
        (void)ptx_OSAL_SemClose(osalCtx, &hTimer->WaitCancelSem);

        ret = ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
        if (PTX_STATUS_SUCCESS == ret)
        {
            osalCtx->TimerCount--;
            (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);
        }

        (void)memset(hTimer, 0, sizeof(ptx_Timer_t));
    }

    return ret;
}

/*
 * ####################################################################################################################
 * API FUNCTIONS / FILE HANDLING
 * ####################################################################################################################
 */

ptx_Status_t ptx_OSAL_FileRead (ptx_OSAL_t *osalCtx, ptx_File_Config_t config, uint8_t *buffer, uint32_t *bufferLen)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != buffer) && (NULL != bufferLen) && (*bufferLen > 0))
    {
        ptx_File_Handle_t file_handle;
        ret = ptx_OSAL_FileHandleOpen(osalCtx, config, &file_handle);

        if (PTX_STATUS_SUCCESS == ret)
        {
            uint32_t buffer_len_internal = *bufferLen;
            ret = ptx_OSAL_FileHandleRead(osalCtx, &file_handle, config, buffer, &buffer_len_internal);

            if (PTX_STATUS_SUCCESS == ret)
            {
                /*
                 *  File has been read properly. Now, Let's check that number of bytes read out are the
                 *  same than the expected by the caller to guarantee length-integrity of data;
                 */
                if (buffer_len_internal <= *bufferLen)
                {
                    *bufferLen = buffer_len_internal;
                }
                else
                {
                    *bufferLen = 0;
                    ret = PTX_STATUS_E_GEN_INTERNAL;
                }
            }

            if (PTX_STATUS_SUCCESS == ret)
            {
                ret = ptx_OSAL_FileHandleClose(osalCtx, &file_handle);
            }
            else
            {
                (void) ptx_OSAL_FileHandleClose(osalCtx, &file_handle);
            }
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_FileWrite (ptx_OSAL_t *osalCtx, ptx_File_Config_t config, uint8_t *buffer, uint32_t bufferLen)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != buffer) && (bufferLen > 0))
    {
        ptx_File_Handle_t file_handle;
        ret = ptx_OSAL_FileHandleOpen(osalCtx, config, &file_handle);

        if (PTX_STATUS_SUCCESS == ret)
        {
            ret = ptx_OSAL_FileHandleWrite(osalCtx, &file_handle, config, buffer, bufferLen);

            if (PTX_STATUS_SUCCESS == ret)
            {
                ret = ptx_OSAL_FileHandleClose(osalCtx, &file_handle);
            }
            else
            {
                (void) ptx_OSAL_FileHandleClose(osalCtx, &file_handle);
            }
        }
    }

    return ret;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */
static void ptx_OSAL_TimerThdProc(void *timerThdParam)
{
    ptx_Status_t st;
    ptx_Timer_t *tim = (ptx_Timer_t*)timerThdParam;
    tim->Thd.ThdState = ptx_Thread_State_RUN;

    do
    {
        /* Wait for the timer to get started by another thread. */
        (void)ptx_OSAL_SemWait(tim->OS, &tim->StartSem);

        do
        {
            /* Timer is started. Wait for the interval to elapse. */
            st = ptx_OSAL_SemTimedWait(tim->OS, &tim->WaitCancelSem, tim->Interval);

            switch (st)
            {
                case PTX_STATUS_SUCCESS:
                    /*
                    * Cancel Signal received:
                    */
                    tim->Running = 0;
                    break;

                case PTX_STATUS_E_GEN_TIMEOUT:
                    /*
                    * Timed out, as should be:
                    */
                    if (PTX_TIMER_PERIODIC != tim->Mode)
                    {
                        /* Single shot timer: do not repeat this action. Stop the inner looping. */
                        tim->Running = 0;
                    }

                    if ((0 != tim->Run) && (NULL != tim->Proc))
                    {
                        tim->Proc(tim->Param);
                    }
                    break;

                default:
                    /*
                    * No action. Should not happen.
                    */
                    tim->Run = 0;
                    break;
            }
        }
        while ((0 != tim->Run) && (0 != tim->Running));

    }
    while (0 != tim->Run);
}

void *ptx_OSAL_ThdInternalFn(void *thdParam)
{
    ptx_Thread_t *thd_param = (ptx_Thread_t*)thdParam;
    if (thd_param->ThdState == ptx_Thread_State_CREATING)
    {
        /* Only transition to RUN if still in CREATING state */
        thd_param->ThdState = ptx_Thread_State_RUN;
    }
    thd_param->ThdFn((void*)thd_param->ThdParam);
    return NULL;
}

static ptx_TimeDiff_t ptx_OSAL_GetTimeStamp(ptx_OSAL_t *osalCtx)
{
    PTX_UNUSED_PARAMETER(osalCtx);

    ptx_TimeDiff_t time_stamp = 0;
    struct timespec ts;
    clockid_t clkid = CLOCK_MONOTONIC;
    (void)clock_gettime(clkid, &ts);

    /*
     * Convert to usec
     */
    time_stamp = (ptx_TimeDiff_t)((ts.tv_sec * (time_t)1000000) + (ts.tv_nsec / (long)1000));

    return time_stamp;
}



static ptx_Status_t ptx_OSAL_GetCompleteFilePath(ptx_OSAL_t *osalCtx, ptx_File_Config_t config, char *completePathOut, uint32_t completePathOutLen)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != config.Name) && (NULL != config.Path) && (NULL != completePathOut) && (completePathOutLen > 0))
    {
        uint32_t name_len = (uint32_t)strlen(config.Name);
        uint32_t path_len = (uint32_t)strlen(config.Path);
        uint32_t complete_pathIn_len = path_len + name_len;

        if(PTX_OSAL_FILE_MAX_PATH_LEN >= complete_pathIn_len)
        {
            if (completePathOutLen >= complete_pathIn_len + 1)
            {
                // Concatenate file name to path
                (void)memset(completePathOut, 0x00, completePathOutLen);
                uint32_t remaining_mem = completePathOutLen - 1;
                (void)strncat(completePathOut, config.Path, remaining_mem);
                remaining_mem -= path_len;
                (void)strncat(completePathOut, config.Name, remaining_mem);

                ret = PTX_STATUS_SUCCESS;
            }
            else
            {
                ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            }
        }
    }

    return ret;
}
 
static ptx_Status_t ptx_OSAL_FileHandleOpen(ptx_OSAL_t *osalCtx, ptx_File_Config_t config, ptx_File_Handle_t *fileHandle)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != fileHandle))
    {
        char complete_path_out [PTX_OSAL_FILE_MAX_PATH_LEN + 1];
        ret = ptx_OSAL_GetCompleteFilePath(osalCtx, config, &complete_path_out[0], PTX_OSAL_FILE_MAX_PATH_LEN + 1);

        /*
         * mode_str_len * 2 + terminating char to be safe
         */
        char mode[PTX_OSAL_FILE_MAX_MODE_LEN];
        memset(mode, 0, PTX_OSAL_FILE_MAX_MODE_LEN);

        if (PTX_STATUS_SUCCESS == ret)
        {
            switch (config.Mode)
            {
                case ptx_File_Mode_Read:
                    (void)strncpy(mode, "r", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;

                case ptx_File_Mode_Write:
                    (void)strncpy(mode, "w", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;

                case ptx_File_Mode_Append:
                    (void)strncpy(mode, "a", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;

                case ptx_File_Mode_ReadUpdate:
                    (void)strncpy(mode, "r+", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;

                case ptx_File_Mode_WriteUpdate:
                    (void)strncpy(mode, "w+", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;

                case ptx_File_Mode_AppendUpdate:
                    (void)strncpy(mode, "a+", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;

                case ptx_File_Mode_BinaryRead:
                    (void)strncpy(mode, "rb", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;

                case ptx_File_Mode_BinaryWrite:
                    (void)strncpy(mode, "wb", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;

                case ptx_File_Mode_BinaryAppend:
                    (void)strncpy(mode, "ab", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;

                case ptx_File_Mode_BinaryReadUpdate:
                    (void)strncpy(mode, "r+b", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;

                case ptx_File_Mode_BinaryWriteUpdate:
                    (void)strncpy(mode, "w+b", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;

                case ptx_File_Mode_BinaryAppendUpdate:
                    (void)strncpy(mode, "a+b", PTX_OSAL_FILE_MAX_MODE_LEN);
                    break;
                default:
                    ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }
            
        }

        if (PTX_STATUS_SUCCESS == ret)
        {
            fileHandle->Handle = (void*)fopen(complete_path_out, mode);

            if (NULL != fileHandle->Handle)
            {
                /*
                 * The following line gives a false positive (reported as a potential memory leak) in SonarQube analysis and is
                 * therefore left out from analysis.
                 *
                 * The reported issue is related to keeping the file handle opened when exiting the function.
                 * It is ensured by the proper usage of OSAL File APIs ptx_OSAL_FileHandleOpen and ptx_OSAL_FileHandleClose that the file handle gets closed.
                 */
                ret = PTX_STATUS_SUCCESS;        
            }
            else
            {
                /*
                 * Error handling; a few errors, deemed important are handled separately here.
                 */
                int err = errno;
                ret = ptx_OSAL_FileErrorConv(err);
            }
        }
    }

    return ret;
}

static ptx_Status_t ptx_OSAL_FileHandleClose(ptx_OSAL_t *osalCtx, ptx_File_Handle_t *fileHandle)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != fileHandle) && (NULL != fileHandle->Handle))
    {
        int rv = fclose((FILE*)fileHandle->Handle);
        fileHandle = NULL;

        if (EOF == rv)
        {
            /*
             * Error
             */
            int err = errno;
            ret = ptx_OSAL_FileErrorConv(err);
        }
        else
        {
            /*
             * OK
             */
            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}

static ptx_Status_t ptx_OSAL_FileHandleWrite(ptx_OSAL_t *osalCtx, ptx_File_Handle_t *fileHandle, ptx_File_Config_t config, uint8_t *dataBlock, uint32_t dataBlockSize)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != fileHandle) && (NULL != fileHandle->Handle) && (NULL != dataBlock) && (0 < dataBlockSize) &&
        ((ptx_File_Mode_Write == config.Mode) || (ptx_File_Mode_Append == config.Mode) || (ptx_File_Mode_WriteUpdate == config.Mode) || (ptx_File_Mode_AppendUpdate == config.Mode) || 
         (ptx_File_Mode_BinaryWrite == config.Mode) || (ptx_File_Mode_BinaryAppend == config.Mode) || (ptx_File_Mode_BinaryWriteUpdate == config.Mode) || (ptx_File_Mode_BinaryAppendUpdate == config.Mode) || 
         (ptx_File_Mode_ReadUpdate == config.Mode) || (ptx_File_Mode_BinaryReadUpdate == config.Mode)))
    {
        uint32_t bw;
        clearerr((FILE*)fileHandle->Handle);
        bw = (uint32_t)fwrite(dataBlock, sizeof(uint8_t), (size_t)dataBlockSize, (FILE*)(fileHandle->Handle));
        if (bw == dataBlockSize)
        {
            /*
             * OK
             */
            ret = PTX_STATUS_SUCCESS;
        }
        else
        {
            if (0 != ferror((FILE*)fileHandle->Handle))
            {
                /*
                 * Error
                 */
                int err = errno;
                ret = ptx_OSAL_FileErrorConv(err);
            }
            else
            {
                /*
                 * OK, but the number of bytes to write was not equal to the actual written bytes.
                 */
                ret = PTX_STATUS_E_GEN_INTERNAL;
            }
        }
    }

    return ret;
}

static ptx_Status_t ptx_OSAL_FileHandleRead(ptx_OSAL_t *osalCtx, ptx_File_Handle_t *fileHandle, ptx_File_Config_t config, uint8_t *dataBlock, uint32_t *dataBlockSize)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != fileHandle) && (NULL != fileHandle->Handle) && (NULL != dataBlock) && (NULL != dataBlockSize) && 
        ((ptx_File_Mode_WriteUpdate == config.Mode) || (ptx_File_Mode_AppendUpdate == config.Mode) || 
         (ptx_File_Mode_BinaryWriteUpdate == config.Mode) || (ptx_File_Mode_BinaryAppendUpdate == config.Mode) || 
         (ptx_File_Mode_ReadUpdate == config.Mode) || (ptx_File_Mode_BinaryReadUpdate == config.Mode) ||
         (ptx_File_Mode_Read == config.Mode) || (ptx_File_Mode_BinaryRead == config.Mode)))
    {
        uint32_t br;
        uint32_t btr = *dataBlockSize;
        clearerr((FILE*)fileHandle->Handle);
        br = (uint32_t)fread(dataBlock, sizeof(uint8_t), (size_t)btr, (FILE*)fileHandle->Handle);
        if (br == btr)
        {
            /*
             * OK
             */
            ret = PTX_STATUS_SUCCESS;
        }
        else
        {
            if (0 != feof((FILE*)fileHandle->Handle))
            {
                /*
                 * OK, but EOF reached before the requested number of bytes could be read.
                 */
                *dataBlockSize = br;
                ret = PTX_STATUS_SUCCESS;
            }
            else if (0 != ferror((FILE*)fileHandle->Handle))
            {
                /*
                 * Error
                 */
                int err = errno;
                ret = ptx_OSAL_FileErrorConv(err);
                *dataBlockSize = 0;
            }
            else
            {
                /*
                 * OK, but nothing read, as the number of requested bytes was 0.
                 */
                *dataBlockSize = 0;
                ret = PTX_STATUS_E_GEN_INTERNAL;
            }
        }
    }

    return ret;
}

static ptx_Status_t ptx_OSAL_FileErrorConv(int errNo)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INTERNAL;

    switch (errNo)
    {
        case EPERM:     /* Operation not permitted */
            ret = PTX_STATUS_E_GEN_NOT_PERMITTED;
            break;

        case ENXIO:     /* No such device or address */
        /* FALL THROUGH */
        case ENOENT:    /* No such file or directory */
            ret = PTX_STATUS_E_GEN_NOT_FOUND;
            break;

        case ENOSPC:    /* No space left on device */
        /* FALL THROUGH */
        case ENOMEM:    /* Out of memory */
            ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            break;

        case EACCES:    /* Permission denied */
            ret = PTX_STATUS_E_GEN_ACCESS_DENIED;
            break;

        case EROFS:     /* Read-only file system */
        /* FALL THROUGH */
        case EFBIG:     /* File too large */
        /* FALL THROUGH */
        case EISDIR:    /* Is a directory */
        /* FALL THROUGH */
        case EBADF:     /* Bad file number */
        /* FALL THROUGH */
        case ESPIPE:    /* Illegal seek */
        /* FALL THROUGH */
        case EIO:       /* I/O error */
        /* FALL THROUGH */
        default:
            /* Treat all these errors the same way */
            ret = PTX_STATUS_E_GEN_INTERNAL;
            break;
    }
    return ret;
}
