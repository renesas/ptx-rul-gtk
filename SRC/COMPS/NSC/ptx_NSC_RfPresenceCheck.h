/**
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
    Module      : NFC Soft Controller (NSC) Interface Header for Presence Check
    File        : ptx_NSC_RfPresenceCheck.h

    Description : Definition of NFC Soft Controller Interface for Presence Check.
                  Contains RF protocol specific methods for card presence checking.
*/

/**
 * \addtogroup NSC_RF_PRESCHECK
 *
 * @{
 */

#if !defined(PTX_NSC_RFPRESENCECHECK_H)
#define PTX_NSC_RFPRESENCECHECK_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Int.h"
#include "ptx_HAL.h"


#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

#define PTX_NSC_PRESCHECK_EMPTY_FRAME_LEN       (3u)        /**< Length of the empty frame for the empty frame presence check */

#define PTX_NSC_PRESCHECK_T2T_FRAMERF_LEN       (2u)        /**< Length of T2T Read command */
#define PTX_NSC_PRESCHECK_T2T_READ_CMD          (0x30u)     /**< T2T Read command */
#define PTX_NSC_PRESCHECK_T2T_MIN_RES_LEN       (1u)        /**< T2T Minimum expected response length (NACK short frame) */
#define PTX_NSC_PRESCHECK_T2T_MAX_RES_TIMEOUT   (5u)        /**< T2T Maximum allowed response time for presence check */
#define PTX_NSC_PRESCHECK_T2T_RES_BITS          (0x04)      /**< Residual bit value for T2T NACK short frame */
#define PTX_NSC_PRESCHECK_T2T_RES_BITS_MASK     (0x0F)      /**< Residual bit mask for T2T NACK short frame */
#define PTX_NSC_PRESCHECK_T2T_RES_ACK           (0x0A)      /**< T2T ACK response short frame */

#define PTX_NSC_PRESCHECK_T3T_FRAMERF_LEN       (0x10u)     /**< Length of T3T Check command */
#define PTX_NSC_PRESCHECK_T3T_CHECK_CMD         (0x06u)     /**< T3T Check command */
#define PTX_NSC_PRESCHECK_T3T_NFCID2_LEN        (0x08u)     /**< T3T NFCID2 length */
#define PTX_NSC_PRESCHECK_T3T_NOS               (1u)        /**< T3T Number of services */
#define PTX_NSC_PRESCHECK_T3T_SERVICECODE       (0x000Bu)   /**< T3T Service Code */
#define PTX_NSC_PRESCHECK_T3T_NOB               (1u)        /**< T3T Number of Blocks */
#define PTX_NSC_PRESCHECK_T3T_BLOCK_ENC         (0x80u)     /**< T3T Block encoding */
#define PTX_NSC_PRESCHECK_T3T_MIN_RES_LEN       (11u)       /**< T3T Minimum expected response length (Check response) */
#define PTX_NSC_PRESCHECK_T3T_MAX_RES_TIMEOUT   (10u)       /**< T3T Maximum allowed response time for Check response */

#define PTX_NSC_PRESCHECK_T5T_FRAMERF_LEN       (0x0Bu)     /**< Length of T5T Read Single command */
#define PTX_NSC_PRESCHECK_T5T_READ_SINGLE_CMD   (0x20u)     /**< T5T Read Single command */
#define PTX_NSC_PRESCHECK_T5T_UID_LEN           (0x08u)     /**< T5T UID length */
#define PTX_NSC_PRESCHECK_T5T_REQ_FLAG          (0x22u)     /**< T5T Request flag */
#define PTX_NSC_PRESCHECK_BLNO                  (0x00u)     /**< T5T block number to read*/
#define PTX_NSC_PRESCHECK_T5T_MIN_RES_LEN       (5u)        /**< T5T Minimum expected response length (Read Single Block with block length 4) */
#define PTX_NSC_PRESCHECK_T5T_MAX_RES_TIMEOUT   (51u)       /**< T5T Maximum allowed response time for Read Single Block */

#define PTX_NSC_PRESCHECK_MSG_FCB               (0x00u)     /**< NSC Message Frame Control Byte */

#define PTX_NSC_PRESCHECK_RCV_MSG_RF_ERR_MASK   (0xC0)      /**< Mask to check if a received NSC Data Message has transmission or collision errors */

#define PTX_NSC_PRESCHECK_SUCCESS_WAIT          (200u)      /**< Minimum wait time between 2 consecutive comands */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Frame RF Type Enumeration
*/
typedef enum ptx_NSC_PresCheck_FrameRf_Types
{
    ptx_NSC_PresCheck_FrameRf_T2T_Read,         /**< Perform Frame RF Presence Check using T2T Read command on Block 0 */
    ptx_NSC_PresCheck_FrameRf_T3T_Check,        /**< Perform Frame RF Presence Check using T3T Check command on Block 0 */
    ptx_NSC_PresCheck_FrameRf_T5T_Read,         /**< Perform Frame RF Presence Check using T5T Read single block command on Block 0 */

    ptx_NSC_PresCheck_FrameRf_Max,
} ptx_NSC_PresCheck_FrameRf_Types_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initiate ISO-DEP protocol Empty I-Block presence check.
 *
 * \param[in]       nsc             Pointer to the component structure.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfPresenceCheck_EmptyFrame (ptx_NSC_t* nsc);

/**
 * \brief Initiate ISO-DEP protocol R(NAK) presence check.
 *
 * \param[in]       nsc             Pointer to the component structure.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfPresenceCheck_RNak (ptx_NSC_t* nsc);

/**
 * \brief Initiate NFC-DEP protocol Attention Command presence check.
 *
 * \param[in]       nsc             Pointer to the component structure.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfPresenceCheck_AttentionCmd (ptx_NSC_t* nsc);

/**
 * \brief Initiate presence check for FrameRF interface (T2T, T3T, T5T).
 *
 * \param[in]       nsc             Pointer to the component structure.
 * \param[in]       frameRfType     Type of presence check
 * \param[in]       id              Id of the card
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfPresenceCheck_FrameRf (ptx_NSC_t* nsc, ptx_NSC_PresCheck_FrameRf_Types_t frameRfType, uint8_t *id);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
