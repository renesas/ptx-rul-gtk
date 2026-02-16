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

    Description : Contains the implementation of the OSAL interface for Windows.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_OSAL.h"
#include <stdio.h>
#include <string.h>

/*
#if !defined(UNICODE)
    #error UNICODE must be defined
#endif
*/

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
 * Thread function. Executed when thread is created.
 */
DWORD WINAPI ptx_OSAL_ThdInternalFn(void *thdParam);


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
         * Free/De-Initialize everything what OSAL itself has allocated or initialized.
         *
         * Mutex member needs to be de-initialized directly here and not in ptx_OSAL_MutexDeInit API, since
         * that API needs a valid Mutex to execute successfully.
         */
        ptx_Mutex_t *mx = &osalCtx->Mutex;

        int error = (CloseHandle(mx->Mx) == 0) ? 1 : 0;
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

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hSem) && (0 != maxCount) && (initialCount <= maxCount))
    {
        ptx_Semaphore_t *sem = hSem;

        (void)memset(sem, 0, sizeof(ptx_Semaphore_t));

        sem->Sem = CreateSemaphore(NULL, (LONG)initialCount, (LONG)maxCount, NULL);
        int st = (NULL == (sem->Sem)) ? 1 : 0;

        if (0 == st)
        {
            ret = ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
            if (PTX_STATUS_SUCCESS == ret)
            {
                osalCtx->SemUsage++;
                osalCtx->PeakSemUsage = (osalCtx->SemUsage > osalCtx->PeakSemUsage) ?
                                        osalCtx->SemUsage : osalCtx->PeakSemUsage;
                (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);
            }
        }
        else
        {
            /* Semaphore creation failed. */
            (void)memset(sem, 0, sizeof(ptx_Semaphore_t));

            ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
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
            int st = CloseHandle(sem->Sem);
            if (0 != st)
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

        int error = (ReleaseSemaphore(sem->Sem, 1, NULL) == 0) ? 1 : 0;
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

        int error = (WaitForSingleObject(sem->Sem, INFINITE) == WAIT_OBJECT_0 ? 0 : 1);
        if (0 == error)
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

        DWORD err = WaitForSingleObject(sem->Sem, msTimeout);

        switch (err)
        {
            case WAIT_OBJECT_0:
                /*
                 * OK
                 */
                ret = PTX_STATUS_SUCCESS;
                break;

            case WAIT_TIMEOUT:
                ret = PTX_STATUS_E_GEN_TIMEOUT;
                break;

            default:
                ret = PTX_STATUS_E_GEN_INTERNAL;
                break;
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
        (void)memset(mx, 0, sizeof(ptx_Mutex_t));

        mx->Mx = CreateMutex(0, FALSE, 0);
        int error = (mx->Mx == 0) ? 1 : 0;

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
                (void)CloseHandle(mx->Mx);
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

        int error = (CloseHandle(mx->Mx) == 0) ? 1 : 0;

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

        volatile DWORD wfso_st = WaitForSingleObject(mx->Mx, INFINITE);

        int error = (wfso_st == WAIT_OBJECT_0) ? 0 : 1;
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

        int error = ReleaseMutex(mx->Mx);
        if (0 != error)
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
    const uint32_t default_thd_size = 0;
    const uint32_t thd_run_immediately = 0;
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    int error = 0;

    /*
     * thdParam may be NULL
     */
    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hThd) && (NULL != thdFn))
    {
        hThd->ThdFn = thdFn;
        hThd->ThdParam = thdParam;

        hThd->Thread = CreateThread(
                           NULL,                   /* Attr */
                           default_thd_size,       /* Stack Size */
                           &ptx_OSAL_ThdInternalFn, /* Fn */
                           (void*)hThd,            /* Param */
                           thd_run_immediately,    /* Mode */
                           NULL);                  /* ID */
        error = (NULL == hThd->Thread) ? 1 : 0;

        if (0 == error)
        {
            (void)ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
            osalCtx->ThreadUsage++;
            osalCtx->PeakThreadUsage = (osalCtx->ThreadUsage > osalCtx->PeakThreadUsage) ?
                                       osalCtx->ThreadUsage : osalCtx->PeakThreadUsage;
            (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);

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

        DWORD wfso_st = WaitForSingleObject(hThd->Thread, INFINITE);

        if (WAIT_OBJECT_0 == wfso_st)
        {
            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}

ptx_Status_t ptx_OSAL_ThreadClose(ptx_OSAL_t *osalCtx, ptx_Thread_t *hThd)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    int error = 0;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != hThd))
    {
        BOOL win_ret = CloseHandle(hThd->Thread);
        error = (win_ret == 0) ? 1 : 0;

        if (0 == error)
        {
            ret = ptx_OSAL_MutexLock(osalCtx, &osalCtx->Mutex);
            if (PTX_STATUS_SUCCESS == ret)
            {
                osalCtx->ThreadUsage--;
                (void)ptx_OSAL_MutexUnlock(osalCtx, &osalCtx->Mutex);
            }
        }
        else
        {
            ret = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return ret;
}

void ptx_OSAL_ThreadSleep(ptx_OSAL_t *osalCtx, uint32_t msSleep)
{
    PTX_UNUSED_PARAMETER(osalCtx);
    Sleep(msSleep);
}

/*
 * ####################################################################################################################
 * OSAL STOPWATCH
 * ####################################################################################################################
 */

ptx_Status_t ptx_OSAL_StopwatchStart(ptx_OSAL_t *osalCtx, ptx_TimeDiff_t *startVal)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != startVal))
    {
        ret = PTX_STATUS_SUCCESS;

        LARGE_INTEGER sv;
        (void)QueryPerformanceCounter(&sv);
        *startVal = (ptx_TimeDiff_t)sv.QuadPart;
    }

    return ret;
}

ptx_Status_t ptx_OSAL_StopwatchStop(ptx_OSAL_t *osalCtx, ptx_TimeDiff_t *startStopVal)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    const uint32_t usec_ticks_per_second = 1000000UL;

    if (PTX_COMP_CHECK(osalCtx, Comp_OSAL) && (NULL != startStopVal))
    {
        ret = PTX_STATUS_SUCCESS;

        LARGE_INTEGER pc = {0};
        LARGE_INTEGER pf = {0};
        LARGE_INTEGER delta;

        QueryPerformanceCounter(&pc);
        QueryPerformanceFrequency(&pf);

        if (0 != pf.QuadPart)
        {
            delta.QuadPart = (LONGLONG)(pc.QuadPart - (LONGLONG) * startStopVal);
            /*
             * Convert to micros before dividing by ticks per second.
             */
            delta.QuadPart *= usec_ticks_per_second;
            delta.QuadPart /= pf.QuadPart;
            *startStopVal = (ptx_TimeDiff_t)delta.QuadPart;
        }
        else
        {
            *startStopVal = 0;
        }

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
                const uint32_t time_to_wait = 10;
                uint32_t idle_time = 0;
                do
                {
                    Sleep(1);
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

DWORD WINAPI ptx_OSAL_ThdInternalFn(void *thdParam)
{
    ptx_Thread_t *thd_param = (ptx_Thread_t*)thdParam;
    thd_param->ThdFn((void*)thd_param->ThdParam);

    return 0;
}

