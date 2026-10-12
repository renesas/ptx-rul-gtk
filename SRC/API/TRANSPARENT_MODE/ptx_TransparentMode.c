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
    Module      : Transparent Mode (TM)
    File        : ptx_TransparentMode.c

    Description : Transparent Mode (TM) implements low-level NFC access to the PTX2xxR/W hardware. This allows an
                  application to implement all sorts of low-level NFC frames in Reader mode to support custom
                  implementations or to implement proprietary protocols which are not supported by default in the RUL Stack.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_TransparentMode.h"
#include "ptx_NSC_Misc.h"
#include "ptx_RUL.h"
#include "ptx_NSC_RfSetParameter.h"
#include "ptx_Status.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
/*
 * Transparent Mode specific defines.
 *
 */

#define PTX_TRANSPARENT_MODE_BPRIME_ACTIVATED                      (uint8_t)1          /* BPrime Technology is Activated Flag. */
#define PTX_TRANSPARENT_MODE_BPRIME_DEACTIVATED                    (uint8_t)0          /* BPrime Technology is Deactivated Flag. */
#define PTX_TRANSPARENT_MODE_LPCD_FREQUENCY                        (uint8_t)3          /* Frequency of LPCD procedure in Hz. */
#define PTX_TRANSPARENT_MODE_ICLASS_SOF_ONLY_ACTIVATED             (uint8_t)1          /* SOF ONLY MODE is Activated Flag. */
#define PTX_TRANSPARENT_MODE_ICLASS_SOF_ONLY_DEACTIVATED           (uint8_t)0          /* SOF ONLY MODE is Deactivated Flag. */
#define PTX_TRANSPARENT_MODE_ICLASS_SKIP_CRC_BYTE_ACTIVATED        (uint8_t)1          /* Skip CRC Byte is Activated Flag. */
#define PTX_TRANSPARENT_MODE_ICLASS_SKIP_CRC_BYTE_DEACTIVATED      (uint8_t)0          /* Skip CRC Byte is Deactivated Flag. */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_TransparentMode_LoadRFParameters (ptx_TransparentMode_t *tmComp, ptx_TransparentMode_RFParams_t *rfParams, uint32_t timeoutMS);
static ptx_Status_t ptx_TransparentMode_CheckTechAndBitrateCompatibility (ptx_TransparentMode_RF_Tech_t techType, ptx_TransparentMode_RF_Bitrate_t txBitrateType, ptx_TransparentMode_RF_Bitrate_t rxBitrateType, uint8_t *activateBPrimeMode);
static ptx_Status_t ptx_TransparentMode_HandleBPrime (ptx_TransparentMode_t *tmComp, uint8_t activateBPrimeMode);
static ptx_Status_t ptx_TransparentMode_HandleSoFOnlyMode(ptx_TransparentMode_t *tmComp, uint8_t enableSoFOnlyMode);
static ptx_Status_t ptx_TransparentMode_HandleSkip1stCRCByteMode(ptx_TransparentMode_t *tmComp, uint8_t enableSkip1stCRCByteMode);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_TransparentMode_Open (ptx_TransparentMode_t *tmComp, ptx_TransparentMode_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != tmComp) && (NULL != initParams) && (NULL != initParams->Rul))
    {
        (void)memset(tmComp, 0, sizeof(ptx_TransparentMode_t));

        tmComp->Rul = initParams->Rul;
        tmComp->CompId = Comp_TransparentMode;
        tmComp->UseDefaultConfigForFieldOffState = initParams->UseDefaultConfigForFieldOffState;
        tmComp->SoFOnlyActive = PTX_TRANSPARENT_MODE_ICLASS_SOF_ONLY_DEACTIVATED;
        tmComp->SkipByteCRCActive = PTX_TRANSPARENT_MODE_ICLASS_SKIP_CRC_BYTE_DEACTIVATED;

        status = ptx_TransparentMode_ResetState(tmComp);
    }

    return status;
}

ptx_Status_t ptx_TransparentMode_Close (ptx_TransparentMode_t *tmComp)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tmComp, Comp_TransparentMode))
    {
        if (ptx_TransparentMode_State_RF_Off == tmComp->state)
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = ptx_TransparentMode_SetState(tmComp, ptx_TransparentMode_State_RF_Off);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            memset(tmComp, 0, sizeof(ptx_TransparentMode_t));
        }
    }

    return status;
}

ptx_Status_t ptx_TransparentMode_ResetState (ptx_TransparentMode_t *tmComp)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tmComp, Comp_TransparentMode))
    {
        tmComp->state = ptx_TransparentMode_State_RF_Off;
        tmComp->BPrimeTechnologyActive = PTX_TRANSPARENT_MODE_BPRIME_DEACTIVATED;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_TransparentMode_SetRFParameters (ptx_TransparentMode_t *tmComp, ptx_TransparentMode_RFParams_t *rfParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tmComp, Comp_TransparentMode) && (NULL != rfParams))
    {
        if (ptx_TransparentMode_State_RF_On == tmComp->state)
        {
            status = ptx_TransparentMode_LoadRFParameters(tmComp, rfParams, 0u);
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }
    }

    return status;
}

ptx_Status_t ptx_TransparentMode_SetState (ptx_TransparentMode_t *tmComp, ptx_TransparentMode_State_t state)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tmComp, Comp_TransparentMode))
    {
        switch (state)
        {
            case ptx_TransparentMode_State_RF_On:
            {
                ptx_RUL_DiscoverConfig_t disc_cfg = {0};
                disc_cfg.EnablePolling = 1u;
                disc_cfg.PollMode = ptx_RUL_PollMode_ContinuousWave;

                status = ptx_RUL_InitiateDiscovery(tmComp->Rul, &disc_cfg);

                if (PTX_STATUS_SUCCESS == status)
                {
                    tmComp->state = ptx_TransparentMode_State_RF_On;
                }

                break;
            }
            case ptx_TransparentMode_State_Lpcd_Poll:
            {
                ptx_RUL_DiscoverConfig_t disc_cfg = {0};
                disc_cfg.EnablePolling = 1u;
                disc_cfg.PollMode = ptx_RUL_PollMode_Lpcd;

                /** Transform LPCD Frequency to an interval in ms. */
                disc_cfg.IdleTime = (uint32_t)((1. / PTX_TRANSPARENT_MODE_LPCD_FREQUENCY) * 1e3);
                disc_cfg.StandbyCfg.EnableWuByHif = 1;
                disc_cfg.StandbyCfg.EnableWuByTimer = 1;

                status = ptx_RUL_InitiateDiscovery(tmComp->Rul, &disc_cfg);

                if (PTX_STATUS_SUCCESS == status)
                {
                    tmComp->state = ptx_TransparentMode_State_Lpcd_Poll;
                }
                break;
            }
            case ptx_TransparentMode_State_RF_Off:
            default:
            {
                status = ptx_TransparentMode_HandleBPrime(tmComp, PTX_TRANSPARENT_MODE_BPRIME_DEACTIVATED);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_TransparentMode_HandleSoFOnlyMode(tmComp, PTX_TRANSPARENT_MODE_ICLASS_SOF_ONLY_DEACTIVATED);
                }
                
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_TransparentMode_HandleSkip1stCRCByteMode(tmComp, PTX_TRANSPARENT_MODE_ICLASS_SKIP_CRC_BYTE_DEACTIVATED);
                }
                
                if ((tmComp->UseDefaultConfigForFieldOffState) && (PTX_STATUS_SUCCESS == status))
                {
                    ptx_TransparentMode_RFParams_t rf_params = {0};

                    /* Reset to default configuration. */
                    rf_params.Tech = ptx_TransparentMode_RF_Tech_A;
                    rf_params.TxRate = ptx_TransparentMode_RF_Bitrate_106;
                    rf_params.RxRate = ptx_TransparentMode_RF_Bitrate_106;
                    rf_params.ResLimit = 1u;
                    rf_params.RxBitOffset = 0u;
                    rf_params.Flags = (uint8_t)(PTX_TRANSPARENT_MODE_FLAGS_TX_PARITY | PTX_TRANSPARENT_MODE_FLAGS_TX_PARITY);

                    status = ptx_TransparentMode_LoadRFParameters(tmComp, &rf_params, 0u);
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    ptx_RUL_DeactivateParams_t deactivate_params = {0};
                    deactivate_params.Type = ptx_RUL_DeactivateType_Generic;
                    deactivate_params.State = ptx_RUL_DeactivateState_Idle;

                    status = ptx_RUL_Deactivate(tmComp->Rul, &deactivate_params);
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    tmComp->state = ptx_TransparentMode_State_RF_Off;
                }

                break;
            }

        }
    }

    return status;
}

ptx_Status_t ptx_TransparentMode_CheckLpcd (ptx_TransparentMode_t *tmComp, ptx_TransparentMode_Lpcd_State_t *cardDetected)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tmComp, Comp_TransparentMode) && (NULL != cardDetected))
    {
        *cardDetected = ptx_TransparentMode_Lpcd_State_Not_Detected;

        if (ptx_TransparentMode_State_Lpcd_Poll == tmComp->state)
        {
            ptx_RUL_Status_Params_t rulStatus;
            rulStatus.Type = ptx_RUL_StatusType_Lpcd;
            status = ptx_RUL_GetStatusInfo(tmComp->Rul, &rulStatus);

            if (PTX_STATUS_SUCCESS == status)
            {
                if (rulStatus.Info.Lpcd.LpcdNtfCounter != 0)
                {
                    *cardDetected = ptx_TransparentMode_Lpcd_State_Detected;
                    ptx_TransparentMode_SetState(tmComp, ptx_TransparentMode_State_RF_On);
                }
                else
                {
                    *cardDetected = ptx_TransparentMode_Lpcd_State_Not_Detected;
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }

    }

    return status;

}

ptx_Status_t ptx_TransparentMode_Exchange (ptx_TransparentMode_t *tmComp,
                                           ptx_TransparentMode_RFParams_t *rfParams,
                                           ptx_TransparentMode_ExchangeParams_t *exchangeParams)
{
    const uint32_t SAFETY_TIMEOUT = 500;
    const uint8_t COLLISION_BIT_POSITION_MASK = 0x07;        /* Mask to read Collision Bit Position from NSC_GET_STATUS_RSP. */
    const uint8_t COLLISION_BYTE_POSITION_MASK = 0x38;       /* Mask to read Collision Byte Position from NSC_GET_STATUS_RSP. */
    const uint8_t COLLISION_BYTE_POSITION_START_BIT = 3;     /* Starting Bit Position of the Collision Byte Position from NSC_GET_STATUS_RSP. */

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(tmComp, Comp_TransparentMode) && (NULL != exchangeParams))
    {
        if ((NULL != exchangeParams->TxData) &&
            (0 != exchangeParams->TxLength) &&
            (NULL != exchangeParams->RxData) &&
            (0 != exchangeParams->RxLength) &&
            (0 != exchangeParams->TimeoutMS))
        {
            if (ptx_TransparentMode_State_RF_On == tmComp->state)
            {
                status = ptx_TransparentMode_LoadRFParameters(tmComp, rfParams, exchangeParams->TimeoutMS);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NSC_DataSetNrResidualTxBits(&(tmComp->Rul->Nsc), exchangeParams->NrTxBits);
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        status = ptx_RUL_Exchange(tmComp->Rul, exchangeParams->TxData, exchangeParams->TxLength, exchangeParams->RxData,
                                                  &(exchangeParams->RxLength), (exchangeParams->TimeoutMS + SAFETY_TIMEOUT));

                        exchangeParams->NrResidualRxBits = tmComp->Rul->Nsc.ResidualRxBits;

                        if (PTX_STATUS_E_NSC_COLLISION == status)
                        {
                            ptx_NSC_ParamStatus_t nscStatus;
                            ptx_Status_t tmpStatus = ptx_NSC_GetStatus(&(tmComp->Rul->Nsc), &nscStatus);

                            if (PTX_STATUS_SUCCESS == tmpStatus)
                            {
                                exchangeParams->NrResidualRxBits = nscStatus.CollInfo & COLLISION_BIT_POSITION_MASK;
                                exchangeParams->RxLength = ((nscStatus.CollInfo & COLLISION_BYTE_POSITION_MASK) >> COLLISION_BYTE_POSITION_START_BIT) + (uint32_t)1;
                            }
                        }

                    }

                }
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_STATE;
            }
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK(s)
 * ####################################################################################################################
 */
static ptx_Status_t ptx_TransparentMode_LoadRFParameters (ptx_TransparentMode_t *tmComp, ptx_TransparentMode_RFParams_t *rfParams, uint32_t timeoutMS)
{
    /*
     * Internally uses a resolution of 128/fc per tick ~ 9,44us; 1 ms is roughly 106 ticks (~1.00059ms)
     * Note: The resolution is limited to 24-bits i.e. the max. value is roughly 158 s.
     */
    const uint32_t BASE_TIMEOUT_1_MS = (uint32_t)106;

    const uint8_t BINARY_1 = (uint8_t)0x01;
    const uint8_t BINARY_0 = (uint8_t)0x00;

    const uint8_t UINT8_FULL_BYTE_MASK = (uint8_t)0xFF;
    const uint32_t UINT32_THREE_BYTE_MASK = (uint32_t)0x00FFFFFF;
    const uint8_t STARTING_BIT_THIRD_BYTE = (uint8_t)16;
    const uint8_t STARTING_BIT_SECOND_BYTE = (uint8_t)8;
    const uint8_t STARTING_BIT_FIRST_BYTE = (uint8_t)0;

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t nr_nsc_rf_params = 0u;
    uint8_t activateBPrimeMode = PTX_TRANSPARENT_MODE_BPRIME_DEACTIVATED;
    ptx_NSC_RfSetParameter_Param_t nsc_rf_params[PTX_NSC_RF_SET_PARAM_MAXPARAMCOUNT];

    if (PTX_COMP_CHECK(tmComp, Comp_TransparentMode))
    {
        (void)memset(&nsc_rf_params, 0, PTX_NSC_RF_SET_PARAM_MAXPARAMCOUNT * sizeof(ptx_NSC_RfSetParameter_Param_t));

        if (NULL != rfParams)
        {
            ptx_TransparentMode_RF_Tech_t correctedRfTech = (rfParams->Tech == ptx_TransparentMode_RF_Tech_BPrime) ? ptx_TransparentMode_RF_Tech_B : rfParams->Tech;

            nsc_rf_params[nr_nsc_rf_params].Type = NSC_RfSetParameter_Param_Type_RfTech;
            nsc_rf_params[nr_nsc_rf_params].Value.RfTech = (ptx_NSC_RfSetParameter_Param_Rf_Tech_t)correctedRfTech;
            nr_nsc_rf_params++;

            status = ptx_TransparentMode_CheckTechAndBitrateCompatibility(rfParams->Tech, rfParams->TxRate, rfParams->RxRate, &activateBPrimeMode);

            nsc_rf_params[nr_nsc_rf_params].Type = NSC_RfSetParameter_Param_Type_TxBitRate;
            nsc_rf_params[nr_nsc_rf_params].Value.TxBitRate.BitrateValue = (uint8_t)rfParams->TxRate;
            nr_nsc_rf_params++;

            nsc_rf_params[nr_nsc_rf_params].Type = NSC_RfSetParameter_Param_Type_RxBitRate;
            nsc_rf_params[nr_nsc_rf_params].Value.RxBitRate.BitrateValue = (uint8_t)rfParams->RxRate;
            nr_nsc_rf_params++;

            nsc_rf_params[nr_nsc_rf_params].Type = NSC_RfSetParameter_Param_Type_Crc;
            nsc_rf_params[nr_nsc_rf_params].Value.Crc.TxCrc = (0 != (rfParams->Flags & PTX_TRANSPARENT_MODE_FLAGS_TX_CRC)) ? BINARY_1 : BINARY_0;
            nsc_rf_params[nr_nsc_rf_params].Value.Crc.RxCrc = (0 != (rfParams->Flags & PTX_TRANSPARENT_MODE_FLAGS_RX_CRC)) ? BINARY_1 : BINARY_0;
            nr_nsc_rf_params++;

            if (ptx_TransparentMode_RF_Tech_A == rfParams->Tech)
            {
                nsc_rf_params[nr_nsc_rf_params].Type = NSC_RfSetParameter_Param_Type_Parity;
                nsc_rf_params[nr_nsc_rf_params].Value.Parity.TxParity = (0 != (rfParams->Flags & PTX_TRANSPARENT_MODE_FLAGS_TX_PARITY)) ? BINARY_1 : BINARY_0;
                nsc_rf_params[nr_nsc_rf_params].Value.Parity.RxParity = (0 != (rfParams->Flags & PTX_TRANSPARENT_MODE_FLAGS_RX_PARITY)) ? BINARY_1 : BINARY_0;
                nr_nsc_rf_params++;

                nsc_rf_params[nr_nsc_rf_params].Type = NSC_RfSetParameter_Param_Type_RxBitOffset;
                nsc_rf_params[nr_nsc_rf_params].Value.RxBitOffset.RxBitOffsetValue = rfParams->RxBitOffset;
                nr_nsc_rf_params++;
            }

            nsc_rf_params[nr_nsc_rf_params].Type = NSC_RfSetParameter_Param_Type_MaxRspLim;
            nsc_rf_params[nr_nsc_rf_params].Value.MaxRspLim.MaxRspLimValue = rfParams->ResLimit;
            nr_nsc_rf_params++;
        }
        else
        {
            /* Special Case - If B-Prime Mode was used previously and no RF-Parameters are provided - keep B-Prime active */
            if (PTX_TRANSPARENT_MODE_BPRIME_DEACTIVATED != tmComp->BPrimeTechnologyActive)
            {
                activateBPrimeMode = PTX_TRANSPARENT_MODE_BPRIME_ACTIVATED;
                status = PTX_STATUS_SUCCESS;
            }
        }

        if (0 != timeoutMS)
        {
            uint32_t timeout_val = (BASE_TIMEOUT_1_MS * (timeoutMS + 1)) & UINT32_THREE_BYTE_MASK;

            nsc_rf_params[nr_nsc_rf_params].Type = NSC_RfSetParameter_Param_Type_Fwt;
            nsc_rf_params[nr_nsc_rf_params].Value.Fwt.FwtValue[PTX_NSC_RF_SET_PARAM_FWT_THIRD_BYTE_INDEX] = (uint8_t)((timeout_val >> STARTING_BIT_THIRD_BYTE) & UINT8_FULL_BYTE_MASK);
            nsc_rf_params[nr_nsc_rf_params].Value.Fwt.FwtValue[PTX_NSC_RF_SET_PARAM_FWT_SECOND_BYTE_INDEX] = (uint8_t)((timeout_val >> STARTING_BIT_SECOND_BYTE) & UINT8_FULL_BYTE_MASK);
            nsc_rf_params[nr_nsc_rf_params].Value.Fwt.FwtValue[PTX_NSC_RF_SET_PARAM_FWT_FIRST_BYTE_INDEX] = (uint8_t)((timeout_val >> STARTING_BIT_FIRST_BYTE) & UINT8_FULL_BYTE_MASK);
            nr_nsc_rf_params++;
        }

        if ((nr_nsc_rf_params <= PTX_NSC_RF_SET_PARAM_MAXPARAMCOUNT) && (PTX_STATUS_SUCCESS == status))
        {
            status = ptx_NSC_RfSetParameter(&(tmComp->Rul->Nsc), &nsc_rf_params[0], nr_nsc_rf_params);

            /* Handle B-Prime Mode */
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_TransparentMode_HandleBPrime(tmComp, activateBPrimeMode);
            }

            /* Handle SoF-Only Mode */
            if (PTX_STATUS_SUCCESS == status)
            {
                uint8_t enable_sof = 0;
                
                if (NULL != rfParams)
                {
                    enable_sof = (0 != (rfParams->Flags & PTX_TRANSPARENT_MODE_FLAGS_RX_SOF_ONLY)) ? PTX_TRANSPARENT_MODE_ICLASS_SOF_ONLY_ACTIVATED : PTX_TRANSPARENT_MODE_ICLASS_SOF_ONLY_DEACTIVATED;
                }
                else
                {
                    /* Keep current state if no params provided */
                    enable_sof = tmComp->SoFOnlyActive;
                }
                
                status = ptx_TransparentMode_HandleSoFOnlyMode(tmComp, enable_sof);
            }

            /* Handle Skip 1st Byte CRC Mode */
            if (PTX_STATUS_SUCCESS == status)
            {
                uint8_t enable_skip_crc = 0;
                
                if (NULL != rfParams)
                {
                    enable_skip_crc = (0 != (rfParams->Flags & PTX_TRANSPARENT_MODE_FLAGS_SKIP_1ST_BYTE_CRC)) ? PTX_TRANSPARENT_MODE_ICLASS_SKIP_CRC_BYTE_ACTIVATED : PTX_TRANSPARENT_MODE_ICLASS_SKIP_CRC_BYTE_DEACTIVATED;
                }
                else
                {
                    /* Keep current state if no params provided */
                    enable_skip_crc = tmComp->SkipByteCRCActive;
                }
                
                status = ptx_TransparentMode_HandleSkip1stCRCByteMode(tmComp, enable_skip_crc);
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }

    }

    return status;
}

static ptx_Status_t ptx_TransparentMode_CheckTechAndBitrateCompatibility(ptx_TransparentMode_RF_Tech_t techType, ptx_TransparentMode_RF_Bitrate_t txBitrateType, ptx_TransparentMode_RF_Bitrate_t rxBitrateType, uint8_t *activateBPrimeMode)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    switch (techType)
    {
        case ptx_TransparentMode_RF_Tech_A: /* Fall through. */
        case ptx_TransparentMode_RF_Tech_B:
            if ((txBitrateType == ptx_TransparentMode_RF_Bitrate_106 || txBitrateType == ptx_TransparentMode_RF_Bitrate_212 || txBitrateType == ptx_TransparentMode_RF_Bitrate_424 || txBitrateType == ptx_TransparentMode_RF_Bitrate_848) &&
                (rxBitrateType == ptx_TransparentMode_RF_Bitrate_106 || rxBitrateType == ptx_TransparentMode_RF_Bitrate_212 || rxBitrateType == ptx_TransparentMode_RF_Bitrate_424 || rxBitrateType == ptx_TransparentMode_RF_Bitrate_848))
            {
                status = PTX_STATUS_SUCCESS;
            }
            break;

        case ptx_TransparentMode_RF_Tech_F:
            if ((txBitrateType == ptx_TransparentMode_RF_Bitrate_212 || txBitrateType == ptx_TransparentMode_RF_Bitrate_424) &&
                (rxBitrateType == ptx_TransparentMode_RF_Bitrate_212 || rxBitrateType == ptx_TransparentMode_RF_Bitrate_424))
            {
                status = PTX_STATUS_SUCCESS;
            }
            break;

        case ptx_TransparentMode_RF_Tech_V:
            if ((txBitrateType == ptx_TransparentMode_RF_Bitrate_26) && (rxBitrateType == ptx_TransparentMode_RF_Bitrate_26))
            {
                status = PTX_STATUS_SUCCESS;
            }
            break;
        case ptx_TransparentMode_RF_Tech_BPrime:
            if (txBitrateType == ptx_TransparentMode_RF_Bitrate_106 && rxBitrateType == ptx_TransparentMode_RF_Bitrate_106 && NULL != activateBPrimeMode)
            {
                *activateBPrimeMode = PTX_TRANSPARENT_MODE_BPRIME_ACTIVATED;
                status = PTX_STATUS_SUCCESS;
            }
            break;
            
        default:
            break;
    }
    return status;
}

static ptx_Status_t ptx_TransparentMode_HandleBPrime(ptx_TransparentMode_t *tmComp, uint8_t activateBPrimeMode)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t reg_address = CL_RX_CONFIG1_REG;
    uint8_t reg_value = 0;

    if (PTX_COMP_CHECK(tmComp, Comp_TransparentMode))
    {
        /* Activate B-Prime Mode ? */
        if ((PTX_TRANSPARENT_MODE_BPRIME_DEACTIVATED != activateBPrimeMode) && (PTX_TRANSPARENT_MODE_BPRIME_DEACTIVATED == tmComp->BPrimeTechnologyActive))
        {
            status = ptx_NSC_Read(&(tmComp->Rul->Nsc), reg_address, 1u, &reg_value);

            if (PTX_STATUS_SUCCESS == status)
            {
                reg_value = (uint8_t)(reg_value | CL_RX_CONFIG1_REG_RX_B_PRIME_EN_MASK);

                status = ptx_NSC_Write(&(tmComp->Rul->Nsc), reg_address, 1u, &reg_value);

                if (PTX_STATUS_SUCCESS == status)
                {
                    tmComp->BPrimeTechnologyActive = PTX_TRANSPARENT_MODE_BPRIME_ACTIVATED;
                }
            }
        }
        /* Deactivate B-Prime Mode ? */
        else if ((PTX_TRANSPARENT_MODE_BPRIME_DEACTIVATED == activateBPrimeMode) && (PTX_TRANSPARENT_MODE_BPRIME_DEACTIVATED != tmComp->BPrimeTechnologyActive))
        {
            status = ptx_NSC_Read(&(tmComp->Rul->Nsc), reg_address, 1u, &reg_value);

            if (PTX_STATUS_SUCCESS == status)
            {
                reg_value = (uint8_t)(reg_value & CL_RX_CONFIG1_REG_RX_B_PRIME_EN_MASK_INV);

                status = ptx_NSC_Write(&(tmComp->Rul->Nsc), reg_address, 1u, &reg_value);

                if (PTX_STATUS_SUCCESS == status)
                {
                    tmComp->BPrimeTechnologyActive = PTX_TRANSPARENT_MODE_BPRIME_DEACTIVATED;
                }
            }
        }
        /* No BPrime Action needed. */
        else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

static ptx_Status_t ptx_TransparentMode_HandleSoFOnlyMode(ptx_TransparentMode_t *tmComp, uint8_t enableSoFOnlyMode)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint16_t reg_address = CL_RX_CONFIG1_REG;
    uint8_t reg_value = 0;

    if (PTX_COMP_CHECK(tmComp, Comp_TransparentMode))
    {
        /* Activate SoF-Only Mode */
        if ((0 != enableSoFOnlyMode) && (0 == tmComp->SoFOnlyActive))
        {
            status = ptx_NSC_Read(&(tmComp->Rul->Nsc), reg_address, 1u, &reg_value);
            
            if (PTX_STATUS_SUCCESS == status)
            {
                reg_value = (uint8_t)(reg_value | CL_RX_CONFIG1_REG_RX_SOF_ONLY_EN_MASK);

                status = ptx_NSC_Write(&(tmComp->Rul->Nsc), reg_address, 1u, &reg_value);

                if (PTX_STATUS_SUCCESS == status)
                {
                    tmComp->SoFOnlyActive = PTX_TRANSPARENT_MODE_ICLASS_SOF_ONLY_ACTIVATED;
                }
            }
        }
        /* Deactivate SoF-Only Mode */
        else if ((0 == enableSoFOnlyMode) && (0 != tmComp->SoFOnlyActive))
        {
            status = ptx_NSC_Read(&(tmComp->Rul->Nsc), reg_address, 1u, &reg_value);
            
            if (PTX_STATUS_SUCCESS == status)
            {
                reg_value = (uint8_t)(reg_value & CL_RX_CONFIG1_REG_RX_SOF_ONLY_EN_MASK_INV);

                status = ptx_NSC_Write(&(tmComp->Rul->Nsc), reg_address, 1u, &reg_value);

                if (PTX_STATUS_SUCCESS == status)
                {
                    tmComp->SoFOnlyActive = PTX_TRANSPARENT_MODE_ICLASS_SOF_ONLY_DEACTIVATED;
                }
            }
        }
        /* No SOF Action needed. */
        else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

static ptx_Status_t ptx_TransparentMode_HandleSkip1stCRCByteMode(ptx_TransparentMode_t *tmComp, uint8_t enableSkip1stCRCByteMode)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t reg_value = 0;

    if (PTX_COMP_CHECK(tmComp, Comp_TransparentMode))
    {
        /* Activate Skip 1st Byte CRC Mode */
        if ((0 != enableSkip1stCRCByteMode) && (0 == tmComp->SkipByteCRCActive))
        {
            /* Set RX register */
            status = ptx_NSC_Read(&(tmComp->Rul->Nsc), CL_RX_CONFIG1_REG, 1u, &reg_value);

            if (PTX_STATUS_SUCCESS == status)
            {
                reg_value = (uint8_t)(reg_value | CL_RX_CONFIG1_REG_RX_SKIP_1ST_CRC_BYTE_EN_MASK);

                status = ptx_NSC_Write(&(tmComp->Rul->Nsc), CL_RX_CONFIG1_REG, 1u, &reg_value);
            }

            /* Set TX register */
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_Read(&(tmComp->Rul->Nsc), CL_TX_CONFIG1_REG, 1u, &reg_value);
            
                if (PTX_STATUS_SUCCESS == status)
                {
                    reg_value = (uint8_t)(reg_value | CL_TX_CONFIG1_REG_RX_SKIP_1ST_CRC_BYTE_EN_MASK);

                    status = ptx_NSC_Write(&(tmComp->Rul->Nsc), CL_TX_CONFIG1_REG, 1u, &reg_value);
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                tmComp->SkipByteCRCActive = PTX_TRANSPARENT_MODE_ICLASS_SKIP_CRC_BYTE_ACTIVATED;
            }
        }
        /* Deactivate Skip 1st Byte CRC Mode */
        else if ((0 == enableSkip1stCRCByteMode) && (0 != tmComp->SkipByteCRCActive))
        {
            /* Clear RX register */
            status = ptx_NSC_Read(&(tmComp->Rul->Nsc), CL_RX_CONFIG1_REG, 1u, &reg_value);
            
            if (PTX_STATUS_SUCCESS == status)
            {
                reg_value = (uint8_t)(reg_value & CL_RX_CONFIG1_REG_RX_SKIP_1ST_CRC_BYTE_EN_MASK_INV);

                status = ptx_NSC_Write(&(tmComp->Rul->Nsc), CL_RX_CONFIG1_REG, 1u, &reg_value);
            }

            /* Clear TX register */
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_Read(&(tmComp->Rul->Nsc), CL_TX_CONFIG1_REG, 1u, &reg_value);
                if (PTX_STATUS_SUCCESS == status)
                {
                    reg_value = (uint8_t)(reg_value & CL_TX_CONFIG1_REG_RX_SKIP_1ST_CRC_BYTE_EN_MASK_INV);
                    status = ptx_NSC_Write(&(tmComp->Rul->Nsc), CL_TX_CONFIG1_REG, 1u, &reg_value);
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                tmComp->SkipByteCRCActive = PTX_TRANSPARENT_MODE_ICLASS_SKIP_CRC_BYTE_DEACTIVATED;
            }
        }else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }
    return status;
}