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
    Module      : LLCP
    File        : ptx_RUL_LLCP_TrxState.h

    Description : Implementation handling the connection state variables of an LLCP connection.
*/

/**
 * \addtogroup RUL_LLCP
 * @{
 */

#if !defined(PTX_RULLLCPTRXSTATE_H)
#define PTX_RULLLCPTRXSTATE_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>
#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_RUL_LLCP_Int.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initializes the LLCP-TrxState component.
 *
 * This API has to be called before any other API function of this component. It will setup & initialise all the
 * internal data structures.
 *
 * \param[in]       ctx                 Pointer to the LLCP-TrxState component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TrxState_Init(ptx_RUL_LLCP_TrxState_t *ctx);

/**
 * \brief Resets the LLCP-TrxState variables.
 *
 * Sets all the internal state variables to zero.
 *
 * \param[in]       ctx                 Pointer to the LLCP-TrxState component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TrxState_Reset(ptx_RUL_LLCP_TrxState_t *ctx);

/**
 * \brief Increments the send state variable (denotes the sequence number modulo 16).
 *
 * \param[in]       ctx                 Pointer to the LLCP-TrxState component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TrxState_IncrementSendState(ptx_RUL_LLCP_TrxState_t *ctx);

/**
 * \brief Increments the receive state variable (denotes the sequence number modulo 16).
 *
 * \param[in]       ctx                 Pointer to the LLCP-TrxState component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TrxState_IncrementReceiveState(ptx_RUL_LLCP_TrxState_t *ctx);

/**
 * \brief Sets the send state acknowledgement variable (denotes the sequence number modulo 16).
 *
 * \param[in]       ctx                 Pointer to the LLCP-TrxState component.
 * \param[in]       vsa                 Send state acknowledgement value.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TrxState_SetSendAcknowledgeState(ptx_RUL_LLCP_TrxState_t *ctx, uint8_t vsa);

/**
 * \brief Sets the receive state acknowledgement variable (denotes the sequence number modulo 16).
 *
 * \param[in]       ctx                 Pointer to the LLCP-TrxState component.
 * \param[in]       vra                 Receive state acknowledgement value.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TrxState_SetReceiveAcknowledgeState(ptx_RUL_LLCP_TrxState_t *ctx, uint8_t vra);

/**
 * \brief Tears the LLCP-TrxState component down.
 *
 * This API tears the complete LLCP-TrxState component down.
 *
 * \param[in]       ctx                 Pointer to the LLCP-TrxState component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_TrxState_DeInit(ptx_RUL_LLCP_TrxState_t *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */