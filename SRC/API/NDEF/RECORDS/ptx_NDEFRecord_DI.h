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
    Module      : NFC Data Exchange Format (NDEF) Device Information (DI) Record
    File        : ptx_NDEFRecord_DI.h

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Device Information (DI) NDEF records.
*/

/**
 * \addtogroup NDEF_RECORD_DI
 * @{
 */

#if !defined(PTX_API_NDEF_RECORDS_RECORDDI_H_)
#define PTX_API_NDEF_RECORDS_RECORDDI_H_

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
 * \name Generic Device Information record definitions.
 * @{
 */
#define PTX_NDEFRECORD_TYPE_RTD_DEVICE_INFO                    ("Di")      /**< Record Type Name */
#define PTX_NDEFRECORD_TYPE_RTD_DEVICE_INFO_LEN                (2u)        /**< Record Type Name length */

#define PTX_NDEFRECORD_DI_MAX_STRLEN                           (256u)      /**< Record Type single max. information payload */
#define PTX_NDEFRECORD_DEVICE_INFORMATION_MAXLEN               (1036u)     /**< Record Type single max. information payload length */
/** @} */

/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/

/**
 * \brief Device Information record TLV type.
 */
typedef enum ptx_NDEFRecord_DI_type
{
    DI_Type_ManufacturerName = 0x00,
    DI_Type_ModelName        = 0x01,
    DI_Type_DeviceUniqueName = 0x02,
    DI_Type_UUID             = 0x03,
    DI_Type_FirmwareVersion  = 0x04,
    DI_Type_RFU              = 0x05
} ptx_NDEFRecord_DI_type_t;

/**
 * \brief Device Information record generic Length Value type.
 */
typedef struct ptx_NDEFRecord_DI_LV
{
    uint8_t     Length;         /**< Device Information String length */
    char       *Value;          /**< Device Information String value */
} ptx_NDEFRecord_DI_LV_t;

/**
 * \brief Device Information record UUID Length Value type.
 */
typedef struct ptx_NDEFRecord_DI_LV_UUID
{
    uint8_t     Length;         /**< Device Information UUID length */
    uint8_t    *Value;          /**< Device Information UUID value */
} ptx_NDEFRecord_DI_LV_UUID_t;

/**
 * \brief Device Information record container.
 */
typedef struct ptx_NDEFRecord_DI
{
    ptx_NDEFRecord_DI_LV_t      ManufacturerName;   /**< required to be set */
    ptx_NDEFRecord_DI_LV_t      ModelName;          /**< required to be set */
    ptx_NDEFRecord_DI_LV_t      DeviceUniqueName;   /**< optional */
    ptx_NDEFRecord_DI_LV_UUID_t Uuid;               /**< optional */
    ptx_NDEFRecord_DI_LV_t      FirmwareVersion;    /**< optional */
} ptx_NDEFRecord_DI_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Sets the device information container to zero.
 *
 * \param[in]       container       Pointer to the device information record.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Di_Init (ptx_NDEFRecord_DI_t *container);

/**
 * \brief Creates an NDEF record container from the given ptx_NDEFRecord_DI_t struct.
 *
 * \param[in,out]   record          Pointer to an uninitialized NDEF record structure.
 * \param[in]       container       Pointer to the initialized ptx_NDEFRecord_DI_t container.
 * \param[in,out]   payloadBuffer   Pointer to a payload buffer for storing the data of dynamic length.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Di_Create (ptx_NDEFRecord_t *record, ptx_NDEFRecord_DI_t *container, uint8_t *payloadBuffer);

/**
 * \brief Parses a given NDEF record as Device Information record and stores it in the ptx_NDEFRecord_DI_t container.
 *
 * \param[in]       srcRecord       Pointer to the NDEF record to be parsed.
 * \param[out]      dst             Pointer to the device information record for storing the parsed data.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Di_Parse (ptx_NDEFRecord_t *srcRecord, ptx_NDEFRecord_DI_t *dst);

/**
 * \brief Sets the mandatory manufacturer name field within the device information container and updates its TLV.
 *
 * \param[in]       container           Pointer to the Device Information container to be updated.
 * \param[in]       manufacturerName    Pointer to the manufacturer character array (must be null terminated).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Di_SetManufacturerName (ptx_NDEFRecord_DI_t *container, char *manufacturerName);

/**
 * \brief Sets the mandatory model name field within the device information container and updates its TLV.
 *
 * \param[in]       container           Pointer to the Device Information container to be updated.
 * \param[in]       modelName       Pointer to the manufacturer character array (must be null terminated).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Di_SetModelName (ptx_NDEFRecord_DI_t *container, char *modelName);

/**
 * \brief Sets the optional device unique name field within the device information container and updates its TLV.
 *
 * \param[in]       container           Pointer to the Device Information container to be updated.
 * \param[in]       deviceUniqueName    Pointer to the unique name character array (must be null terminated).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Di_SetDeviceUniqueName (ptx_NDEFRecord_DI_t *container, char *deviceUniqueName);

/**
 * \brief Sets the optional UUID field within the device information container and updates its TLV.
 *
 * \param[in]       container   Pointer to the Device Information container to be updated.
 * \param[in]       uuid        Pointer to the 16 byte UUID.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Di_SetUUID (ptx_NDEFRecord_DI_t *container, uint8_t uuid[16]);

/**
 * \brief Sets the optional firmware version name within the device information container and updates its TLV.
 *
 * \param[in]       container           Pointer to the Device Information container to be updated.
 * \param[in]       firmwareVersion     Pointer to the unique name character array (must be null terminated).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Di_SetFirmwareVersion (ptx_NDEFRecord_DI_t *container, char *firmwareVersion);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
