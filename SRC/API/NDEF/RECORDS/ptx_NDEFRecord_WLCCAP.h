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
    Module      : NFC Data Exchange Format (NDEF) Wireless Charging Capability (WLCCAP) Record
    File        : ptx_NDEFRecord_WLCCAP.h

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Wireless Charging Capability (WLCCAP) NDEF records.
*/

/**
 * \addtogroup NDEF_RECORD_WLCCAP
 * @{
 */

#if !defined(PTX_API_NDEF_RECORDS_RECORDWLCCAP_H_)
#define PTX_API_NDEF_RECORDS_RECORDWLCCAP_H_

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
 * \name Record type definition for WLC Capability record.
 * @{
 */
#define PTX_NDEFRECORD_TYPE_RTD_WLCCAP                    ("WLCCAP")  /**< Record Type Name */
#define PTX_NDEFRECORD_TYPE_RTD_WLCCAP_LEN                (6u)        /**< Record Type Name length */

#define PTX_NDEFRECORD_WLCCAP_PROTOCOLVERSION_MAJOR       (0xF0)  /**< Major protocol version mask. */
#define PTX_NDEFRECORD_WLCCAP_PROTOCOLVERSION_MAJOR_POS   (4u)    /**< Major protocol version position. */
#define PTX_NDEFRECORD_WLCCAP_PROTOCOLVERSION_MINOR       (0x0F)  /**< Minor protocol version mask. */
#define PTX_NDEFRECORD_WLCCAP_PROTOCOLVERSION_MINOR_POS   (0)     /**< Minor protocol version position. */

#define PTX_NDEFRECORD_WLCCAP_CONFIG_MODEREQ              (0xC0)  /**< MODE_REQ mask. */
#define PTX_NDEFRECORD_WLCCAP_CONFIG_MODEREQ_POS          (6u)    /**< MODE_REQ position. */
#define PTX_NDEFRECORD_WLCCAP_CONFIG_NWTRET               (0x3C)  /**< NWT_RETRIES mask. */
#define PTX_NDEFRECORD_WLCCAP_CONFIG_NWTRET_POS           (2u)    /**< NWT_RETRIES position. */
#define PTX_NDEFRECORD_WLCCAP_CONFIG_NEGWAIT              (0x02)  /**< NEGO_WAIT flag mask. */
#define PTX_NDEFRECORD_WLCCAP_CONFIG_NEGWAIT_POS          (1u)    /**< NEGO_WAIT flag position. */
#define PTX_NDEFRECORD_WLCCAP_CONFIG_RDCONF               (0x01)  /**< RD_CONF flag mask. */
#define PTX_NDEFRECORD_WLCCAP_CONFIG_RDCONF_POS           (0)     /**< RD_CONF flag position. */

/** CAP_WT_INT Field */
#define PTX_NDEFRECORD_WLCCAP_CAPWT_INT                   (0x1F)  /**< CAP_WT_INT mask. */
#define PTX_NDEFRECORD_WLCCAP_CAPWT_INT_MAX               (0x13)  /**< CAP_WT_INT maximum value. */
#define PTX_NDEFRECORD_WLCCAP_CAPWT_INT_POS               (0)     /**< CAP_WT_INT position. */

/** NDEF_RD_WT Field */
#define PTX_NDEFRECORD_WLCCAP_RDWT_INT                    (0xFF)  /**< RD_WT_INT mask. */
#define PTX_NDEFRECORD_WLCCAP_RDWT_INT_MAX                (0xFE)  /**< RD_WT_INT maximum value. */
#define PTX_NDEFRECORD_WLCCAP_RDWT_INT_POS                (0)     /**< RD_WT_INT position. */
#define PTX_NDEFRECORD_WLCCAP_RDWT_INT_FACTOR             (10)    /**< RD_WT Factor to convert. */

/** WLC_WR_TO_INT Field */
#define PTX_NDEFRECORD_WLCCAP_WRTOINT_INT                 (0x07)  /**< WR_TO_INT mask. */
#define PTX_NDEFRECORD_WLCCAP_WRTOINT_INT_MAX             (0x04)  /**< WR_TO_INT maximum value. */
#define PTX_NDEFRECORD_WLCCAP_WRTOINT_INT_POS             (0)     /**< WR_TO_INT position. */

/** WLC_WR_WT_INT Field */
#define PTX_NDEFRECORD_WLCCAP_WRWTINT_INT                 (0x0F)  /**< WR_WT_INT mask. */
#define PTX_NDEFRECORD_WLCCAP_WRWTINT_INT_MAX             (0x0A)  /**< WR_WT_INT maximum value. */
#define PTX_NDEFRECORD_WLCCAP_WRWTINT_INT_POS             (0)     /**< WR_WT_INT position. */
/** @} */

/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/

/**
 * \brief Structure representing the WLC capability container record payload.
 */
typedef struct ptx_NDEFRecord_WLCCAP
{
    uint8_t WLC_PROTOCOL_VER;                   /**< WLC Protocol Version (see [WLC]). */
    uint8_t WLC_CONFIG;                         /**< WLC Protocol Configuration (see [WLC]). */
    uint8_t CAP_WT_INT;                         /**< CAP Waiting Time Integer (see [WLC]). */
    uint8_t NDEF_RD_WT;                         /**< Response waiting time between WLC_INFO-write and next WLC_CTL-read (see [WLC]). */
    uint8_t NDEF_WR_TO_INT;                     /**< Timeout for writing NDEF (see [WLC]). */
    uint8_t NDEF_WR_WT_INT;                     /**< Timeout/blocking time for subsequenz NDEF-write operations (see [WLC]). */
} ptx_NDEFRecord_WLCCAP_t;

/**
 * \brief Defines the possible value of the mode field within the capability record.
 */
typedef enum ptx_NDEFRecord_WLCCAP_WLCMode
{
    Mode_Static       = 0x00,
    Mode_Negotiated   = 0x01,
    Mode_BattFull     = 0x02,
    Mode_RFU          = 0x03
} ptx_NDEFRecord_WLCCAP_Mode_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Initializes the WLCCAP container with zeros.
 *
 * \param[in]   container   Pointer to the uninitialized ptx_NDEFRecord_WLCCAP_t container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_Init (ptx_NDEFRecord_WLCCAP_t *container);

/**
 * \brief Creates an NDEF record container from the given ptx_NDEFRecord_WLCCAP_t struct.
 *
 * \param[in,out]   record      Pointer to an uninitialized NDEF record structure.
 * \param[in]       container   Pointer to the initialized WLCCAP container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_Create(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCCAP_t *container);

/**
 * \brief Extracts the NDEF record payload into the ptx_NDEFRecord_WLCCAP_t container.
 *
 * \param[in]       record      Pointer to an NDEF record structure.
 * \param[in,out]   container   Pointer to the uninitialized WLCCAP container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_Parse(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCCAP_t *container);

/**
 * \brief Setter for the WLCCAP protocol version, split into major and minor version number.
 *
 * \param[in]   container       Pointer to the WLCCAP container.
 * \param[in]   majorVersion    Major version number.
 * \param[in]   minorVersion    Minor version number.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_SetProtVersion (ptx_NDEFRecord_WLCCAP_t *container, uint8_t majorVersion, uint8_t minorVersion);

/**
 * \brief Getter for the WLCCAP protocol version, split into major and minor version number.
 *
 * \param[in]       container       Pointer to the WLCCAP container.
 * \param[in,out]   majorVersion    Valid pointer to store major version number.
 * \param[in,out]   minorVersion    Valid pointer to store minor version number.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_GetProtVersion (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *majorVersion, uint8_t *minorVersion);

/**
 * \brief Setter for the WLCCAP mode.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   mode        Mode of operation.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_SetMode (ptx_NDEFRecord_WLCCAP_t *container, ptx_NDEFRecord_WLCCAP_Mode_t mode);

/**
 * \brief Getter for the WLCCAP mode.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in,out]   mode       Valid pointer to mode of operation.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_GetMode (ptx_NDEFRecord_WLCCAP_t *container, ptx_NDEFRecord_WLCCAP_Mode_t *mode);

/**
 * \brief Setter for the maximum number of repetitions of CAP_WT in WLCCAP.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   retries     Maximum number of repetitions.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_SetNWtRetries (ptx_NDEFRecord_WLCCAP_t *container, uint8_t retries);

/**
 * \brief Getter for the maximum number of repetitions of CAP_WT in WLCCAP.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   retries     Valid pointer to store retries value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_GetNWtRetries (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *retries);

/**
 * \brief Setter for the NEGO_WAIT flag in WLCCAP.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   negoWait    Enable/Disable NEGO_WAIT.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_SetNegWait (ptx_NDEFRecord_WLCCAP_t *container, uint8_t negoWait);

/**
 * \brief Getter for the NEGO_WAIT flag in WLCCAP.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   negoWait    Valid pointer to store NEGO_WAIT flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_GetNegWait (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *negoWait);

/**
 * \brief Setter for the read confirmation flag in WLCCAP.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   rdConf      Enable/Disable read confirmation flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_SetRdConf (ptx_NDEFRecord_WLCCAP_t *container, uint8_t rdConf);

/**
 * \brief Getter for the read confirmation flag in WLCCAP.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   rdConf      Valid Pointer to store the read confirmation flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_GetRdConf (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *rdConf);

/**
 * \brief Setter for the CAP_WT integer value.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   capWt       CAP_WT integer value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_SetCapWt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t capWt);

/**
 * \brief Getter for the CAP_WT integer value.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   capWt       Valid pointer to store the CAP_WT value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_GetCapWt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *capWt);

/**
 * \brief Getter for the CAP_WT value in milliseconds.
 *
 * \param[in]       container       Pointer to the WLCCAP container.
 * \param[in,out]   capWtMillis     Valid pointer to store the CAP_WT value in milliseconds.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_GetCapWtMillis (ptx_NDEFRecord_WLCCAP_t *container, uint32_t *capWtMillis);

/**
 * \brief Setter for the response waiting time.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   rdWT        RD_WT integer value to be set.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_SetRdWtInt (ptx_NDEFRecord_WLCCAP_t *container, uint16_t rdWT);

/**
 * \brief Getter for the response waiting time.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   rdWT        Valid pointer to store the RD_WT integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_GetRdWtInt (ptx_NDEFRecord_WLCCAP_t *container, uint16_t *rdWT);

/**
 * \brief Setter for the NDEF write timeout integer.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   timeout     WR_TO integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_SetWrToInt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t timeout);

/**
 * \brief Getter for the NDEF write timeout integer.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   timeout     Valid pointer to store the WR_TO integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_GetWrToInt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *timeout);

/**
 * \brief Setter for the NDEF write waiting time integer.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   waitingTime Write waiting time integer value to be set.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_SetWrWtInt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t waitingTime);

/**
 * \brief Getter for the NDEF write waiting time integer.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   waitingTime Valid pointer to store the write waiting time integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_GetWrWtInt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *waitingTime);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
