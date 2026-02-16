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
    Module      : RUL Wireless Charging (WLC) Proprietary Extension
    File        : ptx_RUL_WLC_Proprietary.h

    Description : RUL Wireless Charging (WLC) Proprietary Extension implements unique NFC WLC features with the PTX2xxW chip.
                  This module grants the possibility to use proprietary additional features when using the PTX2xxW WLC-P hardware
                  with another Renesas WLC-L hardware (e.g., RRQ36031).
*/

/**
 * \addtogroup RUL_WLC_PROPRIETARY
 * @{
 */

#if !defined(PTX_RULWLCPROPRIETARY_H)
#define PTX_RULWLCPROPRIETARY_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>
#include "ptx_RUL_WLC.h"
#include "ptx_Components.h"
#include "ptx_Status.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name Proprietary T2T Defines.
 * @{
 */
#define PTX_WLC_PROPRIETARY_T2T_UID_SIZE                       (7u)    /**< Length of the proprietary Renesas NFC T2T tags' UID in bytes. */

#define PTX_WLC_PROPRIETARY_T2T_READ_CODE                      (0x3F)   /**< Command Code for proprietary T2T Read operation. */
#define PTX_WLC_PROPRIETARY_T2T_WRITE_CODE                     (0xAF)   /**< Command Code for proprietary T2T Write operation. */

#define PTX_WLC_PROPRIETARY_T2T_HEADER_LEN                     (2u)     /**< Number of Bytes for proprietary T2T operation (Header only). */
#define PTX_WLC_PROPRIETARY_T2T_MAX_PAYLOAD_LEN                (64)     /**< Max. Buffer size for proprietary T2T operation (Payload only). */
#define PTX_WLC_PROPRIETARY_T2T_MAX_MESSAGE_LEN                (PTX_WLC_PROPRIETARY_T2T_HEADER_LEN + PTX_WLC_PROPRIETARY_T2T_MAX_PAYLOAD_LEN)    /**< Max. Buffer size for proprietary T2T operation (Header + Payload). */

#define PTX_WLC_PROPRIETARY_T2T_READ_CMD_PAYLOAD_LEN           (1u)     /**< Number of Bytes for proprietary T2T Read CMD operation (Payload only). */
#define PTX_WLC_PROPRIETARY_T2T_READ_CMD_LEN                   (PTX_WLC_PROPRIETARY_T2T_HEADER_LEN + PTX_WLC_PROPRIETARY_T2T_READ_CMD_PAYLOAD_LEN)     /**< Number of Bytes for proprietary T2T Read CMD operation (Header + Payload). */
#define PTX_WLC_PROPRIETARY_T2T_WRITE_RSP_LEN                  (2u)     /**< Number of Bytes for proprietary T2T Write RSP operation. */

#define PTX_WLC_PROPRIETARY_T2T_OP_CODE_POS                    (0u)     /**< Position of the Operation Code for proprietary T2T operation. */
#define PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_POS                  (1u)     /**< Position of the Block Number for proprietary T2T operation. */
#define PTX_WLC_PROPRIETARY_T2T_PAYLOAD_POS                    (2u)     /**< Position of the Payload for proprietary T2T operation. */

#define PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_TDC_POL              (48u)    /**< Start Block address for proprietary TDC Write Operation */
#define PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_TDC_LIS              (64u)    /**< Start Block address for proprietary TDC Read Operation */
#define PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_SYSTEM_STATUS        (80u)    /**< Start Block address for proprietary Get-Status-Info Operation */
#define PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_SYSTEM_CTRL          (47u)    /**< Start Block address for proprietary Listener System Control Operations. */

#define PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT          (20u)    /**< Default timeout for data exchanges of a proprietary Listener Device. */
/** @} */


/**
 * \name Proprietary TDC Defines.
 * @{
 */
#define PTX_WLC_PROPRIETARY_NSC_DATA_MSG_CODE                  (0x80)   /**< Command Code for internal proprietary Listener Device NSC Data operation. */
#define PTX_WLC_PROPRIETARY_TDC_BUFFER_LEN_MASK                (0x3F)   /**< Byte Mask of Buffer Size value for proprietary TDC Operation. */

#define PTX_WLC_PROPRIETARY_TDC_MAX_PAYLOAD_LEN                (63u)    /**< Max. Buffer size for proprietary TDC operation (Payload only). */
#define PTX_WLC_PROPRIETARY_TDC_HEADER_LEN                     (1u)     /**< Number of Bytes for proprietary TDC operation (Header only). */
#define PTX_WLC_PROPRIETARY_TDC_MAX_MESSAGE_LEN                (PTX_WLC_PROPRIETARY_TDC_HEADER_LEN + PTX_WLC_PROPRIETARY_TDC_MAX_PAYLOAD_LEN)    /**< Max. Buffer size for proprietary TDC operation (Header + Payload). */

#define PTX_WLC_PROPRIETARY_TDC_PAYLOAD_POS                    (1u)     /**< Position of Payload for proprietary TDC Operation. */
#define PTX_WLC_PROPRIETARY_TDC_HEADER_POS                     (0u)     /**< Position of Header for proprietary TDC Operation. */
/** @} */

/**
 * \name Proprietary Listener System Control Defines.
 * @{
 */

#define PTX_WLC_PROPRIETARY_SYSTEM_CTRL_GPIO_0_POS                         (0u)     /**< Position of the GPIO0 Byte in the System Control T2T memory Block. */
#define PTX_WLC_PROPRIETARY_SYSTEM_CTRL_GPIO_1_POS                         (1u)     /**< Position of the GPIO1 Byte in the System Control T2T memory Block. */
#define PTX_WLC_PROPRIETARY_SYSTEM_CTRL_SHIPPING_MODE_ENABLE_POS           (2u)     /**< Position of the Shipping Mode Enable Byte in the System Control T2T memory Block. */


#define PTX_WLC_PROPRIETARY_SYSTEM_CTRL_GPIO_HIGH_LEVEL                    (0x00)   /**< HIGH Value for GPIO Control. */
#define PTX_WLC_PROPRIETARY_SYSTEM_CTRL_GPIO_LOW_LEVEL                     (0x01)   /**< LOW Value for GPIO Control. */

/** @} */

/**
 * \name Proprietary Listener System Status Defines.
 * @{
 */
#define PTX_WLC_PROPRIETARY_SYSTEM_STATUS_NTC_POS              (0u)     /**< Position of the GPIO0 Byte in the GPIO T2T memory Block. */
/** @} */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Listener NTC Status
 */
typedef enum ptx_RUL_WLC_Proprietary_ListenerNtcStatus
{
    NtcStatus_Normal     = 0x00, /*< Listener NTC Status: Normal temperature. */
    NtcStatus_Cold       = 0x02, /*< Listener NTC Status: Under temperature: When set, battery is under 10 degree Celsius. */
    NtcStatus_ECold      = 0x03, /*< Listener NTC Status: Extended under temperature: When set, battery is under 0 degree Celsius. */
    NtcStatus_Hot        = 0x04, /*< Listener NTC Status: Over temperature: When set, battery is above 45 degree Celsius. */
    NtcStatus_EHot       = 0x0C, /*< Listener NTC Status: Extended over temperature: When set, battery is above 60 degree Celsius. */
    NtcStatus_Disabled   = 0xF0  /*< Listener NTC Status: The NTC is disabled. */
} ptx_RUL_WLC_Proprietary_ListenerNtcStatus_t;

/**
 * \brief Listener System Status
 */
typedef struct ptx_RUL_WLC_Proprietary_ListenerSystemStatus
{
    ptx_RUL_WLC_Proprietary_ListenerNtcStatus_t    NtcStatus;  /**< NTC temperature status. */

} ptx_RUL_WLC_Proprietary_ListenerSystemStatus_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Assembles and transmits the proprietary REN_READ_CMD capable of reading 64 bytes at once from the Listener.
 *         Be aware that it is only possible to read block-wise, it is therefore required to reserve enough memory in
 *         rxData to read all required whole blocks.
 *
 * \param[in]        ctx         Pointer to the RUL WLC Component.
 * \param[in]        block       Block number to read the data from.
 * \param[in,out]    rxData      Pointer to a buffer for storing the received data.
 * \param[in,out]    rxDataLen   Pointer to the length of the buffer for storing the received data as input and the actual
 *                               size of the received data as output.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_Proprietary_ReadT2T( ptx_RUL_WLC_t *ctx, uint8_t block, uint8_t *rxData, uint32_t *rxDataLen);

/**
 * \brief Assembles and transmits the proprietary REN_WRITE_CMD capable of writing 64 bytes at once to the Listener.
 *
 * \param[in]   ctx           Pointer to the RUL WLC Component.
 * \param[in]   block         Block number to write the data to.
 * \param[in]   txData        Pointer to  a buffer which stores the to be written data.
 * \param[in]   txDataLen     Length of the data to be written.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_Proprietary_WriteT2T( ptx_RUL_WLC_t *ctx, uint8_t block, uint8_t *txData, uint32_t txDataLen);

/**
 * \brief Controls the output level of the Listener's GPIOs. The Listener's GPIOs must be configured to "WlcpControlled".
 *
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 * \param[in]       setGpio0High    Sets Listener's GPIO0 to HIGH when 1 and to LOW otherwise.
 * \param[in]       setGpio1High    Sets Listener's GPIO1 to HIGH when 1 and to LOW otherwise.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_Proprietary_SetListenerGpios(ptx_RUL_WLC_t *ctx, uint8_t setGpio0High, uint8_t setGpio1High);

/**
 * \brief Enables the Shipping Mode (= "Deep Sleep" Mode) on the Listener to force it into entering this state.
 *
 *
 * \param[in]       ctx                Pointer to the RUL WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_Proprietary_SetListenerShippingModeEnable(ptx_RUL_WLC_t *ctx);

/**
 * \brief Retrieves a system status information from the listener ( e.g. detailed temperature information of the NTC).
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 * \param[in,out]   systemStatus    Pointer to store the status information of the Listener.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_Proprietary_GetListenerSystemStatus(ptx_RUL_WLC_t *ctx, ptx_RUL_WLC_Proprietary_ListenerSystemStatus_t *systemStatus);

/**
 * \brief Writes messages to the Listener's TDC buffer. The customer can choose between NFC Forum compliant write access
 *        and proprietary (= faster) write access via compile switch.
 *
 * \param[in]   ctx             Pointer to the RUL WLC Component.
 * \param[in]   txData          Pointer to a buffer which stores the to be written data.
 * \param[in]   txDataLen       Length of the data to be written.
 * \param[in]   ackTimeoutMs    Timeout to wait for an acknowledgement from the listener (if 0, there will be
 *                              no acknowledgement check).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_Proprietary_TDC_Write(ptx_RUL_WLC_t *ctx, uint8_t *txData, uint8_t txDataLen, uint32_t ackTimeoutMs);

/**
 * \brief Checks if the Listener's host MCU has read the previously sent message and an acknowledgement was received.
 *
 * \param[in]       ctx        Pointer to the RUL WLC Component.
 * \param[in,out]   received   Pointer to store the status, if the message has been read (1 if it was received and 0 if not).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_Proprietary_TDC_IsReceived(ptx_RUL_WLC_t *ctx, uint8_t *received);

/**
 * \brief Reads messages from the Listener's TDC buffer. Customer can choose between NFC Forum compliant read access
 *        and proprietary (= faster) read access via compile switch.
 *
 * \param[in]       ctx             Pointer to the RUL WLC Component.
 * \param[in,out]   rxData          Pointer to a buffer for storing the received data.
 * \param[in,out]   rxDataLen       Pointer to the length of the buffer for storing the received data as input and the actual
 *                                  size of the received data as output.
 * \param[in]       rxTimeoutMs     Timeout for the message to be read from the Listener.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RUL_WLC_Proprietary_TDC_Read(ptx_RUL_WLC_t *ctx, uint8_t *rxData, uint8_t *rxDataLen, uint32_t rxTimeoutMs);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

