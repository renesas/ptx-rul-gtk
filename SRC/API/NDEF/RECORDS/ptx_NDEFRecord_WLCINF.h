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
    Module      : NFC Data Exchange Format (NDEF) Wireless Charging Information (WLCINF) Record
    File        : ptx_NDEFRecord_WLCINF.h

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Wireless Charging Information (WLCINF) NDEF records.
*/

/**
 * \addtogroup NDEF_RECORD_WLCINF
 * @{
 */

#if !defined(PTX_API_NDEF_RECORDS_RECORDWLCINF_H_)
#define PTX_API_NDEF_RECORDS_RECORDWLCINF_H_

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
 * \brief Record type definition for WLC Poll Information record.
 * @{
 */
#define PTX_NDEFRECORD_TYPE_RTD_WLCINF         ("WLCINF")  /**< Record Type Name */
#define PTX_NDEFRECORD_TYPE_RTD_WLCINF_LEN     (6u)        /**< Record Type Name length */

#define PTX_NDEFRECORD_WLCINF_PTX_INT          (0x7F)    /**< P_TX Field mask */
#define PTX_NDEFRECORD_WLCINF_PTX_INT_MAX      (0x64)    /**< P_TX Field max value */
#define PTX_NDEFRECORD_WLCINF_PTX_INT_POS      (0)       /**< P_TX Field position */

#define PTX_NDEFRECORD_WLCINF_WLCPCAP          (0xF0)    /**< WLCPCAP Field mask */
#define PTX_NDEFRECORD_WLCINF_WLCPCAP_POS      (4)       /**< WLCPCAP Field position */
#define PTX_NDEFRECORD_WLCINF_WLCPCAP_STOPSUP  (0x10)    /**< WLCPCAP stop support bit mask */

#define PTX_NDEFRECORD_WLCINF_PCLASS           (0x0F)    /**< PCLASS Field mask */
#define PTX_NDEFRECORD_WLCINF_PCLASS_POS       (0)       /**< PCLASS Field position */
/** @} */



/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/

/**
 * \brief Structure representing the WLC Poll Information Record payload.
 */
typedef struct ptx_NDEFRecord_WLCINF
{
    uint8_t P_TX;                           /**< Transmit Power Level (see [WLC]). */
    uint8_t WLC_PCAP_POWER;                 /**< Poller capabilities (see [WLC]). */
    uint8_t TOT_POWER_STEPS;                /**< Total number of power steps supported by Poller (see [WLC]). */
    uint8_t CUR_POWER_STEP;                 /**< Power step used during last power transfer (see [WLC]). */
    int8_t  NEXT_MIN_STEP_INC;              /**< Step size to increase power level (see [WLC]). */
    int8_t  NEXT_MIN_STEP_DEC;              /**< Step size to decrease power level (see [WLC]). */
} ptx_NDEFRecord_WLCINF_t;

/**
 * \brief Allowed values of the Power Class field within the WLCINF record.
 */
typedef enum ptx_NDEFRecord_WLCINF_PowerClass
{
    PowerClass0 = 0x00, /**< POWER CLASS 0: 250 mW  */
    PowerClass1 = 0x01, /**< POWER CLASS 0: 500 mW  */
    PowerClass2 = 0x02, /**< POWER CLASS 0: 750 mW  */
    PowerClass3 = 0x03  /**< POWER CLASS 0: 1000 mW */
} ptx_NDEFRecord_WLCINF_PowerClass_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Initializes the WLCINF container with zeros.
 *
 * \param[in]   container   Pointer to the uninitialized ptx_NDEFRecord_WLCINF_t container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_Init (ptx_NDEFRecord_WLCINF_t *container);

/**
 * \brief Creates an NDEF record container from the given ptx_NDEFRecord_WLCINF_t struct.
 *
 * \param[in,out]   record      Pointer to an uninitialized NDEF record structure.
 * \param[in]       container   Pointer to the initialized WLCINF container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_Create(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCINF_t *container);

/**
 * \brief Extracts the NDEF record payload into the ptx_NDEFRecord_WLCINF_t container.
 *
 * \param[in]       record      Pointer to an NDEF record structure.
 * \param[in,out]   container   Pointer to the uninitialized WLCINF container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_Parse(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCINF_t *container);

/**
 * \brief Setter for the WLCINF poller transmit power.
 *
 * \param[in]   container   Pointer to the WLCINF container.
 * \param[in]   percent     Current poller transmit power in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_SetPTX(ptx_NDEFRecord_WLCINF_t *container, uint8_t percent);

/**
 * \brief Getter for the WLCINF poller transmit power.
 *
 * \param[in]       container       Pointer to the WLCINF container.
 * \param[in, out]  percent         Valid pointer to store the current poller transmit power in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_GetPTX(ptx_NDEFRecord_WLCINF_t *container, uint8_t *percent);

/**
 * \brief Setter for the WLCINF WPT stop support.
 *
 * \param[in]   container       Pointer to the WLCINF container.
 * \param[in]   wptStopSupport  Enable/Disable WPT stop support.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_SetWptStopSupport(ptx_NDEFRecord_WLCINF_t *container, uint8_t wptStopSupport);

/**
 * \brief Getter for WLCINF stop support.
 *
 * \param[in]       container       Pointer to the WLCINF container.
 * \param[in, out]  wptStopSupport  Valid pointer to store the stop support flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_GetWptStopSupport(ptx_NDEFRecord_WLCINF_t *container, uint8_t *wptStopSupport);

/**
 * \brief Setter for the WLCINF power class.
 *
 * \param[in]   container       Pointer to the WLCINF container.
 * \param[in]   powerClass      Power class to be used for upcoming WPTs.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_SetPowerClass(ptx_NDEFRecord_WLCINF_t *container, ptx_NDEFRecord_WLCINF_PowerClass_t powerClass);

/**
 * \brief Getter for WLCINF power class.
 *
 * \param[in]       container   Pointer to the WLCINF container.
 * \param[in, out]  powerClass  Valid pointer to store current power class into.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_GetPowerClass(ptx_NDEFRecord_WLCINF_t *container, ptx_NDEFRecord_WLCINF_PowerClass_t *powerClass);

/**
 * \brief Setter for the WLCINF total power steps.
 *
 * \param[in]   container   Pointer to the WLCINF container.
 * \param[in]   step        Amount of power steps supported by the poller.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_SetTotPowerSteps(ptx_NDEFRecord_WLCINF_t *container, uint8_t step);

/**
 * \brief Getter for WLCINF total amount of power steps.
 *
 * \param[in]       container   Pointer to the WLCINF container.
 * \param[in, out]  step        Valid pointer to store the amount of supported power steps.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_GetTotPowerSteps(ptx_NDEFRecord_WLCINF_t *container, uint8_t *step);

/**
 * \brief Setter for the WLCINF currently used power step.
 *
 * \param[in]   container   Pointer to the WLCINF container.
 * \param[in]   step        Current power step in use.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_SetCurPowerStep(ptx_NDEFRecord_WLCINF_t *container, uint8_t step);

/**
 * \brief Getter for the WLCINF current power step.
 *
 * \param[in]       container   Pointer to the WLCINF container.
 * \param[in, out]  step        Valid pointer to store the current power step.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_GetCurPowerStep(ptx_NDEFRecord_WLCINF_t *container, uint8_t *step);

/**
 * \brief Setter for the WLCINF minimum power step increase.
 *
 * \param[in]   container   Pointer to the WLCINF container.
 * \param[in]   stepSize    Minimum power step increase.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_SetNextMinStepInc(ptx_NDEFRecord_WLCINF_t *container, int8_t stepSize);

/**
 * \brief Getter for the WLCINF minimum step increase.
 *
 * \param[in]       container   Pointer to the WLCINF container.
 * \param[in, out]  stepSize    Valid pointer to store the step size.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_GetNextMinStepInc(ptx_NDEFRecord_WLCINF_t *container, int8_t *stepSize);

/**
 * \brief Setter for the WLCINF minimum power step decrease.
 *
 * \param[in]   container   Pointer to the WLCINF container.
 * \param[in]   stepSize    Minimum power step decrease.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_SetNextMinStepDec(ptx_NDEFRecord_WLCINF_t *container, int8_t stepSize);

/**
 * \brief Getter for the WLCINF minimum step decrease.
 *
 * \param[in]       container   Pointer to the WLCINF container.
 * \param[in, out]  stepSize    Valid pointer to store the step size.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCINF_GetNextMinStepDec(ptx_NDEFRecord_WLCINF_t *container, int8_t *stepSize);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
