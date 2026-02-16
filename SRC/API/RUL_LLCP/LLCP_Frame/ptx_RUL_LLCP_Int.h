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
    Module      : LLCP
    File        : ptx_RUL_LLCP_Int.h

    Description : LLCP internal data structures and definitions
*/

/**
 * \addtogroup RUL_LLCP
 * @{
 */

#if !defined(PTX_LLCPINT_H)
#define PTX_LLCPINT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_LLCP_PDU_DSAP_MASK          (0x3Fu) /**< Destination Service Access Point mask */
#define PTX_LLCP_PDU_DSAP_OFFSET        (0x02u) /**< Destination Service Access Point offset */
#define PTX_LLCP_PDU_SSAP_MASK          (0x3Fu) /**< Source Service Access Point mask */
#define PTX_LLCP_PDU_SSAP_OFFSET        (0x00u) /**< Source Service Access Point offset */
#define PTX_LLCP_PDU_PTYPE_MASK         (0x0Fu) /**< PDU type bitmask */
#define PTX_LLCP_PDU_PTYPE_OFFSET_HB    (0x06u) /**< PDU type high byte offset */
#define PTX_LLCP_PDU_PTYPE_OFFSET_LB    (0x02u) /**< PDU type low byte offset */
#define PTX_LLCP_PDU_SEQNS_MASK         (0x0Fu) /**< Sequence send number bitmask */
#define PTX_LLCP_PDU_SEQNS_OFFSET       (0x04u) /**< Sequence send number offset */
#define PTX_LLCP_PDU_SEQNR_MASK         (0x0Fu) /**< Sequence receive number bitmask */
#define PTX_LLCP_PDU_SEQNR_OFFSET       (0x00u) /**< Sequence receive number offset */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief NFC Logical Link Control Protocol Data Unit Types
 */
typedef enum ptx_RUL_LLCP_PduType
{
    LLCP_PduType_SYMM       = 0x00, /**< PDU Type Symmetry (SYMM) */
    LLCP_PduType_PAX        = 0x01, /**< PDU Type Parameter Exchange (PAX) */
    LLCP_PduType_AGF        = 0x02, /**< PDU Type Aggregated Frame (AGF) */
    LLCP_PduType_UI         = 0x03, /**< PDU Type Unnumbered Information (UI) */
    LLCP_PduType_CONNECT    = 0x04, /**< PDU Type Connect (CONNECT) */
    LLCP_PduType_DISC       = 0x05, /**< PDU Type Disconnect (DISC) */
    LLCP_PduType_CC         = 0x06, /**< PDU Type Connection Complete (CC) */
    LLCP_PduType_DM         = 0x07, /**< PDU Type Disconnected Mode (DM) */
    LLCP_PduType_FRMR       = 0x08, /**< PDU Type Frame Reject (FRMR) */
    LLCP_PduType_SNL        = 0x09, /**< PDU Type Service Name Lookup (SNL) */
    LLCP_PduType_DPS        = 0x0A, /**< PDU Type Data Protection Setup (DPS) */
    LLCP_PduType_RFU0       = 0x0B, /**< Reserved */
    LLCP_PduType_I          = 0x0C, /**< PDU Type Information (I) */
    LLCP_PduType_RR         = 0x0D, /**< PDU Type Receive Ready (RR) */
    LLCP_PduType_RNR        = 0x0E, /**< PDU Type Receive Not Ready (RNR) */
    LLCP_PduType_RFU        = 0x0F, /**< Reserved */
    LLCP_PduType_Max                /**< Max Value */
} ptx_RUL_LLCP_PduType_t;

/**
 * \brief NFC Logical Link Control Service Access Point Address
 */
typedef struct ptx_RUL_LLCP_SAPA
{
    uint8_t             Destination;            /**< Destination Service Access Point Address */
    uint8_t             Source;                 /**< Source Service Access Point Address */
} ptx_RUL_LLCP_SAPA_t;

/**
 * \brief NFC Logical Link Control Protocol Data Unit
 */
typedef struct ptx_RUL_LLCP_PDU
{
    ptx_RUL_LLCP_PduType_t  Ptype;                  /**< Protocol Data Unit Type */
    ptx_RUL_LLCP_SAPA_t     Sap;                    /**< Service Access Point Addresses */
    uint8_t                 SendSequenceNumber;     /**< Send Sequence Number */
    uint8_t                 ReceiveSequenceNumber;  /**< Receive Sequence Number */
    uint8_t                 InformationLen;         /**< Length of Information */
    uint8_t                *Information;            /**< Pointer to Information (= Payload) */
    uint8_t                 IcvLen;                 /**< Length of Integrity Check Value */
    uint8_t                *Icv;                    /**< Pointer to Integrity Check Value */
} ptx_RUL_LLCP_PDU_t;

typedef struct ptx_RUL_LLCP_TrxState
{
    ptx_Comps_t CompId; /**< Component Id */
    uint8_t     Vr;     /**< Receive State Variable */
    uint8_t     Vra;    /**< Receive Acknowledgement State Variable */
    uint8_t     Vs;     /**< Send State Variable */
    uint8_t     Vsa;    /**< Send Acknowledgement State Variable */
} ptx_RUL_LLCP_TrxState_t;

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
