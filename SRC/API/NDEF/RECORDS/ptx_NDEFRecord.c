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
    Module      : NFC Data Exchange Format (NDEF) Record
    File        : ptx_NDEFRecord.c

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse NDEF records.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NDEFRecord.h"

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / HELPERS
 * ####################################################################################################################
 */

/*
 *  Creates the NDEF header flags byte.
 *
 *    record  Pointer to the NDEF record container.
 *
 * returns Constructed header byte.
 */
static uint8_t ptx_NDEFRecord_ConstructHeaderFlags (ptx_NDEFRecord_t *record);

/*
 *  Calculated the required buffer size of an NDEF record.
 *
 *    record  Pointer to the NDEF record.
 *
 * returns Calculated size of record when written to a buffer.
 */
static uint32_t ptx_NDEFRecord_CalcReqBufferSize(ptx_NDEFRecord_t *ndefRecord);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NDEFRecord_Init(ptx_NDEFRecord_t *record)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != record)
    {
        memset(record, 0, sizeof(ptx_NDEFRecord_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}


ptx_Status_t ptx_NDEFRecord_Create (ptx_NDEFRecord_t *record, ptx_NDEFRecord_TNF_t tnf, uint8_t *type,
                                    uint8_t typeLen,
                                    uint8_t *id,
                                    uint8_t idLen,
                                    uint8_t *payload,
                                    uint32_t payloadLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != record)
    {
        record->HeaderFlags.TNF = (tnf & PTX_FLAG_MASK_TNF);

        if (NULL == type)
        {
            record->Type.Length = 0;
        }
        else
        {
            record->Type.bData = type;
            record->Type.Length = typeLen;
        }

        if (NULL == id)
        {
            record->ID.Length = 0;
        }
        else
        {
            record->ID.Data = id;
            record->ID.Length = idLen;
        }
        ptx_NDEFRecord_SetIL(record, idLen);

        if (NULL == payload)
        {
            record->Payload.Length = 0;
        }
        else
        {
            record->Payload.Data = payload;
            record->Payload.Length = payloadLen;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_CreateEmptyRecord (ptx_NDEFRecord_t *record)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INTERNAL;

    status = ptx_NDEFRecord_Init(record);

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_NDEFRecord_Create(record, TNF_EMPTY, NULL, 0u, NULL, 0u, NULL, 0);
    }

    return status;
}

static uint8_t ptx_NDEFRecord_ConstructHeaderFlags (ptx_NDEFRecord_t *record)
{
    uint8_t header = (uint8_t)  (((record->HeaderFlags.MB) ? PTX_FLAG_MASK_MB : 0x00)       |
                                 ((record->HeaderFlags.ME)  ? PTX_FLAG_MASK_ME : 0x00)      |
                                 ((record->HeaderFlags.CF)  ? PTX_FLAG_MASK_CF : 0x00)      |
                                 ((record->Payload.Length <= 255) ? PTX_FLAG_MASK_SR : 0x00) |
                                 ((record->ID.Length != 0) ? PTX_FLAG_MASK_IL : 0x00)       |
                                 (record->HeaderFlags.TNF & PTX_FLAG_MASK_TNF));
    return header;
}

static uint32_t ptx_NDEFRecord_CalcReqBufferSize(ptx_NDEFRecord_t *ndefRecord)
{
    uint32_t required_len = 0;

    required_len += 1; // HeaderByte
    required_len += 1; // Type.Length
    required_len += 1; // Payload.Length
    if (ndefRecord->HeaderFlags.IL)
    {
        required_len += 1; // Payload.LengthLength
    }
    required_len += ndefRecord->ID.Length;
    required_len += ndefRecord->Type.Length;
    required_len += ndefRecord->Payload.Length;

    return required_len;
}

ptx_Status_t ptx_NDEFRecord_Write(ptx_NDEFRecord_t *ndefRecord, uint8_t *dstBuffer, uint32_t dstBufferSize, uint32_t *dataWritten)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != dstBuffer) && (ptx_NDEFRecord_CalcReqBufferSize(ndefRecord) <= dstBufferSize) )
    {
        uint32_t buffer_prt = 0;
        dstBuffer[buffer_prt] = ptx_NDEFRecord_ConstructHeaderFlags(ndefRecord);
        buffer_prt++;
        dstBuffer[buffer_prt] = ndefRecord->Type.Length;
        buffer_prt++;

        if (255 < ndefRecord->Payload.Length)
        {
            dstBuffer[buffer_prt] = (uint8_t) (ndefRecord->Payload.Length >> 24);
            buffer_prt++;
            dstBuffer[buffer_prt] = (uint8_t) (ndefRecord->Payload.Length >> 16);
            buffer_prt++;
            dstBuffer[buffer_prt] = (uint8_t) (ndefRecord->Payload.Length >>  8);
            buffer_prt++;
            dstBuffer[buffer_prt] = (uint8_t) (ndefRecord->Payload.Length >>  0);
            buffer_prt++;
        }
        else
        {
            dstBuffer[buffer_prt] = (uint8_t) ndefRecord->Payload.Length;
            buffer_prt++;
        }

        if (0 < ndefRecord->HeaderFlags.IL)
        {
            dstBuffer[buffer_prt] = (uint8_t) (ndefRecord->ID.Length);
            buffer_prt++;
        }

        if (0 < ndefRecord->Type.Length)
        {
            memcpy(&dstBuffer[buffer_prt], ndefRecord->Type.bData, ndefRecord->Type.Length);
            buffer_prt += ndefRecord->Type.Length;
        }

        if (0 < ndefRecord->ID.Length)
        {
            memcpy(&dstBuffer[buffer_prt], ndefRecord->ID.Data, ndefRecord->ID.Length);
            buffer_prt += ndefRecord->ID.Length;
        }

        if (0 < ndefRecord->Payload.Length)
        {
            memcpy(&dstBuffer[buffer_prt], ndefRecord->Payload.Data, ndefRecord->Payload.Length);
            buffer_prt += ndefRecord->Payload.Length;
        }

        *dataWritten = buffer_prt;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_Parse(uint8_t *srcBuffer, uint32_t bufferLen, ptx_NDEFRecord_t *record)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != srcBuffer) && (NULL != record) )
    {
        uint8_t *buffer_cpy = srcBuffer;
        uint8_t header_byte = *buffer_cpy;
        buffer_cpy++;

        ptx_NDEFRecord_SetMB(record, header_byte & PTX_FLAG_MASK_MB);
        ptx_NDEFRecord_SetME(record, header_byte & PTX_FLAG_MASK_ME);
        ptx_NDEFRecord_SetCF(record, header_byte & PTX_FLAG_MASK_CF);
        ptx_NDEFRecord_SetSR(record, header_byte & PTX_FLAG_MASK_SR);
        ptx_NDEFRecord_SetIL(record, header_byte & PTX_FLAG_MASK_IL);
        ptx_NDEFRecord_SetTNF(record, header_byte & PTX_FLAG_MASK_TNF);

        record->Type.Length = *buffer_cpy & 0xFF;
        buffer_cpy++;

        if (record->HeaderFlags.SR)
        {
            record->Payload.Length = *buffer_cpy & 0xFF;
            buffer_cpy++;
        }
        else
        {
            record->Payload.Length = *((uint32_t*)buffer_cpy);
            buffer_cpy += 4;
        }

        if (record->HeaderFlags.IL)
        {
            record->ID.Length = *buffer_cpy;
            buffer_cpy++;
        }
        else
        {
            record->ID.Length = 0;
        }

        if (0 != record->Type.Length)
        {
            record->Type.bData = buffer_cpy;
            buffer_cpy += record->Type.Length;
        }
        else
        {
            record->Type.bData = NULL;
        }

        if (0 != record->ID.Length)
        {
            record->ID.Data = buffer_cpy;
            buffer_cpy += record->ID.Length;
        }
        else
        {
            record->ID.Data = NULL;
        }

        record->Payload.Data = buffer_cpy;
        buffer_cpy += record->Payload.Length;

        uint32_t read_len = (uint32_t)(buffer_cpy - srcBuffer);

        if (read_len < bufferLen)
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}
