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
    Module      : NFC Soft Controller (NSC) Interface for State Machine
    File        : ptx_NSC_StateMachine.c

    Description : Implementation of NFC Soft Controller Interface for State Machine.
                  Contains the function(s) to check if requested state transition is allowed or not.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Int.h"
#include "ptx_NSC_StateMachine.h"


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * NSC API State checking table: sets RF_STATES in which an API is allowed to execute. Defined by NSC specification.
 */
static const ptx_NSC_SM_CmdCheckItem_t ptx_NSC_SM_CmdCheckTable[] =
{
    { (uint8_t)ptx_NSC_SM_CmdId_Init,           (uint16_t)(PTX_NSC_SM_RFST_INIT) },
    { (uint8_t)ptx_NSC_SM_CmdId_Read,           (uint16_t)(PTX_NSC_SM_RFST_ALL_IDLE) },
    { (uint8_t)ptx_NSC_SM_CmdId_Write,          (uint16_t)(PTX_NSC_SM_RFST_ALL_IDLE) },
    { (uint8_t)ptx_NSC_SM_CmdId_RfConfig,       (uint16_t)(PTX_NSC_SM_RFST_IDLE) },
    { (uint8_t)ptx_NSC_SM_CmdId_RfDiscover,     (uint16_t)(PTX_NSC_SM_RFST_IDLE) },

    {
        (uint8_t)ptx_NSC_SM_CmdId_RfDeactivate,   (uint16_t)(PTX_NSC_SM_RFST_DISCOVERY + PTX_NSC_SM_RFST_POLL + PTX_NSC_SM_RFST_POLL_DEP +
                                                             PTX_NSC_SM_RFST_POLL_TEST + PTX_NSC_SM_RFST_LISTEN + PTX_NSC_SM_RFST_LISTEN_DEP +
                                                             PTX_NSC_SM_RFST_LISTEN_SLEEP + PTX_NSC_SM_RFST_LISTEN_IDLE)
    },

    { (uint8_t)ptx_NSC_SM_CmdId_RfActivate,     (uint16_t)(PTX_NSC_SM_RFST_POLL) },
    { (uint8_t)ptx_NSC_SM_CmdId_RfSetParam,     (uint16_t)(PTX_NSC_SM_RFST_POLL) },
    { (uint8_t)ptx_NSC_SM_CmdId_RfTest,         (uint16_t)(PTX_NSC_SM_RFST_POLL) },
    { (uint8_t)ptx_NSC_SM_CmdId_WptStart,       (uint16_t)(PTX_NSC_SM_RFST_POLL + PTX_NSC_SM_RFST_POLL_DEP) },
    { (uint8_t)ptx_NSC_SM_CmdId_WptStop,        (uint16_t)(PTX_NSC_SM_RFST_WPT + PTX_NSC_SM_RFST_WPT_DEP) },

    {
        (uint8_t)ptx_NSC_SM_CmdId_Data,           (uint16_t)(PTX_NSC_SM_RFST_POLL + PTX_NSC_SM_RFST_POLL_DEP + PTX_NSC_SM_RFST_LISTEN +
                                                             PTX_NSC_SM_RFST_LISTEN_DEP)
    },
    { (uint8_t)ptx_NSC_SM_CmdId_Standby,        (uint16_t)(PTX_NSC_SM_RFST_IDLE) },
    { (uint8_t)ptx_NSC_SM_CmdId_Wakeup,         (uint16_t)(PTX_NSC_SM_RFST_STANDBY) },
    { (uint8_t)ptx_NSC_SM_CmdId_Loopback,       (uint16_t)(PTX_NSC_SM_RFST_ALL_IDLE) },
    { (uint8_t)ptx_NSC_SM_CmdId_GetStatus,      (uint16_t)(PTX_NSC_SM_RFST_ALL_IDLE) },
    { (uint8_t)ptx_NSC_SM_CmdId_RfSetRoutingTable,    (uint16_t)(PTX_NSC_SM_RFST_ALL_IDLE) },
};


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static uint8_t ptx_NSC_SM_IsCmdAllowed(ptx_NSC_t *nsc, ptx_NSC_SM_CmdId_t cmdId, uint32_t *cmdTableIdx);
static ptx_Status_t ptx_NSC_SM_CheckParams(uint32_t cmdTableIdx, ptx_NSC_State_t state, uint32_t *cmdParams);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_CheckRfStateCmdExecution(ptx_NSC_t *nsc, ptx_NSC_SM_CmdId_t cmdId, uint32_t *cmdParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        if (ptx_NSC_SM_CmdId_Undefined > cmdId)
        {
            const uint32_t invalid_table_index = 0xFFFFu;
            uint32_t cmd_table_index = invalid_table_index;

            uint8_t cmd_allowed = ptx_NSC_SM_IsCmdAllowed(nsc, cmdId, &cmd_table_index);
            if ((0 != cmd_allowed) && (invalid_table_index != cmd_table_index))
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_STATE;
            }

            /* The state was checked. Now check the parameters if any. */
            if ((PTX_STATUS_SUCCESS == status) && (NULL != cmdParams))
            {
                status = ptx_NSC_SM_CheckParams(cmd_table_index, nsc->State, cmdParams);
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
static ptx_Status_t ptx_NSC_SM_CheckParams(uint32_t cmdTableIdx, ptx_NSC_State_t state, uint32_t *cmdParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((uint8_t)ptx_NSC_SM_CmdId_RfDeactivate == ptx_NSC_SM_CmdCheckTable[cmdTableIdx].CmdId)
    {
        /* Additional parameter check required for RfDeactivate command. */
        if (PTX_NSC_SM_DEACT_PARAMLEN == cmdParams[PTX_NSC_SM_DEACT_LEN_IDX])
        {
            ptx_NSC_RfDeact_Type_t type = (ptx_NSC_RfDeact_Type_t)cmdParams[1];

            if (NSC_RfDeact_Type_ProtSpecific == type)
            {
                if (ptx_NSC_State_Poll_Dep == state)
                {
                    status = PTX_STATUS_SUCCESS;
                }
            }
            else
            {
                status = PTX_STATUS_SUCCESS;
            }
        }

        if ((PTX_STATUS_SUCCESS == status) && (NSC_RfDeact_State_Poll == (ptx_NSC_RfDeact_State_t)cmdParams[PTX_NSC_SM_DEACT_STATE_IDX]))
        {
            /* Deactivation to RF_POLL state is allowed only from any of the POLL states. */
            if ((ptx_NSC_State_Poll_Dep != state) && (ptx_NSC_State_Poll_Test != state) && (ptx_NSC_State_Poll != state))
            {
                status = PTX_STATUS_E_GEN_INVALID_STATE;
            }
        }
        else if ((PTX_STATUS_SUCCESS == status) && ( (NSC_RfDeact_State_ListenIdle == (ptx_NSC_RfDeact_State_t)cmdParams[PTX_NSC_SM_DEACT_STATE_IDX]) ||
                                                     (NSC_RfDeact_State_ListenSleep == (ptx_NSC_RfDeact_State_t)cmdParams[PTX_NSC_SM_DEACT_STATE_IDX]) )    )
        {
            /* Deactivation to RF_LISTEN_IDLE/SLEEP states is allowed only from RF_LISTEN state. */
            if (ptx_NSC_State_Listen != state)
            {
                status = PTX_STATUS_E_GEN_INVALID_STATE;
            }
        }
        else
        {
            /* No special handling for other cases, or error already set. */
        }

    }

    return status;
}

static uint8_t ptx_NSC_SM_IsCmdAllowed(ptx_NSC_t *nsc, ptx_NSC_SM_CmdId_t cmdId, uint32_t *cmdTableIdx)
{
    uint8_t is_allowed = 0;
    uint32_t i = 0;
    uint8_t cmd_id = (uint8_t)cmdId;
    uint32_t check_table_size = sizeof(ptx_NSC_SM_CmdCheckTable) / sizeof(ptx_NSC_SM_CmdCheckItem_t);
    uint16_t rf_state = PTX_NSC_SM_BITMASK(nsc->State);

    while (i < check_table_size)
    {
        if (ptx_NSC_SM_CmdCheckTable[i].CmdId == cmd_id)
        {
            /* Command is found... */
            if (0 != (rf_state & ptx_NSC_SM_CmdCheckTable[i].StateBitMask))
            {
                /* and is allowed */
                is_allowed = 1u;
                *cmdTableIdx = i;
            }
            else
            {
                /* and is not allowed => wrong state!!! */
            }

            /* Stop surfing through the items */
            break;
        }

        i++;
    }

    return is_allowed;
}

