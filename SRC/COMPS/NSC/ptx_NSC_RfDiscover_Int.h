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
    Module      : NFC Soft Controller (NSC) Internal Interface Header for RF Discovery
    File        : ptx_NSC_RfDiscover_Int.h

    Description : Definition of NFC Soft Controller Internal Interface for RF Discovery.
                  Contains definition of types specific for RF Discovery interface.
*/

/**
 * \addtogroup NSC_RF_DISCOVER
 *
 * @{
 */

#if !defined(PTX_NSC_RFDISCOVER_INT_H)
#define PTX_NSC_RFDISCOVER_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \addtogroup NSC_RF_DISCOVER_COMMON
 *
 * @{
 */
/**
 * \name Length and Offset definitions.
 * @{
 */
#define PTX_NSC_RF_DISCOVER_IDLE_TIME_LEN           (3u)    /**< Length of the idle time parameter. */
#define PTX_NSC_RF_DISCOVER_STDBY_WU_CONF_LEN       (2u)    /**< Length of the standby/wake-up parameter. */
#define PTX_NSC_RF_DISCOVER_ATTRIBINF_LEN           (15U)   /**< NSC Type ATTRIBINF length */
#define PTX_NSC_RF_DISCOVER_PARAM2_OFFSET           (6U)    /**< NSC Type B PARAM2 offset within ATTRIB */
#define PTX_NSC_RF_DISCOVER_ATRREQ_GEN_LEN          (15U)   /**< NSC Type ATR REQ GEN length */
#define PTX_NSC_RF_DISCOVER_SENSRES_LEN             (2U)    /**< NSC Type SENSRES length */
#define PTX_NSC_RF_DISCOVER_SELRES_LEN              (1u)    /**< NSC Type A SELRES length. */
#define PTX_NSC_RF_DISCOVER_SENSBRES_LEN            (13U)   /**< NSC Type B SENSRES length */
#define PTX_NSC_RF_DISCOVER_NFCID0_LEN              (4u)    /**< NSC Type B NFCID0 length. */
#define PTX_NSC_RF_DISCOVER_APPDATA_LEN             (4u)    /**< NSC Type B Application Data length. */
#define PTX_NSC_RF_DISCOVER_PROTOCOLINFO_LEN        (4u)    /**< NSC Type B Protocol Info length. */
#define PTX_NSC_RF_DISCOVER_NFCID0_OFFSET           (1u)    /**< NSC Type B NFCID0 offset within SENSB_RES. */
#define PTX_NSC_RF_DISCOVER_APPDATA_OFFSET          (5u)    /**< NSC Type B Application Data offset within SENSB_RES. */
#define PTX_NSC_RF_DISCOVER_PROTINFO0_OFFSET        (9u)    /**< NSC Type B Protocol Info offset within SENSB_RES. */
#define PTX_NSC_RF_DISCOVER_PROTINFO1_OFFSET        (10u)   /**< NSC Type B Protocol Info byte 1 offset within SENSB_RES. */
#define PTX_NSC_RF_DISCOVER_PROTINFO2_OFFSET        (11u)   /**< NSC Type B Protocol Info byte 2 offset within SENSB_RES. */
#define PTX_NSC_RF_DISCOVER_PINFO1_FSCI_MASK        (0xF0)  /**< NSC Type B Protocol Info byte 1 FSCI mask. */
#define PTX_NSC_RF_DISCOVER_PINFO2_FWI_MASK         (0xF0)  /**< NSC Type B Protocol Info byte 2 FWI mask. */
#define PTX_NSC_RF_DISCOVER_RATS_FSDI_MASK          (0xF0)  /**< NSC Type A Protocol RATS byte 1 FSDI mask. */
#define PTX_NSC_RF_DISCOVER_ATRRIB_FSDI_MASK        (0x0F)  /**< NSC Type B Protocol ATTRIB byte 0 FSDI mask. */
#define PTX_NSC_RF_DISCOVER_SENSFRES_LEN_OFFSET     (0u)    /**< NSC Type F SENSF_RES length offset */
#define PTX_NSC_RF_DISCOVER_SENSFRES_LEN            (18U)   /**< NSC Type F SENSF_RES length */
#define PTX_NSC_RF_DISCOVER_SENSFREQ_SC_LEN         (2U)    /**< NSC Type F SENSF_REQ System Code length */
#define PTX_NSC_RF_DISCOVER_SENSFRES_MAXLEN         (20U)   /**< NSC Type F SENSRES maximum length, including the length byte. */
#define PTX_NSC_RF_DISCOVER_FIX_VAL_OFF             (1u)    /**< NSC Type F SENSF_RES fixed value offset */
#define PTX_NSC_RF_DISCOVER_FIX_VAL                 (1u)    /**< NSC Type F SENSF_RES fixed value */
#define PTX_NSC_RF_DISCOVER_NFCID2_LEN              (8u)    /**< NSC Type F NFCID2 length. */
#define PTX_NSC_RF_DISCOVER_NFCID2_OFFSET           (2u)    /**< NSC Type F NFCID2 offset within SENSF_RES. */
#define PTX_NSC_RF_DISCOVER_NFCID2_CAP_OFFSET       (2u)    /**< NSC Type F NFCID2 capability offset within SENSF_RES. (Determines support for NFC-DEP) */
#define PTX_NSC_RF_DISCOVER_NFCID2_CAP_NFC_DEP      (1u)    /**< NSC Type F capability indicator for NFC-DEP protocol in NFCID2 */
#define PTX_NSC_RF_DISCOVER_PAD0_LEN                (2u)    /**< NSC Type F PAD0 length. */
#define PTX_NSC_RF_DISCOVER_PAD0_OFFSET             (10u)   /**< NSC Type F PAD0 offset within SENSF_RES. */
#define PTX_NSC_RF_DISCOVER_PAD1_LEN                (3u)    /**< NSC Type F PAD1 length. */
#define PTX_NSC_RF_DISCOVER_PAD1_OFFSET             (12u)   /**< NSC Type F PAD1 offset within SENSF_RES. */
#define PTX_NSC_RF_DISCOVER_PAD2_LEN                (1u)    /**< NSC Type F PAD2 length. */
#define PTX_NSC_RF_DISCOVER_PAD2_OFFSET             (17u)   /**< NSC Type F PAD2 offset within SENSF_RES. */
#define PTX_NSC_RF_DISCOVER_MRTICHECK_OFFSET        (15u)   /**< NSC Type F MRTI_Check offset within SENSF_RES. */
#define PTX_NSC_RF_DISCOVER_MRTIUPDATE_OFFSET       (16u)   /**< NSC Type F MRTI_Update offset within SENSF_RES. */
#define PTX_NSC_RF_DISCOVER_RD_LEN                  (2u)    /**< NSC Type F RD length. */
#define PTX_NSC_RF_DISCOVER_RD_OFFSET               (18u)   /**< NSC Type F RD offset within SENSF_RES. */
#define PTX_NSC_RF_DISCOVER_INVENTORY_RES_LEN       (10U)   /**< NSC Type V INVENTORY length. */
#define PTX_NSC_RF_DISCOVER_UID_LEN                 (8u)    /**< NSC Type V UID length. */
#define PTX_NSC_RF_DISCOVER_UID_OFFSET              (2u)    /**< NSC Type V UID offset within INVENTORY_RES. */
#define PTX_NSC_RF_DISCOVER_ATS_HDR_LEN             (5U)    /**< NSC Type IsoDep ATS header length. */
#define PTX_NSC_RF_DISCOVER_ATS_LEN                 (20U)   /**< NSC Type IsoDep ATS maximum length. */
#define PTX_NSC_RF_DISCOVER_ATTRIBRES_LEN           (16u)   /**< NSC Type IsoDep ATTRIB_RES maximum length. */
#define PTX_NSC_RF_DISCOVER_ATR_RES_MIN_LEN         (4U)    /**< NSC Type NfcDep ATR_RES Minimum length. */
#define PTX_NSC_RF_DISCOVER_ATR_RES_LEN             (19U)   /**< NSC Type NfcDep ATR_RES length. */
#define PTX_NSC_RF_DISCOVER_ATR_RES_G_BYTES_LEN     (15U)   /**< NSC Type NfcDep G-BYTES length from ATR_RES, as defined in NSC Spec. */
#define PTX_NSC_RF_DISCOVER_ATR_RES_G_BYTES_OFFSET  (4U)    /**< NSC Type NfcDep B-BYTES ATR_RES Offset, as defined in NSC Spec. */
#define PTX_NSC_RF_DISCOVER_ATR_RES_TO_OFFSET       (2U)    /**< NSC Type NfcDep TO Byte ATR_RES Offset, as defined in NSC Spec. */
#define PTX_NSC_RF_DISCOVER_ATR_RES_TO_WT_MASK      (0x0F)  /**< NSC Type NfcDep TO Byte ATR_RES Bitmask for WT, as defined in NSC Spec. */
#define PTX_NSC_RF_DISCOVER_ATR_RES_PP_OFFSET       (3U)    /**< NSC Type NfcDep PP Byte ATR_RES Offset, as defined in NSC Spec. */
#define PTX_NSC_RF_DISCOVER_ATR_RES_PP_LR_MASK      (0x30)  /**< NSC Type NfcDep PP Byte ATR_RES Bitmask for LR, as defined in NSC Spec. */
#define PTX_NSC_RF_DISCOVER_ATR_RES_PP_GT_MASK      (0x02)  /**< NSC Type NfcDep PP Byte ATR_RES Bitmask for GT, as defined in NSC Spec. */
#define PTX_NSC_RF_DISCOVER_SENSFRES_MINLEN         (18U)   /**< NSC Type F SENSRES minimum length, including the length byte. */
#define PTX_NSC_RF_DISCOVER_ATR_REQG_MAXLEN         (15U)   /**< NSC NFC-ACM Poll Mode ATR_REQ G (General bytes) maximum length. */
#define PTX_NSC_RF_DISCOVER_LISTEN_LR_MASK          (0x30)  /**< NSC Listen Mode NFC-DEP LR value mask */

#define PTX_NSC_RF_DISCOVER_MAX_PARAMCOUNT          (15u)   /**< Maximum number of parameters accepted to build NSC_RF_DISCOVER_CMD message.
                                                             not more than PTX_NSCDISC_MAX_PARAMCOUNT parameters can be put
                                                             into one NSC_RF_DISCOVER_CMD.  */

/** @} */

/**
 * \name NFCID1 definitions.
 * @{
 */
#define PTX_NSC_NFCID1_LEN_0                        (0u)        /**< NSC NFCID1 length 0 */
#define PTX_NSC_NFCID1_LEN_1                        (4u)        /**< NSC NFCID1 length 1 */
#define PTX_NSC_NFCID1_LEN_2                        (7u)        /**< NSC NFCID1 length 2 */
#define PTX_NSC_NFCID1_LEN_3                        (10u)       /**< NSC NFCID1 length 3 */
#define PTX_NSC_RF_DISCOVER_NFCID1_MAX_LEN          PTX_NSC_NFCID1_LEN_3 /**< NSC maximum NFCID1 length */
/** @} */
/** @} */

/**
 * \name RF-Tech definitions.
 * @{
 */
#define PTX_NSC_TYPES_TECH_POLL_A                   (0x01u)     /**< NSC Type Technology Poll A */
#define PTX_NSC_TYPES_TECH_POLL_B                   (0x02u)     /**< NSC Type Technology Poll B */
#define PTX_NSC_TYPES_TECH_POLL_F                   (0x03u)     /**< NSC Type Technology Poll F */
#define PTX_NSC_TYPES_TECH_POLL_V                   (0x04u)     /**< NSC Type Technology Poll V */
#define PTX_NSC_TYPES_TECH_POLL_ACM                 (0x05u)     /**< NSC Type Technology Poll ACM */

#define PTX_NSC_TYPES_TECH_LISTEN_A                 (0x11u)     /**< NSC Type Technology Listen A */
#define PTX_NSC_TYPES_TECH_LISTEN_B                 (0x12u)     /**< NSC Type Technology Listen B */
#define PTX_NSC_TYPES_TECH_LISTEN_F                 (0x13u)     /**< NSC Type Technology Listen F */
#define PTX_NSC_TYPES_TECH_LISTEN_ACM               (0x15u)     /**< NSC Type Technology Listen ACM */
/** @} */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \addtogroup NSC_RF_DISCOVER_COMMON
 *
 * @{
 */

/**
 * \brief Type of RF Protocol.
 */
typedef enum ptx_NSC_RfProtocol
{
    ptx_NSC_RfProtocol_Undefined  = (0x00u),            /**< RF Protocol Undefined. */
    ptx_NSC_RfProtocol_T2T        = (0x02u),            /**< RF Protocol T2T. */
    ptx_NSC_RfProtocol_T3T        = (0x03u),            /**< RF Protocol T3T. */
    ptx_NSC_RfProtocol_IsoDep     = (0x04u),            /**< RF Protocol ISO-DEP. */
    ptx_NSC_RfProtocol_T5T        = (0x05u),            /**< RF Protocol T5T. */
    ptx_NSC_RfProtocol_NfcDep     = (0x06u),            /**< RF Protocol NFC-DEP. */
} ptx_NSC_RfProtocol_t;

/**
 * \brief Type of RF Technology.
 */
typedef enum ptx_NSC_RfTech
{
    ptx_NSC_RfTech_Passive_Poll_A      = PTX_NSC_TYPES_TECH_POLL_A,        /**< Poll A Technology. */
    ptx_NSC_RfTech_Passive_Poll_B      = PTX_NSC_TYPES_TECH_POLL_B,        /**< Poll B Technology. */
    ptx_NSC_RfTech_Passive_Poll_F      = PTX_NSC_TYPES_TECH_POLL_F,        /**< Poll F Technology. */
    ptx_NSC_RfTech_Passive_Poll_V      = PTX_NSC_TYPES_TECH_POLL_V,        /**< Poll V Technology. */
    ptx_NSC_RfTech_Active_Poll         = PTX_NSC_TYPES_TECH_POLL_ACM,      /**< Poll NFC-ACM Initiator Mode. */
    ptx_NSC_RfTech_Passive_Listen_A    = PTX_NSC_TYPES_TECH_LISTEN_A,      /**< Listen A Technology. */
    ptx_NSC_RfTech_Passive_Listen_B    = PTX_NSC_TYPES_TECH_LISTEN_B,      /**< Listen B Technology. */
    ptx_NSC_RfTech_Passive_Listen_F    = PTX_NSC_TYPES_TECH_LISTEN_F,      /**< Listen F Technology. */
    ptx_NSC_RfTech_Active_Listen       = PTX_NSC_TYPES_TECH_LISTEN_ACM,    /**< Listen NFC-ACM Target mode. */
    ptx_NSC_RfTech_RfTech_Undefined                                        /**< Undefined Rf-Tech. */
} ptx_NSC_RfTech_t;

/**
 * \brief The parameters of the discover command are grouped into logical subsets (POLL, POLL_A, POLL_B, …).
 * ptx_NSC_RfDiscover_Param_Type_t describes the IDs for the individual subset.
 */
typedef enum ptx_NSC_RfDiscover_Param_Type
{
    NSC_RfDiscover_Param_Type_EoC           = 0x00,   /**< End of command. */

    NSC_RfDiscover_Param_Type_Poll          = 0x01,   /**< Poll. */
    NSC_RfDiscover_Param_Type_PollAcm       = 0x02,   /**< ACM. */
    NSC_RfDiscover_Param_Type_PollA         = 0x03,   /**< Poll type A. */
    NSC_RfDiscover_Param_Type_PollB         = 0x04,   /**< Poll type B */
    NSC_RfDiscover_Param_Type_PollF         = 0x05,   /**< Poll type F. */
    NSC_RfDiscover_Param_Type_PollV         = 0x06,   /**< Poll type V. */
    NSC_RfDiscover_Param_Type_PollIsoDep    = 0x08,   /**< Poll ISO DEP protocol. */
    NSC_RfDiscover_Param_Type_PollNfcDep    = 0x09,   /**< Poll NFC DEP protocol. */

    NSC_RfDiscover_Param_Type_PollMax       = 0x0A,   /**< Poll parameters safeguard. */

    NSC_RfDiscover_Param_Type_Listen        = 0x10,   /**< Listen. */
    NSC_RfDiscover_Param_Type_ListenA       = 0x11,   /**< ListenA. */
    NSC_RfDiscover_Param_Type_ListenB       = 0x12,   /**< ListenB. */
    NSC_RfDiscover_Param_Type_ListenF       = 0x13,   /**< ListenF. */
    NSC_RfDiscover_Param_Type_ListenIsoDep  = 0x15,   /**< ListenIsoDep. */
    NSC_RfDiscover_Param_Type_ListenNfcDep  = 0x16,   /**< ListenNfcDep. */

    NSC_RfDiscover_Param_Type_Max           = 0x17,   /**< Safeguard. */
} ptx_NSC_RfDiscover_Param_Type_t;

/**
 * \brief Polling modes.
 */
typedef enum ptx_NSC_RfDiscover_PollMode
{
    NSC_RfDiscover_PollMode_Disable    = 0x00,  /**< Poll mode disabled. */
    NSC_RfDiscover_PollMode_NfcForum   = 0x01,  /**< NFC Forum poll mode. */
    NSC_RfDiscover_PollMode_EMVCo      = 0x02,  /**< EMVCo poll mode. */
    NSC_RfDiscover_PollMode_Iso        = 0x03,  /**< ISO poll mode. */
    NSC_RfDiscover_PollMode_ActivateRf = 0x04,  /**< Activate RF field and move to RF_POLL state. */
    NSC_RfDiscover_PollMode_Lpcd       = 0x05,  /**< LPCD poll mode. */
    NSC_RfDiscover_PollMode_TypeFPropr = 0x06   /**< Proprietary Type-F poll mode (RFU). */
} ptx_NSC_RfDiscover_PollMode_t;

/**
 * \brief Listen Modes.
 */
typedef enum ptx_NSC_RfDiscover_ListenMode
{
    NSC_RfDiscover_ListenMode_Disable = 0x00,     /**< Listen mode disabled. */
    NSC_RfDiscover_ListenMode_Enable  = 0x01,     /**< Listen mode enabled. */
} ptx_NSC_RfDiscover_ListenMode_t;


/**
 * \brief Describes the parameters of the NFC-A.
 * poll mode configuration.
 */
typedef enum ptx_NSC_PollCommand
{
    NSC_PollCommand_REQA = 0x00,        /**< PollCommand REQA. */
    NSC_PollCommand_WUPA = 0x01         /**< PollCommand WUPA. */
} ptx_NSC_PollCommand_t;

/**
 * \brief Describes the parameters of the NFC-F.
 * poll mode configuration.
 */
typedef enum ptx_NSC_BitRate
{
    NSC_BitRate_Undef = 0,  /**< Type F Undefined Bit-rate, deactivate. */
    NSC_BitRate_212   = 1,  /**< Type F BitRate 212 */
    NSC_BitRate_424   = 2   /**< Type F BitRate 424 */
} ptx_NSC_BitRate_t;

/**
 * \brief ptx_NSC_RfDiscover_InitialPollingTech_t defines the
 * initial polling technology for the NFC Forum poll mode.
 */
typedef enum ptx_NSC_RfDiscover_InitialTech
{
    NSC_RfDiscover_InitialTech_A = 0x01, /**< NFC-A. */
    NSC_RfDiscover_InitialTech_B = 0x02, /**< NFC-B. */
    NSC_RfDiscover_InitialTech_F = 0x03, /**< NFC-F. */
    NSC_RfDiscover_InitialTech_V = 0x04  /**< NFC-V. */
} ptx_NSC_RfDiscover_InitialTech_t;

/**
 * \brief Describes the parameters of the general poll mode configuration
 */
typedef struct ptx_NSC_RfDiscover_Param_Poll
{
    ptx_NSC_RfDiscover_PollMode_t   PollMode;                                       /**< Type of polling mode. */
    uint8_t                         GuardTime;                                      /**< Guard time after RF field has been activated (milliseconds). */
    uint8_t                         Ddpc;                                           /**< Enable/Disable Digital Dynamic Power Control. */
    uint8_t                         Lpcd;                                           /**< Enable/Disable Low Power Card Detection.*/
    uint8_t                         IdleTime[PTX_NSC_RF_DISCOVER_IDLE_TIME_LEN];    /**< Time between two consecutive polling cycles (32us units). */
    uint8_t                         StdbyWuCfg[PTX_NSC_RF_DISCOVER_STDBY_WU_CONF_LEN]; /**< Standby and wake-up (WU) configuration. */
    ptx_NSC_RfDiscover_InitialTech_t InitialTech;                                   /**< Sets the initial polling technology for the NFC Forum poll mode. */
    uint8_t                         NtfEnable;                                      /**< Enable transmission of NTFs: LPCD or error notifications. */
    uint8_t                         DevicesLimit;                                   /**< Maximum number of devices to be resolved during NFC poll mode discovery. */
    uint8_t                         RFAmplitudeLimiterEnable;                       /**< Enable/Disable internal RF-Amplitude-Limiter (RFU). */
} ptx_NSC_RfDiscover_Param_Poll_t;

/**
 * \brief General Listen Mode parameters.
 */
typedef struct ptx_NSC_RfDiscover_Param_Listen
{
    ptx_NSC_RfDiscover_ListenMode_t ListenEnable;           /**< Enable Listen Mode. */
    uint8_t                         RfFieldNtfEnable;       /**< Enable Rf-Field notifications. */
} ptx_NSC_RfDiscover_Param_Listen_t;


/**
 * \brief Describes the RF state of the remote device.
 */
typedef enum ptx_NSC_RfDiscover_Ntf_State
{
    ptx_NSC_RfDiscover_Ntf_State_Active = 0,    /**< Active state. */
    ptx_NSC_RfDiscover_Ntf_State_Sleep  = 1     /**< Sleep state. */
} ptx_NSC_RfDiscover_Ntf_State_t;

/** @} */

/**
 * \addtogroup NSC_RF_DISCOVER_NFC_ACM
 *
 * @{
 */

/**
 * \brief NFC-ACM Bit-rate selection parameter.
 */
typedef enum ptx_NSC_PollAcm_BitRate
{
    NSC_PollAcm_BitRate_106 = 0,    /**< NFC-A, 106kbps */
    NSC_PollAcm_BitRate_212 = 1,    /**< NFC-F, 212kbps */
    NSC_PollAcm_BitRate_424 = 2,    /**< NFC-F, 424kbps */
} ptx_NSC_PollAcm_BitRate_t;

/**
 * \brief Describes the parameters of the NFC-ACM poll mode configuration
 */
typedef struct ptx_NSC_RfDiscover_Param_Poll_Acm
{
    uint8_t                     PollFrequency;  /**< Polling frequency (every n cycles; n_max = 10). */
    ptx_NSC_PollAcm_BitRate_t   BitRate;        /**< Bit-rate selection type. */
} ptx_NSC_RfDiscover_Param_Poll_Acm_t;

/** @} */


/**
 * \addtogroup NSC_RF_DISCOVER_NFCA
 *
 * @{
 */

/**
 * \brief Describes the parameters of the NFC-A poll mode configuration
 */
typedef struct ptx_NSC_RfDiscover_Param_Poll_A
{
    uint8_t                 PollFrequency;          /**< Polling frequency (every n cycles; n_max = 10). */
    ptx_NSC_PollCommand_t   PollCommand;            /**< PollCommand type. */
    uint8_t                 BailOut;                /**< Enable/disable bail out when technology is found. */
    uint8_t                 AnticollisionEnable;    /**< Enable/disable resolution process. */
    uint8_t                 StopOnCollision;        /**< Stop (1)/Continue (0) the resolution process in case of collision. */
    uint8_t                 DevicesLimit;           /**< Maximum number of devices resolved during anti-collision. */
} ptx_NSC_RfDiscover_Param_Poll_A_t;

/**
 * \brief Structure representing the RF-Tech payload
 * of the discover notification for NFC-A.
 */
typedef struct ptx_NSC_RfDiscover_Ntf_Param_PollA
{
    uint8_t SensRes[PTX_NSC_RF_DISCOVER_SENSRES_LEN];   /**< Response to SENS_REQ/ALL_REQ. */
    uint8_t NfcId1Len;                                  /**< Length of the NFCID1. */
    uint8_t NfcId1[PTX_NSC_RF_DISCOVER_NFCID1_MAX_LEN]; /**< Response to SENS_REQ/ALL_REQ. */
    uint8_t SelRes;                                     /**< Response to SEL_REQ command.. */
} ptx_NSC_RfDiscover_Ntf_Param_PollA_t;


/**
 * \brief Listen_A parameters.
 */
typedef struct ptx_NSC_RfDiscover_Param_Listen_A
{
    uint8_t EnableListenA;                              /**< Enable Listen_A. */
    uint8_t SensRes[PTX_NSC_RF_DISCOVER_SENSRES_LEN];   /**< SENS_RES. */
    uint8_t SelRes;                                     /**< SEL_RES. */
    uint8_t NfcId1Size;                                 /**< Size of NFCID1. */
    uint8_t NfcId1[PTX_NSC_RF_DISCOVER_NFCID1_MAX_LEN]; /**< NFCID1. */
} ptx_NSC_RfDiscover_Param_Listen_A_t;

/** @} */

/**
 * \addtogroup NSC_RF_DISCOVER_NFCB
 *
 * @{
 */

/**
 * \brief Describes the parameters of the NFC-B poll mode configuration
 */
typedef struct ptx_NSC_RfDiscover_Param_Poll_B
{
    uint8_t PollFrequency;      /**< Polling frequency (every n cycles; n_max = 10). */
    uint8_t SensBReqAfi;        /**< AFI parameter of the SENSB_REQ. */
    uint8_t SensBReqParam;      /**< PARAM parameter of the SENSB_REQ. */
    uint8_t BailOut;            /**< Enable/disable bail out when technology is found. */
    uint8_t StopOnCollision;    /**< Stop (1)/Continue (0) the resolution process in case of collision. */
    uint8_t DevicesLimit;       /**< Maximum number of devices resolved during anti-collision. */
} ptx_NSC_RfDiscover_Param_Poll_B_t;

/**
 * \brief Structure representing the RF-Tech payload of the discover notification for NFC-B
 */
typedef struct ptx_NSC_RfDiscover_Ntf_Param_PollB
{
    uint8_t SensBRes[PTX_NSC_RF_DISCOVER_SENSBRES_LEN];       /**< Response to SENSB_REQ/ALLB_REQ command. */
} ptx_NSC_RfDiscover_Ntf_Param_PollB_t;


/**
 * \brief Listen_B parameters.
 */
typedef struct ptx_NSC_RfDiscover_Param_Listen_B
{
    uint8_t EnableListenB;                               /**< Enable Listen_B. */
    uint8_t RandomIdEnable;                              /**< Enable random NFCID0. */
    uint8_t SensBRes[PTX_NSC_RF_DISCOVER_SENSBRES_LEN];  /**< SENSB_RES. */
} ptx_NSC_RfDiscover_Param_Listen_B_t;
/** @} */

/**
 * \addtogroup NSC_RF_DISCOVER_NFCF
 *
 * @{
 */

/**
 * \brief Describes the parameters of the NFC-F poll mode configuration
 */
typedef struct ptx_NSC_RfDiscover_Param_Poll_F
{
    uint8_t             PollFrequency;                                      /**< Polling frequency (every n cycles; n_max=10). */
    ptx_NSC_BitRate_t   BitRate;                                            /**< Bit Rate, either 212 or 424 kbps */
    uint8_t             BailOut;                                            /**< Enable/disable bail out when technology is found. */
    uint8_t             DevicesLimit;                                       /**< Maximum number of devices resolved during anticollision. */
    uint8_t             SystemCode[PTX_NSC_RF_DISCOVER_SENSFREQ_SC_LEN];	/**< 16-Bit System Code according to FeliCa/T3T. */
    uint8_t             RequestCode;					                    /**< 8-Bit Requested Code according to FeliCa/T3T. */
    uint8_t             InitialTimeSlotNr;				                    /**< 8-Bit Timeslot-Number according to FeliCa/T3T - used only during initial Polling. */
    uint8_t             FieldReset;				                            /**< Performs a reset of the RF-Field before sending the SENSF_REQ (RFU). */
} ptx_NSC_RfDiscover_Param_Poll_F_t;

/**
 * \brief Structure representing the RF-Tech payload of the discover notification for NFC-F
 */
typedef struct ptx_NSC_RfDiscover_Ntf_Param_PollF
{
    uint8_t SensFRes[PTX_NSC_RF_DISCOVER_SENSFRES_MAXLEN];    /**< Response to SENSF_REQ command including the length byte (LEN). */
} ptx_NSC_RfDiscover_Ntf_Param_PollF_t;


/**
 * \brief Listen_F parameters.
 */
typedef struct ptx_NSC_RfDiscover_Param_Listen_F
{
    uint8_t EnableListenF212;                               /**< Enable Listen_F (212 kbps). */
    uint8_t EnableListenF424;                               /**< Enable Listen_F (424 kbps). */
    uint8_t HandleSensfRq;                                  /**< If set to 1, the PTX2xxR/W device SHALL handle SENSF_REQ command in T3T protocol.
                                                                 Otherwise, the HOST is responsible to handle it. */
    uint8_t IncludeRDBytes;                                 /**< If set to 1, the PTX2xxR/W device SHALL include RD bytes in SENSF_RES
                                                                 if it receives SENSF_REQ with RC set to 2. */
    uint8_t SensFRes[PTX_NSC_RF_DISCOVER_SENSFRES_MAXLEN];  /**< SENSF_RES. */
} ptx_NSC_RfDiscover_Param_Listen_F_t;
/** @} */

/**
 * \addtogroup NSC_RF_DISCOVER_NFCV
 *
 * @{
 */
/**
 * \brief Describes the parameters of the NFC-V.
 * poll mode configuration.
 */
typedef struct ptx_NSC_RfDiscover_Param_Poll_V
{
    uint8_t PollFrequency;          /**< Polling frequency (every n cycles; n_max=10). */
    uint8_t StopOnCollision;        /**< Stop (1)/Continue (0) the resolution process in case of collision. */
    uint8_t DevicesLimit;           /**< Maximum number of devices resolved during anticollision. */
} ptx_NSC_RfDiscover_Param_Poll_V_t;

/**
 * \brief Structure representing the RF-Tech payload of the discover notification for NFC-V
 */
typedef struct ptx_NSC_RFDiscover_Ntf_Param_PollV
{
    uint8_t InventoryRes[PTX_NSC_RF_DISCOVER_INVENTORY_RES_LEN];    /**< Response to INVENTORY_REQ command. */
} ptx_NSC_RfDiscover_Ntf_Param_PollV_t;

/** @} */

/**
 * \addtogroup NSC_RF_DISCOVER_PROT_SPEC
 *
 * @{
 */

/**
 * \brief Describes the parameters of the ISO-DEP poll mode configuration
 */
typedef struct ptx_NSC_RfDiscover_Param_Poll_IsoDep
{
    uint8_t AutoActivation; /**< Enable ISO-DEP poll mode automatic activation. */
    uint8_t Hbr;            /**< Enable high bit-rates. */
    uint8_t AttribInfLen;   /**< Length of ATTRIB INF parameter. */
    uint8_t AttribInf[PTX_NSC_RF_DISCOVER_ATTRIBINF_LEN];  /**< ATTRIB INF parameter. */
} ptx_NSC_RfDiscover_Param_Poll_IsoDep_t;

/**
 * \brief Describes the parameters of the NFC-DEP poll mode configuration
 */
typedef struct ptx_NSC_RfDiscover_Param_Poll_NfcDep
{
    uint8_t AutoActivation;         /**< Enable NFC-DEP poll mode automatic activation. */
    uint8_t Hbr;                    /**< Enable high bit-rates. */
    uint8_t AtrReqGeneralLen;       /**< Length of ATR REQ G parameter. */
    uint8_t AtrReqGeneral[PTX_NSC_RF_DISCOVER_ATRREQ_GEN_LEN];  /**< ATR REQ G parameter. */
} ptx_NSC_RfDiscover_Param_Poll_NfcDep_t;

/**
 * \brief Listen_IsoDep parameters.
 */
typedef struct ptx_NSC_RfDiscover_Param_Listen_IsoDep
{
    uint8_t EnableListenT4AT;                               /**< Enable Listen_IsoDep NFC-A. */
    uint8_t EnableListenT4BT;                               /**< Enable Listen_IsoDep NFC-B. */
    uint8_t ATS[PTX_NSC_RF_DISCOVER_ATS_LEN];               /**< Answer To Select. */
    uint8_t AttribResLen;                                   /**< ATTRIB Response length. */
    uint8_t AttribRes[PTX_NSC_RF_DISCOVER_ATTRIBRES_LEN];   /**< ATTRIB Response. */
} ptx_NSC_RfDiscover_Param_Listen_IsoDep_t;

/**
 * \brief Listen_NfcDep parameters.
 */
typedef struct ptx_NSC_RfDiscover_Param_Listen_NfcDep
{
    uint8_t EnableListenNfcDepA;                        /**< Enable Listen_NfcDep NFC-A. */
    uint8_t EnableListenNfcDepF;                        /**< Enable Listen_NfcDep NFC-F. */
    uint8_t AtrResLen;                                  /**< ATR_RES length. */
    uint8_t AtrRes[PTX_NSC_RF_DISCOVER_ATR_RES_LEN];    /**< ATR_RES. */
} ptx_NSC_RfDiscover_Param_Listen_NfcDep_t;
/** @} */

/**
 * \addtogroup NSC_RF_DISCOVER_COMMON
 *
 * @{
 */
/**
 * \brief Union for Poll Parameter.
 */
typedef union ptx_NSC_RfDiscover_Ntf_PollParam
{
    ptx_NSC_RfDiscover_Ntf_Param_PollA_t PollA; /**< Poll A. */
    ptx_NSC_RfDiscover_Ntf_Param_PollB_t PollB; /**< Poll B. */
    ptx_NSC_RfDiscover_Ntf_Param_PollF_t PollF; /**< Poll F. */
    ptx_NSC_RfDiscover_Ntf_Param_PollV_t PollV; /**< Poll V. */
} ptx_NSC_RfDiscover_Ntf_PollParam_t;

/**
 * \brief  Describes the different types.
 */
typedef union ptx_NSC_RfDiscover_Param_Value
{
    ptx_NSC_RfDiscover_Param_Poll_t           Poll;         /**< Poll. */
    ptx_NSC_RfDiscover_Param_Poll_Acm_t       PollAcm;      /**< Poll ACM. */
    ptx_NSC_RfDiscover_Param_Poll_A_t         PollA;        /**< Poll A. */
    ptx_NSC_RfDiscover_Param_Poll_B_t         PollB;        /**< Poll B. */
    ptx_NSC_RfDiscover_Param_Poll_F_t         PollF;        /**< Poll F. */
    ptx_NSC_RfDiscover_Param_Poll_V_t         PollV;        /**< Poll F. */
    ptx_NSC_RfDiscover_Param_Poll_IsoDep_t    PollIsoDep;   /**< Poll ISO DEP. */
    ptx_NSC_RfDiscover_Param_Poll_NfcDep_t    PollNfcDep;   /**< Poll NFC DEP. */

    ptx_NSC_RfDiscover_Param_Listen_t         Listen;       /**< Listen. */
    ptx_NSC_RfDiscover_Param_Listen_A_t       ListenA;      /**< Listen A. */
    ptx_NSC_RfDiscover_Param_Listen_B_t       ListenB;      /**< Listen B. */
    ptx_NSC_RfDiscover_Param_Listen_F_t       ListenF;      /**< Listen F. */
    ptx_NSC_RfDiscover_Param_Listen_IsoDep_t  ListenIsoDep; /**< Listen ISO DEP. */
    ptx_NSC_RfDiscover_Param_Listen_NfcDep_t  ListenNfcDep; /**< Listen NFC DEP. */
} ptx_NSC_RfDiscover_Param_Value_t;

/**
 * \brief Type-Value-Pair storing individual poll mode parameters
 */
typedef struct ptx_NSC_RfDiscover_Param
{
    ptx_NSC_RfDiscover_Param_Type_t   Type;  /**< Discover Type. */
    ptx_NSC_RfDiscover_Param_Value_t  Value; /**< Discover Value. */
} ptx_NSC_RfDiscover_Param_t;

/**
 * \brief Structure representing the payload of
 * the discover notification.
 */
typedef struct ptx_NSC_RfDiscover_Ntf
{
    uint8_t                             IsLastNtf;  /**< Last Discovery NTF. */
    uint8_t                             RfTech;     /**< RF Technology.                     */
    ptx_NSC_RfDiscover_Ntf_PollParam_t  PollParam;  /**< Poll mode notification parameters. */
    ptx_NSC_RfDiscover_Ntf_State_t      RfState;    /**< RF state of the remote device.     */
    uint8_t                             CollPend;   /**< Indicates if more remote devices are pending to be resolved. */
} ptx_NSC_RfDiscover_Ntf_t;

/** @} */

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
