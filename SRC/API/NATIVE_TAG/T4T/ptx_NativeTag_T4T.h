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
    Module      : API for interfacing with a Type 4 Tag (T4T) card
    File        : ptx_NativeTag_T4T.h

    Description : NativeTag T4T implements the interface to communicate with a T4T NFC card.
*/

/**
 * \addtogroup T4T
 *
 * @{
 */

#if !defined(PTX_NATIVETAG_T4T_H)
#define PTX_NATIVETAG_T4T_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Status.h"
#include "ptx_Components.h"
#include "ptx_RUL.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
/**
 * \name T4T Tag specific definitions
 * @{
 */
#define PTX_T4T_MIN_TX_BUFFER_SIZE   (uint32_t)256   /**< Minimum Tx buffer size. */
#define PTX_T4T_MIN_BUFFER_LEN       (uint32_t)256   /**< Minimum internal data buffer length. */
/** @} */
/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief T4T CAPDU fields struct
 */
typedef struct ptx_NativeTag_T4T_CommandAPDUFields
{
    uint8_t             data_length_field[3];               /**< Lc field */
    uint8_t             nbr_data_length_bytes;              /**< length of Lc field */
    uint16_t            nbr_data_bytes;                     /**< length of data field */
    uint8_t             expected_length_field[3];           /**< Le field */
    uint8_t             nbr_expected_length_bytes;          /**< length of Le field */
    uint8_t             ber_tlv_data_field[5];              /**< ber-tlv container */
} ptx_NativeTag_T4T_CommandAPDUFields_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_NativeTag_T4T
{
    /* Components */
    ptx_Comps_t                             CompId;         /**< Component Id */
    ptx_RUL_t                               *rul;           /**< RUL provided by upper layer */
    uint8_t                                 *TxBuffer;      /**< Internal Tx-Buffer provided by upper layer (may be shared with other components) */
    ptx_NativeTag_T4T_CommandAPDUFields_t   Fields;         /**< fields used in command APDU */
} ptx_NativeTag_T4T_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_NativeTag_T4T_InitParams
{
    ptx_RUL_t           *rul;               /**< Allocated instance of RUL component */
    uint8_t             *TxBuffer;          /**< Internal Tx-Buffer provided by upper layer */
    uint32_t             TxBufferSize;      /**< Size of TX Buffer */
} ptx_NativeTag_T4T_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[in]       t4t             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T4T_Open (ptx_NativeTag_T4T_t *t4t, ptx_NativeTag_T4T_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       t4t             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T4T_Close (ptx_NativeTag_T4T_t *t4t);


/**
 * \brief Select NDEF Application for Native Tag T4T
 * \param[in]       t4t                     Pointer to an existing instance of the T4T component.
 * \param[in]       rx                      Pointer to read buffer
 * \param[in,out]   rxLen                   Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout               Timeout value
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_T4T_Select_NDEF_Application (ptx_NativeTag_T4T_t *t4t,
                                                        uint8_t *rx,
                                                        uint32_t *rxLen,
                                                        uint32_t msTimeout);


/**
* \brief Select File for Native Tag T4T
* \param[in]       t4t                     Pointer to an existing instance of the T4T component.
* \param[in]       data                    Pointer to data field
* \param[in]       rx                      Pointer to read buffer
* \param[in,out]   rxLen                   Size of Rx-buffer (in), Length of the received data (out).
* \param[in]       msTimeout               Timeout value
*
* \return Status, indicating whether the operation was successful.
*/
ptx_Status_t ptx_NativeTag_T4T_Select_File (ptx_NativeTag_T4T_t *t4t,
                                            uint8_t *data,
                                            uint8_t *rx,
                                            uint32_t *rxLen,
                                            uint32_t msTimeout);

/**
 * \brief Select command for Native Tag T4T
 *
 * \param[in]       t4t                     Pointer to an existing instance of the T4T component.
 * \param[in]       paramByte1              P1 of command APDU
 * \param[in]       paramByte2              P2 of command APDU
 * \param[in]       data                    Pointer to data field
 * \param[in]       nbrDataBytes            Number of data bytes
 * \param[in]       expectedResponseLen     Number of response bytes expected
 * \param[out]      rx                      Pointer to read buffer
 * \param[in,out]   rxLen                   Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout               Timeout value
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_T4T_Select (ptx_NativeTag_T4T_t *t4t,
                                       uint8_t paramByte1,
                                       uint8_t paramByte2,
                                       uint8_t *data,
                                       uint8_t nbrDataBytes,
                                       uint8_t expectedResponseLen,
                                       uint8_t *rx,
                                       uint32_t *rxLen,
                                       uint32_t msTimeout);

/**
 * \brief Read Binary command for Native Tag T4T
 * \param[in]       t4t                     Pointer to an existing instance of the T4T component.
 * \param[in]       offset                  Data offset
 * \param[in]       nbrExpectedBytes        Number of expected bytes
 * \param[in]       rx                      Pointer to read buffer
 * \param[in,out]   rxLen                   Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout               Timeout value
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_T4T_ReadBinary (ptx_NativeTag_T4T_t *t4t,
                                           uint16_t offset,
                                           uint8_t nbrExpectedBytes,
                                           uint8_t *rx,
                                           uint32_t *rxLen,
                                           uint32_t msTimeout);

/**
 * \brief Read Binary command using ODO for Native Tag T4T
 * \param[in]       t4t                     Pointer to an existing instance of the T4T component.
 * \param[in]       offset                  Data offset
 * \param[in]       nbrExpectedBytes        Number of expected bytes
 * \param[in]       rx                      Pointer to read buffer
 * \param[in,out]   rxLen                   Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout               Timeout value
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_T4T_ReadBinaryODO (ptx_NativeTag_T4T_t *t4t,
                                              uint32_t offset,
                                              uint8_t nbrExpectedBytes,
                                              uint8_t *rx,
                                              uint32_t *rxLen,
                                              uint32_t msTimeout);

/**
 * \brief Update Binary command for Native Tag T4T
 * \param[in]       t4t                     Pointer to an existing instance of the T4T component.
 * \param[in]       offset                  Data offset
 * \param[in]       data                    Pointer to the data field
 * \param[in]       nbrDataBytes            Number of data bytes
 * \param[in]       rx                      Pointer to read buffer
 * \param[in,out]   rxLen                   Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout               Timeout value
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_T4T_UpdateBinary (ptx_NativeTag_T4T_t *t4t,
                                             uint16_t offset,
                                             uint8_t *data,
                                             uint8_t nbrDataBytes,
                                             uint8_t *rx,
                                             uint32_t *rxLen,
                                             uint32_t msTimeout);

/**
 * \brief Update Binary command using ODO for Native Tag T4T
 * \param[in]       t4t                     Pointer to an existing instance of the T4T component.
 * \param[in]       offset                  Data offset
 * \param[in]       data                    Pointer to the data field
 * \param[in]       nbrDataBytes            Number of data bytes
 * \param[in]       rx                      Pointer to read buffer
 * \param[in,out]   rxLen                   Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout               Timeout value
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_T4T_UpdateBinaryODO (ptx_NativeTag_T4T_t *t4t,
                                                uint32_t offset,
                                                uint8_t *data,
                                                uint8_t nbrDataBytes,
                                                uint8_t *rx,
                                                uint32_t *rxLen,
                                                uint32_t msTimeout);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
