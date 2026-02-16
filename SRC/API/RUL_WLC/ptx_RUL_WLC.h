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
    Module      : RUL Wireless Charging (WLC)
    File        : ptx_RUL_WLC.h

    Description : RUL Wireless Charging (WLC) implements NFC WLC capabilities with the PTX2xxW chip. This module grants
                  NFC Forum compliant WLC-P functionality with compatibility to all compliant WLC-L hardware. Additional
                  it is possible to use the API to create and define custom WLC protocols.
*/

/**
 * \addtogroup RUL_WLC
 * @{
 */

#if !defined(PTX_RULWLC_H)
#define PTX_RULWLC_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>

#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_NSC_Wlc.h"
#include "ptx_NSC_CmdProcessor.h"
#include "ptx_NDEFMessage.h"
#include "ptx_NDEFRecord.h"
#include "ptx_NDEFRecord_DI.h"
#include "ptx_NDEFRecord_WLCCAP.h"
#include "ptx_NDEFRecord_WLCINF.h"
#include "ptx_NDEFRecord_WLCCTL.h"
#include "ptx_NDEFRecord_WLCSTAI.h"
#include "ptx_NSC_CmdProcessor.h"

#include "ptx_HAL.h"
#include "ptx_RUL.h"
#include "ptx_NDEF.h"


#if defined(__cplusplus)
extern "C" {
#endif


/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name Generic WLC-Poller defines.
 * @{
 */
#define REN_TECH_CORP_MANUFACTURER_CODE             (0x20)  /**< Renesas Technology Corp. manufacturer code (= first byte of UID) */
#define REN_ELEC_CORP_MANUFACTURER_CODE             (0x41)  /**< Renesas Electronics Corp. manufacturer code (= first byte of UID) */
#define PTX_WLC_MAX_POLLER_NDEF_RECORD_BUFFER_SIZE  (4u)    /**< Specifies the maximum amount of supported NDEF records. */
#define PTX_WLC_MAX_NDEF_MESSAGE_BUFFER_SIZE        (0x100) /**< Specifies the NDEF Message buffer size, used to process NDEF Messages. */
#define PTX_WLC_TX_MULTIPLIER_MAX                   (177u)  /**< Maximum wave amplitude multiplier. */
#define PTX_WLC_PERCENT_FRACTION                    (100u)  /**< Percent fraction. */
/** @} */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Type of the connected WLC-Listener Device.
 */
typedef enum ptx_RUL_WLC_ListenerType
{
    ListenerType_Generic,       /**< Generic Listener Device, which is only compatible to NFC Forum WLC compliant functionality. */
    ListenerType_Proprietary    /**< Proprietary Renesas Listener Device, which is compatible to all NFC Forum WLC compliant and proprietary Renesas functionality. */
} ptx_RUL_WLC_ListenerType_t;

/**
 * \brief WLC Application Notifications.
 */
typedef enum ptx_RUL_WLC_Notification
{
    Notification_PollingStarted,                    /**< Application Notification. Started Discovery Loop for Listener. Payload: None. */
    Notification_ListenerDetected,                  /**< Application Notification. Listener entered RF-field and was discovered. Payload: Number of detected Listeners (type: uint32_t ). */
    Notification_ListenerActivated,                 /**< Application Notification. Discovered Listener was activated. Payload: Parameter of the activated Listener (type: \ref ptx_RUL_CardParams_t ). */
    Notification_CapabilityRecordReceived,          /**< Application Notification. Capability record received from Listener. Payload: Received Capability Record (type: \ref ptx_NDEFRecord_WLCCAP_t ). */
    Notification_StatusInfoRecordReceived,          /**< Application Notification. Status & Information record received from Listener. Payload: Received Status & Information Record (type: \ref ptx_NDEFRecord_WLCSTAI_t ). */
    Notification_DeviceInfoRecordReceived,          /**< Application Notification. Device Information record received from Listener. Payload: Received Device Information Record (type: \ref ptx_NDEFRecord_DI_t ). */
    Notification_InformationRecordSent,             /**< Application Notification. Information record sent to Listener. Payload: Sent Information Record (type: \ref ptx_NDEFRecord_WLCINF_t ). */
    Notification_ControlRecordReceived,             /**< Application Notification. Control record received from Listener. Payload: Received Control Record (type: \ref ptx_NDEFRecord_WLCCTL_t ). */
    Notification_StaticWptCycleStarted,             /**< Application Notification. Static WPT cycle started. Payload: Applied WPT Options (type: \ref ptx_NSC_WLC_WptOptions_t ). */
    Notification_NegotiatedWptCycleStarted,         /**< Application Notification. Negotiated WPT cycle started. Payload: Applied WPT Options (type: \ref ptx_NSC_WLC_WptOptions_t ). */
    Notification_WptCycleFinished,                  /**< Application Notification. WPT cycle Finished. Payload: WPT State the cycle finished with (type: \ref ptx_RUL_WLC_WptState_t ). */
    Notification_SleepStarted,                      /**< Application Notification. Poller started sleep mode. Payload: Total Duration of the Sleep Cycle (type: uint32_t ). */
    Notification_SleepFinished,                     /**< Application Notification. Poller finished sleep mode. Payload: Remaining Duration of the Sleep Cycle (type: uint32_t ). */
    Notification_ListenerDeactivated,               /**< Application Notification. Listener deactivated. Payload: None. */
    Notification_Error_InvalidState,                /**< Application Notification. Error Notification: The WLC state machine is in a different state as which was accessed. Payload: None. */
    Notification_Error_IncompatibleTagProtocol,     /**< Application Notification. Error Notification: All discovered Listeners only have with WLC incompatible tag protocols. Payload: None. */
    Notification_Error_InvalidWlcProtocolVersion,   /**< Application Notification. Error Notification: The WLC protocol version of the activated Listener is incompatible to the one of the Poller. Payload: None. */
    Notification_Error_CapabilityRecordMissing,     /**< Application Notification. Error Notification: The Capability Record cannot be read from the activated Listener, which is therefore not capable of acting as a WLC-L device. Payload: None. */
    Notification_Error_NegoWaitMaxRetries,          /**< Application Notification. Error Notification: The maximum number of retries was reached while waiting for the Listener to be ready to perform in negotiated mode. Payload: None. */
    Notification_Error_ControlRecordMissing,        /**< Application Notification. Error Notification: The Control Record cannot be read from the WLC-L capable Listener device. Payload: None. */
    Notification_Error_ControlRecordMaxRetries,     /**< Application Notification. Error Notification: The maximum number of retries was reached while waiting for a valid Control Record. Payload: None. */
    Notification_Error_InvalidMode,                 /**< Application Notification. Error Notification: The Capability Record contains an invalid WLC Mode parameter. Payload: None. */
    Notification_Error_ProtocolError,               /**< Application Notification. Error Notification: The Control Record error flag indicates a protocol error. Payload: None. */
    Notification_Error_InvalidHoldoffDuration,      /**< Application Notification. Error Notification: The hold-off duration contains an invalid value (must be <= 25% of WPT duration). Payload: None. */
    Notification_Error_WptIcd,                      /**< Application Notification. Error Notification: The WPT cycle stopped due to a Impedance Change Detection (ICD). Payload: None. */
    Notification_Error_WptTimeout,                  /**< Application Notification. Error Notification: The WPT cycle stopped due to a timeout without a response of the chip. Payload: Error Status Code (type: \ref ptx_Status_t ). */
    Notification_Error_SystemStatusInfo,            /**< Application Notification. Error Notification: Internal None-WLC System Information Status Error happened. Payload: System Status Info (type: \ref ptx_RUL_StatusInfoSystem_t ).  */
    Notification_Error_Internal,                    /**< Application Notification. Error Notification: Internal None-WLC specific Error happened. Payload: Error Status Code (type: \ref ptx_Status_t ).  */
} ptx_RUL_WLC_Notification_t;

/**
 * \brief WPT Charging State
 */
typedef enum ptx_RUL_WLC_WptState
{
    WptState_Idle,      /**< WLC WPT state: No Wireless Power Transfer ongoing and no errors. */
    WptState_Ongoing,   /**< WLC WPT state: Wireless Power Transfer ongoing. */
    WptState_ICD,       /**< WLC WPT state: Wireless Power Transfer cancelled due to Impedance Change Detection (ICD). */
    WptState_Error,     /**< WLC WPT state: Error. */
    WptState_Max        /**< WLC WPT state: Max. */
} ptx_RUL_WLC_WptState_t;

/**
 * \brief WPT Post Power Level Option
 */
typedef enum ptx_RUL_WLC_WptPostPwrOption
{
    WptPostPwrOption_A,      /**< Option A. */
    WptPostPwrOption_B,      /**< Option B. */
} ptx_RUL_WLC_WptPostPwrOption_t;

/**
 * \brief Callback function for reporting WLC events/notification to the main application.
 */
typedef void (*ptx_RUL_WLC_NtfCb_t) (void *ctx, ptx_RUL_WLC_Notification_t wlcEvent, void *data, uint32_t dataLen);

/**
 * \brief WLC states
 */
typedef enum ptx_RUL_WLC_State
{
    State_Uninitialized,                /**< WLC Poller is not yet initialized. */
    State_NFCLE_StartPollListener,      /**< Initiate/Start Discovery loop for NFC Listeners. */
    State_NFCLE_PollListener,           /**< Non-blocking wait for LPCD & Discovery loop for NFC Listener devices. */
    State_NFCLE_ActivateListener,       /**< This state activates one of multiple Listeners found during discovery if there are more than one. */
    State_WCCA_ReadCapabilityRecord,    /**< Once a single Listener is activated, this state handles the read of the capability record. */
    State_WCCA_StartPollListenerReady,  /**< Initiate polling until the Listener is ready to accept a charging cycle in negotiated mode. */
    State_WCCA_PollListenerReady,       /**< Non-blocking wait for the Listener to accept a charging cycle in negotiated mode. */
    State_WCC_WriteInfoRecord,          /**< Once the Listener is ready to accept a charging cycle in negotiated mode, this state writes the info record to the Listener. */
    State_WCC_ReadControlRecord,        /**< Read the control record state from the Listener. */
    State_WPT_StartStaticCharging,      /**< Start Static charging WPT cycle. */
    State_WPT_StartNegotiatedCharging,  /**< Start Negotiated charging WPT cycle. */
    State_WPT_PollCharging,             /**< Non-blocking wait for the WPT cycle finish. */
    State_Sleep_StartSleep,             /**< Starts Sleep mode of Poller e.g. due to battery fully charged. */
    State_Sleep_PollSleep,              /**< Non-blocking wait during sleep-duration. */
    State_Off_DeactivateListener,       /**< Deactivates the currently active Listener. */
    State_Error                         /**< Error state --> causes potential abort of state-machine loop. */
} ptx_RUL_WLC_State_t;

/**
 * \brief WLC component initialization parameters.
 */
typedef struct ptx_RUL_WLC_InitParams
{
    /** General Parameters. */
    ptx_RUL_t                              *Rul;                            /**< Pointer to RUL Core Component. */
    ptx_RUL_WLC_NtfCb_t                     NtfCb;                          /**< Application callback used for notification of asynchronous events in the WLC state machine.*/

    /** NFC LE Parameters. */
    ptx_RUL_DiscoverConfig_t                NfcLeDiscoverConfig;            /**< Discovery Configuration for all polling loops. */

    /** WLC CAP Parameters. */
    uint8_t                                 WlcCapVersionMajor;             /**< Major WLC protocol version of the Poller. */
    uint8_t                                 WlcCapVersionMinor;             /**< Minor WLC protocol version of the Poller. */

    /** WLC INFO Parameters. */
    ptx_NDEFRecord_WLCINF_PowerClass_t      WlcInfoPowerClass;              /**< Supported power class of the Poller. */
    uint8_t                                 WlcInfoWaitingTimeMarginDelay;  /**< Additional delay time of the Poller, added after finishing writing a WLC Information Record. */

    /** WLC CTRL Parameters. */
    uint8_t                                 WlcCtrlReadRetryDelay;          /**< Timeout in case the Listener is not ready for its control record to be read. */
    uint8_t                                 WlcCtrlMaxReadRetries;          /**< Maximum number of retries if the Listener is not ready for its control record to be read. */

    /** WPT Cycle Parameters. */
    uint8_t                                 WptPowerLevelMin;               /**< Minimum value used for the TxMultiplier. This setting affects the minimum output power. (Range: 0 - 177) */
    uint8_t                                 WptPowerLevelMax;               /**< Maximum value used for the TxMultiplier. This setting affects the maximum output power. (Range: 0 - 177) */
    uint8_t                                 WptPowerLevelInitial;           /**< The TxMultiplier used for the very first WPT cycle (Range: 0 - 177). Note: If configured to 0 the default polling power level (from the RF-configuration) will be used for the initial WPT cycle.  */
    uint8_t                                 WptIcdEnable;                   /**< Enable/Disable Impedance Change Detection during WPT. */
    uint8_t                                 WptIcdGuardTime;                /**< Impedance Change Detection guard time. */
    uint8_t                                 WptPostPwrLevel;                /**< The TxMultiplier applied after(!) the WPT cycle, depending on the WptPostPwrThreshold and the WptPostPwrOption. Note: If configured to 0 the default polling power level (from the RF-configuration) will be used as WptPostPwrLevel (Range: 0 - 177). */
    uint8_t                                 WptPostPwrThreshold;            /**< The TxMultiplier threshold used to decide which power level to use after the WPT cycle (Range: 0 - 177). */
    ptx_RUL_WLC_WptPostPwrOption_t          WptPostPwrOption;               /**< Poller power option used to decide which power level to use after the WPT cycle.
                                                                                 In case Option_A is selected, the Poller will keep the WPT power level also for the communication cycle, AS LONG AS the current power level is below WptPostPwrThreshold.
                                                                                 In case the WptPostPwrThreshold is exceeded, the Poller will default to WptPostPwrLevel.
                                                                                 In case Option_B is selected, the Poller will always default to WptPostPwrLevel.
                                                                                 */
    uint8_t                                 WptPostSettleTime;              /**< Additional delay time after a successful WPT cycle in ms. (Min = 300us). */

    /** WLC Sleep Parameters. */
    uint32_t                                SleepInterval;                  /**< Interval, after which the Poller runs the presence check during sleep state. */

} ptx_RUL_WLC_InitParams_t;

/**
 * \brief RUL WLC Main component.
 */
typedef struct ptx_RUL_WLC
{
    ptx_Comps_t                             CompId;                                                 /**< Component ID. */
    ptx_RUL_WLC_InitParams_t                InitParams;                                             /**< Initialisation parameters for RUL_WLC. */
    ptx_RUL_WLC_WptState_t                  WptState;                                               /**< WLC WPT cycle state. */
    ptx_RUL_WLC_State_t                     State;                                                  /**< WLC State Machine state. */
    uint8_t                                 StateMachineActive;                                     /**< Flag to activate/deactivate any external and internal WLC State Machine. */
    ptx_RUL_CardParams_t                    Card;                                                   /**< Card parameters of the connected activated Listener device. */
    uint32_t                                CardRegistryCardCnt;                                    /**< Total number of cards, within the card registry. */
    uint32_t                                CardRegistryCardId;                                     /**< ID of the currently selected card, within the card registry. */
    uint8_t                                 NdefTxBuffer[PTX_WLC_MAX_NDEF_MESSAGE_BUFFER_SIZE];     /**< 256 Byte tx buffer for building NDEF messages */
    uint8_t                                 NdefRxBuffer[PTX_WLC_MAX_NDEF_MESSAGE_BUFFER_SIZE];     /**< 256 Byte rx buffer for building NDEF messages */
    uint8_t                                 NdefWorkBuffer[PTX_WLC_MAX_NDEF_MESSAGE_BUFFER_SIZE];   /**< 256 Byte work buffer for building NDEF messages */
    uint8_t                                 NdefMsgBuffer[PTX_WLC_MAX_NDEF_MESSAGE_BUFFER_SIZE];    /**< 256 Byte message buffer for building NDEF messages */
    ptx_NDEF_t                              NdefComp;                                               /**< Context of the generic NDEF operations API. */
    ptx_NDEF_InitParams_t                   NdefInitParams;                                         /**< Initialisation parameters for the NDEF operations API. */
    ptx_NDEFRecord_WLCCAP_t                 RecordWlcCap;                                           /**< Holds the information contained within capability record provided by the Listener. */
    ptx_NDEFRecord_WLCINF_t                 RecordWlcInf;                                           /**< Holds the information contained within the information record. */
    ptx_NDEFRecord_WLCCTL_t                 RecordWlcCtl;                                           /**< Holds the information contained within the control record provided by the Listener. */
    uint8_t                                 RecordWlcCtlValid;                                      /**< Holds the information contained within the control record provided by the Listener. */
    ptx_NDEFRecord_WLCSTAI_t                RecordWlcStai;                                          /**< Holds the information contained within the status and info record provided by the Listener. */
    uint8_t                                 RecordWlcStaiAvailable;                                 /**< Stores, if the optional status and info record was received from the Listener. */
    ptx_NDEFRecord_DI_t                     RecordDevInfo;                                          /**< Holds the information provided in the device information record provided by the Listener. */
    uint8_t                                 RecordDevInfoAvailable;                                 /**< Stores, if the optional device information record was received from the Listener. */
    uint8_t                                 CntrWlcCapNwt;                                          /**< Stores the number of repetitions of CAP_WT until the Listener is ready to operate in negotiated mode. */
    uint8_t                                 WlcCtlCnt;                                              /**< Stores the previous counter from the control record to check for a new entry. */
    uint8_t                                 CntrWlcCtlNwcc;                                         /**< Stores the number of repetitions of trying to read out a valid control record. */
    uint8_t                                 TxMultiplier;                                           /**< Holds the current TxMultiplier applied to the operating volume. This value scales with the transmission
                                                                                                         power of the WPT cycles. */
    uint32_t                                SleepInterval;                                          /**< The interval of the presence check. */
    uint32_t                                SleepTotalDuration;                                     /**< Total duration of the current sleep state. */
    uint32_t                                SleepRemainingDuration;                                 /**< Remaining duration of the current sleep state. */
} ptx_RUL_WLC_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initialize the WLC (Wireless Charging) Component.
 *
 * This function has to be called before any other API functions of this Component.
 *
 * \param[in]   ctx         Pointer to the RUL WLC Component.
 * \param[in]   initParams  Pointer containing init parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_Init(ptx_RUL_WLC_t *ctx, ptx_RUL_WLC_InitParams_t *initParams);

/**
 * \brief De-Initialize the the WLC (Wireless Charging) Component.
 *
 * This function has to be called at last for this Component.
 *
 * \param[in]   ctx         Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_DeInit(ptx_RUL_WLC_t *ctx);

/**
 * \brief Retrieves the type of the Listener Device (either "Generic" or "Proprietary"). This function needs to be used
 * to verify that the Proprietary WLC API is applicable to the currently activated Listener.
 *
 * \param[in]   ctx        Pointer to the RUL WLC Component.
 * \param[out]  type       Pointer to store the information whether the tag is a Generic or Proprietary Listener.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_GetListenerType(ptx_RUL_WLC_t *ctx, ptx_RUL_WLC_ListenerType_t *type);

/**
 * \brief Starts a WPT Cycle.
 *
 * This function will start a Wireless Power Transfer (WPT) Cycle.
 *
 * \param[in]   ctx             Pointer to the RUL WLC Component.
 * \param[in]   wptOptions      Pointer to a data structure containing all related settings to the WPT.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StartPowerTransfer(ptx_RUL_WLC_t *ctx, ptx_NSC_WLC_WptOptions_t *wptOptions);

/**
 * \brief Stops an ongoing WPT.
 *
 * This function will stop an ongoing Wireless Power Transfer (WPT).
 *
 * \param[in]   ctx             Pointer to the RUL WLC Component.
 * \param[in]   postPowerLevel  Power level to proceed with, after the WPT has been stopped/cancelled.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StopPowerTransfer(ptx_RUL_WLC_t *ctx, uint8_t postPowerLevel);

/**
 * \brief Retrieve the current WPT cycle state.
 *
 * This function will retrieve the information about the current WPT cycle state \ref ptx_RUL_WLC_WptState_t.
 *
 * \param[in]   ctx         Pointer to the RUL WLC Component.
 * \param[in]   state       Pointer for storing the current WPT state information.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_GetPowerTransferState(ptx_RUL_WLC_t *ctx, ptx_RUL_WLC_WptState_t *state);

/**
 * \brief Run the internal BLOCKING NFC Forum WLC-P compliant State Machine.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.StateMachineActive is set to zero (No currently active State Machine).
 *
 * This will start the internally implemented NFC Forum WLC-P compliant state machine. Be aware, that this function
 * blocks until the state machine terminates for two different scenarios, by setting \ref ptx_RUL_WLC_t.StateMachineActive
 *  to zero.
 *
 * 1. It will terminate by itself due to an error, if the corresponding notification (see \ref Notification_Error_Internal,
 * \ref Notification_Error_WptTimeout or \ref Notification_Error_SystemStatusInfo) is not handled properly at application level.
 * This must be done by processing the error and setting the status/systemInfo in the argument of the callback to
 * PTX_STATUS_SUCCESS/ptx_RUL_StatusInfoSystem_OK. Otherwise, will the state machine terminate.
 *
 * 2. It is possible to terminate the state machine in any other notification (see \ref ptx_RUL_WLC_Notification_t). In
 * order to do this, the variable \ref ptx_RUL_WLC_t.StateMachineActive needs to be set to zero.
 *
 * \param[in]   ctx         Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_RunPollerStateMachine(ptx_RUL_WLC_t *ctx);


/*
 * ####################################################################################################################
 * FUNCTIONS FOR POLLER STATE MACHINE
 * ####################################################################################################################
 */

/**
 * \brief Starts the polling sequence for Listeners.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_NFCLE_StartPollListener.
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateNFCLE_StartPollListener( ptx_RUL_WLC_t *ctx);

/**
 * \brief Checks for detected Listeners in the RF field through LPCD/Discovery.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_NFCLE_PollListener.
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateNFCLE_PollListener( ptx_RUL_WLC_t *ctx);

/**
 * \brief In case multiple Listeners were detected/discovered, this state function activates one after another, until a
 *        WLC compatible device was found.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_NFCLE_ActivateListener.
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateNFCLE_ActivateListener( ptx_RUL_WLC_t *ctx);

/**
 * \brief Reads and parses the WLC Capability record (and any optional records) with an activated WLC compatible
 *        Listener and continues the protocol for the different charging modes.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_WCCA_ReadCapabilityRecord
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateWCCA_ReadCapabilityRecord( ptx_RUL_WLC_t *ctx);

/**
 * \brief If requested by the WLC Capability record, start waiting until the Listener is ready to proceed with the WLC
 *        protocol for negotiated mode. This step validates the Listener as fully WLC compatible.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_WCCA_StartPollListenerReady.
 *
 * \param[in]        ctx            Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateWCCA_StartPollListenerReady( ptx_RUL_WLC_t *ctx);

/**
 * \brief Checks if the requested waiting time from the Listener elapsed and if it is ready to proceed with the WLC
 *        protocol for negotiated mode.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_WCCA_PollListenerReady.
 *
 * \param[in]        ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateWCCA_PollListenerReady( ptx_RUL_WLC_t *ctx);

/**
 * \brief Writes the WLC Information record to the Listener (without any optional records) for negotiated mode.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_WCC_WriteInfoRecord
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateWCC_WriteInfoRecord( ptx_RUL_WLC_t *ctx);

/**
 * \brief Reads and parses the WLC Control record from the Listener (and any optional records) for negotiated mode.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_WCC_ReadControlRecord.
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateWCC_ReadControlRecord( ptx_RUL_WLC_t *ctx);

/**
 * \brief Starts WPT cycle in Static Charging mode.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_WPT_StartStaticCharging.
 *
 * \param[in]        ctx            Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateWPT_StartStaticCharging( ptx_RUL_WLC_t *ctx);

/**
 * \brief Starts WPT cycle in Negotiating Charging mode.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_WPT_StartNegotiatedCharging.
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateWPT_StartNegotiatedCharging( ptx_RUL_WLC_t *ctx);

/**
 * \brief Checks if the currently ongoing WPT cycle is finished.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_WPT_PollCharging.
 *
 * \param[in]        ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateWPT_PollCharging( ptx_RUL_WLC_t *ctx);

/**
 * \brief Starts the sleep mode for the Poller if requested by the Listener. In this state, the Poller deactivates its
 *        RF-field for a certain amount of time (specified by the Listener). This for example happens when Listener
 *        reports 'battery full' in the capability record.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_Sleep_StartSleep.
 *
 * \param[in]        ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateSleep_StartSleep( ptx_RUL_WLC_t *ctx);

/**
 * \brief Checks if the whole requested sleep duration is finished and if a single sleep interval elapsed. If an
 *        interval elapses, a presence check is executed to ensure that the Listener is still in the field. If it is
 *        still preset, the sleep should continue with the next interval, otherwise it should start from the beginning
 *        by polling for new Listeners.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_Sleep_PollSleep.
 *
 * \param[in]        ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateSleep_PollSleep( ptx_RUL_WLC_t *ctx);

/**
 * \brief Deactivates the currently activated Listener and starts the WLC state machine again in state
 *      State_NFCLE_StartPollListener.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_Off_DeactivateListener.
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateOff_DeactivateListener( ptx_RUL_WLC_t *ctx);

/**
 * \brief Processes any errors that happened in the state machine either through a status or through the system state.
 *        This could either terminate the state machine if left unprocessed or start again in
 *        State_NFCLE_StartPollListener if processed correctly. Processing these errors can be done by handling the
 *        corresponding notifications in the WLC notification callback function.
 *
 * \note This function shall only be called when \ref ptx_RUL_WLC_t.State is State_Error.
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 * \param[in]       wlcStatus       Current Status of the WLC State Machine.
 * \param[in]       systemInfo      Current System Information of the WLC State Machine.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_StateError_ProcessError(ptx_RUL_WLC_t *ctx, ptx_Status_t wlcStatus, ptx_RUL_StatusInfoSystem_t systemInfo);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

