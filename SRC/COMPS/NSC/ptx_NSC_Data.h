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
    Module      : NFC Soft Controller (NSC) Interface Header for Data Exchange
    File        : ptx_NSC_Data.h

    Description : Definition of NFC Soft Controller Interface for Data Exchange.
                  Contains function(s) to handle NSC Data messages transfer.

*/

/**
 * \addtogroup NSC_DATA_HANDLERS
 *
 * @{
 */

#if !defined(PTX_NSC_DATA_H)
#define PTX_NSC_DATA_H

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
 * \brief Transmits arbitrary data
 *
 * \param[in] nsc        Pointer to component.
 * \param[in] payload    Pointer to data.
 * \param[in] len        Length of data.
 * \param[in] isChained  Indicates whether the message is chained or not.
 * \param[in] timeout    Timeout in ms used for waiting for an acknowledgement if data was chained.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_DataTx(ptx_NSC_t *nsc, uint8_t *payload, uint32_t len, uint8_t isChained, uint32_t timeout);

/**
 * \brief Receives arbitrary data.
 *
 * \param[in]   nsc       Pointer to component.
 * \param[out]  payload   Pointer to data.
 * \param[out]  len       Length of data.
 * \param[in]   timeout   Timeout in milliseconds to stop the operation.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_DataRx(ptx_NSC_t *nsc, uint8_t *payload, uint32_t *len, uint32_t timeout);

/**
 * \brief Transmits control message
 *
 * \param[in] nsc       Pointer to component.
 * \param[in] msgType   The type of control message to send.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_CtrlTx(ptx_NSC_t *nsc, uint8_t msgType);

/**
 * \brief Assembles and transmits a bit-stream message.
 *
 * \param[in] nsc           Pointer to component.
 * \param[in] payload       Pointer to data to be formatted into a bit-stream.
 * \param[in] parityBits    Pointer to parity bits to be added into a bit-stream.
 * \param[in] len           Length of data.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_CltMsg (ptx_NSC_t *nsc, uint8_t *payload, uint8_t *parityBits, uint32_t len);

/**
 * \brief Enable CLT message mode.
 *
 * \param[in] nsc           Pointer to component.
 * \param[in] enable        Enable if 0, disable else.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_CltMsg_Enable(ptx_NSC_t *nsc, ptx_NSC_CltMode_t enable);

/**
 * \brief Sets the number of residual tx bits
 *
 * \param[in] nsc                Pointer to component.
 * \param[in] nrResidualTxBits   Number of residual tx bits.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_DataSetNrResidualTxBits(ptx_NSC_t *nsc, uint8_t nrResidualTxBits);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
