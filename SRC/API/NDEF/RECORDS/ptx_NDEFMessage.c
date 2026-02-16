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
    Module      : NFC Data Exchange Format (NDEF) Message
    File        : ptx_NDEFMessage.c

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse NDEF messages.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NDEFMessage.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NDEFMessage_Parse (uint8_t *srcBuffer, uint32_t bufferLen, ptx_NDEFRecord_t *recordBuffer, uint32_t *recordLen, uint8_t ignoreMBME)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != srcBuffer) && (NULL != recordBuffer) && (NULL != recordLen) )
    {
        uint32_t record_len_cpy = *recordLen;
        uint8_t  record_num = 0;
        uint8_t *src_buffer_cpy = srcBuffer;
        uint8_t *src_buffer_end = (uint8_t*)(src_buffer_cpy + bufferLen);
        uint8_t  is_chunked = 0;
        uint8_t  me = 0;

        while (!me && (src_buffer_cpy < src_buffer_end))
        {
            if (record_len_cpy <= record_num)
            {
                status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
                break;
            }

            uint8_t header_byte = *src_buffer_cpy;
            src_buffer_cpy++;

            uint8_t mb = header_byte & PTX_FLAG_MASK_MB;
            me = header_byte & PTX_FLAG_MASK_ME;
            uint8_t cf = header_byte & PTX_FLAG_MASK_CF;
            uint8_t sr = header_byte & PTX_FLAG_MASK_SR;
            uint8_t il = header_byte & PTX_FLAG_MASK_IL;
            uint8_t tnf = header_byte & PTX_FLAG_MASK_TNF;

            if ( (0 == record_num) && !mb && !ignoreMBME)
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
                break;
            }

            if ( (0 != record_num) && mb && !ignoreMBME)
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
                break;
            }

            if (me && cf && mb && !ignoreMBME)
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
                break;
            }

            if (!is_chunked && cf && (TNF_UNCHANGED == tnf) )
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
                break;
            }

            if (is_chunked && (TNF_UNCHANGED != tnf) )
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
                break;
            }

            if (is_chunked && il)
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
                break;
            }

            ptx_NDEFRecord_SetMB(&recordBuffer[record_num], mb);
            ptx_NDEFRecord_SetME(&recordBuffer[record_num], me);
            ptx_NDEFRecord_SetCF(&recordBuffer[record_num], cf);
            ptx_NDEFRecord_SetSR(&recordBuffer[record_num], sr);
            ptx_NDEFRecord_SetIL(&recordBuffer[record_num], il);
            ptx_NDEFRecord_SetTNF(&recordBuffer[record_num], tnf);

            recordBuffer[record_num].Type.Length = (*src_buffer_cpy) & 0xFF;
            src_buffer_cpy++;

            if (is_chunked && (0 != recordBuffer[record_num].Type.Length) )
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
                break;
            }

            uint32_t payload_length = 0;
            if (recordBuffer[record_num].HeaderFlags.SR)
            {
                payload_length = *src_buffer_cpy & 0xFF;
                src_buffer_cpy++;
            }
            else
            {
                payload_length = *((uint32_t*)src_buffer_cpy);
                src_buffer_cpy += 4;
            }

            recordBuffer[record_num].Payload.Length = payload_length;

            if (recordBuffer[record_num].HeaderFlags.IL)
            {
                recordBuffer[record_num].ID.Length = *src_buffer_cpy;
                src_buffer_cpy++;
            }
            else
            {
                recordBuffer[record_num].ID.Length = 0;
            }

            is_chunked = cf;

            if (!is_chunked)
            {
                if (0 != recordBuffer[record_num].Type.Length)
                {
                    recordBuffer[record_num].Type.bData = src_buffer_cpy;
                    src_buffer_cpy += recordBuffer[record_num].Type.Length;
                }
                else
                {
                    recordBuffer[record_num].Type.bData = NULL;
                }

                if (0 != recordBuffer[record_num].ID.Length)
                {
                    recordBuffer[record_num].ID.Data = src_buffer_cpy;
                    src_buffer_cpy += recordBuffer[record_num].ID.Length;
                }
                else
                {
                    recordBuffer[record_num].ID.Data = NULL;
                }
            }

            recordBuffer[record_num].Payload.Data = src_buffer_cpy;
            src_buffer_cpy += recordBuffer[record_num].Payload.Length;
            record_num++;

            if (me || (src_buffer_cpy >= src_buffer_end))
            {
                status = PTX_STATUS_SUCCESS;
            }
        }

        uint32_t read_len = (uint32_t)(src_buffer_cpy - srcBuffer);
        if (read_len < bufferLen)
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }

        *recordLen = record_num;
    }

    return status;
}

ptx_Status_t ptx_NDEFMessage_Create (ptx_NDEFRecord_t *recordBuffer, uint32_t recordSize, uint8_t *dstBuffer, uint32_t *bufferLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != recordBuffer) && (NULL != dstBuffer) && (NULL != bufferLen) )
    {
        uint32_t record_num = 0;
        uint32_t data_written = 0;
        uint8_t *buffer_cpy = dstBuffer;
        uint32_t buffer_len_copy = *bufferLen;

        if (record_num >= recordSize)
        {
            status = PTX_STATUS_SUCCESS;
        }

        while (record_num < recordSize)
        {
            if (0 == record_num)
            {
                ptx_NDEFRecord_SetMB(&recordBuffer[record_num], 1);
            }
            if (recordSize == (record_num + 1))
            {
                ptx_NDEFRecord_SetME(&recordBuffer[record_num], 1);
            }

            uint32_t record_size;
            status = ptx_NDEFRecord_Write(&recordBuffer[record_num], &buffer_cpy[data_written], buffer_len_copy - data_written, &record_size);
            if (PTX_STATUS_SUCCESS != status)
            {
                break;
            }

            data_written += record_size;
            record_num++;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            *bufferLen = data_written;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFMessage_FilterRecordsByType (ptx_NDEFRecord_t *recordBuffer, uint32_t *recordsLen, char* type, uint8_t typeLen, uint32_t *foundRecords)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != recordBuffer) && (NULL != recordsLen) && (NULL != type) && (NULL != foundRecords) )
    {
        uint32_t num_found_records = 0;
        for (uint32_t i = 0; i < *recordsLen; ++i)
        {
            if ( (recordBuffer[i].Type.Length == typeLen) && (0 == strncmp(recordBuffer[i].Type.sData, type, typeLen)) )
            {
                foundRecords[num_found_records] = i;
                num_found_records++;
            }
        }
        *recordsLen = num_found_records;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}
