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
    Module      : RUL
    File        : ptx_RUL_Int.h

    Description : Reader Universal Library (RUL) internal function definition header file
                  Contains internal functions that the RUL module uses internally.
*/

/**
 * \addtogroup RUL
 *
 * @{
 */

#if !defined(PTX_RUL_INT_H)
#define PTX_RUL_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

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
 * \brief RfDiscovery notification handler.
 * NSC command processor discovery notification handler is loaded with this function callback.
 *
 * \param[in]   ctx     Pointer to a callback context (RUL component).
 * \param[in]   ntf     Pointer to discover notification payload structure.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ProcessRfDiscoverNtf (void *ctx, ptx_NSC_RfDiscover_Ntf_t *ntf);

/**
 * \brief RfLpcd notification handler.
 * NSC command processor LPCD notification handler is loaded with this function callback.
 *
 * \param[in]   ctx     Pointer to a callback context (RUL component).
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ProcessRfLpcdNtf (void *ctx);

/**
 * \brief Reset notification handler.
 * NSC command processor reset notification handler is loaded with this function callback.
 *
 * \param[in]   ctx     Pointer to a callback context (RUL component).
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ProcessResetNtf (void *ctx);

/**
 * \brief RfActivate notification handler.
 * NSC command processor activate notification handler is loaded with this function callback.
 *
 * \param[in]   ctx     Pointer to a callback context (RUL component).
 * \param[in]   ntf     Pointer to activate notification payload structure.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ProcessRfActivateNtf (void *ctx, ptx_NSC_RfActivate_Ntf_t *ntf);

/**
 * \brief RfDeactivate notification handler.
 * NSC command processor deactivate notification handler is loaded with this function callback.
 *
 * \param[in]   ctx     Pointer to a callback context (RUL component).
 * \param[in]   ntf     Pointer to deactivate notification payload structure.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ProcessRfDeactivateNtf (void *ctx, ptx_NSC_RfDeactivate_Ntf_t *ntf);

/**
 * \brief Error notification handler.
 * NSC command processor error notification handler is loaded with this function callback.
 *
 * \param[in]   ctx     Pointer to a callback context (RUL component).
 * \param[in]   ntf     Pointer to error notification payload structure.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ProcessErrorNtf (void *ctx, ptx_NSC_Error_Ntf_t *ntf);

/**
 * \brief Field Notification handler.
 *
 * \param[in]   ctx     Pointer to a callback context (RUL component).
 * \param[in]   field   Indicates if the RF Field is off (0) or on (1)
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ProcessRfFieldNtf (void *ctx, uint8_t field);

/**
 * \brief WLC WPT notification handler.
 * NSC command processor WLC Wireless Power Transfer (WPT) handler is loaded with this function callback.
 *
 * \param[in]   ctx     Pointer to a callback context (RUL component).
 * \param[in]   ntf     Pointer to WPT notification payload structure.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ProcessWlcWptStopNtf (void *ctx, ptx_NSC_WLC_WptStop_Ntf_t *ntf);

/**
 * \brief Data message notification handler.
 * NSC command processor data message handler is loaded with this function callback.
 *
 * \param[in]   ctx     Pointer to a callback context (RUL component).
 * \param[in]   msg     Pointer to data message structure.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ProcessDataMsg (void *ctx, ptx_NSC_RfData_Msg_t *msg);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
