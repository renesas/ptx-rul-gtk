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
    Module      : NFC Forum Device Test Application (DTA) T4T Processing
    File        : ptx_NFCForumDTA_T4T_Processing.c

    Description : Contains the implementation to handle the processing of T4T in the NFC Forum DTA. It controls the
                  processing if the NFC Forum DTA is communicating with a card in T4T Mode.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NFCForumDTA_T4T_Processing.h"
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NFCFORUMDTA_T4T_LOOPBACK_TIMEOUT       (50000u)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NFCForumDTA_ISODEP_Loopback(ptx_NFCForumDTA_t *dta);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NFCForumDTA_ProcessT4T(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
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
            case 0x0000:
                status = ptx_NFCForumDTA_ISODEP_Loopback(dta);
                break;

            case 0x0001: // FALL THROUGH
            case 0x0011:
                // Read NDEF Message
                memset(dta->NDEFMessageBuffer, 0x00, sizeof(uint8_t) * dta->NDEFMessageBufferSize);
                status = ptx_NDEF_CheckMessage(&dta->ndef);
                if (PTX_STATUS_SUCCESS == status)
                {
                    *dta->NDEFMessageBufferLen = dta->NDEFMessageBufferSize;
                    status = ptx_NDEF_ReadMessage(&dta->ndef, dta->NDEFMessageBuffer, dta->NDEFMessageBufferLen);
                }
                break;

            case 0x0002: // FALL THROUGH
            case 0x0012:
                // Write NDEF Message
                status = ptx_NDEF_CheckMessage(&dta->ndef);
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_WriteMessage(&dta->ndef, dta->NDEFMessageBuffer, *dta->NDEFMessageBufferLen);
                }
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NFCForumDTA_ISODEP_Loopback(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
        ptx_NFCForumDTA_ISODEP_Loopback_States_t loopbackState = ptx_NFCForumDTA_ISODEP_LB_State_SEND_SOT;

        uint8_t *rx = dta->NDEFMessageBuffer;
        uint32_t rxLen = dta->NDEFMessageBufferSize;

        uint8_t SOT_CMD[] = {0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x4E, 0x46, 0x43, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};
        uint32_t SOT_CMD_LEN = 20;

        uint8_t DTA_RES_EOT[] = {0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x90, 0x00};

        status = PTX_STATUS_SUCCESS;

        while ((PTX_STATUS_SUCCESS == status) && (ptx_NFCForumDTA_ISODEP_LB_State_EXIT != loopbackState))
        {
            switch (loopbackState)
            {
                case ptx_NFCForumDTA_ISODEP_LB_State_SEND_SOT:
                    status = ptx_RUL_Exchange(dta->rul, SOT_CMD, SOT_CMD_LEN, rx, &rxLen, PTX_NFCFORUMDTA_T4T_LOOPBACK_TIMEOUT);
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        loopbackState = ptx_NFCForumDTA_ISODEP_LB_State_CHECK_RX_DATA;
                    }
                    else
                    {
                        loopbackState = ptx_NFCForumDTA_ISODEP_LB_State_ERROR;
                    }
                    break;

                case ptx_NFCForumDTA_ISODEP_LB_State_SEND_APP_DATA:
                    if (0 != (rxLen - 2))
                    {
                        uint8_t *tx = rx;
                        uint32_t txLen = rxLen - 2u;

                        rxLen = dta->NDEFMessageBufferSize;
                        status = ptx_RUL_Exchange(dta->rul, tx, txLen, rx, &rxLen, PTX_NFCFORUMDTA_T4T_LOOPBACK_TIMEOUT);
                        if (PTX_STATUS_SUCCESS == status)
                        {
                            loopbackState = ptx_NFCForumDTA_ISODEP_LB_State_CHECK_RX_DATA;
                        }
                        else
                        {
                            loopbackState = ptx_NFCForumDTA_ISODEP_LB_State_ERROR;
                        }
                    }
                    else
                    {
                        loopbackState = ptx_NFCForumDTA_ISODEP_LB_State_ERROR;
                    }

                    break;

                case ptx_NFCForumDTA_ISODEP_LB_State_CHECK_RX_DATA:
                    if (0 != rxLen)
                    {
                        if (0 == memcmp(rx, DTA_RES_EOT, rxLen))
                        {
                            loopbackState = ptx_NFCForumDTA_ISODEP_LB_State_EXIT;
                        }
                        else
                        {
                            loopbackState = ptx_NFCForumDTA_ISODEP_LB_State_SEND_APP_DATA;
                        }
                    }
                    else
                    {
                        loopbackState = ptx_NFCForumDTA_ISODEP_LB_State_ERROR;
                    }
                    break;

                case ptx_NFCForumDTA_ISODEP_LB_State_ERROR:
                    loopbackState = ptx_NFCForumDTA_ISODEP_LB_State_EXIT;
                    break;

                case ptx_NFCForumDTA_ISODEP_LB_State_EXIT:
                    break;

                default:
                    break;
            }

            if (NULL != dta->applicationCallback)
            {
                dta->applicationCallback(dta);
            }
        }
    }

    return status;
}
