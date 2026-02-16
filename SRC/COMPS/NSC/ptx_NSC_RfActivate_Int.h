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
    Module      : NFC Soft Controller (NSC) Internal Interface Header for RF Activation
    File        : ptx_NSC_RfActivate_Int.h

    Description : Definition of NFC Soft Controller Internal Interface for RF Activation.
                  Contains definition of types specific for RF Activation interface.
*/

/**
 * \addtogroup NSC_RF_ACTIVATE
 *
 * @{
 */

#if !defined(PTX_NSC_RFACTIVATE_INT_H)
#define PTX_NSC_RFACTIVATE_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_RfDiscover_Int.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
/**
 * \name length and offset definitions.
 * @{
 */
#define PTX_NSC_RF_ACTIVATE_BUFFER_SIZE             (34u)       /**< Maximum length of the message buffer. OPCode 1By + RF_TECH 1By + max PARAMS 14By (Typ B) + RF_PROT 1By + max PARAMS 17By */
#define PTX_NSC_NFCID0_LEN                          (4u)        /**< NSC NFCID0 length */
#define PTX_NSC_NFCID2_LEN                          (8u)        /**< NSC NFCID2 length */
#define PTX_NSC_TYPEV_UID_LEN                       (8u)        /**< NSC Type V UID length */

#define PTX_NSC_RFACTIVATE_POLLA_ATS_LEN            (21u)       /**< NFC-A poll mode ISO-DEP notification parameter: ATS length */
#define PTX_NSC_RFACTIVATE_ATTRIBINF_LEN            (15u)       /**< NSC Type ATTRIBINF length */
#define PTX_NSC_RFACTIVATE_GENBYTES_LEN             (15u)       /**< NSC Type General Bytes length */

#define PTX_NSC_RFACTIVATE_POLLB_ATTRIBRES_LEN      (16u)       /**< The length of response to the ATTRIB command */
#define PTX_NSC_RFACTIVATE_LISTENB_ATTRIB_LEN       (24u)       /**< The maximum length of the ATTRIB command */

#define PTX_NSC_RFACTIVATE_ATS_MIN_LEN              (1u)        /**< NFC-A poll mode minimum ATS parameter length */
#define PTX_NSC_RFACTIVATE_ATS_T0_IDX               (1u)        /**< The index of T0 parameter in the ATS (RATS response) */
#define PTX_NSC_RFACTIVATE_ATS_TB_IDX               (3u)        /**< The index of TB parameter in the ATS (RATS response) */
#define PTX_NSC_RFACTIVATE_ATS_FSCI_DEFAULT         (0x08u)     /**< Default value for FSCI in ATS */
#define PTX_NSC_RFACTIVATE_ATS_FWI_DEFAULT          (0x04u)     /**< Default value for FWI in ATS */
#define PTX_NSC_RFACTIVATE_ATS_FWI_RFU              (0x0Fu)     /**< RFU value for FWI in ATS */
#define PTX_NSC_RFACTIVATE_ATS_FWI_MASK             (0xF0)      /**< NSC ATS FWI */

#define PTX_NSC_RFACTIVATE_ATRRES_LEN               (65U)       /**< Maximum length of the Response to ATR_REQ command including the length parameter LEN */
#define PTX_NSC_RFACTIVATE_ATRREQ_LEN               (65U)       /**< Maximum length of the ATR_REQ command including the length parameter LEN */
/** @} */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Describes the NFC-A RF protocol state of the remote device.
 */
typedef enum ptx_NSC_RfAct_PollMode_State_A
{
    ptx_NSC_RfAct_PollMode_State_A_Active = 0, /**< Active state. */
    ptx_NSC_RfAct_PollMode_State_A_Sleep = 1   /**< Sleep state. */
} ptx_NSC_RfAct_PollMode_State_A_t;

/**
 * \brief Describes the NFC-B RF protocol state of the remote device.
 */
typedef enum ptx_NSC_RfAct_PollMode_State_B
{
    ptx_NSC_RfAct_PollMode_State_B_Ready = 0, /**< Ready state. */
    ptx_NSC_RfAct_PollMode_State_B_Sleep = 1  /**< Sleep state. */
} ptx_NSC_RfAct_PollMode_State_B_t;

/**
 * \brief Describes the NFC-F RF protocol state of the remote device.
 */
typedef enum ptx_NSC_RfAct_PollMode_State_F
{
    ptx_NSC_RfAct_PollMode_State_F_Ready_Af = 0, /**< Ready state. */
    ptx_NSC_RfAct_PollMode_State_F_Sleep_Af = 1  /**< Sleep state. */
} ptx_NSC_RfAct_PollMode_State_F_t;

/**
 * \brief Describes the NFC-F RF protocol state of the remote device.
 */
typedef enum ptx_NSC_RfAct_PollMode_BitRate_F
{
    ptx_NSC_RfAct_PollMode_BitRate_212 = 1, /**< BitRate 212 kbps */
    ptx_NSC_RfAct_PollMode_BitRate_424 = 2  /**< BitRate 424 kbps */
} ptx_NSC_RfAct_PollMode_BitRate_F_t;

/**
 * \brief Describes the NSC-Rf Technology A Activation Parameters.
 */
typedef struct ptx_NSC_RfTech_A_Param
{
    ptx_NSC_RfAct_PollMode_State_A_t    RfState;                        /**< RF Device State. */
    uint8_t                             NfcId1Len;                      /**< Length of \ref NfcId1. */
    uint8_t                             NfcId1[PTX_NSC_NFCID1_LEN_3];   /**< Buffer containing NFCID1. */
} ptx_NSC_RfTech_A_Param_t;

/**
 * \brief Describes the NSC Rf-Tech B Activation Parameters.
 */
typedef struct ptx_NSC_RfTech_B_Param
{
    ptx_NSC_RfAct_PollMode_State_B_t    RfState;                    /**< RF Device State. */
    uint8_t                             SensBRes[PTX_NSC_RF_DISCOVER_SENSBRES_LEN]; /**< Response to SENSB_REQ/ALLB_REQ command. */
} ptx_NSC_RfTech_B_Param_t;

/**
 * \brief Describes the NSC Rf-Tech B Activation Parameters.
 */
typedef struct ptx_NSC_RfTech_F_Param
{
    ptx_NSC_RfAct_PollMode_State_F_t    RfState;                    /**< RF Device State. */
    ptx_NSC_RfAct_PollMode_BitRate_F_t  BitRate;                    /**< RF bit-rate */
    uint8_t                             NfcId2[PTX_NSC_NFCID2_LEN]; /**< Buffer containing NFCID0. */
} ptx_NSC_RfTech_F_Param_t;

/**
 * \brief Describes the NSC RF Technology Activation Parameters.
 *
 * \note No specific activation parameters are required for NFC-V.
 */
typedef union ptx_NSC_RfTech_Param
{
    ptx_NSC_RfTech_A_Param_t    RfAct_A;    /**< NSC RF Technology A Activation Parameters. */
    ptx_NSC_RfTech_B_Param_t    RfAct_B;    /**< NSC RF Technology B Activation Parameters. */
    ptx_NSC_RfTech_F_Param_t    RfAct_F;    /**< NSC RF Technology F Activation Parameters. */
} ptx_NSC_RfTech_Param_t;

/**
 * \brief NSC RF Activate structure.
 */
typedef struct ptx_NSC_RfProt_Param_IsoDep
{
    uint8_t HbrEnable;                                          /**< Enable higher bit-rates.         */
    uint8_t AttribInfLen;                                       /**< Length of ATTRIB_INF parameter. */
    uint8_t AttribInf[PTX_NSC_RFACTIVATE_ATTRIBINF_LEN];        /**< ATTRIB_INF parameter.           */
} ptx_NSC_RfProtocol_Param_IsoDep_t;

/**
 * \brief NSC RF Activate structure.
 */
typedef struct ptx_NSC_RfProt_Param_NfcDep
{
    uint8_t HbrEnable;                                          /**< Enable higher bit-rates.         */
    uint8_t GeneralBytesLen;                                    /**< Length of ATTRIB_INF parameter. */
    uint8_t GeneralBytes[PTX_NSC_RFACTIVATE_GENBYTES_LEN];      /**< General Bytes parameter.           */
} ptx_NSC_RfProtocol_Param_NfcDep_t;

/**
 * \brief NSC RF Protocol Activation Parameters.
 */
typedef union ptx_NSC_RfProt_Param
{
    ptx_NSC_RfProtocol_Param_IsoDep_t IsoDep;   /**< ISO-DEP Parameters. */
    ptx_NSC_RfProtocol_Param_NfcDep_t NfcDep;   /**< NFC-DEP Parameters. */
} ptx_NSC_RfProtocol_Param_t;

/**
 * \brief NSC RF Activate structure.
 */
typedef struct ptx_NSC_RfAct_Param
{
    uint8_t                     RfTech;         /**< RF Technology. */
    ptx_NSC_RfTech_Param_t      RfTechParams;   /**< RF Technology specific activation parameters. */
    ptx_NSC_RfProtocol_t        RfProt;         /**< RF Protocol. */
    ptx_NSC_RfProtocol_Param_t  RfProtParams;   /**< RF Protocol specific parameters. */
    uint8_t                     RfLisRoutingInfo;   /**< RF Listen Routing Table Info. */
} ptx_NSC_RfAct_Param_t;

/**
 * \brief ISO DEP Protocol Poll Parameter for Type A.
 */
typedef struct ptx_NSC_RfAct_IsoDep_ProtPollParamA
{
    uint8_t ATS[PTX_NSC_RFACTIVATE_POLLA_ATS_LEN];              /**< RATS Response. */
    uint8_t PPS1;                                               /**< PPS1 parameter of the PPS command. */
    uint8_t FSCI;                                               /**< FSCI parameter. */
} ptx_NSC_RfAct_IsoDep_ProtPollParamA_t;

/**
 * \brief ISO DEP Protocol Poll Parameter for Type B.
 */
typedef struct ptx_NSC_RfAct_IsoDep_ProtPollParamB
{
    uint8_t AttribParam1;                                       /**< Param 1 of the ATTRIB command. */
    uint8_t AttribParam2;                                       /**< Param 2 of the ATTRIB command. */
    uint8_t AttribResLen;                                       /**< Length of the ATTRIB_RES parameter. */
    uint8_t AttribRes[PTX_NSC_RFACTIVATE_POLLB_ATTRIBRES_LEN];  /**< Response to the ATTRIB command. */
} ptx_NSC_RfAct_IsoDep_ProtPollParamB_t;

/**
 * \brief ISO DEP Protocol Listen Parameter for Type A.
 */
typedef struct ptx_NSC_RfAct_IsoDep_ProtListenParamA
{
    uint8_t Rats;   /**< RATS parameter */
    uint8_t Pps1;   /**< PPS1 parameter of the PPS command */
} ptx_NSC_RfAct_IsoDep_ProtListenParamA_t;

/**
 * \brief ISO DEP Protocol Listen Parameter for Type B.
 */
typedef struct ptx_NSC_RfAct_IsoDep_ProtListenParamB
{
    uint8_t AttribLen;                                      /**< Length of ATTRIB command */
    uint8_t Attrib[PTX_NSC_RFACTIVATE_LISTENB_ATTRIB_LEN];  /**< Attrib-command used during device activation */
} ptx_NSC_RfAct_IsoDep_ProtListenParamB_t;

/**
 * \brief ISO DEP Protocol Union for Type A and Type B.
 */
typedef union ptx_NSC_RfAct_Param_IsoDep_ProtParam
{
    ptx_NSC_RfAct_IsoDep_ProtPollParamA_t   PollA;    /**< Poll A. */
    ptx_NSC_RfAct_IsoDep_ProtPollParamB_t   PollB;    /**< Poll B. */
    ptx_NSC_RfAct_IsoDep_ProtListenParamA_t ListenA;  /**< Listen A */
    ptx_NSC_RfAct_IsoDep_ProtListenParamB_t ListenB;  /**< Listen B */
} ptx_NSC_RfAct_IsoDep_ProtParam_t;

/**
 * \brief NFC DEP Protocol Poll Parameter for Type A and F.
 */
typedef struct ptx_NSC_RfAct_NfcDep_ProtPollParamAF
{
    uint8_t AtrRes[PTX_NSC_RFACTIVATE_ATRRES_LEN];      /**< Response to ATR_REQ command including the length parameter */
    uint8_t Brs;                                        /**< BRS parameter of the PSL_REQ command */
} ptx_NSC_RfAct_NfcDep_ProtPollParamAF_t;

/**
 * \brief NFC DEP Protocol Listen Parameter for Type A and F.
 */
typedef struct ptx_NSC_RfAct_NfcDep_ProtListenParamAF
{
    uint8_t AtrReq[PTX_NSC_RFACTIVATE_ATRREQ_LEN];      /**< ATR_REQ command including the length parameter */
    uint8_t Brs;                                        /**< BRS parameter of the PSL_REQ command used during device activation */
    uint8_t Lr;                                         /**< LR parameter of the PSL_REQ command used during device activation */
} ptx_NSC_RfAct_NfcDep_ProtListenParamAF_t;

/**
 * \brief NFC DEP Protocol Union for Type A and Type B.
 */
typedef union ptx_NSC_RfAct_NfcDep_ProtParam
{
    ptx_NSC_RfAct_NfcDep_ProtPollParamAF_t      PollAF;     /**< Poll A and Poll F */
    ptx_NSC_RfAct_NfcDep_ProtListenParamAF_t    ListenAF;   /**< Listen A and Listen F */
} ptx_NSC_RfAct_NfcDep_ProtParam_t;

/**
 * \brief NFC ACT Protocol Union
*/
typedef union ptx_NSC_RfAct_ProtParam
{
    ptx_NSC_RfAct_IsoDep_ProtParam_t        IsoDep;         /**< ISO DEP protocol specific parameters */
    ptx_NSC_RfAct_NfcDep_ProtParam_t        NfcDep;         /**< NFC DEP protocol specific parameters */
} ptx_NSC_RfAct_ProtParam_t;

/**
 * \brief NSC RF Activate structure.
 */
typedef struct ptx_NSC_RfActivate_Ntf
{
    uint8_t                                 RfTech;          /**< RF Technology.                                 */
    ptx_NSC_RfDiscover_Ntf_PollParam_t      RfTechParams;    /**< RF Technology specific activation parameters.  */
    ptx_NSC_RfProtocol_t                    RfProt;          /**< RF Protocol.                                   */
    ptx_NSC_RfAct_ProtParam_t               RfProtParams;    /**< RF Protocol specific parameters.               */
    uint8_t                                 RfRoutingAction; /**< RF Routing Action parameter (Listen-Mode only).*/
} ptx_NSC_RfActivate_Ntf_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
