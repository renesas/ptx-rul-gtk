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
    Module      : NFC Soft Controller (NSC) Internal Interface Header for Error Management
    File        : ptx_NSC_Error_Int.h

    Description : Definition of NFC Soft Controller Internal Interface for Error Management.
                  Contains specific type definitions used for error handling.
*/

/**
 * \addtogroup NSC_COMPONENT
 *
 * @{
 */

#if !defined(PTX_NSC_ERROR_INT_H)
#define PTX_NSC_ERROR_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
/**
 * \name Length, mask, and offset definitions.
 * @{
 */
#define PTX_NSC_ERROR_NTF_PARAMLENGTH   (1u)    /**< Error NTF parameter length. */
#define PTX_NSC_ERROR_NTF_ERROR_INDEX   (0u)    /**< Error byte index. */
#define PTX_NSC_ERROR_NTF_RF_ERROR_MASK (0x10u) /**< Bit-mask indicating that occurred error is an RF-Error. */
/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief NSC protocol error codes.
 */
typedef enum ptx_NSC_ErrorCodes
{
    NSC_ErrorCode_NoErr                     = 0x00u,      /**< No error. Command executed successfully, without errors. */
    NSC_ErrorCode_InvalidCommand            = 0x01u,      /**< Invalid command. */
    NSC_ErrorCode_InvalidParameter          = 0x02u,      /**< Invalid parameter. */
    NSC_ErrorCode_StandbyMode               = 0x08u,      /**< Device in standby mode. */
    NSC_ErrorCode_RfCollisionError          = 0x11u,      /**< RF collision error (valid for EMVCo poll mode). */
    NSC_ErrorCode_RfTimeoutError            = 0x12u,      /**< RF timeout error. */
    NSC_ErrorCode_RfTransmissionError       = 0x13u,      /**< RF transmission error. */
    NSC_ErrorCode_RfProtocolError           = 0x14u,      /**< RF protocol error. */
    NSC_ErrorCode_OverTemperature           = 0x31u,      /**< Over temperature protection error. */
    NSC_ErrorCode_OverCurrent               = 0x32u,      /**< Over current protection error. */
    NSC_ErrorCode_VddTx                     = 0x33u,      /**< VDD Tx error. */
    NSC_ErrorCode_OTPConfig                 = 0x34u,      /**< OTP configuration error. */
    NSC_ErrorCode_OutOfMemory               = 0x35u,      /**< Device ran out of memory (e.g. Data from NFC-HW not read fast enough by Host/Application). */
    NSC_ErrorCode_UnknownError              = 0xFFu,      /**< Unknown error. */
} ptx_NSC_ErrorCodes_t;

/**
 * \brief  Error Notification.
 */
typedef struct ptx_NSC_Error_Ntf
{
    ptx_NSC_ErrorCodes_t Error;     /**< Error Code. */
} ptx_NSC_Error_Ntf_t;

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
