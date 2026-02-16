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
    Module      : NFC Forum Device Test Application (DTA) Multiple Card Handling
    File        : ptx_NFCForumDTA_Select_Card.c

    Description : Contains the implementation to handle multiple cards in the NFC Forum DTA. It controls the selection
                  and activation of cards.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NFCForumDTA_Select_Card.h"
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

static ptx_Status_t ptx_NFCForumDTA_Process_Type_A_Resolution(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);
static ptx_Status_t ptx_NFCForumDTA_Process_Type_B_Resolution(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);
static ptx_Status_t ptx_NFCForumDTA_Process_Type_F_Resolution(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);
static ptx_Status_t ptx_NFCForumDTA_Process_Type_V_Resolution(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NFCForumDTA_ProcessMultipleCards(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        ptx_RUL_CardRfTechType_t techType = dta->rul->CardRegistry.Cards[0].TechType;

        switch (techType)
        {
            case ptx_RUL_CardRfTechType_A:
                status = ptx_NFCForumDTA_Process_Type_A_Resolution(dta, testCfg);
                break;
            case ptx_RUL_CardRfTechType_B:
                status = ptx_NFCForumDTA_Process_Type_B_Resolution(dta, testCfg);
                break;
            case ptx_RUL_CardRfTechType_F:
                status = ptx_NFCForumDTA_Process_Type_F_Resolution(dta, testCfg);
                break;
            case ptx_RUL_CardRfTechType_V:
                status = ptx_NFCForumDTA_Process_Type_V_Resolution(dta, testCfg);
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_STATE;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            dta->State = ptx_NFCForumDTA_State_POLL_DATA_EXCHANGE;
        }

    }


    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */


static ptx_Status_t ptx_NFCForumDTA_Process_Type_A_Resolution(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        // Multiple device resolution for Type A Communication

        uint8_t t2t_platform_index = 0xFF;
        uint8_t t4t_platform_index = 0xFF;
        uint8_t nfc_dep_platform_index = 0xFF;

        for (uint8_t i = 0; i < dta->cardRegistry->CardsCnt; i++)
        {
            if (0 != (dta->cardRegistry->Cards[i].Protocols & ptx_RUL_SupportedProtocols_T2T))
            {
                t2t_platform_index = i;
            }

            if (0 != (dta->cardRegistry->Cards[i].Protocols & ptx_RUL_SupportedProtocols_T4T))
            {
                t4t_platform_index = i;
            }

            if (0 != (dta->cardRegistry->Cards[i].Protocols & ptx_RUL_SupportedProtocols_NfcDep))
            {
                nfc_dep_platform_index = i;
            }
        }

        ptx_RUL_CardProtocol_t protocol = {0};

        if (0xFF != t2t_platform_index)
        {
            // T2T Activation
            protocol.Type = ptx_RUL_CardProtocolType_T2T;

            status = ptx_RUL_SelectCard(dta->rul, t2t_platform_index, protocol);
        }
        else if (0xFF != t4t_platform_index)
        {
            uint8_t selected_nfc_dep = 0;

            if (0xFF != nfc_dep_platform_index)
            {
                if (1 != dta->DTAConfiguration.T4AT_NFC_DEP_PRIORITY)
                {
                    // NFC DEP activation
                    protocol.Type = ptx_RUL_CardProtocolType_NfcDep;

                    // TODO: Add NFC DEP Select Params once available
                    //protocol.Param->NfcDep.Undefined;

                    status = ptx_RUL_SelectCard(dta->rul, nfc_dep_platform_index, protocol);

                    selected_nfc_dep = 1;
                }
            }

            if (0 == selected_nfc_dep)
            {
                // T4AT Activation
                protocol.Type = ptx_RUL_CardProtocolType_T4T;

                protocol.Param.IsoDep.HbrEnable = 0;
                protocol.Param.IsoDep.AttribInfLen = 0;

                status = ptx_RUL_SelectCard(dta->rul, t4t_platform_index, protocol);
            }
        }
        else if (0xFF != nfc_dep_platform_index)
        {
            // NFC DEP activation
            protocol.Type = ptx_RUL_CardProtocolType_NfcDep;

            // TODO: Add NFC DEP Select Params once available
            //protocol.Param->NfcDep.Undefined;

            status = ptx_RUL_SelectCard(dta->rul, nfc_dep_platform_index, protocol);

        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }


        if (PTX_STATUS_SUCCESS == status)
        {
            dta->State = ptx_NFCForumDTA_State_POLL_DATA_EXCHANGE;
        }
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_Process_Type_B_Resolution(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        // Multiple device resolution for Type B Communication

        uint8_t t4t_platform_index = 0xFF;

        for (uint8_t i = 0; i < dta->cardRegistry->CardsCnt; i++)
        {
            if (0 != (dta->cardRegistry->Cards[i].Protocols & ptx_RUL_SupportedProtocols_T4T))
            {
                t4t_platform_index = i;
            }
        }

        ptx_RUL_CardProtocol_t protocol = {0};

        if (0xFF != t4t_platform_index)
        {
            // T4BT Activation
            protocol.Type = ptx_RUL_CardProtocolType_T4T;

            // TODO: Check how ATTRIB inf is built based on SENSB RES

            //uint8_t SensBRes[13] = dta->cardRegistry->Cards[t4t_platform_index].TechParam.TypeB.SensBRes;

            //params.IsoDep.Did =

            status = ptx_RUL_SelectCard(dta->rul, t4t_platform_index, protocol);
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_Process_Type_F_Resolution(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        // Multiple device resolution for Type F Communication

        uint8_t t3t_platform_index = 0xFF;
        uint8_t nfc_dep_platform_index = 0xFF;

        for (uint8_t i = 0; i < dta->cardRegistry->CardsCnt; i++)
        {
            if (0 != (dta->cardRegistry->Cards[i].Protocols & ptx_RUL_SupportedProtocols_T3T))
            {
                t3t_platform_index = i;
            }

            if (0 != (dta->cardRegistry->Cards[i].Protocols & ptx_RUL_SupportedProtocols_NfcDep))
            {
                nfc_dep_platform_index = i;
            }
        }

        ptx_RUL_CardProtocol_t protocol = {0};

        if (0xFF != t3t_platform_index)
        {
            uint8_t selected_nfc_dep = 0;

            if (0xFF != nfc_dep_platform_index)
            {
                if (1 != dta->DTAConfiguration.T3T_NFC_DEP_PRIORITY)
                {
                    // NFC DEP activation
                    protocol.Type = ptx_RUL_CardProtocolType_NfcDep;
                    protocol.Param.NfcDep.GeneralBytesLen = 0;
                    protocol.Param.NfcDep.HbrEnable = 0;

                    status = ptx_RUL_SelectCard(dta->rul, nfc_dep_platform_index, protocol);

                    selected_nfc_dep = 1;
                }
            }

            if (0 == selected_nfc_dep)
            {
                // T3T Activation
                protocol.Type = ptx_RUL_CardProtocolType_T3T;

                status = ptx_RUL_SelectCard(dta->rul, t3t_platform_index, protocol);
            }

        }
        else if (0xFF != nfc_dep_platform_index)
        {
            // NFC DEP activation
            protocol.Type = ptx_RUL_CardProtocolType_NfcDep;

            // TODO: Add NFC DEP Select Params once available
            //protocol.Param->NfcDep.Undefined;

            status = ptx_RUL_SelectCard(dta->rul, nfc_dep_platform_index, protocol);

        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_Process_Type_V_Resolution(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        // Multiple device resolution for Type V Communication

        uint8_t t5t_platform_index = 0xFF;

        for (uint8_t i = 0; i < dta->cardRegistry->CardsCnt; i++)
        {
            if (0 != (dta->cardRegistry->Cards[i].Protocols & ptx_RUL_SupportedProtocols_T5T))
            {
                t5t_platform_index = i;
            }
        }

        // Handle special cases
        if ((0x0031 == testCfg->PATTERN_NUMBER))
        {
            // Handle special Sleep select case for T5T with Pattern Number 0x0031
            status = ptx_NFCForumDTA_T5TSleepSelect(dta);
            if (PTX_STATUS_SUCCESS == status)
            {
                dta->State = ptx_NFCForumDTA_State_POLL_DATA_EXCHANGE;
            }
        }
        else if ((0x0021 == testCfg->PATTERN_NUMBER))
        {
            // Handle special addressed mode case for T5T with Pattern Number 0x0021
            // Select the first detected card as the active one
            dta->State = ptx_NFCForumDTA_State_POLL_DATA_EXCHANGE;
            dta->cardRegistry->ActiveProtocol = ptx_RUL_CardProtocolType_T5T;
            dta->cardRegistry->ActiveCard = &dta->cardRegistry->Cards[0];
            status = PTX_STATUS_SUCCESS;
        }
        else if (0xFF != t5t_platform_index)
        {
            ptx_RUL_CardProtocol_t protocol = {0};

            protocol.Type = ptx_RUL_CardProtocolType_T5T;

            status = ptx_RUL_SelectCard(dta->rul, t5t_platform_index, protocol);
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}
