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
    Module      : NFC Soft Controller (NSC) Interface for Memory Access
    File        : ptx_NSC_Misc.c

    Description : Definition of NFC Soft Controller Interface for Memory Access.
                  Contains function(s) to access the NSC memory, get status of the
                  latest operation or to invoke NSC specific (test) activity.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NSC_Misc.h"
#include "ptx_NSC_CmdProcessor.h"
#include "ptx_NSC_StateMachine.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NSC_READ_CMD_ADDRESS_INDEX          1U      /* Address bytes index (start position) within NSC message */
#define PTX_NSC_RSP_ERRCODE_INDEX               1U      /* The index of the ErrorCode value in the RSP message. */
#define PTX_NSC_READ_CMD_LEN                    4U      /* Total length of NSC_READ_CMD message */

#define PTX_NSC_LOOPBACK_CMD_MAXLEN             257U    /* Maximum length of NSC_LOOPBACK_CMD message */
#define PTX_NSC_LOOPBACK_RSP_MAXLEN             258U    /* Maximum length of NSC_LOOPBACK_RSP message */
#define PTX_NSC_LOOPBACK_RSP_ERRCODE_INDEX      1U      /* The index of the ErrorCode value in the RSP message. */
#define PTX_NSC_LOOPBACK_RSP_LEN_IDX            2U      /* Index of the payload length byte in the RSP message */
#define PTX_NSC_LOOPBACK_RSP_PAYLOAD_IDX        3U      /* Starting index of the payload field in the RSP message */
#define PTX_NSC_LOOPBACK_VALID_RSP_MINLEN       3U      /* Valid Loopback RSP message has at least 3By */

#define PTX_NSC_GETSTATUS_CMD_LEN               1U      /* Length of NSC_GET_STATUS_CMD message. Only 1By (OPCODE). */
#define PTX_NSC_GETSTATUS_RSP_IDX_STATUS        1U      /* Index of ERROR_STATUS parameter. */
#define PTX_NSC_GETSTATUS_RSP_IDX_DDPC          2U      /* Index of DDPC_PL parameter. */
#define PTX_NSC_GETSTATUS_RSP_IDX_RSSI_1        3U      /* Index of RSSI parameter, MSB byte. */
#define PTX_NSC_GETSTATUS_RSP_IDX_RSSI_2        4U      /* Index of RSSI parameter, second byte. */
#define PTX_NSC_GETSTATUS_RSP_IDX_RSSI_3        5U      /* Index of RSSI parameter, LSB byte. */
#define PTX_NSC_GETSTATUS_RSP_IDX_COLLINFO      6U      /* Index of COLL_INFO parameter. */
#define PTX_NSC_GETSTATUS_RSP_DDPC_LEN          1U      /* Length of DDPC_PL parameter. */
#define PTX_NSC_GETSTATUS_RSP_RSSI_LEN          3U      /* Length of RSSI parameter. */
#define PTX_NSC_GETSTATUS_RSP_COLL_LEN          1U      /* Length of COLL_INFO parameter. */

#define PTX_UINT32_1BY_MASK                     (uint32_t)0x000000FF /* Mask for 1st byte (LSB) of an uint32_t value. */
#define PTX_UINT32_2BY_MASK                     (uint32_t)0x0000FF00 /* Mask for 2nd byte of an uint32_t value. */
#define PTX_UINT32_3BY_MASK                     (uint32_t)0x00FF0000 /* Mask for 3rd byte of an uint32_t value. */
#define PTX_UINT32_SHIFT_LEFT(x, n)             (uint32_t)((uint32_t)x << n)    /* Left shift byte x by n bits and cast to uint32_t */


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

ptx_Status_t ptx_NSC_Read(ptx_NSC_t *nsc, uint16_t address, uint8_t len, uint8_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Length can be 0. In such a case there shall be no returned data, i.e. data output buffer shall be untouched. */

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != data))
    {
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_Read, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t buffer[PTX_NSC_READ_CMD_LEN];
        uint16_t idx = 0;

        buffer[idx] = PTX_NSC_READ_CMD_OPCODE;
        idx++;

        /* Received address is expected to be in LSB format. It needs to be translated to MSB. */
        buffer[idx] = (uint8_t)((address >> 8) & 0x00FF);
        idx++;
        buffer[idx] = (uint8_t)(address & 0x00FF);
        idx++;

        buffer[idx] = len;
        idx++;

        status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, buffer, idx);
        if (PTX_STATUS_SUCCESS == status)
        {
            /*
             *  The caller has to ensure to allocate enough space for data. Maximum data len can be 255 since len is an uint8_t.
             *  However, here we need to allocate 2 extra bytes for RSP Code and ERROR_CODE.
             */
            uint8_t rsp_buffer[PTX_NSC_LENGTH_MAX];
            uint32_t rsp_buffer_len = len + PTX_NSC_CMD_PROCESSOR_READ_RSPLEN;
            memset(rsp_buffer, 0, sizeof(uint8_t)*PTX_NSC_LENGTH_MAX);

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_buffer, &rsp_buffer_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_READ_RSP);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_Read, rsp_buffer, rsp_buffer_len, NULL);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Response contains at least minimum READ_RSP message length. We need to make additional check whether
                 *  complete data was received as required.
                 */
                if (((len + PTX_NSC_CMD_PROCESSOR_READ_RSPLEN) == rsp_buffer_len) && (NSC_ErrorCode_NoErr == rsp_buffer[PTX_NSC_RSP_ERRCODE_INDEX]))
                {
                    /* All good. Copy received data to output buffer. */
                    memcpy(data, &rsp_buffer[PTX_NSC_CMD_PROCESSOR_READ_RSPLEN], len);
                }
                else
                {
                    status = PTX_STATUS_E_NSC_RX_ERROR_LENGTH;
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_Write(ptx_NSC_t *nsc, uint16_t address, uint8_t len, uint8_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* For Write command, len shall not be 0. */

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != data) && (0 != len))
    {
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_Write, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /*
         *  Maximum data len can be 255 since len is an uint8_t (1By).
         *  However, here we need to allocate 4 extra bytes for Opcode (1By), Address (2By) and N (1By).
         */
        uint8_t buffer[PTX_NSC_LENGTH_MAX];
        uint16_t idx = 0;

        buffer[idx] = PTX_NSC_WRITE_CMD_OPCODE;
        idx++;

        /* Received address is expected to be in LSB format. It needs to be translated to MSB. */
        buffer[idx] = (uint8_t)((address >> 8) & 0x00FF);
        idx++;
        buffer[idx] = (uint8_t)(address & 0x00FF);
        idx++;

        buffer[idx] = len;
        idx++;

        memcpy(&buffer[idx], data, len);
        idx = (uint16_t)(idx + len);

        status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, buffer, idx);
        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t rsp_buffer[PTX_NSC_CMD_PROCESSOR_WRITE_RSPLEN];
            uint32_t rsp_buffer_len = PTX_NSC_CMD_PROCESSOR_WRITE_RSPLEN;
            memset(rsp_buffer, 0, sizeof(uint8_t)*PTX_NSC_CMD_PROCESSOR_WRITE_RSPLEN);

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_buffer, &rsp_buffer_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_WRITE_RSP);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_Write, rsp_buffer, rsp_buffer_len, NULL);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Complete response received. Check the ERROR_CODE. */
                if (NSC_ErrorCode_NoErr != rsp_buffer[PTX_NSC_RSP_ERRCODE_INDEX])
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_Loopback(ptx_NSC_t *nsc, uint8_t len, uint8_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Length can be 0. In such a case there shall be no data sent. */

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != data))
    {
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_Loopback, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t buffer[PTX_NSC_LOOPBACK_CMD_MAXLEN];
        uint16_t idx = 0;

        buffer[idx] = PTX_NSC_LOOPBACK_CMD_OPCODE;
        idx++;

        /* Payload length */
        buffer[idx] = len;
        idx++;

        /* Payload. Can be 0-255 length. */
        memcpy(&buffer[idx], data, len);
        idx = (uint16_t)(idx + len);

        status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, buffer, idx);
        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t rsp_buffer[PTX_NSC_LOOPBACK_RSP_MAXLEN];
            uint32_t rsp_buffer_len = PTX_NSC_LOOPBACK_VALID_RSP_MINLEN + len;
            memset(rsp_buffer, 0, PTX_NSC_LOOPBACK_RSP_MAXLEN);

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_buffer, &rsp_buffer_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_LOOPBACK_RSP);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_Loopback, rsp_buffer, rsp_buffer_len, NULL);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Response contains at least minimum LOOPBACK_RSP message length. We need to make additional check whether
                 * complete data was received as required.
                 */

                uint8_t payload_len = rsp_buffer[PTX_NSC_LOOPBACK_RSP_LEN_IDX];

                if (((payload_len + PTX_NSC_LOOPBACK_VALID_RSP_MINLEN) == rsp_buffer_len) && (len == payload_len) &&
                    (NSC_ErrorCode_NoErr == rsp_buffer[PTX_NSC_LOOPBACK_RSP_ERRCODE_INDEX]))
                {
                    /* All good. Check the received payload. All the bytes should match. */
                    for (uint8_t i = 0; i < payload_len; i++)
                    {
                        if (data[i] != rsp_buffer[PTX_NSC_LOOPBACK_RSP_PAYLOAD_IDX + i])
                        {
                            status = PTX_STATUS_E_NSC_PROTOCOL;
                            break;
                        }
                    }
                }
                else
                {
                    status = PTX_STATUS_E_NSC_RX_ERROR_LENGTH;
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_GetStatus(ptx_NSC_t *nsc, ptx_NSC_ParamStatus_t *paramStatus)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != paramStatus))
    {
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_GetStatus, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Set the struct to all zeros. */
        memset(paramStatus, 0, sizeof(ptx_NSC_ParamStatus_t));

        uint8_t buffer[PTX_NSC_GETSTATUS_CMD_LEN];
        uint8_t idx = 0;

        buffer[idx] = PTX_NSC_GET_STATUS_CMD_OPCODE;
        idx++;

        status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, buffer, idx);
        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t rsp_buffer[PTX_NSC_CMD_PROCESSOR_GETSTATUS_RSPLEN];
            uint32_t rsp_buffer_len = PTX_NSC_CMD_PROCESSOR_GETSTATUS_RSPLEN;
            memset(rsp_buffer, 0, rsp_buffer_len);

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_buffer, &rsp_buffer_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_GETSTATUS_RSP);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_GetStatus, rsp_buffer, rsp_buffer_len, NULL);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* All good. Length is checked in ProcessRsp, so we shall have the complete - maximum length - message here. */

                /* Set index at the first parameter. */
                idx = PTX_NSC_GETSTATUS_RSP_IDX_DDPC;

                /* Read out the currently used DDPC power level. */
                paramStatus->DdpcPl = rsp_buffer[idx];
                idx++;

                /* Read out the measured RSSI value. */
                paramStatus->Rssi = (uint32_t)(
                                        (PTX_UINT32_SHIFT_LEFT(rsp_buffer[idx], 16) & PTX_UINT32_3BY_MASK) |
                                        (PTX_UINT32_SHIFT_LEFT(rsp_buffer[idx + 1], 8) & PTX_UINT32_2BY_MASK) |
                                        (PTX_UINT32_SHIFT_LEFT(rsp_buffer[idx + 2], 0) & PTX_UINT32_1BY_MASK)
                                    );
                idx = (uint8_t)(idx + PTX_NSC_GETSTATUS_RSP_RSSI_LEN);

                /* Read out the collision information of the last reception. */
                paramStatus->CollInfo = rsp_buffer[idx];
                idx++;

                /* Read out the current IC temperature in degree celsius. */
                paramStatus->Temperature = rsp_buffer[idx];

                /* TODO add Remote RF field strength parameter (2 bytes) */
            }
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

