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
    Module      : NFC Forum Device Test Application (DTA) Data Exchange Handling
    File        : ptx_NFCForumDTA_DataExchange.c

    Description : Contains the implementation to handle data exchange in the NFC Forum DTA. It controls the communication
                  between the NFC Forum DTA and a card.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NFCForumDTA_DataExchange.h"

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

static ptx_Status_t ptx_NFCForumDTA_ProcessPollDataExchangeNDEF(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NFCForumDTA_ProcessPollDataExchange(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && NULL != testCfg)
    {

        if (ptx_RUL_CardProtocolType_NfcDep == dta->cardRegistry->ActiveProtocol)
        {
            status = ptx_NFCForumDTA_ProcessNFCDEP(dta, testCfg);
        }
        else
        {
            status = ptx_NFCForumDTA_ProcessPollDataExchangeNDEF(dta, testCfg);
        }

    }

    return status;
}

#if !defined(PTX_DISABLE_LISTEN_MODE)
ptx_Status_t ptx_NFCForumDTA_ProcessListenEvents(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (PTX_COMP_CHECK(&dta->hce, Comp_HCE)) && (NULL != testCfg))
    {

        uint8_t deactivated = 0;
        uint8_t no_event_count_deact = 0;
        uint8_t rf_protocol = dta->hce.ActiveProtocol;
        status = PTX_STATUS_SUCCESS;

        while (((PTX_STATUS_SUCCESS == status) || (PTX_STATUS_E_GEN_NOT_FOUND == status)) && (0xFF > no_event_count_deact) && (!dta->quitDTA))
        {
            ptx_RUL_HCE_EventRecord_t *record;
            status = ptx_RUL_HCE_GetEvent(&dta->hce, &record);

            if ((PTX_STATUS_SUCCESS == status) && (RUL_HCE_EventType_NoEvent != record->EventID))
            {
                switch (record->EventID)
                {
                    case RUL_HCE_EventType_ExtFieldOn:  // FALL THROUGH
                    case RUL_HCE_EventType_ExtFieldOff: // FALL THROUGH
                        status = PTX_STATUS_SUCCESS;
                        break;

                    case RUL_HCE_EventType_Activated:
                        rf_protocol = dta->hce.ActiveProtocol;
                        status = PTX_STATUS_SUCCESS;
                        break;

                    case RUL_HCE_EventType_Data:
                    {
                        status = ptx_NFCForumDTA_Listen_Processing_CommandProcess(dta, rf_protocol, record->RxMsgData, record->RxMsgDataLen, testCfg);
                        break;
                    }

                    case RUL_HCE_EventType_Deactivated:
                    {
                        deactivated = 1;
                        switch (record->RxMsgDataShortMsgData[0])
                        {
                            case ptx_NSC_RfDeactivate_Reason_Deselect:
                                // Already in listen sleep state, continue until new Activate command is received
                                dta->State = ptx_NFCForumDTA_State_DEACTIVATE_DISCOVER;
                                break;
                            case ptx_NSC_RfDeactivate_Reason_RF_Field:
                                if (ptx_NSC_RfProtocol_T3T == rf_protocol)
                                {
                                    dta->State = ptx_NFCForumDTA_State_WAIT_4_ACTIVATION;
                                }
                                else
                                {
                                    dta->State = ptx_NFCForumDTA_State_DEACTIVATE_DISCOVER;
                                }
                                break;
                            case ptx_NSC_RfDeactivate_Reason_Release:
                                dta->State = ptx_NFCForumDTA_State_WAIT_4_ACTIVATION;
                                break;

                            default:
                                dta->State = ptx_NFCForumDTA_State_DEACTIVATE_DISCOVER;
                                break;
                        }

                        status = PTX_STATUS_SUCCESS;
                        break;
                    }

                    default:
                        status = PTX_STATUS_E_GEN_INVALID_STATE;
                        break;
                }
            }

            if ((0 != deactivated) && (RUL_HCE_EventType_NoEvent == record->EventID))
            {
                no_event_count_deact++;
            }


            if (NULL != dta->applicationCallback)
            {
                dta->applicationCallback(dta);
            }

        }


        if ((PTX_STATUS_SUCCESS == status) && (!deactivated))
        {
            dta->State = ptx_NFCForumDTA_State_DEACTIVATE_IDLE;
        }
        else if (PTX_STATUS_E_GEN_NOT_IMPLEMENTED == status)
        {
            dta->State = ptx_NFCForumDTA_State_DEACTIVATE_DISCOVER;
            status = PTX_STATUS_SUCCESS;
        }


    }

    return status;
}
#endif // PTX_DISABLE_LISTEN_MODE



/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NFCForumDTA_ProcessPollDataExchangeNDEF(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && NULL != testCfg)
    {
        ptx_NDEF_InitParams_t ndefInitParams;
        memset(&ndefInitParams, 0, sizeof(ptx_NDEF_InitParams_t));

        uint8_t tx_buf[PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE];
        memset(tx_buf, 0, sizeof(uint8_t) * PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE);

        ndefInitParams.tx = tx_buf;
        ndefInitParams.txLen = PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE;

        uint8_t rx_buf[PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE];
        memset(rx_buf, 0, sizeof(uint8_t) * PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE);

        ndefInitParams.rx = rx_buf;
        ndefInitParams.rxLen = PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE;

        uint8_t work_buf[PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE];
        memset(work_buf, 0, sizeof(uint8_t) * PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE);

        ndefInitParams.workBuffer = work_buf;
        ndefInitParams.workBufferLen = PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE;

        ndefInitParams.rul = dta->rul;


        if (ptx_RUL_CardProtocolType_T5T == dta->cardRegistry->ActiveProtocol)
        {
            status = ptx_NFCForumDTA_T5TMode(dta, testCfg, &ndefInitParams);
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }


        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NDEF_Open(&dta->ndef, &ndefInitParams);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (dta->cardRegistry->ActiveProtocol)
            {
                case ptx_RUL_CardProtocolType_T2T:
                    status = ptx_NFCForumDTA_ProcessT2T(dta, testCfg);
                    break;
                case ptx_RUL_CardProtocolType_T3T:
                    status = ptx_NFCForumDTA_ProcessT3T(dta, testCfg);
                    break;
                case ptx_RUL_CardProtocolType_T4T:
                    status = ptx_NFCForumDTA_ProcessT4T(dta, testCfg);
                    break;
                case ptx_RUL_CardProtocolType_T5T:
                    status = ptx_NFCForumDTA_ProcessT5T(dta, testCfg);
                    break;
                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NDEF_Close(&dta->ndef);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (dta->cardRegistry->ActiveProtocol)
            {
                case ptx_RUL_CardProtocolType_T2T: /* FALL THROUGH */
                case ptx_RUL_CardProtocolType_T3T: /* FALL THROUGH */
                case ptx_RUL_CardProtocolType_T5T:
                    dta->State = ptx_NFCForumDTA_State_DEACTIVATE_IDLE;
                    break;
                case ptx_RUL_CardProtocolType_T4T:
                    dta->State = ptx_NFCForumDTA_State_DEACTIVATE_SLEEP;
                    break;
                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }
        }
    }

    return status;
}

