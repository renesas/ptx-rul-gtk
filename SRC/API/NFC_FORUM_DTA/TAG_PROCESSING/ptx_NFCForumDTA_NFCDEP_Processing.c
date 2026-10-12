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
    Module      : NFC Forum Device Test Application (DTA) NFC-DEP Processing
    File        : ptx_NFCForumDTA_NFCDEP_Processing.c

    Description : Contains the implementation to handle the processing of NFC-DEP in the NFC Forum DTA. It controls the
                  processing if the NFC Forum DTA is communicating with a card in NFC-DEP Mode.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NFCForumDTA_NFCDEP_Processing.h"
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NFCFORUMDTA_NFCDEP_LOOPBACK_TIMEOUT       (50000u)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NFCForumDTA_NFCDEP_Loopback(ptx_NFCForumDTA_t *dta);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NFCForumDTA_ProcessNFCDEP(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        status = PTX_STATUS_SUCCESS;
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_NFCForumDTA_NFCDEP_Loopback(dta);
        // switch (testCfg->PATTERN_NUMBER)
        // {
        //     case 0x0000:

        //         break;

        //     default:
        //         status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        //         break;
        // }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NFCForumDTA_NFCDEP_Loopback(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
        ptx_NFCForumDTA_NFCDEP_Loopback_States_t loopbackState = ptx_NFCForumDTA_NFCDEP_LB_State_SEND_SOT;

        uint8_t *rx = dta->NDEFMessageBuffer;
        uint32_t rxLen = dta->NDEFMessageBufferSize;

        uint8_t SOT_CMD[] = {0x00, 0x40, 0x00, 0x01, 0x10, 0x02, 0x01, 0x0E};
        uint32_t SOT_CMD_LEN = 8;

        uint8_t DTA_RES_EOT_DSL[] = {0xFF, 0xFF, 0xFF, 0x01, 0x01};
        uint8_t DTA_RES_EOT_RLS[] = {0xFF, 0xFF, 0xFF, 0x01, 0x02};

        status = PTX_STATUS_SUCCESS;

        while ((PTX_STATUS_SUCCESS == status) && (ptx_NFCForumDTA_State_POLL_DATA_EXCHANGE == dta->State))
        {
            switch (loopbackState)
            {
                case ptx_NFCForumDTA_NFCDEP_LB_State_SEND_SOT:
                    status = ptx_RUL_Exchange(dta->rul, SOT_CMD, SOT_CMD_LEN, rx, &rxLen, PTX_NFCFORUMDTA_NFCDEP_LOOPBACK_TIMEOUT);
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        loopbackState = ptx_NFCForumDTA_NFCDEP_LB_State_CHECK_RX_DATA;
                    }
                    else
                    {
                        loopbackState = ptx_NFCForumDTA_NFCDEP_LB_State_ERROR;
                    }
                    break;

                case ptx_NFCForumDTA_NFCDEP_LB_State_SEND_APP_DATA:
                    if (0 != (rxLen - 2))
                    {
                        uint8_t *tx = rx;
                        uint32_t txLen = rxLen;

                        rxLen = dta->NDEFMessageBufferSize;
                        status = ptx_RUL_Exchange(dta->rul, tx, txLen, rx, &rxLen, PTX_NFCFORUMDTA_NFCDEP_LOOPBACK_TIMEOUT);
                        if (PTX_STATUS_SUCCESS == status)
                        {
                            loopbackState = ptx_NFCForumDTA_NFCDEP_LB_State_CHECK_RX_DATA;
                        }
                        else
                        {
                            loopbackState = ptx_NFCForumDTA_NFCDEP_LB_State_ERROR;
                        }
                    }
                    else
                    {
                        loopbackState = ptx_NFCForumDTA_NFCDEP_LB_State_ERROR;
                    }

                    break;

                case ptx_NFCForumDTA_NFCDEP_LB_State_CHECK_RX_DATA:
                    if (0 != rxLen)
                    {
                        if (0 == memcmp(rx, DTA_RES_EOT_DSL, rxLen))
                        {
                            loopbackState = ptx_NFCForumDTA_NFCDEP_LB_State_EXIT_DSL;
                        }
                        else if (0 == memcmp(rx, DTA_RES_EOT_RLS, rxLen))
                        {
                            loopbackState = ptx_NFCForumDTA_NFCDEP_LB_State_EXIT_RLS;
                        }
                        else
                        {
                            loopbackState = ptx_NFCForumDTA_NFCDEP_LB_State_SEND_APP_DATA;
                        }
                    }
                    else
                    {
                        loopbackState = ptx_NFCForumDTA_NFCDEP_LB_State_ERROR;
                    }
                    break;

                case ptx_NFCForumDTA_NFCDEP_LB_State_ERROR:
                    dta->State = ptx_NFCForumDTA_State_DEACTIVATE_IDLE;
                    break;

                case ptx_NFCForumDTA_NFCDEP_LB_State_EXIT_DSL:
                    dta->State = ptx_NFCForumDTA_State_DEACTIVATE_DESLECT;
                    break;
                case ptx_NFCForumDTA_NFCDEP_LB_State_EXIT_RLS:
                    dta->State = ptx_NFCForumDTA_State_DEACTIVATE_RELEASE;
                    break;

                default:
                    dta->State = ptx_NFCForumDTA_State_DEACTIVATE_IDLE;
                    break;
            }

            if (NULL != dta->applicationCallback)
            {
                dta->applicationCallback(dta);
            }
        }

        if (PTX_STATUS_E_NSC_RFERROR == status)
        {
            if (dta->rul->CardRegistry.ActiveCard->TechType == ptx_RUL_CardRfTechType_ACM &&
                dta->rul->NfcMode == ptx_RUL_StatusInfoMode_P2P_Initiator &&
                dta->rul->RfErrorStatus == ptx_RUL_StatusInfoRfError_Protocol)
            {
                /*For ACM Poll, deactivate command need to be sent once RF Protocol error is detected (5 sec is too much to pass compliance tests)*/
                (void)ptx_RUL_Sleep(dta->rul, 10);

            }
            else
            {
                (void)ptx_RUL_Sleep(dta->rul, 5000);

            }
        }

    }

    return status;
}
