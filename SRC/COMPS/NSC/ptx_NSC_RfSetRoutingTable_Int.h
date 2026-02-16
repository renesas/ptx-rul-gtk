/** \file
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
    Module      : NFC Soft Controller (NSC) Internal Interface Header for Routing Table Management
    File        : ptx_NSC_RfSetRoutingTable_Int.h

    Description : Definition of NFC Soft Controller Internal Interface for Routing Table Management.
                  Contains definition of types specific for routing table set-up.
*/

/**
 * \addtogroup NSC_RF_SETROUTING_TABLE
 *
 * @{
 */

#if !defined(PTX_NSC_RFSETROUTINGTABLE_INT_H)
#define PTX_NSC_RFSETROUTINGTABLE_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NSC_RfDiscover_Int.h"
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * Routing table capacity.
 */
#define PTX_NSC_RT_USER_ENTRIES_CAPACITY    10u                 /**< Max Routing Table Entries that can be configured */
#define PTX_NSC_RT_AID_MAX_LEN              16U                 /**< Max AID Length */

#define PTX_NSC_RT_CAPACITY                 256U                /**< Max Routing Table Entries */

#define PTX_NSC_RT_PROT_ENTRY_SIZE          3u                  /**< Protocol Entry Size in Bytes */
#define PTX_NSC_RT_TECH_ENTRY_SIZE          3u                  /**< Technology Entry Size in Bytes*/
#define PTX_NSC_RT_AID_ENTRY_SIZE           2u                  /**< AID Entry Size in Bytes */

/**
 * Types of entry target. Values defined in NSC protocol specification.
 */
typedef enum ptx_NSC_RoutingTarget
{
    Device_Host = 0x10,         /**< Routed to Host */
    Swp1 		= 0x20,         /**< Routed to SWP1 */
    SWP1_TypeA  = 0x21,         /**< RFU */
    SWP1_TypeB  = 0x22,         /**< RFU */
    SWP1_TypeF  = 0x23,         /**< RFU */
    SWP1_TypeV  = 0x24,         /**< RFU */
    Swp2 		= 0x30,         /**< Routed to SWP2 */
    SWP2_TypeA  = 0x31,         /**< RFU */
    SWP2_TypeB  = 0x32,         /**< RFU */
    SWP2_TypeF  = 0x33,         /**< RFU */
    SWP2_TypeV  = 0x34,         /**< RFU */
} ptx_NSC_RoutingTarget_t;

/**
 * Types of routings types
 */
typedef enum ptx_NSC_RoutingType
{
    Technology_Based = 0x01u,                   /**< Technology based routing */
    Protocol_Based = 0x02u,                     /**< Protocol based routing */
    Aid_Based = 0x05,                           /**< AID based routing */
    Apdu_Based,                                 /**< RFU */
    System_Code_Based,                          /**< RFU */
    MaxNumOfTypes                                  /**< RFU */
} ptx_NSC_RoutingType_t;

/**
 * Types of forced routing targets.
 */
typedef enum ptx_NSC_ForcedRoutingTarget
{
    ForcedTarget_DH,            /**< RFU */
    ForcedTarget_SWP1,         /**< RFU */
    ForcedTarget_SWP2,         /**< RFU */
} ptx_NSC_ForcedRoutingTarget_t;

/**
 * Routing table entry. Technology based
 */
typedef struct ptx_NSC_RoutingTech
{
    ptx_NSC_RfTech_t            Tech;           /**< Technology */
    ptx_NSC_RoutingTarget_t     Target;         /**< Target */
    uint8_t                     PowerState;     /**< RFU */
    uint8_t                     Qualifier;      /**< RFU */
} ptx_NSC_RoutingTech_t;

/**
 * Routing table entry. Protocol based
 */
typedef struct ptx_NSC_RoutingProtocol
{
    ptx_NSC_RfProtocol_t        Protocol;       /**< Protocol */
    ptx_NSC_RoutingTarget_t     Target;         /**< Target */
    uint8_t                     PowerState;     /**< RFU */
    uint8_t                     Qualifier;      /**< RFU */
} ptx_NSC_RoutingProtocol_t;

/**
 * Routing table entry. AID based
 */
typedef struct ptx_NSC_RoutingAID
{
    uint8_t                     *Aid;           /**< AID */
    uint32_t                    LengthOfAid;    /**< AID Length */
    ptx_NSC_RoutingTarget_t     Target;         /**< Target */
    uint8_t                     PowerState;     /**< RFU */
    uint8_t                     Qualifier;      /**< RFU */
} ptx_NSC_RoutingAID_t;

/**
 * Entry of routing table. SC based
 */
typedef struct ptx_NSC_RoutingSC
{
    uint8_t                     NrSCEntries;    /**< RFU */
    uint8_t                     *SCEntries;     /**< RFU */
    ptx_NSC_RoutingTarget_t     Target;         /**< RFU */
    uint8_t                     PowerState;     /**< RFU */
    uint8_t                     Qualifier;      /**< RFU */
} ptx_NSC_RoutingSC_t;

/**
 * Entry of routing table. APDU based
 */
typedef struct ptx_NSC_RoutingAPDU
{
    uint8_t                     *ApduAndMaskData;       /**< RFU */
    uint32_t                    ApduAndMaskDataLength;  /**< RFU */
    ptx_NSC_RoutingTarget_t     Target;                 /**< RFU */
    uint8_t                     PowerState;             /**< RFU */
    uint8_t                     Qualifier;              /**< RFU */
} ptx_NSC_RoutingAPDU_t;

/**
 * Compound of all possible routing table entries.
 * They will be used in a list.
 */
typedef union ptx_NSC_RoutingEntry
{
    ptx_NSC_RoutingAID_t        AidRouting;             /**< AID Routing Entry */
    ptx_NSC_RoutingProtocol_t   ProtocolRouting;        /**< Protocol Routing Entry */
    ptx_NSC_RoutingTech_t       TechRouting;            /**< Technology Routing Entry */
    ptx_NSC_RoutingSC_t         SystemCodeRouting;      /**< RFU */
    ptx_NSC_RoutingAPDU_t       ApduRouting;            /**< RFU */
} ptx_NSC_RoutingEntry_t;

/**
 * Forced Routing Info
 */
typedef struct ptx_NSC_ForcedRoutingInfo
{
    uint8_t                         ForcedRoutingEnabled;   /**< RFU */
    ptx_NSC_ForcedRoutingTarget_t   Target;                 /**< RFU */
    uint8_t                         PowerState;             /**< RFU */
} ptx_NSC_ForcedRoutingInfo_t;

/**
 * Routing Table Entry
 */
typedef struct ptx_NSC_RoutingList
{
    ptx_NSC_RoutingEntry_t          Entry;          /**< Routing Entry */
    ptx_NSC_RoutingType_t           EntryType;      /**< Routing Type */
    struct ptx_NSC_RoutingList      *NextEntry;     /**< Next Entry */
} ptx_NSC_RoutingList_t;

/**
 * Entire routing Table Structure
 */
typedef struct ptx_NSC_RoutingTable
{
    ptx_NSC_RoutingList_t           *RoutingEntries;    /**< Routing Entry */
    uint32_t                        NumOfEntries;       /**< Number of Entries */
    uint32_t                        BytesUsed;          /**< Bytes Used */
    ptx_NSC_RoutingList_t           *ReadRoutingEntry;  /**< Read Routing Entry */
    uint32_t                        ReadNumOfEntries;   /**< Read Number of Entries */
    ptx_NSC_ForcedRoutingInfo_t     ForcedRoutingInfo;  /**< RFU */
} ptx_NSC_RoutingTable_t;

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
