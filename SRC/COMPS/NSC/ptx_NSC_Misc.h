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
    Module      : NFC Soft Controller (NSC) Interface Header for Memory Access
    File        : ptx_NSC_Misc.h

    Description : Definition of NFC Soft Controller Interface for Memory Access.
                  Contains function(s) to access the NSC memory, get status of the
                  latest operation or to invoke NSC specific (test) activity.
*/

/**
 * \addtogroup NSC_MISC
 *
 * @{
 */

#if !defined(PTX_NSC_MISCELANEOUS_H)
#define PTX_NSC_MISCELANEOUS_H

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

/**
 * \brief Internal Parameters Status
 */
typedef struct ptx_NSC_ParamStatus
{
    uint8_t     DdpcPl;             /**< Current DDPC power level. */
    uint32_t    Rssi;               /**< The last measured RSSI value. */
    uint8_t     CollInfo;           /**< Collision information of the last reception. */
    uint8_t     Temperature;        /**< IC temperature in degree celsius. */
    uint16_t    RfFieldStrength;    /**< Remote RF field strength */
} ptx_NSC_ParamStatus_t;



/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Read data from the internal Data Memory of PTX2xxR/W.
 *
 * \note This function builds the NSC_READ_CMD, transmits it to the PTX2xxR/W and receives the NSC_READ_RSP sent by PTX2xxR/W.
 *
 * \param[in]   nsc                 Pointer to the component structure.
 * \param[in]   address             Start address of the data source. Expected in LSB format.
 * \param[in]   len                 Number of bytes to read. May be 0.
 * \param[out]  data                Data read from the address specified in \p address.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_Read(ptx_NSC_t *nsc, uint16_t address, uint8_t len, uint8_t *data);

/**
 * \brief Write data to the internal Data Memory of PTX2xxR/W.
 *
 * \note This function builds the NSC_WRITE_CMD, transmits it to the PTX2xxR/W and receives the NSC_WRITE_RSP sent by PTX2xxR/W.
 *
 * \param[in]   nsc                 Pointer to the component structure.
 * \param[in]   address             Start address of the data destination. Expected in LSB format.
 * \param[in]   len                 Number of bytes to write. Must not be 0.
 * \param[in]   data                Data to be written at the address \p address.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_Write(ptx_NSC_t *nsc, uint16_t address, uint8_t len, uint8_t *data);

/**
 * \brief Invoke sending a loopback message.
 *
 * \note This function builds the NSC_LOOPBACK_CMD, transmits it to the PTX2xxR/W, receives the NSC_LOOPBACK_RSP sent by PTX2xxR/W,
 * and checks whether the received data matches the data which was sent. If the data does not match, an error is returned.
 *
 * \param[in]   nsc                 Pointer to the component structure.
 * \param[in]   len                 Number of payload bytes to send/receive. May be 0.
 * \param[in]   data                Data to be sent.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_Loopback(ptx_NSC_t *nsc, uint8_t len, uint8_t *data);

/**
 * \brief Get current status of some internal parameters.
 *
 * \param[in]   nsc                 Pointer to the component structure.
 * \param[out]  paramStatus         Structure that holds all the returned status values.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_GetStatus(ptx_NSC_t *nsc, ptx_NSC_ParamStatus_t *paramStatus);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
