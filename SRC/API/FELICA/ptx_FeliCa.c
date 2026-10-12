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
    Module      : FELICA
    File        : ptx_FeliCa.c

    Description : Implementation of FeliCa library.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_FeliCa.h"
#include "ptx_RUL.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 *  NSC Revision-Info Types.
 */
typedef enum ptx_FeliCa_Polling_State
{
    Polling_State_Reset,
    Polling_State_Start_Polling,
    Polling_State_Get_Number_Card_Responses,
    Polling_State_Parse_And_Store_Card_Data,
    Polling_State_Collision,
    Polling_State_Set_Next_Polling_Cycle,
    Polling_State_Exit,

} ptx_FeliCa_Polling_State_t;


/*
 *  Internal min. sizes for Tx- and Rx-work-buffers
 */
#define PTX_FELICA_MIN_WORK_BUFFER_SIZE     256U

/*
 *  Maximum Transfer Unit.
 */
#define PTX_FELICA_HW_MTU                   (uint32_t)(PTX_FELICA_MIN_WORK_BUFFER_SIZE - 3)

/*
 *  FeliCa Command & Response definitions
 */
#define PTX_FELICA_CMD_REQUEST_RESPONSE                 0x04U   /* Request Response Command Code */
#define PTX_FELICA_RSP_REQUEST_RESPONSE                 0x05U   /* Request Response Response Code */
#define PTX_FELICA_CMD_REQUEST_SERVICE                  0x02U   /* Request Service Command Code */
#define PTX_FELICA_RSP_REQUEST_SERVICE                  0x03U   /* Request Service Response Code */
#define PTX_FELICA_CMD_READ                             0x06U   /* Read the record value of the specified service. */
#define PTX_FELICA_RSP_READ                             0x07U   /* Response code to the Read command. */
#define PTX_FELICA_CMD_WRITE                            0x08U   /* Write records of the specified service. */
#define PTX_FELICA_RSP_WRITE                            0x09U   /* Response code to the Write command. */
#define PTX_FELICA_RSP_POLL                             0x01U   /* Response code of SENSF_RES/POLLING command. */

/*
 *  FeliCa Command Timeout Parameter Offset within PMm (see FeliCa User-Manual)
 */
#define PTX_FELICA_CMD_REQUEST_RESPONSE_TO_OFFSET       3U
#define PTX_FELICA_CMD_REQUEST_SERVICE_TO_OFFSET        2U
#define PTX_FELICA_CMD_READ_TO_OFFSET                   5U
#define PTX_FELICA_CMD_WRITE_TO_OFFSET                  6U

/*
 *  Various FeliCa Constants
 */
#define PTX_FELICA_COMMAND_CODE_LEN                     1
#define PTX_FELICA_NR_SERVICES_LEN                      1
#define PTX_FELICA_NR_BLOCKS_LEN                        1
#define PTX_FELICA_STATUS_FLAGS_LEN                     2
#define PTX_FELICA_SENSF_RES_HEADER_LEN                 2      /* 1 byte RESPONSE-CODE + 1 byte proprietary RF-status appended to response. */
#define PTX_FELICA_SENSF_RES_OFFSET                     2
/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_FeliCa_PollCards(ptx_FeliCa_t *feliCa, uint16_t systemCode, uint8_t requestCode, uint8_t timeslotNr);
static ptx_Status_t ptx_FeliCa_GetNumberCardResponses(ptx_FeliCa_t *feliCa, uint8_t rdPresent, uint8_t *nrCardResponses);
static ptx_Status_t ptx_FeliCa_ParseCardData(ptx_FeliCa_t *feliCa,
                                             uint32_t *parseIndex,
                                             uint8_t rdPresent,
                                             ptx_FeliCa_CardData_t *cardData,
                                             uint8_t *isCollision);
static ptx_Status_t ptx_FeliCa_StoreCardData(ptx_FeliCa_t *feliCa, ptx_FeliCa_CardData_t *cardData);


/*
 *  API-independent (POS / IoT) Data-Exchange function for FeliCa. Works based on internal work-buffers.
 *
 *        feliCa          Pointer to an existing instance of the FeliCa component.
 *        msAppTimeout    Application-timeout in ms that the function is going to wait for receiving data from the card.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_FeliCa_INT_DataExchange (ptx_FeliCa_t *feliCa, uint32_t msAppTimeout);

/*
 *  Adds/Updates the information about available FeliCa cards from underlying component
 *        (POS -> Activation Parameters, IoT -> Card Registry) to FeliCa component.
 *
 *        feliCa          Pointer to an existing instance of the FeliCa component.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_FeliCa_INT_UpdateCardInfo (ptx_FeliCa_t *feliCa);

/*
 *  Calculates the timeout-value in ms for a FeliCa/T3T command based on the command's max. response time parameter.
 *
 *   maxResponseTimeParam Pointer to an existing instance of the FeliCa component.
 *   commandCode          FeliCa/T3T command-code.
 *   n                    Number of blocks / nodes.
 *  timeoutValue         Calculated timeout value [ms].
 *
 * returns Calculated timeout value [ms].
 */
static uint32_t ptx_FeliCa_INT_CalculateTimeout (uint8_t maxResponseTimeParam, uint8_t n);

/*
 *  Sets up a FeliCa/T3T command by
 *        - updating the card information from the underlying component (POS or IoT) if needed,
 *        - checks if there is any card present,
 *        - prepares the internal Tx buffer (set command code and IDm).
 *
 *  feliCa    Pointer to an existing instance of the FeliCa component.
 *  commandCode   FeliCa/T3T command-code.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_FeliCa_INT_SetupCommand (ptx_FeliCa_t *feliCa, uint8_t commandCode);

/*
 *  Performs basic checks on a received response such as
 *        - checking if anything was received at all,
 *        - if any (contactless) error happened during the reception,
 *        - if the received response code matches to the sent command code,
 *        - if the received IDM matches the sent IDM and
 *        provides the information about the received payload for further processing based on the internal work-members.
 *
 *        feliCa      Pointer to an existing instance of the FeliCa component.
 *
 * returns Status, indicating whether the operation was successful.
 */
static ptx_Status_t ptx_FeliCa_INT_CheckResponse (ptx_FeliCa_t *feliCa);
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_FeliCa_Open (ptx_FeliCa_t *feliCa, ptx_FeliCa_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != feliCa) && (NULL != initParams))
    {
        if ((NULL != initParams->Rul) &&
            (NULL != initParams->RxBuffer) &&
            (NULL != initParams->TxBuffer) &&
            (PTX_FELICA_MIN_WORK_BUFFER_SIZE <= initParams->RxBufferSize) &&
            (PTX_FELICA_MIN_WORK_BUFFER_SIZE <= initParams->TxBufferSize))
        {
            /* clear component */
            (void)memset(feliCa, 0, sizeof(ptx_FeliCa_t));

            feliCa->Rul = initParams->Rul;
            /* Note: No need to store the sizes of the buffers due to the valid condition -> Size >= MIN_BUFFER_SIZE i.e.
             *       all following operations can simply use constant PTX_FELICA_MIN_WORK_BUFFER_SIZE which covers all possible
             *       FeliCa/T3T commands.
             */
            feliCa->Tx = initParams->TxBuffer;
            feliCa->Rx = initParams->RxBuffer;

            /* Assign Component ID */
            feliCa->CompId = Comp_FeliCa;

            status = ptx_FeliCa_ResetState(feliCa);

            if (PTX_STATUS_SUCCESS != status)
            {
                /* clear component again */
                (void)memset(feliCa, 0, sizeof(ptx_FeliCa_t));
            }

        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

    }

    return status;
}

ptx_Status_t ptx_FeliCa_Close (ptx_FeliCa_t *feliCa)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa))
    {
        memset(feliCa, 0, sizeof(ptx_FeliCa_t));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_FeliCa_ResetState (ptx_FeliCa_t *feliCa)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa))
    {
        memset(feliCa->Tx, 0, PTX_FELICA_MIN_WORK_BUFFER_SIZE);
        memset(feliCa->Rx, 0, PTX_FELICA_MIN_WORK_BUFFER_SIZE);

        /* It is possible that the application initializes this component in state POLL_ACTIVE/DATA_EXCHANGE of the underlying
         * component when there are already FeliCa cards registered in the system.
         * If that's the case, this information needs to be made available within this component by calling \ref ptx_FeliCa_INT_UpdateCardInfo.
         * If it's not the case, the information gets updated prior to any other FeliCa-command within this component.
         */
        status = ptx_FeliCa_INT_UpdateCardInfo(feliCa);
    }

    return status;
}

ptx_Status_t ptx_FeliCa_ResetCardInfo (ptx_FeliCa_t *feliCa)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa))
    {
        feliCa->CardParams.NrAvailableCards = 0;
        feliCa->CardParams.ActiveCardIndex = 0;
        (void)memset(&feliCa->CardParams.CardData[0], 0, (sizeof(ptx_FeliCa_CardData_t) * PTX_MAX_SUPPORTED_FELICA_CARDS));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_FeliCa_Polling (ptx_FeliCa_t *feliCa,
                                 uint16_t systemCode,
                                 ptx_FeliCa_RequestCode_t requestCode,
                                 ptx_FeliCa_TimeslotNr_t timeslotNr,
                                 uint8_t *nrAvailableCards,
                                 ptx_FeliCa_CardData_t **cardData)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t rd_present = (FeliCa_RC_NoRequestCode == requestCode) ? 0 : 1;
    uint8_t nr_available_cards = 0;
    uint8_t is_collision = 0;
    uint8_t collision_pending = 0;
    uint32_t parse_index = 0;
    ptx_FeliCa_TimeslotNr_t nr_timeslots = timeslotNr;
    ptx_FeliCa_CardData_t card_data;
    ptx_FeliCa_Polling_State_t poll_state = Polling_State_Reset;
    uint8_t nr_polling_cycles = 0;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa) && (NULL != nrAvailableCards) && (NULL != cardData))
    {
        status = PTX_STATUS_SUCCESS;
        while ((Polling_State_Exit != poll_state) && (PTX_STATUS_SUCCESS == status))
        {
            switch (poll_state)
            {
                case Polling_State_Reset:
                    /* triggering a new Polling-command might change the already registered cards - so reset the info on them upfront to avoid any mismatches */
                    status = ptx_FeliCa_ResetCardInfo(feliCa);
                    poll_state = (PTX_STATUS_SUCCESS == status) ? Polling_State_Start_Polling : Polling_State_Exit;
                    break;

                case Polling_State_Start_Polling:
                    /* send Polling-command with given timeslot number and request code */
                    status = ptx_FeliCa_PollCards(feliCa, systemCode, (uint8_t)requestCode, nr_timeslots);
                    poll_state = (PTX_STATUS_SUCCESS == status) ? Polling_State_Get_Number_Card_Responses : Polling_State_Exit;
                    break;

                case Polling_State_Get_Number_Card_Responses:
                    parse_index = 0;
                    nr_available_cards = 0;
                    is_collision = 0;
                    collision_pending = 0;

                    /* determine how many responses were received*/
                    status = ptx_FeliCa_GetNumberCardResponses(feliCa, rd_present, &nr_available_cards);
                    poll_state = (PTX_STATUS_SUCCESS == status) ? Polling_State_Parse_And_Store_Card_Data : Polling_State_Exit;
                    break;

                case Polling_State_Parse_And_Store_Card_Data:
                    for (uint8_t i = 0; i < nr_available_cards; i++)
                    {
                        /* parse received responses one by one and determine whether a collision occurred or not */
                        status = ptx_FeliCa_ParseCardData(feliCa, &parse_index, rd_present, &card_data, &is_collision);

                        if (PTX_STATUS_SUCCESS == status)
                        {
                            if (0 == is_collision)
                            {
                                /* if the response is valid i.e. no collision occurred, store the card data */
                                status = ptx_FeliCa_StoreCardData(feliCa, &card_data);

                            }
                            else
                            {
                                collision_pending = 1;
                            }
                        }
                    }
                    poll_state = (PTX_STATUS_SUCCESS == status) ? Polling_State_Set_Next_Polling_Cycle : Polling_State_Exit;
                    break;

                case Polling_State_Set_Next_Polling_Cycle:
                    /* any additional polling-cycle requires the timeslot number to be set to 16 according to [NFC Forum Activity] */
                    nr_timeslots = FeliCa_TSN_16;
                    poll_state = Polling_State_Exit;

                    /* perform additional polling cycles only in case of a collision */
                    if ((0 != collision_pending) && (nr_polling_cycles < PTX_FELICA_ADDITIONAL_POLLING_CYCLES))
                    {
                        nr_polling_cycles++;
                        poll_state = Polling_State_Start_Polling;
                    }
                    break;

                case Polling_State_Exit:
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INTERNAL;
                    break;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* in case everything went well - provide information about stored cards to calling level */
            status = ptx_FeliCa_GetCardParameters(feliCa, nrAvailableCards, cardData);
        }

    }

    return status;
}

ptx_Status_t ptx_FeliCa_GetCardParameters (ptx_FeliCa_t *feliCa, uint8_t *nrAvailableCards, ptx_FeliCa_CardData_t **cardData)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa) && (NULL != nrAvailableCards) && (NULL != cardData))
    {
        *nrAvailableCards =  feliCa->CardParams.NrAvailableCards;
        *cardData         = &feliCa->CardParams.CardData[0];
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_FeliCa_SelectActiveCard (ptx_FeliCa_t *feliCa, uint8_t selectedCardIndex)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa) && (PTX_MAX_SUPPORTED_FELICA_CARDS > selectedCardIndex))
    {
        feliCa->CardParams.ActiveCardIndex = selectedCardIndex;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_FeliCa_RequestResponse (ptx_FeliCa_t *feliCa, uint8_t *currentMode)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t max_response_t_param;
    uint32_t timeout_value;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa) && (NULL != currentMode))
    {
        status = ptx_FeliCa_INT_SetupCommand(feliCa, PTX_FELICA_CMD_REQUEST_RESPONSE);

        if (PTX_STATUS_SUCCESS == status)
        {
            max_response_t_param = feliCa->CardParams.CardData[feliCa->CardParams.ActiveCardIndex].PMm[PTX_FELICA_CMD_REQUEST_RESPONSE_TO_OFFSET];
            timeout_value = ptx_FeliCa_INT_CalculateTimeout(max_response_t_param, 0);

            status = ptx_FeliCa_INT_DataExchange(feliCa, timeout_value);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* check if received FeliCa/T3T packet is valid and if there is any payload remaining */
                status = ptx_FeliCa_INT_CheckResponse(feliCa);

                if (PTX_STATUS_SUCCESS == status)
                {
                    if (0 != feliCa->RxLen)
                    {
                        /* everything OK - store user information */
                        *currentMode = feliCa->RxPayload[0];
                    }
                    else
                    {
                        status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
                    }
                }
            }
        }

    }

    return status;
}

ptx_Status_t ptx_FeliCa_RequestService (ptx_FeliCa_t *feliCa,
                                        uint8_t nrServices,
                                        uint8_t *serviceList,
                                        uint8_t *nrReadServices,
                                        uint8_t *readServiceList)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t max_response_t_param;
    uint32_t timeout_value;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa) && (32U >= nrServices) && (NULL != serviceList) && (NULL != nrReadServices) && (NULL != readServiceList))
    {
        status = ptx_FeliCa_INT_SetupCommand(feliCa, PTX_FELICA_CMD_REQUEST_SERVICE);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* set remaining parameters */
            feliCa->Tx[feliCa->TxLen] = nrServices;
            feliCa->TxLen++;

            (void)memcpy(&feliCa->Tx[feliCa->TxLen], &serviceList[0], (uint32_t)(nrServices * 2));
            feliCa->TxLen = (uint32_t)(feliCa->TxLen + (uint32_t)((nrServices * 2)));

            feliCa->Tx[0] = (uint8_t)feliCa->TxLen;

            max_response_t_param = feliCa->CardParams.CardData[feliCa->CardParams.ActiveCardIndex].PMm[PTX_FELICA_CMD_REQUEST_SERVICE_TO_OFFSET];
            timeout_value = ptx_FeliCa_INT_CalculateTimeout(max_response_t_param, 0);

            status = ptx_FeliCa_INT_DataExchange(feliCa, timeout_value);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* check if received FeliCa/T3T packet is valid and if there is any payload remaining */
                status = ptx_FeliCa_INT_CheckResponse(feliCa);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* everything OK - received number of services/nodes and actual service/node data */
                    *nrReadServices = feliCa->RxPayload[0];

                    if ((*nrReadServices == nrServices) && (feliCa->RxLen == (uint32_t)((uint32_t)(*nrReadServices * 2) + 1u)))
                    {
                        (void)memcpy(&readServiceList[0], &feliCa->RxPayload[1], (uint32_t)(*nrReadServices * 2));

                    }
                    else
                    {
                        *nrReadServices = 0;
                        status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
                    }
                }
            }
        }

    }

    return status;
}

ptx_Status_t ptx_FeliCa_Read (ptx_FeliCa_t *feliCa,
                              uint8_t nrServices,
                              uint8_t *serviceList,
                              uint8_t nrBlocksToRead,
                              uint8_t *blockListToRead,
                              uint8_t blockListToReadLength,
                              uint8_t *nrBlocksRead,
                              uint8_t *blockDataRead,
                              uint16_t *statusFlags)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t max_response_t_param;
    uint32_t timeout_value;
    uint32_t max_packet_size;
    uint16_t status_flags = 0;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa) && (NULL != serviceList) && (NULL != blockListToRead) && (NULL != blockDataRead) && (NULL != nrBlocksRead))
    {
        max_packet_size = (uint32_t)(PTX_FELICA_COMMAND_CODE_LEN +
                                     PTX_FELICA_IDM_MAX_LENGTH   +
                                     PTX_FELICA_NR_SERVICES_LEN  +
                                     (nrServices << 2)           +
                                     PTX_FELICA_NR_BLOCKS_LEN    +
                                     blockListToReadLength);

        /* do all parameters fit into the transmission buffer ? */
        if (PTX_FELICA_HW_MTU >= max_packet_size)
        {
            status = ptx_FeliCa_INT_SetupCommand(feliCa, PTX_FELICA_CMD_READ);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* set remaining parameters */
                feliCa->Tx[feliCa->TxLen] = nrServices;
                feliCa->TxLen++;

                (void)memcpy(&feliCa->Tx[feliCa->TxLen], &serviceList[0], (uint32_t)(nrServices * 2));
                feliCa->TxLen = (uint32_t)(feliCa->TxLen + (uint32_t)((nrServices * 2)));

                feliCa->Tx[feliCa->TxLen] = nrBlocksToRead;
                feliCa->TxLen++;

                (void)memcpy(&feliCa->Tx[feliCa->TxLen], &blockListToRead[0], (uint32_t)(blockListToReadLength));
                feliCa->TxLen = (uint32_t)(feliCa->TxLen + (uint32_t)((blockListToReadLength)));

                feliCa->Tx[0] = (uint8_t)feliCa->TxLen;

                max_response_t_param = feliCa->CardParams.CardData[feliCa->CardParams.ActiveCardIndex].PMm[PTX_FELICA_CMD_READ_TO_OFFSET];
                timeout_value = ptx_FeliCa_INT_CalculateTimeout(max_response_t_param, nrBlocksToRead);

                status = ptx_FeliCa_INT_DataExchange(feliCa, timeout_value);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* check if received FeliCa/T3T packet is valid and if there is any payload remaining */
                    status = ptx_FeliCa_INT_CheckResponse(feliCa);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* overwrite default initialization */
                        status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;

                        /* everything OK so far - check payload length before storing it */
                        if (PTX_FELICA_STATUS_FLAGS_LEN <= feliCa->RxLen)
                        {
                            status_flags = (uint16_t)((feliCa->RxPayload[0] << 8) | feliCa->RxPayload[1]);

                            /* save flags for upper layer */
                            if (NULL != statusFlags)
                            {
                                *statusFlags = status_flags;
                            }

                            /* read blocks are only available if the 2 status flag bytes show no error i.e. are set to 0 */
                            if (0 == status_flags)
                            {
                                *nrBlocksRead = feliCa->RxPayload[3];

                                if ((uint32_t)((feliCa->RxLen - PTX_FELICA_STATUS_FLAGS_LEN)) == (uint32_t)((PTX_FELICA_NR_BLOCKS_LEN + (nrBlocksToRead * PTX_FELICA_BLOCK_LEN))))
                                {
                                    /* everything OK finally - store read blocks */
                                    (void)memcpy(&blockDataRead[0],
                                                 &feliCa->RxPayload[PTX_FELICA_NR_BLOCKS_LEN + PTX_FELICA_STATUS_FLAGS_LEN],
                                                 (uint32_t)(nrBlocksToRead * PTX_FELICA_BLOCK_LEN));

                                    status = PTX_STATUS_SUCCESS;
                                }
                            }
                        }
                    }
                }
            }

        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

    }

    return status;
}

ptx_Status_t ptx_FeliCa_Write (ptx_FeliCa_t *feliCa,
                               uint8_t nrServices,
                               uint8_t *serviceList,
                               uint8_t nrBlocksToWrite,
                               uint8_t *blockListToWrite,
                               uint8_t blockListToWriteLength,
                               uint8_t *blockData,
                               uint16_t *statusFlags)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t max_response_t_param;
    uint32_t timeout_value;
    uint32_t max_packet_size;
    uint16_t status_flags = 0;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa) && (NULL != serviceList) && (NULL != blockListToWrite) && (NULL != blockData))
    {
        max_packet_size = (uint32_t)(PTX_FELICA_COMMAND_CODE_LEN +
                                     PTX_FELICA_IDM_MAX_LENGTH   +
                                     PTX_FELICA_NR_SERVICES_LEN  +
                                     (nrServices << 2)           +
                                     PTX_FELICA_NR_BLOCKS_LEN    +
                                     blockListToWriteLength      +
                                     PTX_FELICA_BLOCK_LEN * nrBlocksToWrite);

        /* do all parameters fit into the transmission buffer ? */
        if (PTX_FELICA_HW_MTU >= max_packet_size)
        {
            status = ptx_FeliCa_INT_SetupCommand(feliCa, PTX_FELICA_CMD_WRITE);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* set remaining parameters */
                feliCa->Tx[feliCa->TxLen] = nrServices;
                feliCa->TxLen++;

                (void)memcpy(&feliCa->Tx[feliCa->TxLen], &serviceList[0], (uint32_t)(nrServices * 2));
                feliCa->TxLen = (uint32_t)(feliCa->TxLen + (uint32_t)((nrServices * 2)));

                feliCa->Tx[feliCa->TxLen] = nrBlocksToWrite;
                feliCa->TxLen++;

                (void)memcpy(&feliCa->Tx[feliCa->TxLen], &blockListToWrite[0], (uint32_t)(blockListToWriteLength));
                feliCa->TxLen = (uint32_t)(feliCa->TxLen + (uint32_t)((blockListToWriteLength)));

                (void)memcpy(&feliCa->Tx[feliCa->TxLen], &blockData[0], (uint32_t)(PTX_FELICA_BLOCK_LEN * nrBlocksToWrite));
                feliCa->TxLen = (uint32_t)(feliCa->TxLen + (uint32_t)((PTX_FELICA_BLOCK_LEN * nrBlocksToWrite)));

                feliCa->Tx[0] = (uint8_t)feliCa->TxLen;

                max_response_t_param = feliCa->CardParams.CardData[feliCa->CardParams.ActiveCardIndex].PMm[PTX_FELICA_CMD_WRITE_TO_OFFSET];
                timeout_value = ptx_FeliCa_INT_CalculateTimeout(max_response_t_param, nrBlocksToWrite);

                status = ptx_FeliCa_INT_DataExchange(feliCa, timeout_value);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* check if received FeliCa/T3T packet is valid and if there is any payload remaining */
                    status = ptx_FeliCa_INT_CheckResponse(feliCa);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* everything OK so far - check payload length before storing it */
                        if (PTX_FELICA_STATUS_FLAGS_LEN <= feliCa->RxLen)
                        {
                            status_flags = (uint16_t)((feliCa->RxPayload[0] << 8) | feliCa->RxPayload[1]);

                            /* save flags for upper layer */
                            if (NULL != statusFlags)
                            {
                                *statusFlags = status_flags;
                            }

                        }
                        else
                        {
                            status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
                        }
                    }
                }
            }

        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

    }

    return status;
}
/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_FeliCa_INT_DataExchange (ptx_FeliCa_t *feliCa, uint32_t msAppTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa))
    {
        feliCa->RxLen = (uint32_t)PTX_FELICA_MIN_WORK_BUFFER_SIZE;

        status = ptx_RUL_Exchange(feliCa->Rul, &feliCa->Tx[0], feliCa->TxLen, &feliCa->Rx[0], &feliCa->RxLen, msAppTimeout);
    }

    return status;
}

static ptx_Status_t ptx_FeliCa_INT_UpdateCardInfo (ptx_FeliCa_t *feliCa)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t sens_res_len;
    ptx_RUL_Status_Params_t stat_params;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa))
    {
        /* try to add card info from underlying component only in case there is currently no info available for this component */
        if (0 == feliCa->CardParams.NrAvailableCards)
        {
            (void)ptx_FeliCa_ResetCardInfo(feliCa);

            stat_params.Type = ptx_RUL_StatusType_Discover;
            status = ptx_RUL_GetStatusInfo(feliCa->Rul, &stat_params);

            if (PTX_STATUS_SUCCESS == status)
            {
                if (ptx_RUL_StatusInfoDiscover_Active == stat_params.Info.Discover)
                {
                    ptx_RUL_CardRegistry_t *card_registry = NULL;
                    status = ptx_RUL_GetCardRegistry(feliCa->Rul, &card_registry);

                    uint8_t active_card_set = 0;

                    if (NULL != card_registry)
                    {
                        for (uint8_t i = 0; i < card_registry->CardsCnt; i++)
                        {
                            if (ptx_RUL_CardRfTechType_F == card_registry->Cards[i].TechType)
                            {
                                if (PTX_MAX_SUPPORTED_FELICA_CARDS > feliCa->CardParams.NrAvailableCards)
                                {
                                    /* the stored SENSF_RES (= ATQC) also stores the Length- and the Response-code - subtract it here */
                                    sens_res_len = (uint8_t)(PTX_RUL_SENSFRES_LEN - PTX_FELICA_SENSF_RES_OFFSET);

                                    /* Store IDm */
                                    (void)memcpy(&feliCa->CardParams.CardData[feliCa->CardParams.NrAvailableCards].IDm[0],
                                                 &card_registry->Cards[i].TechParam.TypeF.SensFRes[PTX_FELICA_SENSF_RES_OFFSET],
                                                 PTX_FELICA_IDM_MAX_LENGTH);

                                    /* Store PMm */
                                    (void)memcpy(&feliCa->CardParams.CardData[feliCa->CardParams.NrAvailableCards].PMm[0],
                                                 &card_registry->Cards[i].TechParam.TypeF.SensFRes[PTX_FELICA_SENSF_RES_OFFSET + PTX_FELICA_IDM_MAX_LENGTH],
                                                 PTX_FELICA_PMM_MAX_LENGTH);

                                    /* Store RD (if available) */
                                    if ((uint8_t)(PTX_FELICA_IDM_MAX_LENGTH + PTX_FELICA_PMM_MAX_LENGTH + PTX_FELICA_RD_MAX_LENGTH) == sens_res_len)
                                    {
                                        (void)memcpy(&feliCa->CardParams.CardData[feliCa->CardParams.NrAvailableCards].RD[0],
                                                     &card_registry->Cards[i].TechParam.TypeF.SensFRes[PTX_FELICA_SENSF_RES_OFFSET + PTX_FELICA_IDM_MAX_LENGTH + PTX_FELICA_PMM_MAX_LENGTH],
                                                     PTX_FELICA_RD_MAX_LENGTH);

                                        feliCa->CardParams.CardData[feliCa->CardParams.NrAvailableCards].RDLength = PTX_FELICA_RD_MAX_LENGTH;

                                    }
                                    else
                                    {
                                        feliCa->CardParams.CardData[feliCa->CardParams.NrAvailableCards].RDLength = 0;
                                    }

                                    if (0 == active_card_set)
                                    {
                                        /* Find / Set active card index. If the no FeliCa-/T3T-card is active in the underlying IoT-Reader
                                         * component (valid case!), the first card i.e. index 0 will be used (see card-info-reset above).
                                         */
                                        if (card_registry->ActiveCard == &card_registry->Cards[i])
                                        {
                                            feliCa->CardParams.ActiveCardIndex = feliCa->CardParams.NrAvailableCards;
                                            active_card_set = 1U;
                                        }
                                    }

                                    feliCa->CardParams.NrAvailableCards++;

                                }
                                else
                                {
                                    status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

static uint32_t ptx_FeliCa_INT_CalculateTimeout (uint8_t maxResponseTimeParam, uint8_t n)
{
    /* Calculation according to FeliCa User Manual:
     * T x [(B+1) * n + (A+1)] * 4^E where
     * A = maxResponseTimeParam[2:0] and
     * B = maxResponseTimeParam[5:3] and
     * E = maxResponseTimeParam[7:6].
     */

    /* Term 4^E (see above) can only take on 4 defined values as only 2 Bits are used (see above) */
    const uint8_t MAX_RES_TIME_PARAM_E_TABLE[] = {1U, 4U, 16U, 64U};

    /* The value for T is defined as (256 * 16) / fc = ~302 us, we're adding some delta / tolerance here.
     * By default we use 600 us, but some additional delta can be added as well via a #define constant in the Header-file.
     */
    const uint32_t T = (uint32_t)(600U + PTX_FELICA_T_DELTA_US);

    uint8_t A = (maxResponseTimeParam >> 0) & 0x07;
    uint8_t B = (maxResponseTimeParam >> 3) & 0x07;
    uint8_t E = (maxResponseTimeParam >> 6) & 0x03;

    /* T given in us; divide by 1000 at the end to get ms */
    uint32_t timeout_value = (uint32_t)((T * (uint32_t)(((B + 1) * n + (A + 1))) * MAX_RES_TIME_PARAM_E_TABLE[E]) / 1000);

    /* return calculated value (incl. fallback-value in case of a 0-value) */
    return (0 != timeout_value) ? timeout_value : (uint32_t)1;
}

static ptx_Status_t ptx_FeliCa_INT_SetupCommand (ptx_FeliCa_t *feliCa, uint8_t commandCode)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa))
    {
        /* Command might be the first one after action - update internal card information from underlying platform if available */
        status = ptx_FeliCa_INT_UpdateCardInfo(feliCa);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Any card present at all ? */
            if (0 != feliCa->CardParams.NrAvailableCards)
            {
                /* prepare Tx buffer */
                (void)memset(&feliCa->Tx[0], 0, PTX_FELICA_MIN_WORK_BUFFER_SIZE);

                /* Need to prepend LEN. */

                feliCa->Tx[1] = commandCode;
                ptx_FeliCa_CardData_t *active_card = &feliCa->CardParams.CardData[feliCa->CardParams.ActiveCardIndex];
                (void)memcpy(&feliCa->Tx[2], &active_card->IDm[0], PTX_FELICA_IDM_MAX_LENGTH);

                feliCa->TxLen = (uint32_t)(2U + PTX_FELICA_IDM_MAX_LENGTH);

                feliCa->Tx[0] = (uint8_t)feliCa->TxLen;

            }
            else
            {
                status = PTX_STATUS_E_FELICA_PERMISSION_DENIED;
            }
        }

    }

    return status;
}

static ptx_Status_t ptx_FeliCa_INT_CheckResponse (ptx_FeliCa_t *feliCa)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa))
    {
        feliCa->RxPayload = feliCa->Rx;

        /* anything received at all ? */
        if (0 == feliCa->RxLen)
        {
            status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }

        /* sent command code matches received response code ? */
        if (PTX_STATUS_SUCCESS == status)
        {
            if ((0 != feliCa->RxLen) && (feliCa->Tx[1] == (uint8_t)(feliCa->RxPayload[1] - 1u)))
            {
                /* everything OK - advance payload information */
                feliCa->RxPayload++;
                feliCa->RxLen--;
            }
            else
            {
                status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
            }
        }

        /* sent IDM matches received IDM ? */
        if (PTX_STATUS_SUCCESS == status)
        {
            if (PTX_FELICA_IDM_MAX_LENGTH <= feliCa->RxLen)
            {
                int ret_val = memcmp(&feliCa->RxPayload[1], &feliCa->Tx[2], PTX_FELICA_IDM_MAX_LENGTH);

                if (0 == ret_val)
                {
                    /* everything OK - advance payload information */
                    feliCa->RxPayload = feliCa->RxPayload + PTX_FELICA_IDM_MAX_LENGTH + 1u; /* 1 byte command length information. */
                    feliCa->RxLen     = feliCa->RxLen     - PTX_FELICA_IDM_MAX_LENGTH - 1u; /* 1 byte command length information. */

                }
                else
                {
                    status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
                }
            }
            else
            {
                status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
            }
        }

    }

    return status;
}

static ptx_Status_t ptx_FeliCa_PollCards(ptx_FeliCa_t *feliCa, uint16_t systemCode, uint8_t requestCode, uint8_t timeslotNr)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa))
    {
        feliCa->RxLen = (uint32_t)PTX_FELICA_MIN_WORK_BUFFER_SIZE;

        status = ptx_RUL_T3T_SensFReq(feliCa->Rul, systemCode, requestCode, timeslotNr, &feliCa->Rx[0], &feliCa->RxLen);
    }

    return status;
}

static ptx_Status_t ptx_FeliCa_GetNumberCardResponses(ptx_FeliCa_t *feliCa, uint8_t rdPresent, uint8_t *nrCardResponses)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t expected_single_response_length = (uint32_t)(PTX_FELICA_SENSF_RES_HEADER_LEN + PTX_FELICA_IDM_MAX_LENGTH + PTX_FELICA_PMM_MAX_LENGTH);

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa) && (NULL != nrCardResponses))
    {
        if (0 != rdPresent)
        {
            expected_single_response_length = (uint32_t)(expected_single_response_length + PTX_FELICA_RD_MAX_LENGTH);
        }

        if (0 != feliCa->RxLen)
        {
            if (0 == (feliCa->RxLen % expected_single_response_length))
            {
                *nrCardResponses = (uint8_t)(feliCa->RxLen / expected_single_response_length);
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                /* there must not be any leftovers */
                status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
            }
        }
        else
        {
            /* actually not possible (reception already checked at calling level) - but check again nevertheless */
            status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
        }
    }

    return status;
}

static ptx_Status_t ptx_FeliCa_ParseCardData(ptx_FeliCa_t *feliCa,
                                             uint32_t *parseIndex,
                                             uint8_t rdPresent,
                                             ptx_FeliCa_CardData_t *cardData,
                                             uint8_t *isCollision)
{
    const uint8_t VALID_SENSF_RES_MIN_LEN = (2 + PTX_FELICA_IDM_MAX_LENGTH + PTX_FELICA_PMM_MAX_LENGTH);
    const uint8_t VALID_SENSF_RES_MAX_LEN = (2 + PTX_FELICA_IDM_MAX_LENGTH + PTX_FELICA_PMM_MAX_LENGTH + PTX_FELICA_RD_MAX_LENGTH);

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t parse_index = 0;
    uint8_t current_len = 0;
    uint8_t response_code = 0;
    uint8_t rf_status_offset = ((0 == rdPresent) ? VALID_SENSF_RES_MIN_LEN : VALID_SENSF_RES_MAX_LEN);

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa) && (NULL != parseIndex) && (NULL != cardData) && (NULL != isCollision))
    {
        /* default initialization */
        *isCollision = 0;
        (void)memset(cardData, 0, sizeof(ptx_FeliCa_CardData_t));

        /* Note: No need here for any length-checks on the received data -> this has already been checked by a
         *       previous call to "ptx_FeliCa_GetNumberCardResponses".
         *       Nevertheless, additional protocol / integrity checks get performed in any case.
         */
        parse_index = *parseIndex;

        status = PTX_STATUS_SUCCESS;

        /* first check if the current response is valid at all or if there is a collision with another response in the same timeslot */
        if (0 == feliCa->Rx[rf_status_offset])
        {
            /* LEN-byte OK ? */
            current_len = feliCa->Rx[parse_index];
            parse_index++;

            if ((current_len == VALID_SENSF_RES_MIN_LEN) || (VALID_SENSF_RES_MAX_LEN))
            {
                if (((VALID_SENSF_RES_MIN_LEN == current_len) && (0 != rdPresent)) || ((VALID_SENSF_RES_MAX_LEN == current_len) && (0 == rdPresent)))
                {
                    status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    response_code = feliCa->Rx[parse_index];
                    parse_index++;

                    /* RESPONSE-CODE is fixed set to 01h */
                    if (PTX_FELICA_RSP_POLL == response_code)
                    {
                        /* copy IDM */
                        (void)memcpy(&cardData->IDm[0], &feliCa->Rx[parse_index], PTX_FELICA_IDM_MAX_LENGTH);
                        parse_index = (uint32_t)(parse_index + PTX_FELICA_IDM_MAX_LENGTH);

                        /* copy PMM */
                        (void)memcpy(&cardData->PMm[0], &feliCa->Rx[parse_index], PTX_FELICA_PMM_MAX_LENGTH);
                        parse_index = (uint32_t)(parse_index + PTX_FELICA_PMM_MAX_LENGTH);

                        /* copy RD if present */
                        if (VALID_SENSF_RES_MAX_LEN == current_len)
                        {
                            /* 2 additional bytes available ? */
                            if ((uint32_t)(parse_index + PTX_FELICA_RD_MAX_LENGTH) < feliCa->RxLen)
                            {
                                (void)memcpy(&cardData->RD[0], &feliCa->Rx[parse_index], PTX_FELICA_RD_MAX_LENGTH);
                                cardData->RDLength = (uint8_t)PTX_FELICA_RD_MAX_LENGTH;
                                parse_index = (uint32_t)(parse_index + PTX_FELICA_RD_MAX_LENGTH);

                            }
                            else
                            {
                                status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
                            }
                        }

                        /* skip RF-status byte (already checked above) */
                        parse_index++;

                    }
                    else
                    {
                        status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
                    }
                }
            }

        }
        else
        {
            /* there's a collision - data may not be valid so therefore quit for now */
            *isCollision = 1;

            /* update index for next data/response */
            parse_index = (uint8_t)(rf_status_offset + 1);
        }

        *parseIndex = parse_index;

    }

    return status;
}

static ptx_Status_t ptx_FeliCa_StoreCardData(ptx_FeliCa_t *feliCa, ptx_FeliCa_CardData_t *cardData)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t is_present = 0;

    if (PTX_COMP_CHECK(feliCa, Comp_FeliCa) && (NULL != cardData))
    {
        /* check first if card is already present in internal container */
        for (uint8_t i = 0; i < feliCa->CardParams.NrAvailableCards; i++)
        {
            int ret_val = memcmp(&cardData->IDm[0], &feliCa->CardParams.CardData[i].IDm[0], PTX_FELICA_IDM_MAX_LENGTH);

            if (0 == ret_val)
            {
                is_present = 1;
            }
        }

        if (0 == is_present)
        {
            /* not present - store it now if there is still enough storage places left */
            if (PTX_MAX_SUPPORTED_FELICA_CARDS > feliCa->CardParams.NrAvailableCards)
            {
                uint8_t new_index = feliCa->CardParams.NrAvailableCards;

                (void)memcpy(&feliCa->CardParams.CardData[new_index].IDm[0], &cardData->IDm[0], PTX_FELICA_IDM_MAX_LENGTH);
                (void)memcpy(&feliCa->CardParams.CardData[new_index].PMm[0], &cardData->PMm[0], PTX_FELICA_PMM_MAX_LENGTH);
                (void)memcpy(&feliCa->CardParams.CardData[new_index].RD[0],  &cardData->RD[0],  PTX_FELICA_RD_MAX_LENGTH);
                feliCa->CardParams.CardData[new_index].RDLength = cardData->RDLength;
                feliCa->CardParams.NrAvailableCards++;
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            }
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }

    }

    return status;
}

