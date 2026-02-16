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
    Module      : Common Module Interface for Console Logging
    File        : ptx_Log_Console.c

    Description : Logging-implementation for platforms supporting Console-output via printf.
                  Contains the functions used to Initialize, de-initialize the console logging
                  mechanism and the callback to print the data.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Log_Console.h"
#include "ptx_Log_Interface.h"
#include <stdio.h>
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
static ptx_Log_Console_t gLogConsole;

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

    if ((NULL != log) && (NULL != params))
    {
        (void)memset(&gLogConsole, 0, sizeof(ptx_Log_Console_t));

        params->LogCtx  = &gLogConsole;
        params->LogType = ptx_Log_Type_Default;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_Log_Interface_Close (ptx_Log_t *log)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(log, Comp_LOG))
    {
        /* nothing to do */
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

void ptx_Log_Interface_WriteCB (ptx_Log_t *log, const char lvl, char *msg, uint32_t msgLen)
{
    if (PTX_COMP_CHECK(log, Comp_LOG) && (NULL != msg) && (0 != msgLen))
    {
        msg[msgLen] =  '\0';
        printf("%c %s", lvl, msg);
    }
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

