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
    Module      : API for interfacing with a Type 2 Tag (T2T) card
    File        : ptx_NativeTag_T2T.h

    Description : NativeTag T2T implements the interface to communicate with a T2T NFC card.
*/

/**
 * \addtogroup T2T
 *
 * @{
 */

#if !defined(PTX_NATIVETAG_T2T_H)
#define PTX_NATIVETAG_T2T_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
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
 * \name T2T Tag specific definitions
 * @{
 */
#define PTX_T2T_MIN_TX_BUFFER_SIZE          (uint32_t)32       /**< Minimum Tx buffer size. */
#define PTX_T2T_BLOCK_SIZE                  (uint8_t)4         /**< T2T block size. */
#define PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN    (uint8_t)16        /**< Max. Number of Bytes in a T2T Read Response. */
#define PTX_T2T_ACK_CODE                    (uint8_t)0x0A      /**< T2T acknowledge. */
/** @} */
/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Main Component Structure
 */
typedef struct ptx_NativeTag_T2T
{
    ptx_Comps_t             CompId;         /**< Component Id. */

    ptx_RUL_t               *rul;           /**< RUL component. */

    uint8_t                 *TxBuffer;      /**< Internal Tx-Buffer provided by upper layer (may be shared with other components) */
    uint32_t                TxBufferSize;   /**< Tx buffer size */

} ptx_NativeTag_T2T_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_NativeTag_T2T_InitParams
{
    ptx_RUL_t               *rul;           /**< RUL component. */

    uint8_t                 *TxBuffer;      /**< Tx buffer */
    uint32_t                TxBufferSize;   /**< Tx buffer size */

} ptx_NativeTag_T2T_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[in]       t2t   Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T2T_Open (ptx_NativeTag_T2T_t *t2t, ptx_NativeTag_T2T_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       t2t   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T2T_Close (ptx_NativeTag_T2T_t *t2t);

/**
 * \brief Read the T2T blocks.
 *
 * \param[in]       t2t                 Pointer to an existing instance of the T2T component.
 * \param[in]       blockNr             Block number to be read (within sector).
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_T2T_Read (ptx_NativeTag_T2T_t *t2t,
                                     uint8_t blockNr,
                                     uint8_t *rx,
                                     uint32_t *rxLen,
                                     uint32_t msTimeout);

/**
 * \brief Write to the T2T blocks.
 *
 * \param[in]       t2t                 Pointer to an existing instance of the T2T component.
 * \param[in]       blockNr             Block number to be written (within sector).
 * \param[in]       blockData           Data to write.
 * \param[in]       blockDataLen        Length of data to write.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_T2T_Write (ptx_NativeTag_T2T_t *t2t,
                                      uint8_t blockNr,
                                      uint8_t *blockData,
                                      uint8_t blockDataLen,
                                      uint8_t *rx,
                                      uint32_t *rxLen,
                                      uint32_t msTimeout);

/**
 * \brief Select the T2T sector.
 *
 * \param[in]       t2t                 Pointer to an existing instance of the T2T component.
 * \param[in]       secNr               Sector number to be used (default sector is 0).
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_T2T_SectorSelect (ptx_NativeTag_T2T_t *t2t,
                                             uint8_t secNr,
                                             uint8_t *rx,
                                             uint32_t *rxLen,
                                             uint32_t msTimeout);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
