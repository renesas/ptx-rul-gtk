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
    Module      : Host Interface Protocol (HIP) Frame Control
    File        : ptx_HIP_Frame.h

    Description : Host Interface Protocol (HIP) Frame Control handles the correct en-/decoding of each sent and
                  received communication frame with the PTX2xxR/W hardware. It enables frame configuration
                  for certain aspects such as CRC and ACK.
*/

/**
 * \addtogroup HIP
 *
 * @{
 */

#if !defined(PTX_HIP_FRAME_H)
#define PTX_HIP_FRAME_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stddef.h>

#include "ptx_Status.h"
#include "ptx_Components.h"
#include "ptx_HIP_Int.h"
#include "ptx_Log.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

#define PTX_HIP_FRAME_CMD_IS_READ   (0u)    /**< Read Flag */
#define PTX_HIP_FRAME_CMD_IS_WRITE  (1u)    /**< Write Flag */
#define PTX_HIP_HEADER_AND_FOOTER_LENGTH       (3u     /* HIP Header (Length 2By + FCB 1By) */ + 2u /* CRC (Optional)*/)    /**< HIP Header Length */
#define PTX_HIP_DUMMY_BYTE_LENGTH   (1u)    /**< HIP Dummy Byte Length */
#define PTX_HIP_MAX_PAYLOAD_LENGTH  (266u)  /**< HIP Maximum Payload Length - coming from Write Code Memory command:
                                                 PTX input buffer address (2By) + Max. length NSC message (264By) */
#define PTX_HIP_MAX_ADDRESS_LENGTH  (2u)    /**< HIP Maximum Address Length in Payload */
#define PTX_HIP_MAX_FRAME_LENGTH    (PTX_HIP_MAX_PAYLOAD_LENGTH + PTX_HIP_HEADER_AND_FOOTER_LENGTH + PTX_HIP_DUMMY_BYTE_LENGTH) /**< Maximum allowed frame length 272*/

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief HAL Forward declaration.
 */
typedef struct ptx_HAL ptx_HAL_t;

struct ptx_HIP_Frame;

/**
 * \brief HIP Frame Status codes
 */
typedef enum ptx_HIP_Frame_Status
{
    HIP_Frame_Status_ACK          = 0x00,
    HIP_Frame_Status_InvalidCmd   = 0x01,
    HIP_Frame_Status_InvalidLen   = 0x02,
    HIP_Frame_Status_ErrorCRC     = 0x03,
    HIP_Frame_Status_InvalidParam = 0x04,
    HIP_Frame_Status_WBufFull     = 0x05,
    HIP_Frame_Status_MAX          = 0x06
} ptx_HIP_Frame_Status_t;


/**
 * \brief Encode HIP Frame Callback
 *
 * \param[in]       frame           Pointer to component.
 * \param[in]       cmd             Command code
 * \param[in]       addr            Pointer to address
 * \param[in]       addrLen         Length of address pointer.
 * \param[out]      txBuf           Pointer to tx buffer
 * \param[in]       txLen           Length of tx buffer
 * \param[out]      len             Actual length of data in tx buffer.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HIP_Frame_EncodeCb_t) (struct ptx_HIP_Frame *frame, ptx_HIP_Cmd_t cmd, uint8_t *addr, uint32_t addrLen, uint8_t *txBuf, uint32_t txLen, uint16_t *len);

/**
 * \brief Frame configuration
 */
typedef enum ptx_HIP_Frame_Cfg
{
    HIP_Frame_Cfg_ReqAck,
    HIP_Frame_Cfg_AddCrc,

    HIP_Frame_Cfg_MAX,
} ptx_HIP_Frame_Cfg_t;

/**
 * \brief Frame Data
 */
typedef struct ptx_HIP_FrameData
{
    ptx_HIP_Cmd_t   Cmd;        /**< Command */
    uint8_t         IsWriteCmd; /**< Flag write command if set */
    uint8_t         *Addr;      /**< Address pointer */
    uint32_t        AddrLen;    /**< Address pointer length */
    uint8_t         *TxBuf;     /**< Pointer to TX buffer */
    uint32_t        TxBufLen;   /**< Length of TX buffer */
    uint8_t         *RxBuf;     /**< Pointer to RX buffer */
    uint32_t        RxBufLen;   /**< Length of RX buffer */
} ptx_HIP_FrameData_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HIP_Frame
{
    ptx_Comps_t                 CompId;                                     /**< Component ID */

    ptx_HAL_t                   *Hal;                                       /**< Hardware Abstraction Layer, required for communication */

    ptx_HIP_Frame_EncodeCb_t    Encode;                                     /**< Interface function for the encoding of a frame */

    uint8_t                     ReqAck;                                     /**< Flag to request an ack/nack response as answer, if set to != 0 */
    uint8_t                     AddCrc;                                     /**< Flag to add an CRC to the frame, if set to != 0 */

    uint8_t                     ScratchPad[PTX_HIP_MAX_FRAME_LENGTH];       /**< Scratchpad, to form the frame, or receive a frame */
    uint32_t                    ScratchPadSize;                             /**< Scratchpad size */
    ptx_Log_t                   *Log;                                       /**< Initialised Log Context */

} ptx_HIP_Frame_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HIP_Frame_InitParams
{
    ptx_HAL_t                   *Hal;             /**< Pointer to an initialised HAL instance */

    ptx_HIP_Frame_EncodeCb_t    EncodeCb;         /**< Pointer to Encode function */
    ptx_Log_t                  *Log;              /**< Pointer to logger context. */


} ptx_HIP_Frame_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[in]       frame        Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_Frame_Open (ptx_HIP_Frame_t *frame, ptx_HIP_Frame_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       frame        Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_Frame_Close (ptx_HIP_Frame_t *frame);


/**
 * \brief Transmit and Receive data
 *
 * \param[in]       frame        Pointer to component.
 * \param[in,out]   data         Pointer to the frame data.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_Frame_TRx (ptx_HIP_Frame_t *frame, ptx_HIP_FrameData_t *data);

/**
 * \brief Configure a Frame
 *
 * \param[in]       frame           Pointer to component.
 * \param[in]       var             Pointer to configuration
 * \param[in]       val             Pointer to configuration value
 * \param[in]       cnt             Number of configuration parameters in the arrays var and val.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_Frame_Cfg (ptx_HIP_Frame_t *frame, ptx_HIP_Frame_Cfg_t *var, uint8_t *val, uint32_t cnt);

#if defined(PTX_ENABLE_SPI)
/**
 * \brief Encode Data to HIP Frame for SPI transmission
 *
 * \param[in]       frame           Pointer to component.
 * \param[in]       cmd             HIP Command code
 * \param[in]       addr            Pointer to address buffer.
 * \param[in]       addrLen         Length of address buffer
 * \param[in]       txBuf           Pointer to the data to be encoded.
 * \param[in]       txLen           Length of the data buffer.
 * \param[out]      len             Length of the resulting HIP frame.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
ptx_Status_t ptx_HIP_Frame_Encode_SPI (ptx_HIP_Frame_t *frame, ptx_HIP_Cmd_t cmd, uint8_t *addr, uint32_t addrLen, uint8_t *txBuf, uint32_t txLen, uint16_t *len);
#endif
#if defined(PTX_ENABLE_UART)
/**
 * \brief Encode Data to HIP Frame for UART transmission
 *
 * \param[in]       frame           Pointer to component.
 * \param[in]       cmd             HIP Command code
 * \param[in]       addr            Pointer to address buffer.
 * \param[in]       addrLen         Length of address buffer
 * \param[in]       txBuf           Pointer to the data to be encoded.
 * \param[in]       txLen           Length of the data buffer.
 * \param[out]      len             Length of the resulting HIP frame.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
ptx_Status_t ptx_HIP_Frame_Encode_UART (ptx_HIP_Frame_t *frame, ptx_HIP_Cmd_t cmd, uint8_t *addr, uint32_t addrLen, uint8_t *txBuf, uint32_t txLen, uint16_t *len);
#endif
#if defined(PTX_ENABLE_I2C)
/**
 * \brief Encode Data to HIP Frame for I2C transmission
 *
 * \param[in]       frame           Pointer to component.
 * \param[in]       cmd             HIP Command code
 * \param[in]       addr            Pointer to address buffer.
 * \param[in]       addrLen         Length of address buffer
 * \param[in]       txBuf           Pointer to the data to be encoded.
 * \param[in]       txLen           Length of the data buffer.
 * \param[out]      len             Length of the resulting HIP frame.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
ptx_Status_t ptx_HIP_Frame_Encode_I2C (ptx_HIP_Frame_t *frame, ptx_HIP_Cmd_t cmd, uint8_t *addr, uint32_t addrLen, uint8_t *txBuf, uint32_t txLen, uint16_t *len);
#endif
#ifdef PTX_ENABLE_I3C
/**
 * \brief Encode Data to HIP Frame for I3C transmission
 *
 * \param[in]       frame           Pointer to component.
 * \param[in]       cmd             HIP Command code
 * \param[in]       addr            Pointer to address buffer.
 * \param[in]       addrLen         Length of address buffer
 * \param[in]       txBuf           Pointer to the data to be encoded.
 * \param[in]       txLen           Length of the data buffer.
 * \param[out]      len             Length of the resulting HIP frame.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
ptx_Status_t ptx_HIP_Frame_Encode_I3C (ptx_HIP_Frame_t *frame, ptx_HIP_Cmd_t cmd, uint8_t *addr, uint32_t addrLen, uint8_t *txBuf, uint32_t txLen, uint16_t *len);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
