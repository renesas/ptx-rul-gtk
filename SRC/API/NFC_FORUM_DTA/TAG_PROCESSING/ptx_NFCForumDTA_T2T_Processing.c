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
    Module      : NFC Forum Device Test Application (DTA) T2T Processing
    File        : ptx_NFCForumDTA_T2T_Processing.c

    Description : Contains the implementation to handle the processing of T2T in the NFC Forum DTA. It controls the
                  processing if the NFC Forum DTA is communicating with a card in T2T Mode.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NFCForumDTA_T2T_Processing.h"
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NFCForumDTA_ProcessT2T(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        status = PTX_STATUS_SUCCESS;
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        switch (testCfg->PATTERN_NUMBER)
        {
            case 0x0001:
                // Read NDEF Message
                memset(dta->NDEFMessageBuffer, 0x00, sizeof(uint8_t) * dta->NDEFMessageBufferSize);
                status = ptx_NDEF_CheckMessage(&dta->ndef);
                if (PTX_STATUS_SUCCESS == status)
                {
                    *dta->NDEFMessageBufferLen = dta->NDEFMessageBufferSize;
                    status = ptx_NDEF_ReadMessage(&dta->ndef, dta->NDEFMessageBuffer, dta->NDEFMessageBufferLen);
                }
                break;
            case 0x0002:
                // Write NDEF Message
                status = ptx_NDEF_CheckMessage(&dta->ndef);
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_WriteMessage(&dta->ndef, dta->NDEFMessageBuffer, *dta->NDEFMessageBufferLen);
                }
                break;
            case 0x0003:
                // Lock NDEF Tag
                status = ptx_NDEF_CheckMessage(&dta->ndef);
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_LockTag(&dta->ndef);
                }
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t SLP_REQ[2] = {0x50, 0x00};
        uint8_t res[2] = {0};
        uint32_t resLen = 2;
        status = ptx_RUL_Exchange(dta->rul, SLP_REQ, 2, res, &resLen, 10);
    }


    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
