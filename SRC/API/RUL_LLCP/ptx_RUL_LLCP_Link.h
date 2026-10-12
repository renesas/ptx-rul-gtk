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
    File        : ptx_RUL_LLCP_Link.h

    Description : Implementation handling the service access point.
*/

/**
 * \addtogroup RUL_LLCP
 * @{
 */

#if !defined(PTX_RULLLCPLINK_H)
#define PTX_RULLLCPLINK_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>

#include "ptx_Components.h"
#include "ptx_Status.h"

#include "ptx_RUL_LLCP_ServiceAccessPoint.h"
#include "ptx_RUL_LLCP_TrxState.h"
#include "ptx_RUL_LLCP_MessageBuffer.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_LLCP_LINK_DEFAULT_SERVICE_URI_LENGTH  (128u) /**< Max supported length for local URI names. */

/**
 * \brief LLCP Service Discovery Protocol State
 */
typedef enum ptx_RUL_LLCP_ServiceDiscoveryState
{
    SDP_NotAdvertised,              /**< Service Discovery Procedure not advertised */
    SDP_Advertised                  /**< Service Discovery Procedure advertised */
} ptx_RUL_LLCP_ServiceDiscoveryState_t;

/**
 * \brief LLCP Service Access Point Type
 */
typedef enum ptx_RUL_LLCP_Link_Type
{
    LinkType_ConnectionOriented,    /**< Connection Oriented Link */
    LinkType_ConnectionLess         /**< Connection Less Link */
} ptx_RUL_LLCP_Link_Type_t;

/**
 * \brief LLCP Service Access Point State
 */
typedef enum ptx_RUL_LLCP_Link_State
{
    LinkState_Unregistered,         /**< Link Unregistered */
    LinkState_Registered,           /**< Link Registered */
    LinkState_Connected,            /**< Link Connected */
    LinkState_Disconnected,         /**< Link Disconnected */
    LinkState_ConnectionPending,    /**< Link Connection Pending */
    LinkState_DisconnectionPending, /**< Link Disconnection Pending */
    LinkState_FrameRejected         /**< Link Frame Rejected Pending */
} ptx_RUL_LLCP_Link_State_t;

/**
 * \brief LLCP Link Initialisation Parameters
 */
typedef struct ptx_RUL_LLCP_Link_InitParams
{
    uint8_t                     Ssap;           /**< Source SAP address. */
    uint8_t                     Window;         /**< Receive Window. */
    uint8_t                     Miux;           /**< Maximum information unit extension. */
    ptx_RUL_LLCP_Link_Type_t    Type;           /**< Link Type. */
    char                       *ServiceUri;     /**< Service Uri Name. */
    uint8_t                     ServiceUriLen;  /**< Service Uri Length. */
} ptx_RUL_LLCP_Link_InitParams_t;

/**
 * \brief LLCP Link Structure
 */
typedef struct ptx_RUL_LLCP_Link
{
    ptx_Comps_t                             CompId;                                                 /**< Component Id */
    ptx_RUL_LLCP_ServiceAccessPoint_t       Local;                                                  /**< Local SAP */
    ptx_RUL_LLCP_ServiceAccessPoint_t       Remote;                                                 /**< Remote SAP */
    ptx_RUL_LLCP_TrxState_t                 TrxStates;                                              /**< TRX state variables: Vr, Vra, Vs, Vsa */
    ptx_RUL_LLCP_Link_State_t               LinkState;                                              /**< Link State (SAP Registered, SAPs Connected, etc.) */
    ptx_RUL_LLCP_Link_Type_t                Type;                                                   /**< ConnectionOriented, ConnectionLess */
    ptx_RUL_LLCP_ServiceDiscoveryState_t    SdpState;                                               /**< Service Discovery Procedure*/
    uint8_t                                 InUse;                                                  /**< Link in use flag */
    uint8_t                                 BusyReleased;                                           /**< Budy released flag */
    char                                    ServiceUri[PTX_LLCP_LINK_DEFAULT_SERVICE_URI_LENGTH];   /**< Service Uri Name */
} ptx_RUL_LLCP_Link_t;


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

/**
 * \brief Initializes the LLCP-Link component.
 *
 * This API has to be called before any other API function of this component. It will setup & initialise all the
 * internal data structures.
 *
 * \param[in]       ctx                 Pointer to the LLCP-Link component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Link_Init(ptx_RUL_LLCP_Link_t *ctx);

/**
 * \brief Checks if the receive sequence number is valid.
 *
 * \param[in]       ctx                 Pointer to the LLCP-Link component.
 * \param[in]       receivedNr          The received receive sequence number.
 * \param[in,out]   valid               Pointer to store the information if the receive sequence number is valid.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Link_ValidateNrSequence(ptx_RUL_LLCP_Link_t *ctx, uint8_t receivedNr, uint8_t *valid);

/**
 * \brief Checks if the send sequence number is valid.
 *
 * \param[in]       ctx                 Pointer to the LLCP-Link component.
 * \param[in]       receivedNs          The received send sequence number.
 * \param[in,out]   valid               Pointer to store the information if the send sequence number is valid.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Link_ValidateNsSequence(ptx_RUL_LLCP_Link_t *ctx, uint8_t receivedNs, uint8_t *valid);

/**
 * \brief Writes a message into the message inbox of the LLCP link.
 *
 * \param[in]       ctx                 Pointer to the LLCP-Link component.
 * \param[in]       data                Pointer the data that shall be written to the message inbox.
 * \param[in]       dataLength          Length of the data to be written.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Link_WriteMessageInbox(ptx_RUL_LLCP_Link_t *ctx, uint8_t *data, uint32_t dataLength);

/**
 * \brief Reads a message from the inbox.
 *
 * \param[in]       ctx                 Pointer to the LLCP-Link component.
 * \param[in]       data                Pointer to store the data to be retrieved.
 * \param[in,out]   dataLength          Pointer to store the length information of the retrieved data.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Link_ReadMessageInbox(ptx_RUL_LLCP_Link_t *ctx, uint8_t *data, uint32_t *dataLength);

/**
 * \brief Writes a message into the message outbox of the LLCP link.
 *
 * \param[in]       ctx                 Pointer to the LLCP-Link component.
 * \param[in]       data                Pointer to the data, which shall be written to the outbox.
 * \param[in]       dataLength          Length of the data to be written to the outbox.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Link_WriteMessageOutbox(ptx_RUL_LLCP_Link_t *ctx, uint8_t *data, uint32_t dataLength);

/**
 * \brief Registers the LLCP Link with the given initialisation parameters.
 *
 * \param[in]       ctx                 Pointer to the LLCP-Link component.
 * \param[in]       initParams          Pointer to the links initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Link_Register(ptx_RUL_LLCP_Link_t *ctx, ptx_RUL_LLCP_Link_InitParams_t *initParams);

/**
 * \brief Sets the state of the LLCP Link to "disconnected".
 *
 * \param[in]       ctx                 Pointer to the LLCP-Link component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Link_Disconnect(ptx_RUL_LLCP_Link_t *ctx);

/**
 * \brief Resets the LLCP-Link component including state varables and message buffers.
 *
 * \param[in]       ctx                 Pointer to the LLCP-Link component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Link_Reset(ptx_RUL_LLCP_Link_t *ctx);

/**
 * \brief Tears the LLCP-ServiceAccessPoint component down.
 *
 * This API tears the complete LLCP-ServiceAccessPoint component down.
 *
 * \param[in]       ctx                 Pointer to the LLCP-Link component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Link_DeInit(ptx_RUL_LLCP_Link_t *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */