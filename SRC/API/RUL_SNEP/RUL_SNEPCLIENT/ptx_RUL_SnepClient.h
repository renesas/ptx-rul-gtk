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
    Module      : SNEP
    File        : ptx_RUL_SnepClient.h

    Description : Implementation of the SNEP client.
*/

/**
 * \addtogroup RUL_SNEPCLIENT
 * @{
 */

#if !defined(PTX_RULSNEPCLIENT_H)
#define PTX_RULSNEPCLIENT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>

#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_RUL.h"
#include "ptx_RUL_LLCP.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/**
 * \brief General default definitions.
 * @{
 */
#define PTX_SNEPCLIENT_SENDBUFFER_SIZE      (1024u) /**< Default size of the SNEP_Client transmit buffer. */
#define PTX_SNEPCLIENT_RECVEIVEBUFFER_SIZE  (1024u) /**< Default size of the SNEP_Client receive buffer. */
#define PTX_SNEPCLIENT_LENGTH_LENGTH        (4u)    /**< Length of the length field */
#define PTX_SNEPCLIENT_DEFAULT_ADDRESS      (4u)    /**< Default address of the SNEP SAP. */
#define PTX_SNEPCLIENT_DEFAULT_RW           (2u)    /**< Default size of the Receive Window (RW) used for SNEP. */
#define PTX_SNEPCLIENT_DEFAULT_MIUX         (0x7A)  /**< Default Maximum Information Unit (MIUX). */
#define PTX_SNEPCLIENT_DEFAULT_VERSION      (0x10)  /**< Default SNEP version number. */
/** @} */

/**
 * \brief Bitmask to extract the SNEP major version
 * @{
 */
#define PTX_SNEPCLIENT_MAJOR_VERSION_MASK   (0xF0)
/** @} */


/**
 * \brief SNEP frame header definitions.
 * @{
 */
#define PTX_SNEP_FRAME_HEADER_LEN       (6u)
/** @} */

/**
 * \brief SNEP service URI definitions.
 * @{
 */
#define PTX_SNEP_SERVICE_URI_DTA        ("urn:nfc:sn:sneptest")
/** @} */

/**
 * \brief SNEP internal states
 */
typedef enum ptx_RUL_SnepClient_SnepState
{
    ClientLinkState_MacLayerDeactivated,    /**< MAC Layer has been deactivated. */
    ClientLinkState_LlcpLinkUp,             /**< LLCP link is up & running. */
    ClientLinkState_LlcpLinkDown,           /**< LLCP link is down. */
    ClientLinkState_ClientActive,           /**< SNEP client is running. */
    ClientLinkState_ServerActive,           /**< SNEP server is running. */
} ptx_RUL_SnepClient_SnepState_t;

/**
 * \brief Internal statemachine states of the SNEP client
 */
typedef enum ptx_RUL_SnepClient_State
{
    ClientState_Idle,               /**< ClientState_Idle */
    ClientState_PrepareRequest,     /**< ClientState_PrepareRequest */
    ClientState_SendRequest,        /**< ClientState_SendRequest */
    ClientState_SendFragment,       /**< ClientState_SendFragment */
    ClientState_SendContinue,       /**< ClientState_SendContinue */
    ClientState_SendReject,         /**< ClientState_SendReject */
    ClientState_ReceiveResponse,    /**< ClientState_ReceiveResponse */
    ClientState_ReceiveFragment,    /**< ClientState_ReceiveFragment */
    ClientState_OperationFinished   /**< ClientState_OperationFinished */
} ptx_RUL_SnepClient_State_t;

/**
 * \brief SNEP request types.
 */
typedef enum ptx_RUL_SnepClient_RequestType
{
    PutRequest,                             /**< SNEP Put Request. */
    GetRequest                              /**< SNEP Get Request. */
} ptx_RUL_SnepClient_RequestType_t;

/**
 * \brief SNEP request codes.
 */
typedef enum ptx_RUL_SnepClient_RequestCode
{
    ClientRequestCode_Continue    = 0x00,   /**< SNEP Continue request code. */
    ClientRequestCode_Get         = 0x01,   /**< SNEP Get request code. */
    ClientRequestCode_Put         = 0x02,   /**< SNEP Put request code. */
    ClientRequestCode_RFU         = 0x03,   /**< SNEP RFU request code. */
    ClientRequestCode_Reject      = 0x7F,   /**< SNEP Reject request code. */
    ClientRequestCode_Null        = 0xFF    /**< SNEP NULL request code. */
} ptx_RUL_SnepClient_RequestCode_t;

/**
 * \brief SNEP response codes.
 */
typedef enum ptx_RUL_SnepClient_ResponseType
{
    ClientResponseType_Continue            = 0x80,  /**< SNEP Continue response code. */
    ClientResponseType_Success             = 0x81,  /**< SNEP Success response code. */
    ClientResponseType_NotFound            = 0xC0,  /**< SNEP Not Found response code. */
    ClientResponseType_ExcessData          = 0xC1,  /**< SNEP Excess Data response code. */
    ClientResponseType_BadRequest          = 0xC2,  /**< SNEP Bad Request response code. */
    ClientResponseType_NotImplemented      = 0xE0,  /**< SNEP Not Implemented response code. */
    ClientResponseType_UnsupportedVersion  = 0xE1,  /**< SNEP Unsupported Version response code. */
    ClientResponseType_ResponseReject      = 0xFF   /**< SNEP Reject response code. */
} ptx_RUL_SnepClient_ResponseType_t;

/**
 * \brief SNEP Client component structure
 */
typedef struct ptx_RUL_SnepClient
{
    ptx_Comps_t                         CompId;                                         /**< Component Id */
    ptx_RUL_LLCP_t                     *Llcp;                                           /**< Pointer to the LLCP component. */
    ptx_RUL_LLCP_Link_t                *Link;                                           /**< Pointer to the link used for the SNEP communication. */
    ptx_Status_t                        LastSnepStatus;                                 /**< Status of the previous SNEP operation. */
    ptx_RUL_SnepClient_State_t          ClientState;                                    /**< State of the SNEP Operation. */
    ptx_RUL_SnepClient_SnepState_t      State;                                          /**< State of the SNEP Client. */
    ptx_RUL_SnepClient_RequestType_t    RequestType;                                    /**< SNEP Request type. */
    ptx_RUL_SnepClient_ResponseType_t   LastSnepResponseCode;                           /**< Response code of the last SNEP operation. */
    uint32_t                            StateTick;                                      /**< Tick variable for measuring timings. */
    uint8_t                             ConnectedToServer;                              /**< Connection state. */
    uint8_t                             TxContinuationMode;                             /**< Tx continuation mode. */
    uint8_t                             RxContinuationMode;                             /**< Rx continuation mode. */
    uint8_t                             MsgToSendBuffer[PTX_SNEPCLIENT_SENDBUFFER_SIZE];/**< Message buffer for messages to be transmitted. */
    uint32_t                            MsgToSendSize;                                  /**< Size of the TX message buffer. */
    uint32_t                            MsgToSendIndex;                                 /**< Current write index for TX message. */
    uint8_t                             MsgToRecvBuffer[PTX_SNEPCLIENT_RECVEIVEBUFFER_SIZE];  /**< Message buffer for received messages. */
    uint32_t                            MsgToRecvSize;                                  /**< Size of the received message buffer. */
    uint32_t                            MsgToRecvIndex;                                 /**< Current index for received message buffer. */
    uint32_t                            RequestLen;                                     /**< Length of the request. */
    uint8_t                             TxBuffer[PTX_LLCP_FRAME_BUFFER_LEN];            /**< Transmit frame buffer. */
    uint32_t                            TxBufferLen;                                    /**< Transmit frame length. */
    uint8_t                             ExtendedDtaMode;                                /**< Enable DTA functionalities. */
    uint32_t                            ProcessedBytes;                                 /**< Number of bytes already processed. */
    uint32_t                            CurrentNrBytes;                                 /**< Current number of bytes. */
    uint8_t                             ContinueResponseReceived;                       /**< Continue response. */
    ptx_Log_t                           *Log;                                           /**< Pointer to logger context. */


} ptx_RUL_SnepClient_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initiate the SNEP client.
 *
 * \param[in]   ctx       Pointer to an uninitialized SnepClient component.
 * \param[in]   llcp      Pointer to an initialized Llcp component.
 * \param[in]   enableDTA Flag to enable the extended DTA fcuntionality.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepClient_Init(ptx_RUL_SnepClient_t *ctx, ptx_RUL_LLCP_t *llcp, uint8_t enableDTA);

/**
 * \brief Deinitialize the SNEP client.
 *
 * \param[in]   ctx       Pointer to an initialized SnepClient component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepClient_DeInit(ptx_RUL_SnepClient_t *ctx);

/**
 * \brief Starts the default RF-discovery for the SNEP client (with LLCP magic bytes set).
 *
 * \param[in]   ctx       Pointer to an initialized SnepClient component.
 * \param[in]   idleTime  The RF-discovery idle time.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepClient_StartDefaultDiscovery(ptx_RUL_SnepClient_t *ctx, uint32_t idleTime);

/**
 * \brief Waits for a defined time until a peer has been activated.
 *
 * \param[in]   ctx       Pointer to an initialized SnepServer component.
 * \param[in]   activated Pointer to store the information whether the activation was successful.
 * \param[in]   timeout   Time in milliseconds to wait for an activation.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepClient_WaitForActivation(ptx_RUL_SnepClient_t *ctx, uint8_t *activated, uint32_t timeout);

/**
 * \brief Tries to establish a connection to a SNEP server within a given timeout in milliseconds.
 *
 * \param[in]   ctx       Pointer to an initialized SnepClient component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepClient_ConnectToServer(ptx_RUL_SnepClient_t *ctx);

/**
 * \brief Write a message.
 *
 * \param[in]   ctx             Pointer to an initialized SnepClient component.
 * \param[in]   message         Pointer to the message buffer.
 * \param[in]   messageLength   Length of the PUT message.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepClient_PutMessage(ptx_RUL_SnepClient_t *ctx, uint8_t *message, uint32_t messageLength);

/**
 * \brief Retrieve a message.
 *
 * \param[in]   ctx             Pointer to an initialized SnepClient component.
 * \param[in]   message         Pointer to the message buffer.
 * \param[in]   messageLength   Length of the GET message.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepClient_GetMessage(ptx_RUL_SnepClient_t *ctx, uint8_t *message, uint32_t messageLength);

/**
 * \brief SnepClient state machine that needs to be called periodically.
 *
 * \param[in]   ctx             Pointer to an initialized SnepClient component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepClient_Handle(ptx_RUL_SnepClient_t *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */