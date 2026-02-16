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
    Module      : NFC Forum Device Test Application (DTA) Listen Processing
    File        : ptx_NFCForumDTA_Listen_Processing.h

    Description : Contains the implementation to handle the processing of Listen Mode in the NFC Forum DTA. It controls the
                  processing if the NFC Forum DTA is running in Listen Mode.
*/

/**
 * \addtogroup DTA
 *
 * @{
 */

#if !defined(PTX_NFCFORUMDTA_LISTEN_PROCESSING_H)
#define PTX_NFCFORUMDTA_LISTEN_PROCESSING_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

#include "ptx_Status.h"
#include "ptx_Components.h"

#include "ptx_NFCForumDTA_Configuration.h"

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */



/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Process Listen Data Exchange.
 *
 * \param[in]        dta            Pointer to initialized component.
 * \param[in]        protocol       Currently active protocol
 * \param[in]        rx             Buffer with received input data to be processed.
 * \param[in]        rxLen          Length of input buffer
 * \param[in,out]    testCfg        Test configuration
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NFCForumDTA_Listen_Processing_CommandProcess(ptx_NFCForumDTA_t *dta, uint8_t protocol, uint8_t *rx, uint32_t rxLen, ptx_NFCForumDTA_Test_Config_t *testCfg);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
