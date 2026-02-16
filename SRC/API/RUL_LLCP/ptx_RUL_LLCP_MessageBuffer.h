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
    File        : ptx_RUL_LLCP_MessageBuffer.h

    Description : Implementation of the LLCP Message Buffer required for potential retransmission.
*/

/**
 * \addtogroup RUL_LLCP
 * @{
 */
#if !defined(PTX_RULLLCPMESSAGEBUFFER_H)
#define PTX_RULLLCPMESSAGEBUFFER_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>
#include "ptx_Components.h"
#include "ptx_Status.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_LLCP_MESSAGEBUFFER_MESSAGES         (4u)   /**< Worst case ReceiveWindow (RW) size is 16 */
#define PTX_LLCP_MESSAGEBUFFER_MESSAGELENGTH    (264u)  /**< Default MIU Value is 128 + DSAP + SSAP + SEQUENCE_NUM */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/**
 * \brief LLCP Message Structure
 */
typedef struct ptx_RUL_LLCP_Message
{
    uint8_t     Data[PTX_LLCP_MESSAGEBUFFER_MESSAGELENGTH];         /**< Actual LLCP message (PDU) */
    uint32_t    Length;                                             /**< Length information (Miu is an 11 Bit value). */
} ptx_RUL_LLCP_Message_t;

/**
* \brief LLCP Message Structure
*/
typedef struct ptx_RUL_LLCP_MessageBuffer
{
    ptx_Comps_t             CompId;                                     /**< Component Id */
    ptx_RUL_LLCP_Message_t  Messages[PTX_LLCP_MESSAGEBUFFER_MESSAGES];  /**< In worst case up to 16 messages must be stored. */
    uint32_t                WriteIndex;                                 /**< Write index. */
    uint32_t                ReadIndex;                                  /**< Read index. */
} ptx_RUL_LLCP_MessageBuffer_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initializes the LLCP-MessageBuffer component.
 *
 * This API has to be called before any other API function of this component. It will setup & initialise all the
 * internal data structures.
 *
 * \param[in]       ctx                 Pointer to the LLCP-MessageBuffer component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_MessageBuffer_Init(ptx_RUL_LLCP_MessageBuffer_t *ctx);

/**
 * \brief Writes a message into the buffer.
 *
 * \param[in]       ctx                 Pointer to the LLCP-MessageBuffer component.
 * \param[in]       data                Pointer to the message.
 * \param[in]       dataLength          Length of the message.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_MessageBuffer_Write(ptx_RUL_LLCP_MessageBuffer_t *ctx, uint8_t *data, uint32_t dataLength);

/**
 * \brief Returns/Reads the oldest buffered message (FIFO principle).
 *
 * \param[in]       ctx                 Pointer to the LLCP-MessageBuffer component.
 * \param[in,out]   data                Pointer to store the message.
 * \param[in,out]   dataLength          Pointer to store the message length.
 * \param[in]       removeFromBuffer    Flag to control if the fetched message shall be removed from the buffer.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_MessageBuffer_Read(ptx_RUL_LLCP_MessageBuffer_t *ctx, uint8_t *data, uint32_t *dataLength, uint8_t removeFromBuffer);

/**
 * \brief Deletes all buffered messages.
 *
 * \param[in]       ctx                 Pointer to the LLCP-MessageBuffer component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_MessageBuffer_Clear(ptx_RUL_LLCP_MessageBuffer_t *ctx);

/**
 * \brief Used to determine the amount of buffered messages.
 *
 * \param[in]       ctx                 Pointer to the LLCP-MessageBuffer component.
 * \param[in,out]   size                Pointer to store the amount of buffered messages.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_MessageBuffer_Size(ptx_RUL_LLCP_MessageBuffer_t *ctx, uint32_t *size);

/**
 * \brief Tears the LLCP-MessageBuffer component down.
 *
 * This API tears the complete LLCP-MessageBuffer component down.
 *
 * \param[in]       ctx                 Pointer to the LLCP-MessageBuffer component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_MessageBuffer_DeInit(ptx_RUL_LLCP_MessageBuffer_t *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */