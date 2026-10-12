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
    Module      : NFC Soft Controller (NSC) Interface for Presence Check
    File        : ptx_NSC_RfPresenceCheck.c

    Description : Implementation of NFC Soft Controller Interface for Presence Check.
                  Contains RF protocol specific methods for card presence checking.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NSC_RfPresenceCheck.h"
#include "ptx_NSC_Data.h"


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

static ptx_Status_t ptx_NSC_RfPresenceCheck_CheckResponseAck(ptx_NSC_t* nsc);
static ptx_Status_t ptx_NSC_RfPresenceCheck_CheckResponseEmpty(ptx_NSC_t* nsc);
static ptx_Status_t ptx_NSC_RfPresenceCheck_CheckResponseRfFrame(ptx_NSC_t *nsc, uint32_t min_expected_rsp_len, uint32_t timeout);
static ptx_Status_t ptx_NSC_RfPresenceCheck_CheckSpecialCaseT2TRsp(uint8_t *rsp_buf, uint32_t rsp_buf_len);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_RfPresenceCheck_EmptyFrame (ptx_NSC_t* nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_RfPresenceCheck_EmptyFrame");

        uint8_t dummy = 0;

        status = ptx_NSC_DataTx(nsc, &dummy, 0, 0, 0);
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RfPresenceCheck_CheckResponseEmpty(nsc);
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfPresenceCheck_RNak (ptx_NSC_t* nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_RfPresenceCheck_RNak");

        status = ptx_NSC_CtrlTx(nsc, PTX_NSC_DATA_MSG_NACK);
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RfPresenceCheck_CheckResponseAck(nsc);
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfPresenceCheck_AttentionCmd (ptx_NSC_t* nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_RfPresenceCheck_AttentionCmd");

        status = ptx_NSC_CtrlTx(nsc, PTX_NSC_DATA_MSG_NACK);
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RfPresenceCheck_CheckResponseAck(nsc);
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfPresenceCheck_FrameRf (ptx_NSC_t* nsc, ptx_NSC_PresCheck_FrameRf_Types_t frameRfType, uint8_t *id)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_RfPresenceCheck_FrameRf");

        /* Maximum size of the data buffer is defined by the longest protocol message, which is in this case T3T.
         * T2T := 2 Bytes
         * T3T := 16 Bytes
         * T5T := 11 Bytes
         */
        uint8_t frame_rf_buf[PTX_NSC_PRESCHECK_T3T_FRAMERF_LEN];
        uint32_t frame_rf_buf_len = 0;
        uint8_t data_idx = 0;

        uint32_t min_expected_res_len = 0;
        uint32_t timeout = 0;

        switch (frameRfType)
        {
            case ptx_NSC_PresCheck_FrameRf_T2T_Read:
                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_T2T_READ_CMD;
                data_idx++;
                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_BLNO;
                frame_rf_buf_len = PTX_NSC_PRESCHECK_T2T_FRAMERF_LEN;

                min_expected_res_len = PTX_NSC_PRESCHECK_T2T_MIN_RES_LEN;
                timeout = PTX_NSC_PRESCHECK_T2T_MAX_RES_TIMEOUT;

                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_NSC_PresCheck_FrameRf_T3T_Check:
                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_T3T_FRAMERF_LEN;
                data_idx++;

                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_T3T_CHECK_CMD;
                data_idx++;

                memcpy(&frame_rf_buf[data_idx], id, PTX_NSC_PRESCHECK_T3T_NFCID2_LEN);
                data_idx = (uint8_t)(data_idx + PTX_NSC_PRESCHECK_T3T_NFCID2_LEN);

                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_T3T_NOS;
                data_idx++;
                frame_rf_buf[data_idx] = (uint8_t)((PTX_NSC_PRESCHECK_T3T_SERVICECODE & 0xFF00) >> 8);
                data_idx++;
                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_T3T_SERVICECODE & 0x00FF;
                data_idx++;
                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_T3T_NOB;
                data_idx++;
                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_T3T_BLOCK_ENC;
                data_idx++;
                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_BLNO;


                frame_rf_buf_len = PTX_NSC_PRESCHECK_T3T_FRAMERF_LEN;

                min_expected_res_len = PTX_NSC_PRESCHECK_T3T_MIN_RES_LEN;
                timeout = PTX_NSC_PRESCHECK_T3T_MAX_RES_TIMEOUT;

                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_NSC_PresCheck_FrameRf_T5T_Read:
                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_T5T_REQ_FLAG;
                data_idx++;
                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_T5T_READ_SINGLE_CMD;
                data_idx++;

                memcpy(&frame_rf_buf[data_idx], id, PTX_NSC_PRESCHECK_T5T_UID_LEN);
                data_idx = (uint8_t)(data_idx + PTX_NSC_PRESCHECK_T5T_UID_LEN);

                frame_rf_buf[data_idx] = PTX_NSC_PRESCHECK_BLNO;

                frame_rf_buf_len = PTX_NSC_PRESCHECK_T5T_FRAMERF_LEN;

                min_expected_res_len = PTX_NSC_PRESCHECK_T5T_MIN_RES_LEN;
                timeout = PTX_NSC_PRESCHECK_T5T_MAX_RES_TIMEOUT;

                status = PTX_STATUS_SUCCESS;
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Clean residual bit setting because it might influence NSC Data message. */
            nsc->ResidualTxBits = 0;
            status = ptx_NSC_DataTx(nsc, frame_rf_buf, frame_rf_buf_len, 0, PTX_NSC_DATA_TIMEOUT_DEFAULT);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RfPresenceCheck_CheckResponseRfFrame(nsc, min_expected_res_len, timeout);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_Sleep(nsc->HAL, PTX_NSC_PRESCHECK_SUCCESS_WAIT);
        }

    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_RfPresenceCheck_CheckResponseAck(ptx_NSC_t* nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t rsp_buf[PTX_NSC_LENGTH_MAX];
    uint32_t rsp_buf_len = PTX_NSC_LENGTH_MAX;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_RfPresenceCheck_CheckResponseAck");

        status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_buf, &rsp_buf_len, PTX_NSC_TIMEOUT_DEFAULT, 0);

        if ((PTX_STATUS_SUCCESS == status) && (0 != rsp_buf_len))
        {
            switch (rsp_buf[PTX_NSC_DATA_MSG_IDX_OPCODE])
            {
                case PTX_NSC_DATA_MSG_OPCODE:
                    if ((rsp_buf[PTX_NSC_DATA_MSG_IDX_FCB] == PTX_NSC_DATA_MSG_FCB_BIT_CTRL) &&
                        (rsp_buf[PTX_NSC_DATA_MSG_IDX_CTRL] == PTX_NSC_DATA_MSG_ACK))
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    else
                    {
                        status = PTX_STATUS_E_NSC_PROTOCOL;
                    }
                    break;

                case PTX_NSC_CMD_PROCESSOR_ERROR_NTF:
                    status = PTX_STATUS_E_NSC_RFERROR;
                    break;

                default:
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                    break;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_RfPresenceCheck_CheckResponseEmpty(ptx_NSC_t* nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t rsp_buf[PTX_NSC_LENGTH_MAX];
    uint32_t rsp_buf_len = PTX_NSC_LENGTH_MAX;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_RfPresenceCheck_CheckResponseEmpty");

        status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_buf, &rsp_buf_len, PTX_NSC_TIMEOUT_DEFAULT, 0);

        /* If presence check was successful, we shall get empty NSC_RF_DATA_MSG or with a payload. */

        if ((PTX_STATUS_SUCCESS == status) && (0 != rsp_buf_len))
        {
            switch (rsp_buf[PTX_NSC_DATA_MSG_IDX_OPCODE])
            {
                case PTX_NSC_DATA_MSG_OPCODE:
                    if (PTX_NSC_PRESCHECK_MSG_FCB == rsp_buf[PTX_NSC_DATA_MSG_IDX_FCB])
                    {
                        status = PTX_STATUS_SUCCESS;
                    }
                    else
                    {
                        status = PTX_STATUS_E_NSC_PROTOCOL;
                    }
                    break;

                case PTX_NSC_CMD_PROCESSOR_ERROR_NTF:
                    status = PTX_STATUS_E_NSC_RFERROR;
                    break;

                default:
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                    break;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_RfPresenceCheck_CheckResponseRfFrame(ptx_NSC_t* nsc, uint32_t min_expected_rsp_len, uint32_t timeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t rsp_buf[PTX_NSC_LENGTH_MAX];
    uint32_t rsp_buf_len = PTX_NSC_LENGTH_MAX;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (0 != min_expected_rsp_len) && (0 != timeout))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_RfPresenceCheck_CheckResponseRfFrame");

        status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_buf, &rsp_buf_len, timeout, 0);

        if ((PTX_STATUS_SUCCESS == status) && (0 != rsp_buf_len))
        {
            switch (rsp_buf[PTX_NSC_DATA_MSG_IDX_OPCODE])
            {
                case PTX_NSC_DATA_MSG_OPCODE:
                    if ( (rsp_buf[PTX_NSC_DATA_MSG_IDX_FCB] != PTX_NSC_DATA_MSG_FCB_BIT_CTRL) &&
                         (rsp_buf[PTX_NSC_DATA_MSG_IDX_DATALEN] != 0))
                    {
                        if (0x00 == (PTX_NSC_PRESCHECK_RCV_MSG_RF_ERR_MASK & rsp_buf[rsp_buf[PTX_NSC_DATA_MSG_IDX_DATALEN] + PTX_NSC_DATA_MSG_IDX_DATA]) &&
                            (min_expected_rsp_len <= rsp_buf[PTX_NSC_DATA_MSG_IDX_DATALEN]))
                        {
                            if (PTX_NSC_PRESCHECK_T2T_MIN_RES_LEN == rsp_buf[PTX_NSC_DATA_MSG_IDX_DATALEN])
                            {
                                status = ptx_NSC_RfPresenceCheck_CheckSpecialCaseT2TRsp(rsp_buf, rsp_buf_len);
                            }
                            else
                            {
                                status = PTX_STATUS_SUCCESS;
                            }
                        }
                        else
                        {
                            status = PTX_STATUS_E_NSC_RFERROR;
                        }
                    }
                    else
                    {
                        status = PTX_STATUS_E_NSC_PROTOCOL;
                    }
                    break;

                case PTX_NSC_CMD_PROCESSOR_ERROR_NTF:
                    status = PTX_STATUS_E_NSC_RFERROR;
                    break;

                default:
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                    break;
            }
        }
    }

    return status;
}


static ptx_Status_t ptx_NSC_RfPresenceCheck_CheckSpecialCaseT2TRsp(uint8_t *rsp_buf, uint32_t rsp_buf_len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != rsp_buf) && (0 != rsp_buf_len))
    {
        if (PTX_NSC_PRESCHECK_T2T_RES_BITS == (rsp_buf[rsp_buf[PTX_NSC_DATA_MSG_IDX_DATALEN] + PTX_NSC_DATA_MSG_IDX_DATA]))
        {
            if (PTX_NSC_PRESCHECK_T2T_RES_ACK != (rsp_buf[PTX_NSC_DATA_MSG_IDX_DATA] & PTX_NSC_PRESCHECK_T2T_RES_BITS_MASK))
            {
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }
        }
        else
        {
            status = PTX_STATUS_E_NSC_PROTOCOL;
        }
    }


    return status;
}
