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
    File        : ptx_RUL_HCE.h

    Description : RUL HCE API
                  Contains functions to use a PTX Device with RUL HCE functionality.
*/

/**
 * \addtogroup RUL_HCE
 * @{
 */

#if !defined(PTX_RULHCE_H)
#define PTX_RULHCE_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>

#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_NSC_Base.h"
#include "ptx_RUL.h"

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
 * \brief RUL HCE RF-Protocol Type(s)
 */
typedef enum ptx_RUL_HCE_RfProtocol
{
    RUL_HCE_RfProtocol_Undefined = 0x00,/**< Undefined Protocol. */
    RUL_HCE_RfProtocol_T2T       = 0x02,/**< T2T. */
    RUL_HCE_RfProtocol_IsoDep    = 0x04 /**< IsoDep. */

} ptx_RUL_HCE_RfProtocol_t;

/**
 * \brief HCE Event Queue Management (Dimensions)
 */
typedef enum ptx_RUL_HCE_EventQueueDimensions
{
    e_MaxNrOfEventRecords           =  64,  /**< Maximum number of event records. */
    e_SizeOfShortMsgBuffer          =   3,  /**< Size of short buffers. */
    e_SizeOfActivationBuffer        =  16,  /**< Size of activation buffers. */

} ptx_RUL_HCE_EventQueueDimensions_t;

/**
 * \brief HCE Event Queue Management (Ranges)
 */
typedef enum ptx_RUL_HCE_EventQueueRanges
{
    e_EvtRecordIndexMin = 0,                      /**< Minimum index. */
    e_EvtRecordIndexMax = e_MaxNrOfEventRecords - 1, /**< Maximum index. */
    e_EvtMaxNrOfRecords,                          /**< Maximum number of records. */
    e_EvtRecIndexNotSet                           /**< No index. */

} ptx_RUL_HCE_EventQueueRanges_t;

/**
 * \brief RUL HCE Event Identifiers
 */
typedef enum ptx_RUL_HCE_EventType
{
    RUL_HCE_EventType_NoEvent,          /**< No Event. */
    RUL_HCE_EventType_ExtFieldOn,       /**< External Field On */
    RUL_HCE_EventType_ExtFieldOff,      /**< External Field Off. (Received if field was turned of but card NOT activated, if card activated and turned of -> deactivate NTF with reason field of) */
    RUL_HCE_EventType_Activated,        /**< Activated. TODO which type */
    RUL_HCE_EventType_Data,             /**< Data Event. */
    RUL_HCE_EventType_Deactivated,      /**< Deactivated. */
    RUL_HCE_EventType_Error,            /**< Error. */

} ptx_RUL_HCE_EventType_t;

/**
 * \brief RUL HCE Event Record Type
 */
typedef struct
{
    ptx_RUL_HCE_EventType_t     EventID;        /**< Event ID */
    uint8_t                     RxMsgDataShortMsgData[e_SizeOfShortMsgBuffer];      /**< Rx message data short msg data */
    uint8_t                    *RxMsgData;      /**< Rx Message data buffer */
    uint32_t                    RxMsgDataLen;   /**< Rx Message data length */

} ptx_RUL_HCE_EventRecord_t;

/**
 * \brief RUL HCE Event Queue
 */
typedef struct
{
    ptx_RUL_HCE_EventQueueRanges_t    NrOfEntries;      /**< Number of entries */
    ptx_RUL_HCE_EventQueueRanges_t    HeadIndex;        /**< Head Index */
    ptx_RUL_HCE_EventQueueRanges_t    TailIndex;        /**< Tail Index */
    ptx_RUL_HCE_EventQueueRanges_t    MaxNrOfEntries;   /**< Maximum number of entries */
    ptx_RUL_HCE_EventRecord_t         Entries[e_MaxNrOfEventRecords]; /**< Event entries */
} ptx_RUL_HCE_EventQueue_t;

/**
 * \brief RUL HCE Activation status.
 */
typedef enum ptx_RUL_HCE_DiscoverState
{
    RUL_HCE_DiscoverState_No_Card  = 0,
    RUL_HCE_DiscoverState_Activate = 1
} ptx_RUL_HCE_DiscoverState_t;

/**
 * \brief RUL HCE Initialisation.
 */
typedef struct ptx_RUL_HCE_InitParams
{
    uint8_t                             *AppRxBuffer;       /**< Application Rx Buffer. */
    uint32_t                            AppRxBufferSize;    /**< Application Rx Buffer size. */

    ptx_RUL_t                          *Rul;                /**< Pointer to the RUL component. */

} ptx_RUL_HCE_InitParams_t;

/**
 * \brief RUL HCE Structure.
 */
typedef struct ptx_RUL_HCE
{
    ptx_Comps_t                     CompId;             /**< Component Id. */
    ptx_RUL_HCE_RfProtocol_t        Type;               /**< Card Type. */

    ptx_RUL_t                      *Rul;                /**< Pointer to the RUL component. */

    ptx_RUL_HCE_EventQueue_t        EventQ;             /**< Event queue. */

    uint8_t                         ActivationData[e_SizeOfActivationBuffer];   /**< Activation Data. */
    uint8_t                         *AppRxBuffer;       /**< Application Rx Buffer. */
    uint32_t                        AppRxBufferSize;    /**< Application Rx Buffer size. */
    uint32_t                        AppRxBufferIndex;   /**< Application Rx Buffer index. */

    ptx_RUL_HCE_DiscoverState_t     DiscoverState;      /**< Discover State. */

    ptx_NSC_RfProtocol_t            ActiveProtocol;     /**< Currently active protocol */
    uint8_t                         ActiveTech;         /**< Currently active Technology*/

} ptx_RUL_HCE_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief This function initialises the Host Card Emulation Component.
 *
 * \param[in]   hce             Pointer to an allocated instance of the HCE.
 * \param[in]   initParams      Pointer to allocated initialisation parameter structure.
 *
 * \return      Status,         indicating whether the operation was successful.
 *                              See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_HCE_Init(ptx_RUL_HCE_t *hce, ptx_RUL_HCE_InitParams_t *initParams);

/**
 * \brief This function De-initialises the Host Card Emulation Component.
 *
 * \param[in]       hce         Pointer to component.
 *
 * \return Status,              indicating whether the operation was successful.
 *                              See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_HCE_DeInit(ptx_RUL_HCE_t *hce);

/**
 * \brief Internal reset of component state.
 *
 * \param[in]       hce         Pointer to component.
 *
 * \return Status,              indicating whether the operation was successful.
 *                              See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_HCE_ResetState(ptx_RUL_HCE_t *hce);

/**
 * \brief This function allows the user to request latest event notification data
 * received from the PTX card emulation device.
 *
 * Events relevant to the HCE application are;
 * - a remote RF field has been detected or lost,
 * - a remote RF device has been detected (discovered),
 * - a remote RF device previously detected has now been activated,
 * - a remote RF device previously activated has now been deactivated,
 * - an error has occurred in the PTX subsystem,
 * - debug information regarding the NSC stack is available.
 * - an RF data message has been received.
 *
 * \param[in]      hce          Pointer to an allocated instance of the HCE
 *                              stack controller record. 'ptx_Nsc_EventRecord_t *nscEventRep' is
 *                              embedded in this structure for the user to read.
 *
 * \param[in,out]  event        Reference to an event record supplied by the AP into which
 *                              event details can be entered.
 *
 * \return Status,              indicating whether the operation was successful.
 *                              See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_HCE_GetEvent(ptx_RUL_HCE_t *hce, ptx_RUL_HCE_EventRecord_t **event);

/**
 * \brief This function allows the currently activated card to send data to a
 *  nearby card reader.
 *
 * - this is not a blocking function, which means that it will return to the caller immediately after the transmit
 *   data has been configured for sending in the serial communications layer.
 *
 * \note In case the ISO-DEP protocol (according to ISO 14443-4) is used, the system takes internally care of the
 * timeout when the reader communicates with a card depending on the information which was exchanged during
 * activation (e.g. ATS for Type-A or ATQB for Type-B). In this case the caller has still to provide a meaningful
 * value (e.g. multiple seconds)! If a card reported a max. timeout of less than a second, the function will return
 * after this timeout has expired even if the timeout parameter has been set to e.g. 5 seconds.
 *
 * \param[in]   hce             Pointer to an allocated instance of the HCE
 *                              stack controller record.
 * \param[in]   tx              User buffer containing the 'raw' RF data message to send.
 * \param[in]   txLen           Length of data to send which is the actual size of the transmit buffer.
 *
 * \return Status,              indicating whether the operation was successful.
 *                              See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_HCE_SendData(ptx_RUL_HCE_t *hce, uint8_t *tx, uint32_t txLen);

/**
 * \brief Reserves an Event Record at the Tail of the Event Queue.
 *
 * \param[in]       hce              Pointer to component.
 * \param[in]       eventRecord      Reference to the Event Record to be
 *                                   reserved at the end of the Event Queue.
 * \param[out]      eventRecordIndex Index of the reserved event
 *
 * \return Status,                   indicating whether the operation was successful.
 *                                   See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_HCE_ReserveEventRecord(ptx_RUL_HCE_t *hce, ptx_RUL_HCE_EventRecord_t **eventRecord, uint16_t *eventRecordIndex);

/**
 * \brief Adds a new pending Event Record at the Tail of the Event Queue.
 *
 * \param[in]       hce             Pointer to component.
 * \param[out]      recordIndex     Pointer to the Index of the added Event.
 *
 * \return Status,                  indicating whether the operation was successful.
 *                                  See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_HCE_AddNewEventPending(ptx_RUL_HCE_t *hce, uint16_t recordIndex);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

