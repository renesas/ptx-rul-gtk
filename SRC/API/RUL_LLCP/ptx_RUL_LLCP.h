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
    Module      : LLCP
    File        : ptx_RUL_LLCP.h

    Description : Implementation of RUL LLCP
*/

/**
 * \addtogroup RUL_LLCP
 * @{
 */

#if !defined(PTX_RULLLCP_H)
#define PTX_RULLLCP_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>

#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_RUL.h"

#include "ptx_RUL_LLCP_Frame.h"
#include "ptx_RUL_LLCP_Pdu.h"
#include "ptx_RUL_LLCP_TLV.h"
#include "ptx_RUL_LLCP_Link.h"
#include "ptx_HAL.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/**
 * \name LLCP specific default definitions.
 * @{
 */
#define PTX_LLCP_DEFAULT_MIU                            (128u)      /**< Default MIU. */
#define PTX_LLCP_DEFAULT_RW                             (1u)        /**< Default Receive Window (RW) size. */
#define PTX_LLCP_MAX_LINKS                              (5u)        /**< Maximum number of simultaneous LLCP logical links. */
#define PTX_LLCP_DEFAULT_GEN_BYTES_TLVS                 (3u)        /**< Maximum number of TLVs that fiting into general bytes. */
#define PTX_LLCP_GENERAL_BYTES_MAX_LEN                  (15u)       /**< Max length of the LLCP general bytes. */
#define PTX_LLCP_MAX_AGGREGATED_PDUS                    (8u)        /**< Maximum number of PDUs allowed withing an AGF. */
#define PTX_LLCP_WELLKNOWN_SIZE                         (16u)       /**< LLCP Well-Known Threshold (Link addresses below 16 are well known). */
#define PTX_LLCP_MAX_TLV_PARAMS                         (5u)        /**< Maximum number of TLV attached to one PDU */
#define PTX_LLCP_SAP_ADDRESS_LINK_MANAGEMENT            (0x00)      /**< SAP address of the Link Management Service. */
#define PTX_LLCP_SAP_ADDRESS_SERVICE_DISCOVERY          (0x01)      /**< SAP address of the Service Discovery Service. */
#define PTX_LLCP_SAP_ADDRESS_SNEP                       (0x04)      /**< SAP address of the SNEP SAP. */
#define PTX_LLCP_SAP_ADDRESS_MAX                        (64)        /**< Maximum number of SAPs. */
#define PTX_LLCP_DEFAULT_GEN_BYTES_MIUX                 (122u)      /**< Default MIUX value (128 + 122 = 250 Byte per PDU) */
#define PTX_LLCP_DEFAULT_GEN_BYTES_LTO                  (100u)      /**< Default Link Timeout (100ms) */
#define PTX_LLCP_DEFAULT_TLV_LEN_GEN_BYTES              (3u)        /**< Maximum number of TLVs that fit in addition to the GEN_BYTES into ATRRES */
#define PTX_LLCP_DEFAULT_MAGIC_BYTE_0_VALUE             (0x46)      /**< LLCP Magic byte 0 value */
#define PTX_LLCP_DEFAULT_MAGIC_BYTE_1_VALUE             (0x66)      /**< LLCP Magic byte 1 value */
#define PTX_LLCP_DEFAULT_MAGIC_BYTE_2_VALUE             (0x6D)      /**< LLCP Magic byte 2 value */
#define PTX_LLCP_DEFAULT_MAGIC_BYTE_0_INDEX             (0)         /**< ATRRES Magic Bytes index of MB0 */
#define PTX_LLCP_DEFAULT_MAGIC_BYTE_1_INDEX             (1)         /**< ATRRES Magic Bytes index of MB1 */
#define PTX_LLCP_DEFAULT_MAGIC_BYTE_2_INDEX             (2)         /**< ATRRES Magic Bytes index of MB2 */
#define PTX_LLCP_DEFAULT_ATRRES_GEN_BYTES_OFFSET        (4u)        /**< ATRRES General Bytes Offset */
#define PTX_LLCP_DEFAULT_ATRRES_WT_INDEX                (2)         /**< ATRRES WT index */
#define PTX_LLCP_DEFAULT_ATRRES_WT_VALUE                (0x0A)      /**< ATRRES default WT value */
#define PTX_LLCP_DEFAULT_ATRRES_PP_INDEX                (3)         /**< ATRRES PP index */
#define PTX_LLCP_DEFAULT_ATRRES_PP_VALUE                (0x32)      /**< ATRRES default PP value */
#define PTX_LLCP_DEFAULT_GEN_BYTES_MAJOR_VERSION        (1u)        /**< LLCP default major version number */
#define PTX_LLCP_DEFAULT_GEN_BYTES_MINOR_VERSION        (2u)        /**< LLCP default minor version number */
#define PTX_LLCP_DEFAULT_TIMER_TICK_UNIT                (10u)       /**< Default timer tick value (10ms) */
/** @} */


/**
 * \name LLCP specific offset definitions.
 * @{
 */
#define PTX_LLCP_ATRRES_MAGIC_BYTE_0_OFFSET             (18U)       /**< LLCP magic byte 0 offset within ATR_RES. */
#define PTX_LLCP_ATRRES_MAGIC_BYTE_1_OFFSET             (19U)       /**< LLCP magic byte 1 offset within ATR_RES. */
#define PTX_LLCP_ATRRES_MAGIC_BYTE_2_OFFSET             (20U)       /**< LLCP magic byte 2 offset within ATR_RES. */
#define PTX_LLCP_ATRREQ_MAGIC_BYTE_0_OFFSET             (17U)       /**< LLCP magic byte 0 offset within ATR_REQ. */
#define PTX_LLCP_ATRREQ_MAGIC_BYTE_1_OFFSET             (18U)       /**< LLCP magic byte 1 offset within ATR_REQ. */
#define PTX_LLCP_ATRREQ_MAGIC_BYTE_2_OFFSET             (19U)       /**< LLCP magic byte 2 offset within ATR_REQ. */
/** @} */

/**
 * \name LLCP NFC Forum DTA specific definitions.
 * @{
 */
#define PTX_DTA_LLCP_SVC_DTA_PATTERN_NUMBER_NAME        ("urn:nfc:sn:dta-pattern-number")   /**< NFC Forum DTA Pattern Number URN. */
#define PTX_DTA_LLCP_SVC_DISCOVERY_PROT_SERVICE_NAME    ("urn:nfc:sn:sdp")                  /**< NFC Forum DTA SDP URN. */
#define PTX_DTA_LLCP_SVC_DISCOVERY_PROT_SAP             (0x01)                              /**< NFC Forum DTA Service Discovery SAP address. */
/** @} */



/**
 * \brief LLCP Role
 */
typedef enum ptx_RUL_LLCP_Role
{
    Role_Initiator,                                     /**< LLCP Initiator */
    Role_Target,                                        /**< LLCP Target */
    Role_Max                                            /**< Max. value. Not used */
} ptx_RUL_LLCP_Role_t;

/**
 * \brief LLCP component initialisation parameters
 */
typedef struct ptx_RUL_LLCP_InitParameters
{
    ptx_RUL_t          *Rul;                            /**< RUL Core Component. */
    uint8_t            *GeneralBytes;                   /**< General Bytes of ATR_REQ/ATR_RES. */
    uint8_t             GeneralBytesLength;             /**< Length of the General Bytes. */
} ptx_RUL_LLCP_InitParameters_t;

/**
 * \brief LLCP Data Protection Class
 */
typedef enum ptx_RUL_LLCP_Dpc
{
    DPC_Plain,                                          /**< Plain data transfer */
    DPC_Secure,                                         /**< Secured data transfer */
    DPC_Max                                             /**< Max. value. Not used */
} ptx_RUL_LLCP_Dpc_t;

/**
 * \brief LLCP Link Service Class
 */
typedef enum ptx_RUL_LLCP_Lsc
{
    LSC_Connectionless,                                 /**< Connectionless link service only */
    LSC_ConnectionOriented,                             /**< Connection-oriented link service only  */
    LSC_ConnectionlessAndOriented,                      /**< Both connectionless and connection-oriented link service */
    LSC_Max                                             /**< Max. value. Not used */
} ptx_RUL_LLCP_Lsc_t;

/**
 * \brief Service Name struct
 */
typedef struct ptx_RUL_LLCP_Parameter_ServiceName
{
    char    *Name;                                      /**< Name of the Service */
    uint16_t Length;                                    /**< Length of the Servive Name */
} ptx_RUL_LLCP_Parameter_ServiceName_t;

/**
 * \brief Option parameter struct
 */
typedef struct ptx_RUL_LLCP_Parameter_Option
{
    ptx_RUL_LLCP_Lsc_t Lsc;                                 /**< Link Service Class (Connection-less, Connection-oriented or both) */
    ptx_RUL_LLCP_Dpc_t Dpc;                                 /**< Data Protection Class (Plain or Secure) */
} ptx_RUL_LLCP_Parameter_Option_t;

/**
 * \brief LLCP parameter union
 */
typedef union ptx_RUL_LLCP_ParameterValue
{
    uint16_t                                Miux;           /**< Maximum Information Unit Extension (11 Bit Value) */
    uint8_t                                 Lto;            /**< Link Timeout in multiples of 10 milliseconds (8 Bit Value) */
    uint16_t                                Wks;            /**< Well-Known Services (16 Bit Value) */
    uint8_t                                 Rw;             /**< Receive Window Size (4 Bit Value) */
    ptx_RUL_LLCP_Parameter_ServiceName_t    ServiceName;    /**< Service Name URI Struct */
    ptx_RUL_LLCP_Parameter_Option_t         Option;         /**< Option Parameter Struct */
} ptx_RUL_LLCP_ParameterValue_t;

/**
 * \brief LLCP Connection State
 */
typedef enum ptx_RUL_LLCP_ConnectionState
{
    MAC_Layer_Activated,                    /**< MAC Layer Activation */
    MAC_Layer_Deactivation_Pending,         /**< MAC Layer Deactivation Pending */
    MAC_Layer_Deactivated,                  /**< MAC Layer Deactivated */
    Link_Layer_Activated,                   /**< Link Layer Activated */
    Link_Layer_Deactivated_Remote,          /**< Link Layer Deactivated by Remote */
    Link_Layer_Deactivation_Pending_Local,  /**< Link Layer Deactivation Locally Pending */
    Link_Layer_Deactivated_Local            /**< Link Layer Locally Deactivated */
} ptx_RUL_LLCP_ConnectionState_t;

/**
 * \brief LLCP component struct
 */
typedef struct ptx_RUL_LLCP_t
{
    ptx_Comps_t                         CompId;                                         /**< Component Id */
    ptx_RUL_t                          *Rul;                                            /**< RUL Core Component. */
    void *                              *SnepServerClient;                              /**< SNEP Server/Client component (optional). */
    ptx_RUL_LLCP_Role_t                 Role;                                           /**< LLCP Role (Initiator/Target). */
    ptx_RUL_LLCP_Role_t                 Turn;                                           /**< LLCP Role (Initiator/Target). */
    volatile uint32_t                   TimerTicks;                                     /**< TimerTicks counter. Get increments every 10ms. */
    uint32_t                            ApplicationTimerInitValue;                      /**< Internal timestamp initialized with start of connection. */
    uint32_t                            ApplicationTimerTimeout;                        /**< Timeout value. */
    uint32_t                            LastSymmPdu;                                    /**< Timestamp of last reception. */
    uint32_t                            LastRxPdu;                                      /**< Timestamp of last reception. */
    uint64_t                            SapList;                                        /**< List of used SAPs as BitMap.
                                                                                        *    Lowest 16 bits represent Well-Known Services.
                                                                                        */
    uint8_t                             GeneralBytes[PTX_LLCP_GENERAL_BYTES_MAX_LEN];   /**< General Bytes of ATR_REQ/ATR_RES. */
    uint8_t                             GeneralBytesLength;                             /**< Length of the General Bytes. */
    ptx_NSC_RfActivate_Ntf_t            ActivateNtf;                                    /**< Activate NTF Buffer */
    ptx_RUL_LLCP_TLV_Data_t             ActivationTlvs[PTX_LLCP_MAX_TLV_PARAMS];        /**< Buffer for activation TLVs */
    uint32_t                            ActivationTlvsLen;                              /**< Activation TLVs Length */
    uint8_t                             RfFieldPresent;                                 /**< RF-Field Presence Flag */
    uint8_t                             AgreedVersion;                                  /**< Agreed LLCP Verison */
    uint8_t                             Initiate;                                       /**< Initiate first SYMM PDU */
    uint8_t                             Running;                                        /**< Run processing loop flag */
    uint8_t                             TargetLinkTimeout;                              /**< Target Link Timeout */
    uint8_t                             RemoteServiceId;                                /**< Remote Service ID */
    ptx_Status_t                        LastLlcpStatus;                                 /**< Last LLCP Status */
    ptx_RUL_LLCP_Link_t                 Link[PTX_LLCP_MAX_LINKS];                       /**< Available LLCP Links */
    ptx_RUL_LLCP_Link_t                *ActiveLink;                                     /**< Poitner to currently active link */
    ptx_RUL_LLCP_Link_t                *ReceiveLink;                                    /**< Pointer to the receive link */
    ptx_RUL_LLCP_Frame_t                LlcpFrame;                                      /**< Buffer for individual frames */
    ptx_RUL_LLCP_MessageBuffer_t        TxBuffer;                                       /**< Message Buffer */
    ptx_RUL_LLCP_ConnectionState_t      State;                                          /**< Connection State */
    uint8_t                             RxData[PTX_LLCP_FRAME_BUFFER_LEN];              /**< Receive Buffer */
    uint32_t                            RxDataIntLen;                                   /**< Intermediate Receive Length Information */
    uint32_t                            RxDataLen;                                      /**< Receive Length Information */
    uint8_t                             SentI;                                          /**< Flag storing the information wether an I-PDU has been sent. */
    uint8_t                             SentUI;                                         /**< Flag storing the information wether an UI-PDU has been sent. */
    uint8_t                             ReceivedPdu;                                    /**< Flag storing the information wether a PDU has been received (excludes SYMM-PDU). */
    uint8_t                             LinkTimeoutOccured;                             /**< Flag storing the information wether a link timeout has occured. */
    uint8_t                             PeerLlcpVerison;                                /**< LLCP version used by the peer. */
    uint16_t                            PeerLlcpWks;                                    /**< Well Known Service list of the peer. */
    uint16_t                            PeerLlcpMiux;                                   /**< Miux of the peer. */
    ptx_RUL_LLCP_DisconnectReason_t     LastDisconnectReason;                           /**< Reason why the link was terminated. */
} ptx_RUL_LLCP_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initializes the LLCP component.
 *
 * This API has to be called before any other API function of this component. It will setup & initialise all the
 * internal data structures.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       initParameters      Pointer to the LLCP initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Init(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_InitParameters_t *initParameters);


/**
 * \brief Sets the provided general bytes, used for the next LLCP activation.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       generalBytes        Pointer to the General Bytes.
 * \param[in]       generalBytesLen     Length of the provided General Bytes.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_SetCustomGeneralBytes(ptx_RUL_LLCP_t *ctx, uint8_t *generalBytes, uint8_t generalBytesLen);

/**
 * \brief Sets the default values for the general bytes, used for the next LLCP activation.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       generalBytes        Pointer to store the general bytes.
 * \param[in]       generalBytesLen     Pointer to store the length of the general bytes.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_GenerateDefaultGeneralBytes(ptx_RUL_LLCP_t *ctx, uint8_t *generalBytes, uint8_t *generalBytesLen);

/**
 * \brief Starts the default discovery for LLCP  either in POLL or LISTEN mode depending in InitParameters passed to \ref ptx_RUL_LLCP_Init().
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       idleTime           The RF-discovery idle time.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_StartDefaultDiscovery(ptx_RUL_LLCP_t *ctx, uint32_t idleTime);

/**
 * \brief Tears the LLCP component down and releases all resources.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_DeInit(ptx_RUL_LLCP_t *ctx);

/**
 * \brief Exchanges an Information PDU between the specified SAPs.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       link                Existing SAP link.
 * \param[in]       payload             Pointer to the payload.
 * \param[in]       payloadLength       Length of the provided payload.
 * \param[in]       waitForAck          If enabled, the function will block until the acknowledge has beeen received or the application timeout expired.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_SendInfo(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, uint8_t *payload, uint16_t payloadLength, uint8_t waitForAck);

/**
 * \brief Waits for the activation through an Initiator.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       activated           Pointer to store information wether activation has happened.
 * \param[in]       timeoutMs           Timeout in milliseconds to wait for an activation.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_WaitForActivation(ptx_RUL_LLCP_t *ctx, uint8_t *activated, uint32_t timeoutMs);

/**
 * \brief Handles the complete LLC protocol in the background (must be periodically called).
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Handle(ptx_RUL_LLCP_t *ctx);

/**
 * \brief Waits for any incoming LLCP connection.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       timeoutMs           Timeout in milliseconds to wait for an incoming connection.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_WaitForAnyConnection(ptx_RUL_LLCP_t *ctx, uint32_t timeoutMs);

/**
 * \brief Exchanges an Unnumbered Information PDU between the specified SAPs.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       link                Existing SAP link.
 * \param[in]       payload             Pointer to the payload.
 * \param[in]       payloadLength       Length of the provided payload.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_SendUInfo(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, uint8_t *payload, uint16_t payloadLength);

/**
 * \brief Waits for an incoming connection to a SAP.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       link                Pointer to SAP link of interest.
 * \param[in]       timeoutMs           Timeout in milliseconds to wait for an incoming connection.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_WaitForConnection(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, uint32_t timeoutMs);

/**
 * \brief Sends an Service Discovery Request to return the SAP address that is registered in the remote service
 *        environment for the specified service name URI.
 *
 * \param[in]       ctx                     Pointer to the LLCP component.
 * \param[in]       serviceName             Pointer to the Service Name URI of interest.
 * \param[in,out]   dSap                    Pointer to store the discovered SAP address.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_DiscoverService(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Parameter_ServiceName_t *serviceName, uint8_t *dSap);

/**
 * \brief Waits / Blocks for incoming data from a specific SAP of interest.
 *
 * \param[in]       ctx                     Pointer to the LLCP component.
 * \param[in]       link                    Pointer to the Link of interest.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_ReceiveFromLink(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link);

/**
 * \brief Sets up and starts a timer to wait for incoming LLCP frames from a specific SAP.
 *
 * \param[in]       ctx                     Pointer to the LLCP component.
 * \param[in]       link                    Pointer to the Link of interest.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_SetupReceiveStatusHandler(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link);

/**
 * \brief Checks if LLCP frames from a specific SAP have arrived.
 *
 * \param[in]       ctx                     Pointer to the LLCP component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_CheckReceiveStatusHandler(ptx_RUL_LLCP_t *ctx);

/**
 * \brief Search for the first free/unused local SAP.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       wellKnown           Specify, whether to search within the WellKnown SAP range.
 * \param[in]       foundAddress        Pointer to store the address of the free SAP.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_GetFreeSapAddress(ptx_RUL_LLCP_t *ctx, uint8_t wellKnown, uint8_t *foundAddress);

/**
 * \brief Connects to a remote SAP using its address.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       link                Pointer to the struct, describing the link to be stablished.
 * \param[in]       dsap                Address of the destination SAP-
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_ConnectSap(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, uint8_t dsap);

/**
 * \brief Registers a SAP with the provided initialization parameters.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       link                Pointer to the struct, describing the SAP/logical link to be stablished.
 * \param[in]       params              Pointer to the initialization parameters for the link.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_RegisterSap(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t **link, ptx_RUL_LLCP_Link_InitParams_t *params);

/**
 * \brief Connects to a remote SAP using its name.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       link                Pointer to the struct describing the to be stablished link. (Dsap-address not used due to provided service name.)
 * \param[in]       name                Pointer to the datastructure describing the service name.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_ConnectSapByName(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link, char* name);

/**
 * \brief Disconnects from a remote SAP.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       link                Pointer to the existing LLCP SAP / logical link.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_DisconnectSap(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t *link);

/**
 * \brief Enables support for SNEP (Updated WKS list and registers SNEP processor callback).
 *
 * \param[in]       ctx                     Pointer to the LLCP component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_EnableSnep(ptx_RUL_LLCP_t *ctx);

/**
 * \brief Disables support for SNEP
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_DisableSnep(ptx_RUL_LLCP_t *ctx);

/**
 * \brief Retrieves information about the disconnect-reason.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in,out]   reason              Pointer to store the disconnect-reason.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_GetDisconnectReason(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_DisconnectReason_t *reason);

/**
 * \brief Checks if the remote peer supports LLCP.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in,out]   supported           Pointer to store the information whether LLCP is supported.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_IsLlcpSupported(ptx_RUL_LLCP_t *ctx, uint8_t *supported);

/**
 * \brief Prepeares the separation of a link between the source and the destination.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_DisconnectLink(ptx_RUL_LLCP_t *ctx);

/**
 * \brief Checks if the LLCP link is down.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       isDown              Pointer to store the information wheter the link is down or up and running.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_IsLinkDown(ptx_RUL_LLCP_t *ctx, uint8_t *isDown);

/**
 * \brief Allocates the resources for a new LLCP link.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       link                Pointer pointer to return the allocated link.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_AllocateLink(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t **link);

/**
 * \brief Sets the application timeout value in milliseconds, hence defining the maximum amount of time a function blocks the execution.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       applicationTimeout  Application timeout in milliseconds.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_SetApplicationTimeout(ptx_RUL_LLCP_t *ctx, uint32_t applicationTimeout);

/**
 * \brief Retrieves a pointer to the LLCP Link datastructure based on the provided Service URI.
 *
 * \param[in]       ctx                 Pointer to the LLCP component.
 * \param[in]       link                Pointer pointer to store the pointer to the link.
 * \param[in]       requestBySDP        Was it requested by the service discovery procedure?
 * \param[in]       name                Pointer to the character array representing the URI/name.
 * \param[in]       len                 Length of the URI.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_GetLinkFromUri(ptx_RUL_LLCP_t *ctx, ptx_RUL_LLCP_Link_t **link, uint8_t requestBySDP, char* name, uint32_t len);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */