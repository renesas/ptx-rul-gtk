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
    Module      : Utils
    File        : ptx_Utils_VersionInfo.c

    Description : Implements a helper-function to read/print all available card-parameters from the RUL Card-Registry.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Utils_CardInfo.h"
#include "ptx_Utils_Print.h"
#include <stdio.h>
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_Utils_CardInfo_Print_Card_Parameters(ptx_RUL_t *rul, uint32_t cardRegistryId)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    ptx_RUL_CardRegistry_t *card_registry = NULL;
    ptx_RUL_CardParams_t *card_params;
    uint8_t skip_ext_info = 0;

    if (NULL != rul)
    {
        status = ptx_RUL_GetCardRegistry(rul, &card_registry);

        if ((PTX_STATUS_SUCCESS == status) && (card_registry->CardsCnt > cardRegistryId) && (0 != card_registry->CardsCnt))
        {
            card_params = &card_registry->Cards[cardRegistryId];

            if (NULL != card_params)
            {
                switch (card_params->TechType)
                {
                    case ptx_RUL_CardRfTechType_A:
                    {
                        ptx_Utils_Print("%02d. RF-Technology = Type-A", cardRegistryId);
                        ptx_Utils_Print("; SENS_RES: ");
                        ptx_Utils_Print_Buffer(&card_params->TechParam.TypeA.SensRes[0], (uint16_t)sizeof(card_params->TechParam.TypeA.SensRes), 0, 0);
                        ptx_Utils_Print("; NFCID1_LEN: %02X", card_params->TechParam.TypeA.NfcId1Len);
                        ptx_Utils_Print("; NFCID1: ");
                        ptx_Utils_Print_Buffer(&card_params->TechParam.TypeA.NfcId1[0], card_params->TechParam.TypeA.NfcId1Len, 0, 0);
                        ptx_Utils_Print("; SEL_RES: %02X", card_params->TechParam.TypeA.SelRes);
                        break;
                    }

                    case ptx_RUL_CardRfTechType_B:
                    {
                        ptx_Utils_Print("%02d. RF-Technology = Type-B", cardRegistryId);
                        ptx_Utils_Print("; SENSB_RES: ");
                        ptx_Utils_Print_Buffer(&card_params->TechParam.TypeB.SensBRes[0], (uint16_t)sizeof(card_params->TechParam.TypeB.SensBRes), 0, 0);
                        break;
                    }

                    case ptx_RUL_CardRfTechType_F:
                    {
                        ptx_Utils_Print("%02d. RF-Technology = Type-F", cardRegistryId);
                        ptx_Utils_Print("; SENSF_RES: ");
                        ptx_Utils_Print_Buffer(&card_params->TechParam.TypeF.SensFRes[0], (uint16_t)sizeof(card_params->TechParam.TypeF.SensFRes), 0, 0);
                        break;
                    }

                    case ptx_RUL_CardRfTechType_V:
                    {
                        ptx_Utils_Print("%02d. RF-Technology = Type-V", cardRegistryId);
                        ptx_Utils_Print("; INVENTORY_RES: ");
                        ptx_Utils_Print_Buffer(&card_params->TechParam.TypeV.InventoryRes[0], (uint16_t)sizeof(card_params->TechParam.TypeV.InventoryRes), 0, 0);
                        break;
                    }

                    default:
                    {
                        skip_ext_info = 1u;
                        ptx_Utils_Print("Unknown or unsupported Card-Type found.");
                        break;
                    }
                }

                if (0 == skip_ext_info)
                {
                    if (card_registry->ActiveCard == card_params)
                    {
                        switch (card_registry->ActiveProtocol)
                        {
                            case ptx_RUL_CardProtocolType_T2T:
                            {
                                ptx_Utils_Print(";  Protocol: T2T");
                                break;
                            }

                            case ptx_RUL_CardProtocolType_T3T:
                            {
                                ptx_Utils_Print(";  Protocol: T3T");
                                break;
                            }

                            case ptx_RUL_CardProtocolType_T4T:
                            {
                                ptx_Utils_Print(";  Protocol: T4T (ISO-DEP)");
                                break;
                            }

                            case ptx_RUL_CardProtocolType_NfcDep:
                            {
                                ptx_Utils_Print(";  Protocol: P2P (NFC-DEP)");
                                break;
                            }

                            case ptx_RUL_CardProtocolType_T5T:
                            {
                                ptx_Utils_Print(";  Protocol: T5T");
                                break;
                            }

                            default:
                            {
                                skip_ext_info = 1u;
                                ptx_Utils_Print("; Unknown / Unsupported Protocol found");
                                break;
                            }
                        }
                    }
                }

                // TODO: Print Higher-Level Protocol Info
                if (0 == skip_ext_info)
                {

                }

                ptx_Utils_Print("\n");
            }
            else
            {

            } status = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return status;
}


ptx_Status_t ptx_Utils_Check_Rf_Error(ptx_Status_t rfStatus)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    switch (rfStatus)
    {
        case PTX_STATUS_E_NSC_COLLISION:        /* FALL THROUGH */
        case PTX_STATUS_E_NSC_RESIDUAL_BITS:    /* FALL THROUGH */
        case PTX_STATUS_E_GEN_TIMEOUT:          /* FALL THROUGH */
        case PTX_STATUS_E_NSC_RFERROR:
            status = PTX_STATUS_SUCCESS;
            break;

        default:
            status = rfStatus;
            break;
    }

    return status;
}

