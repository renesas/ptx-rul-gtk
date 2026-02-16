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
    Module      : NFC Soft Controller (NSC) Internal Interface Header for Data Exchange
    File        : ptx_NSC_Data_Int.h

    Description : Definition of NFC Soft Controller Internal Interface Header for Data Exchange.
                  Contains type definitions specific for Data message handling.
*/

/**
 * \addtogroup NSC_DATA_TYPES
 *
 * @{
 */

#if !defined(PTX_NSC_DATA_INT_H)
#define PTX_NSC_DATA_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>

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
 * \brief RF Data message type: control message or regular data.
 */
typedef enum ptx_NSC_RfData_Msg_Type
{
    ptx_NSC_RfData_Msg_Type_None,
    ptx_NSC_RfData_Msg_Type_Data,
    ptx_NSC_RfData_Msg_Type_Control,
    ptx_NSC_RfData_Msg_Type_Undefined,
} ptx_NSC_RfData_Msg_Type_t;

/**
 * \brief RF Data message status-byte-present indicator.
 */
typedef enum ptx_NSC_RfData_Msg_StatusBy
{
    ptx_NSC_RfData_Msg_StatusBy_NotPresent,
    ptx_NSC_RfData_Msg_StatusBy_Present,
    ptx_NSC_RfData_Msg_StatusBy_Undefined,
} ptx_NSC_RfData_Msg_StatusBy_t;

/**
 * \brief RF Data message chaining indicator.
 */
typedef enum ptx_NSC_RfData_Msg_Chaining
{
    ptx_NSC_RfData_Msg_Chaining_None,
    ptx_NSC_RfData_Msg_Chaining_Used,
    ptx_NSC_RfData_Msg_Chaining_Undefined,
} ptx_NSC_RfData_Msg_Chaining_t;

/**
 * \brief Structure representing FCB byte (first byte) of data message.
 */
typedef struct ptx_NSC_RfData_Msg_FCB
{
    ptx_NSC_RfData_Msg_Type_t           MsgType;            /**< Message type */
    ptx_NSC_RfData_Msg_StatusBy_t       StatusPresent;      /**< Status byte present */
    ptx_NSC_RfData_Msg_Chaining_t       Chaining;           /**< Chaining */
} ptx_NSC_RfData_Msg_FCB_t;

/**
 * \brief Data length/Control data byte of data message.
 */
typedef union ptx_NSC_RfData_Msg_CtrlLen
{
    uint8_t                             DataLen;            /**< Data length */
    uint8_t                             CtrlData;           /**< Control data: ACK, NAK etc */
} ptx_NSC_RfData_Msg_CtrlLen_t;

/**
 * \brief Structure representing the data message.
 */
typedef struct ptx_NSC_RfData_Msg
{
    ptx_NSC_RfData_Msg_FCB_t            Fcb;                /**< Frame control parameter */
    ptx_NSC_RfData_Msg_CtrlLen_t        CtrlLen;            /**< Data length (Ctrl data) parameter */
    uint8_t                             *Data;              /**< Payload */
    uint8_t                             Status;             /**< Status byte */
} ptx_NSC_RfData_Msg_t;

/**
 * \brief Enable/Disable Clt Mode.
 */
typedef enum ptx_NSC_CltMode
{
    NSC_CltMode_On,     /**< Enable. */
    NSC_CltMode_Off,    /**< Disable. */

    NSC_CltMode_Max,    /**< Max. */
} ptx_NSC_CltMode_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
