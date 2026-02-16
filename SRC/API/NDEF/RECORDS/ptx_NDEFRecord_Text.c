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
    Module      : NFC Data Exchange Format (NDEF) Text Record
    File        : ptx_NDEFRecord_Text.c

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Text NDEF records.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NDEFRecord_Text.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NDEFRecord_Text_Create (ptx_NDEFRecord_t *record, uint8_t *workBuffer, uint32_t workBufferLen, char *languageCode, char *text)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != record) && (NULL != workBuffer) && (NULL != languageCode) && (NULL != text) )
    {
        uint32_t lang_code_len = (uint32_t)strlen(languageCode);
        uint32_t text_len = (uint32_t)strlen(text);
        uint32_t total_len = 1 + lang_code_len + text_len;

        if (workBufferLen >= total_len)
        {
            workBuffer[0] = 0; // ASCII only
            workBuffer[0] = (uint8_t) (workBuffer[0] | (uint8_t) (lang_code_len & 0x3F));

            strcpy((char *) & (workBuffer[1]), languageCode);
            strcpy((char *) & (workBuffer[1 + lang_code_len]), text);
            status = ptx_NDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) PTX_NDEFRECORD_TYPE_RTD_TEXT, PTX_NDEFRECORD_TYPE_RTD_TEXT_LEN, NULL, 0, workBuffer, total_len);
        }
        else
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}
