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
    File        : ptx_RUL.h

    Description : Reader Universal Library (RUL) API
                  Contains functions to use a PTX Device with RUL functionality.
*/

/**
 * \addtogroup RUL
 * @{
 */

#include <stdint.h>
#if !defined(PTX_RUL_H)
#define PTX_RUL_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>
#include <math.h>

#include "ptx_HAL.h"
#include "ptx_HIP.h"
#include "ptx_NSC.h"
#include "ptx_NSC_RUL_RfConfig.h"
#include "ptx_Log.h"
#include "ptx_RUL_Cfg.h"

#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX)
    #include "ptx_OSAL.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif


/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name RUL modes.
 * @{
 */
#define PTX_RUL_MODE_RD     (0x00u) /**< RUL Mode Read. */
#define PTX_RUL_MODE_HCE    (0x01u) /**< RUL Mode Host Card Emulation. */
#define PTX_RUL_MODE_WLC    (0x02u) /**< RUL Mode Wireless Charging. */
/** @} */

/**
 * \name RUL and protocol specific length definitions.
 * @{
 */
#define PTX_RUL_MAX_SUPPORTED_DEVICES   (32u)                           /**< Maximum number of supported Cards. / RF-Devices */
#define PTX_RUL_SENSRES_LEN             PTX_NSC_TYPES_LISASENSRES_LEN   /**< SENSRES Length. */
#define PTX_RUL_SENSBRES_LEN            PTX_NSC_TYPES_SENSBRES_LEN      /**< SENSBRES Length. */
#define PTX_RUL_SENSFRES_LEN            PTX_NSC_TYPES_SENSFRES_LEN      /**< SENSFRES Length. */

#define PTX_RUL_NFCID0_LEN              (4u)                            /**< NFCID0 Max Length. */
#define PTX_RUL_NFCID1_MAX_LEN          PTX_NSC_TYPES_LISANFCID1_LEN    /**< NFCID1 Max Length. */
#define PTX_RUL_NFCID2_LEN              (8u)                            /**< NFCID2 Max Length. */
#define PTX_RUL_APPDATA_LEN             (4u)                            /**< Application Data in SENSB_RES. */
#define PTX_RUL_PROTINFO_LEN            (4u)                            /**< Protocol Info in SENSB_RES. */
#define PTX_RUL_RD_LEN                  (2u)                            /**< Request Data in SENSF_RES. */
#define PTX_RUL_INVENTORYRES_LEN        (10u)                           /**< INVENTORY Length. */
#define PTX_RUL_UID_LEN                 (8u)                            /**< UID Length. */
#define PTX_RUL_ATTRIBINF_LEN           (15u)                           /**< ATTRIB INF Length. */
#define PTX_RUL_NFCDEP_ATRREQ_LEN       (15u)                           /**< NFC-DEP ATRREQ Max Length. */
#define PTX_RUL_NFCDEP_ATRRES_LEN       (19u)                           /**< NFC-DEP ATRRES Max Length. */
#define PTX_RUL_NFCDEP_GENERALBYTES_LEN (15u)                           /**< NFC-DEP General Bytes Length. */
#define PTX_RUL_ACM_GENERALBYTES_LEN    (15u)                           /**< ACM General Bytes Length. */
#define PTX_RUL_ISODEP_ATS_LEN          (20u)                           /**< ISO-DEP ATS Max Length. */
#define PTX_RUL_ISODEP_ATTRIBRES_LEN    (16u)                           /**< ISO-DEP ATTRIB RES Max Length. */
#define PTX_RUL_SENSF_REQ_SYS_CODE_LEN  (2u)                            /**<  SENSF_REQ System Code Length */

/** @} */

/**
 * \name RUL and protocol specific offset definitions.
 * @{
 */
#define PTX_RUL_SENSF_RES_NFCID2_OFFSET                  (2u)                   /**<  Offset for SENSF_RES NFCID2 */
#define PTX_RUL_SENSF_RES_PD2_OFFSET                     (19u)                  /**<  Offset for SENSF_RES PD offset, second byte */
#define PTX_RUL_SENSF_RES_PD2_BR_MASK                    (3u)                   /**<  Mask for SENSF_RES PD bit rate */

#if defined(ENABLE_EXTENDED_CARD_REGISTER)
//Type B 
#define PTX_RUL_SENSB_RES_NFCID0_OFFSET                  (1u)                   /**<  Offset for SENSB_RES NFCID0 */
#define PTX_RUL_SENSB_RES_AFI_OFFSET                     (5u)                   /**<  Offset for SENSB_RES AFI */
#define PTX_RUL_SENSB_RES_NUMBER_APPLICATIONS_OFFSET     (8u)                   /**<  Offset for SENSB_RES Number of applications */
#define PTX_RUL_SENSB_RES_CRC_B_OFFSET                   (6u)                   /**<  Offset for SENSB_RES CRC_B(AID) */
#define PTX_RUL_CRC_B_AID_LEN                            (2u)                   /**<  CRC_B(AID) in SENSB_RES */
#define PTX_RUL_SENSB_BIT_RATE_CAPABILITY_OFFSET         (9u)                   /**<  Offset for SENSB_RES Bit Rate Capability */
#define PTX_RUL_SENSB_FSCI_PROTOCOL_TYPE_OFFSET          (10u)                  /**<  Offset for SENSB_RES FSCI and Protocol Type */
#define PTX_RUL_SENSB_FSCI_MASK                          (0xF0u)                /**<  Mask for SENSB_RES FSCI */
#define PTX_RUL_SENSB_PROTOCOL_TYPE_MASK                 (0x0Fu)                /**<  Mask for SENSB_RES Protocol Type */
#define PTX_RUL_SENSB_FWI_ADC_FO_OFFSET                  (11u)                  /**<  Offset for SENSB_RES FWI ADC and FO */
#define PTX_RUL_SENSB_FWI_MASK                           (0xF0u)                /**<  Mask for SENSB_RES FWI */
#define PTX_RUL_SENSB_ADC_FO_MASK                        (0x0Fu)                /**<  Mask for SENSB_RES FO and ADC*/
#define PTX_RUL_SENSB_SFGI_OFFSET                        (12u)                  /**<  Offset for SENSB_RES SFGI */
//Type F
#define PTX_RUL_SENSF_RES_PAD0_OFFSET                    (10u)                  /**<  offset of PAD0 in SENSF_RES */
#define PTX_RUL_SENSF_RES_PAD1_OFFSET                    (12u)                  /**<  offset of PAD1 in SENSF_RES */
#define PTX_RUL_SENSF_RES_MRTI_CHECK_OFFSET              (15u)                  /**<  offset of MRTI CHECK in SENSF_RES */
#define PTX_RUL_SENSF_RES_MRTI_UPDATE_OFFSET             (16u)                  /**<  offset of MRTI UPDATE in SENSF_RES */
#define PTX_RUL_SENSF_RES_PAD2_OFFSET                    (17u)                  /**<  offset of PAD2 in SENSF_RES */
#define PTX_RUL_SENSF_RES_RD_OFFSET                      (18u)                  /**<  offset of PAD2 in SENSF_RES */
#define PTX_RUL_SENSF_RES_AUTO_BR_SELECT_MASK            (0x80)                 /**<  Mask for automatic bit rate selection feature in SENSF_RES */
#define PTX_RUL_SENSF_RES_SUPPORTED_BR_MASK              (0x0F)                 /**<  Mask for Supported bit rate feature in RD of SENSF_RES */
#define PTX_RUL_SENSF_PAD0_LEN                           (2u)                   /**<  SENSF_REQ System Code Length */
#define PTX_RUL_SENSF_PAD1_LEN                           (3u)                   /**<  SENSF_REQ System Code Length */
//Type V
#define PTX_RUL_INVENTORYRES_REDFLAG_OFFSET              (0u)                   /**< RED_FLAG offset in the INVENTORY_RES */
#define PTX_RUL_INVENTORYRES_DSFID_OFFSET                (1u)                   /**< DSFID offset in the INVENTORY_RES */
#define PTX_RUL_INVENTORYRES_UID_OFFSET                  (2u)                   /**< UID offset in the INVENTORY_RES */
#define PTX_RUL_INVENTORYRES_UID_LEN                     (8u)                   /**< UID length in the INVENTORY_RES */
//Type P2P-ACM
#define PTX_RUL_ATR_RES_NFCID3_OFFSET                    (3u)                   /**<  Offset for ATR_RES NFCID3 */
#define PTX_RUL_ATR_RES_DID_OFFSET                       (13u)                  /**<  Offset of ATR_RES DID */
#define PTX_RUL_ATR_RES_BS_OFFSET                        (14u)                  /**<  Offset of ATR_RES BS */
#define PTX_RUL_ATR_RES_BR_OFFSET                        (15u)                  /**<  Offset of ATR_RES BR */
#define PTX_RUL_ATR_RES_TO_OFFSET                        (16u)                  /**<  Offset of ATR_RES TO */
#define PTX_RUL_ATR_RES_PP_OFFSET                        (17u)                  /**<  Offset of ATR_RES PP */
#define PTX_RUL_ATR_RES_GB_OFFSET                        (18u)                  /**<  Offset General bytes in ATR_RES */
#define PTX_RUL_ATR_RES_WT_MASK                          (0x0F)                 /**<  Mask for WT in PP field of ATR_RES*/
#define PTX_RUL_ATR_RES_NFCID3_LEN                       (10u)                  /**<  Length of ATR_RES NFCID3 */ 
#endif

/** @} */

/**
 * \name RUL and protocol specific value definitions.
 * @{
 */
#define PTX_RUL_NFCDEP_BRS_UNUSED               (0xFF)  /**< Value for BRS not set. PSL_REQ has not been sent. */
#define PTX_RUL_NFCDEP_BRS_MAX_SPEED_ALLOWED    (0x02)  /**< Max. allowed Value for DSI and DRI. */
/** @} */

/**
 * \name Protocol support flags.
 * @{
 */
#define PTX_RUL_SUPPORT_ISODEP_NFCA             (0x20u)   /**< Support ISO-DEP Flag for NFC TypeA. */
#define PTX_RUL_SUPPORT_NFCDEP_NFCA             (0x40u)   /**< Support NFC-DEP Flag for NFC TypeA. */
#define PTX_RUL_SUPPORT_NFCDEP_NFCF             (0x01FEu) /**< Support NFC-DEP Flag for NFC TypeF. */
#define PTX_RUL_SUPPORT_NFCDEP_NFCF_POSITION    (2u)      /**< Position of Support NFC-DEP Flag for NFC TypeF. */
/** @} */

/**
 * \name GPIO defines.
 * @{
 */
#define PTX_RUL_USER_GPIOS              (uint8_t)(8u)   /**< Defines the number of user available GPIO(s). */

#define PTX_RUL_GPIO_EXTENDED_FEATURES  (0x01u) /**< Indicates a GPIO which supports extended features set. */
#define PTX_RUL_GPIO_STANDARD_FEATURES  (0x00u) /**< Indicates a GPIO which supports standard features set. */

#define PTX_RUL_GPIO01_FEATURES         PTX_RUL_GPIO_STANDARD_FEATURES      /**< GPIO1 has standard features set */
#define PTX_RUL_GPIO02_FEATURES         PTX_RUL_GPIO_STANDARD_FEATURES      /**< GPIO2 has standard features set */
#define PTX_RUL_GPIO03_FEATURES         PTX_RUL_GPIO_STANDARD_FEATURES      /**< GPIO3 has standard features set */
#define PTX_RUL_GPIO04_FEATURES         PTX_RUL_GPIO_EXTENDED_FEATURES      /**< GPIO4 has extended features set */
#define PTX_RUL_GPIO05_FEATURES         PTX_RUL_GPIO_STANDARD_FEATURES      /**< GPIO5 has standard features set */
#define PTX_RUL_GPIO06_FEATURES         PTX_RUL_GPIO_EXTENDED_FEATURES      /**< GPIO6 has extended features set */
#define PTX_RUL_GPIO07_FEATURES         PTX_RUL_GPIO_EXTENDED_FEATURES      /**< GPIO7 has extended features set */
#define PTX_RUL_GPIO08_FEATURES         PTX_RUL_GPIO_EXTENDED_FEATURES      /**< GPIO8 has extended features set */

#define PTX_RUL_GPIO_NR_01_CFG_INDEX    (0x00u)     /**< GPIO NR 01 Config Index */
#define PTX_RUL_GPIO_NR_02_CFG_INDEX    (0x01u)     /**< GPIO NR 02 Config Index */
#define PTX_RUL_GPIO_NR_03_CFG_INDEX    (0x02u)     /**< GPIO NR 03 Config Index */
#define PTX_RUL_GPIO_NR_04_CFG_INDEX    (0x03u)     /**< GPIO NR 04 Config Index */
#define PTX_RUL_GPIO_NR_05_CFG_INDEX    (0x04u)     /**< GPIO NR 05 Config Index */
#define PTX_RUL_GPIO_NR_06_CFG_INDEX    (0x05u)     /**< GPIO NR 06 Config Index */
#define PTX_RUL_GPIO_NR_07_CFG_INDEX    (0x06u)     /**< GPIO NR 07 Config Index */
#define PTX_RUL_GPIO_NR_08_CFG_INDEX    (0x07u)     /**< GPIO NR 08 Config Index */
/** @} */

/**
 * \name RF-protocol specific defines.
 * @{
 */
#define PTX_RUL_CARD_RFPROTOCOL_ATS_LEN             (21u)   /**< Max. supported ATS response length (Poll/Reader). */
#define PTX_RUL_CARD_RFPROTOCOL_ATTRIBRES_LEN       (16u)   /**< Max. supported ATTRIB response length (Poll/Reader). */
#define PTX_RUL_CARD_RFPROTOCOL_ATR_RES_LEN         (65u)   /**< Max. supported ATR_RES length (NFC Initiator). */
#define PTX_RUL_CARD_RFPROTOCOL_ATTRIB_LEN          (24u)   /**< Max. supported ATTRIB command length (Listen/Card). */
#define PTX_RUL_CARD_RFPROTOCOL_ATR_REQ_LEN         (65u)   /**< Max. supported ATR_REQ length (NFC Target). */
/** @} */

/**
 * \name RUL-Extension related defines
 * @{
 */
#define PTX_RUL_EXTENSION_INDEX_LLCP   (0u)        /**< Position of the LLCP extension endpoint. */
#define PTX_RUL_EXTENSION_INDEX_WLC    (1u)        /**< Position of the WLC extension endpoint. */
#define PTX_RUL_EXTENSION_INDEX_HCE    (2u)        /**< Position of the HCE extension endpoint. */

#define PTX_RUL_EXTENSION_NTF_UNUSED    (0x00u)     /**< NTF was not used by the extension endpoint */
#define PTX_RUL_EXTENSION_NTF_USED      (0x01u)     /**< NTF was used and processed by the extension endpoint */
/** @} */

/**
 * \name RUL Tx ESD prevention related defines
 * @{
 */
#define PTX_RUL_TX_ESD_PREVENTION_THRESHOLD_MAX             (253u)  /**< Maximum possible measured threshold during ESD prevention calibration routine. */
/** @} */


/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/

/**
 * \brief RUL NFC-A/B Polling Frequency.
 */
typedef enum ptx_RUL_PollFreq
{
    ptx_RUL_PollFreq_Disabled   =     0,        /**< Do not poll for selected technology. */
    ptx_RUL_PollFreq_EveryCycle =     1,        /**< Poll for selected technology on every cycle. */
    ptx_RUL_PollFreq_CycleMin   =     2,        /**< Poll for selected technology every nth cycle, minimum 2. */
    ptx_RUL_PollFreq_CycleMax   =     10,       /**< Poll for selected technology every nth cycle, maximum 10. */
    ptx_RUL_PollFreq_Max                        /**< Poll Frequency Max. */
} ptx_RUL_PollFreq_t;

/**
 * \brief RUL NFC-A/B/F Bail out.
 */
typedef enum ptx_RUL_PollBailOut
{
    ptx_RUL_PollBailOut_Disabled =      0,       /**< No Bail out when NFC-A/B or F technology is found. */
    ptx_RUL_PollBailOut_Enabled  =      1,       /**< Bail out when NFC-A/B or F technology is found. */
    ptx_RUL_PollBailOut_Max                      /**< NFC-A/B/F bail out max. */
} ptx_RUL_PollBailOut_t;

/**
 * \brief RUL Enable/Disable DDPC.
 * DDPC (Digital Dynamic Power Control).
 */
typedef enum ptx_RUL_PollDDPC
{
    ptx_RUL_PollDDPC_Disabled =         0,      /**< DDPC Disabled. */
    ptx_RUL_PollDDPC_Enabled  =         1,      /**< DDPC Enabled. */
    ptx_RUL_PollDDPC_Max                        /**< DDPC Max. */
} ptx_RUL_PollDDPC_t;

/**
 * \brief RUL LPCD (Low Power Card Detection).
 */
typedef enum ptx_RUL_PollLPCD
{
    ptx_RUL_PollLPCD_Disabled  =        0,      /**< LPCD Disabled. */
    ptx_RUL_PollLPCD_Enabled   =        1,      /**< LPCD Enables. */
    ptx_RUL_PollLPCD_HybridMin =        2,      /**< LPCD Hybrid Mode, regular RF Disc used every nth cycle, LPCD used in the others, minimum Value. */
    ptx_RUL_PollLPCD_HybridMax =        255,    /**< LPCD Hybrid Mode, regular RF Disc used every nth cycle, LPCD used in the others, maximum Value. */
    ptx_RUL_PollLPCD_Max                        /**< Safeguard. */
} ptx_RUL_PollLPCD_t;

/**
 * \brief RUL Enable/Disable ISO-DEP automatic activation.
 */
typedef enum ptx_RUL_PollIsoDep
{
    ptx_RUL_PollIsoDep_Disabled =       0,      /**< ISO-DEP Disabled. */
    ptx_RUL_PollIsoDep_Enabled  =       1,      /**< ISO-DEP Enabled. */
    ptx_RUL_PollIsoDep_Max                      /**< ISO-DEP Max. */
} ptx_RUL_PollIsoDep_t;

/**
 * \brief RUL ISO-DEP HBR (High Bit-rates) enable.
 */
typedef enum ptx_RUL_PollIsoDepHBR
{
    ptx_RUL_PollIsoDepHBR_Standard =    0,      /**< Default setting. Standard bitrate (106kbps). */
    ptx_RUL_PollIsoDepHBR_212      =    1,      /**< Higher bit-rates, up to 212kbps may be used. */
    ptx_RUL_PollIsoDepHBR_424      =    2,      /**< Higher bit-rates, up to 424kbps may be used. */
    ptx_RUL_PollIsoDepHBR_848      =    3,      /**< Higher bit-rates, up to 848kbps may be used. */
    ptx_RUL_PollIsoDepHBR_Maximum =    4,      /**< Maximum possible bitrate. */
    ptx_RUL_PollIsoDepHBR_Undefined             /**< Safeguard. */
} ptx_RUL_PollIsoDepHBR_t;

/**
 * \brief RUL Enable/Disable NFC-DEP automatic activation.
 */
typedef enum ptx_RUL_PollNfcDep
{
    ptx_RUL_PollNfcDep_Disabled    =    0,      /**< NFC-DEP Disabled. */
    ptx_RUL_PollNfcDep_Enabled     =    1,      /**< NFC-DEP Enabled. */
    ptx_RUL_PollNfcDep_Max                      /**< NFC-DEP Max. */
} ptx_RUL_PollNfcDep_t;

/**
 * \brief RUL NFC-DEP HBR (High Bit-rates) enable.
 */
typedef enum ptx_RUL_PollNfcDepHBR
{
    ptx_RUL_PollNfcDepHBR_Standard =    0,      /**< Default setting. Activation bitrate shall be used. */
    ptx_RUL_PollNfcDepHBR_212      =    1,      /**< Higher bit-rates, up to 212kbps may be used. */
    ptx_RUL_PollNfcDepHBR_424      =    2,      /**< Higher bit-rates, up to 424kbps may be used. */
    ptx_RUL_PollNfcDepHBR_Maximum  =    3,      /**< Maximum possible bitrate shall be used. */
    ptx_RUL_PollNfcDepHBR_Undefined             /**< Safeguard. */
} ptx_RUL_PollNfcDepHBR_t;

/**
 * \brief RUL NFC-DEP (ACM) HBR (High Bit-rates) enable.
 */
typedef enum ptx_RUL_PollACMBitRate
{
    ptx_RUL_PollACMBitRate_106     =        0,  /**< Default Bit Rate of 106kbps (NFC-A Technology). */
    ptx_RUL_PollACMBitRate_212     =        1,  /**< Bit Rate of 212 kbps (NFC-F Technology). */
    ptx_RUL_PollACMBitRate_424     =        2,  /**< Bit Rate of 424 kbps (NFC-F Technology). */
    ptx_RUL_PollACMBitRate_Undefined            /**< Safeguard. */
} ptx_RUL_PollACMBitRate_t;

/**
 * \brief RUL Poll Mode Configuration parameter.
 * Select different polling modes.
 */
typedef enum ptx_RUL_PollMode
{
    ptx_RUL_PollMode_Disabled       =        0,     /**< POLL MODE Disabled. */
    ptx_RUL_PollMode_NfcForum       =        1,     /**< POLL MODE Nfc Forum. */
    ptx_RUL_PollMode_Emvco          =        2,     /**< POLL MODE EMVCo. */
    ptx_RUL_PollMode_Iso            =        3,     /**< POLL MODE Iso. */
    ptx_RUL_PollMode_ContinuousWave =        4,     /**< POLL MODE Continuous Wave. */
    ptx_RUL_PollMode_Lpcd           =        5,     /**< POLL MODE LPCD. */
    ptx_RUL_PollMode_Max                            /**< POLL MODE Max. */
} ptx_RUL_PollMode_t;

/**
 * \brief RUL Initial Polling Technology parameter.
 * Select between the different initial polling modes.
 */
typedef enum ptx_RUL_InitialPollingTech
{
    ptx_RUL_InitialPollingTech_Default =     0,    /**< Default Initial Poll Mode. Poll mode is ignored. */
    ptx_RUL_InitialPollingTech_NfcA    =     1,    /**< Initial Poll Mode for NFC-A technology. */
    ptx_RUL_InitialPollingTech_NfcB    =     2,    /**< Initial Poll Mode for NFC-B technology. */
    ptx_RUL_InitialPollingTech_NfcF    =     3,    /**< Initial Poll Mode for NFC-F technology. */
    ptx_RUL_InitialPollingTech_NfcV    =     4,    /**< Initial Poll Mode for NFC-V technology. */
    ptx_RUL_InitialPollingTech_Max                 /**< Max initial Poll Mode. */
} ptx_RUL_InitialPollingTech_t;

/**
* \brief RUL StatusInfo Types.
*/
typedef enum ptx_RUL_StatusType
{
    ptx_RUL_StatusType_System,          /**< Status System. */
    ptx_RUL_StatusType_Discover,        /**< Status Discover. */
    ptx_RUL_StatusType_Lpcd,            /**< Status LPCD. */
    ptx_RUL_StatusType_Deactivate,      /**< Status Deactivate. */
    ptx_RUL_StatusType_EmvRemoval,      /**< Status Emv Removal. */
    ptx_RUL_StatusType_LastRfError,     /**< Status Last Rf Error. */
    ptx_RUL_StatusType_NfcMode,         /**< Status NFC Mode. */
    ptx_RUL_StatusType_Max              /**< Safeguard. */
} ptx_RUL_StatusType_t;

/**
 * \brief RUL Status Info System.
 */
typedef enum ptx_RUL_StatusInfoSystem
{
    ptx_RUL_StatusInfoSystem_OK,                /**< System OK. */
    ptx_RUL_StatusInfoSystem_NSCReset,          /**< System Reset. */
    ptx_RUL_StatusInfoSystem_ErrorOvercurrent,  /**< System Error overcurrent. */
    ptx_RUL_StatusInfoSystem_ErrorTemperature,  /**< System Error Temperature. */
    ptx_RUL_StatusInfoSystem_ErrorVddTx,        /**< System Error VDD Tx. */
    ptx_RUL_StatusInfoSystem_ErrorOTP,          /**< System Error OTP configuration. */
    ptx_RUL_StatusInfoSystem_ErrorStandby,      /**< System Error Standby. System exited Standby mode, Discovery is not active. */
    ptx_RUL_StatusInfoSystem_Max                /**< Safeguard. */
} ptx_RUL_StatusInfoSystem_t;

/**
 * \brief RUL Status Info Discover.
 */
typedef enum ptx_RUL_StatusInfoDiscover
{
    ptx_RUL_StatusInfoDiscover_NoCard,                  /**< Discover No Card. */
    ptx_RUL_StatusInfoDiscover_CardDetectionOngoing,    /**< Discover Multiple Cards detected, resolution ongoing */
    ptx_RUL_StatusInfoDiscover_CardsAvailable,          /**< Discover Cards Available. */
    ptx_RUL_StatusInfoDiscover_Active,                  /**< Discover Active. */
    ptx_RUL_StatusInfoDiscover_MultiProtocol,           /**< Discover Multiple Protocols. */
    ptx_RUL_StatusInfoDiscover_EmvCollision,            /**< Discover EMV Collision. */
    ptx_RUL_StatusInfoDiscover_Max                      /**< Safeguard. */
} ptx_RUL_StatusInfoDiscover_t;

/**
 * \brief RUL Status Info LPCD.
 */
typedef struct ptx_RUL_StatusInfoLpcd
{
    uint8_t LpcdNtfCounter;                     /**< Counter for amount of received LPCD NTFs. */
} ptx_RUL_StatusInfoLpcd_t;

/**
 * \brief RUL Status Info Emv Removal
 */
typedef enum ptx_RUL_StatusInfoEmvRemoval
{
    ptx_RUL_StatusInfoEmvRemoval_Undefined, /**< EMV Removal Undefined */
    ptx_RUL_StatusInfoEmvRemoval_Pending,   /**< EMV Removal Pending */
    ptx_RUL_StatusInfoEmvRemoval_Complete,  /**< EMV Removal Complete */
    ptx_RUL_StatusInfoEmvRemoval_Error,     /**< EMV Removal Error */
    ptx_RUL_StatusInfoEmvRemoval_Max        /**< Safeguard */
} ptx_RUL_StatusInfoEmvRemoval_t;

/**
 * \brief RUL Status Deactivate.
 */
typedef enum ptx_RUL_StatusInfoDeactivate
{
    ptx_RUL_StatusInfoDeactivate_Undefined, /**< Deactivate Undefined. */
    ptx_RUL_StatusInfoDeactivate_Pending,   /**< Deactivate Pending. */
    ptx_RUL_StatusInfoDeactivate_Complete,  /**< Deactivate Complete. */
    ptx_RUL_StatusInfoDeactivate_Error,     /**< Deactivate Error. */
    ptx_RUL_StatusInfoDeactivate_Max        /**< Safeguard. */
} ptx_RUL_StatusInfoDeactivate_t;

/**
 * \brief RF Error Types.
 */
typedef enum ptx_RUL_StatusInfoRfError
{
    ptx_RUL_StatusInfoRfError_ErrorNone,    /**< Rf Error None. */
    ptx_RUL_StatusInfoRfError_Collision,    /**< Rf Error Collision. */
    ptx_RUL_StatusInfoRfError_Timeout,      /**< Rf Error Timeout. */
    ptx_RUL_StatusInfoRfError_Transmission, /**< Rf Error Transmission. */
    ptx_RUL_StatusInfoRfError_Protocol,     /**< Rf Error Protocol. */
    ptx_RUL_StatusInfoRfError_Undefined     /**< Rf Error Undefined. */
} ptx_RUL_StatusInfoRfError_t;


/**
 * \brief NFC Mode of Operation.
 */
typedef enum ptx_RUL_StatusInfoMode
{
    ptx_RUL_StatusInfoMode_Idle,            /**< Idle mode. */
    ptx_RUL_StatusInfoMode_Poller,          /**< Poller mode. */
    ptx_RUL_StatusInfoMode_Listener,        /**< Listener mode. */
    ptx_RUL_StatusInfoMode_P2P_Initiator,   /**< P2P Initiator. */
    ptx_RUL_StatusInfoMode_P2P_Target,      /**< P2P Target. */
    ptx_RUL_StatusInfoMode_Undefined        /**< Undefined. */
} ptx_RUL_StatusInfoMode_t;


/**
 * \brief RUL Status Info.
 */
typedef union ptx_RUL_StatusInfo
{
    ptx_RUL_StatusInfoSystem_t      System;     /**< System Status Info. */
    ptx_RUL_StatusInfoDiscover_t    Discover;   /**< Discover Status Info. */
    ptx_RUL_StatusInfoLpcd_t        Lpcd;       /**< LPCD Status Info. */
    ptx_RUL_StatusInfoDeactivate_t  Deactivate; /**< Deactivate Status Info. */
    ptx_RUL_StatusInfoEmvRemoval_t  EmvRemoval; /**< EMV Removal Status Info. */
    ptx_RUL_StatusInfoRfError_t     RfError;    /**< Rf Error Status Info. */
    ptx_RUL_StatusInfoMode_t        NfcMode;    /**< NFC Mode Status Info. */
} ptx_RUL_StatusInfo_t;

/**
 * \brief RUL StatusInfo Parameters.
 */
typedef struct ptx_RUL_Status_Params
{
    ptx_RUL_StatusType_t    Type; /**< Type of requested status. */
    ptx_RUL_StatusInfo_t    Info; /**< Current status. */
} ptx_RUL_Status_Params_t;


/**
 * \brief RUL Discovery Deactivation Types.
 */
typedef enum ptx_RUL_DeactivateType
{
    ptx_RUL_DeactivateType_Generic, /**< DeactivateType Generic. The deactivation is not specific of any protocol. */
    ptx_RUL_DeactivateType_Specific,/**< DeactivateType Specific. The deactivation is protocol specific. */
    ptx_RUL_DeactivateType_Max      /**< DeactivateType Max. Safeguard. */
} ptx_RUL_DeactivateType_t;


/**
 * \brief RUL Discovery Deactivation States.
 */
typedef enum ptx_RUL_DeactivateState
{
    ptx_RUL_DeactivateState_Idle,               /**< Deactivate to state Idle. */
    ptx_RUL_DeactivateState_Poll,               /**< Deactivate to state Poll. */
    ptx_RUL_DeactivateState_Discovery,          /**< Deactivate to state Discovery. */
    ptx_RUL_DeactivateState_DiscoveryRfReset,   /**< Deactivate to state Discovery and do an RfReset. */
    ptx_RUL_DeactivateState_ListenSleep,        /**< Deactivate to state ListenSleep.*/
    ptx_RUL_DeactivateState_ListenIdle,         /**< Deactivate to state ListenIdle.*/
    ptx_RUL_DeactivateState_Max                 /**< DeactivateState_Max. Safeguard. */
} ptx_RUL_DeactivateState_t;
/**
*
 * \brief RUL Discovery Deactivation Parameters.
 */
typedef struct ptx_RUL_DeactivateParams
{
    ptx_RUL_DeactivateType_t    Type;   /**< Deactivation type. */
    ptx_RUL_DeactivateState_t   State;  /**< RF-state after the deactivation. */
} ptx_RUL_DeactivateParams_t;

/**
 * \brief RUL System Reset Types
*/
typedef enum ptx_RUL_SystemResetTypes
{
    ptx_RUL_SystemResetType_Soft,       /**< System Reset by invoking low-level SW commands */
    ptx_RUL_SystemResetType_Hard,       /**< System Reset by toggling reset pin */
    ptx_RUL_SystemResetType_Max,        /**< Safeguard. */
} ptx_RUL_ResetTypes_t;

/**
 * \brief Type of Check Presence Mechanism.
 */
typedef enum ptx_RUL_CheckPresType
{
    ptx_RUL_CheckPresType_NFCDEP,       /**< Presence check for NFCDEP. */
    ptx_RUL_CheckPresType_ISODEP,       /**< Presence check for ISODEP. */
    ptx_RUL_CheckPresType_ISODEP_Empty, /**< Presence check for ISODEP with empty frame. */
    ptx_RUL_CheckPresType_T2T,          /**< Presence check for T2T. */
    ptx_RUL_CheckPresType_T3T,          /**< Presence check for T3T. */
    ptx_RUL_CheckPresType_T5T,          /**< Presence check for T5T. */

    ptx_RUL_CheckPresType_Max,          /**< Safeguard. */
} ptx_RUL_CheckPresType_t;

/**
 * \brief Card Protocol Types.
 */
typedef enum ptx_RUL_CardProtocolType
{
    ptx_RUL_CardProtocolType_Undefined  = 0,    /**< Card protocol type undefined. */
    ptx_RUL_CardProtocolType_T2T        = 2,    /**< Card protocol type T2T. */
    ptx_RUL_CardProtocolType_T3T        = 3,    /**< Card protocol type T3T. */
    ptx_RUL_CardProtocolType_T4T        = 4,    /**< Card protocol type T4T/ISO-DEP. */
    ptx_RUL_CardProtocolType_T5T        = 5,    /**< Card protocol type T5T. */
    ptx_RUL_CardProtocolType_NfcDep     = 6,    /**< Card protocol type NFC-DEP. */
    ptx_RUL_CardProtocolType_Max                /**< Safeguard. */
} ptx_RUL_CardProtocolType_t;

/**
 * \brief ISO DEP Card Protocol Parameters.
 */
typedef struct ptx_RUL_CardProtocolParam_IsoDep
{
    uint8_t HbrEnable;                          /**< Enable higher bit-rates.         */
    uint8_t AttribInfLen;                       /**< Length of ATTRIB_INF parameter.  */
    uint8_t AttribInf[PTX_RUL_ATTRIBINF_LEN];   /**< INF parameter.                   */
} ptx_RUL_CardProtocolParam_IsoDep_t;

/**
 * \brief NFC DEP Card Protocol Parameters.
 */
typedef struct ptx_RUL_CardProtocolParam_NfcDep
{
    uint8_t HbrEnable;                                      /**< Enable higher bit-rates.              */
    uint8_t GeneralBytesLen;                                /**< Length of the GeneralBytes parameter. */
    uint8_t GeneralBytes[PTX_RUL_NFCDEP_GENERALBYTES_LEN];  /**< GeneralBytes parameter                */
} ptx_RUL_CardProtocolParam_NfcDep_t;

/**
 * \brief Card Protocol Parameter.
 */
typedef union ptx_RUL_CardProtocolParam
{
    ptx_RUL_CardProtocolParam_IsoDep_t IsoDep;   /**< ISO-DEP Parameters. */
    ptx_RUL_CardProtocolParam_NfcDep_t NfcDep;   /**< NFC-DEP Parameters. */
} ptx_RUL_CardProtocolParam_t;

/**
 * \brief Card Protocol Types.
 */
typedef struct ptx_RUL_CardProtocol
{
    ptx_RUL_CardProtocolType_t      Type;   /**< Card Protocol Type. */
    ptx_RUL_CardProtocolParam_t     Param;  /**< Card Protocol Parameter. */
} ptx_RUL_CardProtocol_t;

/**
 * \brief Card Tech Types.
 */
typedef enum ptx_RUL_CardRfTechType
{
    ptx_RUL_CardRfTechType_A = 0,   /**< Card Technology Type A. */
    ptx_RUL_CardRfTechType_B = 1,   /**< Card Technology Type B. */
    ptx_RUL_CardRfTechType_F = 2,   /**< Card Technology Type F. */
    ptx_RUL_CardRfTechType_V = 3,   /**< Card Technology Type V. */
    ptx_RUL_CardRfTechType_ACM = 4, /**< Card Technology Type ACM. */
    ptx_RUL_CardRfTechType_Max,     /**< Safeguard. */
} ptx_RUL_CardRfTechType_t;

/**
 * \brief General Bitrate Types.
 */
typedef enum ptx_RUL_RfBitRate
{
    ptx_RUL_RfBitRate_106   = 0x00,     /**< 106 kBit/s */
    ptx_RUL_RfBitRate_212   = 0x01,     /**< 212 kBit/s */
    ptx_RUL_RfBitRate_424   = 0x02,     /**< 424 kBit/s */
    ptx_RUL_RfBitRate_848   = 0x03,     /**< 848 kBit/s */
    ptx_RUL_RfBitRate_26    = 0x06,     /**< 26 kBit/s */
    ptx_RUL_RfBitRate_53    = 0x07      /**< 53 kBit/s */
} ptx_RUL_RfBitRate_t;

#if defined(ENABLE_EXTENDED_CARD_REGISTER)
/**
 * \brief General FSCI to FCI Types.
 */
typedef enum ptx_RUL_FsciToFsc
{
    ptx_RUL_FsciToFsc_16     = 0x00,      /**< 16 bytes. */
    ptx_RUL_FsciToFsc_24     = 0x01,      /**< 24 bytes. */
    ptx_RUL_FsciToFsc_32     = 0x02,      /**< 32 bytes. */
    ptx_RUL_FsciToFsc_40     = 0x03,      /**< 40 bytes. */
    ptx_RUL_FsciToFsc_48     = 0x04,      /**< 28 bytes. */
    ptx_RUL_FsciToFsc_64     = 0x05,      /**< 64 bytes. */
    ptx_RUL_FsciToFsc_96     = 0x06,      /**< 96 bytes. */
    ptx_RUL_FsciToFsc_128    = 0x07,      /**< 128 bytes. */
    ptx_RUL_FsciToFsc_256    = 0x08,      /**< 256 bytes. */
    ptx_RUL_FsciToFsc_512    = 0x09,      /**< 512 bytes. */
    ptx_RUL_FsciToFsc_1024   = 0x0A,      /**< 1024 bytes. */
    ptx_RUL_FsciToFsc_2048   = 0x0B,      /**< 2048 bytes. */
    ptx_RUL_FsciToFsc_4096   = 0x0C       /**< 4096 bytes. */
} ptx_RUL_FsciToFsc_t;

/**
 * \brief Minimum TR2B Types.
 */
typedef enum ptx_RUL_MinTr2B
{
    ptx_RUL_MinTr2B_1792_fc_32_fs    = 0x00,     /**< 1792/fc (10 bd + 32/fs). */
    ptx_RUL_MinTr2B_3328_fc_128_fs   = 0x01,     /**< 3328/fc (10 bd + 128/fs). */
    ptx_RUL_MinTr2B_5376_fc_256_fs   = 0x02,     /**< 5376/fc (10 bd + 256/fs). */
    ptx_RUL_MinTr2B_9472_fc_512_fs   = 0x03,     /**< 9472/fc (10 bd + 512/fs). */
} ptx_RUL_MinTr2B_t;

/**
 * \brief Type B protocol info platfrom types.
 */
typedef enum ptx_RUL_BPlatformType
{
    ptx_RUL_BPlatformType_t4t_not_configured    = 0x00,      /**< Not configured for T4T platfrom. */
    ptx_RUL_BPlatformType_t4t_configured        = 0x01,      /**< Configured for T4T platfrom. */
} ptx_RUL_BPlatformType_t;

/**
 * \brief Type B protocol info ADC Coding Types.
 */
typedef enum ptx_RUL_BAdcCoding
{
    ptx_RUL_BAdcCoding_app_not_defined_in_spec        = 0x00,      /**< Application coding not defined in the current NCI spec. */
    ptx_RUL_BAdcCoding_app_coded_as_nci_spec          = 0x01,      /**< Application is codded ad defined in NCI spec 2.3. */
} ptx_RUL_BAdcCoding_t;

/**
 * \brief Type B protocol info FO Coding Types.
 */
typedef enum ptx_RUL_BFoCoding
{
    ptx_RUL_BFoCoding_nad_did_not_supported        = 0x00,      /**< FO Coding NAD and DID not supported. */
    ptx_RUL_BFoCoding_did_supported                = 0x01,      /**< FO coding DID Supported. */
    ptx_RUL_BFoCoding_nad_supported                = 0x02,      /**< FO Coding NAD supported. */
    ptx_RUL_BFoCoding_did_nad_supported            = 0x03,      /**< FO Coding NAd and DID supported. */
} ptx_RUL_BFoCoding_t;

/**
 * \brief Type F SENSF_RES Advanceed protocol Features Byte 19 automatic 
 * selection of bit rate feature Types.
 */
typedef enum ptx_RUL_FAutomaticSelectableBr
{
    ptx_RUL_FAutomaticSelectableBr_not_supported        = 0x00,      /**< Automatically selectable bit rate not supported. */
    ptx_RUL_FAutomaticSelectableBr_supported            = 0x01,      /**< Automatically selectable bit rate supported.  */
} ptx_RUL_FAutomaticSelectableBr_t;

/**
 * \brief Type F SENSF_RES Advanceed protocol Features Byte 19 automatic 
 * Bit rate support types.
 */
typedef enum ptx_RUL_FBitRateSupported
{
    ptx_RUL_FBitRateSupported_212_kbps                  = 0x01,      /**< . bitrate 212 Kbps is supported*/
    ptx_RUL_FBitRateSupported_424_kbps                  = 0x02,      /**< . bitrate 424 Kbps is supported*/
    ptx_RUL_FBitRateSupported_212_424_kbps              = 0x03,      /**< . bitrate 212 and 424 Kbps is supported*/
    ptx_RUL_FBitRateSupported_848_kbps                  = 0x04,      /**< . bitrate 848 Kbps is supported*/
    ptx_RUL_FBitRateSupported_212_848_kbps              = 0x05,      /**< . bitrate 212 and 848 Kbps is supported*/
    ptx_RUL_FBitRateSupported_424_848_kbps              = 0x06,      /**< . bitrate 424 and 848 Kbps is supported*/
    ptx_RUL_FBitRateSupported_212_424_848_kbps          = 0x07,      /**< . bitrate 212 and 424 and 848 Kbps is supported*/
    ptx_RUL_FBitRateSupported_1_6_mbps                  = 0x08,      /**< . bitrate 1.6 MBps is supported*/
    ptx_RUL_FBitRateSupported_212_kbps_1_6_mbps         = 0x09,      /**< . bitrate 212 kbps and 1.6 MBps are supported*/
    ptx_RUL_FBitRateSupported_424_kbps_1_6_mbps         = 0x0A,      /**< . bitrate 424 kbps and 1.6 MBps are supported*/
    ptx_RUL_FBitRateSupported_212_424_kbps_1_6_mbps     = 0x0B,      /**< . bitrate 212, 424 kbps and 1.6 MBps are supported*/
    ptx_RUL_FBitRateSupported_848_kbps_1_6_mbps         = 0x0C,      /**< . bitrate 848 kbps and 1.6 MBps are supported*/
    ptx_RUL_FBitRateSupported_212_848_kbps_1_6_mbps     = 0x0D,      /**< . bitrate 212, 848 kbps and 1.6 MBps are supported*/
    ptx_RUL_FBitRateSupported_424_848_kbps_1_6_mbps     = 0x0E,      /**< . bitrate 424, 848 kbps and 1.6 MBps are supported*/
    ptx_RUL_FBitRateSupported_212_424_848_kbps_1_6_mbps = 0x0F,      /**< . bitrate 424, 848 kbps and 1.6 MBps are supported*/

} ptx_RUL_FBitRateSupported_t;

/**
 * \brief ATR_RES General bytes availability 
 */
typedef enum ptx_RUL_GeneralBytesAvailability
{
    ptx_RUL_GeneralBytesAvailabilityNotSupported        = 0x00,      /**< Automatically selectable bit rate not supported. */
    ptx_RUL_GeneralBytesAvailabilitySupported            = 0x01,      /**< Automatically selectable bit rate supported.  */
} ptx_RUL_GeneralBytesAvailability_t;
/**
 * \brief Protocol Type in Protocol Info field of Type B SENSB_RES.
 */
typedef struct ptx_RUL_BProtocolType
{
    ptx_RUL_MinTr2B_t minTr2B;                /**< Minimum TR2B field in protocol type field of protocol info of SENSB_RES. */
    ptx_RUL_BPlatformType_t platformType;     /**< Platform type field in protocol type field of protocol info of SENSB_RES. */
} ptx_RUL_BProtocolType_t;
/**
 * \brief Application Data field in SENSB_RES.
 */
typedef struct ptx_RUL_SensBResponseAppData
{
    uint8_t Afi;                                /**< AFI Field in the SENSB_RES. */
    uint8_t CrcBAid[PTX_RUL_CRC_B_AID_LEN];     /**< The CRC_B(AID) in the SENSB_RES. */
    uint8_t NmbrOfApplications;                 /**< The number of applications in SENSB_RES. */
} ptx_RUL_SensBResponseAppData_t;

/**
 * \brief Protocol Info in SENSB_RES.
 */
typedef struct ptx_RUL_SensBResponseProtocolInfo
{
    uint8_t BitRateCapability;                       /**< Bit_Rate_Capability in the protocol info of SENSB_RES. */
    ptx_RUL_FsciToFsc_t Fsci;                        /**< The Frame Size for proximity Card Integer. */
    ptx_RUL_BProtocolType_t ProtocolType;            /**< Encodes the platform Type support and tje minimum TRB2. */
    uint8_t Fwi;                                     /**< The Frame Waiting Time Integer. */
    ptx_RUL_BAdcCoding_t Adc;                        /**< The application data coding supported. */
    ptx_RUL_BFoCoding_t Fo;                          /**< The Frame Option. */
    uint8_t Sfgi;                                    /**< The Start-up Frame Guard Time Integer. */
} ptx_RUL_SensBResponseProtocolInfo_t;
#endif
/**
 * \brief Card Rf Technology Parameter Type A.
 */
typedef struct ptx_RUL_CardRfTechParamA
{
    uint8_t SensRes[PTX_RUL_SENSRES_LEN];       /**< SENS_REQ/ALL_REQ. */
    uint8_t NfcId1Len;                          /**< Length of the Nfc Id. */
    uint8_t NfcId1[PTX_RUL_NFCID1_MAX_LEN];     /**< Nfc Id of the Card. */
    uint8_t SelRes;                             /**< Response to SEL_REQ command. */
    ptx_RUL_RfBitRate_t TxBitRate;              /**< TX Bit Rate. */
    ptx_RUL_RfBitRate_t RxBitRate;              /**< RX Bit Rate. */
} ptx_RUL_CardRfTechParamA_t;

#if defined(ENABLE_EXTENDED_CARD_REGISTER)
/**
 * \brief Card Rf Technology Parameter Type B.
 */
typedef struct ptx_RUL_CardRfTechParamB
{
    uint8_t SensBRes[PTX_RUL_SENSBRES_LEN];             /**< Response to SENSB_REQ/ALLB_REQ command. */
    uint8_t Nfcid0[PTX_RUL_NFCID0_LEN];                 /**< NFCID0 field in the SENSB_RES. */
    ptx_RUL_SensBResponseAppData_t appData;             /**< The Application Data field in the SENSB_RES. */
    ptx_RUL_SensBResponseProtocolInfo_t protocolInfo;   /**< The protocol Info */
    ptx_RUL_RfBitRate_t TxBitRate;                      /**< TX Bit Rate. */
    ptx_RUL_RfBitRate_t RxBitRate;                      /**< RX Bit Rate. */
} ptx_RUL_CardRfTechParamB_t;

/**
 * \brief Card Rf Technology Parameter Type F.
 */
typedef struct ptx_RUL_CardRfTechParamF
{
    uint8_t SensFRes[PTX_RUL_SENSFRES_LEN];                 /**< Response to SENSF_REQ command. */
    uint8_t Nfcid2[PTX_RUL_NFCID2_LEN];                     /**< NFCID2 field in the SENSF_RES */
    uint8_t Pad0[PTX_RUL_SENSF_PAD0_LEN];                   /**< PAD0 field in the SENSF_RES */
    uint8_t Pad1[PTX_RUL_SENSF_PAD1_LEN];                   /**< PAD1 field in the SENSF_RES */
    uint8_t MrtiCheck;                                      /**< MRTI_CHECK field in the SENSF_RES */
    uint8_t MrtiUpdate;                                     /**< MRTI_UPDATE field in the SENSF_RES */
    uint8_t Pad2;                                           /**< Pad2 field in the SENSF_RES */
    ptx_RUL_FAutomaticSelectableBr_t automaticBrSel;        /**< Automatic bit rate selection in byte 19 RD of SENSF_RES */  
    ptx_RUL_FBitRateSupported_t supportedBrFeature ;        /**< Supported Bit Rate in RD field of SENSF_RES. */
    uint8_t RD[PTX_RUL_RD_LEN];                             /**< RD Request Data field in the SENSF_RES */
    ptx_RUL_RfBitRate_t BitRate;                            /**< Bit Rate. */
} ptx_RUL_CardRfTechParamF_t;

/**
 * \brief Card Rf Technology Parameter Type V.
 */
typedef struct ptx_RUL_CardRfTechParamV
{
    uint8_t InventoryRes[PTX_RUL_INVENTORYRES_LEN];     /**< Response to INVENTORY_REQ command */
    uint8_t RedFlag;                                    /**< RED_FLAG byte in the INVENTORY_RES */
    uint8_t Dsfid;                                      /**< DSFID byte in the INVENTORY_RES */
    uint8_t Uid[PTX_RUL_INVENTORYRES_UID_LEN];          /**< UID bytes in the INVENTORY_RES */
    ptx_RUL_RfBitRate_t BitRate;                        /**< Bit Rate. */
} ptx_RUL_CardRfTechParamV_t;

/**
 * \brief Card Rf Technology Parameter Type P2P-ACM.
 */
typedef struct ptx_RUL_CardRfTechParamP2PACM
{
    uint8_t AtrRes[PTX_RUL_CARD_RFPROTOCOL_ATR_RES_LEN];      /**< ATR_RES raw bytes */
    uint8_t Nfcid3[PTX_RUL_ATR_RES_NFCID3_LEN] ;              /**< NFCID3 identifier of target NFC-DEP Protocol */
    uint8_t GeneralBytes[PTX_RUL_ACM_GENERALBYTES_LEN];       /**< Optional general bytes in ATR_RES */
    uint8_t Did;                                              /**< DID field in ATR_RES, the target device identification number */
    uint8_t Bs;                                               /**< The bitrate in ACM supported by the Target in sending */
    uint8_t Br;                                               /**< The bitrate in ACM supported by the Target in receiving  */
    uint8_t To;                                               /**< The TO field in ATR_RES, contains the waiting time value */
    uint8_t PP;                                               /**< PP field in ATR_RES, contains the length reduction and optional parameters presence */
    uint8_t Wt;                                               /**< The Waiting Time value in TO field of in ATR_RES */
} ptx_RUL_CardRfTechParamP2PACM_t;

/**
 * \brief Card Rf Technology Parameter.
 */
typedef union ptx_RUL_CardRfTechParam
{
    ptx_RUL_CardRfTechParamA_t          TypeA;          /**< Card Technology Type A. */
    ptx_RUL_CardRfTechParamB_t          TypeB;          /**< Card Technology Type B. */
    ptx_RUL_CardRfTechParamF_t          TypeF;          /**< Card Technology Type F. */
    ptx_RUL_CardRfTechParamV_t          TypeV;          /**< Card Technology Type V. */
    ptx_RUL_CardRfTechParamP2PACM_t     TypeP2PACM;     /**< Card Technology Type P2P-ACM. */
} ptx_RUL_CardRfTechParam_t;

#else
/**
 * \brief Card Rf Technology Parameter Type B.
 */
typedef struct ptx_RUL_CardRfTechParamB
{
    uint8_t SensBRes[PTX_RUL_SENSBRES_LEN];     /**< Response to SENSB_REQ/ALLB_REQ command. */
    ptx_RUL_RfBitRate_t TxBitRate;              /**< TX Bit Rate. */
    ptx_RUL_RfBitRate_t RxBitRate;              /**< RX Bit Rate. */
} ptx_RUL_CardRfTechParamB_t;

/**
 * \brief Card Rf Technology Parameter Type F.
 */
typedef struct ptx_RUL_CardRfTechParamF
{
    uint8_t SensFRes[PTX_RUL_SENSFRES_LEN];     /**< Response to SENSF_REQ command. */
    ptx_RUL_RfBitRate_t BitRate;                /**< Bit Rate. */
} ptx_RUL_CardRfTechParamF_t;

/**
 * \brief Card Rf Technology Parameter Type V.
 */
typedef struct ptx_RUL_CardRfTechParamV
{
    uint8_t InventoryRes[PTX_RUL_INVENTORYRES_LEN];     /**< Response to INVENTORY_REQ command */
    ptx_RUL_RfBitRate_t BitRate;                        /**< Bit Rate. */
} ptx_RUL_CardRfTechParamV_t;

/**
 * \brief Card Rf Technology Parameter.
 */
typedef union ptx_RUL_CardRfTechParam
{
    ptx_RUL_CardRfTechParamA_t          TypeA;          /**< Card Technology Type A. */
    ptx_RUL_CardRfTechParamB_t          TypeB;          /**< Card Technology Type B. */
    ptx_RUL_CardRfTechParamF_t          TypeF;          /**< Card Technology Type F. */
    ptx_RUL_CardRfTechParamV_t          TypeV;          /**< Card Technology Type V. */
} ptx_RUL_CardRfTechParam_t;
#endif

/**
 * \brief Card RF State.
 */
typedef enum ptx_RUL_CardRfState
{
    ptx_RUL_CardRfState_Active = 0,  /**< State Active. */
    ptx_RUL_CardRfState_Sleep  = 1   /**< State Sleep. */
} ptx_RUL_CardRfState_t;

/**
 * \brief Supported Protocols.
 */
typedef enum ptx_RUL_SupportedProtocols
{
    ptx_RUL_SupportedProtocols_Undefined = 0x00, /**< Protocol Undefined. */
    ptx_RUL_SupportedProtocols_T2T      = 0x01, /**< Protocol T2T. */
    ptx_RUL_SupportedProtocols_T3T      = 0x02, /**< Protocol T3T. */
    ptx_RUL_SupportedProtocols_T4T      = 0x04, /**< Protocol T4T. */
    ptx_RUL_SupportedProtocols_NfcDep   = 0x08, /**< Protocol NFC-DEP. */
    ptx_RUL_SupportedProtocols_T5T      = 0x10  /**< Protocol T5T. */
} ptx_RUL_SupportedProtocols_t;


/**
 * \brief ISO-DEP Type-A Poll-mode specific RF-protocol activation parameters.
 */
typedef struct ptx_RUL_CardRfProtocolParamPollAIsoDep
{
    uint8_t     ATS[PTX_RUL_CARD_RFPROTOCOL_ATS_LEN];               /**< Received ATS Response. */
    uint8_t     PPS1;                                               /**< PPS1 containing activated Bitrate information */
} ptx_RUL_CardRfProtocolParamPollAIsoDep_t;

/**
 * \brief ISO-DEP Type-B Poll-mode specific RF-protocol activation parameters.
 */
typedef struct ptx_RUL_CardRfProtocolParamPollBIsoDep
{
    uint8_t     AttribParam1;                                       /**< Param 1 of the ATTRIB command. */
    uint8_t     AttribParam2;                                       /**< Param 2 of the ATTRIB command. */
    uint8_t     AttribResLength;                                    /**< Length of received Attrib response. */
    uint8_t     AttribRes[PTX_RUL_CARD_RFPROTOCOL_ATTRIBRES_LEN];   /**< Received Attrib response. */
} ptx_RUL_CardRfProtocolParamPollBIsoDep_t;

/**
 * \brief NFC-DEP Initiator-mode specific RF-protocol activation parameters.
 */
typedef struct ptx_RUL_CardRfProtocolParamPollAFACMNfcDep
{
    uint8_t     AtrRes[PTX_RUL_CARD_RFPROTOCOL_ATR_RES_LEN];        /**< Received ATR_RES. */
    uint8_t     Brs;                                                /**< BRS containing acivated Bitrate information. */
} ptx_RUL_CardRfProtocolParamPollAFACMNfcDep_t;

/**
 * \brief ISO-DEP Type-A Listen-mode specific RF-protocol activation parameters.
 */
typedef struct ptx_RUL_CardRfProtocolParamListenAIsoDep
{
    uint8_t     Rats;                                               /**< Received RATS command parameter. */
    uint8_t     Pps1;                                               /**< PPS1 containing acivated Bitrate information. */
} ptx_RUL_CardRfProtocolParamListenAIsoDep_t;

/**
 * \brief ISO-DEP Type-B Listen-mode specific RF-protocol activation parameters.
 */
typedef struct ptx_RUL_CardRfProtocolParamListenBIsoDep
{
    uint8_t     AttribLen;                                          /**< Length of received ATTRIB command. */
    uint8_t     Attrib[PTX_RUL_CARD_RFPROTOCOL_ATTRIB_LEN];         /**< Received ATTRIB command. */
} ptx_RUL_CardRfProtocolParamListenBIsoDep_t;

/**
 * \brief NFC-DEP Target-mode specific RF-protocol activation parameters.
 */
typedef struct ptx_RUL_CardRfProtocolParamsListenAFNfcDep
{
    uint8_t     AtrReq[PTX_RUL_CARD_RFPROTOCOL_ATR_REQ_LEN];        /**< Received ATR_REQ. */
    uint8_t     Brs;                                                /**< BRS containing activated Bitrate information. */
    uint8_t     Lr;                                                 /**< LR containing length reduction information. */
} ptx_RUL_CardRfProtocolParamListenAFNfcDep_t;

/**
 * \brief Generic RF-protocol activation parameters.
 */
typedef union ptx_RUL_CardRfProtocolParam
{
    ptx_RUL_CardRfProtocolParamPollAIsoDep_t        PollAIsoDep;            /**< ISO-DEP Type-A Poll-mode specific RF-protocol activation parameters. */
    ptx_RUL_CardRfProtocolParamPollBIsoDep_t        PollBIsoDep;            /**< ISO-DEP Type-B Poll-mode specific RF-protocol activation parameters. */
    ptx_RUL_CardRfProtocolParamPollAFACMNfcDep_t    PollAFACMNfcDep;        /**< NFC-DEP Initiator-mode specific RF-protocol activation parameters. */
    ptx_RUL_CardRfProtocolParamListenAIsoDep_t      ListenAIsoDep;          /**< ISO-DEP Type-A Listen-mode specific RF-protocol activation parameters. */
    ptx_RUL_CardRfProtocolParamListenBIsoDep_t      ListenBIsoDep;          /**< ISO-DEP Type-B Listen-mode specific RF-protocol activation parameters. */
    ptx_RUL_CardRfProtocolParamListenAFNfcDep_t     ListenAFNfcDep;         /**< NFC-DEP Target-mode specific RF-protocol activation parameters. */
} ptx_RUL_CardRfProtocolParam_t;

/**
 * \brief Card parameters.
 */
typedef struct ptx_RUL_CardParams
{
    ptx_RUL_CardRfTechType_t        TechType;   /**< Card Technology Type. */
    ptx_RUL_CardRfTechParam_t       TechParam;  /**< Card Technology Parameters. */
    ptx_RUL_CardRfState_t           State;      /**< Cards State. */
    uint8_t                         Protocols;  /**< Cards Protocol. */
} ptx_RUL_CardParams_t;

/**
 * \brief Card registry storing information about discovered cards.
 */
typedef struct ptx_RUL_CardRegistry
{
    ptx_RUL_CardParams_t            Cards[PTX_RUL_MAX_SUPPORTED_DEVICES]; /**< Card parameters. */
    uint32_t                        CardsCnt;                             /**< Number of detected cards. */
    ptx_RUL_CardParams_t           *ActiveCard;                           /**< Pointer to active card. */
    ptx_RUL_CardProtocolType_t      ActiveProtocol;                       /**< Used protocol for active card. */
    ptx_RUL_CardRfProtocolParam_t   ActiveProtParam;                      /**< Card Protocol Parameters. */
    uint32_t                        ActiveCardId;                         /**< Id of the card in the Cards list. */
} ptx_RUL_CardRegistry_t;

/**
 * \brief Bits Exchange Mode State.
 */
typedef enum ptx_RUL_BitsExchangeMode
{
    ptx_RUL_BitsExchangeMode_On,        /**< Turn Bits Exchange Mode On. */
    ptx_RUL_BitsExchangeMode_Off,       /**< Turn Bits Exchange Mode Off. */

    ptx_RUL_BitsExchangeMode_Max,       /**< Safeguard. */
} ptx_RUL_BitsExchangeMode_t;

/**
 * \brief Bits Exchange Data Parameters.
 */
typedef struct ptx_RUL_BitsExchangeParams
{
    uint8_t   *Data;      /**< Data container. */
    uint8_t   *Parity;    /**< Parity container. */
    uint32_t  DataLen;    /**< Length of Data and Parity container. */
} ptx_RUL_BitsExchangeParams_t;

/**
 * \brief RSSI Mode Types.
 */
typedef enum ptx_RUL_RSSIMode
{
    ptx_RUL_RSSIMode_Enabled, /**< Enables RSSI Mode. */
    ptx_RUL_RSSIMode_Disabled,/**< Disables RSSI Mode. */

    ptx_RUL_RSSIMode_Max,     /**< Safeguard. */
} ptx_RUL_RSSIMode_t;

/**
 * \brief RSSI Parameters.
 */
typedef struct ptx_RUL_RSSIParams
{
    ptx_RUL_RSSIMode_t  Mode;       /**< RSSI Mode. */
    uint8_t             RssiPeriod; /**< RSSI Period. */
} ptx_RUL_RSSIParams_t;

/**
 * \brief GPIO Configuration.
 */
typedef struct ptx_GPIOCfg
{
    uint8_t Address;            /**< GPIO Register Address. */
    uint8_t ExtendedCfgEn;      /**< GPIO Extended Config Support flag. */
} ptx_GPIOCfg_t;

/**
 * \brief GPIO Pin
 */
typedef enum ptx_RUL_GPIONbr
{
    ptx_RUL_GPIONbr_1 = 1,  /**< GPIO Pin 1. */
    ptx_RUL_GPIONbr_2 = 2,  /**< GPIO Pin 2. */
    ptx_RUL_GPIONbr_3 = 3,  /**< GPIO Pin 3. */
    ptx_RUL_GPIONbr_4 = 4,  /**< GPIO Pin 4. */
    ptx_RUL_GPIONbr_5 = 5,  /**< GPIO Pin 5. */
    ptx_RUL_GPIONbr_6 = 6,  /**< GPIO Pin 6. */
    ptx_RUL_GPIONbr_7 = 7,  /**< GPIO Pin 7. */
    ptx_RUL_GPIONbr_8 = 8,  /**< GPIO Pin 8. */

    ptx_RUL_GPIONbr_Max,    /**< Safeguard. */
} ptx_RUL_GPIONbr_t;

/**
 * GPIO Values.
 */
typedef enum ptx_RUL_GPIOValue
{
    ptx_RUL_GPIOValue_Low,                  /**< GPIO Value is Low. */
    ptx_RUL_GPIOValue_High,                 /**< GPIO Value is High. */

    ptx_RUL_GPIOValue_Max,                  /**< Safeguard. */
} ptx_RUL_GPIOValue_t;

/**
 * GPIO Direction.
 */
typedef enum ptx_RUL_GPIODir
{
    ptx_RUL_GPIODir_Input,                  /**< GPIO Direction Input. */
    ptx_RUL_GPIODir_Output,                 /**< GPIO Direction Output. */

    ptx_RUL_GPIOCfg_Max,                    /**< Safeguard. */
} ptx_RUL_GPIODir_t;

/**
 * Supported GPIO-Flags
 */
typedef enum ptx_RUL_GPIOFlags
{
    ptx_RUL_GPIOFlags_None                      = 0x00u,    /**< None. No Flags. */
    ptx_RUL_GPIOFlags_Internal_Pull_Resistor    = 0x01u,    /**< GPIO uses Internal Pull Resistor. */
    ptx_RUL_GPIOFlags_InputGlitchFilter         = 0x02u,    /**< GPIO uses InputGlitchFilter. */
    ptx_RUL_GPIOFlags_Weakest_Driver_Strength   = 0x10u,    /**< GPIO uses Weakest Driver Strength. */
    ptx_RUL_GPIOFlags_Weak_Driver_Strength      = 0x20u,    /**< GPIO uses Weak Driver Strength. */
    ptx_RUL_GPIOFlags_Strong_Driver_Strength    = 0x40u,    /**< GPIO uses Strong Driver Strength. */
    ptx_RUL_GPIOFlags_Strongest_Driver_Strength = 0x80u,    /**< GPIO uses Strongest Driver Strength. */
} ptx_RUL_GPIOFlags_t;

/**
 * \brief RUL Get Info Types
*/
typedef enum ptx_RUL_InfoTypes
{
    ptx_RUL_InfoType_HW_Version,        /**< Hardware version */
    ptx_RUL_InfoType_Stack_Revision,    /**< Stack revision */
    ptx_RUL_InfoType_FW_Revision,       /**< Firmware revision */
    ptx_RUL_InfoType_FW_Tool_Revision,  /**< Firmware tool revision */
    ptx_RUL_InfoType_RF_Config_Version, /**< RF Config mapping version */
    ptx_RUL_InfoType_Chip_Derivate,     /**< Commercial derivate of the chip */
    ptx_RUL_InfoType_Chip_Temperature,  /**< Current chip temperature. */
    ptx_RUL_InfoType_Mtu_Size,          /**< MTU (max. transfer unit) size. */
    ptx_RUL_InfoType_Local_Changes,     /**< Local changes. */
    ptx_RUL_InfoType_Platform_Revision, /**< Git hash of current version. */

    ptx_RUL_InfoType_Max,               /**< Safeguard. */
} ptx_RUL_InfoTypes_t;


/**
 * \brief Hardware RF- and System-Configuration IDs
*/
typedef enum ptx_RUL_HWConfigID
{
    /* RF-Configuration IDs */
    ptx_RUL_RF_CFG_POLL,            /**< Parameter Id for Poll parameters. */
    ptx_RUL_RF_CFG_POLL_A_106,      /**< Parameter Id for Poll A parameters with 106kbit/s. */
    ptx_RUL_RF_CFG_POLL_B_106,      /**< Parameter Id for Poll B parameters with 106kbit/s. */
    ptx_RUL_RF_CFG_POLL_F_212,      /**< Parameter Id for Poll F parameters with 212k/s. */
    ptx_RUL_RF_CFG_POLL_F_424,      /**< Parameter Id for Poll F parameters with 424kbit/s. */
    ptx_RUL_RF_CFG_POLL_V_26K5,     /**< Parameter Id for Poll V parameters with 26.5kbit/s. */
    ptx_RUL_RF_CFG_POLL_A_212,      /**< Parameter Id for Poll A parameters with 212kbit/s. */
    ptx_RUL_RF_CFG_POLL_A_424,      /**< Parameter Id for Poll A parameters with 424kbit/s. */
    ptx_RUL_RF_CFG_POLL_A_848,      /**< Parameter Id for Poll A parameters with 848kbit/s. */
    ptx_RUL_RF_CFG_POLL_B_212,      /**< Parameter Id for Poll B parameters with 212kbit/s. */
    ptx_RUL_RF_CFG_POLL_B_424,      /**< Parameter Id for Poll B parameters with 424kbit/s. */
    ptx_RUL_RF_CFG_POLL_B_848,      /**< Parameter Id for Poll B parameters with 848kbit/s. */
    ptx_RUL_RF_CFG_LISTEN,          /**< Parameter Id for Listen parameters. */
    ptx_RUL_RF_CFG_LISTEN_A_212,    /**< Parameter Id for Listen A parameters with 212kbit/s. */
    ptx_RUL_RF_CFG_LISTEN_A_424,    /**< Parameter Id for Listen A parameters with 424kbit/s. */
    ptx_RUL_RF_CFG_LISTEN_A_848,    /**< Parameter Id for Listen A parameters with 848kbit/s. */
    ptx_RUL_RF_CFG_LISTEN_B_212,    /**< Parameter Id for Listen B parameters with 212kbit/s. */
    ptx_RUL_RF_CFG_LISTEN_B_424,    /**< Parameter Id for Listen B parameters with 424kbit/s. */
    ptx_RUL_RF_CFG_LISTEN_B_848,    /**< Parameter Id for Listen B parameters with 848kbit/s. */
    ptx_RUL_RF_CFG_POLL_V_53,       /**< Parameter Id for Poll V parameters with 56kbit/s. */
    ptx_RUL_RF_CFG_POLL_V_106,      /**< Parameter Id for Poll V parameters with 106kbit/s. */

    ptx_RUL_RF_CFG_MAX,             /**< Safeguard RF Configuration Parameters. */


    /* System Configuration IDs */
    ptx_RUL_SYS_CFG_ClkSel,         /**< Parameter Id for Clock Selection parameter. */
    ptx_RUL_SYS_CFG_RefFreq,        /**< Parameter Id for reference frequency parameter. */
    ptx_RUL_SYS_CFG_TempErrTresh,   /**< Parameter Id for Temperature Threshold. */
    ptx_RUL_SYS_CFG_TxCurlimGain,   /**< Parameter Id for Current Limiter Gain. */

    ptx_RUL_SYS_CFG_MAX,            /**< Safeguard System Configuration Parameters. */

} ptx_RUL_HWConfigID_t;

/**
 * \brief Hardware Config Value Union
*/
typedef union ptx_RUL_HWConfigValue
{
    uint8_t            *Ptr;       /**< Pointer to array */
    uint32_t            Value;     /**< Value */
} ptx_RUL_HWConfigValue_t;

/**
 * \brief Hardware RF- and System-Configuration Identifiers
 */
typedef struct ptx_RUL_HWConfig
{
    ptx_RUL_HWConfigID_t    ID;         /**< Hardware RF and System Config ID */
    ptx_RUL_HWConfigValue_t Data;       /**< Data */
    uint8_t                 Len;        /**< Length */
} ptx_RUL_HWConfig_t;

typedef void ptx_RUL_Extension_Ctx_t;   /**< Forward declaration of CmdProcessor */

/**
 * \brief RUL ADC Averaging setting.
 */
typedef enum ptx_RUL_AdcAveraging
{
    AdcAveraging_01Samples  = 0x00,     /** ADC averaging with 1 sample. */
    AdcAveraging_02Samples  = 0x01,     /** ADC averaging with 2 samples. */
    AdcAveraging_04Samples  = 0x02,     /** ADC averaging with 4 samples. */
    AdcAveraging_08Samples  = 0x03,     /** ADC averaging with 8 samples. */
    AdcAveraging_16Samples  = 0x04,     /** ADC averaging with 16 samples. */
    AdcAveraging_32Samples  = 0x05,     /** ADC averaging with 32 samples. */
    AdcAveraging_Max                    /** Safeguard. */
} ptx_RUL_AdcAveraging_t;

/**
 * \brief Callback definition to process a reset notification.
 *
 * \param[in]   ctx       Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_RUL_Extension_ProcessResetNtfCb_t) (void *ctx);

/**
 * \brief Callback definition to process a rf field notification.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   field     Flag whether field is ON or OFF.w
 * \param[out]  ntf_used  Flag to set if the callback has used the received notification.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_RUL_Extension_ProcessRfFieldNtfCb_t) (void *ctx, uint8_t field, uint8_t *ntf_used);

/**
 * \brief Callback definition to process a rf discover notification.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   ntf       Pointer to notification.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_RUL_Extension_ProcessRfDiscoverNtfCb_t) (void *ctx, ptx_NSC_RfDiscover_Ntf_t *ntf);

/**
 * \brief Callback definition to process a rf LPCD notification.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[out]  ntf_used  Flag to set if the callback has used the received notification.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_RUL_Extension_ProcessRfLpcdNtfCb_t) (void *ctx, uint8_t *ntf_used);

/**
 * \brief Callback definition to process a rf activate notification.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   ntf       Pointer to notification.
 * \param[out]  ntf_used  Flag to notify that the notification was used by the callback.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_RUL_Extension_ProcessRfActivateNtfCb_t) (void *ctx, ptx_NSC_RfActivate_Ntf_t *ntf, uint8_t *ntf_used);

/**
 * \brief Callback definition to process a rf deactivate notification.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   ntf       Pointer to notification.
 * \param[out]  ntf_used  Flag to notify that the notification was used by the callback.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_RUL_Extension_ProcessRfDeactivateNtfCb_t) (void *ctx, ptx_NSC_RfDeactivate_Ntf_t *ntf, uint8_t *ntf_used);

/**
 * \brief Callback definition to process a error notification.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   ntf       Pointer to notification.
 * \param[out]  ntf_used  Flag to notify that the notification was used by the callback.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_RUL_Extension_ProcessErrorNtfCb_t) (void *ctx, ptx_NSC_Error_Ntf_t *ntf, uint8_t *ntf_used);

/**
 * \brief Callback definition to process a WLC WPT stop notification
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   ntf       Pointer to notification.
 * \param[out]  ntf_used  Flag to set if the callback has used the received notification.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_RUL_Extension_ProcessWlcWptStopNtfCb_t) (void *ctx, ptx_NSC_WLC_WptStop_Ntf_t *ntf, uint8_t *ntf_used);

/**
 * \brief Callback definition to process a data message.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   msg       Pointer to data message (structure).
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_RUL_Extension_ProcessDataMsgCb_t) (void *ctx, ptx_NSC_RfData_Msg_t *msg);

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Extension Callback Structure
 */
typedef struct ptx_RUL_Extension_Endpoints
{
    ptx_Comps_t                                      CompId;              /**< Component ID to identify extensions. */
    void                                            *Extension;           /**< Pointer to extension component (e.g. HCE, WLC etc.). */
    ptx_RUL_Extension_ProcessResetNtfCb_t            ProcessReset;        /**< A-Sync communication reset notification. */
    ptx_RUL_Extension_ProcessRfDiscoverNtfCb_t       ProcessRfDiscover;   /**< A-Sync communication rf discover notification. */
    ptx_RUL_Extension_ProcessRfLpcdNtfCb_t           ProcessRfLpcd;       /**< A-Sync communication rf LPCD notification. */
    ptx_RUL_Extension_ProcessRfActivateNtfCb_t       ProcessRfActivate;   /**< A-Sync communication rf activate notification. */
    ptx_RUL_Extension_ProcessRfDeactivateNtfCb_t     ProcessRfDeactivate; /**< A-Sync communication rf deactivate notification. */
    ptx_RUL_Extension_ProcessErrorNtfCb_t            ProcessError;        /**< A-Sync communication error notification. */
    ptx_RUL_Extension_ProcessRfFieldNtfCb_t          ProcessRfField;      /**< A-Sync communication rf field notification. */
    ptx_RUL_Extension_ProcessWlcWptStopNtfCb_t       ProcessWptStop;      /**< A-Sync communication wlc wpt stop notification. */
    ptx_RUL_Extension_ProcessDataMsgCb_t             ProcessDataMsg;      /**< A-Sync communication data message. */
} ptx_RUL_Extension_Endpoints_t;

/**
 * \brief EMV Collision Filter
 */
typedef struct ptx_RUL_EmvCollisionFilter
{
    uint8_t     Limit;      /**< Collision Filter Limit */
    uint8_t     Counter;    /**< Collision Filter Counter*/
} ptx_RUL_EmvCollisionFilter_t;

/**
 * \brief RUL Device (Communication Interface) Initialisation Parameters.
 */
typedef struct ptx_RUL_InterfaceInitParams
{
    uint8_t                             *InterfaceName;         /**< Device name. E.g. COM port used on OS-based platform for UART interface. */
    uint16_t                            InterfaceNameLen;       /**< Device name string length. */
    ptx_HAL_Interface_Type_t            InterfaceType;          /**< Type of the hardware interface. */
    ptx_HAL_DRV_Interface_Port_Type_t   InterfacePortType;      /**< Implementation Type of the hardware interface port. */
    uint8_t                             InterfacePortID;        /**< ID of the port for the hardware interface. */
    uint8_t                             NumberOfTimers;         /**< Number of requested Timers. */
} ptx_RUL_InterfaceInitParams_t;

/**
 * \brief RUL Initialisation Parameters.
 */
typedef struct ptx_RUL_InitParams
{
    uint8_t                                 EmvCollisionLimit;     /**< EMV collision limit (Required for EMVCO/POS)*/
    ptx_RUL_InterfaceInitParams_t           InterfaceInitParams;   /**< Communication hardware interface parameters. */
} ptx_RUL_InitParams_t;


/**
 * \brief Standby and wake-up configuration.
 *  Enable standby mode by selecting any of the wake-up sources. One or multiple wake-up
 *  sources might be selected.
 *  If standby mode is enabled, wake-up by HIF will automatically be set (internally). However, if
 *  it is required to enable only wake-up by HIF, EnableWuByHif must be explicitly set.
 *  If all standby configuration bytes are set to 0, i.e. none of the wake-up sources is selected,
 *  standby mode will be disabled.
 */
typedef struct ptx_RUL_StandbyCfg
{
    /**
     * Wake-up by Timer.
     * 0 =     Wake-up by timer disabled.
     * 1 =     Wake-up by timer enabled.
     */
    uint8_t     EnableWuByTimer;

    /**
     * Wake-up by LPFD (Low Power Field Detect).
     * 0 =     Wake-up by LPFD disabled.
     * 1 =     Wake-up by LPFD enabled.
     */
    uint8_t     EnableWuByLpfd;

    /**
     * Wake-up by HIF (Host Interface).
     * 0 =     Wake-up by HIF disabled.
     * 1 =     Wake-up by HIF enabled.
     */
    uint8_t     EnableWuByHif;

    /**
     * Wake-up by GPIO.
     * 0 =     Wake-up by GPIO disabled.
     * 1 =     Wake-up by GPIO enabled.
     */
    uint8_t     EnableWuByGpio;
} ptx_RUL_StandbyCfg_t;

/**
 * \brief Custom Type-F Polling Command Parameters.
 *
 *  Note: Overwriting the internal default parameters may lead to inconsistencies with NFC Forum compliance!
 */
typedef struct ptx_RUL_DiscoverConfigTypeFPollCmdParams
{
    uint8_t     OverwriteDefaultParams;                         /**< Indicator to overwrite default command parameters. */
    uint8_t     SystemCode[PTX_RUL_SENSF_REQ_SYS_CODE_LEN];     /**< 16-Bit System Code according to FeliCa/T3T. */
    uint8_t     RequestCode;                                    /**<  8-Bit Requested Code according to FeliCa/T3T. */
    uint8_t     InitialTimeSlotNr;                              /**<  8-Bit Timeslot-Number according to FeliCa/T3T - used only during initial Polling. */
} ptx_RUL_DiscoverConfigTypeFPollCmdParams_t;


/**
 * \brief RUL Discovery Configuration Parameters.
 */
typedef struct ptx_RUL_DiscoverConfig
{
    /** Enable Poll Mode Parameters. */
    uint8_t     EnablePolling;

    /**
     * RF-Discovery Poll Mode.
     * 0 =     Poll Mode Disabled.
     * 1 =     NFC Forum poll mode.
     * 2 =     EMVCo poll mode.
     * 3 =     ISO poll mode.
     * 4 =     Activate RF field and move to RF_POLL state.
     * 5 =     LPCD poll mode.
     */
    ptx_RUL_PollMode_t PollMode;

    /**
     * Guard Time.
     * Additional guard time in milliseconds after the RF field has been
     * activated and before the first RF command is sent.
     * Range: 0 - 255
     */
    uint8_t     GuardTime;

    /**
     * Enable DDPC (Digital Dynamic Power Control).
     * 0 =      DDPC disabled
     * 1 =      DDPC enabled
     */
    uint8_t     EnableDDPC;

    /**
     * Enable LPCD (Low Power Card Detection).
     * 0 =      LPCD is disabled (regular RF discovery cycle).
     * 1 =      LPCD is used in every discovery cycle.
     * 2-255 =  Hybrid mode where the regular RF discovery.
     *          cycle is used in every (2-255)th discovery cycle, and LPCD
     *          is used in other cycles.
     */
    uint8_t     EnableLPCD;

    /** Enable Notifications whenever LPCD gets triggered. */
    uint8_t     EnableLPCDNtf;

    /** Standby and wake-up configuration. */
    ptx_RUL_StandbyCfg_t    StandbyCfg;

    /**
     * Idle time.
     * Idle time between two consecutive discovery cycles, provided in milliseconds.
     */
    uint32_t    IdleTime;

    /**
     * Initial Polling Technology
     * 0 =      Default (Disabled)
     * 1 =      NFC-A
     * 2 =      NFC-B
     * 3 =      NFC-F
     * 4 =      NFC-V
     */
    ptx_RUL_InitialPollingTech_t    InitialPollingTech;

    /** RF-Discovery Loop. Enable Type A technology. */
    uint8_t     PollTypeA;

    /**
     * Type A Polling Frequency.
     * 0 =      Do not poll for Type A technology.
     * 1 =      Poll for Type A technology in every discovery cycle.
     * 2-10 =   Poll for Type A technology in every (2-10)th
     *          discovery cycle.
     */
    uint8_t     PollTypeAPollFrequency;

    /**
     * RF-Discovery Loop. Max. Type A devices to detect.
     * If set to 0 the pre-configured number of devices will be taken. The pre-configured number of devices
     * is defined by the poll mode \see PollMode.
     */
    uint8_t     PollTypeADeviceLimit;

    /**
     * RF-Discovery Loop. Bail out after NFC-A technology is found.
     * 0 = No Bail out when NFC-A technology is found
     * 1 = Bail out when NFC-A technology is found
     * other values are interpreted as 0, no bail out
     */
    uint8_t     PollTypeABailOut;

    /** RF-Discovery Loop. Enable Type B technology. */
    uint8_t     PollTypeB;

    /**
     * NFC-B Polling Frequency.
     * 0 =      Do not poll for NFC-B technology.
     * 1 =      Poll for NFC-B technology in every discovery cycle.
     * 2-10 =   Poll for NFC-B technology in every (2-10)th
     *          discovery cycle.
     */
    uint8_t     PollTypeBPollFrequency;

    /**
     * RF-Discovery Loop. Max. Type B devices to detect.
     * If set to 0 the pre-configured number of devices will be taken. The pre-configured number of devices
     * is defined by the poll mode \see PollMode.
     */
    uint8_t     PollTypeBDeviceLimit;

    /**
     * RF-Discovery Loop. Bail out after NFC-A or NFC-B technology is found
     * 0 = No Bail out when NFC-A or NFC-B technology is found
     * 1 = Bail out when NFC-A or NFC-B technology is found
     * other values are interpreted as 0, no bail out
     */
    uint8_t     PollTypeBBailOut;

    /** Rf-Discovery Loop. Enable Type F Technology 212 kbps */
    uint8_t     PollTypeF212;

    /** Rf-Discovery Loop. Enable Type F Technology 424 kbps */
    uint8_t     PollTypeF424;

    /** RF-Discovery Loop. Type-F Technology Custom Polling Command Parameters. */
    ptx_RUL_DiscoverConfigTypeFPollCmdParams_t PollTypeFCmdParams;

    /**
     * NFC-F Polling Frequency.
     * 0 =      Do not poll for NFC-F technology.
     * 1 =      Poll for NFC-F technology in every discovery cycle.
     * 2-10 =   Poll for NFC-F technology in every (2-10)th
     *          discovery cycle.
     */
    uint8_t     PollTypeFPollFrequency;

    /**
     * RF-Discovery Loop. Max. Type F devices to detect.
     * If set to 0 the pre-configured number of devices will be taken. The pre-configured number of devices
     * is defined by the poll mode \see PollMode.
     */
    uint8_t     PollTypeFDeviceLimit;

    /**
     * RF-Discovery Loop. Bail out after NFC-A, NFC-B or NFC-F technology is found.
     * 0 = No Bail out when NFC-A, NFC-B or NFC-F technology is found
     * 1 = Bail out when NFC-A, NFC-B or NFC-F technology is found
     * other values are interpreted as 0, no bail out
     */
    uint8_t     PollTypeFBailOut;

    /** Rf-Discovery Loop. Enable Type V Technology */
    uint8_t     PollTypeV;

    /**
     * RF-Discovery Loop. Max. Type V devices to detect.
     * If set to 0 the pre-configured number of devices will be taken. The pre-configured number of devices
     * is defined by the poll mode \see PollMode.
     */
    uint8_t     PollTypeVDeviceLimit;

    /**
     * NFC-V Polling Frequency.
     * 0 =      Do not poll for NFC-B technology.
     * 1 =      Poll for NFC-B technology in every discovery cycle.
     * 2-10 =   Poll for NFC-B technology in every (2-10)th
     *          discovery cycle.
     */
    uint8_t     PollTypeVPollFrequency;

    /** RF-Discovery Loop. Disables auto usage of Poll mode ISO-DEP Protocol. */
    uint8_t     DisablePollIsoDep;

    /** RF-Discovery Loop. Disables auto usage of Poll mode NFC-DEP Protocol. */
    uint8_t     DisableNfcDepProtocol;

    /** RF-Discovery Loop. Set HBR for ISO-DEP. */
    ptx_RUL_PollIsoDepHBR_t     PollIsoDepHbr;

    /** RF-Discovery Loop. Set HBR for NFC-DEP. */
    ptx_RUL_PollNfcDepHBR_t     PollNfcDepHbr;

    /** NFC-DEP ATR Request */
    uint8_t     PollNfcDepAtrReq[PTX_RUL_NFCDEP_ATRREQ_LEN];

    /**
     * NFC-ACM Polling Frequency.
     * 0 =      Do not poll for NFC-ACM technology.
     * 1 =      Poll for NFC-ACM technology in every discovery cycle.
     * 2-10 =   Poll for NFC-ACM technology in every (2-10)th
     *          discovery cycle.
     */
    uint8_t     PollACM;

    /** NFC-ACM Bit Rate. */
    ptx_RUL_PollACMBitRate_t     PollAcmBitRate;

    /** NFC-DEP ATR Request Length. */
    uint8_t     PollNfcDepAtrReqLen;

#if !defined(PTX_DISABLE_LISTEN_MODE)
    /**
     * Activate Listen Mode.
     * 0 ... Deactivated.
     * 1 ... Activated.
     * 3 ... Activated, Rf-Field NTFs enabled.
     */
    uint8_t     ListenMode;

    /**
     * Activate Listen NFC-A.
     * 0 ... Deactivated.
     * 1 ... Activated.
     */
    uint8_t     ListenTypeA;

    /**
     * NFCID Length for Listen NFC-A.
     *  0 ... Use a default NFCID.
     *  4 ... NFCID is 4 bytes long.
     *  7 ... NFCID is 7 bytes long.
     * 10 ... NFCID is 10 bytes long.
     *
     * The variable describes how many bytes in #ListenTypeANfcId describe the NFCID.
     */
    uint8_t     ListenTypeANfcIdLen;

    /**
     * Custom NFCID for Listen NFC-A.
     * The NFCID must be either 4/7/10 bytes long.
     * The length of the bytes describing the NFCID is given in #ListenTypeANfcIdLen.
     */
    uint8_t     ListenTypeANfcId[PTX_RUL_NFCID1_MAX_LEN];

    /**
     * SENS_RES Length.
     * 0 ... No custom SENS_RES is provided.
     * 2 ... A custom SENS_RES is provided in #ListenTypeASensRes.
     */
    uint8_t     ListenTypeASensResLen;

    /** Provide SENS_RES parameters. */
    uint8_t     ListenTypeASensRes[PTX_RUL_SENSRES_LEN];


    /**
     * Activate Listen NFC-B.
     * 0 ... Deactivated.
     * 1 ... Activated.
     * 3 ... Activated, random NFCID0 enabled.
     */
    uint8_t     ListenTypeB;

    /**
     * NFCID Length for Listen NFC-B.
     * 0 ... No NFCID is provided.
     * 4 ... NFCID is provided with a length of 4 bytes.
     */
    uint8_t ListenTypeBNfcIdLen;

    /**
     * NFCID for Listen NFC-B.
     * Must contain 4 bytes!
     */
    uint8_t ListenTypeBNfcId[PTX_RUL_NFCID0_LEN];

    /**
     * AppData Length for Listen NFC-B.
     * 0 ... No AppData is provided.
     * 4 ... AppData is provided with a length of 4 bytes.
     */
    uint8_t ListenTypeBAppDataLen;

    /** AppData for Listen NFC-B. */
    uint8_t ListenTypeBAppData[PTX_RUL_APPDATA_LEN];

    /**
     * Protocol Information Length for Listen NFC-B.
     * 0 ... No Protocol Information is provided.
     * 3 ... Protocol Information is provided with a length of 3 bytes.
     */
    uint8_t ListenTypeBProtInfoLen;

    /** Protocol Information for Listen NFC-B. */
    uint8_t ListenTypeBProtInfo[PTX_RUL_PROTINFO_LEN];


    /**
     * Activate Listen NFC-F, 212 kbps.
     * 0 ... Deactivated.
     * 1 ... Activate.
     */
    uint8_t     ListenTypeF212;

    /**
     * Activate Listen NFC-F, 424 kbps.
     * 0 ... Deactivated.
     * 1 ... Activate.
     */
    uint8_t     ListenTypeF424;

    /**
     * NFCID Length for Listen NFC-F.
     * 0 ... No NfcId is provided.
     * 8 ... NFCID for Listen NFC-F is provided.
     */
    uint8_t ListenTypeFNfcIdLen;

    /** NFCID for Listen NFC-F. */
    uint8_t ListenTypeFNfcId[PTX_RUL_NFCID2_LEN];

    /**
     * Request Data Length for Listen NFC-F.
     * 0 ... No Request Data provided.
     * 2 ... Request Data for Listen NFC-F is provided.
     */
    uint8_t ListenTypeFRequestDataLen;

    /** Request Data for Listen NFC-F. */
    uint8_t ListenTypeFRequestData[PTX_RUL_RD_LEN];

    /**
     * Activate Listen ISO-DEP.
     * 0 ... Deactivated.
     * 1 ... Activated - T4AT.
     * 2 ... Activated - T4BT.
     * 3 ... Activated - both T4AT and T4BT.
     */
    uint8_t     ListenIsoDep;

    /**
     * ATS Length for ISO-DEP.
     *    0 ... No ATS provided.
     * < 21 ... Number of bytes in #ListenIsoDepAts to set
     */
    uint8_t     ListenIsoDepAtsLen;

    /** ATS bytes for ISO-DEP. */
    uint8_t     ListenIsoDepAts[PTX_RUL_ISODEP_ATS_LEN];

    /**
     * ATTRIB RES Length for ISO-DEP
     *    0 ... No ATTRIB_RES provided.
     * < 14 ... Number of bytes in #ListenIsoDepAttribRes
     */
    uint8_t     ListenIsoDepAttribResLen;

    /** ATTRIB RES for ISO-DEP. */
    uint8_t     ListenIsoDepAttribRes[PTX_RUL_ISODEP_ATTRIBRES_LEN];

    /**
     * Activate Listen NFC-DEP.
     * 0 ... Deactivated.
     * 1 ... Activated - NFC-A.
     * 2 ... Activated - NFC-F.
     * 3 ... Activated - both NFC-A and NFC-F.
     */
    uint8_t     ListenNfcDep;

    /** NFC-DEP ATR Response Length. */
    uint8_t     ListenNfcDepAtrResLen;

    /** NFC-DEP ATR Response. */
    uint8_t     ListenNfcDepAtrRes[PTX_RUL_NFCDEP_ATRRES_LEN];
#endif /* PTX_DISABLE_LISTEN_MODE */

} ptx_RUL_DiscoverConfig_t;

/**
 * \brief Available Power-modes.
 */
typedef enum ptx_RUL_PowerMode
{
    PowerMode_Standby,                         /**< Stand-by Mode. */
    PowerMode_Wakeup,                          /**< Wake-up / Full-power Mode. */
} ptx_RUL_PowerMode_t;

#if !defined(PTX_DISABLE_LISTEN_MODE)
/**
 * \brief RUL Routing Target.
 */
typedef enum ptx_RUL_RoutingTarget
{
    ptx_RUL_Target_Host = (0x10),               /**< Messages routed to Host. */
    ptx_RUL_Target_Swp1 = (0x20),               /**< Messages routed to SE/UICC1. */
} ptx_RUL_RoutingTarget_t;

/**
 * \brief RUL HCE Routing Type.
 */
typedef enum ptx_RUL_RoutingType
{
    ptx_RUL_Technology_Based                = (0x01u),      /**< Technology based routing */
    ptx_RUL_Protocol_Based                  = (0x02u),      /**< Protocol based routing */
    ptx_RUL_Aid_Based                       = (0x05u),      /**< AID based routing */
    ptx_RUL_RoutingType_Apdu_Based,                         /**< RFU */
    ptx_RUL_RoutingType_System_Code_Based,                  /**< RFU */
    ptx_RUL_RoutingType_NumOfTypes                          /**< RFU */
} ptx_RUL_RoutingType_t;

/**
 * \brief RUL AID Routing Match Type.
 */
typedef enum ptx_RUL_AidMatch
{
    ptx_RUL_AidMatch_Exact,                     /**< Technology based routing */
    ptx_RUL_AidMatch_SelAid_Shorter,            /**< Protocol based routing */
    ptx_RUL_AidMatch_SelAid_Longer              /**< AID based routing */
} ptx_RUL_AidMatch_t;

/**
 * \brief RUL Protocol Routing Type.
 */
typedef enum ptx_RUL_RfProtocol
{
    ptx_RUL_RfProtocol_Undefined  = (0x00u),    /**< RF Protocol Undefined. */
    ptx_RUL_RfProtocol_T2T        = (0x02u),    /**< RF Protocol T2T. */
    ptx_RUL_RfProtocol_T3T        = (0x03u),    /**< RF Protocol T3T. */
    ptx_RUL_RfProtocol_IsoDep     = (0x04u),    /**< RF Protocol ISO-DEP. */
    ptx_RUL_RfProtocol_T5T        = (0x05u),    /**< RF Protocol T5T. */
    ptx_RUL_RfProtocol_NfcDep     = (0x06u)     /**< RF Protocol NFC-DEP. */
} ptx_RUL_RfProtocol_t;

/**
 * \brief RUL Techhnology Routing Type.
 */
typedef enum ptx_RUL_RfTech
{
    ptx_RUL_RfTech_Passive_Poll_A      = PTX_NSC_TYPES_TECH_POLL_A,        /**< Poll A Technology. */
    ptx_RUL_RfTech_Passive_Poll_B      = PTX_NSC_TYPES_TECH_POLL_B,        /**< Poll B Technology. */
    ptx_RUL_RfTech_Passive_Poll_F      = PTX_NSC_TYPES_TECH_POLL_F,        /**< Poll F Technology. */
    ptx_RUL_RfTech_Passive_Poll_V      = PTX_NSC_TYPES_TECH_POLL_V,        /**< Poll V Technology. */
    ptx_RUL_RfTech_Active_Poll         = PTX_NSC_TYPES_TECH_POLL_ACM,      /**< Poll NFC-ACM Initiator Mode. */
    ptx_RUL_RfTech_Passive_Listen_A    = PTX_NSC_TYPES_TECH_LISTEN_A,      /**< Listen A Technology. */
    ptx_RUL_RfTech_Passive_Listen_B    = PTX_NSC_TYPES_TECH_LISTEN_B,      /**< Listen B Technology. */
    ptx_RUL_RfTech_Passive_Listen_F    = PTX_NSC_TYPES_TECH_LISTEN_F,      /**< Listen F Technology. */
    ptx_RUL_RfTech_Active_Listen       = PTX_NSC_TYPES_TECH_LISTEN_ACM,    /**< Listen NFC-ACM Target mode. */
    ptx_RUL_RfTech_RfTech_Undefined                                        /**< Undefined Rf-Tech. */
} ptx_RUL_RfTech_t;


/**
 * \brief RUL AID Routing Table Entry.
 */
typedef struct ptx_RUL_RoutingAid
{
    uint8_t                     Aid[PTX_NSC_RT_AID_MAX_LEN];                /**< AID */
    uint32_t                    Length;                                     /**< AID Length */
    ptx_RUL_AidMatch_t          MatchType;                                  /**< AID Match Type */
    ptx_RUL_RoutingTarget_t     Target;                                     /**< Routing Target */
} ptx_RUL_RoutingAid_t;

/**
 * \brief RUL Protocol Routing Table Entry.
 */
typedef struct ptx_RUL_RoutingProtocol
{
    ptx_RUL_RfProtocol_t        Protocol;       /**< Protocol */
    ptx_RUL_RoutingTarget_t     Target;         /**< Routing Target */
} ptx_RUL_RoutingProtocol_t;

/**
 * \brief RUL Technology Routing Table Entry.
 */
typedef struct ptx_RUL_RoutingTech
{
    ptx_RUL_RfTech_t            Tech;           /**< Technology */
    ptx_RUL_RoutingTarget_t     Target;         /**< Routing Target */
} ptx_RUL_RoutingTech_t;

/**
 * \brief RUL possible routing table entries.
 */
typedef union ptx_RUL_RoutingEntry
{
    ptx_RUL_RoutingAid_t        AidRouting;         /**< AID Routing */
    ptx_RUL_RoutingProtocol_t   ProtocolRouting;    /**< Protocol Routing */
    ptx_RUL_RoutingTech_t       TechRouting;        /**< Technology Routing */
} ptx_RUL_RoutingEntry_t;

/**
 * \brief RUL Routing List.
 */
typedef struct ptx_RUL_RoutingList
{
    ptx_RUL_RoutingType_t       Type;               /**< Entry Type */
    ptx_RUL_RoutingEntry_t      Entry;              /**< Routing Entry */
} ptx_RUL_RoutingList_t;

#endif

/**
 * \brief RUL Type.
 */
typedef struct ptx_RUL
{
    ptx_Comps_t                     CompId;                         /**< Component ID. */
    ptx_HAL_t                       Hal;                            /**< HAL context. */
    ptx_HIP_t                       Hip;                            /**< HIP context. */
    ptx_NSC_t                       Nsc;                            /**< NSC context. */
    ptx_Log_t                       Log;                            /**< LOG context. */
    ptx_RUL_CardRegistry_t          CardRegistry;                   /**< Card Registry. */

    ptx_RUL_DiscoverConfig_t        DiscoverConfig;                 /**< Discovery Configuration */

    ptx_RUL_EmvCollisionFilter_t    EmvCollisionFilter;             /**< EMV Collision Filter */

    ptx_RUL_StatusInfoSystem_t      SystemStatus;                   /**< System Status. */
    ptx_RUL_StatusInfoDiscover_t    DiscoverStatus;                 /**< Discover Status. */
    ptx_RUL_StatusInfoLpcd_t        LpcdStatus;                     /**< LPCD Status. */
    ptx_RUL_StatusInfoEmvRemoval_t  EmvRemovalStatus;               /**< EMV Removal Status. */
    ptx_RUL_StatusInfoDeactivate_t  DeactivateStatus;               /**< Deactivate Status. */
    ptx_RUL_StatusInfoRfError_t     RfErrorStatus;                  /**< RF Error Status. */
    ptx_RUL_StatusInfoMode_t        NfcMode;                        /**< NFC Mode of operation (poller, listener, etc). */

    ptx_RUL_Extension_Endpoints_t   ExtensionEndpoints[PTX_RUL_MAX_EXTENSIONS];     /**< Pointer to extension endpoints for async. communication. */
    ptx_RUL_Extension_Endpoints_t  *ActiveExtensionEndpoint;                        /**< Pointer to active extension endpoint. */

    uint8_t                         GpioCfg[PTX_RUL_USER_GPIOS];    /**< GPIO Config Array. */

#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX)
    ptx_OSAL_t                      Osal;                           /**< OSAL context used by HAL on WIN/LIN platforms. */   
#endif

} ptx_RUL_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief RUL Component Initialisation.
 *
 * Initialised the component, and sets up all internal pointers, etc.
 *
 * \param[in,out]   rul         Pointer to RUL component.
 * \param[in]       initParams  Pointer to initialisation parameters.
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER     Invalid input parameter.
 * \return Others                                 Operation failed (typically in a sub-component API) with an error value other than the values described above, 
 *                                                for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_Init(ptx_RUL_t *rul, ptx_RUL_InitParams_t *initParams);

/**
 * \brief Initializes the hardware (including FW-download, RF-configuration, etc.).
 *
 * \param[in,out]   rul         Pointer to RUL component.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_InitHardware(ptx_RUL_t* rul);

/**
 * \brief RUL Component Deinitialization.
 *
 * \param[in]   rul     Pointer to RUL component.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_DeInit(ptx_RUL_t *rul);

/**
 * \brief Invoke Hard- or Soft-Reset of NFC chip.
 *
 * Software reset is done by calling low-level HIP protocol RST command.
 * Hardware reset is done by toggling the reset pin of the NFC chip.
 *
 * \note The API shall just invoke the actions required. The state of the NFC chip
 * after the reset action has been executed is not checked within the API.
 * This means, it is assumed that there is a proper connection of MCUs GPIO pin
 * to reset pin of the NFC chip, and that successful toggle of the MCU GPIO pin
 * also successfully toggles NFC chip reset pin.
 *
 * \param[in]   rul         Pointer to RUL component.
 * \param[in]   resetType   Reset type.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
*/
ptx_Status_t ptx_RUL_Reset(ptx_RUL_t* rul, ptx_RUL_ResetTypes_t resetType);

/**
 * \brief Stop/restart Card Discovery.
 *
 * \param[in]   rul     Pointer to RUL component.
 * \param[in]   params  Pointer to deactivation parameters.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_Deactivate(ptx_RUL_t* rul, ptx_RUL_DeactivateParams_t* params);

/**
 * \brief Get System Status Information.
 *
 * \param[in]   rul             Pointer to RUL component.
 * \param[in]   statusParams    Pointer to Status Parameters.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_GetStatusInfo(ptx_RUL_t *rul, ptx_RUL_Status_Params_t *statusParams);

/**
 * \brief Switch BitsExchange on/off.
 *
 * \param[in]   rul         Pointer to RUL component.
 * \param[in]   enable      Enable state to be set.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_BitsExchangeMode(ptx_RUL_t *rul, ptx_RUL_BitsExchangeMode_t enable);

/**
 * \brief Exchange Raw RF Bit Stream.
 *
 * \param[in]       rul         Pointer to RUL component.
 * \param[in]       txParams    Pointer to Tx parameters.
 * \param[in]       rxParams    Pointer to Rx parameters.
 * \param[out]      numBits     Total number of bits processed.
 * \param[in]       msTimeout   Timeout in milliseconds.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_BitsExchange(ptx_RUL_t *rul, ptx_RUL_BitsExchangeParams_t *txParams, ptx_RUL_BitsExchangeParams_t *rxParams, uint32_t *numBits, uint32_t msTimeout);

/**
 * \brief Data Exchange.
 *
 * \param[in]       rul         Pointer to RUL component.
 * \param[in]       tx          Tx data.
 * \param[in]       txLen       Tx data length.
 * \param[out]      rx          Rx data container.
 * \param[in,out]   rxLen       Rx data container length as input, Rx data length as output.
 * \param[in]       msTimeout   Timeout in milliseconds.
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER     Invalid input parameter.
 * \return Others                                 Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_Exchange(ptx_RUL_t *rul, uint8_t *tx, uint32_t txLen, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout);


/**
 * \brief Configure the Specified GPIO Pin.
 *
 * \param[in]   rul         Pointer to RUL component.
 * \param[in]   gpioNbr     GPIO Number to configure. Be Aware that access to every GPIO Pin is not always permitted, if it is used otherwise.
 *                          Only ptx_RUL_GPIONbr_1, ptx_RUL_GPIONbr_4 and ptx_RUL_GPIONbr_6 can keep their configuration during Standby State.
 * \param[in]   gpioDir     GPIO Direction.
 * \param[in]   gpioFlags   Configured Flags.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ConfigGPIO(ptx_RUL_t *rul, ptx_RUL_GPIONbr_t gpioNbr, ptx_RUL_GPIODir_t gpioDir, ptx_RUL_GPIOFlags_t gpioFlags);

/**
 * \brief Write to GPIO Pin Specified.
 *
 * \param[in]   rul         Pointer to RUL component.
 * \param[in]   gpioNbr     GPIO Number to write to. Be Aware that access to every GPIO Pin is not always permitted, if it is used otherwise.
 * \param[in]   value       Value to write.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_NOT_PERMITTED        GPIO Pin is not permitted to be written to.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WriteGPIO(ptx_RUL_t *rul, ptx_RUL_GPIONbr_t gpioNbr, ptx_RUL_GPIOValue_t value);

/**
 * \brief Read to GPIO Pin Specified.
 *
 * \param[in]    rul         Pointer to RUL component.
 * \param[in]    gpioNbr     GPIO Number to read from. Be Aware that access to every GPIO Pin is not always permitted, if it is used otherwise.
 * \param[out]   value       Value container.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return PTX_STATUS_E_GEN_NOT_PERMITTED        GPIO Pin is not permitted to be read from.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ReadGPIO(ptx_RUL_t *rul, ptx_RUL_GPIONbr_t gpioNbr, ptx_RUL_GPIOValue_t *value);

/**
 * \brief Sleep for a specified duration (milliseconds).
 *
 * \param[in]    rul        Pointer to RUL component.
 * \param[in]    ms         Sleep duration in milliseconds.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_Sleep(ptx_RUL_t* rul, uint32_t ms);

/**
 * \brief Send tx Data
 *
 * \param[in]    rul        Pointer to RUL component.
 * \param[in]    tx         Tx data.
 * \param[in]    txLen      Tx data length.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_Send(ptx_RUL_t* rul, uint8_t* tx, uint32_t txLen);

/**
 * \brief Receive rx Data
 *
 * \param[in]       rul         Pointer to RUL component.
 * \param[out]      rx          Rx data container.
 * \param[in,out]   rxLen       Rx data container length as input, Rx data length as output.
 * \param[in]       msTimeout   Timeout in milliseconds.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_Receive(ptx_RUL_t* rul, uint8_t* rx, uint32_t* rxLen, uint32_t msTimeout);

/**
 * \brief Get various information about the system (Stack, DFY-Code/-Toolchain, Chip-ID, Product-ID etc.).
 *
 * \param[in]       rul         Pointer to RUL component.
 * \param[in]       infoType    Type of revision Information requested
 * \param[out]      infoBuf     The Information buffer for the Requested Type
 * \param[in,out]   infoBufLen  The length of the Information buffer
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_GetInfo(ptx_RUL_t* rul, ptx_RUL_InfoTypes_t infoType, uint8_t* infoBuf, uint8_t* infoBufLen);

/**
 * \brief T3T SensF-REQ command.
 *
 * \param[in] rul           Pointer to RUL component.
 * \param[in] systemCode    System code.
 * \param[in] requestCode   Request code.
 * \param[in] tsn           Time slot number.
 * \param[out] rx           Rx buffer.
 * \param[in, out]          rxLength Length of Rx buffer and length of Rx-data.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_T3T_SensFReq(ptx_RUL_t* rul, uint16_t systemCode, uint8_t requestCode, uint8_t tsn, uint8_t* rx, uint32_t* rxLength);

/**
 * \brief T5T isolated End-of-Frame command.
 *
 * \param[in] rul           Pointer to RUL component.
 * \param[out] rx           Rx buffer.
 * \param[in, out]          rxLength Length of Rx buffer and length of Rx-data.
 * \param[in] msTimeout     Timeout in ms.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_T5T_Isolated_EoF (ptx_RUL_t* rul, uint8_t* rx, uint32_t* rxLength, uint32_t msTimeout);


/**
 * \brief Update the RF- and System-Configuration parameters of the NFC hardware
 *
 * This function allows the change of RF- and System-Configuration parameters at runtime
 *
 * \param[in]   rul             Pointer to RUL component.
 * \param[in]   nrConfigs       Number of hardware configurations.
 * \param[in]   configs         Pointer to hardware configuration structures.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_UpdateHardwareConfiguration(ptx_RUL_t* rul, uint32_t nrConfigs, ptx_RUL_HWConfig_t* configs);


/**
 * \brief Initiate Card Discovery for Card types Given in the Discover Configuration.
 *
 * \param[in]   rul     Pointer to RUL component.
 * \param[in]   cfg     Pointer to discover configuration. This parameter can be NULL. In such a case
 *                      previously set Discovery Configuration parameters will be used.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_InitiateDiscovery(ptx_RUL_t *rul, ptx_RUL_DiscoverConfig_t *cfg);


/**
 * \brief Add a given card to the internal card registry structure.
 *
 * \param[in]   rul             Pointer to RUL component.
 * \param[in]   discoveredCard  Pointer to card that will be added to the card registry.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES  Insufficient resources or lack of memory. See \ref ptx_Status_t.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_AddToCardRegistry(ptx_RUL_t* rul, ptx_RUL_CardParams_t* discoveredCard);

/**
 * \brief Clears the registry of discovered cards / RF-Devices.
 *
 * \param[in]   rul     Pointer to RUL component.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_ClearCardRegistry(ptx_RUL_t* rul);

/**
 * \brief Determines the supported protocol of a specified card.
 *
 * \param[in]   rul     Pointer to RUL component.
 * \param[in]   card    Pointer to card parameters.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_DetermineProtocol(ptx_RUL_t* rul, ptx_RUL_CardParams_t* card);

/**
 * \brief Retrieves the card registry data structure
 *
 * \param[in]       rul         Pointer to RUL component.
 * \param[in,out]   registry    Pointer pointer to get access to the card registry.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_GetCardRegistry(ptx_RUL_t* rul, ptx_RUL_CardRegistry_t** registry);

/**
 * \brief Card Presence Check.
 *
 * \param[in]   rul         Pointer to RUL component.
 * \param[in]   type        Type of presence check mechanism on ISO-DEP protocol. If higher protocol is not ISO-DEP this parameter is ignored.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_CheckCardPresence(ptx_RUL_t *rul, ptx_RUL_CheckPresType_t type);

/**
 * \brief Select a card.
 *
 * \param[in]   rul             Pointer to RUL component.
 * \param[in]   cardRegistryId  Card registry ID to identify the card.
 * \param[in]   protocol        Specific Card Protocol.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SelectCard(ptx_RUL_t* rul, uint32_t cardRegistryId, ptx_RUL_CardProtocol_t protocol);

/**
 * \brief Get activation parameters of an activated target.
 *
 * \note The function shall be called once a card has been activated.
 *
 * \param[in]   rul             Pointer to RUL component.
 * \param[out]  cardParams      Card/activation parameters of the activated target.
 *
 * \return PTX_STATUS_SUCCESS                       Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER       Invalid input parameter.
 * \return PTX_STATUS_E_GEN_INVALID_STATE           No active target.
 * \return Others                                   Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                                  for further information see \ref ptx_Status_t.
*/
ptx_Status_t ptx_RUL_GetCardParameters(ptx_RUL_t* rul, ptx_RUL_CardParams_t** cardParams);

/**
 * \brief Adds an add-on extension to RUL
 *
 * This function registers an add-on library and its corresponding end points with RUL.
 *
 * \note The caller has to make sure, that the add-on is properly de-initialized!
 *
 * \param[in]   rul             Pointer to RUL component.
 * \param[in]   endpoint        Pointer to the endpoint data structure
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return PTX_STATUS_E_GEN_INVALID_STATE        Unknown state in which the extension should be registered.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_RegisterExtensionEndpoints(ptx_RUL_t *rul, ptx_RUL_Extension_Endpoints_t* endpoint);

/**
 * \brief Removes an add-on extension from RUL
 *
 * This function removes a registered add-on library and its corresponding end points from RUL.
 *
 * \note The caller has to make sure, that the add-on is properly de-initalized!
 *
 * \param[in]   rul             Pointer to RUL component.
 * \param[in]   compId          Component Identifier for the extension.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 */
ptx_Status_t ptx_RUL_RemoveExtensionEndpoints(ptx_RUL_t *rul, ptx_Comps_t compId);

/**
 * \brief Sets the PTX2xxR/W into the desired power mode state
 *
 * \param[in]   rul             Pointer to RUL component.
 * \param[in]   powerMode       Desired power mode
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SetPowerMode(ptx_RUL_t *rul, ptx_RUL_PowerMode_t powerMode);

/**
 * \brief Write to Digital-to-Analog-Converter Register.
 *
 * \param[in]   rul         Pointer to RUL component.
 * \param[in]   value       Value to be converted.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WriteDAC(ptx_RUL_t *rul, uint8_t value);

/**
 * \brief RUL RSSI Measurement at 13.56MHz using the CW power level of 0x58 and an average of 16 samples.
 * \deprecated Use \ref ptx_RUL_RSSIMeasurement instead
 *
 * \param[in]       rul                 Pointer to RUL component.
 * \param[out]      result              Pointer to store the measurement result.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_GetRSSIValue(ptx_RUL_t *rul, uint32_t *result);

/**
 * \brief RUL RSSI Measurement at 13.56MHz using the given CW power level.
 *
 * \param[in]       rul                 Pointer to RUL component.
 * \param[in]       cwPowerLevel        CW multiplier value used for the measurement.
 * \param[in]       averaging           The amount of averaged measurements.
 * \param[out]      result              Pointer to store the measurement result.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_RSSIMeasurement(ptx_RUL_t *rul, uint8_t cwPowerLevel, ptx_RUL_AdcAveraging_t averaging, uint32_t *result);

/**
 * \brief RUL RF Field Amplitude Measurement at the provided frequency.
 * *
 * \param[in]       rul             Pointer to RUL component.
 * \param[in]       targetFrequency Target frequency for the amplitude measurement.
 * \param[out]      averaging       Number of measurement to average.
 * \param[out]      rfAmp           Pointer to the measurement result.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_ADCCLIPPING          ADC clipping detected.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_RfFieldMeasurement(ptx_RUL_t *rul, uint32_t targetFrequency, ptx_RUL_AdcAveraging_t averaging, uint32_t *rfAmp);

/**
 * \brief Performs calibration routine to measure the Tx ESD prevention threshold through RSSI measurements. 
 * This calibration needs to be done during design-in phase to determine the threshold at which the output 
 * power must be limited in order to prevent ESD clamping during operation.  
 *
 * \note The results are applicable to all devices using the same antenna system configuration and it is therefore
 * not required to perform per-device calibration.
 *
 * \param[in]       rul             Pointer to RUL component.
 * \param[out]      results         Pointer to an array, which stores all measurements of the calibration routine.
 * \param[in]       resultsLength   Length of the array to store the calibration measurements (also used to define 
 *                                  the maximum number of TX Multiplier values (RF Amplitudes) used for the calibration Routine).
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return PTX_STATUS_E_GEN_OUT_OF_RANGE         A value/measurement exceeds the valid range.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_CalibrateTxEsdPreventionThreshold(ptx_RUL_t *rul, uint8_t *results, uint8_t resultsLength);

#if !defined(PTX_DISABLE_LISTEN_MODE)
/**
 * \brief RUL Set a Routing Table List.
 * *
 * \param[in]       rul             Pointer to RUL component.
 * \param[in]       length          Number of routing table in the list.
 * \param[in]       rt_list         Pointer to the Routing Table List to add.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_NOT_IMPLEMENTED      Routing functionality for parameter in rt_list not implemented.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                Operation failed (typically in a sub-component API) with an error value other than the values described above.
 *                                               for further information see \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_RoutingTableConfig(ptx_RUL_t *rul, uint32_t length, ptx_RUL_RoutingList_t *rt_list);

#endif

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

