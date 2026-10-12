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
    Module      : NFC Soft Controller (NSC) Interface Header for Routing Table Management
    File        : ptx_NSC_RoutingTable.h

    Description : Definition of NFC Soft Controller Interface for Routing Table Management.
                  Contains function(s) to set NSC routing table and to send the command according to NSC protocol.
*/

/**
 * \addtogroup NSC_RF_SETROUTING_TABLE
 *
 * @{
 */

#if !defined(PTX_NSC_RFSETROUTINGTABLE_H)
#define PTX_NSC_RFSETROUTINGTABLE_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Int.h"
#include "ptx_NSC_RfSetRoutingTable_Int.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \brief Forward declaration of ptx_NSC structure.
 */
struct ptx_NSC;

/**
 * \brief Search filter for the Routing List.
 */
#define PTX_NSC_ROUTINGENTRY_ALL_TYPES                      (0xFFu)     /**< Flag searching for all types of entries. */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Function used to build and send NSC_RFSETROUTINGTABLE_CMD.
 *
 * \note This function sends the Routing Table entries to the chip. Entries were configured before.
 *
 * \param[in]    nsc                Pointer to the component structure.
 *
 * \return Status, indicating whether the operation was successful. See definition of \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable(struct ptx_NSC *nsc);

/**
 * \brief Init NSC Routing Table.
 *
 * \internal
 *
 * \param[in]    nscCtx   Pointer to the component structure.
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable_Init(struct ptx_NSC *nscCtx);

/**
 * \brief De-init NSC Routing Table.
 *
 * \internal
 *
 * \param[in]    nscCtx   Pointer to the component structure.
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable_Deinit(struct ptx_NSC *nscCtx);

/**
 * \brief Function used to add protocol entry on the routing table.
 *
 * \param[in]    nscCtx                Pointer to the component structure.
 * \param[in]    tech                  Technology to route.
 * \param[in]    target                Target to route to in case of match.
 * \param[in]    qualifier             Qualifier needed by NCI.
 * \param[in]    powerState            Power State.
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable_AddEntryTech(struct ptx_NSC *nscCtx, ptx_NSC_RfTech_t tech,
                                                    ptx_NSC_RoutingTarget_t target, uint8_t qualifier, uint8_t powerState);

/**
 * \brief Function used to add protocol entry on the routing table.
 *
 * \param[in]    nscCtx                Pointer to the component structure.
 * \param[in]    protocol              Protocol to route.
 * \param[in]    target                Target to route to in case of match.
 * \param[in]    qualifier             Qualifier for this entry.
 * \param[in]    powerState            Power State for this entry.
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable_AddEntryProtocol(struct ptx_NSC *nscCtx,  ptx_NSC_RfProtocol_t protocol,
                                                        ptx_NSC_RoutingTarget_t target, uint8_t qualifier, uint8_t powerState);

/**
 * \brief Function used to add aid based entry on the routing table.
 *
 * \param[in]    nscCtx                Pointer to the component structure.
 * \param[in]    aid                   Pointer to aid buffer.
 * \param[in]    lengthOfAid           Length of the aid buffer.
 * \param[in]    target                Target to route to in case of match.
 * \param[in]    qualifier             Qualifier for this entry.
 * \param[in]    powerState            Power State for this entry.
 * \param[in]    aidMatchType          Aid-lenght match specification.
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable_AddEntryAid(struct ptx_NSC *nscCtx, uint8_t *aid, uint32_t lengthOfAid,
                                                   ptx_NSC_RoutingTarget_t target, uint8_t qualifier, uint8_t powerState, uint8_t aidMatchType);

/**
 * \brief Function used to add system-code based entry on the routing table.
 *
 * \param[in]    nscCtx                Pointer to the component structure.
 * \param[in]    nrSCEntries           Number of System-Code entries (max. 32).
 * \param[in]    scEntries             System-Code entries (2 bytes each).
 * \param[in]    target                Target to route to in case of match.
 * \param[in]    qualifier             Qualifier for this entry.
 * \param[in]    powerState            Power State for this entry.
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable_AddEntrySystemCode(struct ptx_NSC *nscCtx, uint8_t nrSCEntries, uint8_t *scEntries,
                                                          ptx_NSC_RoutingTarget_t target, uint8_t qualifier, uint8_t powerState);

/**
 * \brief Function used to add APDU based entry on the routing table.
 *
 * \param[in]    nscCtx                Pointer to the component structure.
 * \param[in]    apduAndMaskDataLength Length of Combined APDU- and Mask-Data.
 * \param[in]    apduAndMaskData       APDU- (1st Half) and Mask-Data (2nd Half)
 * \param[in]    target                Target to route to in case of match.
 * \param[in]    qualifier             Qualifier for this entry.
 * \param[in]    powerState            Power State for this entry.
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable_AddEntryAPDU(struct ptx_NSC *nscCtx, uint32_t apduAndMaskDataLength, uint8_t *apduAndMaskData,
                                                    ptx_NSC_RoutingTarget_t target, uint8_t qualifier, uint8_t powerState);

/**
 * \brief Clear the entire routing table.
 *
 * \param[in]     nscCtx                Pointer to the component structure.
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable_ClearEntries(struct ptx_NSC *nscCtx);

/**
 * \brief Gets the number of entries that the routing table currently has.
 *
 * \param[in]    nscCtx                Pointer to the component structure.
 * \param[in]    numOfItems            Pointer where number of entries is filled in by this function.
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable_GetNumOfEntries(struct ptx_NSC *nscCtx, uint8_t *numOfItems);

/**
 * \brief Gets the entry data of the requested entry type, starting from the given index in the list.
 *
 * \param[in]     nscCtx               Pointer to the component structure.
 * \param[in]     entryType            The type of entry to search for.
 * \param[out]    rtEntry              Storage for the retreived entry data.
 * \param[in,out] itemIdx              [IN] RT list index to start searching from. [OUT] the index of the found entry.
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable_GetEntry(struct ptx_NSC *nscCtx, uint8_t entryType, ptx_NSC_RoutingList_t *rtEntry, uint8_t *itemIdx);

/**
 * \brief Enables / Disables Forced-Routing Mode.
 *
 * \param[in]    nscCtx                Pointer to the component structure.
 * \param[in]    mode                  Enable Forced-Routing (!= 0) or disable it (0).
 * \param[in]    target                Forced Routing Target.
 * \param[in]    powerState            Power State for this entry.
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfSetRoutingTable_EnableForcedRouting(struct ptx_NSC *nscCtx, uint8_t mode, ptx_NSC_ForcedRoutingTarget_t target, uint8_t powerState);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
