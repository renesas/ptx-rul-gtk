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
    Module      : NFC Data Exchange Format (NDEF) Text Record
    File        : ptx_NDEFRecord_Text.h

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Text NDEF records.
*/

/**
 * \addtogroup NDEF_RECORD_TEXT
 * @{
 */

#if !defined(PTX_API_NDEF_RECORDS_RECORD_TEXT_H_)
#define PTX_API_NDEF_RECORDS_RECORD_TEXT_H_

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
 * \name Generic Test record definitions.
 * @{
 */
#define PTX_NDEFRECORD_TYPE_RTD_TEXT           ("T")          /**< Record Type Name */
#define PTX_NDEFRECORD_TYPE_RTD_TEXT_LEN       (1u)           /**< Record Type Name length */
/** @} */


/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Creates an NDEF Text record from specified plain text and language code.
 *
 * \param[in,out]   record          Pointer to an uninitialized NDEF record structure.
 * \param[in]       workBuffer      Pointer to a buffer which will be used to hold the data of the NDEF record
 * \param[in]       workBufferLen   Size of the provided workBuffer
 * \param[in]       languageCode    Language code (must be null terminated).
 * \param[in]       text            Plain text (must be null terminated).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Text_Create (ptx_NDEFRecord_t *record, uint8_t *workBuffer, uint32_t workBufferLen, char *languageCode, char *text);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
