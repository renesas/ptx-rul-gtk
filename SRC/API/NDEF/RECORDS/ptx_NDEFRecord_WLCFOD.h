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
    Module      : NFC Data Exchange Format (NDEF) Wireless Charging Foreign Object Detection (WLCFOD) Record
    File        : ptx_NDEFRecord_WLCFOD.h

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Wireless Charging Foreign Object Detection (WLCFOD) NDEF records.
*/

/**
 * \addtogroup NDEF_RECORD_WLCFOD
 * @{
 */

#if !defined(PTX_API_NDEF_RECORDS_RECORDWLCFOD_H_)
#define PTX_API_NDEF_RECORDS_RECORDWLCFOD_H_

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
 * \brief Record type definition for WLC JiFOD Method record.
 * @{
 */
#define TYPE_RTD_WLCFOD         ("WLCFOD")      /**< Record Type Name */
#define TYPE_RTD_WLCFOD_LEN     (6u)            /**< Record Type Name length */
/** @} */


/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/
/**
 * \brief Structure representing the mandatory payload of the WLC JiFOD Record.
 */
typedef struct ptx_NDEFRecord_WLCFOD
{
    uint8_t COMPANY_ID;       /**< Company identifier (see [WLC]). */
    uint8_t METHOD_ID;        /**< JiFOD Method identifier (see [WLC]). */
} ptx_NDEFRecord_WLCFOD_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Initializes the WLCFOD container with zeros.
 *
 * \param[in]   container   Pointer to the uninitialized WLCFOD container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCFOD_Init (ptx_NDEFRecord_WLCFOD_t *container);

/**
 * \brief Create an NDEF record container from the given ptx_NDEFRecord_WLCFOD_t struct.
 *
 * \param[in,out]   record      Pointer to an uninitialized NDEF record structure.
 * \param[in]       container   Pointer to the initialized capability container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCFOD_Create(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCFOD_t *container);

/**
 * \brief Extracts the NDEF record payload into the ptx_NDEFRecord_WLCFOD_t container.
 *
 * \param[in]       record      Pointer to an NDEF record structure.
 * \param[in,out]   container   Pointer to the uninitialized WLCFOD container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCFOD_Parse(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCFOD_t *container);

/**
 * \brief Setter for the WLCFOD company ID.
 *
 * \param[in]       container   Pointer to the WLCFOD container.
 * \param[in]       companyId   Company ID.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCFOD_SetCompanyId(ptx_NDEFRecord_WLCFOD_t *container, uint8_t companyId);

/**
 * \brief Getter for the WLCFOD company ID.
 *
 * \param[in]       container   Pointer to the WLCFOD container.
 * \param[in,out]   companyId   Valid pointer to store the Company ID.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCFOD_GetCompanyId(ptx_NDEFRecord_WLCFOD_t *container, uint8_t *companyId);

/**
 * \brief Setter for the WLCFOD method ID.
 *
 * \param[in]       container   Pointer to the WLCFOD container.
 * \param[in]       methodId    Used method ID for FOD.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCFOD_SetMethodId(ptx_NDEFRecord_WLCFOD_t *container, uint8_t methodId);

/**
 * \brief Getter for the WLCFOD method ID.
 *
 * \param[in]       container   Pointer to the WLCFOD container.
 * \param[in,out]   methodId    Valid pointer to store the used method ID for FOD.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_WLCFOD_GetMethodId(ptx_NDEFRecord_WLCFOD_t *container, uint8_t *methodId);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
