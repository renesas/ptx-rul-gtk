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
    Module      : ptx_RUL_HCE
    File        : ptx_RUL_HCE.c

    Description : Implementation of RUL_HCE
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NSC_Data.h"

#include "ptx_RUL_HCE.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */



/*
 * NSC DEACTIVATE-Reasons.
 *
 */
#define PTX_RUL_HCE_DEACTIVATE_FINISHED             (uint8_t)0x01       /* HCE Deactivate command finished */
#define PTX_RUL_HCE_DEACTIVATE_REASON_DESELECT      (uint8_t)0x02       /* HCE Deactivate reason: Deselect */
#define PTX_RUL_HCE_DEACTIVATE_REASON_RELEASE       (uint8_t)0x03       /* HCE Deactivate reason: Release */
#define PTX_RUL_HCE_DEACTIVATE_REASON_FIELD_OFF     (uint8_t)0x04       /* HCE Deactivate reason: Field off */
/*  */
/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_RUL_HCE_RegisterExtension(ptx_RUL_t *rul, ptx_RUL_HCE_t *hce);
static ptx_Status_t ptx_RUL_HCE_RemoveExtension(ptx_RUL_t *rul);


static ptx_Status_t ptx_RUL_HCE_ProcessDataMsg(void *ctx, ptx_NSC_RfData_Msg_t *msg);
static ptx_Status_t ptx_RUL_HCE_ProcessError(void *ctx, ptx_NSC_Error_Ntf_t *ntf, uint8_t *ntf_used);
static ptx_Status_t ptx_RUL_HCE_ProcessReset(void *ctx);
static ptx_Status_t ptx_RUL_HCE_ProcessRfActivate(void *ctx, ptx_NSC_RfActivate_Ntf_t *ntf, uint8_t *ntf_used);
static ptx_Status_t ptx_RUL_HCE_ProcessRfField(void *ctx, uint8_t field, uint8_t *ntf_used);
static ptx_Status_t ptx_RUL_HCE_ProcessRfDeactivate(void *ctx, ptx_NSC_RfDeactivate_Ntf_t *ntf, uint8_t *ntf_used);

/*
 * Read latest event.
 */
static ptx_Status_t ptx_RUL_HCE_ReadFromTail   ( ptx_RUL_HCE_t *hce, uint16_t *recordIndex );

/*
 * Read oldest event.
 */
static ptx_Status_t ptx_RUL_HCE_ReadFromHead   ( ptx_RUL_HCE_t *hce, uint16_t *recordIndex );

/*
 * Add new event to queue.
 */
static ptx_Status_t ptx_RUL_HCE_AddToTail      ( ptx_RUL_HCE_t *hce, uint16_t recordIndex );

/*
 * Remove oldest event from the queue.
 */
static ptx_Status_t ptx_RUL_HCE_RemoveFromHead ( ptx_RUL_HCE_t *hce, uint16_t *recordIndex );

/*
 * Read next event from queue.
 */
static ptx_Status_t ptx_RUL_HCE_ReadNextEvent  ( ptx_RUL_HCE_t *hce, ptx_RUL_HCE_EventRecord_t **evtRecord, uint16_t *indexOfRecordRead );

/*
 * Remove an event.
 */
static ptx_Status_t ptx_RUL_HCE_ReleaseEvent   ( ptx_RUL_HCE_t *hce, ptx_RUL_HCE_EventRecord_t *evtRecord, uint16_t recordIndex );

/*
 * Initialise event queue.
 */
static ptx_Status_t ptx_RUL_HCE_InitEventQueue ( ptx_RUL_HCE_t *hce );

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_RUL_HCE_Init(ptx_RUL_HCE_t *hce, ptx_RUL_HCE_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != hce) && (NULL != initParams))
    {
        hce->Rul                  = initParams->Rul;

        hce->AppRxBufferSize      = initParams->AppRxBufferSize;
        hce->AppRxBuffer          = initParams->AppRxBuffer;

        hce->CompId = Comp_HCE;

        status = ptx_RUL_HCE_ResetState(hce);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_HCE_RegisterExtension(hce->Rul, hce);
        }

        if (PTX_STATUS_SUCCESS != status)
        {
            hce->CompId = Comp_None;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_HCE_DeInit(ptx_RUL_HCE_t *hce)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hce, Comp_HCE))
    {
        status = ptx_RUL_HCE_RemoveExtension(hce->Rul);

        hce->CompId = Comp_None;
        if (PTX_STATUS_SUCCESS == status)
        {
            memset(hce, 0, sizeof(ptx_RUL_HCE_t));
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_HCE_ResetState(ptx_RUL_HCE_t *hce)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hce, Comp_HCE))
    {
        status = ptx_RUL_HCE_InitEventQueue(hce);

        if (PTX_STATUS_SUCCESS == status)
        {
            hce->DiscoverState = RUL_HCE_DiscoverState_No_Card;
            hce->ActiveProtocol = ptx_NSC_RfProtocol_Undefined;
            hce->ActiveTech = ptx_NSC_RfTech_RfTech_Undefined;

            memset(hce->AppRxBuffer, 0, hce->AppRxBufferSize);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_HCE_GetEvent(ptx_RUL_HCE_t *hce, ptx_RUL_HCE_EventRecord_t **event)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint16_t record_index = 0;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (NULL != event))
    {
        ptx_RUL_Status_Params_t status_params;
        memset(&status_params, 0, sizeof(ptx_RUL_Status_Params_t));
        status_params.Type = ptx_RUL_StatusType_System;

        /* Required for extension callbacks to be executed */
        status = ptx_RUL_GetStatusInfo(hce->Rul, &status_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Get the next event currently pending in the event queue. This event is currently
            * the oldest event pending and is positioned at the end (tail) of the queue. */
            status = ptx_RUL_HCE_ReadNextEvent(hce, event, &record_index);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_HCE_ReleaseEvent(hce, *event, record_index);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_HCE_SendData(ptx_RUL_HCE_t *hce, uint8_t *tx, uint32_t txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (NULL != tx))
    {
        /* Tx Rf Data. */
        status = ptx_NSC_DataTx(&(hce->Rul->Nsc), tx, txLen, 0U);
    }

    return status;
}

ptx_Status_t ptx_RUL_HCE_ReserveEventRecord(ptx_RUL_HCE_t *hce, ptx_RUL_HCE_EventRecord_t **evtRecord, uint16_t *eventRecordIndex)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint16_t record_index;
    ptx_RUL_HCE_EventRecord_t *evt_record;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (NULL != evtRecord) && (NULL != eventRecordIndex))
    {
        status = ptx_RUL_HCE_ReadFromTail(hce, &record_index);

        if (PTX_STATUS_SUCCESS == status)
        {
            evt_record = &hce->EventQ.Entries[record_index];
            *evtRecord = evt_record;
            *eventRecordIndex = record_index;

            /* Initialise event record to a known state. */
            evt_record->EventID        = RUL_HCE_EventType_NoEvent;
            evt_record->RxMsgData      = NULL;
            evt_record->RxMsgDataLen   = 0;
            (void)memset(&evt_record->RxMsgDataShortMsgData[0], 0xFF, e_SizeOfShortMsgBuffer);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_HCE_AddNewEventPending(ptx_RUL_HCE_t *hce, uint16_t recordIndex)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hce, Comp_HCE))
    {
        status = ptx_RUL_HCE_AddToTail(hce, recordIndex);
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_RUL_HCE_RegisterExtension(ptx_RUL_t *rul, ptx_RUL_HCE_t *hce)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && PTX_COMP_CHECK(hce, Comp_HCE))
    {
        ptx_RUL_Extension_Endpoints_t ep;
        memset(&ep, 0, sizeof(ptx_RUL_Extension_Endpoints_t));

        ep.CompId = hce->CompId;
        ep.Extension = hce;
        ep.ProcessDataMsg = ptx_RUL_HCE_ProcessDataMsg;
        ep.ProcessError = ptx_RUL_HCE_ProcessError;
        ep.ProcessReset = ptx_RUL_HCE_ProcessReset;
        ep.ProcessRfActivate = ptx_RUL_HCE_ProcessRfActivate;
        ep.ProcessRfField = ptx_RUL_HCE_ProcessRfField;
        ep.ProcessRfDeactivate = ptx_RUL_HCE_ProcessRfDeactivate;

        status = ptx_RUL_RegisterExtensionEndpoints(rul, &ep);
    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_RemoveExtension(ptx_RUL_t *rul)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        status = ptx_RUL_RemoveExtensionEndpoints(rul, Comp_HCE);
    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_ProcessDataMsg(void *ctx, ptx_NSC_RfData_Msg_t *msg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_HCE_t *hce = ctx;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (NULL != msg))
    {
        if (RUL_HCE_DiscoverState_Activate == hce->DiscoverState)
        {
            ptx_RUL_HCE_EventRecord_t *record;
            uint16_t idx;

            status = ptx_RUL_HCE_ReserveEventRecord(hce, &record, &idx);
            if (PTX_STATUS_SUCCESS == status)
            {
                if (ptx_NSC_RfData_Msg_Type_Control != msg->Fcb.MsgType)
                {
                    record->EventID = RUL_HCE_EventType_Data;

                    if ((hce->AppRxBufferIndex + msg->CtrlLen.DataLen) <= hce->AppRxBufferSize)
                    {
                        memcpy(&hce->AppRxBuffer[hce->AppRxBufferIndex], msg->Data, msg->CtrlLen.DataLen);

                        hce->AppRxBufferIndex += msg->CtrlLen.DataLen;

                        record->RxMsgData = hce->AppRxBuffer;
                        record->RxMsgDataLen = hce->AppRxBufferIndex;
                    }
                    else
                    {
                        record->RxMsgData = NULL;
                    }

                    if (ptx_NSC_RfData_Msg_Chaining_None == msg->Fcb.Chaining)
                    {
                        hce->AppRxBufferIndex = 0;
                        status = ptx_RUL_HCE_AddNewEventPending(hce, idx);
                    }
                }
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_ProcessError(void *ctx, ptx_NSC_Error_Ntf_t *ntf, uint8_t *ntf_used)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_HCE_t *hce = ctx;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (NULL != ntf))
    {
        ptx_RUL_HCE_EventRecord_t *record;
        uint16_t idx;

        status = ptx_RUL_HCE_ReserveEventRecord(hce, &record, &idx);

        if (PTX_STATUS_SUCCESS == status)
        {
            record->EventID = RUL_HCE_EventType_Error;
            record->RxMsgDataShortMsgData[0] = ntf->Error;

            hce->DiscoverState = RUL_HCE_DiscoverState_No_Card;
            hce->ActiveProtocol = ptx_NSC_RfProtocol_Undefined;
            hce->ActiveTech = ptx_NSC_RfTech_RfTech_Undefined;
            hce->AppRxBufferIndex = 0;

            status = ptx_RUL_HCE_AddNewEventPending(hce, idx);

            if (PTX_STATUS_SUCCESS == status)
            {
                *ntf_used = PTX_RUL_EXTENSION_NTF_USED;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_ProcessReset(void *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_HCE_t *hce = ctx;

    if (PTX_COMP_CHECK(hce, Comp_HCE))
    {
        ptx_RUL_HCE_EventRecord_t *record;
        uint16_t idx;

        status = ptx_RUL_HCE_ReserveEventRecord(hce, &record, &idx);

        if (PTX_STATUS_SUCCESS == status)
        {
            record->EventID = RUL_HCE_EventType_NoEvent;

            hce->AppRxBufferIndex = 0;
            hce->DiscoverState = RUL_HCE_DiscoverState_No_Card;

            status = ptx_RUL_HCE_AddNewEventPending(hce, idx);
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_ProcessRfActivate(void *ctx, ptx_NSC_RfActivate_Ntf_t *ntf, uint8_t *ntf_used)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_HCE_t *hce = ctx;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (NULL != ntf) && (NULL != ntf_used))
    {
        if ((ptx_NSC_RfTech_Passive_Listen_A == ntf->RfTech) ||
            (ptx_NSC_RfTech_Passive_Listen_B == ntf->RfTech) ||
            (ptx_NSC_RfTech_Passive_Listen_F == ntf->RfTech) ||
            (ptx_NSC_RfTech_Active_Listen == ntf->RfTech))
        {
            ptx_RUL_HCE_EventRecord_t *record;
            uint16_t idx;

            status = ptx_RUL_HCE_ReserveEventRecord(hce, &record, &idx);

            if (PTX_STATUS_SUCCESS == status)
            {
                record->EventID = RUL_HCE_EventType_Activated;
                record->RxMsgDataShortMsgData[0] = ntf->RfTech;
                record->RxMsgDataShortMsgData[1] = (uint8_t)ntf->RfProt;

                hce->DiscoverState = RUL_HCE_DiscoverState_Activate;
                hce->ActiveProtocol = ntf->RfProt;
                hce->ActiveTech = ntf->RfTech;

                status = ptx_RUL_HCE_AddNewEventPending(hce, idx);

                if (PTX_STATUS_SUCCESS == status)
                {
                    *ntf_used = PTX_RUL_EXTENSION_NTF_USED;
                }
            }
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
            *ntf_used = PTX_RUL_EXTENSION_NTF_UNUSED;
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_ProcessRfField(void *ctx, uint8_t field, uint8_t *ntf_used)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_HCE_t *hce = ctx;

    if (PTX_COMP_CHECK(hce, Comp_HCE))
    {
        ptx_RUL_HCE_EventRecord_t *record;
        uint16_t idx;

        status = ptx_RUL_HCE_ReserveEventRecord(hce, &record, &idx);

        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 == field)
            {
                record->EventID = RUL_HCE_EventType_ExtFieldOff;
                hce->DiscoverState = RUL_HCE_DiscoverState_No_Card;
            }
            else
            {
                record->EventID = RUL_HCE_EventType_ExtFieldOn;
                hce->DiscoverState = RUL_HCE_DiscoverState_Activate;
            }

            record->RxMsgDataLen = 0;

            hce->AppRxBufferIndex = 0;

            status = ptx_RUL_HCE_AddNewEventPending(hce, idx);

            if (PTX_STATUS_SUCCESS == status)
            {
                *ntf_used = PTX_RUL_EXTENSION_NTF_USED;
            }

        }

    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_ProcessRfDeactivate(void *ctx, ptx_NSC_RfDeactivate_Ntf_t *ntf, uint8_t *ntf_used)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_HCE_t *hce = ctx;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (NULL != ntf))
    {
        ptx_RUL_HCE_EventRecord_t *record;
        uint16_t idx;

        status = ptx_RUL_HCE_ReserveEventRecord(hce, &record, &idx);

        if (PTX_STATUS_SUCCESS == status)
        {
            record->EventID = RUL_HCE_EventType_Deactivated;
            record->RxMsgDataShortMsgData[0] = ntf->Reason;

            hce->DiscoverState = RUL_HCE_DiscoverState_No_Card;
            hce->ActiveProtocol = ptx_NSC_RfProtocol_Undefined;
            hce->ActiveTech = ptx_NSC_RfTech_RfTech_Undefined;

            status = ptx_RUL_HCE_AddNewEventPending(hce, idx);
        }

        if (    (PTX_STATUS_SUCCESS == status)
                && (NSC_ErrorCode_NoErr != ntf->Error)   )
        {
            status = ptx_RUL_HCE_ReserveEventRecord(hce, &record, &idx);

            if (PTX_STATUS_SUCCESS == status)
            {
                record->EventID = RUL_HCE_EventType_Error;
                record->RxMsgDataShortMsgData[0] = ntf->Error;

                status = ptx_RUL_HCE_AddNewEventPending(hce, idx);
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            *ntf_used = PTX_RUL_EXTENSION_NTF_USED;
        }

    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_ReadFromTail(ptx_RUL_HCE_t *hce, uint16_t *recordIndex)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (recordIndex != NULL))
    {
        /* Check that the event queue is not empty? */
        if (hce->EventQ.NrOfEntries < hce->EventQ.MaxNrOfEntries)
        {
            /* Get the index of the event record currently at the tail of the queue. */
            *recordIndex = hce->EventQ.TailIndex;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_ReadFromHead(ptx_RUL_HCE_t *hce, uint16_t *recordIndex)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (recordIndex != NULL))
    {
        /* Check that the event queue is not empty? */
        if (0 != hce->EventQ.NrOfEntries)
        {
            /* Get the index of the event record currently at the head of the queue. */
            *recordIndex = hce->EventQ.HeadIndex;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_AddToTail(ptx_RUL_HCE_t *hce, uint16_t recordIndex)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hce, Comp_HCE))
    {
        /* Check that the event queue is not full? */
        if (hce->EventQ.NrOfEntries < hce->EventQ.MaxNrOfEntries)
        {
            /* Check that the user is referring to the current and valid event record
             * at the end (tail) of the queue. This test assumes that ptx_NSC_ReserveEventRecord
             * had been called prior to this action. */
            if (recordIndex == hce->EventQ.TailIndex)
            {
                /* all ok */
                if (hce->EventQ.TailIndex == 0 )
                {
                    /* then at the beginning of the array of records. */
                    hce->EventQ.TailIndex = (hce->EventQ.MaxNrOfEntries) - 1;
                }
                else
                {
                    hce->EventQ.TailIndex--;
                }

                hce->EventQ.NrOfEntries++;

                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_RemoveFromHead(ptx_RUL_HCE_t *hce, uint16_t *recordIndex)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (recordIndex != NULL))
    {
        /* Check that the event queue is not empty? */
        if (0 != hce->EventQ.NrOfEntries)
        {
            *recordIndex = hce->EventQ.HeadIndex;

            if (hce->EventQ.HeadIndex == 0)
            {
                /* then head at the beginning of the array of records. */
                hce->EventQ.HeadIndex = (hce->EventQ.MaxNrOfEntries) - 1;
            }
            else
            {
                hce->EventQ.HeadIndex--;
            }

            hce->EventQ.NrOfEntries--;

            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_ReadNextEvent(ptx_RUL_HCE_t *hce, ptx_RUL_HCE_EventRecord_t **evtRecord, uint16_t *indexOfRecordRead)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint16_t record_index;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (evtRecord != NULL) && (indexOfRecordRead != NULL))
    {
        status = ptx_RUL_HCE_ReadFromHead(hce, &record_index);

        if (PTX_STATUS_SUCCESS == status)
        {
            *evtRecord = &hce->EventQ.Entries[record_index];
            *indexOfRecordRead = record_index;
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_ReleaseEvent(ptx_RUL_HCE_t *hce, ptx_RUL_HCE_EventRecord_t *evtRecord, uint16_t recordIndex)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint16_t head_index;

    if (PTX_COMP_CHECK(hce, Comp_HCE) && (evtRecord != NULL))
    {
        /* Verify that the buffer reference information is valid */
        if (evtRecord == &hce->EventQ.Entries[recordIndex])
        {
            status = ptx_RUL_HCE_RemoveFromHead(hce, &head_index);
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

    }

    return status;
}

static ptx_Status_t ptx_RUL_HCE_InitEventQueue(ptx_RUL_HCE_t *hce)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != hce)
    {
        hce->EventQ.NrOfEntries = 0;
        hce->EventQ.HeadIndex = e_EvtRecordIndexMin;
        hce->EventQ.TailIndex = e_EvtRecordIndexMin;
        hce->EventQ.MaxNrOfEntries = e_EvtMaxNrOfRecords;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}
