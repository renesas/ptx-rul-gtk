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
#include <string.h>
#include <stdlib.h>
#include "ptx_NSC_RfSetRoutingTable_Int.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/* Define the ordering of RoutingType */
typedef enum ptx_NSC_RoutingTypeOrder
{
    PTX_NSC_ROUTING_TYPE_ORDER_AID = 0,
    PTX_NSC_ROUTING_TYPE_ORDER_PROTOCOL,
    PTX_NSC_ROUTING_TYPE_ORDER_TECHNOLOGY,
    PTX_NSC_ROUTING_TYPE_ORDER_MAX
} ptx_NSC_RoutingTypeOrder_t;

/* Mapping of RoutingTypes to their order */
static const ptx_NSC_RoutingTypeOrder_t sRoutingTypeOrderMap[] =
{
    [Aid_Based] = PTX_NSC_ROUTING_TYPE_ORDER_AID,
    [Protocol_Based] = PTX_NSC_ROUTING_TYPE_ORDER_PROTOCOL,
    [Technology_Based] = PTX_NSC_ROUTING_TYPE_ORDER_TECHNOLOGY,
};

/*
 * Definitions for NSC_RFSETROUTINGTABLE_CMD.
 */
#define PTX_NSC_RFSETROUTINGTABLE_CMD_OPCODE_LENGTH         (0x01u)     /* NSC RF Set Routing Table Opcode length */
#define PTX_NSC_RFSETROUTINGTABLE_CMD_RT_LENGTH             (0x01u)     /* NSC RF Set Routing Table pID length */
#define PTX_NSC_RFSETROUTINGTABLE_CMD_PID_LENGTH            (0x01u)     /* NSC RF Set Routing Table pID length */

#define PTX_NSC_RFSETROUTINGTABLE_CMD_OVH_LENGTH            PTX_NSC_RFSETROUTINGTABLE_CMD_OPCODE_LENGTH + \
    PTX_NSC_RFSETROUTINGTABLE_CMD_PID_LENGTH + \
    PTX_NSC_RFSETROUTINGTABLE_CMD_RT_LENGTH

#define PTX_NSC_RFSETROUTINGTABLE_BUFFER_MAX_LEN            PTX_NSC_LENGTH_MAX      /* RF Config Buffer maximum size */

/* Variables to define the maximum capacity of the routing table and reserve the memory */
static ptx_NSC_RoutingList_t sRtListCapacity[PTX_NSC_RT_USER_ENTRIES_CAPACITY];
static uint8_t sAidData[PTX_NSC_RT_USER_ENTRIES_CAPACITY][PTX_NSC_RT_AID_MAX_LEN];

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_RfSetRoutingTable_Add_NewEntry (ptx_NSC_t *nscCtx, ptx_NSC_RoutingList_t *rtNew);
static ptx_Status_t ptx_NSC_RfSetRoutingTable_Get_Memory(ptx_NSC_t *nscCtx, ptx_NSC_RoutingList_t **rtNew);
static void ptx_NSC_RfSetRoutingTable_DeinitList(ptx_NSC_t *nscCtx);

static ptx_Status_t ptx_NSC_RoutingTable_CheckNscLength(ptx_NSC_t *nsc);
static ptx_Status_t ptx_NSC_RoutingTable_AddToCmd(ptx_NSC_t *nsc, uint8_t *buffer, uint32_t *buffer_offset);
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
        (void)memset(&(nscCtx->RoutingTable), 0, sizeof(ptx_NSC_RoutingTable_t));

        (void)memset(&sRtListCapacity, 0, sizeof(ptx_NSC_RoutingList_t)*PTX_NSC_RT_USER_ENTRIES_CAPACITY);
        /* initialize start to linked list to NULL*/
        nscCtx->RoutingTable.RoutingEntries = NULL;

        (void)memset(&sAidData, 0, PTX_NSC_RT_AID_MAX_LEN * PTX_NSC_RT_USER_ENTRIES_CAPACITY);

        for (uint32_t idx = 0; idx < PTX_NSC_RT_USER_ENTRIES_CAPACITY; idx++)
        {
            sRtListCapacity[idx].Entry.AidRouting.Aid = &sAidData[idx][0];
            sRtListCapacity[idx].EntryType = MaxNumOfTypes;
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
    /* First in search order -> put it first in table */
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    const uint32_t max_lenthOfAid = 256;
    uint32_t aid_entry_size = 2 + lengthOfAid;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC) && (NULL != aid) && (lengthOfAid <= max_lenthOfAid ))
    {
        if ((nscCtx->RoutingTable.BytesUsed  + aid_entry_size) <= (uint32_t)PTX_NSC_RT_CAPACITY)
        {
            ptx_NSC_RoutingList_t *rt_new = NULL;
            if (PTX_NSC_RT_USER_ENTRIES_CAPACITY > nscCtx->RoutingTable.NumOfEntries)
            {
                /* there is room left in the table */
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                /* Specified Routing Size is too small, see PTX_NSC_RT_USER_ENTRIES_CAPACITY */
                status = PTX_STATUS_E_NSC_INSUFFICIENT_RESOURCES;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* at this point we know that there is still room for an entry */
                status = ptx_NSC_RfSetRoutingTable_Get_Memory(nscCtx, &rt_new);

                if (PTX_STATUS_SUCCESS == status && NULL != rt_new)
                {
                    rt_new->EntryType = (ptx_NSC_RoutingType_t) (Aid_Based | aidMatchType);
                    (void)memcpy(rt_new->Entry.AidRouting.Aid, aid, lengthOfAid);
                    rt_new->Entry.AidRouting.LengthOfAid = lengthOfAid;
                    rt_new->Entry.AidRouting.Target = target;
                    rt_new->Entry.AidRouting.Qualifier = qualifier;
                    rt_new->Entry.AidRouting.PowerState = powerState;

                    /* add it to the linked list */
                    status = ptx_NSC_RfSetRoutingTable_Add_NewEntry (nscCtx, rt_new);
                    nscCtx->RoutingTable.BytesUsed += aid_entry_size;
                }
            }
        }
        else
        {
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
        if (nscCtx->RoutingTable.BytesUsed + PTX_NSC_RT_TECH_ENTRY_SIZE <= (uint32_t)PTX_NSC_RT_CAPACITY)
        {
            ptx_NSC_RoutingList_t *rt_new = NULL;
            if (PTX_NSC_RT_USER_ENTRIES_CAPACITY > nscCtx->RoutingTable.NumOfEntries)
            {
                /* there is room left in the table */
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                /* Specified Routing Size is too small, see PTX_NSC_RT_USER_ENTRIES_CAPACITY */
                status = PTX_STATUS_E_NSC_INSUFFICIENT_RESOURCES;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* at this point we know that there is still room for an entry */
                status = ptx_NSC_RfSetRoutingTable_Get_Memory(nscCtx, &rt_new);

                if (PTX_STATUS_SUCCESS == status && NULL != rt_new)
                {
                    rt_new->EntryType = Technology_Based;
                    rt_new->Entry.TechRouting.Tech = tech;
                    rt_new->Entry.TechRouting.Target = target;
                    rt_new->Entry.TechRouting.Qualifier = qualifier;
                    rt_new->Entry.TechRouting.PowerState = powerState;

                    /* add it to the linked list */
                    status = ptx_NSC_RfSetRoutingTable_Add_NewEntry (nscCtx, rt_new);
                    nscCtx->RoutingTable.BytesUsed += PTX_NSC_RT_TECH_ENTRY_SIZE;
                }
            }
        }
        else
        {
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
        if (nscCtx->RoutingTable.BytesUsed + PTX_NSC_RT_PROT_ENTRY_SIZE <= (uint32_t)PTX_NSC_RT_CAPACITY)
        {
            ptx_NSC_RoutingList_t *rt_new = NULL;
            if (PTX_NSC_RT_USER_ENTRIES_CAPACITY > nscCtx->RoutingTable.NumOfEntries)
            {
                /* there is room left in the table */
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                /* Specified Routing Size is too small, see PTX_NSC_RT_USER_ENTRIES_CAPACITY */
                status = PTX_STATUS_E_NSC_INSUFFICIENT_RESOURCES;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* at this point we know that there is still room for an entry */
                status = ptx_NSC_RfSetRoutingTable_Get_Memory(nscCtx, &rt_new);


                if (PTX_STATUS_SUCCESS == status)
                {
                    rt_new->EntryType = Protocol_Based;
                    rt_new->Entry.ProtocolRouting.Target = target;
                    rt_new->Entry.ProtocolRouting.Protocol = protocol;
                    rt_new->Entry.ProtocolRouting.Qualifier = qualifier;
                    rt_new->Entry.ProtocolRouting.PowerState = powerState;

                    /* add it to the linked list */
                    status = ptx_NSC_RfSetRoutingTable_Add_NewEntry (nscCtx, rt_new);
                    nscCtx->RoutingTable.BytesUsed += PTX_NSC_RT_PROT_ENTRY_SIZE;
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
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
        ptx_NSC_RfSetRoutingTable_DeinitList(nscCtx);
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_GetEntryStart(ptx_NSC_t *nscCtx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC))
    {
        nscCtx->RoutingTable.ReadNumOfEntries = 0;
        status = PTX_STATUS_SUCCESS;
    }
    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_GetEntry(ptx_NSC_t *nscCtx, ptx_NSC_RoutingEntry_t *entry, ptx_NSC_RoutingType_t *entryType)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC) && (NULL != nscCtx->RoutingTable.RoutingEntries) && (NULL != entry) && (NULL != entryType))
    {
        status = PTX_STATUS_SUCCESS;

        if (0 == nscCtx->RoutingTable.ReadNumOfEntries)
        {
            /* First item to read */
            nscCtx->RoutingTable.ReadRoutingEntry = nscCtx->RoutingTable.RoutingEntries;
        }
        else
        {
            if (NULL == nscCtx->RoutingTable.ReadRoutingEntry )
            {
                /* Last entry reached in the previous loop. Not current entry available. */
                status = PTX_STATUS_E_GEN_NOT_FOUND;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Copy values to the provided buffers by the caller. */
            (void)memcpy(entry, &nscCtx->RoutingTable.ReadRoutingEntry->Entry, sizeof(ptx_NSC_RoutingEntry_t));
            *entryType = nscCtx->RoutingTable.ReadRoutingEntry->EntryType;

            nscCtx->RoutingTable.ReadNumOfEntries++;

            /* Jump to following entry in the list. This will be used by the following request. */
            nscCtx->RoutingTable.ReadRoutingEntry = nscCtx->RoutingTable.ReadRoutingEntry->NextEntry;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_GetNumOfEntries(ptx_NSC_t *nscCtx, uint32_t *numOfItems)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC) && (NULL != numOfItems))
    {
        *numOfItems = nscCtx->RoutingTable.NumOfEntries;
        st = PTX_STATUS_SUCCESS;
    }

    return st;
}

ptx_Status_t ptx_NSC_RfSetRoutingTable_EnableForcedRouting(ptx_NSC_t *nscCtx, uint8_t mode, ptx_NSC_ForcedRoutingTarget_t target, uint8_t powerState)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC))
    {
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

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) )
    {
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_RfSetRoutingTable, NULL);

        if (PTX_STATUS_SUCCESS == status)
        {

            status = ptx_NSC_RoutingTable_CheckNscLength(nsc);

            if (PTX_STATUS_SUCCESS == status)
            {
                uint8_t nsc_bufer[PTX_NSC_RFSETROUTINGTABLE_BUFFER_MAX_LEN];
                memset(nsc_bufer, 0, PTX_NSC_RFSETROUTINGTABLE_BUFFER_MAX_LEN);
                uint32_t nsc_buffer_pos = 0;

                /* Append RF-Set Routing Table command opcode. */
                nsc_bufer[nsc_buffer_pos] = (uint8_t)PTX_NSC_RF_SET_ROUTING_TABLE_CMD_OPCODE;
                nsc_buffer_pos++;

                /* Append RT Length */
                nsc_bufer[nsc_buffer_pos] = (uint8_t)nsc->RoutingTable.NumOfEntries;
                nsc_buffer_pos++;

                /* Append pID value */
                nsc_bufer[nsc_buffer_pos] = 0x01u;
                nsc_buffer_pos++;

                status = ptx_NSC_RoutingTable_AddToCmd(nsc, nsc_bufer, &nsc_buffer_pos);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, nsc_bufer, nsc_buffer_pos);
                }
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

static ptx_Status_t ptx_NSC_RfSetRoutingTable_Add_NewEntry (ptx_NSC_t *nscCtx, ptx_NSC_RoutingList_t *rtNew)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC) && (NULL != rtNew))
    {
        /* Determine the order rank of the new entry's RoutingType */
        ptx_NSC_RoutingTypeOrder_t rt_new_order_rank = sRoutingTypeOrderMap[rtNew->EntryType];
        ptx_NSC_RoutingList_t *rt_head = nscCtx->RoutingTable.RoutingEntries;
        ptx_NSC_RoutingTypeOrder_t rt_head_rank = NULL != rt_head ? sRoutingTypeOrderMap[rt_head->EntryType] : PTX_NSC_ROUTING_TYPE_ORDER_MAX;

        /* If the list is empty or the new entry should be the new head, update the head */
        if ((rt_head == NULL) || (rt_head_rank > rt_new_order_rank))
        {
            rtNew->NextEntry = rt_head;
            nscCtx->RoutingTable.RoutingEntries = rtNew;
            nscCtx->RoutingTable.NumOfEntries++;
            status = PTX_STATUS_SUCCESS;
        }
        else if ((rt_head != NULL) && (rt_head_rank <= rt_new_order_rank))
        {
            /* Traverse the list to find the correct position for the new entry */
            ptx_NSC_RoutingList_t *current = rt_head;
            while (current->NextEntry != NULL && sRoutingTypeOrderMap[current->NextEntry->EntryType] <= rt_new_order_rank)
            {
                current = current->NextEntry;
            }

            /* Insert the new entry at the correct position */
            rtNew->NextEntry = current->NextEntry;
            current->NextEntry = rtNew;
            nscCtx->RoutingTable.NumOfEntries++;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_NSC_INTERNAL;
        }
    }
    return status;
}

static ptx_Status_t ptx_NSC_RfSetRoutingTable_Get_Memory(ptx_NSC_t *nscCtx, ptx_NSC_RoutingList_t **rtNew)
{
    ptx_Status_t status = PTX_STATUS_E_NSC_INSUFFICIENT_RESOURCES;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC))
    {
        // Iterate and find a space to place the new RT entry
        for (uint32_t idx = 0; idx < PTX_NSC_RT_USER_ENTRIES_CAPACITY; idx++)
        {
            if (sRtListCapacity[idx].EntryType == MaxNumOfTypes)
            {
                // Check for unused entries
                *rtNew = &sRtListCapacity[idx];
                status = PTX_STATUS_SUCCESS;
                break;
            }
        }
    }

    return status;
}

static void ptx_NSC_RfSetRoutingTable_DeinitList(ptx_NSC_t *nscCtx)
{
    if (PTX_COMP_CHECK(nscCtx, Comp_NSC))
    {
        nscCtx->RoutingTable.NumOfEntries = 0;
        nscCtx->RoutingTable.ReadNumOfEntries = 0;
        nscCtx->RoutingTable.RoutingEntries = NULL;
        nscCtx->RoutingTable.BytesUsed = 0;

        /* reset internal buffers */
        (void)memset(&sRtListCapacity, 0, sizeof(ptx_NSC_RoutingList_t)*PTX_NSC_RT_USER_ENTRIES_CAPACITY);
        (void)memset(&sAidData, 0, PTX_NSC_RT_AID_MAX_LEN * PTX_NSC_RT_USER_ENTRIES_CAPACITY);

        for (uint32_t idx = 0; idx < PTX_NSC_RT_USER_ENTRIES_CAPACITY; idx++)
        {
            sRtListCapacity[idx].Entry.AidRouting.Aid = &sAidData[idx][0];
            sRtListCapacity[idx].EntryType = MaxNumOfTypes;
        }
    }
}

static ptx_Status_t ptx_NSC_RoutingTable_CheckNscLength(ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t nscRtCmdLength = PTX_NSC_RFSETROUTINGTABLE_CMD_OVH_LENGTH + nsc->RoutingTable.BytesUsed;

    if ( PTX_NSC_RFSETROUTINGTABLE_BUFFER_MAX_LEN >= nscRtCmdLength)
    {
        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }

    return status;
}

static ptx_Status_t ptx_NSC_RoutingTable_AddToCmd(ptx_NSC_t *nsc, uint8_t *buffer, uint32_t *buffer_offset)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t idx = *buffer_offset;

    uint32_t tableEntries;
    uint32_t fetchedEntries = 0;
    uint32_t parsedTableEntries = 0;

    status = ptx_NSC_RfSetRoutingTable_GetNumOfEntries(nsc, &tableEntries);

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_NSC_RfSetRoutingTable_GetEntryStart(nsc);

        ptx_NSC_RoutingEntry_t  rt_entry;
        ptx_NSC_RoutingType_t   rt_entry_type;

        while ((PTX_STATUS_SUCCESS == status) && (fetchedEntries < tableEntries))
        {
            status = ptx_NSC_RfSetRoutingTable_GetEntry(nsc, &rt_entry, &rt_entry_type);

            fetchedEntries++;

            if (PTX_STATUS_SUCCESS == status)
            {
                buffer[idx] = (uint8_t)rt_entry_type;
                idx++;

                switch (rt_entry_type)
                {
                    case Technology_Based:
                        buffer[idx] = (uint8_t)rt_entry.TechRouting.Tech;
                        idx++;
                        buffer[idx] = (uint8_t)rt_entry.TechRouting.Target;
                        idx++;
                        break;

                    case Protocol_Based:
                        buffer[idx] = (uint8_t)rt_entry.ProtocolRouting.Protocol;
                        idx++;
                        buffer[idx] = (uint8_t)rt_entry.ProtocolRouting.Target;
                        idx++;
                        break;

                    case Aid_Based:
                        buffer[idx] = (uint8_t)rt_entry.AidRouting.LengthOfAid;
                        idx++;

                        (void)memcpy(&buffer[idx], rt_entry.AidRouting.Aid, rt_entry.AidRouting.LengthOfAid);
                        idx += (uint8_t)rt_entry.AidRouting.LengthOfAid;
                        break;

                    case Apdu_Based:
                    /* FALL THROUGH */
                    case System_Code_Based:
                    /* FALL THROUGH */
                    default:
                        break;
                }
                parsedTableEntries++;
            }
        }

        if (tableEntries == parsedTableEntries)
        {
            /* Return the correct status when all entries have been parsed */
            status = PTX_STATUS_SUCCESS;

            *buffer_offset = idx;
        }
    }

    return status;
}
