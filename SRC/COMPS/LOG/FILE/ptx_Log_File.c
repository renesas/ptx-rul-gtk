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
    Module      : Common Module Interface for File Logging
    File        : ptx_Log_File.c

    Description : Logging-implementation for platforms supporting writing into Text-files.
                  Contains the functions used to Initialize, de-initialize the console logging
                  mechanism and the callback to print the data.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Log_File.h"
#include "ptx_Log_Interface.h"
#include <stdio.h>
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
static ptx_Log_File_t gLogFile;

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
ptx_Status_t ptx_Log_Interface_Open (ptx_Log_t *log, ptx_Log_InitParams_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != params)
    {
        (void)memset(&gLogFile, 0, sizeof(ptx_Log_File_t));

        params->LogCtx  = &gLogFile;
        params->LogType = ptx_Log_Type_Default;

        sprintf(&gLogFile.FilePath[0], "%s%s%s", PTX_LOG_FILE_PATH, PTX_LOG_PLAT_SEPERATOR, PTX_LOG_FILE_NAME);

        gLogFile.IsCacheOverflow = 0;
        gLogFile.LogCacheIndex = 0;

#if defined(PTX_LOG_FILE_DIRECT_WRITE_MODE)
        gLogFile.FileHandle = (void*)fopen(gLogFile.FilePath, "a");
#else
        gLogFile.FileHandle = (void*)fopen(gLogFile.FilePath, "w");
#endif

        if (NULL != gLogFile.FileHandle)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_OPEN;
        }
    }

    PTX_UNUSED_PARAMETER(log);

    return status;
}

ptx_Status_t ptx_Log_Interface_Close (ptx_Log_t *log)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(log, Comp_LOG))
    {
        ptx_Log_File_t *log_ctx = (ptx_Log_File_t *)log->LogCtx;

#if !defined(PTX_LOG_FILE_DIRECT_WRITE_MODE)
        /* In case of an overflow of the Log-cache, print the entire cache */
        uint32_t nr_entries_to_write = (0 == log_ctx->IsCacheOverflow) ? log_ctx->LogCacheIndex : PTX_LOG_FILE_NR_LOG_CACHE_ENTRIES;
        for (uint32_t i = 0; i < nr_entries_to_write; i++)
        {
            fprintf((FILE*)log_ctx->FileHandle, "%s", &log_ctx->LogCache[i][0]);
        }
#endif
        (void)fclose((FILE*)log_ctx->FileHandle);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

void ptx_Log_Interface_WriteCB (ptx_Log_t *log, const char lvl, char *msg, uint32_t msgLen)
{
    if (PTX_COMP_CHECK(log, Comp_LOG) && (NULL != msg))
    {
        ptx_Log_File_t *log_ctx = (ptx_Log_File_t *)log->LogCtx;

#if defined(PTX_LOG_FILE_DIRECT_WRITE_MODE)
        fprintf((FILE*)log_ctx->FileHandle, "%06d: %c %s", log_ctx->LogCacheIndex, lvl, msg);
        fflush((FILE*)log_ctx->FileHandle);
        /*Write directly into file - no overflow-protection needed for the index*/
        log_ctx->LogCacheIndex++;
#else
        snprintf(&log_ctx->LogCache[log_ctx->LogCacheIndex][0], PTX_LOG_FILE_ENTRY_MAX_LENGTH, "%06d: %c %s", (int)log_ctx->LogCacheIndex, lvl, msg);
        log_ctx->LogCacheIndex++;

        if (log_ctx->LogCacheIndex >= PTX_LOG_FILE_NR_LOG_CACHE_ENTRIES)
        {
            log_ctx->LogCacheIndex = 0;
            log_ctx->IsCacheOverflow = 1u;
        }
#endif
    }

    PTX_UNUSED_PARAMETER(msgLen);
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

