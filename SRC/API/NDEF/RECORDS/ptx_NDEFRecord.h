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
    Module      : NFC Data Exchange Format (NDEF) Record
    File        : ptx_NDEFRecord.h

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse NDEF records.
*/

/**
 * \addtogroup NDEF_RECORD
 * @{
 */

#if !defined(PTX_API_NDEF_RECORDS_RECORD_H_)
#define PTX_API_NDEF_RECORDS_RECORD_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>
#include <string.h>
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
 * \brief Message begin flag
 */
#define PTX_FLAG_MASK_MB    (0x80)

/**
 * \brief Message end flag
 */
#define PTX_FLAG_MASK_ME    (0x40)

/**
 * \brief Chunked flag
 */
#define PTX_FLAG_MASK_CF    (0x20)

/**
 * \brief Short record flag
 */
#define PTX_FLAG_MASK_SR    (0x10)

/**
 * \brief ID length flag
 */
#define PTX_FLAG_MASK_IL    (0x08)

/**
 * \brief Type name field mask
 */
#define PTX_FLAG_MASK_TNF   (0x07)

/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/


/**
 * \brief  NDEF Record Type Name Format Field
 */
typedef enum ptx_NDEFRecord_TNF
{
    TNF_EMPTY           = 0x00,
    TNF_WELL_KNOWN_TYPE = 0x01,
    TNF_MEDIA_TYPE      = 0x02,
    TNF_ABSOLUTE_URI    = 0x03,
    TNF_EXTERNAL_TYPE   = 0x04,
    TNF_UNKNOWN         = 0x05,
    TNF_UNCHANGED       = 0x06
} ptx_NDEFRecord_TNF_t;

/**
 * \brief  NDEF Record Type Field
 */
typedef struct ptx_NDEFRecord_Type
{
    uint8_t Length;                 /**< Type-field length */
    union
    {
        char    *sData;
        uint8_t *bData;
    };
} ptx_NDEFRecord_Type_t;

/**
 * \brief  NDEF Record ID Field
 */
typedef struct ptx_NDEFRecord_Id
{
    uint8_t Length;                 /**< ID-field length */
    uint8_t *Data;                  /**< ID-field value */
} ptx_NDEFRecord_Id_t;

/**
 * \brief  NDEF Record Payload Field
 */
typedef struct ptx_NDEFRecord_Payload
{
    uint32_t Length;                /**< Payload-field length */
    uint8_t *Data;                  /**< Payload-field value */
} ptx_NDEFRecord_Payload_t;

/**
 * \brief NDEF Record Header Flags
 */
typedef struct ptx_NDEFRecord_HeaderFlags
{
    uint8_t MB;    /**< Message Begin Flag */
    uint8_t ME;    /**< Message End Flag */
    uint8_t CF;    /**< Chunk Flag */
    uint8_t SR;    /**< Short Record Flag */
    uint8_t IL;    /**< ID Length Flag */
    uint8_t TNF;   /**< Type Name Format Field */
} ptx_NDEFRecord_HeaderFlags_t;

/**
 * \brief Generic NDEF Record container struct.
 */
typedef struct ptx_NDEFRecord
{
    ptx_NDEFRecord_HeaderFlags_t HeaderFlags;    /**< Record Header Byte */
    ptx_NDEFRecord_Type_t        Type;           /**< Type Field */
    ptx_NDEFRecord_Id_t          ID;             /**< ID Field */
    ptx_NDEFRecord_Payload_t     Payload;        /**< Payload Field */
} ptx_NDEFRecord_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Setter for NDEF record message begin flag.
 *
 * \param[in]   record  Pointer to the NDEF record container.
 * \param[in]   MB      Enable/Disable message begin flag.
 *
 * \return void
 */
static inline void ptx_NDEFRecord_SetMB (ptx_NDEFRecord_t *record, uint8_t MB)
{
    record->HeaderFlags.MB = (MB > 0 ? 1 : 0);
}

/**
 * \brief Setter for NDEF record message end flag.
 *
 * \param[in]   record  Pointer to the NDEF record container.
 * \param[in]   ME      Enable/Disable message end flag.
 *
 * \return void
 */
static inline void ptx_NDEFRecord_SetME (ptx_NDEFRecord_t *record, uint8_t ME)
{
    record->HeaderFlags.ME = (ME > 0 ? 1 : 0);
}

/**
 * \brief Setter for NDEF record chunked flag.
 *
 * \param[in]   record  Pointer to the NDEF record container.
 * \param[in]   CF      Enable/Disable chunked flag.
 *
 * \return void
 */
static inline void ptx_NDEFRecord_SetCF(ptx_NDEFRecord_t *record, uint8_t CF)
{
    record->HeaderFlags.CF = (CF > 0 ? 1 : 0);
}

/**
 * \brief Setter for NDEF record short record flag.
 *
 * \param[in]   record  Pointer to the NDEF record container.
 * \param[in]   SR      Enable/Disable short record flag.
 *
 * \return void
 */
static inline void ptx_NDEFRecord_SetSR(ptx_NDEFRecord_t *record, uint8_t SR)
{
    record->HeaderFlags.SR = (SR > 0 ? 1 : 0);
}

/**
 * \brief Setter for NDEF record ID length flag.
 *
 * \param[in]   record  Pointer to the NDEF record container.
 * \param[in]   MB      Enable/Disable ID length flag.
 *
 * \return void
 */
static inline void ptx_NDEFRecord_SetIL(ptx_NDEFRecord_t *record, uint8_t IL)
{
    record->HeaderFlags.IL = (IL > 0 ? 1 : 0);
}

/**
 * \brief Setter for NDEF record TNF byte.
 *
 * \param[in]   record  Pointer to the NDEF record container.
 * \param[in]   TNF     TNF byte to be written to the record
 *
 * \return void
 */
static inline void ptx_NDEFRecord_SetTNF(ptx_NDEFRecord_t *record, uint8_t TNF)
{
    record->HeaderFlags.TNF = TNF & PTX_FLAG_MASK_TNF;
}

/**
 * \brief Initializes NDEF record container.
 *
 * \param[in]   record  Pointer to the NDEF record container to be initialized.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Init(ptx_NDEFRecord_t *record);

/**
 * \brief Creates an NDEF record container from the given parameters.
 *
 * \param[in,out]   record      Pointer to an uninitialized NDEF record structure.
 * \param[in]       tnf         Type Name Format Field.
 * \param[in]       type        Pointer to the NDEF record type.
 * \param[in]       typeLen     Record type length.
 * \param[in]       id          Pointer to the NDEF Record ID.
 * \param[in]       idLen       Record ID length.
 * \param[in]       payload     Pointer to the payload data.
 * \param[in]       payloadLen  Payload data length.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Create (ptx_NDEFRecord_t *record, ptx_NDEFRecord_TNF_t tnf, uint8_t *type,
                                    uint8_t typeLen,
                                    uint8_t *id,
                                    uint8_t idLen,
                                    uint8_t *payload,
                                    uint32_t payloadLen);

/**
 * \brief Prepares the ptx_NDEFRecord_t struct to create an empty NDEF record.
 *
 * \param[in,out]   record  Pointer to the NDEF record container to be initialized as empty record.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_CreateEmptyRecord (ptx_NDEFRecord_t *record);

/**
 * \brief Writes a ptx_NDEFRecord_t struct to a given buffer.
 *
 * \param[in]       ndefRecord      Initialized NDEF record.
 * \param[in]       dstBuffer       Pointer to the buffer, which will contain the written data.
 * \param[in,out]   dstBufferSize   Length of the provided buffer.
 * \param[in,out]   dataWritten     Valid pointer to an integer, that will contain the actual amount of written data.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Write(ptx_NDEFRecord_t *ndefRecord, uint8_t *dstBuffer, uint32_t dstBufferSize, uint32_t *dataWritten);

/**
 * \brief Parses a NDEF record from a given buffer.
 *
 * \param[in]       srcBuffer   Buffer to parse the data from.
 * \param[in]       bufferLen   Length of the provided buffer.
 * \param[in,out]   record      Pointer to an ptx_NDEFRecord_t struct which will be initialized with the parsed data.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFRecord_Parse(uint8_t *srcBuffer, uint32_t bufferLen, ptx_NDEFRecord_t *record);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
