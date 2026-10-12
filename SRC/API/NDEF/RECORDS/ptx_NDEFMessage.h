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
    Module      : NFC Data Exchange Format (NDEF) Message
    File        : ptx_NDEFMessage.h

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse NDEF messages.
*/

/**
 * \addtogroup NDEF_MESSAGE
 * @{
 */

#if !defined(PTX_API_NDEF_MESSAGE_H_)
#define PTX_API_NDEF_MESSAGE_H_

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
 * \brief Parses an NDEF Message from a buffer, into its individual NDEF records.
 *
 * \param[in]       srcBuffer       Pointer to the buffer to parse the NDEF massage from.
 * \param[in]       bufferLen       Length of the buffer that needs to be parsed.
 * \param[out]      recordBuffer    Buffer of NDEF records to store the parsed data.
 * \param[in,out]   recordLen       Length of the NDEF record buffer.
 * \param[in]       ignoreMBME      Ignore message begin and message end flags during parsing.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFMessage_Parse (uint8_t *srcBuffer, uint32_t bufferLen, ptx_NDEFRecord_t *recordBuffer, uint32_t *recordLen, uint8_t ignoreMBME);

/**
 * \brief Creates an NDEF Message from individual NDEF records and stores it in a buffer.
 *
 * \param[in]       recordBuffer    Pointer to an existing (and initialized) NDEF record buffer array.
 * \param[in]       recordSize      Size of the NDEF record buffer.
 * \param[in]       dstBuffer       Pointer to an existing buffer, where the data gets written to.
 * \param[in,out]   bufferLen       Pointer to an integer describing the available 'dstBuffer' length (in). Actual written length (out).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFMessage_Create (ptx_NDEFRecord_t *recordBuffer, uint32_t recordSize, uint8_t *dstBuffer, uint32_t *bufferLen);

/**
 * \brief Filters a given array of NDEF Records by its record type.
 *
 * \param[in]   recordBuffer    Pointer to an existing NDEF record buffer array.
 * \param[in]   recordsLen      Amount of records to filter.
 * \param[in]   type            Pointer to the record type to search for.
 * \param[in]   typeLen         Length of the given record type.
 * \param[in]   foundRecords    Pointer to an existing array, to store the IDs of the found/filtered records.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_NDEFMessage_FilterRecordsByType (ptx_NDEFRecord_t *recordBuffer, uint32_t *recordsLen, char* type, uint8_t typeLen, uint32_t *foundRecords);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
