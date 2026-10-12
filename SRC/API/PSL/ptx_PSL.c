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
    Module      : PSL
    File        : ptx_PSL.c

    Description : Implementation of Product Support Library (PSL), to alleviate the integration of the PTX2xxR/W
                  into the final target application by providing a set of functions to identify and debug potential
                  performance issues during the design-in phase.
 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_PSL.h"
#include "ptx_NSC.h"
#include "ptx_NSC_Misc.h"
#include "ptx_NSC_RUL_RfConfig.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_PSL_QMEAS_DEFAULT_FREQ_SPAN (3000000U)                  /* Default frequency span of 3MHz. */
#define PTX_PSL_QMEAS_DEFAULT_FREQSTEP  (5000U)                     /* Default frequency step of 5kHz. */
#define PTX_PSL_QMEAS_DEFAULT_AVERAGING (AdcAveraging_08Samples)    /* Default ADC averaging of 8 samples. */

/*
 *  Default Q-Measurement configuration.
 */
const ptx_PSL_QMeas_Parameters_t DefaultQMeasParameters =
{
    .FreqSpan   = PTX_PSL_QMEAS_DEFAULT_FREQ_SPAN,
    .FreqStep   = PTX_PSL_QMEAS_DEFAULT_FREQSTEP,
    .AvgFactor  = PTX_PSL_QMEAS_DEFAULT_AVERAGING,
};


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_PSL_HandlePmuSignal(ptx_PSL_t *psl, ptx_PSL_AnaMuxTestSignals_t signal);
static ptx_Status_t ptx_PSL_HandleTxTestSignals(ptx_PSL_t *psl, ptx_PSL_AnaMuxTestSignals_t signal);
static ptx_Status_t ptx_PSL_HandleAMuxSelection(ptx_PSL_t *psl, ptx_PSL_AnaMuxTestSignals_t signal);
static ptx_Status_t ptx_PSL_DigSwapTestMux(ptx_PSL_t *psl, uint8_t enableSwap);
static ptx_Status_t ptx_PSL_DigSwapTb0(ptx_PSL_t *psl, ptx_PSL_TbSwapType_t swapType);
static ptx_Status_t ptx_PSL_DigSwapTb1(ptx_PSL_t *psl, ptx_PSL_TbSwapType_t swapType);
static ptx_Status_t ptx_PSL_DigSelSysClkPin(ptx_PSL_t *psl, uint8_t pinNbr);
static ptx_Status_t ptx_PSL_EnableDigPads8_1(ptx_PSL_t *psl);
static ptx_Status_t ptx_PSL_SetTMuxCtrl(ptx_PSL_t *psl);
static ptx_Status_t ptx_PSL_GetDefaultQMeasParams(ptx_PSL_t *psl, ptx_PSL_QMeas_Parameters_t *defParams);
static ptx_Status_t ptx_PSL_ResetDigPads8_1(ptx_PSL_t *psl);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_PSL_Init(ptx_PSL_t *psl, ptx_PSL_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != psl) && (NULL != initParams))
    {
        memset(psl, 0, sizeof(ptx_PSL_t));

        /* Expect valid and already initialized RUL component. Log may be NULL. */
        if (PTX_COMP_CHECK(initParams->Rul, Comp_RUL))
        {
            psl->Rul = initParams->Rul;
            psl->Log = initParams->Log;
            psl->CompId = Comp_Support;

            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_PSL_RfFieldMeasurement(ptx_PSL_t *psl, uint32_t targetFrequency, ptx_RUL_AdcAveraging_t averaging, uint32_t *rfAmp)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support) && (NULL != rfAmp) && (averaging < AdcAveraging_Max))
    {
        status = ptx_RUL_RfFieldMeasurement(psl->Rul, targetFrequency, averaging, rfAmp);
    }

    return status;
}

ptx_Status_t ptx_PSL_RssiMeasurement(ptx_PSL_t *psl, uint8_t cwPowerLevel, ptx_RUL_AdcAveraging_t averaging, uint32_t *result)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(psl, Comp_Support) && (NULL != result) && (AdcAveraging_Max > averaging) )
    {
        status = ptx_RUL_GetRSSIValue(psl->Rul, cwPowerLevel, averaging, result);
    }

    return status;
}

ptx_Status_t ptx_PSL_QMeasurement(ptx_PSL_t *psl, ptx_PSL_QMeas_Parameters_t *measParams, ptx_PSL_QMeas_Result_t *measResult)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    if (PTX_COMP_CHECK(psl, Comp_Support) && (NULL != measResult))
    {
        ptx_PSL_QMeas_Parameters_t measurement_params;
        uint32_t buffer_size = 0U;

        memset(&measurement_params, 0, sizeof(ptx_PSL_QMeas_Parameters_t));

        if (NULL == measParams)
        {
            status = ptx_PSL_GetDefaultQMeasParams(psl, &measurement_params);
        }
        else
        {
            (void) memcpy(&measurement_params, measParams, sizeof(ptx_PSL_QMeas_Parameters_t));
            status = PTX_STATUS_SUCCESS;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /*
             * Validate measurement parameters:
             * - FreqStep must not be 0 (division by zero / endless loop protection)
             * - AvgFactor must be valid
             * - FreqSpan must be within supported range
             */
            if ((0U == measurement_params.FreqSpan) ||
                (0U == measurement_params.FreqStep) ||
                (measurement_params.FreqSpan < measurement_params.FreqStep) ||
                (measurement_params.AvgFactor >= AdcAveraging_Max) ||
                (measurement_params.FreqSpan > PTX_PSL_QMEAS_MAX_FREQ_SPAN))
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            buffer_size = (measurement_params.FreqSpan / measurement_params.FreqStep) + 1U;
            /*
             * Check if we have enough memory for the given measurement parameters.
             * Large FreqSpan values and small FreqStep values result in high memory usage.
             * We allow a maximum of PTX_PSL_QMEAS_BUFFERSIZE samples.
             */
            if ((0U == buffer_size) || (buffer_size > PTX_PSL_QMEAS_BUFFERSIZE))
            {
                status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Prepare the buffer for the RF amplitude measurement results. */
            uint32_t rf_amplitudes[PTX_PSL_QMEAS_BUFFERSIZE];
            (void) memset(rf_amplitudes, 0x00, PTX_PSL_QMEAS_BUFFERSIZE * sizeof(uint32_t));

            /* Start sweeping through the frequencies and measure the RF field amplitude at each step. */
            uint32_t idx = 0;
            uint32_t max_amplitude = 0;
            uint32_t max_amplitude_idx = 0;
            uint32_t start_freq = PTX_PSL_QMEAS_CENTER_FREQ - (measurement_params.FreqSpan / 2u);
            uint32_t stop_freq  = PTX_PSL_QMEAS_CENTER_FREQ + (measurement_params.FreqSpan / 2u);

            for (uint32_t cur_freq = start_freq;
                 (cur_freq < stop_freq) && (PTX_STATUS_SUCCESS == status) && (idx < buffer_size);
                 cur_freq += measurement_params.FreqStep)
            {
                /* Get the amplitude at current frequency */
                status = ptx_PSL_RfFieldMeasurement(psl, cur_freq, measurement_params.AvgFactor, &rf_amplitudes[idx]);
                if (rf_amplitudes[idx] > max_amplitude)
                {
                    max_amplitude = rf_amplitudes[idx];
                    max_amplitude_idx = idx;
                }
                idx++;
            }

            if ((PTX_STATUS_SUCCESS == status) && (idx > 0U))
            {
                /* We got all the results. There might be more than one sample with the same max. amplitude value. Check this and choose the middle one. */
                uint32_t amp_idx;
                for (amp_idx = max_amplitude_idx; amp_idx < idx; amp_idx++)
                {
                    if (rf_amplitudes[amp_idx] != max_amplitude)
                    {
                        break;
                    }
                }

                if ((amp_idx - max_amplitude_idx) >= 2U)
                {
                    amp_idx = ((amp_idx - max_amplitude_idx) / 2U) + max_amplitude_idx;
                }

                /* Calculate -3dB value (0.708 * MaxAmplitude). */
                uint32_t corner_freq_ampvalue = (uint32_t)(((uint64_t)max_amplitude * 708U) / PTX_PSL_AMP_MEAS_FACTOR);

                /*
                 * For -3dB amplitude values find the matching frequencies.
                 * First, lower frequency. Then, upper.
                 */
                uint32_t lower_corner_freq_idx = 0;
                uint32_t upper_corner_freq_idx = 0;

                /* Start from the left and go to the resonant frequency. */
                for (uint32_t i = 0; i < amp_idx; i++)
                {
                    if (rf_amplitudes[i] > corner_freq_ampvalue)
                    {
                        break;
                    }
                    lower_corner_freq_idx = i;
                }

                /* Start from the resonant frequency and go to the right lobe. */
                for (uint32_t i = amp_idx; i < idx; i++)
                {
                    if (rf_amplitudes[i] < corner_freq_ampvalue)
                    {
                        break;
                    }
                    upper_corner_freq_idx = i;
                }

                /* Calculate the measurement results. */
                measResult->ResonanceFreq    = start_freq + (measurement_params.FreqStep * max_amplitude_idx);
                measResult->LowerCornerFreq = start_freq + (measurement_params.FreqStep * lower_corner_freq_idx);
                measResult->UpperCornerFreq = start_freq + (measurement_params.FreqStep * upper_corner_freq_idx);

                if (measResult->UpperCornerFreq != measResult->LowerCornerFreq)
                {
                    measResult->QFactor = (measResult->ResonanceFreq * 100U) / (measResult->UpperCornerFreq - measResult->LowerCornerFreq);
                }
                else
                {
                    /* Error during the measurement: corner frequencies cannot have the same values */
                    status = PTX_STATUS_E_GEN_INTERNAL;
                }
            }
            else if (PTX_STATUS_SUCCESS == status)
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
            }

            /* Deactivate to Idle in any case, but do not overwrite the error status */
            ptx_Status_t temp_stat = ptx_NSC_RfTestStop(&(psl->Rul->Nsc));
            status = (PTX_STATUS_SUCCESS == status) ? temp_stat : status;
        }
    }
    return status;
}

ptx_Status_t ptx_PSL_SetDdpcParameters(ptx_PSL_t *psl, ptx_PSL_Ddpc_Params_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support) && (NULL != params) && (params->LowThreshold < params->HighThreshold) )
    {
        ptx_NSC_t *nsc = &(psl->Rul->Nsc);
        /*
         * The call to ptx_NSC_RfConfig_SetParam() will update the NSC RF config datastructures stored in the RAM.
         */
        (void) ptx_NSC_RfConfig_SetParam(nsc, ParamId_PollPwrLvl0, params->PowerLevel0);
        (void) ptx_NSC_RfConfig_SetParam(nsc, ParamId_PollPwrLvl1, params->PowerLevel1);
        (void) ptx_NSC_RfConfig_SetParam(nsc, ParamId_PollDdpcAvg, params->AvgFactor);
        /* The threshold parameter is a 9 Bit value split into  "_Thr0" and "_Thr1". */
        (void) ptx_NSC_RfConfig_SetParam(nsc, ParamId_PollPwrLvl0_Thr1, (uint8_t) ((params->LowThreshold >> 8) & 0x01));
        (void) ptx_NSC_RfConfig_SetParam(nsc, ParamId_PollPwrLvl0_Thr0, (uint8_t) (params->LowThreshold & 0xFF));
        /* The threshold parameter is a 9 Bit value split into  "_Thr0" and "_Thr1". */
        (void) ptx_NSC_RfConfig_SetParam(nsc, ParamId_PollPwrLvl1_Thr1, (uint8_t) ((params->HighThreshold >> 8) & 0x01));
        (void) ptx_NSC_RfConfig_SetParam(nsc, ParamId_PollPwrLvl1_Thr0, (uint8_t) (params->HighThreshold & 0xFF));
        /*
         * Write the updated RF config section to the device.
         */
        ptx_RUL_HWConfig_t poll_config;
        poll_config.ID      = ptx_RUL_RF_CFG_POLL;
        poll_config.Data.Ptr = ptx_NSC_RfConfig_Poll;
        poll_config.Len     = ptx_NSC_RfConfig_Poll_len;

        status = ptx_RUL_UpdateHardwareConfiguration(psl->Rul, 1u, &poll_config);
    }

    return status;
}

ptx_Status_t ptx_PSL_RunDdpcEvaluation(ptx_PSL_t *psl, ptx_PSL_Ddpc_Measurement_Params_t *measParams, ptx_PSL_Ddpc_Results_t *results)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(psl, Comp_Support) && (NULL != measParams) && (NULL != results) )
    {
        /* Run the RSSI measurement with the lower power level. */
        status = ptx_PSL_RssiMeasurement(psl, measParams->PowerLevel0, measParams->AvgFactor, &results->RssiPowerLevel0);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Run the RSSI measurement with the higher power level. */
            status = ptx_PSL_RssiMeasurement(psl, measParams->PowerLevel1, measParams->AvgFactor, &results->RssiPowerLevel1);
        }
    }

    return status;
}

ptx_Status_t ptx_PSL_RunDdpcTest(ptx_PSL_t *psl, uint8_t measIntervalMs)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        ptx_NSC_RfTest_InParams_t rf_test_params;
        (void) memset(&rf_test_params, 0, sizeof(ptx_NSC_RfTest_InParams_t));

        rf_test_params.SeqId = NSC_RfTest_SeqId_DDPC;
        rf_test_params.Params.DDPCParams.DDPC_Period = measIntervalMs;

        status = ptx_NSC_RfTest(&(psl->Rul->Nsc), &rf_test_params, NULL);
    }

    return status;
}

ptx_Status_t ptx_PSL_GetDdpcStatus(ptx_PSL_t *psl, ptx_PSL_Ddpc_Status_t *ddpcStatus)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support) && (NULL != ddpcStatus))
    {
        ptx_NSC_ParamStatus_t status_information;
        memset(&status_information, 0, sizeof(ptx_NSC_ParamStatus_t));

        status = ptx_NSC_GetStatus(&(psl->Rul->Nsc), &status_information);

        if (PTX_STATUS_SUCCESS == status)
        {
            ddpcStatus->PowerLevel  = status_information.DdpcPl;
            ddpcStatus->Rssi        = status_information.Rssi;
        }
    }

    return status;
}

ptx_Status_t ptx_PSL_StopDdpcTest(ptx_PSL_t *psl)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        status = ptx_NSC_RfTestStop(&(psl->Rul->Nsc));
    }

    return status;
}

ptx_Status_t ptx_PSL_WriteRegister(ptx_PSL_t *psl, uint16_t address, uint8_t value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        status = ptx_NSC_Write(&(psl->Rul->Nsc), address, 1u, &value);
    }

    return status;
}

ptx_Status_t ptx_PSL_ReadRegister(ptx_PSL_t *psl, uint16_t address, uint8_t *value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support) && (NULL != value))
    {
        status = ptx_NSC_Read(&(psl->Rul->Nsc), address, 1u, value);
    }

    return status;
}

ptx_Status_t ptx_PSL_ModifyRegister(ptx_PSL_t *psl, uint16_t address, uint8_t bitValue, uint8_t mask)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        uint8_t reg_value = 0x00;
        status = ptx_PSL_ReadRegister(psl, address, &reg_value);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Find bit offsets in mask (mask is assumed to be valid). */
            uint8_t tmp_mask = 0x01;
            /* Offset tracking of first (most significant) non-zero bit within mask. */
            uint8_t offset_last_bit = 0;
            while (0 == (mask & (tmp_mask << offset_last_bit)) && (8 > offset_last_bit))
            {
                ++offset_last_bit;
            }

            /* Clear bit slice to be manipulated. */
            reg_value = (uint8_t)(reg_value & (~mask & 0xFF));

            /* Bit value should store the bits to be set in correct order starting from its own lowest bit. */
            reg_value = (uint8_t)(reg_value | ((bitValue & 0xFF) << (offset_last_bit)));

            status = ptx_PSL_WriteRegister(psl, address, reg_value);

        }
    }

    return status;
}

ptx_Status_t ptx_PSL_ResetAllRegs(ptx_PSL_t *psl)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        status = ptx_PSL_WriteRegister(psl, PAD_DIG1_IO1_REG, PAD_DIG1_IO1_REG_RST);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG2_IO1_REG, PAD_DIG2_IO1_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG3_IO1_REG, PAD_DIG3_IO1_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG4_IO2_REG, PAD_DIG4_IO2_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG5_IO2_REG, PAD_DIG5_IO2_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG6_IO2_REG, PAD_DIG6_IO2_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG7_IO2_REG, PAD_DIG7_IO2_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG8_IO2_REG, PAD_DIG8_IO2_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_TEST_BUF_REG, ANA_TEST_BUF_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_TEST_SEL0_REG, ANA_TEST_SEL0_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_TEST_SEL1_REG, ANA_TEST_SEL1_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_TEST_SEL2_REG, ANA_TEST_SEL2_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_TEST_SEL3_REG, ANA_TEST_SEL3_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_RX_ADC_TEST_REG, ANA_RX_ADC_TEST_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_CLOCK_TEST_REG, ANA_CLOCK_TEST_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_RX_TEST_REG, ANA_RX_TEST_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_RX_TMUXA_REG, ANA_RX_TMUXA_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_RX_TMUXB_REG, ANA_RX_TMUXB_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_SUPPLY_TEST_REG, ANA_SUPPLY_TEST_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_PMU_TEST_REG, ANA_PMU_TEST_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_SWP_TEST1_REG, ANA_SWP_TEST1_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, TSENS_DECIM_REG, TSENS_DECIM_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, SYS_DFT_TB_CONTROL_REG, SYS_DFT_TB_CONTROL_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, SYS_DFT_TB0_SEL_REG, SYS_DFT_TB0_SEL_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, SYS_DFT_TB1_SEL_REG, SYS_DFT_TB1_SEL_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_TMUX_CONTROL0_REG, PAD_TMUX_CONTROL0_REG_RST);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_TEST_SHORT_REG, ANA_TEST_SHORT_REG_RST);
        }
    }

    return status;
}

ptx_Status_t ptx_PSL_AnaMuxSelectSignal(ptx_PSL_t *psl, ptx_PSL_AnaTestPin_t testPinNr, ptx_PSL_AnaMuxTestSignals_t signal)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        uint8_t amux_channel = (uint8_t)((signal >> 8u) & 0xFF);

        switch (testPinNr)
        {
            case AnaTestPin_Atst0Buff:
                status = ptx_PSL_WriteRegister(psl, PAD_DIG4_IO2_REG, PAD_DIG4_IO2_REG_RST);
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Disable bypass */
                    status = ptx_PSL_ModifyRegister(psl, ANA_TEST_BUF_REG, 0, 0x01);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Enable buffer */
                    status = ptx_PSL_ModifyRegister(psl, ANA_TEST_BUF_REG, 1u, 0x10);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_WriteRegister(psl, ANA_TEST_SEL0_REG, amux_channel);
                }
                break;
            case AnaTestPin_Atst1Buff:
                status = ptx_PSL_WriteRegister(psl, PAD_DIG6_IO2_REG, PAD_DIG6_IO2_REG_RST);
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Disable bypass */
                    status = ptx_PSL_ModifyRegister(psl, ANA_TEST_BUF_REG, 0, 0x02);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Enable buffer */
                    status = ptx_PSL_ModifyRegister(psl, ANA_TEST_BUF_REG, 1u, 0x20);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_WriteRegister(psl, ANA_TEST_SEL1_REG, amux_channel);
                }
                break;
            case AnaTestPin_Atst0:
                status = ptx_PSL_WriteRegister(psl, PAD_DIG4_IO2_REG, PAD_DIG4_IO2_REG_RST);
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Disable buffer */
                    status = ptx_PSL_ModifyRegister(psl, ANA_TEST_BUF_REG, 0, 0x10);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Enable bypass */
                    status = ptx_PSL_ModifyRegister(psl, ANA_TEST_BUF_REG, 1u, 0x01);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_WriteRegister(psl, ANA_TEST_SEL0_REG, amux_channel);
                }
                break;
            case AnaTestPin_Atst1:
                status = ptx_PSL_WriteRegister(psl, PAD_DIG6_IO2_REG, PAD_DIG6_IO2_REG_RST);
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Disable buffer */
                    status = ptx_PSL_ModifyRegister(psl, ANA_TEST_BUF_REG, 0, 0x20);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Enable bypass */
                    status = ptx_PSL_ModifyRegister(psl, ANA_TEST_BUF_REG, 1u, 0x02);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_WriteRegister(psl, ANA_TEST_SEL1_REG, amux_channel);
                }
                break;
            case AnaTestPin_Atst2:
                status = ptx_PSL_WriteRegister(psl, PAD_DIG7_IO2_REG, PAD_DIG7_IO2_REG_RST);
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Enable bypass (no buffer implemented) */
                    status = ptx_PSL_ModifyRegister(psl, ANA_TEST_BUF_REG, 1u, 0x04);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_WriteRegister(psl, ANA_TEST_SEL2_REG, amux_channel);
                }
                break;
            case AnaTestPin_Atst3:
                status = ptx_PSL_WriteRegister(psl, PAD_DIG8_IO2_REG, PAD_DIG8_IO2_REG_RST);
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Enable bypass (no buffer implemented) */
                    status = ptx_PSL_ModifyRegister(psl, ANA_TEST_BUF_REG, 1u, 0x08);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_WriteRegister(psl, ANA_TEST_SEL3_REG, amux_channel);
                }
                break;

            default:
                /* Error already set. */
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Enable test signals (if an enable switch is implemented) last step */
            status = ptx_PSL_HandleAMuxSelection(psl, signal);
        }
    }

    return status;
}

ptx_Status_t ptx_PSL_ResetShortOutputSignal(ptx_PSL_t *psl)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        status = ptx_PSL_WriteRegister(psl, ANA_TEST_SHORT_REG, ANA_TEST_SHORT_REG_RST);
    }

    return status;
}

ptx_Status_t ptx_PSL_ShortOutputSignal(ptx_PSL_t *psl, ptx_PSL_AnaTestPin_t pin1, ptx_PSL_AnaTestPin_t pin2)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        uint8_t bit_mask = 0;
        switch (pin1)
        {
            case AnaTestPin_Atst0:
            /*  Fall through. */
            case AnaTestPin_Atst0Buff:
                switch (pin2)
                {
                    case AnaTestPin_Atst1:
                    /* Fall through. */
                    case AnaTestPin_Atst1Buff:
                        bit_mask = 0x01;
                        status = PTX_STATUS_SUCCESS;
                        break;
                    case AnaTestPin_Atst2:
                        bit_mask = 0x02;
                        status = PTX_STATUS_SUCCESS;
                        break;
                    case AnaTestPin_Atst3:
                        bit_mask = 0x04;
                        status = PTX_STATUS_SUCCESS;
                        break;

                    case AnaTestPin_Atst0:
                    case AnaTestPin_Atst0Buff:
                    /* Fall through. */
                    default:
                        /* Error already set. */
                        break;
                }
                break;
            case AnaTestPin_Atst1:
            /*  Fall through. */
            case AnaTestPin_Atst1Buff:
                switch (pin2)
                {
                    case AnaTestPin_Atst0:
                    /* Fall through. */
                    case AnaTestPin_Atst0Buff:
                        bit_mask = 0x01;
                        status = PTX_STATUS_SUCCESS;
                        break;
                    case AnaTestPin_Atst2:
                        bit_mask = 0x08;
                        status = PTX_STATUS_SUCCESS;
                        break;
                    case AnaTestPin_Atst3:
                        bit_mask = 0x10;
                        status = PTX_STATUS_SUCCESS;
                        break;

                    case AnaTestPin_Atst1:
                    case AnaTestPin_Atst1Buff:
                    /* Fall through. */
                    default:
                        /* Error already set. */
                        break;
                }
                break;
            case AnaTestPin_Atst2:
                switch (pin2)
                {
                    case AnaTestPin_Atst0:
                    /* Fall through. */
                    case AnaTestPin_Atst0Buff:
                        bit_mask = 0x02;
                        status = PTX_STATUS_SUCCESS;
                        break;
                    case AnaTestPin_Atst1:
                    /* Fall through. */
                    case AnaTestPin_Atst1Buff:
                        bit_mask = 0x08;
                        status = PTX_STATUS_SUCCESS;
                        break;
                    case AnaTestPin_Atst3:
                        bit_mask = 0x20;
                        status = PTX_STATUS_SUCCESS;
                        break;

                    case AnaTestPin_Atst2:
                    /* Fall through. */
                    default:
                        /* Error already set. */
                        break;
                }
                break;
            case AnaTestPin_Atst3:
                switch (pin2)
                {
                    case AnaTestPin_Atst0:
                    /* Fall through. */
                    case AnaTestPin_Atst0Buff:
                        bit_mask = 0x04;
                        status = PTX_STATUS_SUCCESS;
                        break;
                    case AnaTestPin_Atst1:
                    /* Fall through. */
                    case AnaTestPin_Atst1Buff:
                        bit_mask = 0x10;
                        status = PTX_STATUS_SUCCESS;
                        break;
                    case AnaTestPin_Atst2:
                        bit_mask = 0x20;
                        status = PTX_STATUS_SUCCESS;
                        break;

                    case AnaTestPin_Atst3:
                    /* Fall through. */
                    default:
                        /* Error already set. */
                        break;
                }
                break;
            default:
                /* Error already set. */
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ModifyRegister(psl, ANA_TEST_SHORT_REG, 1u, bit_mask);
        }
    }

    return status;
}

ptx_Status_t ptx_PSL_DigMuxConfigure(ptx_PSL_t *psl, uint8_t swapTestMux, ptx_PSL_TbSwapType_t tb1SwapCfg, ptx_PSL_TbSwapType_t tb0SwapCfg, uint8_t sysClkPinNbr)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        status = ptx_PSL_DigSwapTestMux(psl, swapTestMux);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_DigSwapTb1(psl, tb1SwapCfg);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_DigSwapTb0(psl, tb0SwapCfg);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_DigSelSysClkPin(psl, sysClkPinNbr);
        }
    }

    return status;
}

ptx_Status_t ptx_PSL_DigMuxReset(ptx_PSL_t *psl)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        status = ptx_PSL_ResetDigPads8_1(psl);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, SYS_DFT_TB_CONTROL_REG, SYS_DFT_TB_CONTROL_REG_RST);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, SYS_DFT_TB0_SEL_REG, SYS_DFT_TB0_SEL_REG_RST);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, SYS_DFT_TB1_SEL_REG, SYS_DFT_TB1_SEL_REG_RST);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_TMUX_CONTROL0_REG, PAD_TMUX_CONTROL0_REG_RST);
        }
    }

    return status;
}

ptx_Status_t ptx_PSL_DigMuxSelectSignal(ptx_PSL_t *psl, ptx_PSL_TB_t signal)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {

        switch (signal.Type)
        {
            case TbTypes_Tb0:
                if (Tb0_AnaDbg1 == signal.Signal.SigTb0)
                {
                    status = ptx_PSL_WriteRegister(psl, ANA_CLOCK_TEST_REG, 1 << ANA_CLOCK_TEST_REG_PFD_TST_SEL_POS);
                }
                status = ptx_PSL_WriteRegister(psl, SYS_DFT_TB0_SEL_REG, (uint8_t)signal.Signal.SigTb0);
                break;
            case TbTypes_Tb1:
                status = ptx_PSL_WriteRegister(psl, SYS_DFT_TB1_SEL_REG, (uint8_t)signal.Signal.SigTb1);
                break;
            default:
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_EnableDigPads8_1(psl);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_SetTMuxCtrl(psl);
        }
    }

    return status;
}

ptx_Status_t ptx_PSL_DeInit(ptx_PSL_t *psl)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        status = ptx_PSL_ResetAllRegs(psl);

        if (PTX_STATUS_SUCCESS == status)
        {
            memset(psl, 0, sizeof(ptx_PSL_t));
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_PSL_HandlePmuSignal(ptx_PSL_t *psl, ptx_PSL_AnaMuxTestSignals_t signal)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        uint8_t bit_mask = 0x00;
        uint8_t sub_block_signal = (uint8_t) (signal & 0xFF);

        switch (signal)
        {
            case TestSignal_PmuTestOut0LL:
            case TestSignal_PmuTestOut0VsFromLpBias:
            case TestSignal_PmuTestOut0LL14:
            case TestSignal_PmuTestOut0LL15:
                bit_mask = ANA_PMU_TEST_REG_PMU_TEST_SEL0_MASK;
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_PmuTestOut0IbRef0u4Test:
            case TestSignal_PmuTestOut0Ib0u1TuneTest:
            case TestSignal_PmuTestOut0Ib1uRefTest:
            case TestSignal_PmuTestOut0Ib1uTuneTest:
            case TestSignal_PmuTestOut0Ib20uTuneTest:
            case TestSignal_PmuTestOut0VrefTest:
            case TestSignal_PmuTestOut0V2iHpVrefTest:
            case TestSignal_PmuTestOut0V2iHpVrefTuneTest:
            case TestSignal_PmuTestOut0V2iLpVrefTest:
            case TestSignal_PmuTestOut0V2iLpVrefTuneTest:
                /* Enables all test signals. */
                status = ptx_PSL_ModifyRegister(psl, ANA_SUPPLY_TEST_REG, 1u, ANA_SUPPLY_TEST_REG_BIAS_TEST_EN_MASK);
                bit_mask = ANA_PMU_TEST_REG_PMU_TEST_SEL0_MASK;
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_PmuTestOut0Ldo2v5OutTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_SUPPLY_TEST_REG, 1u, ANA_SUPPLY_TEST_REG_LDO2V5_TEST_EN_MASK);
                bit_mask = ANA_PMU_TEST_REG_PMU_TEST_SEL0_MASK;
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_PmuTestOut0HpLdoVoutTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_SUPPLY_TEST_REG, 1u, ANA_SUPPLY_TEST_REG_HPLDO_TEST_EN_MASK);
                bit_mask = ANA_PMU_TEST_REG_PMU_TEST_SEL0_MASK;
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_PmuTestOut1LL:
            case TestSignal_PmuTestOut1VsFromLpBias:
            case TestSignal_PmuTestOut1LL14:
            case TestSignal_PmuTestOut1LL15:
                bit_mask = ANA_PMU_TEST_REG_PMU_TEST_SEL1_MASK;
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_PmuTestOut1IbRef0u4Test:
            case TestSignal_PmuTestOut1Ib0u1TuneTest:
            case TestSignal_PmuTestOut1Ib1uRefTest:
            case TestSignal_PmuTestOut1Ib1uTuneTest:
            case TestSignal_PmuTestOut1Ib20uTuneTest:
            case TestSignal_PmuTestOut1VrefTest:
            case TestSignal_PmuTestOut1V2iHpVrefTest:
            case TestSignal_PmuTestOut1V2iHpVrefTuneTest:
            case TestSignal_PmuTestOut1V2iLpVrefTest:
            case TestSignal_PmuTestOut1V2iLpVrefTuneTest:
                /* Enables all test signals. */
                status = ptx_PSL_ModifyRegister(psl, ANA_SUPPLY_TEST_REG, 1u, ANA_SUPPLY_TEST_REG_BIAS_TEST_EN_MASK);
                bit_mask = ANA_PMU_TEST_REG_PMU_TEST_SEL1_MASK;
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_PmuTestOut1Ldo2v5OutTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_SUPPLY_TEST_REG, 1u, ANA_SUPPLY_TEST_REG_LDO2V5_TEST_EN_MASK);
                bit_mask = ANA_PMU_TEST_REG_PMU_TEST_SEL1_MASK;
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_PmuTestOut1HpLdoVoutTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_SUPPLY_TEST_REG, 1u, ANA_SUPPLY_TEST_REG_HPLDO_TEST_EN_MASK);
                bit_mask = ANA_PMU_TEST_REG_PMU_TEST_SEL1_MASK;
                status = PTX_STATUS_SUCCESS;
                break;
            default:
                /* Error already assigned. */
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Modify subblock channel part. */
            status = ptx_PSL_ModifyRegister(psl, ANA_PMU_TEST_REG, sub_block_signal, bit_mask);
        }
    }

    return status;
}

static ptx_Status_t ptx_PSL_HandleTxTestSignals(ptx_PSL_t *psl, ptx_PSL_AnaMuxTestSignals_t signal)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        uint8_t sub_block_signal = (uint8_t) (signal & 0xFF);
        uint8_t registerValue = 0;
        switch (signal)
        {
            case TestSignal_TxTestHiZ:
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_TxTest05VsPmos:
            case TestSignal_TxTest05VdNmos:
                status = ptx_PSL_ReadRegister(psl, ANA_TX_PA_REG, &registerValue);
                if ((PTX_STATUS_SUCCESS == status) && (ANA_TX_PA_REG_TX_PA_BIAS_EN_MASK != (registerValue & ANA_TX_PA_REG_TX_PA_BIAS_EN_MASK)))
                {
                    status = PTX_STATUS_E_PSL_ENABLESIGNALMISSING;
                }
                break;
            case TestSignal_TxTestAmpOutIsense:
                status = ptx_PSL_ReadRegister(psl, ANA_TX_PA_REG, &registerValue);
                if ((PTX_STATUS_SUCCESS == status) && (ANA_TX_PA_REG_TX_PA_BIAS_EN_MASK != (registerValue & ANA_TX_PA_REG_TX_PA_BIAS_EN_MASK)))
                {
                    status = PTX_STATUS_E_PSL_ENABLESIGNALMISSING;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_ModifyRegister(psl, ANA_TX_PA_REG, 1u, ANA_TX_PA_REG_TX_PA_BIAS_EN_MASK);
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_ReadRegister(psl, ANA_TX_CURLIM_CTRL_REG, &registerValue);
                }

                if ((PTX_STATUS_SUCCESS == status) && (ANA_TX_CURLIM_CTRL_REG_TX_CURLIM_EN_MASK != (registerValue & ANA_TX_CURLIM_CTRL_REG_TX_CURLIM_EN_MASK)))
                {
                    status = PTX_STATUS_E_PSL_ENABLESIGNALMISSING;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_ModifyRegister(psl, ANA_TX_PA_REG, 1u, ANA_TX_PA_REG_TX_PA_BIAS_EN_MASK);
                }
                break;
            default:
                /* Error already assigned. */
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ModifyRegister(psl, ANA_TX_TEST_REG, sub_block_signal, ANA_TX_TEST_REG_TX_TEST_SEL_MASK);
        }
    }

    return status;
}

static ptx_Status_t ptx_PSL_HandleAMuxSelection(ptx_PSL_t *psl, ptx_PSL_AnaMuxTestSignals_t signal)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        uint8_t sub_block_signal = (uint8_t) (signal & 0xFF);

        switch (signal)
        {
            case TestSignal_Vs0:
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_RxAdcBufLdoTestI:
                status = ptx_PSL_ModifyRegister(psl, ANA_RX_ADC_TEST_REG, 1u, ANA_RX_ADC_TEST_REG_RX_ADC_BUF_LDO_TEST_SEL_I_MASK);
                break;
            case TestSignal_RxAdcCoreAmpLdoTestI:
                status = ptx_PSL_ModifyRegister(psl, ANA_RX_ADC_TEST_REG, 1u, ANA_RX_ADC_TEST_REG_RX_ADC_CORE_AMP_LDO_TEST_SEL_I_MASK);
                break;
            case TestSignal_RxAdcTestPI:    /* Fall through */
            case TestSignal_RxAdcTestNI:
                status = ptx_PSL_ModifyRegister(psl, ANA_RX_ADC_TEST_REG, 1u, ANA_RX_ADC_TEST_REG_RX_ADC_TEST_EN_I_MASK);
                break;
            case TestSignal_RxAdcBufLdoTestQ:
                status = ptx_PSL_ModifyRegister(psl, ANA_RX_ADC_TEST_REG, 1u, ANA_RX_ADC_TEST_REG_RX_ADC_BUF_LDO_TEST_SEL_Q_MASK);
                break;
            case TestSignal_RxAdcCoreAmpLdoTestQ:
                status = ptx_PSL_ModifyRegister(psl, ANA_RX_ADC_TEST_REG, 1u, ANA_RX_ADC_TEST_REG_RX_ADC_CORE_AMP_LDO_TEST_SEL_Q_MASK);
                break;
            case TestSignal_RxAdcTestPQ:    /* Fall through */
            case TestSignal_RxAdcTestNQ:
                status = ptx_PSL_ModifyRegister(psl, ANA_RX_ADC_TEST_REG, 1u, ANA_RX_ADC_TEST_REG_RX_ADC_TEST_EN_Q_MASK);
                break;
            case TestSignal_LfInTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_CLOCK_TEST_REG, 1u, ANA_CLOCK_TEST_REG_LFIN_TO_TEST_MASK);
                break;
            case TestSignal_LfVsTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_CLOCK_TEST_REG, 1u, ANA_CLOCK_TEST_REG_LFVS_TO_TEST_MASK);
                break;
            case TestSignal_LdoPllVcoVoutTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_CLOCK_TEST_REG, 1u, ANA_CLOCK_TEST_REG_LDO_PLL_VCO_TEST_EN_MASK);
                break;
            case TestSignal_LdoPllAnaVoutTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_CLOCK_TEST_REG, 1u, ANA_CLOCK_TEST_REG_LDO_PLL_ANA_TEST_EN_MASK);
                break;
            case TestSignal_LdoXtalVoutTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_CLOCK_TEST_REG, 1u, ANA_CLOCK_TEST_REG_LDO_XTAL_TST_EN_MASK);
                break;
            case TestSignal_RxLdoTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_RX_TEST_REG, 1u, ANA_RX_TEST_REG_RX_LDO_TST_EN_MASK);
                break;
            case TestSignal_RxRefTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_RX_TEST_REG, 1u, ANA_RX_TEST_REG_RX_REF_TST_EN_MASK);
                break;
            case TestSignal_RxClockExtLdoTst:
                status = ptx_PSL_ModifyRegister(psl, ANA_RX_TEST_REG, 1u, ANA_RX_TEST_REG_RX_CLOCKEXT_TST_EN_MASK);
                break;
            case TestSignal_RxA0TestTiaIp:      /* Fall through */
            case TestSignal_RxA1TestTiaIn:      /* Fall through */
            case TestSignal_RxA0TestDccIp:      /* Fall through */
            case TestSignal_RxA1TestDccIn:      /* Fall through */
            case TestSignal_RxA0TestBBinIp:     /* Fall through */
            case TestSignal_RxA1TestBBinIn:     /* Fall through */
            case TestSignal_RxA0TestBBoutIp:    /* Fall through */
            case TestSignal_RxA1TestBBoutIn:
                /* Enable the MUX; set differential mode; set I channel; Signal is don't care due to differential mode! */
                status = ptx_PSL_WriteRegister(psl, ANA_RX_TMUXA_REG, 0x01);
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Configure the module and block. */
                    status = ptx_PSL_ModifyRegister(psl, ANA_RX_TMUXA_REG, sub_block_signal, 0x60);
                }
                break;
            case TestSignal_RxA0TestBuffer:     /* Fall through */
            case TestSignal_RxA1TestBuffer:
                /* Enable the MUX; Set buffer mode. */
                status = ptx_PSL_WriteRegister(psl, ANA_RX_TMUXA_REG, 0x07);
                break;
            /* All Q channel signals. */
            case TestSignal_RxB0TestTiaQp:      /* Fall through */
            case TestSignal_RxB1TestTiaQn:      /* Fall through */
            case TestSignal_RxB0TestDCCQp:      /* Fall through */
            case TestSignal_RxB1TestDCCQn:      /* Fall through */
            case TestSignal_RxB0TestBBinQp:     /* Fall through */
            case TestSignal_RxB1TestBBinQn:     /* Fall through */
            case TestSignal_RxB0TestBBoutQp:    /* Fall through */
            case TestSignal_RxB1TestBBoutQn:
                /* Enable the MUX; Set differential mode; Set Q channel; Signal is don't care due to differential mode. */
                status = ptx_PSL_WriteRegister(psl, ANA_RX_TMUXB_REG, 0x09);
                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Configure the module and block. */
                    status = ptx_PSL_ModifyRegister(psl, ANA_RX_TMUXB_REG, sub_block_signal, 0x60);
                }
                break;
            case TestSignal_RxB0TestBuffer: /* Fall through */
            case TestSignal_RxB1TestBuffer:
                /* Enable the MUX; Set buffer mode. */
                status = ptx_PSL_WriteRegister(psl, ANA_RX_TMUXB_REG, 0x07);
                break;
            case TestSignal_RxIBankIbc5uTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_RX_TEST_REG, 1u, ANA_RX_TEST_REG_RX_IBANK_TST_EN_MASK);
                break;
            case TestSignal_RxVmixTest:
                status = ptx_PSL_ModifyRegister(psl, ANA_RX_TEST_REG, 1u, ANA_RX_TEST_REG_RX_VMIX_TST_EN_MASK);
                break;
            case TestSignal_PmuTestOut0LL:                  /* Fall through */
            case TestSignal_PmuTestOut0IbRef0u4Test:        /* Fall through */
            case TestSignal_PmuTestOut0Ib0u1TuneTest:       /* Fall through */
            case TestSignal_PmuTestOut0Ib1uRefTest:         /* Fall through */
            case TestSignal_PmuTestOut0Ib1uTuneTest:        /* Fall through */
            case TestSignal_PmuTestOut0Ib20uTuneTest:       /* Fall through */
            case TestSignal_PmuTestOut0VrefTest:            /* Fall through */
            case TestSignal_PmuTestOut0V2iHpVrefTest:       /* Fall through */
            case TestSignal_PmuTestOut0V2iHpVrefTuneTest:   /* Fall through */
            case TestSignal_PmuTestOut0V2iLpVrefTest:       /* Fall through */
            case TestSignal_PmuTestOut0V2iLpVrefTuneTest:   /* Fall through */
            case TestSignal_PmuTestOut0Ldo2v5OutTest:       /* Fall through */
            case TestSignal_PmuTestOut0HpLdoVoutTest:       /* Fall through */
            case TestSignal_PmuTestOut0VsFromLpBias:        /* Fall through */
            case TestSignal_PmuTestOut0LL14:                /* Fall through */
            case TestSignal_PmuTestOut0LL15:                /* Fall through */
            case TestSignal_PmuTestOut1LL:                  /* Fall through */
            case TestSignal_PmuTestOut1IbRef0u4Test:        /* Fall through */
            case TestSignal_PmuTestOut1Ib0u1TuneTest:       /* Fall through */
            case TestSignal_PmuTestOut1Ib1uRefTest:         /* Fall through */
            case TestSignal_PmuTestOut1Ib1uTuneTest:        /* Fall through */
            case TestSignal_PmuTestOut1Ib20uTuneTest:       /* Fall through */
            case TestSignal_PmuTestOut1VrefTest:            /* Fall through */
            case TestSignal_PmuTestOut1V2iHpVrefTest:       /* Fall through */
            case TestSignal_PmuTestOut1V2iHpVrefTuneTest:   /* Fall through */
            case TestSignal_PmuTestOut1V2iLpVrefTest:       /* Fall through */
            case TestSignal_PmuTestOut1V2iLpVrefTuneTest:   /* Fall through */
            case TestSignal_PmuTestOut1Ldo2v5OutTest:       /* Fall through */
            case TestSignal_PmuTestOut1HpLdoVoutTest:       /* Fall through */
            case TestSignal_PmuTestOut1VsFromLpBias:        /* Fall through */
            case TestSignal_PmuTestOut1LL14:                /* Fall through */
            case TestSignal_PmuTestOut1LL15:
                status = ptx_PSL_HandlePmuSignal(psl, signal);
                break;
            case TestSignal_SwpLdoTestOut:
                status = ptx_PSL_ModifyRegister(psl, ANA_SWP_TEST1_REG, 1u, ANA_SWP_TEST1_REG_SWP_LDO_TEST_EN_MASK);
                break;
            case TestSignal_HiZ26:
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_HiZ27:
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_TxTestHiZ:                      /* Fall through */
            case TestSignal_TxTest05VsPmos:                 /* Fall through */
            case TestSignal_TxTest05VdNmos:                 /* Fall through */
            case TestSignal_TxTestAmpOutIsense:
                status = ptx_PSL_HandleTxTestSignals(psl, signal);
                break;
            case TestSignal_TsensTestOutN:                  /* Fall through */
            case TestSignal_TsensTestOutP:
                status = ptx_PSL_ModifyRegister(psl, TSENS_DECIM_REG, 1u, TSENS_DECIM_REG_TSENS_EN_MASK);
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_ModifyRegister(psl, TSENS_DECIM_REG, 1u, TSENS_DECIM_REG_TSENS_TEST_EN_MASK);
                }
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_ModifyRegister(psl, SYS_CLOCK_CONTROL0_REG, 1u, SYS_CLOCK_CONTROL0_REG_CLOCK_TSENS_EN_MASK);
                }
                break;
            case TestSignal_XtalTestOut:
                status = ptx_PSL_ModifyRegister(psl, ANA_CLOCK_TEST_REG, 1u, ANA_CLOCK_TEST_REG_XTAL_TEST_EN_MASK);
                break;
            case TestSignal_HiZ:                            /* Fall through */
            case TestSignal_RxDacOut0:                      /* Fall through */
            case TestSignal_RxDacOut1:                      /* Fall through */
            case TestSignal_TestAdc0DacOut:                 /* Fall through */
            case TestSignal_TestAdc1DacOut:
                status = PTX_STATUS_SUCCESS;
                break;
            case TestSignal_ScaledVbat:
                status = ptx_PSL_ModifyRegister(psl, ANA_SUPPLY_TEST_REG, 1u, ANA_SUPPLY_TEST_REG_VBAT_TEST_EN_MASK);
                break;
            case TestSignal_ScaledVdpa:
                status = ptx_PSL_ModifyRegister(psl, ANA_SUPPLY_TEST_REG, 1u, ANA_SUPPLY_TEST_REG_VDPA_TEST_EN_MASK);
                break;
            case TestSignal_PolyResistor:
                status = PTX_STATUS_SUCCESS;
                break;
            default:
                break;
        }

    }

    return status;
}

static ptx_Status_t ptx_PSL_DigSwapTestMux(ptx_PSL_t *psl, uint8_t enableSwap)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        uint8_t value = (0 != enableSwap) ? (0x01) : (0x00);

        status = ptx_PSL_ModifyRegister(psl, SYS_DFT_TB0_SEL_REG, value, SYS_DFT_TB0_SEL_REG_TB_SWAP_EN_MASK);
    }

    return status;
}

static ptx_Status_t ptx_PSL_DigSwapTb0(ptx_PSL_t *psl, ptx_PSL_TbSwapType_t swapType)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        status = ptx_PSL_ModifyRegister(psl, SYS_DFT_TB_CONTROL_REG, swapType, SYS_DFT_TB_CONTROL_REG_TB0_BIT_SWAP_SEL_MASK);
    }

    return status;
}

static ptx_Status_t ptx_PSL_DigSwapTb1(ptx_PSL_t *psl, ptx_PSL_TbSwapType_t swapType)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        status = ptx_PSL_ModifyRegister(psl, SYS_DFT_TB_CONTROL_REG, swapType, SYS_DFT_TB_CONTROL_REG_TB1_BIT_SWAP_SEL_MASK);
    }

    return status;
}

static ptx_Status_t ptx_PSL_DigSelSysClkPin(ptx_PSL_t *psl, uint8_t pinNbr)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support) && (0x08 >= pinNbr))
    {
        status = ptx_PSL_EnableDigPads8_1(psl);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_SetTMuxCtrl(psl);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ModifyRegister(psl, SYS_DFT_TB_CONTROL_REG, pinNbr, SYS_DFT_TB_CONTROL_REG_TB_CLOCK_SEL_MASK);
        }

    }

    return status;
}

static ptx_Status_t ptx_PSL_EnableDigPads8_1(ptx_PSL_t *psl)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        psl->IgnoreGpio2 = 0;

        uint8_t sys_clock_ctrl_val = 0;
        status = ptx_PSL_ReadRegister(psl, SYS_CLOCK_CONTROL1_REG, &sys_clock_ctrl_val);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* GPIO2 is used as a reference clock input signal and cannot be used for digital test bus signals. */
            psl->IgnoreGpio2 = (sys_clock_ctrl_val & SYS_CLOCK_CONTROL1_REG_CLOCK_REF_GPIO_EN_MASK);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ModifyRegister(psl, PAD_DIG8_IO2_REG, 0, PAD_DIGX_IOX_REG_OEN_MASK);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ModifyRegister(psl, PAD_DIG7_IO2_REG, 0, PAD_DIGX_IOX_REG_OEN_MASK);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ModifyRegister(psl, PAD_DIG6_IO2_REG, 0, PAD_DIGX_IOX_REG_OEN_MASK);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ModifyRegister(psl, PAD_DIG5_IO2_REG, 0, PAD_DIGX_IOX_REG_OEN_MASK);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ModifyRegister(psl, PAD_DIG4_IO2_REG, 0, PAD_DIGX_IOX_REG_OEN_MASK);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ModifyRegister(psl, PAD_DIG3_IO1_REG, 0, PAD_DIGX_IOX_REG_OEN_MASK);
        }

        if (PTX_STATUS_SUCCESS == status && (0 == psl->IgnoreGpio2))
        {
            status = ptx_PSL_ModifyRegister(psl, PAD_DIG2_IO1_REG, 0, PAD_DIGX_IOX_REG_OEN_MASK);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ModifyRegister(psl, PAD_DIG1_IO1_REG, 0, PAD_DIGX_IOX_REG_OEN_MASK);
        }
    }

    return status;
}

static ptx_Status_t ptx_PSL_SetTMuxCtrl(ptx_PSL_t *psl)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        uint8_t mask = 0xFF;

        if (psl->IgnoreGpio2)
        {
            mask = 0xFD;
        }

        status = ptx_PSL_WriteRegister(psl, PAD_TMUX_CONTROL0_REG, mask);
    }

    return status;
}


static ptx_Status_t ptx_PSL_GetDefaultQMeasParams(ptx_PSL_t *psl, ptx_PSL_QMeas_Parameters_t *defParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support) && (NULL != defParams))
    {
        memcpy(defParams, &DefaultQMeasParameters, sizeof(ptx_PSL_QMeas_Parameters_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_PSL_ResetDigPads8_1(ptx_PSL_t *psl)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support))
    {
        status = ptx_PSL_WriteRegister(psl, PAD_DIG8_IO2_REG, PAD_DIG8_IO2_REG_RST);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG7_IO2_REG, PAD_DIG7_IO2_REG_RST);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG6_IO2_REG, PAD_DIG6_IO2_REG_RST);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG5_IO2_REG, PAD_DIG5_IO2_REG_RST);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG4_IO2_REG, PAD_DIG4_IO2_REG_RST);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG3_IO1_REG, PAD_DIG3_IO1_REG_RST);
        }

        if ((PTX_STATUS_SUCCESS == status) && (0 == psl->IgnoreGpio2))
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG2_IO1_REG, PAD_DIG2_IO1_REG_RST);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, PAD_DIG1_IO1_REG, PAD_DIG1_IO1_REG_RST);
        }
    }

    return status;
}
