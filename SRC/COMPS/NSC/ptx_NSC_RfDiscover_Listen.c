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
    Module      : NFC Soft Controller (NSC) Interface for RF Discovery Listen Mode
    File        : ptx_NSC_RfDiscover_Listen.c

    Description : Implementation of NFC Soft Controller Interface for RF Discovery Listen Mode.
                  Contains function(s) to handle RF Discovery Listen Mode parameters.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_RfDiscover.h"


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * Definitions for NSC Discover Command - Listen Mode.
 *
 */
#define PTX_NSC_RF_DISCOVER_PARAM_LISTEN_LENGTH             (1u)    /* Length of the Listen parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_LISTEN_A_LENGTH           (15u)   /* Length of the NFC-A Listen parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_LISTEN_B_LENGTH           (14u)   /* Length of the NFC-B Listen parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_LISTEN_F_LENGTH           (21u)   /* Length of the NFC-F Listen parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_LISTEN_ISODEP_LENGTH      (38u)   /* Length of the ISO-DEP Listen parameter. */
#define PTX_NSC_RF_DISCOVER_PARAM_LISTEN_NFCDEP_LENGTH      (21u)   /* Length of the NFC-DEP Listen parameter. */


#define PTX_NSC_RF_DISCOVER_LISTEN_ENABLEBYTES_MASK         (0x03u) /* Mask for relevant enabling bits of first byte in Listen parameters. */
#define PTX_NSC_RF_DISCOVER_LISTEN_FIELDNTF_OFFSET          (0x01u) /* Offset of Rf-Field NTF Enable bit. */
#define PTX_NSC_RF_DISCOVER_LISTEN_A_ENABLEBYTES_MASK       (0x01u) /* Mask for relevant enabling bits of first byte in NFC-A Listen parameters. */
#define PTX_NSC_RF_DISCOVER_LISTEN_B_ENABLEBYTES_MASK       (0x03u) /* Mask for relevant enabling bits of first byte in NFC-B Listen parameters. */
#define PTX_NSC_RF_DISCOVER_LISTEN_B_ENABLEB_OFFSET         (0x01u) /* Offset NFC-B Listen Enable bit. */
#define PTX_NSC_RF_DISCOVER_LISTEN_B_RANDOMID_OFFSET        (0x01u) /* Offset of Random NFCID0 Enable bit. */
#define PTX_NSC_RF_DISCOVER_LISTEN_F_ENABLEBYTES_MASK       (0x0Fu) /* Mask for relevant enabling bits of first byte in NFC-F Listen parameters. */
#define PTX_NSC_RF_DISCOVER_LISTEN_F_ENABLEF212_OFFSET      (0x00u) /* Offset of NFC-F 212 kbps Enable bit. */
#define PTX_NSC_RF_DISCOVER_LISTEN_F_ENABLEF424_OFFSET      (0x01u) /* Offset of NFC-F 424 kbps Enable bit. */
#define PTX_NSC_RF_DISCOVER_LISTEN_F_HANDLESENRQ_OFFSET     (0x02u) /* Offset of SENSF_RQ handler Enable bit. */
#define PTX_NSC_RF_DISCOVER_LISTEN_F_ADDRDBYTES_OFFSET      (0x03u) /* Offset of SENSF_RES RD bytes Enable bit. */

#define PTX_NSC_RF_DISCOVER_LISTEN_ISODEP_ENABLEBYTES_MASK  (0x03u) /* Mask for relevant enabling bits of first byte in ISO-DEP Listen parameters. */
#define PTX_NSC_RF_DISCOVER_LISTEN_ISODEP_ENABLEA_OFFSET    (0x00u) /* Offset of ISO-DEP T4AT Enable bit. */
#define PTX_NSC_RF_DISCOVER_LISTEN_ISODEP_ENABLEB_OFFSET    (0x01u) /* Offset of ISO-DEP T4BT Enable bit. */
#define PTX_NSC_RF_DISCOVER_LISTEN_NFCDEP_ENABLEBYTES_MASK  (0x03u) /* Mask for relevant enabling bits of first byte in NFC-DEP Listen parameters. */
#define PTX_NSC_RF_DISCOVER_LISTEN_NFCDEP_ENABLEA_OFFSET    (0x00u) /* Offset of NFC-DEP NFC-A Enable bit. */
#define PTX_NSC_RF_DISCOVER_LISTEN_NFCDEP_ENABLEF_OFFSET    (0x01u) /* Offset of NFC-DEP NFC-F Enable bit. */
#define PTX_NSC_RF_DISCOVER_LISTEN_A_SELRES_T4T             (0x20u) /* Default ListenA SEL_RES. */
#define PTX_NSC_RF_DISCOVER_LISTEN_A_SELRES_NFCDEP          (0x40u) /* Default ListenA SEL_RES. */
/*  */


/*
 * Default Listen-Configuration values.
 *
 */
static const uint8_t ptx_NSC_RfDiscover_ListenA_NfcId1[PTX_NSC_RF_DISCOVER_NFCID1_MAX_LEN] = {0x08, 0x01, 0x02, 0x03, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};  /* Default ListenA NFCID1. */
static const uint8_t ptx_NSC_RfDiscover_ListenA_SensRes[PTX_NSC_RF_DISCOVER_SENSRES_LEN] = {0x01, 0x00};                                                    /* Default ListenA SENS_RES. */

/* Using sparse initialization with designated initializers to comply with MISRA Rule 9.3. */
static const uint8_t ptx_NSC_RfDiscover_ListenIsoDep_ATS[PTX_NSC_RF_DISCOVER_ATS_LEN] = {[0] = 0x05, [1] = 0x78, [2] = 0x00, [3] = 0x71, [4] = 0x02};       /* Default ListenIsoDep ATS. */
/* Using sparse initialization with designated initializers to comply with MISRA Rule 9.3. */
static const uint8_t ptx_NSC_RfDiscover_ListenIsoDep_AttribRes[PTX_NSC_RF_DISCOVER_ATTRIBRES_LEN] = {[0] = 0x00, [1] = 0x00};                               /* Default ListenIsoDep ATTRIB_RES. */

static const uint8_t ptx_NSC_RfDiscover_ListenB_NfcId0[PTX_NSC_RF_DISCOVER_NFCID0_LEN] = {0x80, 0x01, 0x03, 0x02};                                          /* Default ListenB NFCID0. */
static const uint8_t ptx_NSC_RfDiscover_ListenB_ApplicationData[PTX_NSC_RF_DISCOVER_APPDATA_LEN] = {0x00, 0x00, 0x00, 0x00};                                /* Default ListenB SENSB_RES Application Data. */
static const uint8_t ptx_NSC_RfDiscover_ListenB_ProtocolInfo[PTX_NSC_RF_DISCOVER_PROTOCOLINFO_LEN] = {0x80, 0x87, 0x73, 0x00};                              /* Default ListenB SENSB_RES ProtocolInfo. */

static const uint8_t ptx_NSC_RfDiscover_ListenF_NfcId2[PTX_NSC_RF_DISCOVER_NFCID2_LEN] = {0x02, 0xFE, 0x80, 0x40, 0x30, 0x20, 0x10, 0x00};                  /* Default ListenF NFCID2. */
static const uint8_t ptx_NSC_RfDiscover_ListenF_Pad0[PTX_NSC_RF_DISCOVER_PAD0_LEN] = {0x00, 0x00};                                                          /* Default ListenF PAD0. */
static const uint8_t ptx_NSC_RfDiscover_ListenF_Pad0_NfcDep[PTX_NSC_RF_DISCOVER_PAD0_LEN] = {0xFF, 0xFF};
static const uint8_t ptx_NSC_RfDiscover_ListenF_Pad1[PTX_NSC_RF_DISCOVER_PAD1_LEN] = {0x00, 0x00, 0x00};                                                          /* Default ListenF PAD1. */
static const uint8_t ptx_NSC_RfDiscover_ListenF_Pad2[PTX_NSC_RF_DISCOVER_PAD2_LEN] = {0x00};

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_ConfigureListen(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigureListenA(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigureListenB(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigureListenF(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigureListenIsoDep(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_ConfigureListenNfcDep(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);
static ptx_Status_t ptx_NSC_SetDefaultDiscParams(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NSC_GetRfDiscoveryParam_Listen(ptx_NSC_t *nsc, ptx_NSC_RfDiscover_Param_Type_t type, ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != cfg) && (NULL != params))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_GetRfDiscoveryParam_Listen");

        switch (type)
        {
            case NSC_RfDiscover_Param_Type_Listen:
                status = ptx_NSC_ConfigureListen(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_ListenA:
                status = ptx_NSC_ConfigureListenA(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_ListenB:
                status = ptx_NSC_ConfigureListenB(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_ListenF:
                status = ptx_NSC_ConfigureListenF(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_ListenIsoDep:
                status = ptx_NSC_ConfigureListenIsoDep(cfg, params);
                break;
            case NSC_RfDiscover_Param_Type_ListenNfcDep:
                status = ptx_NSC_ConfigureListenNfcDep(cfg, params);
                break;
            default:
                /* Error already set. Invalid parameter. */
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_ParamParser_Listen(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_LISTEN_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Listen_t *param_listen = &param->Listen;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_Listen;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = (uint8_t)(
                                    ((uint8_t)param_listen->ListenEnable |
                                     (uint8_t)(param_listen->RfFieldNtfEnable << PTX_NSC_RF_DISCOVER_LISTEN_FIELDNTF_OFFSET))
                                    & PTX_NSC_RF_DISCOVER_LISTEN_ENABLEBYTES_MASK
                                );
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}

ptx_Status_t ptx_NSC_ParamParser_ListenA(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_LISTEN_A_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Listen_A_t *param_listen_a = &param->ListenA;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_ListenA;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = (uint8_t)(param_listen_a->EnableListenA & PTX_NSC_RF_DISCOVER_LISTEN_A_ENABLEBYTES_MASK);
        cmd_idx++;

        (void)memcpy(&((*bufferPos)[cmd_idx]), param_listen_a->SensRes, PTX_NSC_RF_DISCOVER_SENSRES_LEN);
        cmd_idx += PTX_NSC_RF_DISCOVER_SENSRES_LEN;

        (*bufferPos)[cmd_idx] = param_listen_a->SelRes;
        cmd_idx++;

        if (PTX_NSC_RF_DISCOVER_NFCID1_MAX_LEN >= param_listen_a->NfcId1Size)
        {
            (*bufferPos)[cmd_idx] = param_listen_a->NfcId1Size;
            cmd_idx++;

            /* Prepare padding 0x00 in case of short NFCID1. */
            memset(&((*bufferPos)[cmd_idx]), 0x00, PTX_NSC_RF_DISCOVER_NFCID1_MAX_LEN);
            (void)memcpy(&((*bufferPos)[cmd_idx]), param_listen_a->NfcId1, param_listen_a->NfcId1Size);
            cmd_idx += PTX_NSC_RF_DISCOVER_NFCID1_MAX_LEN;

            *bufferPos += cmd_idx;
            *availableLen -= cmd_idx;

            status = PTX_STATUS_SUCCESS;
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}

ptx_Status_t ptx_NSC_ParamParser_ListenB(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_LISTEN_B_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Listen_B_t *param_listen_b = &param->ListenB;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_ListenB;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = (uint8_t)(
                                    ((uint8_t)param_listen_b->EnableListenB |
                                     (uint8_t)(param_listen_b->RandomIdEnable << PTX_NSC_RF_DISCOVER_LISTEN_B_RANDOMID_OFFSET))
                                    & PTX_NSC_RF_DISCOVER_LISTEN_B_ENABLEBYTES_MASK
                                );
        cmd_idx++;

        (void)memcpy(&((*bufferPos)[cmd_idx]), param_listen_b->SensBRes, PTX_NSC_RF_DISCOVER_SENSBRES_LEN);
        cmd_idx += PTX_NSC_RF_DISCOVER_SENSBRES_LEN;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}

ptx_Status_t ptx_NSC_ParamParser_ListenF(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_LISTEN_F_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Listen_F_t *param_listen_f = &param->ListenF;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_ListenF;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = (uint8_t)(((uint8_t)param_listen_f->EnableListenF212   |
                                           (uint8_t)(param_listen_f->EnableListenF424 << PTX_NSC_RF_DISCOVER_LISTEN_F_ENABLEF424_OFFSET)   |
                                           (uint8_t)(param_listen_f->HandleSensfRq << PTX_NSC_RF_DISCOVER_LISTEN_F_HANDLESENRQ_OFFSET)     |
                                           (uint8_t)(param_listen_f->IncludeRDBytes << PTX_NSC_RF_DISCOVER_LISTEN_F_ADDRDBYTES_OFFSET)
                                          ) & PTX_NSC_RF_DISCOVER_LISTEN_F_ENABLEBYTES_MASK);
        cmd_idx++;

        (void)memcpy(&((*bufferPos)[cmd_idx]), param_listen_f->SensFRes, PTX_NSC_RF_DISCOVER_SENSFRES_MAXLEN);
        cmd_idx += PTX_NSC_RF_DISCOVER_SENSFRES_MAXLEN;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}

ptx_Status_t ptx_NSC_ParamParser_ListenIsoDep(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_LISTEN_ISODEP_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Listen_IsoDep_t *param_listen_iso = &param->ListenIsoDep;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_ListenIsoDep;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = (uint8_t)(
                                    ((uint8_t)param_listen_iso->EnableListenT4AT |
                                     (uint8_t)(param_listen_iso->EnableListenT4BT << PTX_NSC_RF_DISCOVER_LISTEN_ISODEP_ENABLEB_OFFSET))
                                    & PTX_NSC_RF_DISCOVER_LISTEN_ISODEP_ENABLEBYTES_MASK
                                );
        cmd_idx++;

        (void)memcpy(&((*bufferPos)[cmd_idx]), param_listen_iso->ATS, PTX_NSC_RF_DISCOVER_ATS_LEN);
        cmd_idx += PTX_NSC_RF_DISCOVER_ATS_LEN;

        if (PTX_NSC_RF_DISCOVER_ATTRIBRES_LEN >= param_listen_iso->AttribResLen)
        {
            (*bufferPos)[cmd_idx] = param_listen_iso->AttribResLen;
            cmd_idx++;

            /* Prepare padding 0x00 in case of short ATTRIB_RES. */
            memset(&((*bufferPos)[cmd_idx]), 0x00, PTX_NSC_RF_DISCOVER_ATTRIBRES_LEN);
            (void)memcpy(&((*bufferPos)[cmd_idx]), param_listen_iso->AttribRes, param_listen_iso->AttribResLen);
            cmd_idx += PTX_NSC_RF_DISCOVER_ATTRIBRES_LEN;

            *bufferPos += cmd_idx;
            *availableLen -= cmd_idx;

            status = PTX_STATUS_SUCCESS;
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}

ptx_Status_t ptx_NSC_ParamParser_ListenNfcDep(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState)
{
    /* Prevent compiler unused-parameter warning */
    PTX_UNUSED_PARAMETER(nextState);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_NSC_RF_DISCOVER_PARAM_LISTEN_NFCDEP_LENGTH < *availableLen)
    {
        uint32_t cmd_idx = 0;
        ptx_NSC_RfDiscover_Param_Listen_NfcDep_t *param_listen_nfc = &param->ListenNfcDep;

        (*bufferPos)[cmd_idx] = NSC_RfDiscover_Param_Type_ListenNfcDep;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = (uint8_t)(
                                    ((uint8_t)param_listen_nfc->EnableListenNfcDepA |
                                     (uint8_t)(param_listen_nfc->EnableListenNfcDepF << PTX_NSC_RF_DISCOVER_LISTEN_NFCDEP_ENABLEF_OFFSET))
                                    & PTX_NSC_RF_DISCOVER_LISTEN_NFCDEP_ENABLEBYTES_MASK
                                );
        cmd_idx++;

        if (PTX_NSC_RF_DISCOVER_ATR_RES_LEN >= param_listen_nfc->AtrResLen)
        {
            (*bufferPos)[cmd_idx] = param_listen_nfc->AtrResLen;
            cmd_idx++;

            /* Prepare padding 0x00 in case of short ATTRIB_RES. */
            memset(&((*bufferPos)[cmd_idx]), 0x00, PTX_NSC_RF_DISCOVER_ATR_RES_LEN);
            (void)memcpy(&((*bufferPos)[cmd_idx]), param_listen_nfc->AtrRes, param_listen_nfc->AtrResLen);
            cmd_idx += PTX_NSC_RF_DISCOVER_ATR_RES_LEN;

            *bufferPos += cmd_idx;
            *availableLen -= cmd_idx;

            status = PTX_STATUS_SUCCESS;
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}



/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NSC_ConfigureListen(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_Listen;
        ptx_NSC_RfDiscover_Param_Listen_t *listenParams = &params->Value.Listen;
        memset(listenParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Listen_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->ListenMode)
            {
                case NSC_Listen_Disabled:
                    listenParams->ListenEnable = NSC_RfDiscover_ListenMode_Disable;
                    listenParams->RfFieldNtfEnable = 0x00;
                    break;
                case NSC_Listen_Enabled:
                    listenParams->ListenEnable = NSC_RfDiscover_ListenMode_Enable;
                    listenParams->RfFieldNtfEnable = 0x00;
                    break;
                case NSC_Listen_Enabled_RfNtf:
                    listenParams->ListenEnable = NSC_RfDiscover_ListenMode_Enable;
                    listenParams->RfFieldNtfEnable = 0x01;
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return status;
}

static ptx_Status_t ptx_NSC_ConfigureListenA(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_ListenA;
        ptx_NSC_RfDiscover_Param_Listen_A_t *listenAParams = &params->Value.ListenA;
        memset(listenAParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Listen_A_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->ListenTypeA)
            {
                case NSC_ListenA_Disabled:
                    listenAParams->EnableListenA = NSC_ListenA_Disabled;
                    break;
                case NSC_ListenA_Enabled:
                    listenAParams->EnableListenA = NSC_ListenA_Enabled;
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }

            listenAParams->SelRes = 0x00;
            if (0 != cfg->ListenIsoDep)
            {
                listenAParams->SelRes |= 0x20;
            }
            if (0 != cfg->ListenNfcDep)
            {
                listenAParams->SelRes |= 0x40;
            }

            if ((PTX_NSC_NFCID1_LEN_1 == cfg->ListenTypeANfcIdLen) || (PTX_NSC_NFCID1_LEN_2 == cfg->ListenTypeANfcIdLen) || (PTX_NSC_NFCID1_LEN_3 == cfg->ListenTypeANfcIdLen))
            {
                listenAParams->NfcId1Size = cfg->ListenTypeANfcIdLen;
                memcpy(listenAParams->NfcId1, cfg->ListenTypeANfcId, cfg->ListenTypeANfcIdLen);
            }

            if (PTX_NSC_RF_DISCOVER_SENSRES_LEN == cfg->ListenTypeASensResLen)
            {
                memcpy(listenAParams->SensRes, cfg->ListenTypeASensRes, PTX_NSC_RF_DISCOVER_SENSRES_LEN);
            }
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return status;
}

static ptx_Status_t ptx_NSC_ConfigureListenB(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_ListenB;
        ptx_NSC_RfDiscover_Param_Listen_B_t *listenBParams = &params->Value.ListenB;
        memset(listenBParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Listen_B_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->ListenTypeB)
            {
                case NSC_ListenB_Disabled:
                    listenBParams->EnableListenB = NSC_ListenB_Disabled;
                    listenBParams->RandomIdEnable = 0x00;
                    break;
                case NSC_ListenB_Enabled:
                    listenBParams->EnableListenB = NSC_ListenB_Enabled;
                    listenBParams->RandomIdEnable = 0x00;
                    break;
                case NSC_ListenB_Enabled_RandomId:
                    listenBParams->EnableListenB = NSC_ListenB_Enabled;
                    listenBParams->RandomIdEnable = 0x01;
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }

            if (PTX_NSC_RF_DISCOVER_NFCID0_LEN == cfg->ListenTypeBNfcIdLen)
            {
                memcpy(&listenBParams->SensBRes[PTX_NSC_RF_DISCOVER_NFCID0_OFFSET], cfg->ListenTypeBNfcId, PTX_NSC_RF_DISCOVER_NFCID0_LEN);
            }

            if (PTX_NSC_RF_DISCOVER_APPDATA_LEN == cfg->ListenTypeBAppDataLen)
            {
                memcpy(&listenBParams->SensBRes[PTX_NSC_RF_DISCOVER_APPDATA_OFFSET], cfg->ListenTypeBAppData, PTX_NSC_RF_DISCOVER_APPDATA_LEN);
            }

            if (PTX_NSC_RF_DISCOVER_PROTOCOLINFO_LEN == cfg->ListenTypeBProtInfoLen)
            {
                memcpy(&listenBParams->SensBRes[PTX_NSC_RF_DISCOVER_PROTINFO0_OFFSET], cfg->ListenTypeBProtInfo, PTX_NSC_RF_DISCOVER_PROTOCOLINFO_LEN);
            }
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return status;
}

static ptx_Status_t ptx_NSC_ConfigureListenF(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_ListenF;
        ptx_NSC_RfDiscover_Param_Listen_F_t *listenFParams = &params->Value.ListenF;
        memset(listenFParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Listen_F_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->ListenTypeF212)
            {
                case NSC_ListenF212_Disabled:
                    listenFParams->EnableListenF212 = NSC_ListenF212_Disabled;
                    break;
                case NSC_ListenF212_Enabled:
                    listenFParams->EnableListenF212 = NSC_ListenF212_Enabled;
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->ListenTypeF424)
            {
                case NSC_ListenF424_Disabled:
                    listenFParams->EnableListenF424 = NSC_ListenF424_Disabled;
                    break;
                case NSC_ListenF424_Enabled:
                    listenFParams->EnableListenF424 = NSC_ListenF424_Enabled;
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->ListenTypeFSenRqHandler)
            {
                case NSC_ListenF_Handler_Host:
                    listenFParams->HandleSensfRq = NSC_ListenF_Handler_Host;
                    break;
                case NSC_ListenF_Handler_NscDevice:
                    listenFParams->HandleSensfRq = NSC_ListenF_Handler_NscDevice;
                    break;
                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->ListenTypeFIncludeRD)
            {
                case NSC_ListenF_DoNotIncludeRD:
                    listenFParams->IncludeRDBytes = NSC_ListenF_DoNotIncludeRD;
                    break;
                case NSC_ListenF_IncludeRD:
                    listenFParams->IncludeRDBytes = NSC_ListenF_IncludeRD;
                    break;
                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (PTX_NSC_RF_DISCOVER_RD_LEN == cfg->ListenTypeFRequestDataLen)
            {
                listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_SENSFRES_LEN_OFFSET] = PTX_NSC_RF_DISCOVER_SENSFRES_MAXLEN;
                memcpy(&listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_RD_OFFSET], cfg->ListenTypeFRequestData, PTX_NSC_RF_DISCOVER_RD_LEN);
            }
            else if (0 == cfg->ListenTypeFRequestDataLen)
            {
                listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_SENSFRES_LEN_OFFSET] = PTX_NSC_RF_DISCOVER_SENSFRES_LEN;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (PTX_NSC_RF_DISCOVER_NFCID2_LEN == cfg->ListenTypeFNfcIdLen)
            {
                memcpy(&listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_NFCID2_OFFSET], cfg->ListenTypeFNfcId, PTX_NSC_RF_DISCOVER_NFCID2_LEN);
            }
            else if (0 == cfg->ListenTypeFNfcIdLen)
            {
                /* No user NFCID provided, using the default NFCID */
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return status;
}

static ptx_Status_t ptx_NSC_ConfigureListenIsoDep(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_ListenIsoDep;
        ptx_NSC_RfDiscover_Param_Listen_IsoDep_t *listenIsoDepParams = &params->Value.ListenIsoDep;
        memset(listenIsoDepParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Listen_IsoDep_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->ListenIsoDep)
            {
                case NSC_ListenIsoDep_Disabled:
                    listenIsoDepParams->EnableListenT4AT = 0x00;
                    listenIsoDepParams->EnableListenT4BT = 0x00;
                    break;
                case NSC_ListenIsoDep_T4AT_Enabled:
                    listenIsoDepParams->EnableListenT4AT = 0x01;
                    listenIsoDepParams->EnableListenT4BT = 0x00;
                    break;
                case NSC_ListenIsoDep_T4BT_Enabled:
                    listenIsoDepParams->EnableListenT4AT = 0x00;
                    listenIsoDepParams->EnableListenT4BT = 0x01;
                    break;
                case NSC_ListenIsoDep_Both_Enabled:
                    listenIsoDepParams->EnableListenT4AT = 0x01;
                    listenIsoDepParams->EnableListenT4BT = 0x01;
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Continue loading ATS and ATTRIB_RES parameters only if valid lengths are provided. */
                if ((PTX_NSC_RF_DISCOVER_ATS_LEN >= cfg->ListenIsoDepATSLen) && (0 != cfg->ListenIsoDepATSLen))
                {
                    memcpy(listenIsoDepParams->ATS, cfg->ListenIsoDepATS, cfg->ListenIsoDepATSLen);
                }

                if ((PTX_NSC_RF_DISCOVER_ATTRIBRES_LEN >= cfg->ListenIsoDepAttribResLen) && (0 != cfg->ListenIsoDepAttribResLen))
                {
                    listenIsoDepParams->AttribResLen = cfg->ListenIsoDepAttribResLen;
                    memcpy(listenIsoDepParams->AttribRes, cfg->ListenIsoDepAttribRes, cfg->ListenIsoDepAttribResLen);
                }
            }
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return status;
}

static ptx_Status_t ptx_NSC_ConfigureListenNfcDep(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        params->Type = NSC_RfDiscover_Param_Type_ListenNfcDep;
        ptx_NSC_RfDiscover_Param_Listen_NfcDep_t *listenNfcDepParams = &params->Value.ListenNfcDep;
        memset(listenNfcDepParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_Listen_NfcDep_t));

        status = ptx_NSC_SetDefaultDiscParams(cfg, params);

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (cfg->ListenNfcDep)
            {
                case NSC_ListenNfcDep_Disabled:
                    listenNfcDepParams->EnableListenNfcDepA = 0x00;
                    listenNfcDepParams->EnableListenNfcDepF = 0x00;
                    break;
                case NSC_ListenNfcDep_NFCA_Enabled:
                    listenNfcDepParams->EnableListenNfcDepA = 0x01;
                    listenNfcDepParams->EnableListenNfcDepF = 0x00;
                    break;
                case NSC_ListenNfcDep_NFCF_Enabled:
                    listenNfcDepParams->EnableListenNfcDepA = 0x00;
                    listenNfcDepParams->EnableListenNfcDepF = 0x01;
                    break;
                case NSC_ListenNfcDep_Both_Enabled:
                    listenNfcDepParams->EnableListenNfcDepA = 0x01;
                    listenNfcDepParams->EnableListenNfcDepF = 0x01;
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }

            if (cfg->ListenNfcDepAtrResLen <= PTX_NSC_RF_DISCOVER_ATR_RES_LEN)
            {
                listenNfcDepParams->AtrResLen = cfg->ListenNfcDepAtrResLen;
                memcpy(listenNfcDepParams->AtrRes, cfg->ListenNfcDepAtrRes, listenNfcDepParams->AtrResLen);
            }
        }
    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return status;
}

static ptx_Status_t ptx_NSC_SetDefaultDiscParams(ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != cfg) && (NULL != params) )
    {
        switch (params->Type)
        {
            case NSC_RfDiscover_Param_Type_Listen:
            {
                ptx_NSC_RfDiscover_Param_Listen_t *listenParams = &params->Value.Listen;

                listenParams->ListenEnable = NSC_RfDiscover_ListenMode_Enable;
                listenParams->RfFieldNtfEnable = 1;

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_ListenA:
            {
                ptx_NSC_RfDiscover_Param_Listen_A_t *listenAParams = &params->Value.ListenA;

                listenAParams->EnableListenA = NSC_ListenA_Enabled;
                memcpy(listenAParams->SensRes, ptx_NSC_RfDiscover_ListenA_SensRes, PTX_NSC_RF_DISCOVER_SENSRES_LEN);
                memcpy(listenAParams->NfcId1, ptx_NSC_RfDiscover_ListenA_NfcId1, PTX_NSC_NFCID1_LEN_1 + 1u); /* NFCID1_4bytes + BCC. */
                listenAParams->NfcId1Size = PTX_NSC_NFCID1_LEN_1;

                listenAParams->SelRes = PTX_NSC_RF_DISCOVER_LISTEN_A_SELRES_T4T;

                if (0 != cfg->ListenNfcDep)
                {
                    listenAParams->SelRes = (listenAParams->SelRes | PTX_NSC_RF_DISCOVER_LISTEN_A_SELRES_NFCDEP);
                }

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_ListenB:
            {
                ptx_NSC_RfDiscover_Param_Listen_B_t *listenBParams = &params->Value.ListenB;

                listenBParams->EnableListenB = NSC_ListenB_Enabled;
                listenBParams->RandomIdEnable = 0x00; /* Disable Random Id. */
                listenBParams->SensBRes[0] = 0x50; /* Fixed value. */
                memcpy(&listenBParams->SensBRes[PTX_NSC_RF_DISCOVER_NFCID0_OFFSET], ptx_NSC_RfDiscover_ListenB_NfcId0, PTX_NSC_RF_DISCOVER_NFCID0_LEN);
                memcpy(&listenBParams->SensBRes[PTX_NSC_RF_DISCOVER_APPDATA_OFFSET], ptx_NSC_RfDiscover_ListenB_ApplicationData, PTX_NSC_RF_DISCOVER_APPDATA_LEN);
                memcpy(&listenBParams->SensBRes[PTX_NSC_RF_DISCOVER_PROTINFO0_OFFSET], ptx_NSC_RfDiscover_ListenB_ProtocolInfo, PTX_NSC_RF_DISCOVER_PROTOCOLINFO_LEN);

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_ListenF:
            {
                ptx_NSC_RfDiscover_Param_Listen_F_t *listenFParams = &params->Value.ListenF;

                listenFParams->EnableListenF212 = NSC_ListenF212_Enabled;
                listenFParams->EnableListenF424 = NSC_ListenF424_Disabled;
                listenFParams->HandleSensfRq    = NSC_ListenF_Handler_NscDevice;
                listenFParams->IncludeRDBytes   = NSC_ListenF_DoNotIncludeRD;

                listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_SENSFRES_LEN_OFFSET] = PTX_NSC_RF_DISCOVER_SENSFRES_LEN;
                listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_FIX_VAL_OFF] = PTX_NSC_RF_DISCOVER_FIX_VAL;
                memcpy(&listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_NFCID2_OFFSET], ptx_NSC_RfDiscover_ListenF_NfcId2, PTX_NSC_RF_DISCOVER_NFCID2_LEN);
                memcpy(&listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_PAD0_OFFSET], ptx_NSC_RfDiscover_ListenF_Pad0, PTX_NSC_RF_DISCOVER_PAD0_LEN);
                memcpy(&listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_PAD1_OFFSET], ptx_NSC_RfDiscover_ListenF_Pad1, PTX_NSC_RF_DISCOVER_PAD1_LEN);
                listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_MRTICHECK_OFFSET] = 0xFF;   /* Maximum MRT. */
                listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_MRTIUPDATE_OFFSET] = 0xFF;  /* Maximum MRT. */
                memcpy(&listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_PAD2_OFFSET], ptx_NSC_RfDiscover_ListenF_Pad2, PTX_NSC_RF_DISCOVER_PAD2_LEN);

                if ((NSC_ListenNfcDep_Both_Enabled == cfg->ListenNfcDep) || (NSC_ListenNfcDep_NFCF_Enabled == cfg->ListenNfcDep))
                {
                    listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_NFCID2_CAP_OFFSET] = PTX_NSC_RF_DISCOVER_NFCID2_CAP_NFC_DEP;
                    memcpy(&listenFParams->SensFRes[PTX_NSC_RF_DISCOVER_PAD0_OFFSET], ptx_NSC_RfDiscover_ListenF_Pad0_NfcDep, PTX_NSC_RF_DISCOVER_PAD0_LEN);
                }

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_ListenIsoDep:
            {
                ptx_NSC_RfDiscover_Param_Listen_IsoDep_t *listenIsoParams = &params->Value.ListenIsoDep;

                listenIsoParams->EnableListenT4AT = NSC_ListenIsoDep_T4AT_Enabled;
                listenIsoParams->EnableListenT4BT = NSC_ListenIsoDep_T4BT_Enabled;
                memcpy(&listenIsoParams->ATS[0], ptx_NSC_RfDiscover_ListenIsoDep_ATS, ptx_NSC_RfDiscover_ListenIsoDep_ATS[0]); /* ATS contains it's own length. */
                listenIsoParams->AttribResLen = 2u; /* 1 byte + empty higher-Layer response. */
                memcpy(&listenIsoParams->AttribRes[0], ptx_NSC_RfDiscover_ListenIsoDep_AttribRes, listenIsoParams->AttribResLen);

                status = PTX_STATUS_SUCCESS;

                break;
            }

            case NSC_RfDiscover_Param_Type_ListenNfcDep:
            {
                ptx_NSC_RfDiscover_Param_Listen_NfcDep_t *listenNfcParams = &params->Value.ListenNfcDep;
                uint8_t idx = 0;

                listenNfcParams->EnableListenNfcDepA = NSC_ListenNfcDep_NFCA_Enabled;
                listenNfcParams->EnableListenNfcDepF = NSC_ListenNfcDep_NFCF_Enabled;
                listenNfcParams->AtrRes[idx++] = 0x00; /* Lowest bit-rate enabled. */
                listenNfcParams->AtrRes[idx++] = 0x00; /* Lowest bit-rate enabled. */
                listenNfcParams->AtrRes[idx++] = 0x0A; /* Maximum waiting time. */
                listenNfcParams->AtrRes[idx++] = 0x30; /* Set maximum frame 254 byte, no general bytes. */
                listenNfcParams->AtrResLen = idx;

                status = PTX_STATUS_SUCCESS;

                break;
            }

            default:
            {
                /* Invalid parameter. */
                break;
            }
        }
    }

    return status;
}
