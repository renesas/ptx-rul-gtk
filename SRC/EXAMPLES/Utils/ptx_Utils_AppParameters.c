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
    Module      : Utils
    File        : ptx_Utils_AppParameters.c

    Description : Defines generic (Demo-)Application-parameters which are used as inputs parameters to initialize the RUL Stack component.
                  Implements a helper-function to to parse optionally provided command-line arguments (Windows / Linux platforms only).
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Utils_AppParameters.h"
#include "ptx_Utils_Print.h"
#include <stdio.h>
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

#if defined(PTX_PLATFORM_OS)
void ptx_Utils_AppParameters_Load(uint32_t nrOfArgs, char *argv[], ptx_Utils_AppParams_t *appParams)
{
    uint32_t argument_length;
    char *current_argument;

    if ((0 != nrOfArgs) && (PTX_UTILS_MAX_NR_DEMO_PARAMS > nrOfArgs) && (NULL != argv) && (NULL != appParams))
    {
        for (uint32_t i = 0; i < nrOfArgs; i++)
        {
            current_argument = argv[i];

            /* Device-name provided as command-line argument ? */
            if (0 == strncmp(&current_argument[0], "-dn", 3))
            {
                argument_length = (uint32_t)((uint16_t)strlen(&current_argument[3]));

                if (0 < argument_length)
                {
                    (void)memset(appParams->DeviceName, 0, PTX_UTILS_DEVICE_NAME_MAX_LENGTH);
                    (void)memcpy(&appParams->DeviceName[0], &current_argument[3], argument_length);
                    appParams->DeviceNameLen = (uint16_t)argument_length;
                }
            }

            if (0 == strcmp(&current_argument[0], "-help"))
            {
                ptx_Utils_Print("===========================\n");
                ptx_Utils_Print("Supported Arguments:       \n");
                ptx_Utils_Print("-dnX .... Device Name where \"X\" designates the actual platform name of the device. Examples for \"X\": \"COM4\" on Windows, \"/dev/spidev0.0\" on Linux. \n");
                ptx_Utils_Print("===========================\n");
            }
        }
    }
}
#endif

