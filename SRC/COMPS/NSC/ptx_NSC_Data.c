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
    Module      : NFC Soft Controller (NSC) Interface Header for Data Exchange
    File        : ptx_NSC_Data.c

    Description : Implementation of NFC Soft Controller Interface Header for Data Exchange.
                  Contains function(s) to handle NSC Data messages transfer.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NSC_Data.h"
#include "ptx_NSC_CmdProcessor.h"
#include "ptx_NSC_RfSetParameter.h"
#include "ptx_NSC_StateMachine.h"


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
/*
 * NSC Data definitions.
 */
#define PTX_NSC_DATA_CLTMODE_NBRRFPARAMETERS            (uint8_t)4u         /* Number of Rf-Parameters set for Clt mode. */
#define PTX_NSC_DATA_CLTMODE_FWT                        (uint32_t)0x274C    /* CLT mode timeout. FWT has time units of 128/Fc . This is 9,43 microseconds. CLT timeout is has maximum 100 milliseconds. So, 1060 FWT Time Units. */


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NSC_Data_CheckAck(uint8_t *data, uint32_t len);
static ptx_Status_t ptx_NSC_Data_CheckMsg(ptx_NSC_t *nsc, uint8_t *data, uint32_t len);
static ptx_Status_t ptx_NSC_Data_CheckRxNtf(uint8_t *data, uint32_t len);
static ptx_Status_t ptx_NSC_Data_CheckRxMsg(ptx_NSC_t *nsc, uint8_t *data, uint32_t len);
static ptx_Status_t ptx_NSC_CltMsg_NumBytes(ptx_NSC_t *nsc, uint32_t length, uint8_t *numBitsLastByte, uint32_t *numBytesTotal);
static ptx_Status_t ptx_NSC_CltMsg_BuildRawBits(ptx_NSC_t *nsc, uint8_t *pldBytes, uint8_t *pldParityBits, uint32_t lengthBytes, uint8_t *pldRawBits, uint32_t lengthRawBits);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_DataTx(ptx_NSC_t *nsc, uint8_t *payload, uint32_t len, uint8_t isChained)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * The API shall allow also len to be 0. In such a case an empty payload NSC_RF_DATA_MSG shall be sent to PTX2xxR/W.
     * However, payload parameter shall always be provided valid for simplicity sake (might be a dummy).
     */

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != payload))
    {
        /* Data message allowed only in RF_POLL(DEP) states */
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_Data, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        uint32_t length_pending = (uint32_t)len;
        uint32_t offset = 0;

        do
        {
            uint32_t transfer_length = (length_pending > nsc->MtU) ? nsc->MtU : length_pending;
            uint8_t chaining;
            if ((length_pending > nsc->MtU) || (isChained > 0U))
            {
                chaining = PTX_NSC_DATA_MSG_CHAINING_TRUE;
            }
            else
            {
                chaining = PTX_NSC_DATA_MSG_CHAINING_FALSE;
            }

            uint8_t data[PTX_NSC_LENGTH_MAX];

            data[PTX_NSC_DATA_MSG_IDX_OPCODE] = PTX_NSC_DATA_MSG_OPCODE;
            data[PTX_NSC_DATA_MSG_IDX_FCB] = (PTX_NSC_DATA_MSG_CHAINING_TRUE == chaining) ? PTX_NSC_DATA_MSG_FCB_BIT_CHAIN : 0;
            data[PTX_NSC_DATA_MSG_IDX_DATALEN] = (uint8_t) transfer_length;

            memcpy(&data[PTX_NSC_DATA_MSG_IDX_DATA], &payload[offset], transfer_length);

            uint32_t statusByteLength = 0;
            if (nsc->ResidualTxBits != 0)
            {
                statusByteLength = 1;
                data[PTX_NSC_DATA_MSG_IDX_FCB] |= PTX_NSC_DATA_MSG_FCB_BIT_STATUS;
                data[PTX_NSC_LENGTH_DATAMSG_MIN + transfer_length] = PTX_NSC_DATA_MSG_STATUSBY_BIT_RES & nsc->ResidualTxBits;
            }

            status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_1, data, transfer_length + PTX_NSC_DATA_MSG_HEADER_LENGTH + statusByteLength);
            if ((PTX_STATUS_SUCCESS == status) && (PTX_NSC_DATA_MSG_CHAINING_TRUE == chaining))
            {
                /*
                 * If chaining, then expect to get ACK from PTX2xxR/W.
                 */
                uint8_t rsp[PTX_NSC_DATA_MSG_ACK_LENGTH];
                uint32_t rsp_len = PTX_NSC_DATA_MSG_ACK_LENGTH;

                uint32_t timeout = PTX_NSC_TIMEOUT_DEFAULT;

                /*
                 * RFU
                 */
                #if 0
                if (PTX_NSC_FWI_UNSET != nsc->FWI)
                {
                    timeout = ((PTX_NSC_FWI_MICROSECOND_STEP_SIZE << nsc->FWI) + PTX_NSC_FWI_T4T_DEFAULT) / 1000;
                }
                #endif
                status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp, &rsp_len, timeout, 0);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NSC_Data_CheckAck(rsp, rsp_len);
                }
            }

            length_pending -= transfer_length;
            offset += transfer_length;

        }
        while ( (PTX_STATUS_SUCCESS == status) && (length_pending > 0) && (0 != len));
    }

    return status;
}

ptx_Status_t ptx_NSC_DataRx(ptx_NSC_t *nsc, uint8_t *payload, uint32_t *len, uint32_t timeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != payload) && (NULL != len) && (0 != *len))
    {
        uint8_t rx_buf[PTX_NSC_LENGTH_MAX];
        uint32_t rx_buf_len = PTX_NSC_LENGTH_MAX;
        uint8_t chaining;
        uint32_t offset = 0;

        uint8_t copy_payload_if_error = 0;

        do
        {
            chaining = PTX_NSC_DATA_MSG_CHAINING_FALSE;

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rx_buf, &rx_buf_len, (uint32_t)timeout, 0);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Check if the received message is valid.
                 * Note: CheckMsg might return a PTX_STATUS_E_NSC_COLLISION or PTX_STATUS_E_NSC_RESIDUAL_BITS error.
                 * This happens if DATA_MSG has been in a proper format, but the status byte was set to
                 * indicate a collision or/and residual bits during the RF reception.
                 * In such a case, the message payload shall also be copied to the provided output buffer.
                 * Error status is just forwarded up.
                 */
                status = ptx_NSC_Data_CheckMsg(nsc, rx_buf, rx_buf_len);
                if ((PTX_STATUS_E_NSC_COLLISION == status) || (PTX_STATUS_E_NSC_RESIDUAL_BITS == status))
                {
                    copy_payload_if_error = 1U;
                }
            }

            if ((PTX_STATUS_SUCCESS == status) || (1 == copy_payload_if_error))
            {
                uint32_t msg_len = rx_buf[PTX_NSC_DATA_MSG_IDX_DATALEN];

                if ((offset + msg_len) <= *len)
                {
                    /* Data payload can be empty. Check this. */
                    if (0 != msg_len)
                    {
                        memcpy(&payload[offset], &rx_buf[PTX_NSC_DATA_MSG_IDX_DATA], msg_len);
                        offset += msg_len;

                        /* Check if the message is chained. */
                        chaining = (0 != (PTX_NSC_DATA_MSG_FCB_BIT_CHAIN & rx_buf[PTX_NSC_DATA_MSG_IDX_FCB])) ? PTX_NSC_DATA_MSG_CHAINING_TRUE : PTX_NSC_DATA_MSG_CHAINING_FALSE;
                    }
                }
                else
                {
                    status = PTX_STATUS_E_NSC_RX_ERROR_LENGTH;
                }
            }

        }
        while ((PTX_STATUS_SUCCESS == status) && (PTX_NSC_DATA_MSG_CHAINING_TRUE == chaining));

        if ((PTX_STATUS_SUCCESS == status) || (1 == copy_payload_if_error))
        {
            /* Update the number of the bytes actually copied to the input buffer. */
            *len = offset;
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_CtrlTx(ptx_NSC_t *nsc, uint8_t msgType)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        /* Data message allowed only in RF_POLL(DEP) states */
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_Data, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t data[PTX_NSC_DATA_MSG_ACK_LENGTH];

        data[PTX_NSC_DATA_MSG_IDX_OPCODE] = PTX_NSC_DATA_MSG_OPCODE;
        data[PTX_NSC_DATA_MSG_IDX_FCB] = PTX_NSC_DATA_MSG_FCB_BIT_CTRL;

        switch (msgType)
        {
            case PTX_NSC_DATA_MSG_ACK:
            /* FALL THROUGH */
            case PTX_NSC_DATA_MSG_NACK:
            /* FALL THROUGH */
            case PTX_NSC_DATA_MSG_EOF:
                data[PTX_NSC_DATA_MSG_IDX_CTRL] = msgType;
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_1, data, PTX_NSC_DATA_MSG_ACK_LENGTH);
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_CltMsg (ptx_NSC_t *nsc, uint8_t *pldBytes, uint8_t *pldParityBits, uint32_t len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != pldBytes) && (NULL != pldParityBits)
        && (len > 0) && (len <= PTX_NSC_MAX_TRANSFER_UNIT_SIZE) )
    {
        /* Data message allowed only in RF_POLL(DEP) states */
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_Data, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t num_bits_last_byte = 0;
        uint32_t num_bytes_bitstream = 0;

        /* Allocate enough space for the message. It cannot be more than NSC maximum. */
        uint8_t bitstream[PTX_NSC_LENGTH_MAX];

        status = ptx_NSC_CltMsg_NumBytes (nsc, len, &num_bits_last_byte, &num_bytes_bitstream);
        if (PTX_STATUS_SUCCESS == status)
        {
            if ((num_bytes_bitstream > 0) && (num_bytes_bitstream <= (uint32_t)PTX_NSC_MAX_TRANSFER_UNIT_SIZE - 2u) )
            {
                (void)memset(bitstream, 0, PTX_NSC_LENGTH_MAX);
                status = ptx_NSC_CltMsg_BuildRawBits (nsc, pldBytes, pldParityBits, len, bitstream, num_bytes_bitstream);
            }
            else
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Prepare Data Message of the length: OpCode + FCB + payload length + payload + residual bits.
             * In order not to waste more memory here, shift the data in the existing buffer.
             * The size of the allocated buffer has enough space for 4 more bytes. This has been assured
             * by the buffer allocated size definition and by the previous payload length check.
             */
            for (uint32_t i = 0; i < num_bytes_bitstream; i++)
            {
                bitstream[num_bytes_bitstream - i + 2u] = bitstream[num_bytes_bitstream - i - 1u];
            }

            bitstream[PTX_NSC_DATA_MSG_IDX_OPCODE] = PTX_NSC_DATA_MSG_OPCODE;
            /* If there is status byte present, set the flag in FCB */
            bitstream[PTX_NSC_DATA_MSG_IDX_FCB] = PTX_NSC_DATA_MSG_FCB_BIT_STATUS;
            bitstream[PTX_NSC_DATA_MSG_IDX_DATALEN] = (uint8_t)(num_bytes_bitstream);
            bitstream[num_bytes_bitstream + 3u] = num_bits_last_byte;

            status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_1, bitstream, num_bytes_bitstream + 1u + PTX_NSC_DATA_MSG_HEADER_LENGTH);
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_CltMsg_Enable(ptx_NSC_t *nsc, ptx_NSC_CltMode_t enable)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        ptx_NSC_RfSetParameter_Param_t rf_par[PTX_NSC_DATA_CLTMODE_NBRRFPARAMETERS];
        uint8_t idx = 0;

        rf_par[idx].Type = NSC_RfSetParameter_Param_Type_Parity;
        rf_par[idx].Value.Parity.TxParity = (NSC_CltMode_On == enable) ? NSC_RfSetParameter_Parity_Disable : NSC_RfSetParameter_Parity_Enable;
        rf_par[idx].Value.Parity.RxParity = (NSC_CltMode_On == enable) ? NSC_RfSetParameter_Parity_Disable : NSC_RfSetParameter_Parity_Enable;
        idx++;

        rf_par[idx].Type = NSC_RfSetParameter_Param_Type_Crc;
        rf_par[idx].Value.Crc.TxCrc = (NSC_CltMode_On == enable) ? NSC_RfSetParameter_Crc_Disable : NSC_RfSetParameter_Crc_Enable;
        rf_par[idx].Value.Crc.RxCrc = (NSC_CltMode_On == enable) ? NSC_RfSetParameter_Crc_Disable : NSC_RfSetParameter_Crc_Enable;
        idx++;

        rf_par[idx].Type = NSC_RfSetParameter_Param_Type_Fwt;
        rf_par[idx].Value.Fwt.FwtValue[0] = (NSC_CltMode_On == enable) ? ((uint8_t)((PTX_NSC_DATA_CLTMODE_FWT >> 0)  & (uint8_t)0xFF)) : 0x00;
        rf_par[idx].Value.Fwt.FwtValue[1] = (NSC_CltMode_On == enable) ? ((uint8_t)((PTX_NSC_DATA_CLTMODE_FWT >> 8)  & (uint8_t)0xFF)) : 0x00;
        rf_par[idx].Value.Fwt.FwtValue[2] = (NSC_CltMode_On == enable) ? ((uint8_t)((PTX_NSC_DATA_CLTMODE_FWT >> 16) & (uint8_t)0xFF)) : 0x00;
        idx++;

        /* In any case, set 1, as this is needed for CLT mode, and the default. */
        rf_par[idx].Type = NSC_RfSetParameter_Param_Type_MaxRspLim;
        rf_par[idx].Value.MaxRspLim.MaxRspLimValue = 1u;
        idx++;

        status = ptx_NSC_RfSetParameter(nsc, &rf_par[0], idx);
    }

    return status;
}

ptx_Status_t ptx_NSC_DataSetNrResidualTxBits(ptx_NSC_t *nsc, uint8_t nrResidualTxBits)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        nsc->ResidualTxBits = nrResidualTxBits;
        status = PTX_STATUS_SUCCESS;
    }
    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_CltMsg_BuildRawBits (ptx_NSC_t *nsc, uint8_t *pldBytes, uint8_t *pldParityBits, uint32_t lengthBytes, uint8_t *pldRawBits, uint32_t lengthRawBits)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    PTX_UNUSED_PARAMETER(nsc);

    if ((NULL != pldBytes) && (NULL != pldParityBits) && (NULL != pldRawBits))
    {
        /* Ensure there is enough capacity for writing the bit-stream */
        uint32_t num_tot_bits = (lengthBytes * 8u) + 8u;
        uint32_t is_last_byte = ((num_tot_bits % 8u) == 0) ? 0 : 1u;
        uint32_t num_bytes_bitstream = ((num_tot_bits - (num_tot_bits % 8u)) / 8u) + is_last_byte;

        if (lengthRawBits >= num_bytes_bitstream)
        {
            uint32_t index_raw_bits = 0;
            uint32_t index_raw_bytes = 0;
            uint32_t index_payload = 0;
            uint8_t par_temp = 0;
            (void)memset(pldRawBits, 0, lengthRawBits);

            do
            {
                /* Write payload */
                if (0 == (index_raw_bits % 8u))
                {
                    /* Full byte of payload in a full byte of bit-stream */
                    pldRawBits[index_raw_bytes] = pldBytes[index_payload];
                }
                else
                {
                    /* Some bits in current byte of bit-stream, some others in the next byte */
                    uint16_t two_bytes_temp = 0x0000;
                    uint8_t num_shift_bits = index_raw_bits % 8u;
                    two_bytes_temp = (uint16_t) (pldBytes[index_payload] << num_shift_bits);

                    pldRawBits[index_raw_bytes] = (((uint8_t)(0x00FF & two_bytes_temp)) | (pldRawBits[index_raw_bytes]) );
                    pldRawBits[index_raw_bytes + 1u] = ((uint8_t)(0x00FF & ((0xFF00 & two_bytes_temp) >> 8u)));
                }
                index_raw_bits += 8u;
                index_raw_bytes ++;

                /* Write parity */
                par_temp = (uint8_t) ((pldParityBits[index_payload] & 0x01) << (index_raw_bits % 8u)) ;
                pldRawBits[index_raw_bytes] = pldRawBits[index_raw_bytes] | par_temp;
                index_raw_bits ++;
                index_raw_bytes = (0 == (index_raw_bits % 8u)) ? (index_raw_bytes + 1) : index_raw_bytes;

                index_payload ++;

            }
            while (index_payload < lengthBytes);

            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_CltMsg_NumBytes (ptx_NSC_t *nsc, uint32_t length, uint8_t *numBitsLastByte, uint32_t *numBytesTotal)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    PTX_UNUSED_PARAMETER(nsc);

    if ((NULL != numBitsLastByte) && (NULL != numBytesTotal))
    {
        uint32_t num_bits = (8 * length) + length;
        uint32_t is_last_byte = ((num_bits % 8u) == 0) ? 0 : 1u;
        *numBitsLastByte = num_bits % 8u;
        *numBytesTotal = ((num_bits - *numBitsLastByte) / 8u) + is_last_byte;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_NSC_Data_CheckRxMsg(ptx_NSC_t *nsc, uint8_t *data, uint32_t len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != data) && (PTX_NSC_LENGTH_DATAMSG_MIN <= len))
    {
        uint8_t cont_checking = 0;
        uint32_t status_byte_present = 0;
        uint32_t total_len = 0;

        status = PTX_STATUS_E_NSC_PROTOCOL;

        /* Check that the opcode is DATA_MSG and that FCB.CTRL bit is not set. */
        if ((PTX_NSC_DATA_MSG_OPCODE == data[PTX_NSC_DATA_MSG_IDX_OPCODE]) &&
            (0 == (data[PTX_NSC_DATA_MSG_IDX_FCB] & PTX_NSC_DATA_MSG_FCB_BIT_CTRL)) )
        {
            cont_checking = 1u;
        }

        if (0 != cont_checking)
        {
            /* Check if status byte is present. */
            if ( 0 != (data[PTX_NSC_DATA_MSG_IDX_FCB] & PTX_NSC_DATA_MSG_FCB_BIT_STATUS) )
            {
                status_byte_present = 1u;
            }

            /* Check DataLen field. When summed up, must match input len parameter. */
            total_len = (uint32_t)(PTX_NSC_LENGTH_DATAMSG_MIN + data[PTX_NSC_DATA_MSG_IDX_DATALEN] + status_byte_present);

            if (total_len == len)
            {
                status = PTX_STATUS_SUCCESS;
            }
        }

        /* Final thing: If message is OK and there is a status byte present, check the content of it. */
        if ( (PTX_STATUS_SUCCESS == status) && (0 != status_byte_present) )
        {
            /* Status byte is the last byte in the message. */
            uint8_t status_byte = data[total_len - 1];

            if (0 != (PTX_NSC_DATA_MSG_STATUSBY_BIT_ERR & status_byte))
            {
                /* General error during reception. If this is set, nothing else can be processed with the data of
                 * the response and it should be handled as an error in all cases.
                 */
                status = PTX_STATUS_E_NSC_RFERROR;
            }
            else
            {
                if (0 != (PTX_NSC_DATA_MSG_STATUSBY_BIT_COLL & status_byte))
                {
                    /* Collision occurred during the last data exchange. Overwrite the status and return specific error message.
                     * This case can only be applicable and further processed for Type A and V in specific states.
                     * In all other cases this represents an error.
                     */
                    status = PTX_STATUS_E_NSC_COLLISION;
                }

                if (0 != (PTX_NSC_DATA_MSG_STATUSBY_BIT_RES & status_byte))
                {
                    /* The number of valid bits of the last byte for the last data exchange. Overwrite the status and return specific error message.
                     * This case can only be applicable and further processed for Type A in specific scenarios.
                     * In all other cases this represents an error.
                     */
                    nsc->ResidualRxBits = (uint8_t)(status_byte & PTX_NSC_DATA_MSG_STATUSBY_BIT_RES);

                    if (PTX_STATUS_E_NSC_COLLISION != status)
                    {
                        status = PTX_STATUS_E_NSC_RESIDUAL_BITS;
                    }
                }
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_Data_CheckRxNtf(uint8_t *data, uint32_t len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /**
     * This routine is called if a notification is received during DataMsg exchange.
     * Then, it is expected to get an error notification. Anyway, it is an NSC protocol error.
     */

    if ((NULL != data) && (PTX_NSC_CMD_PROCESSOR_ERROR_NTFLEN == len) &&
        (PTX_NSC_CMD_PROCESSOR_ERROR_NTF == data[PTX_NSC_DATA_MSG_IDX_OPCODE]))
    {
        uint8_t ntfError = data[PTX_NSC_CMD_OPCODE_LEN];

        switch (ntfError)
        {
            case NSC_ErrorCode_RfTimeoutError:
            case NSC_ErrorCode_RfCollisionError:
            case NSC_ErrorCode_RfTransmissionError:
            case NSC_ErrorCode_RfProtocolError:
                status = PTX_STATUS_E_NSC_RFERROR;
                break;

            default:
                status = PTX_STATUS_E_NSC_PROTOCOL;
                break;
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_Data_CheckMsg(ptx_NSC_t *nsc, uint8_t *data, uint32_t len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != data)
    {
        if  (PTX_NSC_LENGTH_DATAMSG_MIN <= len)
        {
            status = ptx_NSC_Data_CheckRxMsg(nsc, data, len);
        }
        else
        {
            status = ptx_NSC_Data_CheckRxNtf(data, len);
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_Data_CheckAck(uint8_t *data, uint32_t len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != data) && (0 != len))
    {
        uint8_t cont_checking = 0;

        status = PTX_STATUS_E_NSC_PROTOCOL;

        if ((PTX_NSC_DATA_MSG_OPCODE == data[PTX_NSC_DATA_MSG_IDX_OPCODE]) &&
            (PTX_NSC_DATA_MSG_ACK_LENGTH == (uint8_t)len) )
        {
            cont_checking = 1u;
        }

        if ((0 == cont_checking) && (PTX_NSC_CMD_PROCESSOR_ERROR_NTFLEN == len) &&
            (PTX_NSC_CMD_PROCESSOR_ERROR_NTF == data[PTX_NSC_DATA_MSG_IDX_OPCODE]))
        {
            uint8_t ntfError = data[PTX_NSC_CMD_OPCODE_LEN];

            switch (ntfError)
            {
                case NSC_ErrorCode_RfTimeoutError:
                case NSC_ErrorCode_RfCollisionError:
                case NSC_ErrorCode_RfTransmissionError:
                case NSC_ErrorCode_RfProtocolError:
                    status = PTX_STATUS_E_NSC_RFERROR;
                    break;

                default:
                    /* nothing to do here - see default status assignment */
                    break;
            }
        }

        /*
         * Intentionally comparing bit value of FCB_CTRL with the whole FCB byte,
         * since for a valid ACK only this bit can and must be set.
         */
        if ((0 != cont_checking) && (PTX_NSC_DATA_MSG_FCB_BIT_CTRL == data[PTX_NSC_DATA_MSG_IDX_FCB]) &&
            (PTX_NSC_DATA_MSG_ACK == data[PTX_NSC_DATA_MSG_IDX_CTRL]) )
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}
