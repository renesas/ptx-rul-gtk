/**
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
    Module      : SDK Example (NFC Forum Basic Discovery)
    File        : EXAMPLES/ptx_Example_NFCForumBasicDiscovery/ptx_ExampleApplication.h

    Description : This application example demonstrates how to
                  - setup and start the basic NFC Forum RF-Discovery loop,
                  - retrieve Card/Tag parameters from the Card-registry,
                  - perform simple NFC data exchanges with various Cards/Tags,
                  using the RUL-API. Typical use cases for the NFC Forum RF-Discovery
                  and data exchanges are IoT applications.
*/

/**
 * \addtogroup SDK_EXAMPLES
 *
 * @{
 */

#if !defined(_PTX_SDK_EXAMPLE_NFC_FORUM_BASIC_DISCOVERY_H_)
#define _PTX_SDK_EXAMPLE_NFC_FORUM_BASIC_DISCOVERY_H_

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * DEFINES / CONSTANTS
 * ####################################################################################################################
 */

/**
 * \brief Version number of current Demo-Application.
 */
const char *PTX_DEMO_VERSION = "V1.0";

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Demo-Application States.
 */
typedef enum ptx_DemoAppState
{
    DemoAppState_StartDiscovery,       /**< Start NFC Forum Discovery / Polling-Loop. */
    DemoAppState_WaitForDiscovery,     /**< Wait until Polling-Loop discovered, resolved and potentially also activated available card(s). */
    DemoAppState_SelectCard,           /**< Select specific card in case multiple cards were found or one card supporting multiple protocols. */
    DemoAppState_DataExchange,         /**< Perform example data exchange for every activated card using standard RUL API Exchange-functions. */
    DemoAppState_DeactivateReader,     /**< Deactivate Reader i.e. switch RF-Field off. */
    DemoAppState_RestartDiscovery,     /**< Re-Start NFC Forum Discovery / Polling-Loop. */
    DemoAppState_SystemError,          /**< Critical (System-)Error occurred. */
    DemoAppState_Exit,                 /**< Stop Demo-Application (no specific reason)). */
} ptx_DemoAppState_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

#if defined(PTX_PLATFORM_MCU)
int ptx_App_Entry(void);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

