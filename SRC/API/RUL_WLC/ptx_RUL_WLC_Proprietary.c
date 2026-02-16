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
    File        : ptx_RUL_WLC_Proprietary.c

    Description : RUL Wireless Charging (WLC) Proprietary Extension implements unique NFC WLC features with the PTX2xxW chip.
                  This module grants the possibility to use proprietary additional features when using the PTX2xxW WLC-P hardware
                  with another Renesas WLC-L hardware (e.g., RRQ36031).
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stdbool.h>
#include "ptx_RUL_WLC_Proprietary.h"
#include "ptx_NativeTag_T2T.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

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

ptx_Status_t ptx_RUL_WLC_Proprietary_ReadT2T(ptx_RUL_WLC_t *ctx, uint8_t block, uint8_t *rxData, uint32_t *rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && (NULL != rxData) && (NULL != rxLen) && (PTX_WLC_PROPRIETARY_T2T_MAX_PAYLOAD_LEN >= *rxLen) )
    {
        uint8_t tx_buffer[PTX_WLC_PROPRIETARY_T2T_READ_CMD_LEN];
        uint32_t tx_buffer_len = sizeof(tx_buffer);

        tx_buffer[PTX_WLC_PROPRIETARY_T2T_OP_CODE_POS] = PTX_WLC_PROPRIETARY_T2T_READ_CODE;
        tx_buffer[PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_POS] = block;
        tx_buffer[PTX_WLC_PROPRIETARY_T2T_PAYLOAD_POS] = (uint8_t) ((*rxLen / PTX_T2T_BLOCK_SIZE) + (*rxLen % PTX_T2T_BLOCK_SIZE ? 1u : 0u));

        status = ptx_RUL_Exchange(ctx->InitParams.Rul, tx_buffer, tx_buffer_len, rxData, rxLen, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_Proprietary_WriteT2T(ptx_RUL_WLC_t *ctx, uint8_t block, uint8_t *txData, uint32_t txDataLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC) && (NULL != txData) && (PTX_WLC_PROPRIETARY_T2T_MAX_PAYLOAD_LEN >= txDataLen) )
    {
        uint8_t rx_buffer[PTX_WLC_PROPRIETARY_T2T_WRITE_RSP_LEN];
        uint32_t rx_buffer_len = sizeof(rx_buffer);

        uint8_t tx_buffer[PTX_WLC_PROPRIETARY_T2T_MAX_MESSAGE_LEN];

        tx_buffer[PTX_WLC_PROPRIETARY_T2T_OP_CODE_POS] = PTX_WLC_PROPRIETARY_T2T_WRITE_CODE;
        tx_buffer[PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_POS] = block;
        memcpy(&tx_buffer[PTX_WLC_PROPRIETARY_T2T_PAYLOAD_POS], txData, txDataLen);

        uint8_t blocks = (uint8_t) ((txDataLen / PTX_T2T_BLOCK_SIZE) + (txDataLen % PTX_T2T_BLOCK_SIZE ? 1u : 0u));
        uint8_t bytes_to_transmit = (uint8_t) (blocks * PTX_T2T_BLOCK_SIZE) + PTX_WLC_PROPRIETARY_T2T_HEADER_LEN;
        status = ptx_RUL_Exchange(ctx->InitParams.Rul, tx_buffer, bytes_to_transmit, rx_buffer, &rx_buffer_len, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);

        /* If residual bits have been received, which is required when writing to the T2T
         * (receives a 4 bit long ACK/NACK response), set the status to success.
         */
        if (PTX_STATUS_E_NSC_RESIDUAL_BITS == status)
        {
            status = PTX_STATUS_SUCCESS;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (0 != rx_buffer_len)
            {
                status = (PTX_T2T_ACK_CODE != rx_buffer[0]) ? (PTX_STATUS_E_NDEF_PROTOCOL_ERROR) : (status);
            }
            else
            {
                status = PTX_STATUS_E_NSC_RFERROR;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_Proprietary_SetListenerGpios(ptx_RUL_WLC_t *ctx, uint8_t setGpio0High, uint8_t setGpio1High)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC))
    {
        /* Read back Block content to not overwrite the remaining content. */
        uint8_t rx_buffer[PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN];
        uint32_t rx_buffer_len = sizeof(rx_buffer);

        status = ptx_NativeTag_T2T_Read(&(ctx->NdefComp.tagCtx.t2t.t2t), PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_SYSTEM_CTRL, rx_buffer, &rx_buffer_len, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t tx_buffer[PTX_T2T_BLOCK_SIZE] = {0};
            uint8_t tx_buffer_len = sizeof(tx_buffer);

            uint8_t rx_buffer_write[PTX_WLC_PROPRIETARY_T2T_WRITE_RSP_LEN];
            uint32_t rx_buffer_write_len = sizeof(rx_buffer_write);

            /* Add Block Content from Read and overwrite with new GPIO settings. */
            memcpy(tx_buffer, rx_buffer, PTX_T2T_BLOCK_SIZE);
            tx_buffer[PTX_WLC_PROPRIETARY_SYSTEM_CTRL_GPIO_0_POS] = ((setGpio0High == 1) ? PTX_WLC_PROPRIETARY_SYSTEM_CTRL_GPIO_HIGH_LEVEL : PTX_WLC_PROPRIETARY_SYSTEM_CTRL_GPIO_LOW_LEVEL);    /* Payload byte 0 controlling GPIO0. */
            tx_buffer[PTX_WLC_PROPRIETARY_SYSTEM_CTRL_GPIO_1_POS] = ((setGpio1High == 1) ? PTX_WLC_PROPRIETARY_SYSTEM_CTRL_GPIO_HIGH_LEVEL : PTX_WLC_PROPRIETARY_SYSTEM_CTRL_GPIO_LOW_LEVEL);    /* Payload byte 1 controlling GPIO1. */
            status = ptx_NativeTag_T2T_Write(&(ctx->NdefComp.tagCtx.t2t.t2t), PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_SYSTEM_CTRL, tx_buffer, tx_buffer_len, rx_buffer_write, &rx_buffer_write_len, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_Proprietary_SetListenerShippingModeEnable(ptx_RUL_WLC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC))
    {
        /* Read back Block content to not overwrite the remaining content. */
        uint8_t rx_buffer[PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN];
        uint32_t rx_buffer_len = sizeof(rx_buffer);

        status = ptx_NativeTag_T2T_Read(&(ctx->NdefComp.tagCtx.t2t.t2t), PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_SYSTEM_CTRL, rx_buffer, &rx_buffer_len, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t tx_buffer[PTX_T2T_BLOCK_SIZE] = {0};
            uint8_t tx_buffer_len = sizeof(tx_buffer);

            uint8_t rx_buffer_write[PTX_WLC_PROPRIETARY_T2T_WRITE_RSP_LEN];
            uint32_t rx_buffer_write_len = sizeof(rx_buffer_write);

            /* Add Block Content from Read and overwrite with new Shipping Mode Enable settings. */
            memcpy(tx_buffer, rx_buffer, PTX_T2T_BLOCK_SIZE);
            tx_buffer[PTX_WLC_PROPRIETARY_SYSTEM_CTRL_SHIPPING_MODE_ENABLE_POS] = 0x01;
            status = ptx_NativeTag_T2T_Write(&(ctx->NdefComp.tagCtx.t2t.t2t), PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_SYSTEM_CTRL, tx_buffer, tx_buffer_len, rx_buffer_write, &rx_buffer_write_len, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_Proprietary_GetListenerSystemStatus(ptx_RUL_WLC_t *ctx, ptx_RUL_WLC_Proprietary_ListenerSystemStatus_t *statusInfo)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != statusInfo)
    {
        uint8_t rx_buffer[PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN];
        uint32_t rx_buffer_len = sizeof(rx_buffer);

        status = ptx_NativeTag_T2T_Read(&(ctx->NdefComp.tagCtx.t2t.t2t), PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_SYSTEM_STATUS, rx_buffer, &rx_buffer_len, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);

        if (PTX_STATUS_SUCCESS == status)
        {
            statusInfo->NtcStatus = (ptx_RUL_WLC_Proprietary_ListenerNtcStatus_t) rx_buffer[PTX_WLC_PROPRIETARY_SYSTEM_STATUS_NTC_POS];
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_Proprietary_TDC_Write(ptx_RUL_WLC_t *ctx, uint8_t *txData, uint8_t txDataLen, uint32_t ackTimeoutMs)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && (NULL != txData) && (PTX_WLC_PROPRIETARY_TDC_MAX_PAYLOAD_LEN >= txDataLen) )
    {
#if defined(PTX_ENABLE_PROPRIETARY_TDC_NFC_FORUM)
        /*< Start with second data block and write the first one at last. */
        uint32_t buffer_pos = PTX_T2T_BLOCK_SIZE - PTX_WLC_PROPRIETARY_TDC_HEADER_LEN;
        uint8_t block = PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_TDC_POL + 1u;
        uint32_t remaining_tdc_message_length = txDataLen - (PTX_T2T_BLOCK_SIZE - PTX_WLC_PROPRIETARY_TDC_HEADER_LEN);

        uint8_t rx_buffer[PTX_WLC_PROPRIETARY_T2T_WRITE_RSP_LEN];
        uint32_t rx_buffer_len = sizeof(rx_buffer);

        /*
         * The poller buffer within the Listener's memory starts on block 48. As soon as block 48 gets modified,
         * by the poller, the Listener will indicate the modification to the it's host MCU by throwing an interrupt.
         * Hence, we need to make sure, to write the complete remaining payload FIRST, and only afterwards we update
         * block 48!
         * Using the T2T write command, we can only write 4 bytes at once!
         */
        status = PTX_STATUS_SUCCESS;
        while ( (PTX_STATUS_SUCCESS == status) &&  (0u < remaining_tdc_message_length) )
        {
            /* Write the data in chunks of 4 bytes (NFC_FORUM compliant). */
            status = ptx_NativeTag_T2T_Write(&(ctx->NdefComp.tagCtx.t2t.t2t), block++, &txData[buffer_pos], PTX_T2T_BLOCK_SIZE, rx_buffer, &rx_buffer_len, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);
            /* Increment block number. */
            buffer_pos += PTX_T2T_BLOCK_SIZE;
            /* Decrement number of remaining bytes of the TDC message. */
            if (PTX_T2T_BLOCK_SIZE < remaining_tdc_message_length)
            {
                remaining_tdc_message_length = remaining_tdc_message_length - PTX_T2T_BLOCK_SIZE;
            }
            else
            {
                remaining_tdc_message_length = 0;
            }
        }

        /* Write the remaining data  of the first block into block 48. */
        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t data[PTX_T2T_BLOCK_SIZE] = {0};
            /* The first byte in block 48 indicates the number of written bytes to the Listener and the operation code. */
            data[PTX_WLC_PROPRIETARY_TDC_HEADER_POS] = PTX_WLC_PROPRIETARY_NSC_DATA_MSG_CODE | txDataLen;
            /* Copy remaining data into reserved memory. */
            memcpy(&data[PTX_WLC_PROPRIETARY_TDC_PAYLOAD_POS], txData, PTX_T2T_BLOCK_SIZE - 1u);
            /* Write the data in chunks of 4 bytes (NFC_FORUM compliant). */
            status = ptx_NativeTag_T2T_Write(&(ctx->NdefComp.tagCtx.t2t.t2t), PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_TDC_POL, &data[0], PTX_T2T_BLOCK_SIZE, rx_buffer, &rx_buffer_len, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);
        }
#else
        /* Reserve 64 bytes at once. */
        uint8_t data[PTX_WLC_PROPRIETARY_TDC_MAX_MESSAGE_LEN] = {0};
        /* The first byte in block 48 indicates the number of written bytes to the Listener and the operation code. */
        data[PTX_WLC_PROPRIETARY_TDC_HEADER_POS] = PTX_WLC_PROPRIETARY_NSC_DATA_MSG_CODE | txDataLen;
        /* Copy remaining data into reserved memory. */
        memcpy(&data[PTX_WLC_PROPRIETARY_TDC_PAYLOAD_POS], txData, txDataLen);
        /* Write the data using one single write operation (PTX proprietary command). */
        status = ptx_RUL_WLC_Proprietary_WriteT2T(ctx, PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_TDC_POL, data, txDataLen + PTX_WLC_PROPRIETARY_TDC_HEADER_LEN);

#endif

        /* Check if we want to wait fot the read acknowledge. */
        if ( (PTX_STATUS_SUCCESS == status) && (0 != ackTimeoutMs) )
        {
            uint8_t received;
            uint8_t is_elapsed;

            /* Prepare the timer. */
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_HAL_Tmr_Start(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, HAL_DRV_Tmr_Unit_mSec, ackTimeoutMs);
            }

            do
            {
                /* Did the Listener host MCU read the package? */
                status = ptx_RUL_WLC_Proprietary_TDC_IsReceived(ctx, &received);
                /* Check the timer status. */

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_HAL_Tmr_IsElapsed(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, &is_elapsed);
                }

            }
            while ( (PTX_STATUS_SUCCESS == status) && (0 == received) && (0 == is_elapsed) );

            ptx_HAL_Tmr_Stop(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1);

            /* Did the Listener's host MCU read the package within the given timeout? */
            if (0 == received)
            {
                status = PTX_STATUS_E_GEN_TIMEOUT;
            }
        }

    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_Proprietary_TDC_IsReceived(ptx_RUL_WLC_t *ctx, uint8_t *received)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && (NULL != received) )
    {
        /* Explicitly set "received" variable to zero. */
        *received = 0;

        /* Reserve memory for read operation. */
        uint8_t read_data[PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN];
        uint32_t read_data_len = sizeof(read_data);
#if defined(PTX_ENABLE_PROPRIETARY_TDC_NFC_FORUM)
        /* Read the first block of the poller buffer on the Listener device. */
        status = ptx_NativeTag_T2T_Read (&(ctx->NdefComp.tagCtx.t2t.t2t), PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_TDC_POL, read_data, &read_data_len, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);
#else
        read_data_len = PTX_T2T_BLOCK_SIZE;
        /* Do the proprietary read operation (reads 4 bytes per transaction instead of 16 byte). */
        status = ptx_RUL_WLC_Proprietary_ReadT2T (ctx, PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_TDC_POL, read_data, &read_data_len);
#endif

        /*
         * In case the host MCU (of the Listener) has read the message, the first byte in the
         * poller buffer (indicating the message length) will be set to zero.
         */
        if ( (PTX_STATUS_SUCCESS == status) && (0u == read_data[PTX_WLC_PROPRIETARY_TDC_HEADER_POS]) )
        {
            *received = 1;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_WLC_Proprietary_TDC_Read(ptx_RUL_WLC_t *ctx, uint8_t *rxData, uint8_t *rxLen, uint32_t rxTimeoutMs)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(ctx, Comp_WLC) && (NULL != rxData) && (NULL != rxLen) )
    {
        uint8_t rx_data[PTX_WLC_PROPRIETARY_TDC_MAX_MESSAGE_LEN];

        if (0 != rxTimeoutMs)
        {
            status = ptx_HAL_Tmr_Start(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, HAL_DRV_Tmr_Unit_mSec, rxTimeoutMs);


        }

        uint32_t rx_data_len;
        uint8_t is_elapsed;
        uint8_t total_tdc_message_length = 0;
        do
        {
#if defined(PTX_ENABLE_PROPRIETARY_TDC_NFC_FORUM)
            rx_data_len = PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN;
            uint8_t block_num = PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_TDC_LIS;

            /* Use the 'standard'T2T read command to retrieve the data (16 bytes per transaction).
             * Read first Block to check First Byte with length information.
             */
            status = ptx_NativeTag_T2T_Read(&(ctx->NdefComp.tagCtx.t2t.t2t), block_num, rx_data, &rx_data_len, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);

            if (PTX_STATUS_SUCCESS == status)
            {
                total_tdc_message_length = (rx_data[PTX_WLC_PROPRIETARY_TDC_HEADER_POS] & PTX_WLC_PROPRIETARY_TDC_BUFFER_LEN_MASK);
                uint8_t remaining_tdc_message_length = total_tdc_message_length;

                /* If we have a payload bigger than 15 Bytes (16 Bytes read length without header byte), we need to read
                 * the remaining bytes until all data was read. For every NFC Forum compliant T2T read we can only read 16 Bytes at once.
                 */
                if ((PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN - PTX_WLC_PROPRIETARY_TDC_HEADER_LEN) < remaining_tdc_message_length)
                {
                    uint32_t buffer_pos = PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN;
                    remaining_tdc_message_length = remaining_tdc_message_length - PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN;
                    while ( (PTX_STATUS_SUCCESS == status) &&  (0u < remaining_tdc_message_length) )
                    {
                        block_num += (PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN / PTX_T2T_BLOCK_SIZE);
                        rx_data_len = PTX_T2T_READ_RSP_MAX_PAYLOAD_LEN;
                        status = ptx_NativeTag_T2T_Read(&(ctx->NdefComp.tagCtx.t2t.t2t), block_num, &rx_data[buffer_pos], &rx_data_len, PTX_WLC_PROPRIETARY_T2T_COMMUNICATION_TIMEOUT);

                        buffer_pos += rx_data_len;
                        if (rx_data_len < remaining_tdc_message_length)
                        {
                            remaining_tdc_message_length = (uint8_t)(remaining_tdc_message_length - rx_data_len);
                        }
                        else
                        {
                            remaining_tdc_message_length = 0;
                        }
                    }
                }
            }

#else
            rx_data_len = PTX_T2T_BLOCK_SIZE;
            /* Read first Block to check First Byte with length information. */
            status = ptx_RUL_WLC_Proprietary_ReadT2T(ctx, PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_TDC_LIS, rx_data, &rx_data_len);

            if (PTX_STATUS_SUCCESS == status)
            {
                total_tdc_message_length = (rx_data[PTX_WLC_PROPRIETARY_TDC_HEADER_POS] & PTX_WLC_PROPRIETARY_TDC_BUFFER_LEN_MASK);
                uint8_t remaining_tdc_message_length = total_tdc_message_length;

                /* If we have a payload bigger than 3 Bytes (4 Bytes read length without header byte), we need to read
                 * the remaining bytes until all data was read. For the Proprietary T2T Read, 64 Bytes (Max. TDC Message Size) can be read at once.
                 */
                if ((PTX_T2T_BLOCK_SIZE - PTX_WLC_PROPRIETARY_TDC_HEADER_LEN) < remaining_tdc_message_length)
                {

                    uint8_t num_blocks = (uint8_t) (((remaining_tdc_message_length + PTX_WLC_PROPRIETARY_TDC_HEADER_LEN) / PTX_T2T_BLOCK_SIZE) + ((remaining_tdc_message_length + PTX_WLC_PROPRIETARY_TDC_HEADER_LEN) % PTX_T2T_BLOCK_SIZE ? 1u : 0u));
                    rx_data_len = num_blocks * PTX_T2T_BLOCK_SIZE;

                    status = ptx_RUL_WLC_Proprietary_ReadT2T(ctx, PTX_WLC_PROPRIETARY_T2T_BLOCK_NUM_TDC_LIS, rx_data, &rx_data_len);
                }
            }
#endif
            /* Check the timer status. Did it elapse?*/
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_HAL_Tmr_IsElapsed(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1, &is_elapsed);
            }

            /* Read out the until time elapses or until (rx_data[0] != 0). */
        }
        while (    (PTX_STATUS_SUCCESS == status)
                   &&  (0 == total_tdc_message_length)
                   &&  (0 != rxTimeoutMs)
                   &&  (0 == is_elapsed) );

        /* If the timer was running, we need to stop it! */
        if (0 != rxTimeoutMs)
        {
            ptx_HAL_Tmr_Stop(&(ctx->InitParams.Rul->Hal), HAL_Tmr_Id_1);
        }

        /* If everything was successful, copy the relevant information. */
        if (PTX_STATUS_SUCCESS == status)
        {
            *rxLen = total_tdc_message_length;
            memcpy(rxData, &rx_data[PTX_WLC_PROPRIETARY_TDC_PAYLOAD_POS], *rxLen);
        }
        else
        {
            *rxLen = 0; /*< Explicitly set to 0 in case of error. */
        }
    }

    return status;
}
