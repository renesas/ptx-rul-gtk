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
    Module      : NFC Data Exchange Format (NDEF) Wireless Charging Control (WLCCTL) Record
    File        : ptx_NDEFRecord_WLCCTL.h

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Wireless Charging Control (WLCCTL) NDEF records.
*/

/**
 * \addtogroup NDEF_RECORD_WLCCTL
 * @{
 */

#if !defined(PTX_API_NDEF_RECORDS_RECORDWLCCTL_H_)
#define PTX_API_NDEF_RECORDS_RECORDWLCCTL_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NDEFRecord.h"
#include "ptx_Status.h"

#if defined(__cplusplus)
extern "C" {
#endif
/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name Record type definition for WLC Listen Control record.
 * @{
 */
#define PTX_NDEFRECORD_TYPE_RTD_WLCCTL                     ("WLCCTL")  /**< Record Type Name */
#define PTX_NDEFRECORD_TYPE_RTD_WLCCTL_LEN                 (6u)        /**< Record Type Name length */

#define PTX_NDEFRECORD_WLCCTL_STATUSINFO_ERRORFLAG         (0x80)  /**< STATUS_INFO ERROR mask */
#define PTX_NDEFRECORD_WLCCTL_STATUSINFO_ERRORFLAG_POS     (7u)    /**< STATUS_INFO ERROR position */
#define PTX_NDEFRECORD_WLCCTL_STATUSINFO_BATTERYSTATUS     (0x18)  /**< STATUS_INFO BATSTAT mask */
#define PTX_NDEFRECORD_WLCCTL_STATUSINFO_BATTERYSTATUS_POS (3u)    /**< STATUS_INFO BATSTAT position */
#define PTX_NDEFRECORD_WLCCTL_STATUSINFO_COUNTER           (0x07)  /**< STATUS_INFO COUNTER mask */
#define PTX_NDEFRECORD_WLCCTL_STATUSINFO_COUNTER_POS       (0)     /**< STATUS_INFO COUNTER position */

#define PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTREQ             (0xC0)  /**< WPT_CONF REQUEST mask */
#define PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTREQ_POS         (6u)    /**< WPT_CONF REQUEST position */
#define PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTDURATION        (0x3E)  /**< WPT_CONF DURATION mask */
#define PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTDURATION_POS    (1u)    /**< WPT_CONF DURATION position */
#define PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTDURATION_MAX    (0x13)  /**< WPT_CONF DURATION maximum value */
#define PTX_NDEFRECORD_WLCCTL_WPTCONFIG_INFOREQ            (0x01)  /**< WPT_CONF INFO REQUEST mask */
#define PTX_NDEFRECORD_WLCCTL_WPTCONFIG_INFOREQ_POS        (0)     /**< WPT_CONF INFO REQUEST position */

#define PTX_NDEFRECORD_WLCCTL_DVRINFO_FLAG                 (0xC0)  /**< DVRINFO INFO SUPPORT mask */
#define PTX_NDEFRECORD_WLCCTL_DVRINFO_FLAG_POS             (6u)    /**< DVRINFO INFO SUPPORT position */
#define PTX_NDEFRECORD_WLCCTL_DVRINFO_VALUE                (0x3F)  /**< DVRINFO INTEGER mask */
#define PTX_NDEFRECORD_WLCCTL_DVRINFO_VALUE_POS            (0)     /**< DVRINFO INTEGER position */

#define PTX_NDEFRECORD_WLCCTL_HOLDOFFWT                    (0x0F)  /**< HOLDOFF_WT INTEGER mask */
#define PTX_NDEFRECORD_WLCCTL_HOLDOFFWT_MAX                (0x0F)  /**< HOLDOFF_WT INTEGER maximum value */
#define PTX_NDEFRECORD_WLCCTL_HOLDOFFWT_POS                (0)     /**< HOLDOFF_WT INTEGER position */

#define PTX_NDEFRECORD_WLCCTL_ERRINFO_TEMP                 (0x01)  /**< ERROR_INFO TEMP ERROR mask */
#define PTX_NDEFRECORD_WLCCTL_ERRINFO_PROT                 (0x02)  /**< ERROR_INFO PROTOCOL ERROR mask */

#define PTX_NDEFRECORD_WLCCTL_PWRADJREQ_MAX                (20)    /**< PWR_ADJ maximum value */
#define PTX_NDEFRECORD_WLCCTL_PWRADJREQ_MIN                (-10)   /**< PWR_ADJ minimum value (signed) */
#define PTX_NDEFRECORD_WLCCTL_PWRADJREQ_DEFAULT            (0x00)  /**< PWR_ADJ default value */
#define PTX_NDEFRECORD_WLCCTL_PWRADJ_PERCENT_PER_STEP      (5)     /**< PWR_ADJ percent conversion factor */

#define PTX_NDEFRECORD_WLCCTL_ERRORINFO_PROP_MASK          (0x07)  /**< Proprietary PTX ERROR_INFO mask */
#define PTX_NDEFRECORD_WLCCTL_ERRORINFO_PROP_POS           (0x00)  /**< Proprietary PTX ERROR_INFO position */
/** @} */

/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/
/**
 * \brief Structure representing the WLC Listen Control Record's payload.
 */
typedef struct ptx_NDEFRecord_WLCCTL
{
    uint8_t STATUS_INFO;            /**< Status Info about Listener (see [WLC]). */
    uint8_t WPT_CONFIG;             /**< Power transfer configuration (see [WLC]). */
    int8_t  POWER_ADJ_REQ;          /**< Power adjustment request configuration (see [WLC]). */
    uint8_t BATTERY_LEVEL;          /**< Listener battery level information (see [WLC]). */
    uint8_t DVR_INFO;               /**< DVR parameter (see [WLC]). */
    uint8_t HOLD_OFF_WT_INT;        /**< Hold-off waiting time integer (see [WLC]). */
    uint8_t ERROR_INFO;             /**< Additional error information (see [WLC]). */
} ptx_NDEFRecord_WLCCTL_t;

/**
 * \brief Battery status information.
 */
typedef enum ptx_NDEFRecord_WLCCTL_BatteryStatus
{
    NoInformation       = 0x00,
    BatteryCharging     = 0x01,
    NoBatteryCharging   = 0x20
} ptx_NDEFRecord_WLCCTL_BatteryStatus_t;

/**
 * \brief DVR support status.
 */
typedef enum ptx_NDEFRecord_WLCCTL_DvrSupport
{
    DVRSupport_No       = 0x00,
    DVRSupport_Yes_NA   = 0x01,
    DVRSupport_Yes      = 0x02
} ptx_NDEFRecord_WLCCTL_DvrSupport_t;

/**
 * \brief Proprietary ERROR_INFO data.
 */
typedef enum ptx_NDEFRecord_WLCCTL_Proprietary_ErrorInfo
{
    ErrorInfo_OK                    = 0x00,
    ErrorInfo_IcTemperature         = 0x01,
    ErrorInfo_Protocol              = 0x02,
    ErrorInfo_BatteryConnection     = 0x04,
    ErrorInfo_BatteryTemperature    = 0x08,
    ErrorInfo_TcmTimeout            = 0x0C,
    ErrorInfo_CcmTimeout            = 0x10,
    ErrorInfo_CvmTimeout            = 0x14,
    ErrorInfo_IcOverpower           = 0x18
} ptx_NDEFRecord_WLCCTL_Proprietary_ErrorInfo_t;
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Initializes the WLCCTL container with zeros.
 *
 * \param[in]   container   Pointer to the uninitialized WLCCTL container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_Init (ptx_NDEFRecord_WLCCTL_t *container);

/**
 * \brief Create an NDEF record container from the given ptx_NDEFRecord_WLCCTL_t struct.
 *
 * \param[in,out]   record      Pointer to an uninitialized NDEF record structure.
 * \param[in]       container   Pointer to the initialized WLCCTL container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_Create(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCCTL_t *container);

/**
 * \brief Extracts the NDEF record payload into the ptx_NDEFRecord_WLCCTL_t container.
 *
 * \param[in]       record      Pointer to an NDEF record structure.
 * \param[in,out]   container   Pointer to the uninitialized WLCCTL container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_Parse(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCCTL_t *container);

/**
 * \brief Getter for the WLCCTL length of the WLCCTL record.
 *        ERROR_INFO is optional, hence length can change.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   len         Valid pointer to store the container length.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetLength(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *len);

/**
 * \brief Setter for the WLCCTL error flag within STATUS_INFO field.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   error       Set/clear error flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetErrorFlag(ptx_NDEFRecord_WLCCTL_t *container, uint8_t error);

/**
 * \brief Getter for the WLCCTL error flag within the STATUS_INFO field.
 *        If this flag is set, additional ERROR_INFO field must be set.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   error         Valid pointer to store the error flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetErrorFlag(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *error);

/**
 * \brief Setter for the WLCCTL battery status information.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   batteryStatus  Current battery status.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetBatteryStatus(ptx_NDEFRecord_WLCCTL_t *container, ptx_NDEFRecord_WLCCTL_BatteryStatus_t batteryStatus);

/**
 * \brief Getter for the WLCCTL battery status field.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   batteryStatus  Valid pointer to store the battery status.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetBatteryStatus(ptx_NDEFRecord_WLCCTL_t *container, ptx_NDEFRecord_WLCCTL_BatteryStatus_t *batteryStatus);

/**
 * \brief Setter for the WLCCTL counter.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   counter     Counter value (3 Bit).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetCounter(ptx_NDEFRecord_WLCCTL_t *container, uint8_t counter);

/**
 * \brief Getter for the WLCCTL counter field. If the counter changes between
 *        readouts, this indicates to the poller that the Listener updated its
 *        WLCCTL record.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   counter     Valid pointer to store the counter value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetCounter(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *counter);

/**
 * \brief Setter for the WLCCTL WPT duration integer.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   wptRequest  WPT request.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetWptReq(ptx_NDEFRecord_WLCCTL_t *container, uint8_t wptRequest);

/**
 * \brief Getter for the WLCCTL WPT request flag.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   wptRequest     Valid pointer to store the WPT request flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetWptReq(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *wptRequest);

/**
 * \brief Setter for the WLCCTL WPT duration integer.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   duration    WPT duration integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetWptDuration(ptx_NDEFRecord_WLCCTL_t *container, uint8_t duration);

/**
 * \brief Getter for the WLCCTL WPT duration integer.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   duration    Valid pointer to store the WPT duration integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetWptDuration(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *duration);

/**
 * \brief Getter for the WLCCTL WPT duration time in milliseconds.
 *
 * \param[in]       container       Pointer to the WLCCTL container.
 * \param[in,out]   wptDurationMs   Valid pointer to store the WPT duration in milliseconds.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetWptDurationMillis(ptx_NDEFRecord_WLCCTL_t *container, uint32_t *wptDurationMs);

/**
 * \brief Setter for the WLCCTL INFO_REQ flag
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   request     Set/clear request flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetWptInfoReq(ptx_NDEFRecord_WLCCTL_t *container, uint8_t request);

/**
 * \brief Getter for the WLCCTL INFO_REQ flag.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   request     Valid pointer to store the INFO_REQ flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetWptInfoReq(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *request);

/**
 * \brief Setter for the WLCCTL power adjust level.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   adjustLevel Power adjust level.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetPowerAdjReq(ptx_NDEFRecord_WLCCTL_t *container, int8_t adjustLevel);

/**
 * \brief Getter for the WLCCTL power adjust request of the Listener.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   adjustLevel Valid pointer to store the power adjust integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetPowerAdjReq(ptx_NDEFRecord_WLCCTL_t *container, int8_t *adjustLevel);

/**
 * \brief Setter for the WLCCTL battery level.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   batteryLevel   Battery level in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetBatteryLevel(ptx_NDEFRecord_WLCCTL_t *container, uint8_t batteryLevel);

/**
 * \brief Getter for the WLCCTL battery level.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   batteryLevel   Valid pointer to store the battery level in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetBatteryLevel(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *batteryLevel);

/**
 * \brief Setter for the WLCCTL DVR support value.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   support     DVR support value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetDvrFlag(ptx_NDEFRecord_WLCCTL_t *container, ptx_NDEFRecord_WLCCTL_DvrSupport_t support);

/**
 * \brief Getter for the WLCCTL DVR_INFO support.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   support     Valid pointer to store the support information.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetDvrFlag(ptx_NDEFRecord_WLCCTL_t *container, ptx_NDEFRecord_WLCCTL_DvrSupport_t *support);

/**
 * \brief Setter for the WLCCTL DVR integer.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   currDvr     Current DVR integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetDvrValue(ptx_NDEFRecord_WLCCTL_t *container, uint8_t currDvr);

/**
 * \brief Getter for the WLCCTL DVR integer.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   currDvr     Valid pointer to store the DVR integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetDvrValue(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *currDvr);

/**
 * \brief Getter for the WLCCTL HOLD_OFF_WT integer.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   holdOffWt   Valid pointer to store HOLD_OFF_WT integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetHoldOffWt(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *holdOffWt);

/**
 * \brief Getter for the WLCCTL HOLD_OFF_WT in milliseconds.
 *
 * \param[in]       container       Pointer to the WLCCTL container.
 * \param[in,out]   holdOffWtMillis Valid pointer to store HOLD_OFF_WT in milliseconds.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetHoldOffWtMillis(ptx_NDEFRecord_WLCCTL_t *container, uint32_t *holdOffWtMillis);

/**
 * \brief Setter for the WLCCTL HOLD_OFF_WT integer.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   holdOffWt   HOLD_OFF_WT integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetHoldOffWt(ptx_NDEFRecord_WLCCTL_t *container, uint8_t holdOffWt);

/**
 * \brief Setter for the WLCCTL over temperature error.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   ovTemp      Set/Clear temperature error flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetErrorInfoOvTemp(ptx_NDEFRecord_WLCCTL_t *container, uint8_t ovTemp);

/**
 * \brief Getter for the WLCCTL over temperature error.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   ovTemp      Valid pointer to store flag, if an error has occurred.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetErrorInfoOvTemp(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *ovTemp);

/**
 * \brief Setter for the WLCCTL protocol error.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   protError   Set/Clear protocol error flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_SetErrorInfoProtocol(ptx_NDEFRecord_WLCCTL_t *container, uint8_t protError);

/**
 * \brief Getter for the WLCCTL protocol error.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   protError   Valid pointer to store flag, if an error has occurred.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetErrorInfoProtocol(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *protError);

/**
 * \brief Getter for the proprietary PTX error codes.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   err         Valid pointer to store the PTX error code.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCTL_GetErrorInfoProprietary(ptx_NDEFRecord_WLCCTL_t *container, ptx_NDEFRecord_WLCCTL_Proprietary_ErrorInfo_t *err);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
