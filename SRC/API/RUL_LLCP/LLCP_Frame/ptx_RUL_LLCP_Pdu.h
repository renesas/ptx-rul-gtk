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
    File        : ptx_RUL_LLCP_Pdu.h

    Description : Implementation of the Logical Link Control Protocol Data Unit (LLCP PDU).
*/

/**
 * \addtogroup RUL_LLCP
 * @{
 */

#if !defined(PTX_LLCPPDU_H)
#define PTX_LLCPPDU_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Components.h"
#include "ptx_Status.h"

#include "ptx_RUL_LLCP_TLV.h"
#include "ptx_RUL_LLCP_Frame.h"
#include "ptx_RUL_LLCP_Int.h"

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/**
 * \name SAP validity check
 *
 * @{
 */

/**
 * \brief Check whether the SAP is valid.
 */
#define PTX_SAP_CHECK(ptx_sap_mask, ptx_sap) \
    (((~ptx_sap_mask) & (ptx_sap)) == 0)

/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief LLCP Reject Report
 */
typedef struct ptx_RUL_LLCP_RejectReport
{
    uint8_t                 W;                      /**< Well-formedness Error, PDU was invalid or not well formed. */
    uint8_t                 I;                      /**< Information Field Error, PDU contained an incorrect or unexpected information field. */
    uint8_t                 R;                      /**< Receive Sequence Error, PDU contained an invalid N(R) in the sequence field. */
    uint8_t                 S;                      /**< Send Sequence Error, PDU contained an invalid N(S) in the sequence field. */
    ptx_RUL_LLCP_PduType_t  Ptype;                  /**< Indicates the type of the rejected PDU. */
    uint8_t                 SendSequenceNumber;     /**< Current value of the send state variable. */
    uint8_t                 ReceiveSequenceNumber;  /**< Current value of the receive state variable. */
    uint8_t                 Vs;                     /**< Current value of the send state variable. */
    uint8_t                 Vr;                     /**< Current value of the receive state variable. */
    uint8_t                 Vsa;                    /**< Current value of the send acknowledge state variable. */
    uint8_t                 Vra;                    /**< Current value of the receive acknowledge state variable. */
} ptx_RUL_LLCP_RejectReport_t;


/**
 * \brief LLCP Disconnect Reason
 */
typedef enum ptx_RUL_LLCP_DisconnectReason
{
    DisconnectReason_Disconnected       = 0x00, /**< Indicates reception of DISCONNECT PDU. */
    DisconnectReason_NoActiveConnection = 0x01, /**< Indicates that the target SAP has no active connection. */
    DisconnectReason_NoServiceBound     = 0x02, /**< Indicates that there is no service attached to the specified target SAP. */
    DisconnectReason_ConnectRejected    = 0x03, /**< Indicates that the Connect-Request was rejected by the service layer. */
    DisconnectReason_PermRejectSameSap  = 0x10, /**< Indicates that the communication partner will permanently reject any CONNECT PDUs with the same target SAP address. */
    DisconnectReason_PermRejectAnySap   = 0x11, /**< Indicates that the communication partner will permanently reject any CONNECT PDUs with any target SAP address. */
    DisconnectReason_TempRejectSameSap  = 0x20, /**< Indicates that the communication partner will temporarily reject any CONNECT PDUs with the same target SAP address. */
    DisconnectReason_TempRejectAnySap   = 0x21, /**< Indicates that the communication partner will temporarily reject any CONNECT PDUs with any target SAP address. */
    DisconnectReason_Max                        /**< Max. value. Not used */
} ptx_RUL_LLCP_DisconnectReason_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Writes a Symmetry PDU to the internal frame buffer.
 *
 * \param[in]       ctx             Pointer to the LLCP-Frame structure.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_Sym (ptx_RUL_LLCP_Frame_t *frame);

/**
 * \brief Writes a Parameter Exchange PDU to the internal frame buffer.
 *
 * \param[in]       ctx             Pointer to the LLCP-Frame structure.
 * \param[in]       tlvs            Pointer to a byte array containing the TLV structures to be appended to the PDU.
 * \param[in]       tlvLength       Amount of entries in the TLV structure array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_Pax (ptx_RUL_LLCP_Frame_t *ctx, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvsLength);

/**
 * \brief Writes an AGF PDU to the internal frame buffer.
 *
 * \param[in]       ctx             Pointer to the LLCP-Frame structure.
 * \param[in]       pdus            Pointer to the aggregated PDUs as byte array.
 * \param[in]       pdusLen         Length of the aggregated PDUs in bytes.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_Agf (ptx_RUL_LLCP_Frame_t *ctx, uint8_t *pdus, uint8_t pdusLen);

/**
 * \brief Writes an Unnumbered Information PDU to the internal frame buffer.
 *
 * \param[in]       ctx             Pointer to the a LLCP-Frame structure.
 * \param[in]       dsap            Address of the sestinateion Service Access Point.
 * \param[in]       ssap            Address of the source Service Access Point.
 * \param[in]       information     Pointer to the byte array contining the information to be transmitted.
 * \param[in]       informationLen  Lenggth in bytes of the information to be transmitted.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_Ui (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, uint8_t *information, uint8_t informationLen);

/**
 * \brief Writes a Connect PDU to the internal frame buffer.
 *
 * \param[in]       ctx             Pointer to the LLCP-Frame structure.
 * \param[in]       dsap            Address of the sestinateion Service Access Point.
 * \param[in]       ssap            Address of the source Service Access Point.
 * \param[in]       tlvs            Pointer to a byte array containing the TLV structures to be appended to the PDU.
 * \param[in]       tlvLength       Amount of entries in the TLV structure array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_Connect (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvLength);

/**
 * \brief Writes a Disconnect PDU to the internal frame buffer.
 *
 * \param[in]       ctx             Pointer to the LLCP-Frame structure.
 * \param[in]       dsap            Address of the sestinateion Service Access Point.
 * \param[in]       ssap            Address of the source Service Access Point.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_Disconnect (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap);

/**
 * \brief Writes a Connection Complete PDU to the internal frame buffer.
 *
 * \param[in]       ctx             Pointer to the LLCP-Frame structure.
 * \param[in]       dsap            Address of the sestinateion Service Access Point.
 * \param[in]       ssap            Address of the source Service Access Point.
 * \param[in]       tlvs            Pointer to a byte array containing the TLV structures to be appended to the PDU.
 * \param[in]       tlvLength       Amount of entries in the TLV structure array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_ConnectionComplete (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvLength);

/**
 * \brief Writes a Disconnect Mode PDU to the internal frame buffer.
 *
 * \param[in]       ctx             Pointer to the LLCP-Frame structure.
 * \param[in]       dsap            Address of the sestinateion Service Access Point.
 * \param[in]       ssap            Address of the source Service Access Point.
 * \param[in]       reason          Pointer to the structure containing the reason for the disconnection.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_DisconnectMode (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, ptx_RUL_LLCP_DisconnectReason_t reason);

/**
 * \brief Writes a Frame Reject PDU to the internal frame buffer.
 *
 * \param[in]       ctx             Pointer to the LLCP-Frame structure.
 * \param[in]       dsap            Address of the sestinateion Service Access Point
 * \param[in]       ssap            Address of the source Service Access Point.
 * \param[in]       rejectReport    Pointer to the structure containing the reason for the frame reject PDU.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_FrameReject (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, ptx_RUL_LLCP_RejectReport_t *rejectReport);

/**
 * \brief Writes a Information PDU to the internal frame buffer.
 *
 * \param[in]       ctx             Pointer to the LLCP-Frame structure.
 * \param[in]       dsap            Address of the sestinateion Service Access Point.
 * \param[in]       ssap            Address of the source Service Access Point.
 * \param[in]       sseq            Send sequence number
 * \param[in]       rseq            Receive sequence number
 * \param[in]       information     Pointer to the byte array that shall be transmitted as payload.
 * \param[in]       informationLen  Length of the byte array that shall be transmitted as payload.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_Information (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, uint8_t sseq, uint8_t rseq, uint8_t *information, uint8_t informationLen);

/**
 * \brief Writes a Receive Ready PDU to the internal frame buffer.
 *
 * \param[in]       ctx                     Pointer to the LLCP-PDU structure.
 * \param[in]       dsap                    Address of the sestinateion Service Access Point.
 * \param[in]       ssap                    Address of the source Service Access Point.
 * \param[in]       sseq                    Send sequence number
 * \param[in]       rseq                    Receive sequence number
 * \param[in]       receiveSequenceNumber   Receive sequence number.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_ReceiveReady (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, uint8_t receiveSequenceNumber);

/**
 * \brief Writes a Receive Not Ready PDU to the internal frame buffer.
 *
 * \param[in]       ctx                     Pointer to the LLCP-PDU structure.
 * \param[in]       dsap                    Address of the sestinateion Service Access Point.
 * \param[in]       ssap                    Address of the source Service Access Point.
 * \param[in]       sseq                    Send sequence number
 * \param[in]       rseq                    Receive sequence number
 * \param[in]       receiveSequenceNumber   Receive sequence number.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_ReceiveNotReady (ptx_RUL_LLCP_Frame_t *ctx, uint8_t dsap, uint8_t ssap, uint8_t receiveSequenceNumber);

/**
 * \brief Writes a Service Name Lookup PDU to the internal frame buffer.
 *
 * \param[in]       ctx         Pointer to the LLCP-PDU structure.
 * \param[in]       tlvs        Pointer to a byte array containing the TLV structures to be appended to the PDU.
 * \param[in]       tlvLength   Amount of entries in the TLV structure array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_ServiceNameLookup (ptx_RUL_LLCP_Frame_t *ctx, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvLength);

/**
 * \brief Writes a Data Protection Setup PDU to the internal frame buffer.
 *
 * \param[in]       ctx         Pointer to the LLCP-PDU structure.
 * \param[in]       tlvs        Pointer to a byte array containing the TLV structures to be appended to the PDU.
 * \param[in]       tlvLength   Amount of entries in the TLV structure array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_PDU_DataProtectionSetup (ptx_RUL_LLCP_Frame_t *ctx, ptx_RUL_LLCP_TLV_Data_t *tlvs, uint32_t tlvLength);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
