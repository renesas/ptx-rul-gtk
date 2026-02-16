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
    Module      : NFC Soft Controller (NSC) Interface for Testing
    File        : ptx_NSC_RfTest.c

    Description : Implementation of NFC Soft Controller Interface for Testing.
                  Contains function(s) to evaluate system RF performance.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_RfTest.h"
#include "ptx_NSC_StateMachine.h"
#include "ptx_NSC_RfDiscover.h"
#include "ptx_NSC_RfDeactivate.h"
#include "ptx_NSC_RfSetParameter.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NSC_RF_TEST_BUFFER_SIZE_DDPC    (3U)        /* Command length for DDPC sequence.  */
#define PTX_NSC_RF_TEST_BUFFER_SIZE_RFAMP   (12U)       /* Command length for RF Amplitude measurement sequence.  */
#define PTX_NSC_RF_TEST_BUFFER_SIZE_PRBS    (5U)        /* Command length for PRBS9 / PRBS15 sequence. */
#define PTX_NSC_RF_TEST_BUFFER_SIZE_CARRIER (3U)        /* Command length for Carrier sequence*/
#define PTX_NSC_RF_TEST_RSPLEN_RFAMP        (5U)        /* NSC RF Test Response length for RfAmplitude measurement. */
#define PTX_NSC_RF_TEST_RSPLEN_DDPC         (2U)        /* NSC RF Test Response length for DDPC sequence. */
#define PTX_NSC_RF_TEST_RSPLEN_PRBS         (2U)        /* NSC RF Test Response length for PRBS sequence. */
#define PTX_NSC_RF_TEST_RSPLEN_CARRIER      (2U)        /* NSC RF Test Response length for Carrier sequence. */

#define PTX_NSC_RF_TEST_RSP_ID_OFFSET       (0U)        /* NSC response opcode byte offset */
#define PTX_NSC_RF_TEST_RSP_STATUS_OFFSET   (1U)        /* NSC response status byte offset */
#define PTX_NSC_RF_TEST_RSP_PAYLOAD_OFFSET  (2U)        /* NSC response payload offset */

#define PTX_NSC_RF_TEST_AVGFACTORINT_MAX    (5U)        /* Maximum valid value for AVG_FACTOR_INT parameter */

#define PTX_NSC_RF_TEST_FRAME_CFG_SOF_EOF   (7U)        /* bit 7 in frame config byte to disable SoF/EoF */
#define PTX_NSC_RF_TEST_FRAME_CFG_START_STOP (6U)       /* bit 6 in frame config byte to disable start stop bits */
#define PTX_NSC_RF_TEST_FRAME_CFG_PARITY    (5U)        /* bit 5 in frame config byte to disable parity bits */
#define PTX_NSC_RF_TEST_FRAME_CFG_PRBS_TYPE (0U)        /* bit 0 in frame config byte to set PRBS type 15 (=1) / 9 (=0) */

#define PTX_NSC_RFTEST_PRBS_SEQ_ID          (uint8_t) 0x04 /** Test sequence ID for PRBS 9/15 */

#define PTX_NAX_MAX_NR_PARAMETERS           (3u)        /* Maxium number of parameters for Discovery */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NSC_RfTest_ProcessRsp(uint8_t *msg, uint32_t msgLen);
static ptx_Status_t ptx_NSC_RfTest_RfAmp(ptx_NSC_t* nsc, ptx_NSC_RfTest_Params_RfAmp_t *inParams, ptx_NSC_RfTest_RfAmp_Out_t *outRes);
static ptx_Status_t ptx_NSC_RfTest_DDPC(ptx_NSC_t* nsc, ptx_NSC_RfTest_Params_DDPC_t *inParams);
static ptx_Status_t ptx_NSC_RfTest_PRBS(ptx_NSC_t* nsc, ptx_NSC_RfTest_Params_PRBS_t *inParams, ptx_NSC_RfTest_SeqId_t PRBSType);
static ptx_Status_t ptx_NSC_RfTest_Carrier(ptx_NSC_t* nsc, ptx_NSC_RfTest_Params_Carrier_t *inParams);

static ptx_Status_t ptx_NSC_SetTestMode(ptx_NSC_t* nsc);
static ptx_Status_t ptx_NSC_PollModeDiscovery(ptx_NSC_t* nsc);
static ptx_Status_t ptx_NSC_DeactivateToIdle(ptx_NSC_t* nsc);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NSC_RfTest(ptx_NSC_t* nsc, ptx_NSC_RfTest_InParams_t *inParams, ptx_NSC_RfTest_Output_t *outRes)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != inParams) )
    {
        status = ptx_NSC_SetTestMode(nsc);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

        /* Check valid parameters for this specific test. */

        switch (inParams->SeqId)
        {
            case NSC_RfTest_SeqId_MeasRFAmp:
            {
                if (inParams->Params.RfAmpParams.AvgFactorInt > PTX_NSC_RF_TEST_AVGFACTORINT_MAX)
                {
                    inParams->Params.RfAmpParams.AvgFactorInt = PTX_NSC_RF_TEST_AVGFACTORINT_MAX;
                }
                status = ptx_NSC_RfTest_RfAmp(nsc, &inParams->Params.RfAmpParams, &outRes->RfAmpOut);
                break;
            }

            case NSC_RfTest_SeqId_DDPC:
            {
                status = ptx_NSC_RfTest_DDPC(nsc, &inParams->Params.DDPCParams);
                break;
            }

            case NSC_RfTest_SeqId_PRBS_9:
            /* FALL THROUGH */
            case NSC_RfTest_SeqId_PRBS_15:
            {
                status = ptx_NSC_RfTest_PRBS(nsc, &inParams->Params.PRBSParams, inParams->SeqId);
                break;
            }

            case NSC_RfTest_SeqId_Carrier:
            {
                status = ptx_NSC_RfTest_Carrier(nsc, &inParams->Params.CarrierParams);
                break;
            }

            default:
            {
                /* Do nothing. Error is already set. */
                break;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_RfTestStop(ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        status = ptx_NSC_DeactivateToIdle(nsc);
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_RfTest_RfAmp(ptx_NSC_t* nsc, ptx_NSC_RfTest_Params_RfAmp_t *inParams, ptx_NSC_RfTest_RfAmp_Out_t *outRes)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Allocate buffer for the RF-Test command. */
    uint8_t nsc_buf[PTX_NSC_RF_TEST_BUFFER_SIZE_RFAMP];
    uint8_t *nsc_buf_pos = &nsc_buf[0];
    ptx_NSC_RfTest_Params_RfAmp_t *params = inParams;

    *nsc_buf_pos = PTX_NSC_RF_TEST_CMD_OPCODE;
    nsc_buf_pos++;

    *nsc_buf_pos = (uint8_t) NSC_RfTest_SeqId_MeasRFAmp;
    nsc_buf_pos++;

    /* RF Amplitude measurement specific parameters */
    *nsc_buf_pos = params->XtalDiv2En;
    nsc_buf_pos++;

    uint8_t temp[4];
    temp[0] = ((uint8_t *)&params->GatePw)[1];
    temp[1] = ((uint8_t *)&params->GatePw)[0];
    (void) memcpy(nsc_buf_pos, &temp[0], 2U);
    nsc_buf_pos += 2U;

    temp[0] = ((uint8_t *)&params->SysCntTg)[1];
    temp[1] = ((uint8_t *)&params->SysCntTg)[0];
    (void) memcpy(nsc_buf_pos, &temp[0], 2U);
    nsc_buf_pos += 2U;

    temp[0] = ((uint8_t *)(&(params->NHost)))[3];
    temp[1] = ((uint8_t *)(&(params->NHost)))[2];
    temp[2] = ((uint8_t *)(&(params->NHost)))[1];
    temp[3] = ((uint8_t *)(&(params->NHost)))[0];
    (void) memcpy(nsc_buf_pos, &temp[0], 4U);
    nsc_buf_pos += 4U;

    *nsc_buf_pos = params->AvgFactorInt;
    nsc_buf_pos++;

    status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, nsc_buf, (uint32_t)(nsc_buf_pos - nsc_buf));
    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t nsc_rsp[PTX_NSC_RF_TEST_RSPLEN_RFAMP];
        uint32_t nsc_rsp_len = PTX_NSC_RF_TEST_RSPLEN_RFAMP;

        status = ptx_NSC_CmdProcessor_FetchMsg(nsc, nsc_rsp, &nsc_rsp_len, PTX_NSC_TIMEOUT_DEFAULT, 0);

        if (PTX_STATUS_SUCCESS == status)
        {
            if (PTX_NSC_RF_TEST_RSPLEN_RFAMP == nsc_rsp_len)
            {
                status = ptx_NSC_RfTest_ProcessRsp(nsc_rsp, nsc_rsp_len);
            }
            else
            {
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Everything fine. Extract the response. */
            outRes->RfAmplitude = 0;
            uint8_t rf_amplitude[3];
            (void)memcpy(&rf_amplitude[0], &nsc_rsp[PTX_NSC_RF_TEST_RSP_PAYLOAD_OFFSET], 3U);

            /* The order of bytes received is LSB first */
            ((uint8_t *) & (outRes->RfAmplitude))[0] = rf_amplitude[0];
            ((uint8_t *) & (outRes->RfAmplitude))[1] = rf_amplitude[1];
            ((uint8_t *) & (outRes->RfAmplitude))[2] = rf_amplitude[2];
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_RfTest_DDPC(ptx_NSC_t* nsc, ptx_NSC_RfTest_Params_DDPC_t *inParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Allocate buffer for the RF-Test command. */
    uint8_t nsc_buf[PTX_NSC_RF_TEST_BUFFER_SIZE_DDPC];
    uint8_t *nsc_buf_pos = nsc_buf;
    ptx_NSC_RfTest_Params_DDPC_t *params = inParams;

    *nsc_buf_pos = PTX_NSC_RF_TEST_CMD_OPCODE;
    nsc_buf_pos++;

    *nsc_buf_pos = (uint8_t) NSC_RfTest_SeqId_DDPC;
    nsc_buf_pos++;

    *nsc_buf_pos = params->DDPC_Period;
    nsc_buf_pos++;

    status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, nsc_buf, (uint32_t)(nsc_buf_pos - nsc_buf));
    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t nsc_rsp[PTX_NSC_RF_TEST_RSPLEN_DDPC];
        uint32_t nsc_rsp_len = PTX_NSC_RF_TEST_RSPLEN_DDPC;

        status = ptx_NSC_CmdProcessor_FetchMsg(nsc, nsc_rsp, &nsc_rsp_len, PTX_NSC_TIMEOUT_DEFAULT, 0);

        if (PTX_STATUS_SUCCESS == status)
        {
            if (PTX_NSC_RF_TEST_RSPLEN_DDPC == nsc_rsp_len)
            {
                status = ptx_NSC_RfTest_ProcessRsp(nsc_rsp, nsc_rsp_len);
            }
            else
            {
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_RfTest_PRBS(ptx_NSC_t* nsc, ptx_NSC_RfTest_Params_PRBS_t *inParams, ptx_NSC_RfTest_SeqId_t PRBSType)
{

    ptx_Status_t st = PTX_STATUS_SUCCESS;

    /* Only poll mode technologies are supported */
    if ((RF_TEST_TECH_A_POLL != inParams->Technology) && (RF_TEST_TECH_B_POLL != inParams->Technology) &&
        (RF_TEST_TECH_F_POLL != inParams->Technology) && (RF_TEST_TECH_V_POLL != inParams->Technology))
    {
        st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    /* Check validity of flags */
    if ((PTX_NSC_RFTEST_DISAB_SOFEOF_B != inParams->EnabSofEof) && (PTX_NSC_RFTEST_ENAB_SOFEOF_B != inParams->EnabSofEof))
    {
        st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    if ((PTX_NSC_RFTEST_DISAB_STARTSTOP_B != inParams->EnabStartStopBits) && (PTX_NSC_RFTEST_ENAB_STARTSTOP_B != inParams->EnabStartStopBits))
    {
        st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    if ((PTX_NSC_RFTEST_DISAB_PARITY_A != inParams->EnabParityBits) && (PTX_NSC_RFTEST_ENAB_PARITY_A != inParams->EnabParityBits))
    {
        st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    /* Supported bit-rates */
    if ((RF_TEST_BITRATE_106 != inParams->Bitrate) &&
        (RF_TEST_BITRATE_212 != inParams->Bitrate) &&
        (RF_TEST_BITRATE_424 != inParams->Bitrate) &&
        (RF_TEST_BITRATE_848 != inParams->Bitrate) &&
        (RF_TEST_BITRATE_26_5 != inParams->Bitrate) &&
        (RF_TEST_BITRATE_53 != inParams->Bitrate))
    {
        st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    /* Supported bit-rates for NFC-F are 212/424 kbits/s */
    if ((RF_TEST_TECH_F_POLL == inParams->Technology) && ((RF_TEST_BITRATE_106 == inParams->Bitrate)
                                                          || (RF_TEST_BITRATE_848 == inParams->Bitrate)
                                                          || (RF_TEST_BITRATE_26_5 == inParams->Bitrate)
                                                          || (RF_TEST_BITRATE_53 == inParams->Bitrate)))
    {
        st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    /* Not supported bit-rates for NFC-A and B are 26.5/53 kbits/s */
    if ((RF_TEST_TECH_A_POLL == inParams->Technology || RF_TEST_TECH_B_POLL == inParams->Technology)
        && ((RF_TEST_BITRATE_26_5 == inParams->Bitrate) || (RF_TEST_BITRATE_53 == inParams->Bitrate)))
    {
        st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    /* Supported bit-rates for NFC-V are 26.5/53 kbits/s */
    if ((RF_TEST_TECH_V_POLL == inParams->Technology) && (RF_TEST_BITRATE_848 >= inParams->Bitrate))
    {
        st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    /* NFC_A_PARITY only applicable if NFC_POLL_A is selected */
    if ((RF_TEST_TECH_A_POLL != inParams->Technology) && (PTX_NSC_RFTEST_ENAB_PARITY_A == inParams->EnabParityBits))
    {
        st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    /* NFC_B_SOF_EOF only applicable if NFC_POLL_B is selected */
    if ((RF_TEST_TECH_B_POLL != inParams->Technology) && (PTX_NSC_RFTEST_ENAB_SOFEOF_B == inParams->EnabSofEof))
    {
        st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    if (PTX_STATUS_SUCCESS == st)
    {
        ptx_NSC_RfSetParameter_Param_t params;
        params.Type = NSC_RfSetParameter_Param_Type_Crc;
        params.Value.Crc.RxCrc = inParams->EnabCRC ? NSC_RfSetParameter_Crc_Enable : NSC_RfSetParameter_Crc_Disable;
        params.Value.Crc.TxCrc = inParams->EnabCRC ? NSC_RfSetParameter_Crc_Enable : NSC_RfSetParameter_Crc_Disable;

        st = ptx_NSC_RfSetParameter(nsc, &params, 1);
    }

    /* Build command if checks were successful. */
    if (PTX_STATUS_SUCCESS == st)
    {
        /* Allocate buffer for the RF-Test command. */
        uint8_t nsc_buf[PTX_NSC_RF_TEST_BUFFER_SIZE_PRBS];
        uint8_t *nsc_buf_pos = nsc_buf;

        *nsc_buf_pos = PTX_NSC_RF_TEST_CMD_OPCODE;
        nsc_buf_pos++;

        *nsc_buf_pos = PTX_NSC_RFTEST_PRBS_SEQ_ID;
        nsc_buf_pos++;

        *nsc_buf_pos = (uint8_t) inParams->Technology;
        nsc_buf_pos++;

        *nsc_buf_pos = (uint8_t) inParams->Bitrate;
        nsc_buf_pos++;

        /* build frame config byte */
        uint8_t rf_frame_config = 0x00;

        if (PTX_NSC_RFTEST_DISAB_SOFEOF_B == inParams->EnabSofEof)
        {
            /* set bit to 1 to disable */
            rf_frame_config = rf_frame_config | ((uint8_t) 1 << (uint8_t) PTX_NSC_RF_TEST_FRAME_CFG_SOF_EOF);
        }

        if (PTX_NSC_RFTEST_DISAB_STARTSTOP_B == inParams->EnabStartStopBits)
        {
            /* set bit to 1 to disable */
            rf_frame_config = rf_frame_config | ((uint8_t) 1 << (uint8_t) PTX_NSC_RF_TEST_FRAME_CFG_START_STOP);
        }

        if (PTX_NSC_RFTEST_DISAB_PARITY_A == inParams->EnabParityBits)
        {
            /* set bit to 1 to disable */
            rf_frame_config = rf_frame_config | ((uint8_t) 1 << (uint8_t) PTX_NSC_RF_TEST_FRAME_CFG_START_STOP);
        }

        switch (PRBSType)
        {
            case NSC_RfTest_SeqId_PRBS_9:
                /* Nothing to do bit 0 of rf_frame_config already set to 0 */
                break;
            case NSC_RfTest_SeqId_PRBS_15:
                /* set bit 0 to 1 */
                rf_frame_config = rf_frame_config | ((uint8_t) 1 << (uint8_t) PTX_NSC_RF_TEST_FRAME_CFG_PRBS_TYPE);
                break;

            default:
                break;
        }

        *nsc_buf_pos = (uint8_t) rf_frame_config;
        nsc_buf_pos++;

        st = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, nsc_buf, (uint32_t)(nsc_buf_pos - nsc_buf));
        if (PTX_STATUS_SUCCESS == st)
        {
            uint8_t nsc_rsp[PTX_NSC_RF_TEST_RSPLEN_PRBS];
            uint32_t nsc_rsp_len = PTX_NSC_RF_TEST_RSPLEN_PRBS;

            st = ptx_NSC_CmdProcessor_FetchMsg(nsc, nsc_rsp, &nsc_rsp_len, PTX_NSC_TIMEOUT_DEFAULT, 0);

            if (PTX_STATUS_SUCCESS == st)
            {
                if (PTX_NSC_RF_TEST_RSPLEN_PRBS == nsc_rsp_len)
                {
                    st = ptx_NSC_RfTest_ProcessRsp(nsc_rsp, nsc_rsp_len);
                }
                else
                {
                    st = PTX_STATUS_E_NSC_PROTOCOL;
                }
            }
        }
    }

    return st;
}

static ptx_Status_t ptx_NSC_RfTest_Carrier(ptx_NSC_t* nsc, ptx_NSC_RfTest_Params_Carrier_t *inParams)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Allocate buffer for the RF-Test command. */
    uint8_t nsc_buf[PTX_NSC_RF_TEST_BUFFER_SIZE_CARRIER];
    uint8_t *nsc_buf_pos = nsc_buf;

    *nsc_buf_pos = PTX_NSC_RF_TEST_CMD_OPCODE;
    nsc_buf_pos++;

    *nsc_buf_pos = (uint8_t) NSC_RfTest_SeqId_Carrier;
    nsc_buf_pos++;

    *nsc_buf_pos = (uint8_t) inParams->RffAmp;
    nsc_buf_pos++;

    st = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, nsc_buf, (uint32_t)(nsc_buf_pos - nsc_buf));
    if (PTX_STATUS_SUCCESS == st)
    {
        uint8_t nsc_rsp[PTX_NSC_RF_TEST_RSPLEN_CARRIER];
        uint32_t nsc_rsp_len = PTX_NSC_RF_TEST_RSPLEN_CARRIER;

        st = ptx_NSC_CmdProcessor_FetchMsg(nsc, nsc_rsp, &nsc_rsp_len, PTX_NSC_TIMEOUT_DEFAULT, 0);

        if (PTX_STATUS_SUCCESS == st)
        {
            if (PTX_NSC_RF_TEST_RSPLEN_CARRIER == nsc_rsp_len)
            {
                st = ptx_NSC_RfTest_ProcessRsp(nsc_rsp, nsc_rsp_len);
            }
            else
            {
                st = PTX_STATUS_E_NSC_PROTOCOL;
            }
        }
    }

    return st;
}

static ptx_Status_t ptx_NSC_RfTest_ProcessRsp(uint8_t *msg, uint32_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != msg) && (0 != msgLen))
    {
        status = PTX_STATUS_E_NSC_PROTOCOL;
        if ((PTX_NSC_CMD_PROCESSOR_RFTEST_RSP == msg[PTX_NSC_RF_TEST_RSP_ID_OFFSET]) && (0 == msg[PTX_NSC_RF_TEST_RSP_STATUS_OFFSET]))
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_SetTestMode(ptx_NSC_t* nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        if (ptx_NSC_State_Idle == nsc->State)
        {
            /* Start discovery to POLL with default polling configuration. */
            status = ptx_NSC_PollModeDiscovery(nsc);
        }
        else if (ptx_NSC_State_Poll == nsc->State)
        {
            /* Do nothing, we are ready. */
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            /* From any other state: deactivate to IDLE and InitiateDiscovery to POLL. */
            status = ptx_NSC_DeactivateToIdle(nsc);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Start discovery to POLL with default polling configuration. */
                status = ptx_NSC_PollModeDiscovery(nsc);
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_PollModeDiscovery(ptx_NSC_t* nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_NSC_RfDiscovery_Cfg_t nsc_cfg;
    memset(&nsc_cfg, 0, sizeof(ptx_NSC_RfDiscovery_Cfg_t));

    nsc_cfg.PollMode = NSC_RfDiscover_PollMode_ActivateRf;
    nsc_cfg.PollTypeAPollFrequency = 1u;
    nsc_cfg.PollTypeBPollFrequency = 1u;

    ptx_NSC_RfDiscover_Param_t discoverParams[PTX_NAX_MAX_NR_PARAMETERS];
    uint32_t discoverParamsSize = 0;
    memset(&discoverParams, 0x00, sizeof(ptx_NSC_RfDiscover_Param_t) * PTX_NAX_MAX_NR_PARAMETERS);

    status = ptx_NSC_GetRfDiscoveryParam(nsc, NSC_RfDiscover_Param_Type_Poll, &nsc_cfg, &discoverParams[discoverParamsSize]);
    discoverParamsSize++;
    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_NSC_GetRfDiscoveryParam(nsc, NSC_RfDiscover_Param_Type_PollA, &nsc_cfg, &discoverParams[discoverParamsSize]);
        discoverParamsSize++;
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_NSC_GetRfDiscoveryParam(nsc, NSC_RfDiscover_Param_Type_PollB, &nsc_cfg, &discoverParams[discoverParamsSize]);
        discoverParamsSize++;
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_NSC_RfDiscover(nsc, discoverParams, discoverParamsSize);
    }

    return status;
}

static ptx_Status_t ptx_NSC_DeactivateToIdle(ptx_NSC_t* nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_NSC_RfDeact_Param_t deactivation_parameters;
    memset(&deactivation_parameters, 0, sizeof(deactivation_parameters));
    deactivation_parameters.Type = NSC_RfDeact_Type_Generic;
    deactivation_parameters.State = NSC_RfDeact_State_Idle;

    uint8_t ntf_to_follow = 0;
    status = ptx_NSC_RfDeactivate(nsc, &deactivation_parameters, &ntf_to_follow);

    return status;
}
