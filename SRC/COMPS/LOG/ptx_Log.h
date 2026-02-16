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
    Module      : Common Module: Logging Interface Implementation
    File        : ptx_Log.h

    Description : Log core implementation. Contains component definition and the API to invoke
                  specific logger implementation (logger type) and log messages formatter.
                  The Message formatter is based on "zf_log" (see https://github.com/wonder-mice/zf_log).
                  Logger type is defined by the user by using the appropriate log-initialization parameters.
*/

/**
 * \addtogroup LOG
 *
 * @{
 */

#if !defined(PTX_LOG_H)
#define PTX_LOG_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Components.h"
#include "ptx_Status.h"
#include <stddef.h>

#if !defined(PTX_LOG_DISABLED)
    /* Note: zf_log specific defines need to be set before the inclusion of the Header to become effective.  "*/
    #define ZF_LOG_OUTPUT_LEVEL ZF_LOG_VERBOSE                      /**< Default Log Output Level */
    #define ZF_LOG_LEVEL        ZF_LOG_VERBOSE                      /**< Default Log Level */
    #include "zf_log.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif


/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * PTX_LOG_DISABLED is a compile option which is set when none of the supported logger types has been chosen.
 */
#if !defined(PTX_LOG_DISABLED)
/**
 * \brief Write String using Logging-Level I = Info.
 */
#define  PTX_LOG_I(logCtx, ...) \
    ptx_Log_SetLevelChar(logCtx, 'I');  \
    ZF_LOGI( __VA_ARGS__);

/**
 * \brief Write String using Logging-Level V = Verbose.
 */
#define  PTX_LOG_V(logCtx, ...) \
    ptx_Log_SetLevelChar(logCtx, 'V');  \
    ZF_LOGV( __VA_ARGS__);

/**
 * \brief Write String using Logging-Level D = Debug.
 */
#define  PTX_LOG_D(logCtx, ...) \
    ptx_Log_SetLevelChar(logCtx, 'D');  \
    ZF_LOGD( __VA_ARGS__);

/**
 * \brief Write String using Logging-Level W = Warning.
 */
#define  PTX_LOG_W(logCtx, ...) \
    ptx_Log_SetLevelChar(logCtx, 'W');  \
    ZF_LOGW( __VA_ARGS__);

/**
 * \brief Write String using Logging-Level E = Error.
 */
#define  PTX_LOG_E(logCtx, ...) \
    ptx_Log_SetLevelChar(logCtx, 'E');  \
    ZF_LOGE( __VA_ARGS__);

/**
 * \brief Write String using Logging-Level F = Fatal.
 */
#define  PTX_LOG_F(logCtx, ...) \
    ptx_Log_SetLevelChar(logCtx, 'F');  \
    ZF_LOGF( __VA_ARGS__);

/**
 * \brief Write Memory Space / Data and String using Logging-Level I = Info.
 */
#define  PTX_LOG_I_MEM(logCtx, memBuffer, memBufferSize,...) \
    ptx_Log_SetLevelChar(logCtx, 'I');  \
    ZF_LOGI_MEM(memBuffer, (unsigned int)memBufferSize, __VA_ARGS__);

/**
 * \brief Write Memory Space / Data and String using Logging-Level V = Verbose.
 */
#define  PTX_LOG_V_MEM(logCtx, memBuffer, memBufferSize,...) \
    ptx_Log_SetLevelChar(logCtx, 'V');  \
    ZF_LOGV_MEM(memBuffer, (unsigned int)memBufferSize, __VA_ARGS__);

/**
 * \brief Write Memory Space / Data and String using Logging-Level D = Debug.
 */
#define  PTX_LOG_D_MEM(logCtx, memBuffer, memBufferSize,...) \
    ptx_Log_SetLevelChar(logCtx, 'D');  \
    ZF_LOGD_MEM(memBuffer, (unsigned int)memBufferSize, __VA_ARGS__);

#else

/**
 * \brief Empty Logging-Level I in case Log is disabled.
 */
#define  PTX_LOG_I(logCtx, ...)

/**
 * \brief Empty Logging-Level V in case Log is disabled.
 */
#define  PTX_LOG_V(logCtx, ...)

/**
 * \brief Empty Logging-Level D in case Log is disabled.
 */
#define  PTX_LOG_D(logCtx, ...)

/**
 * \brief Empty Logging-Level W in case Log is disabled.
 */
#define  PTX_LOG_W(logCtx, ...)

/**
 * \brief Empty Logging-Level E in case Log is disabled.
 */
#define  PTX_LOG_E(logCtx, ...)

/**
 * \brief Empty Logging-Level F in case Log is disabled.
 */
#define  PTX_LOG_F(logCtx, ...)

/**
 * \brief Empty Logging-Level I-MEM in case Log is disabled.
 */
#define  PTX_LOG_I_MEM(logCtx, memBuffer, memBufferSize, ...)

/**
 * \brief Empty Logging-Level V-MEM in case Log is disabled.
 */
#define  PTX_LOG_V_MEM(logCtx, memBuffer, memBufferSize, ...)

/**
 * \brief Empty Logging-Level D-MEM in case Log is disabled.
 */
#define  PTX_LOG_D_MEM(logCtx, memBuffer, memBufferSize, ...)

#endif

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * Typedef (including forward declarations)
 */
typedef void *ptx_Log_LogCbCtx_t;

/**
 * \brief Logger type used.
 */
typedef enum ptx_Log_Type
{
    ptx_Log_Type_None,      /**< Log Type NONE */
    ptx_Log_Type_Default,   /**< Log Type DEFAULT (e.g. Console, Serial, File, RTT, ...)*/

} ptx_Log_Type_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_Log_InitParams
{
    ptx_Log_Type_t          LogType;            /**< Logger type to be used: provided by the caller, default (internal), or none (no logging used). */

    ptx_Log_LogCbCtx_t      LogCtx;             /**< Context for Log callback */

} ptx_Log_InitParams_t;

/**
 * \brief Main Component Struture
 */
typedef struct ptx_Log
{
    ptx_Comps_t             CompId;             /**< Logger component ID. */
    ptx_Log_Type_t          LogType;            /**< Logger used by the current application: external - provided by the caller, otherwise - use default. */

    ptx_Log_LogCbCtx_t      LogCtx;             /**< Context for Log callback */
    char                    LogLevel;           /**< Log level character */

} ptx_Log_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 * User-defined set of callback routines can be provided within initParams if the user wants to customize
 * the logging. If no parameters are provided, default logger will be used.
 *
 * \param[in]       log             Pointer to component.
 * \param[in]       params           Pointer to initialisation parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_Log_Init (ptx_Log_t *log, ptx_Log_InitParams_t *params);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       log   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_Log_DeInit (ptx_Log_t *log);

/**
 * \brief Sets Log-Level to be used in Macros.
 *
 * \param[in]       log     Pointer to component.
 * \param[in]       lvlChar Log level character.
 *
 */
void ptx_Log_SetLevelChar (ptx_Log_t *log, char lvlChar);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
