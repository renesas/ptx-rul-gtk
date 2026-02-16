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
    Module      : NFC Soft Controller (NSC) Interface Header for Revision Information
    File        : ptx_NSC_RevisionInfo.h

    Description : Definition of NFC Soft Controller (NSC) Interface Header for Revision Information.
                  Contains function(s) to gather various system information.
*/

/**
 * \addtogroup NSC_REVISION_INFO
 *
 * @{
 */

#ifndef PTX_NSC_REVISIONINFO_H
#define PTX_NSC_REVISIONINFO_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NSC_Int.h"
#include "ptx_NSC_Registers.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name NSC version information
 * @{
 */

#define PTX_STACK_REVISION_MAJOR        1       /**< Stack Revision Major Version Number */
#define PTX_STACK_REVISION_MINOR        2       /**< Stack Revision Minor Version Number */
#define PTX_STACK_REVISION_PATCH        0       /**< Stack Revision Patch Version Number */

#define PTX_PLATFORM_REVISION           ((uint64_t)0x941f00d53)   /**< Revision number of the stack */
#define PTX_PLATFORM_LOCAL_CHANGES      0       /**< Local changes in the stack */

/** @} */

extern const uint8_t ptx_NSC_RfConfig_Version;  /**< The version of the rf config */

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
 * \brief Get various information about the system (DFY-Code/-Toochain, Chip-ID, Local changes etc.)
 *
 * \note It is up to the caller to allocate enough space for the returned information data. Actual number
 * of data bytes is returned in infoDataLen.
 *
 * \param[in]       nscCtx              Pointer to the component structure.
 * \param[in]       infoType            Type of system information to retrieve.
 * \param[out]      infoData            Buffer for the retrieved data.
 * \param[out]      infoDataLen         The length of returned data (the number of bytes returned).
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_GetInfo(ptx_NSC_t *nscCtx, ptx_NSC_InfoType_t infoType, uint8_t *infoData, uint8_t *infoDataLen);

#ifdef __cplusplus
}
#endif

#endif /* Guard */

/** @} */
