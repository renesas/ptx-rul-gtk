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
    Module      : FELICA
    File        : ptx_FeliCa.h

    Description : Felicity Card (FeliCa) library implementing standard FeliCa commands.
*/

/**
 * \addtogroup FELICA
 *
 * @{
 */

#if !defined(PTX_FELICA_H)
#define PTX_FELICA_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>
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
 * \brief Max. supported FeliCa cards - can be changed depending on target application
 */
#define PTX_MAX_SUPPORTED_FELICA_CARDS                      (uint8_t)3      /**< Max. supported FeliCa cards */

/**
 * \brief FeliCa card Parameter & general Definitions
 */
#define PTX_FELICA_IDM_MAX_LENGTH                           8              /**< Manufacture ID Length */
#define PTX_FELICA_PMM_MAX_LENGTH                           8              /**< Manufacture Parameter Length */
#define PTX_FELICA_RD_MAX_LENGTH                            2              /**< Request Data Length */
#define PTX_FELICA_BLOCK_LEN                                16             /**< FeliCa Block Size */

/**
 * \brief FeliCa Timeout Calculation: Delta value in [us] to be added to variable T (256 * 16 / fc), can be used on
 *        platforms where the timing calculation can't be implemented very accurately like on PC-platforms.
 */
#define PTX_FELICA_T_DELTA_US                               5000U

/**
 * \brief Polling starts initially with given timeslot number and in case of a collision (2 or more responses in same timeslot),
 *        one additional polling cycle with timeslot number = 16 gets performed (2nd cycle is according to [NFC Forum Activity]).
 *        In case of many cards present, it can still happen that even with the 2nd cycle, that not all cards can be resolved.
 *        The define below can be used to perform an additional polling cycles a backup solution to increase the likelihood
 *        that really all cards get resolved. The default value must at least be set to 1 or higher.
 *        Examples:
 *        o) The application always just uses a single card - then the value can be set to 1.
 *           Only a single cycle will be performed anyway because there's no collision.
 *        o) The application always uses an inital timeslot value of 16, the likelihood of getting a collision increases with
 *           an increased number of present cards. An additional cycle may be recommended i.e. setting the define to 2.
 */
#define PTX_FELICA_ADDITIONAL_POLLING_CYCLES                (uint8_t)1

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief FeliCa Request Codes (RC)
 */
typedef enum ptx_FeliCa_RequestCode
{
    FeliCa_RC_NoRequestCode             = 0,
    FeliCa_RC_SystemCodeRequested       = 1,
    FeliCa_RC_AdvancedProtocolFeatures  = 2,

} ptx_FeliCa_RequestCode_t;

/**
 * \brief FeliCa Timeslot Numbers (TSN)
 */
typedef enum ptx_FeliCa_TimeslotNr
{
    FeliCa_TSN_01   = 0,
    FeliCa_TSN_02   = 1,
    FeliCa_TSN_04   = 3,
    FeliCa_TSN_08   = 7,
    FeliCa_TSN_16   = 15,

} ptx_FeliCa_TimeslotNr_t;

/**
 * \brief FeliCa card Data
 */
typedef struct ptx_FeliCa_CardData
{
    uint8_t                 IDm[PTX_FELICA_IDM_MAX_LENGTH];                 /**< Manufacture ID */
    uint8_t                 PMm[PTX_FELICA_PMM_MAX_LENGTH];                 /**< Manufacture Parameter */
    uint8_t                 RD[PTX_FELICA_RD_MAX_LENGTH];                   /**< Request Data */
    uint8_t                 RDLength;                                       /**< Request Data Length (2 = Present, 0 = Not Present) */

} ptx_FeliCa_CardData_t;

/**
 * \brief FeliCa card Parameters
 */
typedef struct ptx_FeliCa_CardParam
{
    ptx_FeliCa_CardData_t    CardData[PTX_MAX_SUPPORTED_FELICA_CARDS];       /**< Card Data for available FeliCa cards */
    uint8_t                 NrAvailableCards;                               /**< Number of available cards */
    uint8_t                 ActiveCardIndex;                                /**< Marks currently active FeliCa Card (default = 0) */

} ptx_FeliCa_CardParam_t;


/**
 * \brief FeliCa component.
 */
typedef struct ptx_FeliCa
{
    ptx_Comps_t             CompId;                                 /**< Component ID. */
    ptx_RUL_t               *Rul;                                   /**< Pointer to initialised RUL component. */

    ptx_FeliCa_CardParam_t  CardParams;                             /**< FeliCa Card Parameters */
    uint16_t                LastStatus;                             /**< Status of previously/last executed FeliCa-Read/-Write command */

    uint8_t                 *Tx;                                    /**< Pointer to Tx work-buffer */
    uint32_t                TxLen;                                  /**< Length of data to transmit */
    uint8_t                 *Rx;                                    /**< Pointer to Rx work-buffer */
    uint32_t                RxLen;                                  /**< Length of data received */

    uint8_t                 *RxPayload;                             /**< Pointer to received payload */
} ptx_FeliCa_t;

/**
 * \brief FeliCa component. Initialisation Parameters
 */
typedef struct ptx_FeliCa_InitParams
{
    ptx_RUL_t               *Rul;               /**< Pointer to initialised RUL component. */
    uint8_t                 *TxBuffer;          /**< Pointer to temporary Tx work-buffer allocated at application level */
    uint32_t                TxBufferSize;     /**< Size of temporary Tx work-buffer allocated at application level */
    uint8_t                 *RxBuffer;          /**< Pointer to temporary Rx work-buffer allocated at application level */
    uint32_t                RxBufferSize;     /**< Size of temporary Rx work-buffer allocated at application level */
} ptx_FeliCa_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialisation.
 *
 * \param[in]       feliCa          Pointer to component.
 * \param[in]       initParams      Pointer to initialisation parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_FeliCa_Open (ptx_FeliCa_t *feliCa, ptx_FeliCa_InitParams_t *initParams);

/**
 * \brief Component De-Initialisation.
 *
 * \param[in]       feliCa          Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_FeliCa_Close (ptx_FeliCa_t *feliCa);

/**
 * \brief Internal reset of component state.
 *
 * \param[in]       feliCa          Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_FeliCa_ResetState (ptx_FeliCa_t *feliCa);

/**
 * \brief Resets the internal information about available FeliCa cards.
 *
 * \param[in]   feliCa               Pointer to an existing instance of the FeliCa component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t
 */
ptx_Status_t ptx_FeliCa_ResetCardInfo (ptx_FeliCa_t *feliCa);

/**
 * \brief Performs the FeliCa polling procedure and activates all available cards (limited by \ref PTX_MAX_SUPPORTED_FELICA_CARDS).
 *
 * \param[in]       feliCa           Pointer to an existing instance of the FeliCa component.
 * \param[in]       systemCode       FeliCa System-Code.
 * \param[in]       requestCode      FeliCa Request-Code.
 * \param[in]       timeslotNr       FeliCa Timeslot-Number.
 * \param[out]      nrAvailableCards Number of available FeliCa cards.
 * \param[out]      cardData         Pointer to pointer to FeliCa card data structure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_FeliCa_Polling (ptx_FeliCa_t *feliCa,
                                 uint16_t systemCode,
                                 ptx_FeliCa_RequestCode_t requestCode,
                                 ptx_FeliCa_TimeslotNr_t timeslotNr,
                                 uint8_t *nrAvailableCards,
                                 ptx_FeliCa_CardData_t **cardData);

/**
 * \brief Returns the internally stored information of all detected FeliCa cards.
 *
 * \note This API-call is optional and can be used to retrieve further details of a card like manufacturer data (ID, PM, etc.).
 *
 * \param[in]       feliCa              Pointer to an existing instance of the FeliCa component.
 * \param[out]      nrAvailableCards    Number of available FeliCa cards.
 * \param[out]      cardData            Pointer to pointer to FeliCa card data structure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_FeliCa_GetCardParameters (ptx_FeliCa_t *feliCa, uint8_t *nrAvailableCards, ptx_FeliCa_CardData_t **cardData);

/**
 * \brief Selects the currently active FeliCa card.
 *
 * \note This API-call is optional. If not used, the first card found (index = 0) will be used by default.
 *
 * \param[in]       feliCa              Pointer to an existing instance of the FeliCa component.
 * \param[in]       selectedCardIndex   Index/Number of FeliCa card to be used for further commands (default = 0).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_FeliCa_SelectActiveCard (ptx_FeliCa_t *feliCa, uint8_t selectedCardIndex);

/**
 * \brief Performs a FeliCa "Request Response"-command.
 *
 * \param[in]       feliCa              Pointer to an existing instance of the FeliCa component.
 * \param[out]      currentMode         The current FeliCa card mode.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_FeliCa_RequestResponse (ptx_FeliCa_t *feliCa, uint8_t *currentMode);

/**
 * \brief Performs a FeliCa "Request Service"-command.
 *
 * \param[in]      feliCa            Pointer to an existing instance of the FeliCa component.
 * \param[in]      nrServices        Number of requested services/codes.
 * \param[in]      serviceList       Pointer to list of requested services/codes (Little Endian).
 * \param[out]     nrReadServices    Number of requested services/codes.
 * \param[out]     readServiceList   Pointer to list of requested services/codes (Little Endian).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_FeliCa_RequestService (ptx_FeliCa_t *feliCa,
                                        uint8_t nrServices,
                                        uint8_t *serviceList,
                                        uint8_t *nrReadServices,
                                        uint8_t *readServiceList);

/**
 * \brief Performs a FeliCa "Request Read"-command (without Encryption).
 *
 * \param[in]      feliCa                   Pointer to an existing instance of the FeliCa component.
 * \param[in]      nrServices               Number of requested services/codes.
 * \param[in]      serviceList              Pointer to list of requested services/codes (Little Endian).
 * \param[in]      nrBlocksToRead           Number of blocks to read.
 * \param[in]      blockListToRead          Pointer to list of block(-numbers) to read.
 * \param[in]      blockListToReadLength    Size / Length of list of block(-numbers).
 * \param[out]     nrBlocksRead             Pointer to number of blocks actually read.
 * \param[out]     blockDataRead            Pointer to buffer containing the read block data.
 * \param[out]     statusFlags              Optional: FeliCa Status-Flags in case of a PROTOCOL-error.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_FeliCa_Read (ptx_FeliCa_t *feliCa,
                              uint8_t nrServices,
                              uint8_t *serviceList,
                              uint8_t nrBlocksToRead,
                              uint8_t *blockListToRead,
                              uint8_t blockListToReadLength,
                              uint8_t *nrBlocksRead,
                              uint8_t *blockDataRead,
                              uint16_t *statusFlags);

/**
 * \brief Performs a FeliCa "Request Write"-command (without Encryption).
 *
 * \param[in]      feliCa                   Pointer to an existing instance of the FeliCa component.
 * \param[in]      nrServices               Number of requested services/codes.
 * \param[in]      serviceList              Pointer to list of requested services/codes (Little Endian).
 * \param[in]      nrBlocksToWrite          Number of blocks to write.
 * \param[in]      blockListToWrite         Pointer to list of block(-numbers).
 * \param[in]      blockListToWriteLength   Size / Length of list of block(-numbers).
 * \param[in]      blockData                Pointer to buffer containing the block data to write.
 * \param[out]     statusFlags              Optional: FeliCa Status-Flags in case of a PROTOCOL-error.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_FeliCa_Write (ptx_FeliCa_t *feliCa,
                               uint8_t nrServices,
                               uint8_t *serviceList,
                               uint8_t nrBlocksToWrite,
                               uint8_t *blockListToWrite,
                               uint8_t blockListToWriteLength,
                               uint8_t *blockData,
                               uint16_t *statusFlags);
#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
