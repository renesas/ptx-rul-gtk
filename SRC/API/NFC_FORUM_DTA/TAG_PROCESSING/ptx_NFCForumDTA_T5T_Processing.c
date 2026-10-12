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
    Module      : NFC Forum Device Test Application (DTA) T5T Processing
    File        : ptx_NFCForumDTA_T5T_Processing.c

    Description : Contains the implementation to handle the processing of T5T in the NFC Forum DTA. It controls the
                  processing if the NFC Forum DTA is communicating with a card in T5T Mode.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NFCForumDTA_T5T_Processing.h"

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

ptx_Status_t ptx_NFCForumDTA_ProcessT5T(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
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
            case 0x0001: // FALL THROUGH
            case 0x0011:
            case 0x0021:
            case 0x0031:
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

            case 0x0003:
                // Transition to READ Only
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
    return status;
}

ptx_Status_t ptx_NFCForumDTA_T5TSleepSelect(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
        ptx_NativeTag_T5T_t t5t;
        ptx_NativeTag_T5T_InitParams_t initParams;
        memset(&t5t, 0x00, sizeof(ptx_NativeTag_T5T_t));
        memset(&initParams, 0x00, sizeof(ptx_NativeTag_T5T_InitParams_t));

        uint8_t tx_buf[PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE];
        memset(tx_buf, 0x00, sizeof(uint8_t) * PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE);

        initParams.TxBuffer = tx_buf;
        initParams.TxBufferSize = PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE;
        initParams.rul = dta->rul;

        ptx_NativeTag_T5T_Open(&t5t, &initParams);

        if (2 <= dta->cardRegistry->CardsCnt)
        {
            // Set FWT in order to receive timeout notification on sleep command
            uint32_t rf_ParamLen = 1u;
            ptx_NSC_RfSetParameter_Param_t rf_params[1];
            uint32_t timeout = 0xFF;
            rf_params->Type = NSC_RfSetParameter_Param_Type_Fwt;
            rf_params->Value.Fwt.FwtValue[2] = (uint8_t)((timeout >> 16) & (uint8_t)0xFF);
            rf_params->Value.Fwt.FwtValue[1] = (uint8_t)((timeout >> 8) & (uint8_t)0xFF);
            rf_params->Value.Fwt.FwtValue[0] = (uint8_t)((timeout >> 0) & (uint8_t)0xFF);

            status = ptx_NSC_RfSetParameter(&(dta->rul->Nsc), &rf_params[0], rf_ParamLen);

            uint8_t *uid_ptr = &dta->cardRegistry->Cards[1].TechParam.TypeV.InventoryRes[2];
            uint8_t uid_len = PTX_T5T_UID_LEN;
            uint32_t rx_len = dta->NDEFMessageBufferSize;

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NativeTag_T5T_Sleep(&t5t, 0, uid_ptr, uid_len, dta->NDEFMessageBuffer, &rx_len, PTX_T5T_DEFAULT_TIMEOUT_MS);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                // Reset FWT to default value before continuing
                timeout = 0x00;
                rf_params->Type = NSC_RfSetParameter_Param_Type_Fwt;
                rf_params->Value.Fwt.FwtValue[2] = (uint8_t)((timeout >> 16) & (uint8_t)0xFF);
                rf_params->Value.Fwt.FwtValue[1] = (uint8_t)((timeout >> 8) & (uint8_t)0xFF);
                rf_params->Value.Fwt.FwtValue[0] = (uint8_t)((timeout >> 0) & (uint8_t)0xFF);
                status = ptx_NSC_RfSetParameter(&(dta->rul->Nsc), &rf_params[0], rf_ParamLen);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                uid_ptr = &dta->cardRegistry->Cards[0].TechParam.TypeV.InventoryRes[2];
                rx_len = dta->NDEFMessageBufferSize;
                status = ptx_NativeTag_T5T_Select(&t5t, 0, uid_ptr, uid_len, dta->NDEFMessageBuffer, &rx_len, PTX_T5T_DEFAULT_TIMEOUT_MS);

                if (PTX_STATUS_SUCCESS == status)
                {
                    dta->rul->CardRegistry.ActiveCard = &dta->rul->CardRegistry.Cards[0];
                    dta->rul->CardRegistry.ActiveProtocol = ptx_RUL_CardProtocolType_T5T;
                }
            }
        }

        (void)ptx_NativeTag_T5T_Close(&t5t);
    }

    return status;
}


ptx_Status_t ptx_NFCForumDTA_T5TMode(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg, ptx_NDEF_InitParams_t *ndefInitParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg) && (NULL != ndefInitParams))
    {
        if ((0x0021 == testCfg->PATTERN_NUMBER) || (0x0012 == testCfg->PATTERN_NUMBER) || (0x0001 == testCfg->PATTERN_NUMBER) || (0x0002 == testCfg->PATTERN_NUMBER) || (0x0003 == testCfg->PATTERN_NUMBER))
        {
            ndefInitParams->t5tMode = ptx_NDEF_T5T_Mode_Addressed;
        }
        else if (0x0031 == testCfg->PATTERN_NUMBER)
        {
            ndefInitParams->t5tMode = ptx_NDEF_T5T_Mode_Selected;
        }
        else
        {
            ndefInitParams->t5tMode = ptx_NDEF_T5T_Mode_NonAddressed;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;

}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
