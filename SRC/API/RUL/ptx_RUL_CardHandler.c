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
    Module      : RUL
    File        : ptx_RUL_CardHandler.c

    Description : Reader Universal Library (RUL) Card Registry implementation.
                  Contains functions to manage the card registry, determine protocols, and update active protocol parameters.
 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_RUL.h"

#include "ptx_NSC_RfActivate.h"
#include "ptx_NSC_RfPresenceCheck.h"

#include <stdint.h>


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
static void ptx_RUL_UpdateActiveProtocolParams(ptx_RUL_CardRegistry_t *cardRegistry, ptx_NSC_RfAct_ProtParam_t *data);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_RUL_AddToCardRegistry(ptx_RUL_t* rul, ptx_RUL_CardParams_t *discoveredCard)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != discoveredCard) )
    {
        if (PTX_RUL_MAX_SUPPORTED_DEVICES > rul->CardRegistry.CardsCnt)
        {
            memcpy(&rul->CardRegistry.Cards[rul->CardRegistry.CardsCnt], discoveredCard, sizeof(ptx_RUL_CardParams_t));
            rul->CardRegistry.CardsCnt++;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_ClearCardRegistry(ptx_RUL_t* rul)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        rul->DiscoverStatus = ptx_RUL_StatusInfoDiscover_NoCard;
        rul->DeactivateStatus = ptx_RUL_StatusInfoDeactivate_Undefined;
        rul->EmvRemovalStatus = ptx_RUL_StatusInfoEmvRemoval_Undefined;
        rul->RfErrorStatus = ptx_RUL_StatusInfoRfError_ErrorNone;
        rul->LpcdStatus.LpcdNtfCounter = 0;

        memset(&rul->CardRegistry, 0, sizeof(rul->CardRegistry));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_DetermineProtocol(ptx_RUL_t* rul, ptx_RUL_CardParams_t* card)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != card) )
    {
        card->Protocols = ptx_RUL_SupportedProtocols_Undefined;

        status = PTX_STATUS_SUCCESS;
        switch (card->TechType)
        {
            case ptx_RUL_CardRfTechType_A:
            {
                if (0 != card->TechParam.TypeA.NfcId1Len)
                {
                    /* NFC-DEP check */
                    if (0 != (card->TechParam.TypeA.SelRes & PTX_RUL_SUPPORT_NFCDEP_NFCA))
                    {
                        card->Protocols |= ptx_RUL_SupportedProtocols_NfcDep;
                    }

                    /* ISO-DEP check */
                    if (0 != (card->TechParam.TypeA.SelRes & PTX_RUL_SUPPORT_ISODEP_NFCA))
                    {
                        card->Protocols |= ptx_RUL_SupportedProtocols_T4T;
                    }

                    /*
                     * T2T/MIFARE
                     * Note: T2T in this case refers more to ISO 14443 Layer 3 which is always true. It does not
                     *       really mean that a card supports T2T/Mifare Card Emulation (see Definitions of NFC Forum).
                     */
                    if (ptx_RUL_SupportedProtocols_Undefined == card->Protocols)
                    {
                        card->Protocols |= ptx_RUL_SupportedProtocols_T2T;
                    }
                }
                break;
            }
            case ptx_RUL_CardRfTechType_B:
            {
                card->Protocols |= ptx_RUL_SupportedProtocols_T4T;
                break;
            }
            case ptx_RUL_CardRfTechType_F:
            {
                if (0x01 == card->TechParam.TypeF.SensFRes[PTX_RUL_SUPPORT_NFCDEP_NFCF_POSITION] &&
                    (0xFE == card->TechParam.TypeF.SensFRes[PTX_RUL_SUPPORT_NFCDEP_NFCF_POSITION + 1]))
                {
                    card->Protocols |= ptx_RUL_SupportedProtocols_NfcDep;
                }
                else
                {
                    card->Protocols |= ptx_RUL_SupportedProtocols_T3T;
                }
                break;
            }
            case ptx_RUL_CardRfTechType_V:
            {
                card->Protocols |= ptx_RUL_SupportedProtocols_T5T;
                break;
            }
            case ptx_RUL_CardRfTechType_ACM:
            {
                card->Protocols |= ptx_RUL_SupportedProtocols_NfcDep;
                break;
            }
            default:
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_GetCardRegistry(ptx_RUL_t* rul, ptx_RUL_CardRegistry_t** registry)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != registry))
    {
        *registry = &rul->CardRegistry;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RUL_CheckCardPresence(ptx_RUL_t* rul, ptx_RUL_CheckPresType_t type)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        switch (type)
        {
            case ptx_RUL_CheckPresType_NFCDEP:
                if (ptx_RUL_CardProtocolType_NfcDep == rul->CardRegistry.ActiveProtocol)
                {
                    status = ptx_NSC_RfPresenceCheck_AttentionCmd(&(rul->Nsc));
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                }
                break;

            case ptx_RUL_CheckPresType_ISODEP:
                if (ptx_RUL_CardProtocolType_T4T == rul->CardRegistry.ActiveProtocol)
                {
                    status = ptx_NSC_RfPresenceCheck_RNak(&(rul->Nsc));
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                }
                break;

            case ptx_RUL_CheckPresType_ISODEP_Empty:
                if (ptx_RUL_CardProtocolType_T4T == rul->CardRegistry.ActiveProtocol)
                {
                    status = ptx_NSC_RfPresenceCheck_EmptyFrame(&(rul->Nsc));
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                }
                break;

            case ptx_RUL_CheckPresType_T2T:
                if (ptx_RUL_CardProtocolType_T2T == rul->CardRegistry.ActiveProtocol)
                {
                    status = ptx_NSC_RfPresenceCheck_FrameRf(&(rul->Nsc), ptx_NSC_PresCheck_FrameRf_T2T_Read, rul->CardRegistry.ActiveCard->TechParam.TypeA.NfcId1);
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                }
                break;

            case ptx_RUL_CheckPresType_T3T:
                if (ptx_RUL_CardProtocolType_T3T == rul->CardRegistry.ActiveProtocol)
                {
                    uint8_t NfcId2[PTX_NSC_NFCID2_LEN];
                    memcpy(NfcId2, &rul->CardRegistry.ActiveCard->TechParam.TypeF.SensFRes[2], PTX_NSC_NFCID2_LEN);
                    status = ptx_NSC_RfPresenceCheck_FrameRf(&(rul->Nsc), ptx_NSC_PresCheck_FrameRf_T3T_Check, NfcId2);
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                }
                break;

            case ptx_RUL_CheckPresType_T5T:
                if (ptx_RUL_CardProtocolType_T5T == rul->CardRegistry.ActiveProtocol)
                {
                    uint8_t Uid[PTX_RUL_UID_LEN];
                    memcpy(Uid, &rul->CardRegistry.ActiveCard->TechParam.TypeV.InventoryRes[2], PTX_RUL_UID_LEN);
                    status = ptx_NSC_RfPresenceCheck_FrameRf(&(rul->Nsc), ptx_NSC_PresCheck_FrameRf_T5T_Read, Uid);
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                }
                break;


            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SelectCard(ptx_RUL_t *rul, uint32_t cardRegistryId, ptx_RUL_CardProtocol_t protocol)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Check if cardRegistryId & ProtocolType are within bounds. */
    if ( PTX_COMP_CHECK(rul, Comp_RUL) &&
         (rul->CardRegistry.CardsCnt >= (cardRegistryId + 1)) &&
         (ptx_RUL_CardProtocolType_Undefined < protocol.Type) &&
         (ptx_RUL_CardProtocolType_Max > protocol.Type) )
    {
        ptx_RUL_CardParams_t card_entry = rul->CardRegistry.Cards[cardRegistryId];

        ptx_NSC_RfAct_Param_t activate_parameters;
        memset(&activate_parameters, 0, sizeof(activate_parameters));

        status = PTX_STATUS_SUCCESS;

        switch (card_entry.TechType)
        {
            case ptx_RUL_CardRfTechType_A:
                activate_parameters.RfTech = ptx_NSC_RfTech_Passive_Poll_A;
                activate_parameters.RfTechParams.RfAct_A.RfState = (ptx_NSC_RfAct_PollMode_State_A_t) card_entry.State;
                activate_parameters.RfTechParams.RfAct_A.NfcId1Len = card_entry.TechParam.TypeA.NfcId1Len;
                memcpy(activate_parameters.RfTechParams.RfAct_A.NfcId1, card_entry.TechParam.TypeA.NfcId1, card_entry.TechParam.TypeA.NfcId1Len);
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_CardRfTechType_B:
                activate_parameters.RfTech = ptx_NSC_RfTech_Passive_Poll_B;
                activate_parameters.RfTechParams.RfAct_B.RfState = (ptx_NSC_RfAct_PollMode_State_B_t) card_entry.State;
                memcpy(activate_parameters.RfTechParams.RfAct_B.SensBRes, card_entry.TechParam.TypeB.SensBRes, PTX_RUL_SENSBRES_LEN);
                break;

            case ptx_RUL_CardRfTechType_F:
                activate_parameters.RfTech = ptx_NSC_RfTech_Passive_Poll_F;
                activate_parameters.RfTechParams.RfAct_F.RfState = (ptx_NSC_RfAct_PollMode_State_F_t) card_entry.State;
                memcpy(activate_parameters.RfTechParams.RfAct_F.NfcId2, &card_entry.TechParam.TypeF.SensFRes[PTX_RUL_SENSF_RES_NFCID2_OFFSET], PTX_RUL_NFCID2_LEN);
                activate_parameters.RfTechParams.RfAct_F.BitRate = card_entry.TechParam.TypeF.SensFRes[PTX_RUL_SENSF_RES_PD2_OFFSET] & PTX_RUL_SENSF_RES_PD2_BR_MASK;
                if (0x00 == activate_parameters.RfTechParams.RfAct_F.BitRate)
                {
                    activate_parameters.RfTechParams.RfAct_F.BitRate = (0 == rul->DiscoverConfig.PollTypeF424) ? ptx_NSC_RfAct_PollMode_BitRate_212 : ptx_NSC_RfAct_PollMode_BitRate_424;
                }
                break;

            case ptx_RUL_CardRfTechType_V:
                activate_parameters.RfTech = ptx_NSC_RfTech_Passive_Poll_V;
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        switch (protocol.Type)
        {
            case ptx_RUL_CardProtocolType_T2T:
                activate_parameters.RfProt = ptx_NSC_RfProtocol_T2T;
                break;
            case ptx_RUL_CardProtocolType_T3T:
                activate_parameters.RfProt = ptx_NSC_RfProtocol_T3T;
                break;
            case ptx_RUL_CardProtocolType_T4T:
                activate_parameters.RfProt = ptx_NSC_RfProtocol_IsoDep;
                activate_parameters.RfProtParams.IsoDep.HbrEnable = protocol.Param.IsoDep.HbrEnable;
                if (PTX_NSC_RFACTIVATE_ATTRIBINF_LEN >= protocol.Param.IsoDep.AttribInfLen)
                {
                    activate_parameters.RfProtParams.IsoDep.AttribInfLen = protocol.Param.IsoDep.AttribInfLen;
                    memcpy(activate_parameters.RfProtParams.IsoDep.AttribInf, protocol.Param.IsoDep.AttribInf, protocol.Param.IsoDep.AttribInfLen);
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                }
                break;
            case ptx_RUL_CardProtocolType_NfcDep:
                activate_parameters.RfProt = ptx_NSC_RfProtocol_NfcDep;
                activate_parameters.RfProtParams.NfcDep.HbrEnable = protocol.Param.NfcDep.HbrEnable;
                if (PTX_NSC_RFACTIVATE_GENBYTES_LEN >= protocol.Param.NfcDep.GeneralBytesLen)
                {
                    activate_parameters.RfProtParams.NfcDep.GeneralBytesLen = protocol.Param.NfcDep.GeneralBytesLen;
                    memcpy(activate_parameters.RfProtParams.NfcDep.GeneralBytes, protocol.Param.NfcDep.GeneralBytes, protocol.Param.NfcDep.GeneralBytesLen);
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                }
                break;
            case ptx_RUL_CardProtocolType_T5T:
                activate_parameters.RfProt = ptx_NSC_RfProtocol_T5T;
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /*
             * Allocate space for data returned during activation: protocol parameters.
             */
            ptx_NSC_RfAct_ProtParam_t data = {0};
            status = ptx_NSC_RfActivate(&(rul->Nsc), &activate_parameters, &data);

            if (PTX_STATUS_SUCCESS == status)
            {
                rul->CardRegistry.ActiveCard = &rul->CardRegistry.Cards[cardRegistryId];
                rul->CardRegistry.ActiveCardId = cardRegistryId;
                rul->CardRegistry.ActiveProtocol = protocol.Type;
                ptx_RUL_UpdateActiveProtocolParams(&(rul->CardRegistry), &data);

                rul->DiscoverStatus = ptx_RUL_StatusInfoDiscover_Active;

                if ((ptx_RUL_CardRfTechType_F == card_entry.TechType) && (ptx_RUL_CardProtocolType_T3T == protocol.Type))
                {
                    if (ptx_NSC_RfAct_PollMode_BitRate_424 == activate_parameters.RfTechParams.RfAct_F.BitRate)
                    {
                        rul->CardRegistry.ActiveCard->TechParam.TypeF.BitRate = ptx_RUL_RfBitRate_424;
                    }
                    else /* ptx_NSC_RfAct_PollMode_BitRate_212 */
                    {
                        rul->CardRegistry.ActiveCard->TechParam.TypeF.BitRate = ptx_RUL_RfBitRate_212;
                    }
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_GetCardParameters(ptx_RUL_t* rul, ptx_RUL_CardParams_t** cardParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != cardParams))
    {
        if (NULL != rul->CardRegistry.ActiveCard)
        {
            *cardParams = rul->CardRegistry.ActiveCard;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static void ptx_RUL_UpdateActiveProtocolParams(ptx_RUL_CardRegistry_t *cardRegistry, ptx_NSC_RfAct_ProtParam_t *data)
{
    if ((NULL != cardRegistry) && (NULL != cardRegistry->ActiveCard) && (NULL != data))
    {
        /* Parameters are provided only for IsoDep and NfcDep protocols in poll mode. */

        if (ptx_RUL_CardProtocolType_T4T == cardRegistry->ActiveProtocol)
        {
            if (ptx_RUL_CardRfTechType_A == cardRegistry->ActiveCard->TechType)
            {
                (void)memcpy(cardRegistry->ActiveProtParam.PollAIsoDep.ATS, data->IsoDep.PollA.ATS, PTX_RUL_CARD_RFPROTOCOL_ATS_LEN);
                cardRegistry->ActiveProtParam.PollAIsoDep.PPS1 = data->IsoDep.PollA.PPS1;
            }
            else if (ptx_RUL_CardRfTechType_B == cardRegistry->ActiveCard->TechType)
            {
                cardRegistry->ActiveProtParam.PollBIsoDep.AttribParam1 = data->IsoDep.PollB.AttribParam1;
                cardRegistry->ActiveProtParam.PollBIsoDep.AttribParam2 = data->IsoDep.PollB.AttribParam2;
                cardRegistry->ActiveProtParam.PollBIsoDep.AttribResLength = data->IsoDep.PollB.AttribResLen;
                (void)memcpy(cardRegistry->ActiveProtParam.PollBIsoDep.AttribRes, data->IsoDep.PollB.AttribRes, PTX_RUL_CARD_RFPROTOCOL_ATTRIBRES_LEN);
            }
            else
            {
                /* No parameters provided for other technologies. */
            }
        }
        else if (ptx_RUL_CardProtocolType_NfcDep == cardRegistry->ActiveProtocol)
        {
            cardRegistry->ActiveProtParam.PollAFACMNfcDep.Brs = data->NfcDep.PollAF.Brs;
            (void)memcpy(cardRegistry->ActiveProtParam.PollAFACMNfcDep.AtrRes, data->NfcDep.PollAF.AtrRes, PTX_RUL_CARD_RFPROTOCOL_ATR_RES_LEN);
        }
        else
        {
            /* No parameters provided for other protocols. */
        }
    }
}

