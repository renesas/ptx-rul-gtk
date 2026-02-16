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
    Module      : Point of Sale (POS) Device Test Environment (DTE) Add-on Application Interface (API)
    File        : ptx_POS_DTE.h

    Description : Implementation of POS DTE.
                  Contains the type and definition of the functions involved in POS DTE procedures.

*/

/**
 * \addtogroup POS_DTE
 * @{
 */

#if !defined(PTX_POSDTE_H)
#define PTX_POSDTE_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>

#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_RUL.h"
#include "ptx_Log.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \brief Defines for various delay/wait times.
 */
#define PTX_POS_DTE_RF_DETECTION_WAIT_TIME_MS           (uint32_t)5u      /**< Artificial delay between checking for updated RF-Discovery state to offload CPU. */
#define PTX_POS_DTE_CARD_REMOVAL_WAIT_TIME_MS           (uint32_t)10u     /**< Artificial delay between checking the state of the EMV Card Removal procedure. */
#define PTX_POS_DTE_EXCHANGE_PPSE_WAIT_TIME_MS          (uint32_t)10u     /**< Artificial delay before sending PPSE-Select APDU after ISO-DEP activation. */
#define PTX_POS_DTE_EXCHANGE_APDU_WAIT_TIME_MS          (uint32_t)10u     /**< Artificial delay before sending next C-APDU after ISO-DEP activation. */
#define PTX_POS_DTE_TRANSAC_CARRIER_RESET_WAIT_MS       (uint32_t)5u      /**< Giard time after RF-Field/-Carrier was turned on to send the next data packet via RF. */
#define PTX_POS_DTE_LOOPBACK_POWER_OFF_RESET_WAIT_MS    (uint32_t)15u     /**< Min. Reset time for POWER-OFF sequence during EMV Loopback. */

/**
 * \brief Default timeout-value for activation and standard ISO-DEP data exchanges (large value because managed autonomously by FW)
 */
#define PTX_POS_DTE_DEFAULT_ACTIVATION_TIMEOUT_MS       (uint32_t)100u    /**< Default timeout used for low-level Type-A and -B commands (e.g.during activation) */
#define PTX_POS_DTE_DEFAULT_PROTOCOL_TIMEOUT_MS         (uint32_t)50000u  /**< Default timeout for standard protocol operation */

/**
 * \brief Default timeout-values for TRANSAC_A and _B.
 */
#define PTX_POS_DTE_MIN_TIME_MS_TRANSAC_CMD             (uint32_t)1u      /**< Min. Waiting Time [ms] between TRANSAC-commands (typicall 2 ms according to. EMV but includes processing time here)*/
#define PTX_POS_DTE_MIN_TIME_MS_TRANSAC_END             (uint32_t)100u    /**< Min. Waiting Time [ms] after TRANSAC-sequence completed */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief EMV TRANSAC_A application States
 */
typedef enum ptx_POS_DTE_TransacA_State
{
    ptx_POS_DTE_TransacA_Init,
    ptx_POS_DTE_TransacA_CMD_WUPA_1,
    ptx_POS_DTE_TransacA_CMD_HLTA,
    ptx_POS_DTE_TransacA_CMD_WUPB,
    ptx_POS_DTE_TransacA_CMD_WUPA_2,
    ptx_POS_DTE_TransacA_CMD_ANTICOLLISION,
    ptx_POS_DTE_TransacA_CMD_SELECT,
    ptx_POS_DTE_TransacA_CMD_RATS,
    ptx_POS_DTE_TransacA_CMD_IBLOCK_1,
    ptx_POS_DTE_TransacA_CMD_IBLOCK_2,
    ptx_POS_DTE_TransacA_END

} ptx_POS_DTE_TransacA_State_t;

/**
 * \brief EMV TRANSAC_B application States
 */
typedef enum ptx_POS_DTE_TransacB_State
{
    ptx_POS_DTE_TransacB_Init,
    ptx_POS_DTE_TransacB_CMD_WUPB_1,
    ptx_POS_DTE_TransacB_CMD_WUPA,
    ptx_POS_DTE_TransacB_CMD_WUPB_2,
    ptx_POS_DTE_TransacB_CMD_ATTRIB,
    ptx_POS_DTE_TransacB_CMD_IBLOCK_1,
    ptx_POS_DTE_TransacB_CMD_IBLOCK_2,
    ptx_POS_DTE_TransacB_END

} ptx_POS_DTE_TransacB_State_t;

/**
 * \brief EMV Loopback application States
 */
typedef enum ptx_POS_DTE_LoopbackState
{
    ptx_POS_DTE_State_StartPolling,
    ptx_POS_DTE_State_WaitForActivation,
    ptx_POS_DTE_State_ExchangePPSE,
    ptx_POS_DTE_State_ExchangeAPDU,
    ptx_POS_DTE_State_StartCardRemoval,
    ptx_POS_DTE_State_WaitCardRemoval,
    ptx_POS_DTE_State_RestartPolling,
    ptx_POS_DTE_State_DeactivateReader,
    ptx_POS_DTE_State_SystemError,
    ptx_POS_DTE_State_Exit

} ptx_POS_DTE_LoopbackState_t;

/**
 * \brief Supported RF-Technologies to be used for EMV Loopback application
 */
typedef enum ptx_POS_DTE_TargetRFTech
{
    ptx_POS_DTE_TargetRFTech_A,
    ptx_POS_DTE_TargetRFTech_B,
    ptx_POS_DTE_TargetRFTech_A_B,

} ptx_POS_DTE_TargetRFTech_t;

/**
 * \brief RF-Field/-Carrier Request i.e. turn on or off
 */
typedef enum ptx_POS_DTE_CarrierState
{
    ptx_POS_DTE_Carrier_OFF = 0,
    ptx_POS_DTE_Carrier_ON  = 1

} ptx_POS_DTE_CarrierState_t;

/**
 * \brief Generic callback function.
 * *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef void(*ptx_POS_DTE_Callback_Generic_t)();

/**
 * \brief Callback function executed when the APDU is received.
 *
 * \param[in]   data         Pointer to the received APDU data.
 * \param[in]   dataLength   Length of the received APDU data.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef void(*ptx_POS_DTE_Callback_APDU_t)(uint8_t* data, uint32_t dataLength);

/**
 * \brief POS-DTE Initialization Parameters
 */
typedef struct ptx_POS_DTE_InitParams
{
    ptx_RUL_t                                   *Rul;                   /**< Pointer to the RUL component. */
    uint32_t                                    ResetDelay;             /**< Reset delay in ms used for ptx_POS_DTE_Reset. */
    uint8_t                                     IgnoreRFError;          /**< Flag to suppress possible RF-Errors during loopback application (not an issue by default - depending on scnearion). */
    uint8_t                                     EnableDDPC;             /**< Flag to enable DDPC when using this add-on api. */
    uint8_t                                     IopLoopback;            /**< Enable/Disable interoperability loopback application. */
    ptx_POS_DTE_Callback_Generic_t              CbSuccess;              /**< Success callback for interoperability loopback function. */
    ptx_POS_DTE_Callback_Generic_t              CbError;                /**< Error callback for interoperability loopback function. */
    ptx_POS_DTE_Callback_APDU_t                 CbApdu;                 /**< APDU callback. */
} ptx_POS_DTE_InitParams_t;

/**
 * \brief Main Component Struture
 */
typedef struct ptx_POS_DTE
{
    /* Components */
    ptx_Comps_t                                 CompId;                 /**< Component Id */

    ptx_RUL_t                                   *Rul;                   /**< RUL Component */
    ptx_Log_t                                   *Logger;                /**< Logger Component */

    ptx_RUL_DiscoverConfig_t                    DiscoveryCfg;           /**< Pointer to the RF-discovery configuration. */

    uint32_t                                    ResetDelay;             /**< Reset delay in ms */
    uint8_t                                     IgnoreRFError;          /**< Flag to suppress possible RF-Errors during loopback application (not an issue by default - depending on scenario). */
    uint8_t                                     IopLoopback;            /**< Enable/Disable interoperability loopback. */
    uint8_t                                     CardID[4];              /**< Card-ID internally used for Type-B ("PUPI"). */
    ptx_POS_DTE_Callback_Generic_t              CbSuccess;              /**< Success callback for interoperability loopback function. */
    ptx_POS_DTE_Callback_Generic_t              CbError;                /**< Error callback for interoperability loopback function. */
    ptx_POS_DTE_Callback_APDU_t                 CbApdu;                 /**< APDU callback. */

    ptx_POS_DTE_CarrierState_t                  CarrierState;           /**< Store the carrier state. Used because RF_DEACTIVATE cmd fails if carrier is not turned ON */
    ptx_POS_DTE_LoopbackState_t                 LoopbackState;          /**< State of the loopback state machine. */
    ptx_POS_DTE_TransacA_State_t                TransacAState;          /**< State of the TransacA state machine. */
    ptx_POS_DTE_TransacB_State_t                TransacBState;          /**< State of the TransacB state machine. */
    uint8_t                                     DisableFwt;             /**< Frame Wait Time disabling needed for TRANSAC A and B implementation  */
    uint8_t                                     EnableDDPC;             /**< Flag to enable DDPC when using this add-on api. */

} ptx_POS_DTE_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initializes POS_DTE Component.
 *
 * \param[in]   ctx         Pointer to POS_DTE context.
 * \param[in]   initParams  Pointer to POS_DTE_InitParams.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_POS_DTE_Init(ptx_POS_DTE_t* ctx, ptx_POS_DTE_InitParams_t* initParams);

/**
 * \brief De-initializes POS_DTE Component.
 *
 * \param[in]   ctx         Pointer to POS_DTE context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_POS_DTE_DeInit(ptx_POS_DTE_t* ctx);

/**
 * \brief Internal reset of component state.
 *
 * \param[in]   ctx         Pointer to POS_DTE context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_POS_DTE_ResetState(ptx_POS_DTE_t* ctx);

/**
 * \brief Turns RF-field/-carrier on or off.
 *
 * The PCD starts to emit an unmodulated carrier.
 *
 * \param[in]   ctx         Pointer to POS_DTE context.
 * \param[in]   carrierReq  Requested Carrier setting.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_Carrier(ptx_POS_DTE_t* ctx, ptx_POS_DTE_CarrierState_t carrierReq);

/**
 * \brief Activates the RF-Polling sequence according to EMVCo.
 *
 * Activate the polling for Type-A and Type-B cards.
 *
 * \param[in]   ctx         Pointer to POS_DTE context.
 * \param[in]   rfTech      Target RF-Technology to use for Polling.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_Polling(ptx_POS_DTE_t* ctx, ptx_POS_DTE_TargetRFTech_t rfTech);

/**
 * \brief Switches RF-field/-carrier off and on again.
 *
 * Deactivates the reader and turns on the field again. The reset time is handled in hardware.
 *
 * \param[in]   ctx         Pointer to POS_DTE context.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_Reset(ptx_POS_DTE_t* ctx);

/**
 * \brief Deactivates the reader
 *
 * \param[in]   ctx         Pointer to POS_DTE context.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_PowerOff(ptx_POS_DTE_t* ctx);

/**
 * \brief Sends Type-A WUPA command.
 *
 * \param[in]   ctx         Pointer to POS_DTE context.
 * \param[out]  rx          Pointer to buffer where response can be stored (optional, can be set to NULL, if used - shall at least have a size of 128 bytes).
 * \param[out]  rxLen       Pointer to variable where length of response can be stored (optional, can be set to NULL).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_WupA(ptx_POS_DTE_t* ctx, uint8_t *rx, uint32_t *rxLen);

/**
 * \brief Sends Type-A RATS command with prior AntiCollision-sequence.
 *
 * Sends a RATS command and does all the anti collision procedures until the command is actually sent.
 *
 * If call succeeds and if provided, the content of "rx" will look as follows:
 * - 2 Byte(s) ATQA/SENS_RES
 * - 1 Byte(s) UID/NFCID1 Length m
 * - m Byte(s) UID/NFCID1
 * - 1 Byte(s) SAK/SEL_RES
 * - 1 Byte(s) ATS/RATS_RES Length n
 * - n Byte(s) ATS/RATS_RES
 *
 * \param[in]   ctx         Pointer to POS_DTE context.
 * \param[out]  rx          Pointer to buffer where response can be stored (optional, can be set to NULL, if used - shall at least have a size of 128 bytes).
 * \param[out]  rxLen       Pointer to variable where length of response can be stored (optional, can be set to NULL).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_RATS(ptx_POS_DTE_t *ctx, uint8_t *rx, uint32_t *rxLen);

/**
 * \brief Sends Type-B WUPB command
 *
 * If call succeeds and if provided, the content of "rx" will look as follows:
 * - 12 or 13 Byte(s) ATQB/SENSB_RES
 *
 * \param[in]   ctx         Pointer to POS_DTE context.
 * \param[out]  rx          Pointer to buffer where response can be stored (optional, can be set to NULL, if used - shall at least have a size of 128 bytes).
 * \param[out]  rxLen       Pointer to variable where length of response can be stored (optional, can be set to NULL).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_WupB(ptx_POS_DTE_t *ctx, uint8_t *rx, uint32_t *rxLen);

/**
 * \brief Sends Type-B ATTRIB command
 *
 * Sends a ATTRIB command and does all the anti collision procedures until the command is actually sent.
 *
 * If call succeeds and if provided, the content of "rx" will look as follows:
 * - 1 Byte(s) ATQB/SENSB_RES Length m
 * - m Byte(s) ATQB/SENSB_RES
 * - 1 Byte(s) ATTRIB_RES Length n
 * - n Byte(s) ATTRIB_RES
 *
 * \param[in]   ctx          Pointer to POS_DTE context.
 * \param[out]  rx           Pointer to buffer where response can be stored (optional, can be set to NULL, if used - shall at least have a size of 128 bytes).
 * \param[out]  rxLen        Pointer to variable where length of response can be stored (optional, can be set to NULL).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_ATTRIB(ptx_POS_DTE_t *ctx, uint8_t *rx, uint32_t *rxLen);

/**
 * \brief Starts / Resumes the TRANSA_A-sequence application.
 *
 * Sends a pre-defined Type-A set of commands, using fixed intervals between the commands.
 * The PCD does not care about any answer that the PICC may provide.
 *
 * Note: The function is based on a state-machine implementation and is supposed to be called in a loop at Application-level.
 *       The Application can choose whether the TRANSA_A-sequence application shall be executed endlessly or just for n-times.
 *       When the Application has to decided to stop the TRANSA_A-sequence application, it must call \ref ptx_POS_DTE_Transac_Stop to
 *       shut-down the internal states of the HW and SW gracefully.
 *
 * \param[in]   ctx     Pointer to POS_DTE context.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_TransacA(ptx_POS_DTE_t *ctx);

/**
 * \brief Provides the current internal state of the TRANSA_A-sequence application (optional function call).
 *
 * \param[in]   ctx     Pointer to POS_DTE context.
 * \param[out]  state   Pointer to to structure where the state shall be stored..
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_GetTransacAState(ptx_POS_DTE_t *ctx, ptx_POS_DTE_TransacA_State_t *state);

/**
 * \brief Starts / Resumes the TRANSA_B-sequence application.
 *
 * Sends a pre-defined Type-B set of commands, using fixed intervals between the commands.
 * The PCD does not care about any answer that the PICC may provide.
 *
 * Note: The function is based on a state-machine implementation and is supposed to be called in a loop at Application-level.
 *       The Application can choose whether the TRANSA_B-sequence application shall be executed endlessly or just for n-times.
 *       When the Application has to decided to stop the TRANSA_B-sequence application, it must call \ref ptx_POS_DTE_Transac_Stop to
 *       shut-down the internal states of the HW and SW gracefully.
 *
 * \param[in]   ctx     Pointer to POS_DTE context.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_TransacB(ptx_POS_DTE_t *ctx);

/**
 * \brief Provides the current internal state of the TRANSA_B-sequence application (optional function call).
 *
 * \param[in]   ctx     Pointer to POS_DTE context.
 * \param[out]  state   Pointer to to structure where the state shall be stored..
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_GetTransacBState(ptx_POS_DTE_t *ctx, ptx_POS_DTE_TransacB_State_t *state);

/**
 * \brief Stops an going TRANSA_A or _B sequence.
 *
 * \param[in]   ctx     Pointer to POS_DTE context.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_Transac_Stop(ptx_POS_DTE_t *ctx);

/**
 * \brief Starts / Resumes the EMV Loopback application.
 *
 * Note: The function is based on a state-machine implementation and is supposed to be called in a loop at Appliation-level.
 *       The Application can choose whether the Loopback application shall be executed endlessly or just for n-times.
 *       When the Application has to decided to stop the Loopback application, it must call \ref ptx_POS_DTE_Loopback_Stop to
 *       shut-down the internal states of the HW and SW gracefully.
 *
 * \param[in]   ctx         Pointer to POS_DTE context.
 * \param[in]   rfTech      Target RF-Technology to use for Polling.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_POS_DTE_Loopback(ptx_POS_DTE_t *ctx, ptx_POS_DTE_TargetRFTech_t rfTech);

/**
 * \brief Provides the current internal state of the EMB Loopback application (optional functin call).
 *
 * \param[in]   ctx     Pointer to POS_DTE context.
 * \param[out]  state   Pointer to to structure where the state shall be stored..
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_POS_DTE_GetLoopbackState(ptx_POS_DTE_t *ctx, ptx_POS_DTE_LoopbackState_t *state);

/**
 * \brief Stops the EMV Loopback application.
 *
 * \param[in]   ctx     Pointer to POS_DTE context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_POS_DTE_Loopback_Stop(ptx_POS_DTE_t *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

