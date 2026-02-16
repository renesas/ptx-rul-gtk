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
    Module      : Operating System Abstraction Layer (OSAL) Library Functions
    File        : ptx_OSAL_Ext_Lib.c

    Description : Contains functions to interact with external libraries on Windows.
*/
#include "ptx_OSAL_Ext_Lib.h"
#include <windows.h>

ptx_Status_t ptx_OSAL_LibOpen(ptx_OSAL_t *osalCtx, ptx_OSAL_Lib_t *libH, const char *fileNameID)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    if ((NULL != osalCtx) && (NULL != fileNameID) && (NULL != libH))
    {
        ptx_OSAL_Lib_t *lh = libH;
        (void)memset(lh, 0, sizeof(ptx_OSAL_Lib_t));

        uint32_t lib_name_len = (uint32_t)strlen(fileNameID);
        if (lib_name_len >= MAX_LIB_NAME_LEN)
        {
            return PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
        wchar_t lib_name[MAX_LIB_NAME_LEN];
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

    return ret;
}

ptx_Status_t ptx_OSAL_LibClose(ptx_OSAL_t *osalCtx, ptx_OSAL_Lib_t *libH)
{
    ptx_Status_t ret = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != osalCtx) && (NULL != libH))
    {
        ptx_OSAL_Lib_t *lib_h = libH;
        if (NULL != lib_h->LibH)
        {
            (void)FreeLibrary((HMODULE)lib_h->LibH);
            lib_h->LibH = NULL;
        }

        ret = PTX_STATUS_SUCCESS;
    }

    return ret;
}

void *ptx_OSAL_GetLibFn(ptx_OSAL_t *osalCtx, ptx_OSAL_Lib_t *libH, const char *fnName)
{
    void *fn_addr = NULL;

    if ((NULL != osalCtx) && (NULL != libH))
    {
        FARPROC proc = GetProcAddress((HINSTANCE)libH->LibH, fnName);
        if (NULL != proc)
        {
            /*
             * Make generic pointer through address/cast. Otherwise generates a warning.
             */
            fn_addr = *(void **)&proc;
        }
    }

    return fn_addr;
}
