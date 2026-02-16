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
    Module      : NFC Soft Controller (NSC) Interface Header for RF Deactivation
    File        : ptx_NSC_RfDeactivate.h

    Description : Definition of NFC Soft Controller Interface for RF Deactivation.
                  Contains functions(s) to start RF deactivation and to gather information
                  during the process.
*/

/**
 * \addtogroup NSC_RF_DEACTIVATE
 *
 * @{
 */

#if !defined(PTX_NSC_RFDEACTIVATE_H)
#define PTX_NSC_RFDEACTIVATE_H

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
 * \brief Deactivate NSC and turn it into one of the selected RF states.
 *
 * The function can be used to stop RF Discovery process if RF_IDLE param->State is set.
 * Discovery can be re-started if RF_DISCOVERY (with or without RF field reset) is set.
 * Setting RF_POLL param->State will trigger immediate transition to RF_POLL state.
 *
 * \note 1: State transition highly depends on the current NSC state, so not all transitions are allowed in a certain state.
 *
 * \note 2: A special case when this API might return an error is the exit from Standby mode in RF_DISCOVERY state. In that case,
 * the command shall be properly sent, but the received message shall not be RF_DEACTIVATE_RSP. Instead, NSC_ERROR_NTF (Standby Error)
 * is received, and automatically dispatched to RUL error notification handler. Therefore, the API returns an error.
 * NSC shall be in RF_IDLE state after that notification. It is up to the caller (application) to properly handle the returned error,
 * since this is not a critical system error, but an expected system behaviour.
 *
 * \param[in]       nsc         Pointer to the component structure.
 * \param[in]       param       Pointer to the structure of the RF Deactivate parameters.
 * \param[in,out]   ntfToFollow Pointer to variable indicating if deactivation was completed immediately.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_RfDeactivate(ptx_NSC_t* nsc, ptx_NSC_RfDeact_Param_t* param, uint8_t* ntfToFollow);

/**
 * \brief This function parses the NSC_DEACTIVATE_NTF and stores it in the corresponding structure.
 *
 * \param[in]   buffer      Pointer to buffer, containing the data to be interpreted.
 * \param[in]   bufferLen   Length of the data to be interpreted.
 * \param[in]   ntf         Pointer to structure for storing the interpreted notification data.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_ProcessRfDeactivateNtf(uint8_t *buffer, uint32_t bufferLen, ptx_NSC_RfDeactivate_Ntf_t *ntf);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
