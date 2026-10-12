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
    Module      : Common Module: Logging Interface Implementation
    File        : ptx_Log.c

    Description : Log core implementation. Contains component definition and the API to invoke
                  specific logger implementation (logger type) and log messages formatter.
                  The Message formatter is based on "zf_log" (see https://github.com/wonder-mice/zf_log).
                  Logger type is defined by the user by using the appropriate log-initialization parameters.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <string.h>
#include "ptx_Log.h"
#include "ptx_Log_Interface.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * ZF logger definitions
 */
#define MAX_LOG_BUF_SZ 512

#if !defined(PTX_LOG_DISABLED)
    static const unsigned LOG_PUT_MASK = ZF_LOG_PUT_SRC | ZF_LOG_PUT_MSG;
#endif

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
#if !defined(PTX_LOG_DISABLED)
    static void ptx_Log_MsgTrx(const zf_log_message *msg, void *arg);
#endif

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_Log_Init (ptx_Log_t *log, ptx_Log_InitParams_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != log)
    {
#if !defined(PTX_LOG_DISABLED)
        ptx_Log_InitParams_t init_params;

        (void)memset(log, 0, sizeof(ptx_Log_t));
        (void)memset(&init_params, 0, sizeof(ptx_Log_InitParams_t));

        /* default initialization - may be overwritten by implemented Log-variant */
        log->LogType = ptx_Log_Type_None;

        /* If the caller provided valid log init parameters, use those. Otherwise, initialize the interface.  */
        status = ptx_Log_Interface_Open(log, &init_params);

        if ((PTX_STATUS_SUCCESS == status) && (ptx_Log_Type_None != init_params.LogType))
        {
            log->CompId = Comp_LOG;
            log->LogType = init_params.LogType;
            log->LogCtx = init_params.LogCtx;

            /* Default formatter is ZF log. Set the callback which logs the messages formatted by ZF. */
            zf_log_set_output_v(LOG_PUT_MASK, log, ptx_Log_MsgTrx);

        }
#else
        /* In case we disabled logging, just check if context was set correctly. */
        status = PTX_STATUS_SUCCESS;
#endif
    }

    /* Init_params are RFU */
    PTX_UNUSED_PARAMETER(params);

    return status;
}

ptx_Status_t ptx_Log_DeInit (ptx_Log_t *log)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

#if !defined(PTX_LOG_DISABLED)
    if (PTX_COMP_CHECK(log, Comp_LOG))
    {
        /* Close the logger. */
        status = ptx_Log_Interface_Close(log);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Clean the component members only in case of successful closure. */
            (void)memset(log, 0, sizeof(ptx_Log_t));
        }
    }
#else
    status = PTX_STATUS_SUCCESS;
    PTX_UNUSED_PARAMETER(log);
#endif
    return status;
}

void ptx_Log_SetLevelChar (ptx_Log_t *log, char lvlChar)
{
    if (PTX_COMP_CHECK(log, Comp_LOG))
    {
        log->LogLevel = lvlChar;
    }
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
#if !defined(PTX_LOG_DISABLED)
static void ptx_Log_MsgTrx(const zf_log_message *msg, void *arg)
{
    ptx_Log_t *log_ctx = (ptx_Log_t *)arg;

    if (PTX_COMP_CHECK(log_ctx, Comp_LOG))
    {
        /* p points to the log message end. By default, message is not terminated
         * with 0, but it has some space allocated for EOL area, so there is always
         * some place for terminating zero in the end (see ZF_LOG_EOL_SZ define in
         * zf_log.c).
         */
        *msg->p = 0;

        /* Transfer message via the underlying communication channel. */

        /* Append CR LF */
        uint32_t len = ((uint32_t)strlen(msg->buf));
        uint32_t total_len = len;

        if ((len + 2) < MAX_LOG_BUF_SZ)
        {
            msg->buf[len] = 0x0D;
            msg->buf[len + 1] = 0x0A;
            msg->buf[len + 2] = 0;
            total_len = len + 2;
        }
        else if ((len < MAX_LOG_BUF_SZ) && (len >= 2))
        {
            msg->buf[len - 2] = 0x0D;
            msg->buf[len - 1] = 0x0A;
            msg->buf[len] = 0;
            total_len = len;
        }
        else
        {
            /* Leave it be. */
        }

        /* Write actual Log-Entry */
        ptx_Log_Interface_WriteCB(log_ctx, log_ctx->LogLevel, msg->buf, total_len);
    }
}
#endif
