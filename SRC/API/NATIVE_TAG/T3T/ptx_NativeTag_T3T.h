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
    Module      : API for interfacing with a Type 3 Tag (T3T) card
    File        : ptx_NativeTag_T3T.h

    Description : NativeTag T3T implements the interface to communicate with a T3T NFC card.
*/

/**
 * \addtogroup T3T
 *
 * @{
 */

#if !defined(PTX_NATIVETAG_T3T_H)
#define PTX_NATIVETAG_T3T_H

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
 * \name T3T Tag specific definitions
 *
 * @{
 */
#define PTX_T3T_MIN_TX_BUFFER_SIZE              (uint32_t)256   /**< Minimum Tx Buffer Size */
#define PTX_T3T_NFCID2_SIZE                     (uint8_t)8      /**< NFCID2 Size */
#define PTX_T3T_BLOCKSIZE                       (uint8_t)16     /**< T3T block size. */
#define PTX_T3T_SENSFRES_OFFSET_NFCID2          (uint8_t)2u     /**< Offset of NFCID2 within SENSF-RES. */
#define PTX_T3T_SENSFRES_OFFSET_MRTI_CHECK      (uint8_t)15u    /**< Offset of MRTI_Check within SENSF-RES. */
#define PTX_T3T_SENSFRES_OFFSET_MRTI_UPDATE     (uint8_t)16u    /**< Offset of MRTI_Update within SENSF-RES. */
/** @} */
/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */
/**
 * \brief MRTI parameters.
 */
typedef struct ptx_NativeTag_T3T_MRTI
{
    uint8_t     MRTICheck;                      /**< MRTI for Check operations. */
    uint8_t     MRTIUpdate;                     /**< MRTI for Update operations. */
} ptx_NativeTag_T3T_MRTI_t;

/**
 * \brief Service Code List parameters.
 */
typedef struct ptx_NativeTag_T3T_Services
{
    uint8_t     NOS;                            /**< Number of services. */
    uint8_t     ServiceCodeListLen;             /**< Length of the Service Code List. */
    uint8_t     *ServiceCodeList;               /**< Service Code List. */
} ptx_NativeTag_T3T_Services_t;

/**
 * \brief Block List parameters.
 */
typedef struct ptx_NativeTag_T3T_Blocks
{
    uint8_t     NOB;                            /**< Number of blocks. */
    uint8_t     BlockListLen;                   /**< Length of Block List. */
    uint8_t     *BlockList;                     /**< Block List. */
} ptx_NativeTag_T3T_Blocks_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_NativeTag_T3T
{
    ptx_Comps_t         CompId;                         /**< Component Id */

    ptx_RUL_t           *rul;                           /**< RUL provided by upper layer */
    uint8_t             *TxBuffer;                      /**< Internal Tx-Buffer provided by upper layer (may be shared with other components) */
    uint8_t             NFCID2[PTX_T3T_NFCID2_SIZE];    /**< ID of the tag */
    uint8_t             MRTI_Check;                     /**< Value stored for calculation of timeout value */
    uint8_t             MRTI_Update;                    /**< Value stored for calculation of timeout value */
    uint32_t            TagTimeoutCheck;                /**< Timeout value of the tag for check operation */
    uint32_t            TagTimeoutUpdate;               /**< Timeout value of the tag for update operation*/
} ptx_NativeTag_T3T_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_NativeTag_T3T_InitParams
{
    ptx_RUL_t           *rul;                           /**< RUL provided by upper layer */
    uint8_t             *TxBuffer;                      /**< Internal Tx-Buffer provided by upper layer (may be shared with other components) */
    uint32_t            TxBufferSize;                   /**< Size of TX Buffer */
    uint8_t             NFCID2[PTX_T3T_NFCID2_SIZE];    /**< ID of the tag. */
    uint8_t             NFCID2Len;                      /**< Length of the ID. */
    uint8_t             MRTI_Check;                     /**< Value stored for calculation of timeout value */
    uint8_t             MRTI_Update;                    /**< Value stored for calculation of timeout value */
} ptx_NativeTag_T3T_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[in]       t3t             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T3T_Open (ptx_NativeTag_T3T_t *t3t, ptx_NativeTag_T3T_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       t3t   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T3T_Close (ptx_NativeTag_T3T_t *t3t);


/**
 * \brief T3T SENSF_REQ Command, Detect Tag and NDEF Support.
 *
 * \param[in]       t3t                 Pointer to an existing instance of the T3T component.
 * \param[in]       sc                  System code of the NFC forum device to be polled for.
 * \param[in]       rc                  Request code for additional information in the SENSF_RES.
 * \param[in]       tsn                 Time slot number, used for collision resolution.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NativeTag_T3T_SENSF_REQ (ptx_NativeTag_T3T_t *t3t,
                                          uint16_t sc,
                                          uint8_t rc,
                                          uint8_t tsn,
                                          uint8_t *rx,
                                          uint32_t *rxLen);

/**
 * \brief T3T Check Command.
 *
 * \param[in]       t3t                 Pointer to an existing instance of the T3T component.
 * \param[in]       serviceInfo         Information about Services to be checked.
 * \param[in]       blockInfo           Information about Blocks to be checked.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NativeTag_T3T_Check (ptx_NativeTag_T3T_t *t3t,
                                      ptx_NativeTag_T3T_Services_t serviceInfo,
                                      ptx_NativeTag_T3T_Blocks_t blockInfo,
                                      uint8_t *rx,
                                      uint32_t *rxLen,
                                      uint32_t msTimeout);

/**
 * \brief T3T Update Command.
 *
 * \param[in]       t3t                 Pointer to an existing instance of the T3T component.
 * \param[in]       serviceInfo         Information about Services to be updated.
 * \param[in]       blockInfo           Information about Blocks to be updated.
 * \param[in]       blockData           Data to be written to the blocks.
 * \param[in]       blockDataLen        Length of the data to be written to the blocks.
 * \param[out]      rx                  Pointer to Rx-buffer where the received data from the card shall be stored.
 * \param[in,out]   rxLen               Size of Rx-buffer (in), Length of the received data (out).
 * \param[in]       msTimeout           Timeout in ms that the function is going to wait for receiving data from the card.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NativeTag_T3T_Update (ptx_NativeTag_T3T_t *t3t,
                                       ptx_NativeTag_T3T_Services_t serviceInfo,
                                       ptx_NativeTag_T3T_Blocks_t blockInfo,
                                       uint8_t *blockData,
                                       uint8_t blockDataLen,
                                       uint8_t *rx,
                                       uint32_t *rxLen,
                                       uint32_t msTimeout);

/**
 * \brief Sets / Updates the NFCID2 to be used.
 *
 * \param[in]       t3t                 Pointer to an existing instance of the T3T component.
 * \param[in]       nbr                 Number of blocks per read operation.
 * \param[in]       nbw                 Number of blocks per write operation.
 * \param[in]       NFCID2              Pointer to NFCID2 of the given Tag.
 * \param[in]       NFCID2Len           Length of NFCID2.
 * \param[in]       mrtiInfo            Information about MRTI Times of the T3T.
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NativeTag_T3T_SetTagParams (ptx_NativeTag_T3T_t *t3t,
                                             uint8_t *NFCID2,
                                             uint8_t NFCID2Len,
                                             uint8_t nbr,
                                             uint8_t nbw,
                                             ptx_NativeTag_T3T_MRTI_t mrtiInfo);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
