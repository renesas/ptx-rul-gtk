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
    Module      : NFC Soft Controller (NSC) Interface for Routing Table Management
    File        : ptx_NSC_RfSetRoutingTable.c

    Description : Implementation of NFC Soft Controller Interface for Routing Table Management.
                  Contains function(s) to set routing table according to NSC protocol.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Int.h"
#include "ptx_NSC_StateMachine.h"
#include "ptx_NSC_RfSetRoutingTable.h"
#include "ptx_NSC_RfSetRoutingTable_Int.h"
#include <string.h>


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * Definitions for NSC_RFSETROUTINGTABLE_CMD.
 */
#define PTX_NSC_RFSETROUTINGTABLE_CMD_OPCODE_LENGTH         (0x01u)     /* NSC RF Set Routing Table Opcode length */
#define PTX_NSC_RFSETROUTINGTABLE_CMD_RT_LENGTH_PARAMLEN    (0x01u)     /* NSC RF Set Routing Table RT_LENGTH parameter length */
#define PTX_NSC_RFSETROUTINGTABLE_CMD_PID_PARAMLEN          (0x01u)     /* NSC RF Set Routing Table pID length */
#define PTX_NSC_RFSETROUTINGTABLE_CMD_PID_VALUE             (0x01u)     /* NSC RF Set Routing Table pID value */
#define PTX_NSC_RFSETROUTINGTABLE_CMD_RT_LENGTH_INDEX       PTX_NSC_RFSETROUTINGTABLE_CMD_OPCODE_LENGTH /* Offset of RT_LENGTH parameter in the buffer */

#define PTX_NSC_RFSETROUTINGTABLE_CMD_OVH_LENGTH            PTX_NSC_RFSETROUTINGTABLE_CMD_OPCODE_LENGTH + \
    PTX_NSC_RFSETROUTINGTABLE_CMD_PID_PARAMLEN + \
    PTX_NSC_RFSETROUTINGTABLE_CMD_RT_LENGTH_PARAMLEN

#define PTX_NSC_RFSETROUTINGTABLE_BUFFER_MAX_LEN            PTX_NSC_LENGTH_MAX      /* RF Config Buffer maximum size */

#define PTX_NSC_RFSETROUTINGTABLE_ENTRYTYPE_MASK            (0x0Fu)     /* Lower nibble is relevant for detecting the type */
#define PTX_NSC_RFSETROUTINGTABLE_AID_MATCH_BITS            (0x30u)     /* SELECT AID relevant bits */

#define PTX_NSC_ROUTINGENTRY_SIZE_TECH                      (3u)        /* Technology routing entry size 3By: Type + Technology + Target */
#define PTX_NSC_ROUTINGENTRY_SIZE_PROT                      (3u)        /* Protocol routing entry size 3By: Type + Protocol + Target */
#define PTX_NSC_ROUTINGENTRY_SIZE_AID_FIXED                 (3u)        /* AID routing entry size -fixed part 3By: Type + Aid length + Target */


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static void ptx_NSC_RfSetRoutingTable_DeinitList(ptx_NSC_t *nscCtx);
static ptx_NSC_RoutingList_t *ptx_NSC_RfSetRoutingTable_GetUnusedEntry (ptx_NSC_RoutingList_t *rtList, uint8_t *usedEntriesCount);
static void ptx_NSC_RfSetRoutingTable_LoadEntries(ptx_NSC_RoutingList_t *rtList, uint8_t routetPrio, uint8_t *buffer, uint32_t *buffer_offset);
static ptx_Status_t ptx_NSC_RoutingTable_CheckSize(ptx_NSC_RoutingTable_t *routingTable);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_RfSetRoutingTable_Init(ptx_NSC_t *nscCtx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* At this point NSC Comp ID has to be valid and NSC Extension Initialized. */

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC))
    {
        PTX_LOG_I(nscCtx->Log, "ptx_NSC_RfSetRoutingTable_Init");

        (void)memset(&(nscCtx->RoutingTable), 0, sizeof(ptx_NSC_RoutingTable_t));

        for (uint32_t idx = 0; idx < PTX_NSC_RT_USER_ENTRIES_CAPACITY; idx++)
        {
            /* Initially set to invalid entry type */
            nscCtx->RoutingTable.RoutingEntries[idx].EntryType = MaxNumOfTypes;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_Deinit(ptx_NSC_t *nscCtx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC))
    {
        PTX_LOG_I(nscCtx->Log, "ptx_NSC_RfSetRoutingTable_Deinit");

        /*
         * De-allocate Routing Table List.
         */
        ptx_NSC_RfSetRoutingTable_DeinitList(nscCtx);

        /*
         * Set Routing Table memory zero.
         */
        (void)memset(&(nscCtx->RoutingTable), 0, sizeof(ptx_NSC_RoutingTable_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_AddEntryAid(ptx_NSC_t *nscCtx, uint8_t *aid, uint32_t lengthOfAid,
                                                   ptx_NSC_RoutingTarget_t target, uint8_t qualifier, uint8_t powerState, uint8_t aidMatchType)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC) && (NULL != aid) && (lengthOfAid <= PTX_NSC_RT_AID_MAX_LEN))
    {
        PTX_LOG_I(nscCtx->Log, "ptx_NSC_RfSetRoutingTable_AddEntryAid");

        /* Get the first unused entry from the list. If all are used, NULL is returned. */
        ptx_NSC_RoutingList_t *rt_new = ptx_NSC_RfSetRoutingTable_GetUnusedEntry(&nscCtx->RoutingTable.RoutingEntries[0], NULL);

        if (NULL != rt_new)
        {
            rt_new->EntryType = Aid_Based;

            /* Copy AID data. */
            (void)memcpy(rt_new->Entry.AidRouting.Aid, aid, lengthOfAid);
            rt_new->Entry.AidRouting.LengthOfAid = lengthOfAid;
            rt_new->Entry.AidRouting.Target = target;
            rt_new->Entry.AidRouting.Qualifier = qualifier;
            rt_new->Entry.AidRouting.PowerState = powerState;
            
            /* Adjust matching bits */
            rt_new->Entry.AidRouting.MatchingType = (uint8_t)(aidMatchType & PTX_NSC_RFSETROUTINGTABLE_AID_MATCH_BITS);

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            /* Routing table size limit has been reached. */
            status = PTX_STATUS_E_NSC_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_AddEntryTech(ptx_NSC_t *nscCtx, ptx_NSC_RfTech_t tech,
                                                    ptx_NSC_RoutingTarget_t target, uint8_t qualifier, uint8_t powerState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC))
    {
        PTX_LOG_I(nscCtx->Log, "ptx_NSC_RfSetRoutingTable_AddEntryTech");

        /* Get the first unused entry from the list. If all are used, NULL is returned. */
        ptx_NSC_RoutingList_t *rt_new = ptx_NSC_RfSetRoutingTable_GetUnusedEntry(&nscCtx->RoutingTable.RoutingEntries[0], NULL);

        if (NULL != rt_new)
        {
            rt_new->EntryType = Technology_Based;
            rt_new->Entry.TechRouting.Tech = tech;
            rt_new->Entry.TechRouting.Target = target;
            rt_new->Entry.TechRouting.Qualifier = qualifier;
            rt_new->Entry.TechRouting.PowerState = powerState;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            /* Routing table size limit has been reached. */
            status = PTX_STATUS_E_NSC_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_AddEntryProtocol(ptx_NSC_t *nscCtx,  ptx_NSC_RfProtocol_t protocol,
                                                        ptx_NSC_RoutingTarget_t target, uint8_t qualifier, uint8_t powerState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC))
    {
        PTX_LOG_I(nscCtx->Log, "ptx_NSC_RfSetRoutingTable_AddEntryProtocol");

        /* Get the first unused entry from the list. If all are used, NULL is returned. */
        ptx_NSC_RoutingList_t *rt_new = ptx_NSC_RfSetRoutingTable_GetUnusedEntry(&nscCtx->RoutingTable.RoutingEntries[0], NULL);

        if (NULL != rt_new)
        {
            rt_new->EntryType = Protocol_Based;
            rt_new->Entry.ProtocolRouting.Target = target;
            rt_new->Entry.ProtocolRouting.Protocol = protocol;
            rt_new->Entry.ProtocolRouting.Qualifier = qualifier;
            rt_new->Entry.ProtocolRouting.PowerState = powerState;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            /* Routing table size limit has been reached. */
            status = PTX_STATUS_E_NSC_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_AddEntrySystemCode(ptx_NSC_t *nscCtx, uint8_t nrSCEntries, uint8_t *scEntries,
                                                          ptx_NSC_RoutingTarget_t target, uint8_t qualifier, uint8_t powerState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;

    /* NOT SUPPORTED */
    PTX_UNUSED_PARAMETER(nscCtx);
    PTX_UNUSED_PARAMETER(nrSCEntries);
    PTX_UNUSED_PARAMETER(scEntries);
    PTX_UNUSED_PARAMETER(target);
    PTX_UNUSED_PARAMETER(qualifier);
    PTX_UNUSED_PARAMETER(powerState);

    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_AddEntryAPDU(ptx_NSC_t *nscCtx, uint32_t apduAndMaskDataLength, uint8_t *apduAndMaskData,
                                                    ptx_NSC_RoutingTarget_t target, uint8_t qualifier, uint8_t powerState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;

    /* NOT SUPPORTED */
    PTX_UNUSED_PARAMETER(nscCtx);
    PTX_UNUSED_PARAMETER(apduAndMaskDataLength);
    PTX_UNUSED_PARAMETER(apduAndMaskData);
    PTX_UNUSED_PARAMETER(target);
    PTX_UNUSED_PARAMETER(qualifier);
    PTX_UNUSED_PARAMETER(powerState);

    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_ClearEntries(ptx_NSC_t *nscCtx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC))
    {
        PTX_LOG_I(nscCtx->Log, "ptx_NSC_RfSetRoutingTable_ClearEntries");

        ptx_NSC_RfSetRoutingTable_DeinitList(nscCtx);
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_GetNumOfEntries(ptx_NSC_t *nscCtx, uint8_t *numOfItems)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC) && (NULL != numOfItems))
    {
        PTX_LOG_I(nscCtx->Log, "ptx_NSC_RfSetRoutingTable_GetEntryStart");

        /* To get the number of used entry we need to provide valid numOfItems argument. Returned value is irrelevant. */
        (void) ptx_NSC_RfSetRoutingTable_GetUnusedEntry (nscCtx->RoutingTable.RoutingEntries, numOfItems);
        st = PTX_STATUS_SUCCESS;
    }

    return st;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_GetEntry(ptx_NSC_t *nscCtx, uint8_t entryType, ptx_NSC_RoutingList_t *rtEntry, uint8_t *itemIdx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC) && (NULL != rtEntry) && (NULL != itemIdx))
    {
        PTX_LOG_I(nscCtx->Log, "ptx_NSC_RfSetRoutingTable_GetEntry");

        uint8_t item_found = 0;

        /* Iterate starting from the given index and find the first entry with the provided type. */
        for (uint8_t i = *itemIdx; i < PTX_NSC_RT_USER_ENTRIES_CAPACITY; i++)
        {
            item_found = 0;
            if (MaxNumOfTypes == nscCtx->RoutingTable.RoutingEntries[i].EntryType)
            {
                /* Unused entry. Skip. */
                continue;
            }

            if (PTX_NSC_ROUTINGENTRY_ALL_TYPES == entryType)
            {
                /* Just take the first used entry. The type does not matter. */
                item_found = 1;
            }
            else if (entryType == (uint8_t)nscCtx->RoutingTable.RoutingEntries[i].EntryType)
            {
                /* Exact entry type found. */
                item_found = 1;
            }
            else
            {
                /* Just continue searching for a used entry. */
            }

            if(1 == item_found)
            {
                /* Entry type found. */
                memcpy(rtEntry, &nscCtx->RoutingTable.RoutingEntries[i], sizeof(ptx_NSC_RoutingList_t));
                *itemIdx = i;
                st = PTX_STATUS_SUCCESS;
                break;
            }
        }
    }

    return st;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_EnableForcedRouting(ptx_NSC_t *nscCtx, uint8_t mode, ptx_NSC_ForcedRoutingTarget_t target, uint8_t powerState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC))
    {
        PTX_LOG_I(nscCtx->Log, "ptx_NSC_RfSetRoutingTable_EnableForcedRouting");

        nscCtx->RoutingTable.ForcedRoutingInfo.ForcedRoutingEnabled = mode;
        nscCtx->RoutingTable.ForcedRoutingInfo.Target = target;
        nscCtx->RoutingTable.ForcedRoutingInfo.PowerState = powerState;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable(ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_RfSetRoutingTable");

        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_RfSetRoutingTable, NULL);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RoutingTable_CheckSize(&nsc->RoutingTable);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t nsc_bufer[PTX_NSC_RFSETROUTINGTABLE_BUFFER_MAX_LEN];
            memset(nsc_bufer, 0, PTX_NSC_RFSETROUTINGTABLE_BUFFER_MAX_LEN);
            uint32_t nsc_buffer_pos = 0;

            /* Add command opcode. */
            nsc_bufer[nsc_buffer_pos] = PTX_NSC_RF_SET_ROUTING_TABLE_CMD_OPCODE;
            nsc_buffer_pos++;

            /* Skip RT Length: it will be added after all entries are added. Here just account 1 byte for RT_LENGTH. */
            nsc_buffer_pos++;

            /* Add pID value */
            nsc_bufer[nsc_buffer_pos] = PTX_NSC_RFSETROUTINGTABLE_CMD_PID_VALUE;
            nsc_buffer_pos++;

            for(uint8_t route_priority = (uint8_t) NSC_RTPriority_Aid; route_priority < (uint8_t) NSC_RTPriority_SIZE; route_priority++)
            {
                /* Load all entries with the given priority into the command buffer. */
                ptx_NSC_RfSetRoutingTable_LoadEntries(nsc->RoutingTable.RoutingEntries, route_priority, nsc_bufer, &nsc_buffer_pos);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Append RT Length: total number of bytes - (opcode length + RT_LENGTH parameter length) */
                nsc_bufer[PTX_NSC_RFSETROUTINGTABLE_CMD_RT_LENGTH_INDEX] = (uint8_t)(nsc_buffer_pos-2);
                status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, nsc_bufer, nsc_buffer_pos);
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t rsp_buffer[PTX_NSC_CMD_PROCESSOR_RFSETROUTINGTABLE_RSPLEN];
            uint32_t rsp_buffer_len = PTX_NSC_CMD_PROCESSOR_RFSETROUTINGTABLE_RSPLEN;
            memset(rsp_buffer, 0, sizeof(uint8_t)*PTX_NSC_CMD_PROCESSOR_RFSETROUTINGTABLE_RSPLEN);

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_buffer, &rsp_buffer_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_RFSETROUTINGTABLE_RSP);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_RfSetRoutingTbl, rsp_buffer, rsp_buffer_len, NULL);
            }
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_NSC_RoutingList_t *ptx_NSC_RfSetRoutingTable_GetUnusedEntry (ptx_NSC_RoutingList_t *rtList, uint8_t *usedEntriesCount)
{
    /**
     * There are few options to use this function:
     * 1) To get the first unused entry. Parameter usedEntriesCount can be NULL.
     * 2) To get the number of used entries in the list. Parameter usedEntriesCount must not be NULL.
     * Or a mix of the 2 options: all parameters must be valid then.
     */
    
    ptx_NSC_RoutingList_t *free_entry = NULL;
    uint8_t used_entries_cnt = 0;

    if (NULL != rtList)
    {
        /* Iterate and find a space the first unused entry. */
        for (uint8_t idx = 0; idx < PTX_NSC_RT_USER_ENTRIES_CAPACITY; idx++)
        {
            if (MaxNumOfTypes == rtList[idx].EntryType)
            {
                free_entry = &rtList[idx];
                break;
            }
            
            used_entries_cnt++;
        }
    }

    if(NULL != usedEntriesCount)
    {
        /* Update the argument if provided. */
        *usedEntriesCount = used_entries_cnt;
    }

    return free_entry;
}

static void ptx_NSC_RfSetRoutingTable_DeinitList(ptx_NSC_t *nscCtx)
{
    if (PTX_COMP_CHECK(nscCtx, Comp_NSC))
    {
        PTX_LOG_V(nscCtx->Log, "ptx_NSC_RfSetRoutingTable_DeinitList");

        /* Reset the entry list */
        (void)memset(&nscCtx->RoutingTable.RoutingEntries[0], 0, PTX_NSC_RT_USER_ENTRIES_CAPACITY * sizeof(ptx_NSC_RoutingList_t));

        for (uint32_t idx = 0; idx < PTX_NSC_RT_USER_ENTRIES_CAPACITY; idx++)
        {
            /* Set to invalid entry type */
            nscCtx->RoutingTable.RoutingEntries[idx].EntryType = MaxNumOfTypes;
        }
    }
}

static ptx_Status_t ptx_NSC_RoutingTable_CheckSize(ptx_NSC_RoutingTable_t *routingTable)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;

    /* Check if all the entries can fit into the command buffer. If yes, return SUCCESS.
     * Otherwise, keep the error.
     */

    uint32_t cmd_length = PTX_NSC_RFSETROUTINGTABLE_CMD_OVH_LENGTH;

    /* Iterate and find a space the first unused entry. */
    for (uint32_t idx = 0; idx < PTX_NSC_RT_USER_ENTRIES_CAPACITY; idx++)
    {
        if (Technology_Based == routingTable->RoutingEntries[idx].EntryType)
        {
            cmd_length = (uint32_t)(cmd_length + PTX_NSC_ROUTINGENTRY_SIZE_TECH);
        }
        else if (Protocol_Based == routingTable->RoutingEntries[idx].EntryType)
        {
            cmd_length = (uint32_t)(cmd_length + PTX_NSC_ROUTINGENTRY_SIZE_PROT);
        }
        else if (Aid_Based == routingTable->RoutingEntries[idx].EntryType)
        {
            cmd_length = (uint32_t)(cmd_length + PTX_NSC_ROUTINGENTRY_SIZE_AID_FIXED);
            /* Add variable part: AID length */
            cmd_length = (uint32_t)(cmd_length + routingTable->RoutingEntries[idx].Entry.AidRouting.LengthOfAid);
        }
        else
        {
            /* Just continue */
        }
    }

    if ( PTX_NSC_RFSETROUTINGTABLE_BUFFER_MAX_LEN >= cmd_length)
    {
        
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static void ptx_NSC_RfSetRoutingTable_LoadEntries(ptx_NSC_RoutingList_t *rtList, uint8_t routetPrio, uint8_t *buffer, uint32_t *buffer_offset)
{
    /*
     * Load all the entries of the given priority from the list into the buffer.
     */

    uint32_t pos = *buffer_offset;

    /* Map priority to the entry type. */
    ptx_NSC_RoutingType_t entry_type = MaxNumOfTypes;
    switch((ptx_NSC_RTPriority_t)routetPrio)
    {
        case NSC_RTPriority_Aid:
            entry_type = Aid_Based;
            break;
        case NSC_RTPriority_Prot:
            entry_type = Protocol_Based;
            break;
        case NSC_RTPriority_Tech:
            entry_type = Technology_Based;
            break;
        default:
            /* All others are not supported. */
            break;
    }

    for (uint8_t idx = 0; (MaxNumOfTypes != entry_type) && (idx < PTX_NSC_RT_USER_ENTRIES_CAPACITY); idx++)
    {
        if (entry_type == rtList[idx].EntryType)
        {
            /* Load the entry into the buffer. */
            buffer[pos] = (uint8_t)entry_type;
            pos++;

            switch (entry_type)
            {
                case Technology_Based:
                    buffer[pos] = (uint8_t)rtList[idx].Entry.TechRouting.Tech;
                    pos++;
                    buffer[pos] = (uint8_t)rtList[idx].Entry.TechRouting.Target;
                    pos++;
                    break;

                case Protocol_Based:
                    buffer[pos] = (uint8_t)rtList[idx].Entry.ProtocolRouting.Protocol;
                    pos++;
                    buffer[pos] = (uint8_t)rtList[idx].Entry.ProtocolRouting.Target;
                    pos++;
                    break;

                case Aid_Based:
                    /* For AID entry type, we need to add the matching. */
                    buffer[pos-1] = (uint8_t)(buffer[pos-1] | rtList[idx].Entry.AidRouting.MatchingType);
                    buffer[pos] = (uint8_t)rtList[idx].Entry.AidRouting.LengthOfAid;
                    pos++;
                    (void)memcpy(&buffer[pos], rtList[idx].Entry.AidRouting.Aid, rtList[idx].Entry.AidRouting.LengthOfAid);
                    pos += (uint8_t)rtList[idx].Entry.AidRouting.LengthOfAid;
                    buffer[pos] = (uint8_t)rtList[idx].Entry.AidRouting.Target;
                    pos++;
                    break;

                default:
                    /* Skip unused entry. */
                    break;
            }
        }
    }

    *buffer_offset = pos;
}

