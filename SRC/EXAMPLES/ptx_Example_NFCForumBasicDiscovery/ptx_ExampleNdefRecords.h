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
    Module      : SDK Example (NFC Forum Basic Discovery)
    File        : EXAMPLES/ptx_Example_NFCForumBasicDiscovery/ptx_ExampleNdefRecords.h

    Description : Table-driven definitions of standard NDEF record types (URI, Text, Smart Poster,
                  Phone, Email, SMS, Geo location, vCard, MIME, Wi-Fi, Android application record).
                  Each type defines its input fields and an encoder, which turns the field values
                  into an NDEF record. Records are combined into an NDEF message using the SDK's
                  NDEF record/message API.
                  To add a new record type, define its fields and encoder and add an entry to the
                  record type table in ptx_ExampleNdefRecords.c.
*/

#if !defined(_PTX_SDK_EXAMPLE_NDEF_RECORDS_H_)
#define _PTX_SDK_EXAMPLE_NDEF_RECORDS_H_

#include <stdint.h>
#include <glib.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Kind of an input field.
 */
typedef enum {
    NdefField_Text,                     /**< Free text entry. */
    NdefField_Choice                    /**< Selection from a fixed list of choices. */
} NdefFieldKind;

/**
 * \brief Definition of an input field of an NDEF record type.
 */
typedef struct {
    const char *label;                  /**< Field label shown in the UI. */
    NdefFieldKind kind;                 /**< Kind of field. */
    gboolean required;                  /**< Field must not be empty. */
    const char *defaultValue;           /**< Initial entry text, or the initially selected choice (NULL: first). */
    const char * const *choices;        /**< NULL-terminated list of choices (NdefField_Choice only). */
} NdefFieldDef;

/**
 * \brief Encoded NDEF record, ready to be added to an NDEF message.
 */
typedef struct {
    uint8_t tnf;                        /**< Type Name Format (ptx_NDEFRecord_TNF_t). */
    GByteArray *type;                   /**< Record type. */
    GByteArray *payload;                /**< Record payload. */
    gchar *summary;                     /**< Human-readable one-line description. */
} NdefRecordData;

/**
 * \brief Encoder of an NDEF record type.
 *
 * \param[in]   values  Field values, one per field definition (choice fields: the selected choice).
 * \param[out]  out     Initialized record data to fill in.
 * \param[out]  error   Set to a user-readable message if the values are invalid.
 *
 * \return TRUE on success.
 */
typedef gboolean (*NdefEncodeFn)(const gchar * const *values, NdefRecordData *out, GError **error);

/**
 * \brief Definition of an NDEF record type.
 */
typedef struct {
    const char *name;                   /**< Record type name shown in the UI. */
    const NdefFieldDef *fields;         /**< Input fields. */
    guint nFields;                      /**< Number of input fields. */
    NdefEncodeFn encode;                /**< Encoder. */
} NdefRecordTypeDef;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Returns the table of supported NDEF record types.
 *
 * \param[out]  count   Number of entries in the table.
 */
const NdefRecordTypeDef *ptx_ExampleNdef_GetRecordTypes(guint *count);

/**
 * \brief Returns the URI prefix of an NFC Forum URI identifier code, NULL for unknown codes.
 */
const char *ptx_ExampleNdef_GetUriPrefix(uint8_t code);

/**
 * \brief Validates the field values and encodes an NDEF record of the given type.
 *
 * \param[in]   def     Record type definition.
 * \param[in]   values  Field values, one per field definition.
 * \param[out]  error   Set to a user-readable message on failure.
 *
 * \return Newly allocated record data (free with ptx_ExampleNdef_RecordDataFree), NULL on failure.
 */
NdefRecordData *ptx_ExampleNdef_EncodeRecord(const NdefRecordTypeDef *def, const gchar * const *values, GError **error);

/**
 * \brief Frees record data returned by ptx_ExampleNdef_EncodeRecord.
 */
void ptx_ExampleNdef_RecordDataFree(NdefRecordData *data);

/**
 * \brief Builds an NDEF message from a list of records.
 *
 * \param[in]       records Array of NdefRecordData*, at least one record.
 * \param[out]      buffer  Destination buffer.
 * \param[in,out]   len     Size of the buffer (in), length of the NDEF message (out).
 * \param[out]      error   Set to a user-readable message on failure.
 *
 * \return TRUE on success.
 */
gboolean ptx_ExampleNdef_BuildMessage(GPtrArray *records, uint8_t *buffer, uint32_t *len, GError **error);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */
