/**
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
    Module      : Transparent Mode (TM)
    File        : ptx_TransparentMode.h

    Description : Transparent Mode (TM) implements low-level NFC access to the PTX2xxR/W hardware. This allows an
                  application to implement all sorts of low-level NFC frames in Reader mode to support custom
                  implementations or to implement proprietary protocols which are not supported by default in the RUL Stack.
*/

/**
 * \addtogroup TRANSPARENT_MODE
 *
 * @{
 */

#if !defined(PTX_TRANSPARENT_MODE_H)
#define PTX_TRANSPARENT_MODE_H

/**
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Components.h"
#include "ptx_RUL.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name Transparent-Mode RF-Flags.
 *
 * @{
 */
#define PTX_TRANSPARENT_MODE_FLAGS_TX_PARITY                    (uint8_t)0x01            /**< Enable Tx-Parity (Type-A only). */
#define PTX_TRANSPARENT_MODE_FLAGS_RX_PARITY                    (uint8_t)0x02            /**< Enable Rx-Parity (Type-A only). */
#define PTX_TRANSPARENT_MODE_FLAGS_TX_CRC                       (uint8_t)0x04            /**< Enable Tx-CRC (all Technology Types). */
#define PTX_TRANSPARENT_MODE_FLAGS_RX_CRC                       (uint8_t)0x08            /**< Enable Rx-CRC (all Technology Types). */
#define PTX_TRANSPARENT_MODE_FLAGS_RX_SOF_ONLY                  (uint8_t)0x10            /**< Enable SoF-Reception only (Start-of-Frame) - Only valid for NFCV reader mode. */
#define PTX_TRANSPARENT_MODE_FLAGS_SKIP_1ST_BYTE_CRC            (uint8_t)0x20            /**< If enabled, 1st Byte in Tx- and Rx-direction is excluded from automatic CRC-calculation - Type-V.  */
/** @} */

/**
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Transparent-Mode States.
 */
typedef enum ptx_TransparentMode_State
{
    ptx_TransparentMode_State_RF_On,
    ptx_TransparentMode_State_RF_Off,
    ptx_TransparentMode_State_Lpcd_Poll,

} ptx_TransparentMode_State_t;

/**
 * \brief Transparent-Mode LPCD state, whether a card was detected or not.
 */
typedef enum ptx_TransparentMode_Lpcd_State
{
    ptx_TransparentMode_Lpcd_State_Not_Detected,
    ptx_TransparentMode_Lpcd_State_Detected,

} ptx_TransparentMode_Lpcd_State_t;



/**
 * \brief Transparent-Mode supported RF-Technologies.
 */
typedef enum ptx_TransparentMode_RF_Tech
{
    ptx_TransparentMode_RF_Tech_A      = 0x01,
    ptx_TransparentMode_RF_Tech_B      = 0x02,
    ptx_TransparentMode_RF_Tech_F      = 0x03,
    ptx_TransparentMode_RF_Tech_V      = 0x04,
    ptx_TransparentMode_RF_Tech_BPrime = 0x05

} ptx_TransparentMode_RF_Tech_t;

/**
 * \brief Transparent-Mode supported RF-Bitrates for Type A, B, F and V.
 */
typedef enum ptx_TransparentMode_RF_Bitrate
{
    ptx_TransparentMode_RF_Bitrate_106 = 0x00,
    ptx_TransparentMode_RF_Bitrate_212 = 0x01,
    ptx_TransparentMode_RF_Bitrate_424 = 0x02,
    ptx_TransparentMode_RF_Bitrate_848 = 0x03,
    ptx_TransparentMode_RF_Bitrate_26  = 0x06,
    ptx_TransparentMode_RF_Bitrate_53  = 0x07

} ptx_TransparentMode_RF_Bitrate_t;

/**
 * \brief Transparent-Mode Initialization RF Parameters.
 */
typedef struct ptx_TransparentMode_RFParams
{
    ptx_TransparentMode_RF_Tech_t       Tech;                   /**< RF-Technology. */
    ptx_TransparentMode_RF_Bitrate_t    TxRate;                 /**< Tx-Bitrate. */
    ptx_TransparentMode_RF_Bitrate_t    RxRate;                 /**< Rx-Bitrate. */
    uint8_t                             Flags;                  /**< CRC- and Parity-Settings (Parity-Settings only for NFC-A Tech). */
    uint8_t                             RxBitOffset;            /**< Number of Bits for the offset of the first received byte (Only for NFC-A Tech). */
    uint8_t                             ResLimit;               /**< Max. Number of responses (if set to 0, Receiver automatically enabled again until TimeoutMS expires (needed for T3T). */

} ptx_TransparentMode_RFParams_t;

/**
 * \brief Transparent-Mode Data Exchange Parameters.
 */
typedef struct ptx_TransparentMode_ExchangeParams
{
    uint8_t     *TxData;                    /**< Buffer containing the data to send. */
    uint8_t     TxLength;                   /**< Length of "TxData". */
    uint8_t     NrTxBits;                   /**< Number of Bits to be sent for last Byte (Only for NFC-A Tech). */
    uint8_t     *RxData;                    /**< Pointer to buffer where the data will be received. */
    uint32_t    RxLength;                   /**< As input, capacity of "RxData". As output, actual number of bytes written on "RxData". */
    uint8_t     NrResidualRxBits;           /**< Number of (residual/valid) Bits received in the last byte (Only for NFC-A Tech). */
    uint32_t    TimeoutMS;                  /**< Timeout given in [ms]. */

} ptx_TransparentMode_ExchangeParams_t;

/**
 * \brief Main Component Struture.
 */
typedef struct ptx_TransparentMode
{
    ptx_Comps_t                         CompId;                             /**< Component Id. */
    ptx_RUL_t                           *Rul;                               /**< Pointer to initialised RUL component. */

    ptx_TransparentMode_State_t         state;                              /**< Current State of Transparent-Mode. */
    uint8_t                             UseDefaultConfigForFieldOffState;   /**< Flag for default RF-parameter reset. 0 = Off, else = On. */
    uint8_t                             BPrimeTechnologyActive;             /**< Internal Flag indicating whether B-Prime Mode is active or not. 0 = Off, else = On. */
    uint8_t                             SoFOnlyActive;                /**< Internal Flag indicating that the SoF-only option is enabled */
    uint8_t                             SkipByteCRCActive;            /**< Internal Flag indicating that the option to exclude the 1st byte from Tx-/Rx-calculation is enabled */
    ptx_Log_t                           *Log;                               /**< Logger Component */

} ptx_TransparentMode_t;

/**
 * \brief Main Component Initialization Parameters.
 */
typedef struct ptx_TransparentMode_InitParams
{
    ptx_RUL_t               *Rul;                                /**< Pointer to initialised RUL component. */

    uint8_t                 UseDefaultConfigForFieldOffState;    /**< Flag for default RF-parameter reset. 0 = Off, else = On. */
    ptx_Log_t               *Log;                                /**< Logger Component */

} ptx_TransparentMode_InitParams_t;

/**
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialisation. Sets the state to RF Off.
 *
 * \param[in]       tmComp          Pointer to component.
 * \param[in]       initParams      Pointer to initialisation parameters.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Something went wrong in the lower layers. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_TransparentMode_Open (ptx_TransparentMode_t *tmComp, ptx_TransparentMode_InitParams_t *initParams);

/**
 * \brief Component De-Initialisation. If not in RF Off State, this function automatically switches to it before de-initializing the component.
 *
 * \param[in]       tmComp          Pointer to component.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Something went wrong in the lower layers. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_TransparentMode_Close (ptx_TransparentMode_t *tmComp);

/**
 * \brief Internal reset of component state.
 *
 * \param[in]       tmComp          Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_TransparentMode_ResetState (ptx_TransparentMode_t *tmComp);

/**
 * \brief Configures the HW using the provided RF-Parameters (used for all following RF-Exchanges, can be overwritten).
 *        Only possible if Transparent-Mode is in RF On state.
 *
 * \param[in]   tmComp                  Pointer to an existing instance of the Transparent-Mode component.
 * \param[in]   rfParams                Pointer to RF-parameters.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES  Error during loading of the rf parameters.
 * \return Others                                   Something went wrong in the lower layers. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_TransparentMode_SetRFParameters (ptx_TransparentMode_t *tmComp, ptx_TransparentMode_RFParams_t *rfParams);

/**
 * \brief Sets the State of Transparent Mode.
 *
 * \param[in]       tmComp              Pointer to an existing instance of the Transparent-Mode component.
 * \param[in]       state               State which will be set in Transparent Mode. ptx_TransparentMode_State_RF_On
 *                                      activates the RF field and moves to POLL state. ptx_TransparentMode_State_RF_Off
 *                                      deactivates the field as well as RUL and moves to IDLE state. ptx_TransparentMode_State_Lpcd_Poll
 *                                      activates the Low Power Card Detection, moves to POLL state and waits for a card in the RF field.
 *                                      Once a card was detected it moves to IDLE state.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Something went wrong in the lower layers. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_TransparentMode_SetState (ptx_TransparentMode_t *tmComp, ptx_TransparentMode_State_t state);

/**
 * \brief Checks whether a card was detected or not. If a card was detected, it switches Transparent Mode
 *        automatically to ptx_TransparentMode_State_RF_On state. This function only works if
 *        Transparent Mode is in state ptx_TransparentMode_State_Lpcd_Poll. Be aware that the LPCD measurement
 *        detects impedance changes in the RF field and not directly cards.
 *
 * \param[in]       tmComp              Pointer to an existing instance of the Transparent-Mode component.
 * \param[out]      cardDetected        Give information whether a card was detected or not.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Something went wrong in the lower layers. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_TransparentMode_CheckLpcd (ptx_TransparentMode_t *tmComp, ptx_TransparentMode_Lpcd_State_t *cardDetected);


/**
 * \brief Performs a RF data exchange. Only possible if Transparent-Mode is in RF On state.
 *
 *
 * \param[in]       tmComp              Pointer to an existing instance of the Transparent-Mode component.
 * \param[in]       rfParams            Pointer to RF-parameters (optional, otherwise parameters from \ref ptx_TransparentMode_SetRFParameters are used).
 * \param[in]       exchangeParams      Pointer to the Exchange-Parameters.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES  Error during loading of the RF-parameters.
 * \return Others                                   Something went wrong in the lower layers. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_TransparentMode_Exchange (ptx_TransparentMode_t *tmComp,
                                           ptx_TransparentMode_RFParams_t *rfParams,
                                           ptx_TransparentMode_ExchangeParams_t *exchangeParams);

#if defined(__cplusplus)
}
#endif

#endif /** Guard */

/** @} */
