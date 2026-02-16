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
    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hThd) && (NULL != thdFn))
    {
        (void)memset(hThd, 0, sizeof(ptx_Thread_t));

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
        ret = PTX_STATUS_E_GEN_INTERNAL;

        int error = pthread_join((pthread_t)hThd->Thread, NULL);
        if (0 == error)
        {
            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_ThreadClose(ptx_OSAL_t *osalCtx, ptx_Thread_t *hThd)
{
    (void)hThd;
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hThd))
    {
        /*
         * Join is enough - it is called before the call to this API. No explicit close() fn needed.
         */
        ret = ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
        if (PTX_STATUS_SUCCESS == ret)
        {
            osalCtx->ThreadUsage--;
            (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);
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
                const uint32_t time_to_wait = 1000UL;
                uint32_t idle_time = 0;
                do
                {
                    usleep(10);
                    idle_time++;
                }
                while ((0 != hTimer->Running) && (time_to_wait > idle_time));

                if (idle_time >= time_to_wait)
                {
                    ret = PTX_STATUS_E_HALTIMER_INTERNAL_STOP;
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
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */
static void ptx_OSAL_TimerThdProc(void *timerThdParam)
{
    ptx_Status_t st;
    ptx_Timer_t *tim = (ptx_Timer_t*)timerThdParam;

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


