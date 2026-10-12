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
    File        : ptx_HIP_Frame.c

    Description : Host Interface Protocol (HIP) Frame Control handles the correct en-/decoding of each sent and
                  received communication frame with the PTX2xxR/W hardware. It enables frame configuration
                  for certain aspects such as CRC and ACK.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <string.h>

#include "ptx_HAL.h"
#include "ptx_HIP_Frame.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_HIP_PAYLOAD_MAX_LEN                 (4095u)

#define PTX_HIP_FRAME_LEN_SIZE                  (2u)
#define PTX_HIP_FRAME_LEN_OFFSET_HI             (0u)
#define PTX_HIP_FRAME_LEN_OFFSET_LO             (1u)

#define PTX_HIP_FRAME_UART_SOF                  (0x55)

#define PTX_HIP_RX_FCB_OFFSET                   (2u)
#define PTX_HIP_FCB_LEN                         (1u)

#define PTX_HIP_FCB_CRC_MASK                    (0x02)
#define PTX_HIP_FCB_ACK_MASK                    (0x04)
#define PTX_HIP_FCB_CMD_MASK                    (0xF0)

#define PTX_HIP_FCB_CRC_SHIFT                   (1u)
#define PTX_HIP_FCB_ACK_SHIFT                   (2u)
#define PTX_HIP_FCB_CMD_SHIFT                   (4u)

#define PTX_HIP_ACK_LEN                         (1u)
#define PTX_HIP_CRC_LEN                         (2u)

#define PTX_HIP_CRC_L_BYTE_OFFSET               (2u)
#define PTX_HIP_CRC_H_BYTE_OFFSET               (1u)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_HIP_Frame_Encode (ptx_HIP_Frame_t *frame, ptx_HIP_Cmd_t cmd, uint8_t *addr, uint32_t addrLen, uint8_t *txBuf, uint32_t txLen, uint16_t scratchPadStartPos, uint16_t *len);

static ptx_Status_t ptx_HIP_Frame_Decode (ptx_HIP_Frame_t *frame, uint8_t *buf, uint32_t len, ptx_HIP_FrameData_t *data);

static ptx_Status_t ptx_HIP_Frame_CheckAckByte (uint8_t ackByte);

static uint16_t ptx_HIP_Frame_CalcCRC16 (uint8_t *data, uint32_t len);

static uint16_t ptx_HIP_Frame_CalcRxFrameLen(ptx_HIP_Frame_t *frame, ptx_HIP_FrameData_t *frameData);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_HIP_Frame_Open (ptx_HIP_Frame_t *frame, ptx_HIP_Frame_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     *  Since HIP_Frame_TRx relies on valid hal component and encode CB, do not propagate SUCCESS further. Check the parameters and
     *  return error status if they are not valid.
     */

    if ((NULL != frame) && (NULL != initParams) && (NULL != initParams->Hal) && (NULL != initParams->EncodeCb) && (NULL != initParams->Log))
    {
        PTX_LOG_V(initParams->Log, "ptx_HIP_Frame_Open");

        frame->CompId = Comp_HIP;

        frame->ReqAck = 0u;
        frame->AddCrc = 0u;

        frame->Hal            = initParams->Hal;
        frame->Encode         = initParams->EncodeCb;
        frame->Log            = initParams->Log;

        frame->ScratchPadSize = PTX_HIP_MAX_FRAME_LENGTH;
        memset(frame->ScratchPad, 0, sizeof(uint8_t)*PTX_HIP_MAX_FRAME_LENGTH);
        
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HIP_Frame_Close (ptx_HIP_Frame_t *frame)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(frame, Comp_HIP))
    {
        PTX_LOG_V(frame->Log, "ptx_HIP_Frame_Close");

        frame->CompId = 0;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HIP_Frame_TRx (ptx_HIP_Frame_t *frame, ptx_HIP_FrameData_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(frame, Comp_HIP) && (NULL != frame->Encode) && (HIP_Cmd_Rst <= data->Cmd) && (HIP_Cmd_MAX > data->Cmd))
    {
        PTX_LOG_V(frame->Log, "ptx_HIP_Frame_TRx");

        uint16_t tx_frame_len = 0;
        uint16_t rx_frame_len = ptx_HIP_Frame_CalcRxFrameLen(frame, data);

        status = frame->Encode(frame, data->Cmd, data->Addr, data->AddrLen, data->TxBuf, data->TxBufLen, &tx_frame_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t *tx[1] = {frame->ScratchPad};
            uint32_t tx_len[1] = { (uint32_t) (tx_frame_len) };
            uint8_t *rx[1] = {(rx_frame_len > 0) ? frame->ScratchPad : NULL};
            uint32_t rx_len[1] = {rx_frame_len};
            uint8_t rx_buff_cnt = (rx_len[0] > 0) ? (1) : (0);

            status = ptx_HAL_Interface_TRx(frame->Hal, tx, tx_len, 1, rx, rx_len, rx_buff_cnt);
        }

        if ((PTX_STATUS_SUCCESS == status) && ((1 == frame->ReqAck) || (PTX_HIP_FRAME_CMD_IS_READ == data->IsWriteCmd)))
        {
            status = ptx_HIP_Frame_Decode(frame, frame->ScratchPad, rx_frame_len, data);
        }
    }

    return status;
}

ptx_Status_t ptx_HIP_Frame_Cfg (ptx_HIP_Frame_t *frame, ptx_HIP_Frame_Cfg_t *var, uint8_t *val, uint32_t cnt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(frame, Comp_HIP) && (NULL != var) && (NULL != val))
    {
        PTX_LOG_V(frame->Log, "ptx_HIP_Frame_Cfg");

        for (uint32_t i = 0; i < cnt; ++i)
        {
            switch (var[i])
            {
                case HIP_Frame_Cfg_ReqAck:
                    frame->ReqAck = val[i];
                    status = PTX_STATUS_SUCCESS;
                    break;

                case HIP_Frame_Cfg_AddCrc:
                    frame->AddCrc = val[i];
                    status = PTX_STATUS_SUCCESS;
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }

            if (PTX_STATUS_SUCCESS != status)
            {
                /* Abort the loop in case of an invalid parameter */
                break;
            }
        }
    }

    return status;
}

#if defined(PTX_ENABLE_SPI)
ptx_Status_t ptx_HIP_Frame_Encode_SPI (ptx_HIP_Frame_t *frame, ptx_HIP_Cmd_t cmd, uint8_t *addr, uint32_t addrLen, uint8_t *txBuf, uint32_t txLen, uint16_t *len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(frame, Comp_HIP))
    {
        PTX_LOG_V(frame->Log, "ptx_HIP_Frame_Encode_SPI");

        uint16_t scratchPadPos = 0;
        status = ptx_HIP_Frame_Encode(frame, cmd, addr, addrLen, txBuf, txLen, scratchPadPos, len);

        if (PTX_STATUS_SUCCESS == status)
        {
            // Dummy Byte
            frame->ScratchPad[*len] = 0x00;
            *len = (uint16_t)(*len + 1);
        }
    }

    return status;
}
#endif
#if defined(PTX_ENABLE_UART)
ptx_Status_t ptx_HIP_Frame_Encode_UART (ptx_HIP_Frame_t *frame, ptx_HIP_Cmd_t cmd, uint8_t *addr, uint32_t addrLen, uint8_t *txBuf, uint32_t txLen, uint16_t *len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(frame, Comp_HIP))
    {
        PTX_LOG_V(frame->Log, "ptx_HIP_Frame_Encode_UART");

        // UART SoF
        uint16_t scratchPadPos = 0;
        frame->ScratchPad[scratchPadPos] = PTX_HIP_FRAME_UART_SOF;
        scratchPadPos++;

        status = ptx_HIP_Frame_Encode(frame, cmd, addr, addrLen, txBuf, txLen, scratchPadPos, len);
    }

    return status;
}
#endif
#if defined(PTX_ENABLE_I2C)
ptx_Status_t ptx_HIP_Frame_Encode_I2C (ptx_HIP_Frame_t *frame, ptx_HIP_Cmd_t cmd, uint8_t *addr, uint32_t addrLen, uint8_t *txBuf, uint32_t txLen, uint16_t *len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(frame, Comp_HIP))
    {
        PTX_LOG_V(frame->Log, "ptx_HIP_Frame_Encode_I2C");

        uint16_t scratchPadPos = 0;
        status = ptx_HIP_Frame_Encode(frame, cmd, addr, addrLen, txBuf, txLen, scratchPadPos, len);
    }

    return status;
}
#endif
#if defined(PTX_ENABLE_I3C)
ptx_Status_t ptx_HIP_Frame_Encode_I3C (ptx_HIP_Frame_t *frame, ptx_HIP_Cmd_t cmd, uint8_t *addr, uint32_t addrLen, uint8_t *txBuf, uint32_t txLen, uint16_t *len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(frame, Comp_HIP))
    {
        uint16_t scratchPadPos = 0;
        status = ptx_HIP_Frame_Encode(frame, cmd, addr, addrLen, txBuf, txLen, scratchPadPos, len);
    }

    return status;
}
#endif


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

/*
 * Frame Format, Frame length must not exceed 4095 bytes! Padding is optional and not part of the length.
 * Length contains FCB, Payload, and CRC.
 *
 * +-----+-----+-----+-----+-----+---||---+-----+-----+-----+||-----+-----+
 * |    LEN    | FCB |             Payload      |    CRC    |||  Padding  |
 * +-----+-----+-----+-----+-----+---||---+-----+-----+-----+||-----+-----+
 *
 * Frame Control Byte Layout
 *
 *    7     6     5     4     3     2     1     0      bit
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 * |        OPCODE         | RFU | RAK | CRC | CB  |   Description
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 *
 * OPCODE ... Defines a type of Command
 * RFU    ... Reserved for future use
 * RAK    ... Request Acknowledge
 * CRC    ... Cyclic Redundancy Check (calculated as ISO/IEC 14443-3, Type-A standard; CRC_A)
 * CB     ... Chaining Bit
 */
static ptx_Status_t ptx_HIP_Frame_Encode(ptx_HIP_Frame_t *frame, ptx_HIP_Cmd_t cmd, uint8_t *addr, uint32_t addrLen, uint8_t *txBuf, uint32_t txLen, uint16_t scratchPadStartPos, uint16_t *len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(frame, Comp_HIP))
    {
        PTX_LOG_V(frame->Log, "ptx_HIP_Frame_Encode");

        uint8_t *frame_ptr = &frame->ScratchPad[scratchPadStartPos];

        uint32_t frame_length = PTX_HIP_FRAME_LEN_SIZE;

        uint8_t fcb = (uint8_t)(0x00
                                | ((((uint8_t)cmd) << PTX_HIP_FCB_CMD_SHIFT) & PTX_HIP_FCB_CMD_MASK)
                                | ((frame->ReqAck << PTX_HIP_FCB_ACK_SHIFT) & PTX_HIP_FCB_ACK_MASK)
                                | ((frame->AddCrc << PTX_HIP_FCB_CRC_SHIFT) & PTX_HIP_FCB_CRC_MASK));

        frame_ptr[frame_length] = fcb;
        frame_length += PTX_HIP_FCB_LEN;

        if (NULL != addr)
        {
            if (PTX_HIP_MAX_ADDRESS_LENGTH >= addrLen)
            {
                memcpy(&frame_ptr[frame_length], addr, addrLen);
                frame_length += addrLen;

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

        if ((NULL != txBuf) && (PTX_STATUS_SUCCESS == status))
        {
            if (PTX_HIP_MAX_PAYLOAD_LENGTH >= (addrLen + txLen))
            {
                memcpy(&frame_ptr[frame_length], txBuf, txLen);
                frame_length += txLen;

                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            // We need to know the actual frame length up front, as CRC is calculated over the whole thing!
            frame_length += (0 == frame->AddCrc) ? 0 : PTX_HIP_CRC_LEN;

            uint32_t act_frame_length = frame_length - PTX_HIP_FRAME_LEN_SIZE;

            uint16_t *frame_len_buffer = (uint16_t*)&act_frame_length;
            PTX_HIP_BSWAP16(*frame_len_buffer);
            memcpy(frame_ptr, frame_len_buffer, PTX_HIP_FRAME_LEN_SIZE);

            if (0 != frame->AddCrc)
            {
                uint16_t crc = ptx_HIP_Frame_CalcCRC16(frame_ptr, frame_length - PTX_HIP_CRC_LEN);
                frame_ptr[frame_length - PTX_HIP_CRC_L_BYTE_OFFSET] = (uint8_t)(crc & 0x00FF);
                frame_ptr[frame_length - PTX_HIP_CRC_H_BYTE_OFFSET] = (uint8_t)((crc & 0xFF00) >> 8);
            }

            *len = (uint16_t)(frame_length + scratchPadStartPos);
        }
    }

    return status;
}

static ptx_Status_t ptx_HIP_Frame_Decode (ptx_HIP_Frame_t *frame, uint8_t *buf, uint32_t len, ptx_HIP_FrameData_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(frame, Comp_HIP) && (NULL != buf) && (NULL != data))
    {
        PTX_LOG_V(frame->Log, "ptx_HIP_Frame_Decode");

        uint16_t frame_len = (uint16_t) ( (buf[PTX_HIP_FRAME_LEN_OFFSET_HI] << 8) | (buf[PTX_HIP_FRAME_LEN_OFFSET_LO]) );

        if (frame_len == (len - PTX_HIP_FRAME_LEN_SIZE))
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_HIP_RX_FRAME_INVALID_LENGTH;
        }

        uint8_t cmd = HIP_Cmd_MAX;
        uint8_t req_ack = 0;
        uint8_t add_crc = 0;

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t fcb = buf[PTX_HIP_RX_FCB_OFFSET];
            cmd = (fcb & PTX_HIP_FCB_CMD_MASK) >> PTX_HIP_FCB_CMD_SHIFT;
            req_ack = (fcb & PTX_HIP_FCB_ACK_MASK) >> PTX_HIP_FCB_ACK_SHIFT;
            add_crc = (fcb & PTX_HIP_FCB_CRC_MASK) >> PTX_HIP_FCB_CRC_SHIFT;
        }

        if ((PTX_STATUS_SUCCESS == status) && (cmd != data->Cmd))
        {
            status = PTX_STATUS_E_HIP_RX_FRAME_INVALID_CMD;
        }

        if ((PTX_STATUS_SUCCESS == status) && (1u == add_crc))
        {
            uint16_t crc = ptx_HIP_Frame_CalcCRC16(buf, len - PTX_HIP_CRC_LEN);

            if ((buf[len - PTX_HIP_CRC_H_BYTE_OFFSET] == ((crc & 0xFF00) >> 8)) &&
                (buf[len - PTX_HIP_CRC_L_BYTE_OFFSET] == (crc & 0x00FF)))
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_HIP_RX_FRAME_INVALID_CRC;
            }
        }

        if ((PTX_STATUS_SUCCESS == status) && (PTX_HIP_FRAME_CMD_IS_WRITE == data->IsWriteCmd))
        {
            if (1u == req_ack)
            {
                status = ptx_HIP_Frame_CheckAckByte(buf[3]);
            }
        }

        if ((PTX_STATUS_SUCCESS == status) && (PTX_HIP_FRAME_CMD_IS_READ == data->IsWriteCmd))
        {
            uint16_t rx_data_len = (uint16_t) (frame_len - 1u) /* FCB */;

            if (1u == add_crc)
            {
                rx_data_len = (uint16_t) (rx_data_len - PTX_HIP_CRC_LEN);
            }

            if (rx_data_len <= data->RxBufLen)
            {
                memcpy(data->RxBuf, &buf[3], rx_data_len);
            }
            else
            {
                status = PTX_STATUS_E_HIP_RX_BUFFER_TOO_SMALL;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_HIP_Frame_CheckAckByte (uint8_t ackByte)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_HIP_Frame_Status_t state = (ptx_HIP_Frame_Status_t)ackByte;

    if (HIP_Frame_Status_ACK == state)
    {
        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        if ((HIP_Frame_Status_ACK < state) && (HIP_Frame_Status_MAX > state))
        {
            /* Map returned HIP protocol error (NAK value) to PTX error code */
            switch (state)
            {
                case HIP_Frame_Status_InvalidCmd:
                    status = PTX_STATUS_E_HIP_NACK_INVALID_CMD;
                    break;
                case HIP_Frame_Status_InvalidLen:
                    status = PTX_STATUS_E_HIP_NACK_INVALID_LEN;
                    break;
                case HIP_Frame_Status_ErrorCRC:
                    status = PTX_STATUS_E_HIP_NACK_ERROR_CRC;
                    break;
                case HIP_Frame_Status_InvalidParam:
                    status = PTX_STATUS_E_HIP_NACK_INVALID_PARAM;
                    break;
                case HIP_Frame_Status_WBufFull:
                    status = PTX_STATUS_E_HIP_NACK_WRITE_BUF_FULL;
                    break;
                default:
                    /* Invalid value. Error already set. */
                    break;
            }
        }
    }

    return status;
}

static uint16_t ptx_HIP_Frame_CalcCRC16 (uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFF; /* CRC Init Value */

    do
    {
        uint8_t b = *data;
        data++;

        /* feed the input into the shift registers and xor with crc */
        b = (b ^ (uint8_t)(crc & 0x00FF));
        b = (uint8_t)(b ^ (b << 4));

        /* Calculate the CRC value */
        crc = (crc >> 8) ^ (uint32_t)((b << 8) ^ (b << 3) ^ (b >> 4));
        --len;
    }
    while (len != 0);

    return (uint16_t)~crc;
}

static uint16_t ptx_HIP_Frame_CalcRxFrameLen(ptx_HIP_Frame_t *frame, ptx_HIP_FrameData_t *frameData)
{
    uint32_t frame_rx_len = 0;

    if (PTX_COMP_CHECK(frame, Comp_HIP))
    {        
        PTX_LOG_V(frame->Log, "ptx_HIP_Frame_CalcRxFrameLen");


        if ((PTX_HIP_FRAME_CMD_IS_WRITE == frameData->IsWriteCmd) && (1 == frame->ReqAck))
        {
            frame_rx_len += PTX_HIP_FRAME_LEN_SIZE + PTX_HIP_FCB_LEN + PTX_HIP_ACK_LEN;
        }

        if (PTX_HIP_FRAME_CMD_IS_READ == frameData->IsWriteCmd)
        {
            frame_rx_len += PTX_HIP_FRAME_LEN_SIZE + PTX_HIP_FCB_LEN + frameData->RxBufLen;
        }

        if (( (PTX_HIP_FRAME_CMD_IS_READ == frameData->IsWriteCmd) || (1 == frame->ReqAck) ) && (1 == frame->AddCrc))
        {
            frame_rx_len += PTX_HIP_CRC_LEN;
        }
    }

    return (uint16_t)frame_rx_len;
}
