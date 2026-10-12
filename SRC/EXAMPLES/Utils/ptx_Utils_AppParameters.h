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
    File        : ptx_Utils_AppParameters.h

    Description : Defines generic (Demo-)Application-parameters which are used as inputs parameters to initialize the RUL Stack component.
                  Implements a helper-function to to parse optionally provided command-line arguments (Windows / Linux platforms only).
*/

/**
 * \addtogroup SDK_EXAMPLE_UTILS
 *
 * @{
 */


#if !defined(PTX_UTILS_APP_PARAMETERS_H_)
#define PTX_UTILS_APP_PARAMETERS_H_


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>


#if defined(__cplusplus)
extern "C" {
#endif


/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \brief Max. number of command-line arguments.
 */
#define PTX_UTILS_MAX_NR_DEMO_PARAMS    3u

/**
 * \brief Max. number of command-line arguments.
 */
#define PTX_UTILS_DEVICE_NAME_MAX_LENGTH    64

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief  Demo Application Parameters / Arguments.
 *
 */
typedef struct ptx_Utils_AppParams
{
    char DeviceName[PTX_UTILS_DEVICE_NAME_MAX_LENGTH];          /**< Device name (e.g. "COM4" on Windows, "/dev/spidev0.0" on Linux) */
    uint16_t DeviceNameLen;                                     /**< Device name length */
} ptx_Utils_AppParams_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

#if defined(PTX_PLATFORM_OS)
/**
 * \brief Parses command-line arguments from Windows- or Linux-console and optionally overwrites previously set application-parameters.
 *
 *
 * \param[in]       nrOfArgs    Number of command-line arguments.
 * \param[in]       argv        Pointer to actual command-line arguments.
 * \param[in,out]   appParams   Pointer to Application-parameter structure.
 *
 * \return None
 */
void ptx_Utils_AppParameters_Load(uint32_t nrOfArgs, char *argv[], ptx_Utils_AppParams_t *appParams);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
