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
    Module      : NFC Forum Device Test Application (DTA) Configuration
    File        : ptx_NFCForumDTA_Configuration.h

    Description : Contains the configuration and other types and definitions of the NFC Forum DTA to manage the flow of the DTA.
*/

/**
 * \addtogroup DTA
 * @{
 */

#if !defined(PTX_DTACONFIGURATION_H)
#define PTX_DTACONFIGURATION_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Status.h"

#include "ptx_RUL.h"

#include "ptx_NDEF.h"

#if !defined(PTX_DISABLE_LISTEN_MODE)
    #include "ptx_RUL_HCE.h"
#endif // PTX_DISABLE_LISTEN_MODE

#include "ptx_RUL_LLCP.h"

#include "ptx_RUL_SnepServer.h"

#include "ptx_RUL_SnepClient.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name NFC Forum DTA specific defines.
 * @{
 */
#define CON_SYS_CODE_SIZE                               (3u)            /**< Type-F max. System Code size. */
#define CON_SYS_CODE_0                                  (0xFFFF)        /**< Type-F System Code pattern 1. */
#define CON_SYS_CODE_1                                  (0x12FC)        /**< Type-F System Code pattern 2. */
#define CON_SYS_CODE_2                                  (0x8EFC)        /**< Type-F System Code pattern 3. */

#define PTX_NFCFORUMDTA_NDEF_MESSAGE_BUFFER_SIZE        (7000u)         /**< Max. Buffer size for NDEF message buffer. */
#define PTX_NFCFORUMDTA_NDEF_TRANSMISSION_BUFFER_SIZE   (2048u)          /**< Max. single NDEF-transmission buffer size. */

#define PTX_NFCFORUMDTA_TYPE_F_BR212                    (212u)          /**< Type-F 212 kBit/s bitrate. */
#define PTX_NFCFORUMDTA_TYPE_F_BR424                    (424u)          /**< Type-F 424 kBit/s bitrate. */

#define PTX_NFCFORUMDTA_SNEPSERVER_PATTERN                  (0x1300)    /**< DTA Test pattern number for the SNEP Server. */
#define PTX_NFCFORUMDTA_SNEPCLIENT_PATTERN_PUT_REQ_NORMAL   (0x1301)    /**< DTA Test pattern number for the SNEP Client PUT request scenario (normal record). */
#define PTX_NFCFORUMDTA_SNEPCLIENT_PATTERN_PUT_REQ_LONG     (0x1302)    /**< DTA Test pattern number for the SNEP Client PUT request scenario (long record). */
#define PTX_NFCFORUMDTA_SNEPCLIENT_PATTERN_GET_REQ          (0x1303)    /**< DTA Test pattern number for the SNEP Client GET request scenario. */
#define PTX_NFCFORUMDTA_SNEPSERVER_DTA_MODE_MASK            (0x40)      /**< Test pattern boundary to differentiate between LLCP & SNEP scenarios. */
#define PTX_NFCFORUMDTA_SNEP_PATTERNNUMBER_BOUNDARY         (0x12FF)    /**< Test pattern boundary to differentiate between LLCP & SNEP scenarios. */
#define PTX_NFCFORUMDTA_SNEP_PATTERNNUMBER_MASK             (0xFF0F)    /**< Bitmask to extract the pattern number independent of the NFC bitrate. */

#define PTX_NFCFORUMDTA_LLCP_DEFAULT_MIUX                   (0x7A)                          /**< Default MIUX for LLCP DTA scenarios. */
#define PTX_NFCFORUMDTA_LLCP_INBOUND_CL_ECHO_SERVICE_NAME   ("urn:nfc:sn:dta-cl-echo-in")   /**< LLCP DTA inbound connection less echo service name. */
#define PTX_NFCFORUMDTA_LLCP_INBOUND_CL_SAP_ADDRESS         (0x11)                          /**< LLCP DTA inbound connection less SAP address. */
#define PTX_NFCFORUMDTA_LLCP_OUTBOUND_CL_ECHO_SERVICE_NAME  ("urn:nfc:sn:dta-cl-echo-out")  /**< LLCP DTA outbound connection less echo service name. */
#define PTX_NFCFORUMDTA_LLCP_OUTBOUND_CL_SAP_ADDRESS        (0x13)                          /**< LLCP DTA outbound connection less SAP address. */
#define PTX_NFCFORUMDTA_LLCP_INBOUND_CO_ECHO_SERVICE_NAME   ("urn:nfc:sn:dta-co-echo-in")   /**< LLCP DTA inbound connection oriented echo service name. */
#define PTX_NFCFORUMDTA_LLCP_INBOUND_CO_SAP_ADDRESS         (0x12)                          /**< LLCP DTA inbound connection oriented SAP address. */
#define PTX_NFCFORUMDTA_LLCP_OUTBOUND_CO_ECHO_SERVICE_NAME  ("urn:nfc:sn:dta-co-echo-out")  /**< LLCP DTA outbound connection oriented echo service name. */
#define PTX_NFCFORUMDTA_LLCP_OUTBOUND_CO_SAP_ADDRESS        (0x14)                          /**< LLCP DTA outbound connection oriented SAP address. */
#define PTX_NFCFORUMDTA_LLCP_T_DELAY_CO                     (3100)                          /**< LLCP DTA symmetry procedure delay time. */
#define PTX_NFCFORUMDTA_LLCP_INITIAL_CONNECT_TO             (3000)                          /**< LLCP DTA initial connection timeout. */
#define PTX_NFCFORUMDTA_LLCP_POST_PROCESSING_TO             (10000)                         /**< LLCP DTA post processing timeout. */

#define ptx_NFCForumDTA_TECH_MODE_POLL_NFC_A            (1u)            /**< Type-A Poll technology/mode mask. */
#define ptx_NFCForumDTA_TECH_MODE_POLL_NFC_B            (2u)            /**< Type-B Poll technology/mode mask. */
#define ptx_NFCForumDTA_TECH_MODE_POLL_NFC_F            (4u)            /**< Type-F Poll technology/mode mask. */
#define ptx_NFCForumDTA_TECH_MODE_POLL_NFC_V            (8u)            /**< Type-V Poll technology/mode mask. */
#define ptx_NFCForumDTA_TECH_MODE_POLL_NFC_ACM          (16u)           /**< Type-ACM Initiator technology/mode mask. */

#define ptx_NFCForumDTA_TECH_MODE_LISTEN_NFC_A          (0x0020)        /**< Type-A Listen technology/mode mask. */
#define ptx_NFCForumDTA_TECH_MODE_LISTEN_NFC_B          (0x0040)        /**< Type-B Listen technology/mode mask. */
#define ptx_NFCForumDTA_TECH_MODE_LISTEN_NFC_F          (0x0080)        /**< Type-F Listen technology/mode mask. */
#define ptx_NFCForumDTA_TECH_MODE_LISTEN_NFC_V          (0x0100)        /**< Type-V Listen technology/mode mask. */
#define ptx_NFCForumDTA_TECH_MODE_LISTEN_NFC_ACM        (0x0200)        /**< Type-ACM Target technology/mode mask. */

#define ptx_NFCForumDTA_LISTEN_NFC_F_RD                 {0x8E, 0xFC}    /**< Type-F SENS_RES RD bytes. */
#define ptx_NFCForumDTA_LISTEN_NFC_F_RD_LEN             (2)             /**< Type-F SENS_RES RD length. */

/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief NFC Forum DTA Configuration Parameters.
*/
typedef struct ptx_NFCForumDTA_Config
{

    /** Listen Mode Configuration Parameters */

    uint8_t CON_LISTEN_ACM;         /**< Listen to Active Communication Mode */
    uint8_t CON_LISTEN_DEP_A;       /**< Listen for NFC-A Technology with NFC-DEP support */
    uint8_t CON_LISTEN_DEP_F;       /**< Listen for NFC-F Technology with NFC-DEP support */
    uint8_t CON_LISTEN_T3TP;        /**< Listen for NFC-F Technology with Type 3 Tag Platform support */
    uint8_t CON_LISTEN_T4ATP;       /**< Listen for NFC-A Technology with Type 4 Tag Platform support */
    uint8_t CON_LISTEN_T4BTP;       /**< Listen for NFC-B Technology with Type 4 Tag Platform support */

    uint8_t *CON_SYS_CODE;          /**< SYS_CODE */
    uint8_t *CON_SENSF_RES;         /**< SENSF_RES */

    /** Technology Detection Activity Configuration Parameters */
    uint8_t CON_POLL_ACM;           /**< Poll for NFC-ACM */
    uint8_t CON_POLL_A;             /**< Poll for NFC-A */
    uint8_t CON_EXT_SENSB_RES;      /**< Supports extended SENSB_RES */
    uint8_t CON_POLL_B;             /**< Poll for NFC-B */
    uint8_t CON_POLL_F;             /**< Poll for NFC-F */
    uint8_t CON_POLL_V;             /**< Poll for NFC-V */
    uint8_t CON_POLL_P;             /**< Poll for NFC-P */

    uint8_t CON_BAIL_OUT_A;         /**< Bail-out after NFC-A */
    uint8_t CON_BAIL_OUT_B;         /**< Bail-out after NFC-B */
    uint8_t CON_BAIL_OUT_F;         /**< Bail-out after NFC-F */

    uint8_t CON_BITR;               /**< Bit Rate for NFC-F. 2:212 kbps, 3:424 kbps */
    uint8_t CON_BITR_ACM;           /**< Bit Rate for NFC-ACM. 1:106 kbps, 2:212 kbps, 3:424 kbps */

    uint8_t T4AT_NFC_DEP_PRIORITY;  /**< 0:NFC_DEP Priority, 1:T4AT Priority */
    uint8_t T3T_NFC_DEP_PRIORITY;   /**< 0:NFC_DEP Priority, 1:T3T Priority */


    /** Collision Resolution Activity Configuration Parameters */
    uint8_t CON_DEVICES_LIMIT_A;    /**< Device Limit for NFC-A polling */
    uint8_t CON_DEVICES_LIMIT_B;    /**< Device Limit for NFC-B polling */
    uint8_t CON_DEVICES_LIMIT_F;    /**< Device Limit for NFC-F polling */
    uint8_t CON_DEVICES_LIMIT_V;    /**< Device Limit for NFC-V polling */

    uint8_t CON_ANTICOLL;           /**< Use Anti-collision */

    /**  Device Activation Activity Configuration Parameter */
    uint8_t *CON_ATR;               /**< ATR_REQ Command parameter */
    uint8_t CON_GB;                 /**< General bytes of the ATR_REQ or Higher Layer INF of ATTRIB */
    uint8_t CON_RATS;               /**< RATS Command Parameters */
    uint8_t *CON_ATTRIB;            /**< ATTRIB Command Parameters */
    uint8_t CON_BITR_NFC_DEP;       /**< Bit Rate for NFC-DEP. 1:106 kbps, 2:212 kbps, 3:424 kbps */

    uint8_t REACTIVATION;           /**< Reactivation requested */

} ptx_NFCForumDTA_Config_t;

/**
 * \brief NFC Forum DTA Device Type
 * Allows for selection between Reader and Universal device
 */
typedef enum ptx_NFCForumDTA_DevType
{
    ptx_NFCForumDTA_DevType_Reader                  = 0,
    ptx_NFCForumDTA_DevType_Universal               = 1,
} ptx_NFCForumDTA_DevType_t;

/**
 * \brief NFC Forum DTA Test Profile Configuration Parameters
*/
typedef struct ptx_NFCForumDTA_Test_Config
{
    uint16_t    PATTERN_NUMBER;             /**< Pattern Number */
    uint32_t    DiscoveryTimeout;           /**< Discovery Timeout */
    uint8_t     EnableDDPC;                 /**< Enable or Disable DDPC (Digital Dynamic Power Control) */
    ptx_NFCForumDTA_DevType_t DeviceType;   /**< NFC Forum Device Type */
} ptx_NFCForumDTA_Test_Config_t;

/**
 * \brief DTA States
*/
typedef enum ptx_NFCForumDTA_State
{
    ptx_NFCForumDTA_State_INIT                        = 0,
    ptx_NFCForumDTA_State_START_RF_DISCOVERY          = 1,
    ptx_NFCForumDTA_State_WAIT_4_ACTIVATION           = 2,
    ptx_NFCForumDTA_State_POLL_SELECT_DEVICE          = 3,
    ptx_NFCForumDTA_State_POLL_DATA_EXCHANGE          = 4,
    ptx_NFCForumDTA_State_LISTEN_PROCESS_EVENTS       = 5,
    ptx_NFCForumDTA_State_PROCESS_LLCP                = 6,
    ptx_NFCForumDTA_State_DEACTIVATE_DISCOVER         = 7,
    ptx_NFCForumDTA_State_DEACTIVATE_IDLE             = 8,
    ptx_NFCForumDTA_State_DEACTIVATE_SLEEP            = 9,
    ptx_NFCForumDTA_State_DEACTIVATE_DESLECT          = 10,
    ptx_NFCForumDTA_State_DEACTIVATE_RELEASE          = 11,
    ptx_NFCForumDTA_State_DEACTIVATE_LISTEN_SLEEP     = 12,
    ptx_NFCForumDTA_State_DEACTIVATE_WAIT_USER        = 13,
    ptx_NFCForumDTA_State_RESTART_RF_DISCOVERY        = 14,
    ptx_NFCForumDTA_State_WAIT_4_USER_INPUT           = 15,
    ptx_NFCForumDTA_State_RESTART_SYSTEM              = 16,
    ptx_NFCForumDTA_State_FINISHED                    = 17,

    ptx_NFCForumDTA_State_MAX,
} ptx_NFCForumDTA_State_t;

/**
 * \brief Callback function that can be used to interrupt the dta execution
 *
 * \param[in]   ctx     Pointer to the context
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER     Invalid input parameter.
*/
typedef void(*ptx_NFCForumDTA_ApplicationCallback_t)(void* ctx);

/**
 * \brief DTA Initialization Parameters
*/
typedef struct ptx_NFCForumDTA_InitParams
{
    ptx_RUL_t                               *rul;                   /**< Pointer to an initialized RUL component. */
    ptx_NFCForumDTA_ApplicationCallback_t   applicationCallback;    /**< Callback-function for higher-level application. */
    uint8_t                                 *NDEFMessageBuffer;     /**< Pointer to an initialized Message Buffer. */
    uint32_t                                NDEFMessageBufferSize;  /**< Size of the NDEF Message Bugger */
    uint32_t                                *NDEFMessageBufferLen;  /**< Size of the currently stored NDEF Message in the buffer */
    uint8_t                                 *HCEMessageBuffer;      /**< Pointer to an initialized Message Buffer for HCE */
    uint32_t                                HCEMessageBufferSize;   /**< Size of the currently stored HCE Message in the buffer */
} ptx_NFCForumDTA_InitParams_t;

/**
 * \brief DTA Type.
 */
typedef struct ptx_NFCForumDTA
{
    ptx_Comps_t                 CompId;                         /**< Component ID. */

    ptx_RUL_t                   *rul;                           /**< RUL Component */
    ptx_NDEF_t                  ndef;                           /**< NDEF Component */
#if !defined(PTX_DISABLE_LISTEN_MODE)
    ptx_RUL_HCE_t               hce;                            /**< HCE Component */
#endif // PTX_DISABLE_LISTEN_MODE
    ptx_RUL_LLCP_t              llcp;                           /**< LLCP Component */
    ptx_RUL_SnepServer_t        snepServer;                     /**< SNEP Server Component */
    ptx_RUL_SnepClient_t        snepClient;                     /**< SNEP Client Component */

    ptx_NFCForumDTA_Config_t    DTAConfiguration;               /**< DTA Configuration */
    ptx_NFCForumDTA_State_t     State;                          /**< Current DTA State. */
    uint8_t                     quitDTA;                        /**< Parameter that decides how long the dta is executed, can be modified by the ApplicationCallback to quit the DTA preemptively */
    uint8_t                     quitEndlessExecution;           /**< Can be used to exit endless Execution of the DTA. Shall be modified by ApplicationCallback. */
    ptx_RUL_CardRegistry_t      *cardRegistry;                  /**< Card Registry */
    ptx_NFCForumDTA_ApplicationCallback_t  applicationCallback; /**< Additional Callback to get an interrupt signal to stop the DTA in case of endless loopback*/

    uint8_t                     *NDEFMessageBuffer;             /**< Message Buffer to save NDEF Messages between Tests */
    uint32_t                    NDEFMessageBufferSize;          /**< NDEF Message Buffer Size */
    uint32_t                    *NDEFMessageBufferLen;          /**< NDEF Message Buffer Length */
} ptx_NFCForumDTA_t;

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

