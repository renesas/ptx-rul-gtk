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
    Module      : NFC Soft Controller (NSC) Interface Header for RF Activation
    File        : ptx_NSC_RfActivate.h

    Description : Definition of NFC Soft Controller Interface for RF Activation.
                  Contains function(s) to start RF Activation and to gather the information
                  during the process.
*/

/**
 * \addtogroup NSC_RF_ACTIVATE
 *
 * @{
 */


#if !defined(PTX_NSC_RFACTIVATE_H)
#define PTX_NSC_RFACTIVATE_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Int.h"
#include "ptx_NSC_RfActivate_Int.h"

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
 * \brief Function used to select card for activation when the chip is reader and there are more than one card within the field.
 *
 * \param[in]       nsc         Pointer to the component structure.
 * \param[in]       param       Pointer to the structure of the RF Activate parameters.
 * \param[in,out]   data        Pointer to data returned in NSC_RF_ACTIVATE_RSP message.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_RfActivate(ptx_NSC_t *nsc, ptx_NSC_RfAct_Param_t *param, ptx_NSC_RfAct_ProtParam_t *data);

/**
 * \brief This function parses the NSC_RF_ACTIVATE_NTF and stores it in the corresponding structure.
 *
 * \param[in]   buffer      Pointer to buffer, containing the data to be interpreted.
 * \param[in]   bufferLen   Length of the data to be interpreted.
 * \param[in]   ntf         Pointer to structure for storing the interpreted notification data.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_ProcessRfActivateNtf(uint8_t *buffer, uint32_t bufferLen, ptx_NSC_RfActivate_Ntf_t *ntf);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
