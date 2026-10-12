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
    Module      : NFC Soft Controller (NSC) Interface Header for Wireless Charging (WLC)
    File        : ptx_NSC_Wlc.h

    Description : Definition of NFC Soft Controller Interface for Wireless Charging.
                  Contains the function(s) to start and stop charging and to gather
                  information during the charging process.
*/

/**
 * \addtogroup NSC_WLC
 *
 * @{
 */

#if !defined(PTX_NSC_WLC_H)
#define PTX_NSC_WLC_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Int.h"
#include "ptx_NSC_Wlc_Int.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
/**
 * \name Definitions for NSC_WLC_WPT_START_CMD
 *
 * @{
 */
#define PTX_NSC_WLC_WPT_START_CMD_LEN               (8u)        /**< NSC WPT start command code length */
/** @} */

/**
 * \name Definitions for NSC_WLC_WPT_STOP_CMD
 *
 * @{
 */
#define PTX_NSC_WLC_WPT_STOP_CMD_LEN                (2u)        /**< NSC WPT stop command code length */
/** @} */

/**
 * \name Definitions for NSC_WLC_WPT_STOP_NTF
 *
 * @{
 */
#define PTX_NSC_WLC_WPT_STOP_NTF_LEN                (2u)        /**< NSC WPT stop notification code length */
#define PTX_NSC_WLC_WPT_STOP_NTF_REASON_INDEX       (0u)        /**< Notification reason index. */
#define PTX_NSC_WLC_WPT_STOP_NTF_PARAM_LENGTH       (PTX_NSC_WLC_WPT_STOP_NTF_LEN - 1)  /**< NSC WPT stop notification parameters length. */
/** @} */

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
 * \brief Function used to start a WLC Wireless Power Transfer (WPT) .
 *
 * \param[in]       nsc         Pointer to the component structure.
 * \param[in]       options     Structure containing parameters required to start a WPT (e.g. power level).
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_WLC_WptStart(ptx_NSC_t *nsc, ptx_NSC_WLC_WptOptions_t* options);

/**
 * \brief Function used to stop an ongoing WLC Wireless Power Transfer (WPT).
 *
 * \param[in]       nsc                    Pointer to the component structure.
 * \param[in]       wptPostPowerLevel      Power level applied to the listener after the WPT.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_WLC_WptStop(ptx_NSC_t *nsc, uint8_t wptPostPowerLevel);

/**
 * \brief Processes the NSC_WLC_WPT_STOP_NTF payload.
 *
 * \param[in]       buffer         Pointer to the buffer containing the NTF payload.
 * \param[in]       bufferLen      Length of the buffer containing the NTF payload.
 * \param[in,out]   ntf            Pointer to store the parsed NTF information.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_WLC_ProcessWptStopNtf (uint8_t *buffer, uint32_t bufferLen, ptx_NSC_WLC_WptStop_Ntf_t* ntf);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
