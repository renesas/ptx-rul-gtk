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
    Module      : NFC Data Exchange Format (NDEF) Wireless Charging Status and Information (WLCSTAI) Record
    File        : ptx_NDEFRecord_WLCSTAI.h

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Wireless Charging Status and Information (WLCSTAI) NDEF records.
*/

/**
 * \addtogroup NDEF_RECORD_WLCSTAI
 * @{
 */

#if !defined(PTX_API_NDEF_RECORDS_RECORDWLCSTAI_H_)
#define PTX_API_NDEF_RECORDS_RECORDWLCSTAI_H_

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
* \brief Record type definition for WLC Status and Information record.
 * @{
*/
#define PTX_NDEFRECORD_TYPE_RTD_WLCSTAI       ("WLCSTAI")   /**< Record Type Name */
#define PTX_NDEFRECORD_TYPE_RTD_WLCSTAI_LEN   (7u)          /**< Record Type Name length */

/** CONTROL_BYTE_1 Field */
#define PTX_NDEFRECORD_WLCSTAI_CTRL1_BATLVL   (0x01)  /**< CTRL1 BATTLVL set mask */
#define PTX_NDEFRECORD_WLCSTAI_CTRL1_RECPWR   (0x02)  /**< CTRL1 RECPWR set mask */
#define PTX_NDEFRECORD_WLCSTAI_CTRL1_RECVLT   (0x04)  /**< CTRL1 RECVLT set mask */
#define PTX_NDEFRECORD_WLCSTAI_CTRL1_RECCUR   (0x08)  /**< CTRL1 RECCUR set mask */
#define PTX_NDEFRECORD_WLCSTAI_CTRL1_TEMPBAT  (0x10)  /**< CTRL1 TEMPBAT set mask */
#define PTX_NDEFRECORD_WLCSTAI_CTRL1_TEMPWLCL (0x20)  /**< CTRL1 TEMPWLCL set mask */
#define PTX_NDEFRECORD_WLCSTAI_CTRL1_RFU      (0x40)  /**< CTRL1 RFU */
#define PTX_NDEFRECORD_WLCSTAI_CTRL1_CTRL2    (0x80)  /**< CTRL1 CTRL2 set mask */

/** CONTROL_BYTE_2 Field */
#define PTX_NDEFRECORD_WLCSTAI_CTRL2_BATVLT   (0x01)  /**< CTRL2 BATTVLT set mask */
#define PTX_NDEFRECORD_WLCSTAI_CTRL2_BATCUR   (0x02)  /**< CTRL2 BATTCUR set mask */

/** BATTERY_LEVEL Field */
#define PTX_NDEFRECORD_WLCSTAI_BATLVL         (0x7F)  /**< BATLVL mask */
#define PTX_NDEFRECORD_WLCSTAI_BATLVL_MAX     (0x64)  /**< BATLVL maximum value */
#define PTX_NDEFRECORD_WLCSTAI_BATLVL_POS     (0x00)  /**< BATLVL position */
/** @} */


/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/

/**
 * \brief Structure representing the WLC Status and Info Record payload.
 */
typedef struct ptx_NDEFRecord_WLCSTAI
{
    uint8_t CONTROL;                /**< Control-Byte 1 definining presence of actual information bytes (see [WLC]). */
    uint8_t BATTERY_LEVEL;          /**< Battery Level (see [WLC]). */
    uint8_t RECEIVE_POWER;          /**< Receive Power (see [WLC]). */
    uint8_t RECEIVE_VOLTAGE;        /**< Receive Voltage (see [WLC]). */
    uint8_t RECEIVE_CURRENT;        /**< Receive Current (see [WLC]). */
    int8_t  TEMPERATURE_BATTERY;    /**< Battery Temperature (see [WLC]). */
    int8_t  TEMPERATURE_WLCL;       /**< Listener Temperature (see [WLC]). */
    uint8_t RFU;                    /**< RFU. */
    uint8_t CONTROL2;               /**< Control-Byte 2 definining presence of actual information bytes (see [WLC]). */
    uint8_t BATTERY_VOLTAGE;        /**< Battery Voltage (see [WLC]). */
    uint8_t BATTERY_CURRENT;        /**< Battery Current (see [WLC]). */
} ptx_NDEFRecord_WLCSTAI_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Initializes the WLCSTAI container with zeros.
 *
 * \param[in]   container   Pointer to the uninitialized ptx_NDEFRecord_WLCSTAI_t container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_Init (ptx_NDEFRecord_WLCSTAI_t *container);

/**
 * \brief Create an NDEF record container from the given ptx_NDEFRecord_WLCSTAI_t struct.
 *
 * \param[in,out]   record      Pointer to an uninitialized NDEF record structure.
 * \param[in]       container   Pointer to the initialized WLCSTAI container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_Create(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCSTAI_t *container);

/**
 * \brief Extracts the NDEF record payload into the ptx_NDEFRecord_WLCSTAI_t container.
 *
 * \param[in]       record      Pointer to an NDEF record structure.
 * \param[in,out]   container   Pointer to the uninitialized WLCSTAI container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_Parse(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCSTAI_t *container);

/**
 * \brief Setter for the WLCSTAI battery level.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       percent     Battery level in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetBatteryLevel(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t percent);

/**
 * \brief Getter for the WLCSTAI battery level measured by the Listener.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   percent     Valid pointer to store the battery level in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetBatteryLevel(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *percent);

/**
 * \brief Setter for the WLCSTAI power level.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       power       Power level.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetReceivePower(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t power);

/**
 * \brief Getter for the WLCSTAI received power by the Listener.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   power       Valid pointer to store the power.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetReceivePower(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *power);

/**
 * \brief Setter for the WLCSTAI voltage level.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       voltage     Voltage level.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetReceiveVoltage(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t voltage);

/**
 * \brief Getter for the WLCSTAI received voltage by the Listener.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   voltage     Valid pointer to store the voltage.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetReceiveVoltage(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *voltage);

/**
 * \brief Setter for the WLCSTAI current.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       current     Current value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetReceiveCurrent(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t current);

/**
 * \brief Getter for the WLCSTAI received current by the Listener.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   current     Valid pointer to store the current.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetReceiveCurrent(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *current);

/**
 * \brief Setter for the WLCSTAI battery temperature.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       celsius     Temperature in celsius.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetTemperatureBat(ptx_NDEFRecord_WLCSTAI_t *container, int8_t celsius);

/**
 * \brief Getter for the WLCSTAI battery temperature.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   celsius     Valid pointer to store the temperature in degree celsius.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetTemperatureBat(ptx_NDEFRecord_WLCSTAI_t *container, int8_t *celsius);

/**
 * \brief Setter for the WLCSTAI Listener temperature.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       celsius     Temperature in celsius.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetTemperatureWlcl(ptx_NDEFRecord_WLCSTAI_t *container, int8_t celsius);

/**
 * \brief Getter for the WLCSTAI Listener temperature.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   celsius     Valid pointer to store the temperature in degree celsius.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetTemperatureWlcl(ptx_NDEFRecord_WLCSTAI_t *container, int8_t *celsius);

/**
 * \brief Setter for the WLCSTAI battery voltage.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       voltage     Battery voltage.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetBatteryVoltage(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t voltage);

/**
 * \brief Getter for the WLCSTAI battery voltage.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   voltage     Valid pointer to store the battery voltage.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetBatteryVoltage(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *voltage);

/**
 * \brief Setter for the WLCSTAI battery current.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       current     Battery charge current.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetBatteryCurrent(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t current);

/**
 * \brief Getter for the WLCSTAI battery current.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   current     Valid pointer to store the battery charge current.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetBatteryCurrent(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *current);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
