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
    Module      : NFC Soft Controller (NSC) Interface for Set Parameter
    File        : ptx_NSC_RfSetParameter.c

    Description : Implementation of NFC Soft Controller Interface Header for Set Parameter.
                  Contains function(s) to customize parameters of the forthcoming RF transactions,
                  as might be required by the currently active RF interface or protocol.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_RfSetParameter.h"
#include "ptx_NSC_StateMachine.h"


/*
 * RF-Set-Parameter defines.
 */
#define PTX_NSC_RF_SET_PARAM_BUFFER_SIZE        (20u)   /* Message buffer (max.) length: Opcode 1By + Param IDs: 8By +
                                                             All param values: 10By + EoC 1By. */


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_RfSetParameter_Dispatch(ptx_NSC_t *nscCtx, uint8_t *txBuffer, uint32_t txBufferLen);
static uint8_t ptx_NSC_Param_CheckBitrate(uint8_t bitrate);
static ptx_Status_t ptx_NSC_ParamParser_RfTech(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param);
static ptx_Status_t ptx_NSC_ParamParser_TxBitRate(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param);
static ptx_Status_t ptx_NSC_ParamParser_RxBitRate(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param);
static ptx_Status_t ptx_NSC_ParamParser_Parity(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param);
static ptx_Status_t ptx_NSC_ParamParser_Crc(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param);
static ptx_Status_t ptx_NSC_ParamParser_RxBitOffset(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param);
static ptx_Status_t ptx_NSC_ParamParser_MaxRspLim(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param);
static ptx_Status_t ptx_NSC_ParamParser_Fwt(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_RfSetParameter(ptx_NSC_t *nsc, ptx_NSC_RfSetParameter_Param_t *setParams, uint32_t paramCnt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != setParams) &&
         (0 < paramCnt) && (PTX_NSC_RF_SET_PARAM_MAXPARAMCOUNT >= paramCnt))
    {
        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_RfSetParam, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Allocate buffer for the RF-Set Param command. */
        uint8_t nsc_buf[PTX_NSC_RF_SET_PARAM_BUFFER_SIZE];
        uint8_t *nsc_buf_pos = nsc_buf;

        /* Empty the buffer to avoid Debug headaches */
        (void)memset(nsc_buf, 0, PTX_NSC_RF_SET_PARAM_BUFFER_SIZE);

        /* Append RF-Set Param command opcode. */
        *nsc_buf_pos = PTX_NSC_RF_SET_PARAM_CMD_OPCODE;
        nsc_buf_pos++;

        uint32_t nsc_buf_free_len = PTX_NSC_RF_SET_PARAM_BUFFER_SIZE - PTX_NSC_CMD_OPCODE_LEN;

        uint32_t param_idx = 0;

        /* Iterate over specified parameters. */
        do
        {
            switch (setParams[param_idx].Type)
            {
                case NSC_RfSetParameter_Param_Type_RfTech:
                    status = ptx_NSC_ParamParser_RfTech(&nsc_buf_pos, &nsc_buf_free_len, &setParams[param_idx].Value);
                    break;
                case NSC_RfSetParameter_Param_Type_TxBitRate:
                    status = ptx_NSC_ParamParser_TxBitRate(&nsc_buf_pos, &nsc_buf_free_len, &setParams[param_idx].Value);
                    break;
                case NSC_RfSetParameter_Param_Type_RxBitRate:
                    status = ptx_NSC_ParamParser_RxBitRate(&nsc_buf_pos, &nsc_buf_free_len, &setParams[param_idx].Value);
                    break;
                case NSC_RfSetParameter_Param_Type_Parity:
                    status = ptx_NSC_ParamParser_Parity(&nsc_buf_pos, &nsc_buf_free_len, &setParams[param_idx].Value);
                    break;
                case NSC_RfSetParameter_Param_Type_Crc:
                    status = ptx_NSC_ParamParser_Crc(&nsc_buf_pos, &nsc_buf_free_len, &setParams[param_idx].Value);
                    break;
                case NSC_RfSetParameter_Param_Type_RxBitOffset:
                    status = ptx_NSC_ParamParser_RxBitOffset(&nsc_buf_pos, &nsc_buf_free_len, &setParams[param_idx].Value);
                    break;
                case NSC_RfSetParameter_Param_Type_MaxRspLim:
                    status = ptx_NSC_ParamParser_MaxRspLim(&nsc_buf_pos, &nsc_buf_free_len, &setParams[param_idx].Value);
                    break;
                case NSC_RfSetParameter_Param_Type_Fwt:
                    status = ptx_NSC_ParamParser_Fwt(&nsc_buf_pos, &nsc_buf_free_len, &setParams[param_idx].Value);
                    break;
                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                param_idx++;
            }
        }
        while ( (param_idx < paramCnt) && (PTX_STATUS_SUCCESS == status) );

        if (PTX_STATUS_SUCCESS == status)
        {
            if (nsc_buf_free_len != 0)
            {
                /* All data has been parsed successfully and there is space left for the EoC parameter. */

                /* Append 'End-of-Command'. */
                *nsc_buf_pos = NSC_RfSetParameter_Param_Type_EoC;
                nsc_buf_pos++;

                /* Send the data. */
                status = ptx_NSC_RfSetParameter_Dispatch(nsc, nsc_buf, (uint32_t)(nsc_buf_pos - nsc_buf));
            }
            else
            {
                status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
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

static ptx_Status_t ptx_NSC_RfSetParameter_Dispatch(ptx_NSC_t *nscCtx, uint8_t *txBuffer, uint32_t txBufferLen)
{
    ptx_Status_t status = ptx_NSC_CmdProcessor_DispatchMsg(nscCtx, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, txBuffer, txBufferLen);

    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t nsc_rsp[PTX_NSC_CMD_PROCESSOR_RFSETPARAM_RSPLEN];
        uint32_t nsc_rsp_len = sizeof(nsc_rsp);

        status = ptx_NSC_CmdProcessor_FetchMsg(nscCtx, nsc_rsp, &nsc_rsp_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_RFSETPARAM_RSP);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_CmdProcessor_ProcessRsp(nscCtx, ptx_NSC_CmdProcessor_RspId_RfSetParam, nsc_rsp, nsc_rsp_len, NULL);
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_ParamParser_RfTech(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;

    if (PTX_NSC_RF_SET_PARAM_RF_TECH_LEN < *availableLen)
    {
        uint32_t cmd_idx = 0;

        (*bufferPos)[cmd_idx] = NSC_RfSetParameter_Param_Type_RfTech;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param->RfTech;
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
};

static ptx_Status_t ptx_NSC_ParamParser_TxBitRate(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t isValidBitrate = ptx_NSC_Param_CheckBitrate(param->TxBitRate.BitrateValue);

    if ((0 != isValidBitrate) && (PTX_NSC_RF_SET_PARAM_TX_BIT_RATE_LEN < *availableLen))
    {
        uint32_t cmd_idx = 0;

        (*bufferPos)[cmd_idx] = NSC_RfSetParameter_Param_Type_TxBitRate;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param->TxBitRate.BitrateValue;
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else if (isValidBitrate)
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }
    else
    {
        /* Invalid bit-rate */
    }

    return status;
}

static uint8_t ptx_NSC_Param_CheckBitrate(uint8_t bitrate)
{
    uint8_t valid_bitrate = 0;

    switch (bitrate)
    {
        case PTX_NSC_RF_SET_BITRATE_106:    /* FALL THROUGH */
        case PTX_NSC_RF_SET_BITRATE_212:    /* FALL THROUGH */
        case PTX_NSC_RF_SET_BITRATE_424:    /* FALL THROUGH */
        case PTX_NSC_RF_SET_BITRATE_848:    /* FALL THROUGH */
        case PTX_NSC_RF_SET_BITRATE_26_5:   /* FALL THROUGH */
        case PTX_NSC_RF_SET_BITRATE_53:
            valid_bitrate = 1;
            break;
        default:
            /* Invalid bit-rate value already set. */
            break;
    }

    return valid_bitrate;
}

static ptx_Status_t ptx_NSC_ParamParser_RxBitRate(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t isValidBitrate = ptx_NSC_Param_CheckBitrate(param->RxBitRate.BitrateValue);

    if ((0 != isValidBitrate) && (PTX_NSC_RF_SET_PARAM_RX_BIT_RATE_LEN < *availableLen))
    {
        uint32_t cmd_idx = 0;

        (*bufferPos)[cmd_idx] = NSC_RfSetParameter_Param_Type_RxBitRate;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param->RxBitRate.BitrateValue;
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }
    else if (isValidBitrate)
    {
        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
    }
    else
    {
        /* invalid bit-rate */
    }

    return status;
}

static ptx_Status_t ptx_NSC_ParamParser_Parity(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;

    if (PTX_NSC_RF_SET_PARAM_PARITY_LEN < *availableLen)
    {
        uint32_t cmd_idx = 0;

        (*bufferPos)[cmd_idx] = NSC_RfSetParameter_Param_Type_Parity;
        cmd_idx++;

        uint8_t parity_byte = (uint8_t) (((param->Parity.RxParity & PTX_NSC_RF_SET_PARAM_FLAG_MASK) << PTX_NSC_RF_SET_PARAM_RX_START_BIT)
                                         | ((param->Parity.TxParity & PTX_NSC_RF_SET_PARAM_FLAG_MASK)));

        (*bufferPos)[cmd_idx] = parity_byte;
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_NSC_ParamParser_Crc(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;

    if (PTX_NSC_RF_SET_PARAM_CRC_LEN < *availableLen)
    {
        uint32_t cmd_idx = 0;

        (*bufferPos)[cmd_idx] = NSC_RfSetParameter_Param_Type_Crc;
        cmd_idx++;

        uint8_t crc_byte = (uint8_t) (((param->Crc.RxCrc & PTX_NSC_RF_SET_PARAM_FLAG_MASK) << PTX_NSC_RF_SET_PARAM_RX_START_BIT)
                                      | ((param->Crc.TxCrc & PTX_NSC_RF_SET_PARAM_FLAG_MASK)));

        (*bufferPos)[cmd_idx] = crc_byte;
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_NSC_ParamParser_RxBitOffset(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;

    if (PTX_NSC_RF_SET_PARAM_RX_BIT_OFFSET_LEN < *availableLen)
    {
        uint32_t cmd_idx = 0;

        (*bufferPos)[cmd_idx] = NSC_RfSetParameter_Param_Type_RxBitOffset;
        cmd_idx++;

        uint8_t rx_bit_offset_byte = (uint8_t)((param->RxBitOffset.RxBitOffsetValue & PTX_NSC_RF_SET_PARAM_RX_BITOFFSET_MASK) << PTX_NSC_RF_SET_PARAM_RX_START_BIT);

        (*bufferPos)[cmd_idx] = rx_bit_offset_byte;
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_NSC_ParamParser_MaxRspLim(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;

    if (PTX_NSC_RF_SET_PARAM_BIT_MAXRSPLIM_LEN < *availableLen)
    {
        uint32_t cmd_idx = 0;

        (*bufferPos)[cmd_idx] = NSC_RfSetParameter_Param_Type_MaxRspLim;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param->MaxRspLim.MaxRspLimValue;
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_NSC_ParamParser_Fwt(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfSetParameter_Param_Value_t *param)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;

    if (PTX_NSC_RF_SET_PARAM_BIT_FWT_LEN < *availableLen)
    {
        uint32_t cmd_idx = 0;

        (*bufferPos)[cmd_idx] = NSC_RfSetParameter_Param_Type_Fwt;
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param->Fwt.FwtValue[PTX_NSC_RF_SET_PARAM_FWT_THIRD_BYTE_INDEX];
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param->Fwt.FwtValue[PTX_NSC_RF_SET_PARAM_FWT_SECOND_BYTE_INDEX];
        cmd_idx++;

        (*bufferPos)[cmd_idx] = param->Fwt.FwtValue[PTX_NSC_RF_SET_PARAM_FWT_FIRST_BYTE_INDEX];
        cmd_idx++;

        *bufferPos += cmd_idx;
        *availableLen -= cmd_idx;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}
