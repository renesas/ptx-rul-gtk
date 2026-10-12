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
    Module      : LLCP
    File        : ptx_RUL_LLCP_ServiceAccessPoint.h

    Description : Service Access Point (SAP) implementation.
*/

/**
 * \addtogroup RUL_LLCP
 * @{
 */

#if !defined(PTX_RULLLCPSERVICEACCESSPOINT_H)
#define PTX_RULLLCPSERVICEACCESSPOINT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>

#include "ptx_Components.h"
#include "ptx_Status.h"

#include "ptx_RUL_LLCP_TrxState.h"
#include "ptx_RUL_LLCP_MessageBuffer.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */


/**
 * \brief LLCP Service Access Point Initialisation Parameters
 */
typedef struct ptx_RUL_LLCP_ServiceAccessPoint_InitParams
{
    uint8_t     Address;    /**< SAP Address */
    uint8_t     Window;     /**< SAP Receive Window */
    uint8_t     Miu;        /**< SAP Maximum Information Unit */
} ptx_RUL_LLCP_ServiceAccessPoint_InitParams_t;

/**
 * \brief LLCP Service Access Point Structure
 */
typedef struct ptx_RUL_LLCP_ServiceAccessPoint
{
    ptx_Comps_t                         CompId;         /**< Component Id */
    ptx_RUL_LLCP_MessageBuffer_t        MessageBuffer;  /**< Message Buffer for potential retransmission of already sent frames. */
    uint8_t                             Window;         /**< Receive Window Size */
    uint8_t                             Busy;           /**< Storing the information if the remote SAP is busy. */
    uint8_t                             Address;        /**< Address of the local (= this) Service Access Point (SAP). */
    uint16_t                            Miu;            /**< Transmission unit size of the local Service Access Point (SAP). */
} ptx_RUL_LLCP_ServiceAccessPoint_t;

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initializes the LLCP-ServiceAccessPoint component.
 *
 * This API has to be called before any other API function of this component. It will setup & initialise all the
 * internal data structures.
 *
 * \param[in]       ctx                 Pointer to the LLCP-ServiceAccessPoint component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_Init(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx);

/**
 * \brief Checks if the SAP is of type "WellKnown"
 *
 * \param[in]       ctx                 Pointer to the LLCP-ServiceAccessPoint component.
 * \param[in,out]   isWellKnown         Pointer to store the information whether SAP is of type "Wellknown".
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_IsWellKnown(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx, uint8_t *isWellKnown);

/**
 * \brief Registers a given SAP. Sets the addres, receive window and MIU.
 *
 * \param[in]       ctx                 Pointer to the LLCP-ServiceAccessPoint component.
 * \param[in]       initParams          Pointer to the initialisation parameters for the given SAP.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_Register(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx, ptx_RUL_LLCP_ServiceAccessPoint_InitParams_t *initParams);

/**
 * \brief Writes data into the message buffer of a given SAP
 *
 * \param[in]       ctx                 Pointer to the LLCP-ServiceAccessPoint component.
 * \param[in]       data                Pointer to the data that shall be stored within the message buffer.
 * \param[in]       dataLength          Length of the data to be stored.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_WriteMessageBuffer(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx, uint8_t *data, uint32_t dataLength);

/**
 * \brief Reads data from the message buffer of a given SAP
 *
 * \param[in]       ctx                 Pointer to the LLCP-ServiceAccessPoint component.
 * \param[in]       data                Pointer to store the data to be read.
 * \param[in,out]   dataLength          Pointer to store the length information.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_ReadMessageBuffer(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx, uint8_t *data, uint32_t *dataLength);

/**
 * \brief Tears the LLCP-ServiceAccessPoint component down.
 *
 * This API tears the complete LLCP-ServiceAccessPoint component down.
 *
 * \param[in]       ctx                 Pointer to the LLCP-ServiceAccessPoint component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_ServiceAccessPoint_DeInit(ptx_RUL_LLCP_ServiceAccessPoint_t *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */