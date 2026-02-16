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
    File        : ptx_RUL_LLCP_Frame.h

    Description : Implementation to assemble and parse LLCP frames.
*/

/**
 * \addtogroup RUL_LLCP
 * @{
 */

#if !defined(PTX_LLCPFRAME_H)
#define PTX_LLCPFRAME_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_RUL_LLCP_TLV.h"
#include "ptx_RUL_LLCP_Int.h"
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_LLCP_FRAME_BUFFER_LEN (264u)  /**< Length of the PDU buffer */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */
typedef struct ptx_RUL_LLCP_Frame
{
    ptx_Comps_t CompId;                                 /**< Component ID */
    uint32_t    ScratchPadSize;                         /**< Scratchpad size (= Size of the buffer). */
    uint32_t    ScratchPadIndex;                        /**< Number of used bytes within the buffer. */
    uint8_t     ScratchPad[PTX_LLCP_FRAME_BUFFER_LEN];  /**< Scratchpad, to build the frame, or parse a frame. */
} ptx_RUL_LLCP_Frame_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initializes the LLCP-Frame component.
 *
 * This API has to be called before any other API function of this component. It will setup & initialise all the
 * internal data structures.
 *
 * \param[in]       ctx                 Pointer to the LLCP-Frame component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Frame_Open (ptx_RUL_LLCP_Frame_t *ctx);

/**
 * \brief Tears the LLCP-Frame component down.
 *
 * This API tears the complete LLCP-Frame component down.
 *
 * \param[in]       ctx         Pointer to the LLCP-Frame component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Frame_Close (ptx_RUL_LLCP_Frame_t *ctx);

/**
 * \brief Writes a given PDU strcuture into the LLCP frame buffer.
 *
 * \param[in]       ctx         Pointer to the LLCP-Frame component.
 * \param[in]       pdu         Pointer to a PDU structure to be written to the internal buffer.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Frame_WritePdu (ptx_RUL_LLCP_Frame_t *ctx, ptx_RUL_LLCP_PDU_t *pdu);

/**
 * \brief Parses PDUs from the internal byte buffer into \ref ptx_RUL_LLCP_PDU_t structures.
 *        The method will also split AGFs into it's individual PDUs.
 *
 * \param[in]       ctx         Pointer to the LLCP-Frame component.
 * \param[in,out]   pdus        Pointer to an array of PDU structures for storing the parsed data.
 * \param[in,out]   pdusLength  Pointer to store the amount of PDUs.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_LLCP_Frame_Parse (ptx_RUL_LLCP_Frame_t *ctx, ptx_RUL_LLCP_PDU_t *pdus, uint32_t *pdusLength);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
