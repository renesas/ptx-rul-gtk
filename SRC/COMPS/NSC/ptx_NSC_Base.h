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
    Module      : NFC Soft Controller (NSC) Basic Interface Header
    File        : ptx_NSC_Base.h

    Description : Definition of NFC Soft Controller Basic Interface.
                  Contains function(s) to handle NSC initialization, de-initialization and reset.
*/

/**
 * \addtogroup NSC_COMPONENT
 *
 * @{
 */

#if !defined(PTX_NSC_H)
#define PTX_NSC_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NSC_Int.h"


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
 * \brief Component Initialization.
 *
 * \param[in]       nsc             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_Init (ptx_NSC_t *nsc, ptx_NSC_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       nsc   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_DeInit (ptx_NSC_t *nsc);

/**
 * \brief Reset NSC chip.
 * Hard or Soft reset shall be performed. Soft-Reset uses low level HIP Reset command.
 * Hard-Reset shall invoke GPIO API to toggle NSC chip reset pin.
 *
 * \note Hard reset shall clear CMEM (code memory) section. Soft reset does not clear CMEM.
 *
 * \note The API shall invoke reset of the NSC chip. It is expected to get NSC_RESET_NTF after
 * the API execution since DFY shall be enabled.
 *
 * \param[in]   nsc         Pointer to component.
 * \param[in]   resetType   Reset Type performed: Soft or Hard.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_Reset (ptx_NSC_t *nsc, ptx_NSC_Reset_Types_t resetType);

/**
 * \brief Retrieve NSC_RESET_NTF message.
 * Waits for the message for a specified (default NSC) timeout.
 * Parses the message and fills internal NSC structure parameters accordingly.
 *
 * \param[in]   nsc         Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_GetResetNtf(ptx_NSC_t *nsc);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
