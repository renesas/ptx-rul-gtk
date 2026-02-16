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
    Module      : Utils
    File        : ptx_Utils_VersionInfo.h

    Description : Implements a helper-function to read/print all available card-parameters from the RUL Card-Registry.
*/

/**
 * \addtogroup SDK_EXAMPLE_UTILS
 *
 * @{
 */


#if !defined(PTX_UTILS_CARD_INFO_H_)
#define PTX_UTILS_CARD_INFO_H_


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_RUL.h"
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif


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
 * \brief Prints out formatted message. Depending on the underlying platform, output message shall be
 * written to a predefined Uart port (RA platform), or to standard output stream (Windows or Linux platform).
 *
 * \note The function accepts input arguments the same way as printf function.
 *
 * \param[in]   rul              Pointer to an initialized instance of the RUL API component.
 * \param[in]   cardRegistryId   Card registry ID to identify the card.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_Utils_CardInfo_Print_Card_Parameters(ptx_RUL_t *rul, uint32_t cardRegistryId);


/**
 * \brief Determines if an error is a standard rf error
 * This function can be used to determine between a true issue and an rf error that can happen due to changes in the rf field
 * such as a tag being removed, moved, or another tag causing a collision.
 * In the case of an RF Error this function returns success, else this function returns the provided error code
 *
 *
 * \param[in]   status           The status code to be checked for an RF Error.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_Utils_Check_Rf_Error(ptx_Status_t status);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
