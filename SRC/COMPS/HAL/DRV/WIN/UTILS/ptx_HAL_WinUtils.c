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
    Module      : HAL Windows Utils
    File        : ptx_HAL_WinUtils.c

    Description : Windows utility methods for dll handling, semaphore, mutex and threads
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_WinUtils.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_HAL_WIN_UTILS_MAX_LIB_NAME      4096

/*
 * Thread function. Executed when thread is created.
 */
DWORD WINAPI ptx_ThdInternalFn(void *thdParam);

ptx_Status_t ptx_SemInit(ptx_Win_Semaphore_t *hSem, uint32_t initialCount, uint32_t maxCount)
{
    int st = 0;
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != hSem) && (0 != maxCount) && (initialCount <= maxCount))
    {
        ptx_Win_Semaphore_t sem = (ptx_Win_Semaphore_t)malloc(sizeof(struct ptx_Win_Semaphore));
        if (NULL != sem)
        {
            memset(sem, 0, sizeof(struct ptx_Win_Semaphore));
            ret = PTX_STATUS_SUCCESS;
        }
        else
        {
            ret = PTX_STATUS_E_GEN_INTERNAL;
        }

        if (NULL != sem)
        {
            sem->Sem = CreateSemaphore(NULL, (LONG)initialCount, (LONG)maxCount, NULL);
            st = (NULL == (sem->Sem)) ? 1 : 0;

            if (0 != st)
            {
                ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            }
        }
        else
        {
            ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }

        /*
         * Set anyway:
         */
        *hSem = sem;
    }

    return ret;
};

ptx_Status_t ptx_SemClose(ptx_Win_Semaphore_t *hSem)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != hSem)
    {
        ptx_Win_Semaphore_t sem = *hSem;

        if (NULL != sem)
        {
            (void)CloseHandle(sem->Sem);

            free(sem);
            *hSem = NULL;

            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}

ptx_Status_t ptx_SemPost(ptx_Win_Semaphore_t *hSem)
{
    int st = 0;
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != hSem)
    {
        ptx_Win_Semaphore_t sem = *hSem;

        ret = PTX_STATUS_SUCCESS;
        st = (ReleaseSemaphore(sem->Sem, 1, NULL) == 0);
    }

    if (0 != st)
    {
        ret = PTX_STATUS_E_GEN_INTERNAL;
    }

    return ret;
}

ptx_Status_t ptx_SemWait(ptx_Win_Semaphore_t *hSem)
{
    int error = 0;
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != hSem)
    {
        ret = PTX_STATUS_SUCCESS;

        error =  (WaitForSingleObject((*hSem)->Sem, INFINITE) == WAIT_FAILED ? 1 : 0);
    }

    if (0 != error)
    {
        ret = PTX_STATUS_E_GEN_INTERNAL;
    }

    return ret;
}


ptx_Status_t ptx_MutexLock(ptx_Win_Mutex_t *hMutex)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    int error = 0;

    if (NULL != hMutex)
    {
        volatile DWORD wfso_st = WaitForSingleObject(hMutex->Mx, INFINITE);
        error = (wfso_st == WAIT_FAILED) ? 1 : 0;

        if (0 != error)
        {
            ret = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}

ptx_Status_t ptx_MutexUnlock(ptx_Win_Mutex_t *hMutex)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    int error = 0;

    if (NULL != hMutex)
    {
        error = (ReleaseMutex(hMutex->Mx) == 0) ? 1 : 0;

        ret = PTX_STATUS_SUCCESS;
    }

    if (0 != error)
    {
        ret = PTX_STATUS_E_GEN_INTERNAL;
    }

    return ret;
}

ptx_Status_t ptx_MutexInit(ptx_Win_Mutex_t *hMutex)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    int error = 0;

    if (NULL != hMutex)
    {
        ret = PTX_STATUS_SUCCESS;

        if (NULL != hMutex)
        {
            hMutex->Mx = CreateMutex(0, FALSE, 0);
            error = (hMutex->Mx == 0) ? 1 : 0;

            if (0 != error)
            {
                ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            }
        }
        else
        {
            ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
    }

    return ret;
}


ptx_Status_t ptx_MutexDestroy(ptx_Win_Mutex_t *hMutex)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    int error = 0;

    if (NULL != hMutex)
    {
        error = (CloseHandle(hMutex->Mx) == 0) ? 1 : 0;

        if (0 == error)
        {
            ret = PTX_STATUS_SUCCESS;
        }
        else
        {
            ret = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return ret;
}

DWORD WINAPI ptx_ThdInternalFn(void *thdParam)
{
    ptx_Win_Thread_t *thd_param = (ptx_Win_Thread_t*)thdParam;
    if (NULL != thd_param)
    {
        thd_param->ThdFn((void*)thd_param->ThdParam);
    }

    return 0;
}

ptx_Status_t ptx_ThreadCreate(ptx_Win_Thread_t *hThd, pptx_Win_ThreadProc_t thdFn, void *thdParam)
{
    const uint32_t default_thd_size = 0;
    const uint32_t thd_run_immediately = 0;
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    int error = 0;

    /*
     * thdParam may be NULL
     */
    if ((NULL != hThd) && (NULL != thdFn))
    {
        hThd->ThdFn = thdFn;
        hThd->ThdParam = thdParam;

        hThd->Thread = CreateThread(
                           NULL,                   /* Attr */
                           default_thd_size,       /* Stack Size */
                           &ptx_ThdInternalFn,     /* Fn */
                           (void*)hThd,            /* Param */
                           thd_run_immediately,    /* Mode */
                           NULL);                  /* ID */
        error = (NULL == hThd->Thread) ? 1 : 0;

        if (0 != error)
        {
            ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
        else
        {
            ret = PTX_STATUS_SUCCESS;
        }
    }

    (void)thd_run_immediately;
    (void)default_thd_size;
    return ret;
}

ptx_Status_t ptx_LibOpen(const char *fileNameID, ptx_Win_Lib_t **libH)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    wchar_t lib_name[PTX_HAL_WIN_UTILS_MAX_LIB_NAME];

    if ((NULL != fileNameID) && (NULL != libH))
    {
        ptx_Win_Lib_t *lh = malloc(sizeof(ptx_Win_Lib_t));
        memset(lh, 0, sizeof(ptx_Win_Lib_t));

        if (NULL != lh)
        {
            *libH = lh;

            uint32_t lib_name_len = (uint32_t)strlen(fileNameID) + 1u;

            if (lib_name_len <= PTX_HAL_WIN_UTILS_MAX_LIB_NAME)
            {
                uint32_t i;

                for (i = 0; i < lib_name_len; i++)
                {
                    lib_name[i] = (wchar_t)fileNameID[i];
                }
                lib_name[i] = (wchar_t)0;

                lh->LibH = (void *)LoadLibraryW(lib_name);
                if (NULL == lh->LibH)
                {
                    lh->LastError = (uint32_t)GetLastError();
                }

                if (NULL != lh->LibH)
                {
                    lh->IsLoaded = 1;
                    ret = PTX_STATUS_SUCCESS;
                }
                else
                {
                    lh->IsLoaded = 0;
                    ret = PTX_STATUS_E_GEN_NOT_FOUND;
                }
            }
            else
            {
                ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            }
        }
        else
        {
            ret = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        if (NULL != libH)
        {
            *libH = NULL;
        }
    }

    return ret;
}

ptx_Status_t ptx_LibClose(ptx_Win_Lib_t **libCtx)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != libCtx) && (NULL != *libCtx))
    {
        ptx_Win_Lib_t *lib_h = *libCtx;
        if (NULL != lib_h->LibH)
        {
            (void)FreeLibrary((HMODULE)lib_h->LibH);
            ret = PTX_STATUS_SUCCESS;
        }
        else
        {
            ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
        free(lib_h);
        lib_h = NULL;
    }

    return ret;
}

ptx_Status_t ptx_LibFn(ptx_Win_Lib_t *libH, const char *fnName, void **fnAddr)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != libH) && (NULL != fnAddr))
    {
        void *fn_addr = NULL;

        FARPROC proc = GetProcAddress((HINSTANCE)libH->LibH, fnName);
        if (NULL != proc)
        {
            /*
             * Make generic pointer through address/cast. Otherwise generates a warning.
             */
            fn_addr = *(void **)&proc;
        }
        else
        {
            /*
             * Could not be found/loaded.
             */
            fn_addr = NULL;
        }

        if (NULL != fn_addr)
        {
            /*
             * OK, function loaded.
             */
        }

        *fnAddr = fn_addr;

        ret = PTX_STATUS_SUCCESS;
    }

    return ret;
}

ptx_Status_t ptx_ThreadJoin(ptx_Win_Thread_t *hThd)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    int error = 0;

    if (NULL != hThd)
    {
        DWORD wfso_st = WaitForSingleObject(hThd->Thread, INFINITE);

        if (WAIT_OBJECT_0 != wfso_st)
        {
            error = 1;
        }
        ret = PTX_STATUS_SUCCESS;
    }

    if (0 != error)
    {
        ret = PTX_STATUS_E_GEN_INTERNAL;
    }

    return ret;
}

ptx_Status_t ptx_ThreadClose(ptx_Win_Thread_t *hThd)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    int error = 0;

    if (NULL != hThd)
    {
        BOOL win_ret = CloseHandle(hThd->Thread);
        error = (win_ret == 0) ? 1 : 0;

        if (0 != error)
        {
            ret = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            ret = PTX_STATUS_SUCCESS;
        }
    }

    return ret;
}

ptx_Status_t ptx_SemTimedWait(ptx_Win_Semaphore_t *hSem, uint32_t msTimeout)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != hSem)
    {
        ptx_Win_Semaphore_t sem = *hSem;
        ret = PTX_STATUS_SUCCESS;

        DWORD err = WaitForSingleObject(sem->Sem, msTimeout);

        switch (err)
        {
            case WAIT_OBJECT_0:
                /*
                 * OK
                 */
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
