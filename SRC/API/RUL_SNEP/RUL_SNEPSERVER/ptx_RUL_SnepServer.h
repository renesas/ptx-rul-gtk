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
    Module      : SNEP
    File        : ptx_RUL_SnepServer.h

    Description : Implementation of the SNEP server.
*/

/**
 * \addtogroup RUL_SNEPSERVER
 * @{
 */

#if !defined(PTX_RULSNEPSERVER_H)
#define PTX_RULSNEPSERVER_H

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
 * \name General SNEP definitions.
 * @{
 */
#define PTX_SNEPSERVER_SENDBUFFER_SIZE      (1024u) /**< Default size of the SNEP_Server transceive buffer. */
#define PTX_SNEPSERVER_DEFAULT_ADDRESS      (4u)    /**< Default address of the SNEP SAP. */
#define PTX_SNEPSERVER_DEFAULT_RW           (2u)    /**< Default size of the Receive Window (RW) used for SNEP. */
#define PTX_SNEPSERVER_DEFAULT_MIUX         (0x7A)  /**< Default Maximum Information Unit (MIUX). */
#define PTX_SNEPSERVER_DEFAULT_VERSION      (0x10)  /**< Default SNEP version number. */
#define PTX_SNEPSERVER_LENGTH_LENGTH        (4u)    /**< Length of the length field */
/** @} */

/**
 * \brief Bitmask to extract the SNEP major version
 * @{
 */
#define PTX_SNEPSERVER_MAJOR_VERSION_MASK   (0xF0)
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
#define PTX_SNEP_SERVICE_URI            ("urn:nfc:sn:snep")     /**< SNEP Service URI */
#define PTX_SNEP_SERVICE_URI_DTA        ("urn:nfc:sn:sneptest") /**< SNEP DTA Service URI */
/** @} */

/**
 * \brief Internal state machine states of the SNEP server.
 */
typedef enum ptx_RUL_SnepServer_State
{
    ServerState_ReceiveRequest,     /**< ServerState_ReceiveRequest */
    ServerState_ReceiveFragment,    /**< ServerState_ReceiveFragment */
    ServerState_SendMessage,        /**< ServerState_SendMessage */
    ServerState_SendFragment,       /**< ServerState_SendFragment */
    ServerState_SendDisc,           /**< ServerState_SendDisc */
    ServerState_ServerDown          /**< ServerState_ServerDown */
} ptx_RUL_SnepServer_State_t;

/**
 * \brief SNEP request codes.
 */
typedef enum ptx_RUL_SnepServer_RequestType
{
    ServerRequestType_RequestContinue    = 0x00,    /**< SNEP Continue request code. */
    ServerRequestType_RequestGet         = 0x01,    /**< SNEP GET request code. */
    ServerRequestType_RequestPut         = 0x02,    /**< SNEP PUT request code. */
    ServerRequestType_RequestRFU         = 0x03,    /**< SNEP RFU request code. */
    ServerRequestType_RequestReject      = 0x7F,    /**< SNEP reject request code. */
    ServerRequestType_RequestNull        = 0xFF     /**< SNEP null request code. */
} ptx_RUL_SnepServer_RequestType_t;

/**
 * \brief SNEP response codes.
 */
typedef enum ptx_RUL_SnepServer_ResponseCode
{
    ServerResponseCode_Null                 = 0x00, /**< SNEP Null response code. */
    ServerResponseCode_Continue             = 0x80, /**< SNEP Continue response code. */
    ServerResponseCode_Success              = 0x81, /**< SNEP Success response code. */
    ServerResponseCode_NotFound             = 0xC0, /**< SNEP Not Found response code. */
    ServerResponseCode_ExcessData           = 0xC1, /**< SNEP Excess Data response code. */
    ServerResponseCode_BadRequest           = 0xC2, /**< SNEP Bad Request response code. */
    ServerResponseCode_NotImplemented       = 0xE0, /**< SNEP Not Implemented response code. */
    ServerResponseCode_UnsupportedVersion   = 0xE1, /**< SNEP Unsupported Version response code. */
    ServerResponseCode_Reject               = 0xFF  /**< SNEP Reject response code. */
} ptx_RUL_SnepServer_ResponseCode_t;

/**
 * \brief SNEP Server component
 */
typedef struct ptx_RUL_SnepServer
{
    ptx_Comps_t                     CompId;                                     /**< Component Id */
    ptx_RUL_LLCP_t                 *Llcp;                                       /**< LLCP component structure. */
    ptx_RUL_LLCP_Link_t            *Link;                                       /**< Pointer to the link used for the SNEP communication. */
    ptx_Status_t                    LastSnepStatus;                             /**< SNEP status of the previous SNEP operation. */
    ptx_RUL_SnepServer_State_t      ServerState;                                /**< State of the SNEP Operation. */
    uint8_t                         Running;                                    /**< Variable storing the information wether the server is still running. */
    uint32_t                        StateTick;                                  /**< Internal timer tick (incremented every 10ms). */
    uint8_t                         TxContinuationMode;                         /**< Tx continuation mode. */
    uint8_t                         RxContinuationMode;                         /**< Rx continuation mode. */
    uint8_t                         ContinueReqHandled;                         /**< Flag storing the information wether the first continue request has been handled. */
    uint32_t                        ProcessedBytes;                             /**< Processed number of bytes. */
    uint32_t                        CurrentNrBytes;                             /**< Received bytes of the DTA GET_MESSAGE */
    uint8_t                         MsgBuffer[PTX_SNEPSERVER_SENDBUFFER_SIZE];  /**< Buffer to store the DTA GET_MESSAGE */
    uint32_t                        MsgBufferIndex;                             /**< Buffer to store the DTA GET_MESSAGE */
    uint32_t                        MsgBufferLen;                               /**< Length of the DTA GET_MESSAGE. */
    uint8_t                         TxBuffer[PTX_LLCP_FRAME_BUFFER_LEN];        /**< TX frame buffer. */
    uint32_t                        TxBufferLen;                                /**< TX frame length. */
    uint32_t                        RequestLen;                                 /**< Length of the SNEP request. */
    uint8_t                         ExtendedDtaMode;                            /**< Is the extended DTA functionality enabled? */
    uint8_t                         DtaFragmentation;                           /**< DTA Fragmentation mode */
    uint8_t                         DisconnectPending;                          /**< Is a disconnection pending? */
} ptx_RUL_SnepServer_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initiate the SNEP server.
 *
 * \param[in]   ctx       Pointer to an uninitialized SnepServer component.
 * \param[in]   llcp      Pointer to an initialized LLCP component.
 * \param[in]   enableDTA Flag to enable the extended DTA fcuntionality.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepServer_Init(ptx_RUL_SnepServer_t *ctx, ptx_RUL_LLCP_t *llcp, uint8_t enableDTA);

/**
 * \brief Stops the SNEP server.
 *
 * \param[in]   ctx       Pointer to an uninitialized SnepServer component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepServer_Stop(ptx_RUL_SnepServer_t *ctx);

/**
 * \brief Deinitializes the SNEP server.
 *
 * \param[in]   ctx       Pointer to an initialized SnepServer component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepServer_DeInit(ptx_RUL_SnepServer_t *ctx);

/**
 * \brief Starts the default RF-discovery for the SNEP server (with LLCP magic bytes set).
 *
 * \param[in]   ctx       Pointer to an initialized SnepServer component.
 * \param[in]   idleTime  The RF-discovery idle time.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepServer_StartDefaultDiscovery(ptx_RUL_SnepServer_t *ctx, uint32_t idleTime);

/**
 * \brief Waits for a defined time until a peer has been activated.
 *
 * \param[in]   ctx       Pointer to an initialized SnepServer component.
 * \param[in]   activated Pointer to store the information whether the activation was successful.
 * \param[in]   timeout   Time in milliseconds to wait for an activation.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepServer_WaitForActivation(ptx_RUL_SnepServer_t *ctx, uint8_t *activated, uint32_t timeout);

/**
 * \brief Waits for an incoming connection from a client.
 *
 * \param[in]   ctx       Pointer to an initialized SnepServer component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepServer_WaitForClient(ptx_RUL_SnepServer_t *ctx);

/**
 * \brief Write a message as response to a GET request.
 *
 * \param[in]   ctx             Pointer to an initialized SnepServer component.
 * \param[in]   message         Pointer to the message to be send.
 * \param[in]   messageLength   Length of the message to be send.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepServer_PutMessage(ptx_RUL_SnepServer_t *ctx, uint8_t *message, uint32_t messageLength);

/**
 * \brief Retrieve a message writte by the client via a PUT request.
 *
 * \param[in]   ctx             Pointer to an initialized SnepServer component.
 * \param[in]   message         Pointer to the message to be send.
 * \param[in]   messageLength   Pointer to store the message length;
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepServer_GetMessage(ptx_RUL_SnepServer_t *ctx, uint8_t *message, uint32_t *messageLength);

/**
 * \brief SnepServer state machine that needs to be called periodically.
 *
 * \param[in]   ctx             Pointer to an initialized SnepClient component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_RUL_SnepServer_Handle(ptx_RUL_SnepServer_t *ctx);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
