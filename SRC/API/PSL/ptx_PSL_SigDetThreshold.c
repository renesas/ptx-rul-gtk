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
    File        : ptx_PSL_SigDetThreshold.c

    Description : Implementation of the Signal Detector Threshold Measurement.
 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_PSL.h"
#include "ptx_NSC_RfTest.h"
#include <math.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 *  SigDet, Dscope defines
 */
#define PTX_PSL_DSCOPE_VALID_RANGE          (1000)  /* Valid measurement threshold. If the measurement value 
                                                     *  is above the threshold, something went wrong. */
#define PTX_PSL_SIGDET_NR_OF_CHANNELS       (4)     /* Array index for the standard deviation. */
#define PTX_PSL_SIGDET_NR_OF_DATA_POINTS    (256)   /* Amount of data points. */
#define PTX_PSL_SIGDET_NR_OF_STAT_ITEMS     (3)     /* Amount of statistical parameters. */
#define PTX_PSL_SIGDET_AVG_IDX              (0)     /* Array index for the average. */
#define PTX_PSL_SIGDET_VAR_IDX              (1)     /* Array index for the variance. */
#define PTX_PSL_SIGDET_STDDEV_IDX           (2)     /* Array index for the standard deviation. */
#define PTX_PSL_DSCOPE_CHANNEL0_SHIFT       (0)     /* Bitshift for DSCOPE Channel0 register configurations. */
#define PTX_PSL_DSCOPE_CHANNEL1_SHIFT       (4)     /* Bitshift for DSCOPE Channel1 register configurations. */
#define PTX_PSL_CMEM_CHUNK_SIZE             (128)   /* Amount of CMEM words that can be read with one HIP frame. */
#define PTX_PSL_RAM0_START_ADDRESS          (0x2000)/* Start address of RAM0 module. */
#define PTX_PSL_RAM1_START_ADDRESS          (0x4000)/* Start address of RAM1 module. */
#define PTX_PSL_CMEM_SECTION_0_LEN          (8)     /* Length of the section to restore in CMEM 0. */
#define PTX_PSL_CMEM_SECTION_1_LEN          (2)     /* Length of the section to restore in CMEM 1. */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/* Helper functions to control the Dscope block. */
static ptx_Status_t ptx_PSL_DScopeConfigureSigDetMeas(ptx_PSL_t *psl);
static ptx_Status_t ptx_PSL_DScopeCaptureImmediate(ptx_PSL_t *psl, uint16_t *data);

/* Helper functions for the analog and digital block configuration. */
static ptx_Status_t ptx_PSL_ConfigureFrontEnd(ptx_PSL_t *psl, ptx_PSL_NfcType_t nfc_type);
static ptx_Status_t ptx_PSL_ApplyStaticConfiguration(ptx_PSL_t *psl);
static ptx_Status_t ptx_PSL_StartTx(ptx_PSL_t *psl, uint8_t txMultiplier);
static ptx_Status_t ptx_PSL_StopTx(ptx_PSL_t *psl);

static ptx_Status_t ptx_PSL_DScopeEnable(ptx_PSL_t *psl);
static ptx_Status_t ptx_PSL_DScopeSource(ptx_PSL_t *psl, ptx_PSL_DScopeSource_t ch0Source, ptx_PSL_DScopeSource_t ch1Source);
static ptx_Status_t ptx_PSL_DScopeStrobe(ptx_PSL_t *psl, ptx_PSL_DScopeStrobe_t ch0Strobe, ptx_PSL_DScopeStrobe_t ch1Strobe);
static ptx_Status_t ptx_PSL_DScopeCaptureSize(ptx_PSL_t *psl, ptx_PSL_DScopeCaptureSize_t ch0Size, ptx_PSL_DScopeCaptureSize_t ch1Size);
static ptx_Status_t ptx_PSL_DScopeCaptureOffset(ptx_PSL_t *psl, ptx_PSL_DScopeCaptureOffset_t ch0Offset, ptx_PSL_DScopeCaptureOffset_t ch1Offset);
static ptx_Status_t ptx_PSL_DScopeDeInit(ptx_PSL_t *psl);
static ptx_Status_t ptx_PSL_DScopeStartSampling(ptx_PSL_t *psl, uint8_t startCh0, uint8_t startCh1);
static ptx_Status_t ptx_PSL_DScopeStopSampling(ptx_PSL_t *psl, uint8_t stopCh0, uint8_t stopCh1);
static uint16_t     ptx_PSL_DScopeCaptureSizeToCmemWords(ptx_PSL_DScopeCaptureSize_t size);
static ptx_Status_t ptx_PSL_DScopeReadCapture(ptx_PSL_t *psl, uint16_t *destinationData, ptx_PSL_DScopeRamModule_t module, ptx_PSL_DScopeCaptureOffset_t offset, ptx_PSL_DScopeCaptureSize_t size);
static ptx_Status_t ptx_PSL_DScopeCaptureImmediate(ptx_PSL_t *psl, uint16_t *data);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_PSL_SigDetMeasurement(ptx_PSL_t *psl, ptx_PSL_SigDet_InParams_t *inParams, uint32_t *sigDetValue)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(psl, Comp_Support) && (NULL != inParams) && (NULL != sigDetValue))
    {
        if (ptx_NSC_State_Idle != psl->Rul->Nsc.State)
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }

        /* Save current CMEM content */
        uint16_t cmem0_section_to_restore[PTX_PSL_CMEM_SECTION_0_LEN] = {0};
        uint16_t cmem1_section_to_restore[PTX_PSL_CMEM_SECTION_1_LEN] = {0};

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HIP_RdCMem(&(psl->Rul->Hip), DSCOPE_RAM0_READ_ADDR, cmem0_section_to_restore, sizeof(cmem0_section_to_restore) / sizeof(cmem0_section_to_restore[0]));
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HIP_RdCMem(&(psl->Rul->Hip), DSCOPE_RAM1_FW_VERSION_ADDR, cmem1_section_to_restore, sizeof(cmem1_section_to_restore) / sizeof(cmem1_section_to_restore[0]));
        }

        /* Save current frontend configuration */
        uint8_t rx_fe_reg_value = 0;
        uint8_t rx_bb_reg_value = 0;
        uint8_t drx_df_ctrl_reg_value = 0;
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ReadRegister(psl, ANA_RX_FE_CONFIG_REG, &rx_fe_reg_value);
            status |= ptx_PSL_ReadRegister(psl, ANA_RX_BB_CONFIG_REG, &rx_bb_reg_value);
            status |= ptx_PSL_ReadRegister(psl, CL_DRX_DF_CTRL_REG, &drx_df_ctrl_reg_value);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_DScopeConfigureSigDetMeas(psl);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ConfigureFrontEnd(psl, inParams->NfcType);
        }

        /* Apply static block configuration common to all types and data rates. */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_ApplyStaticConfiguration(psl);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_StartTx(psl, inParams->TxMultiplier);
        }

        int16_t channel_data[PTX_PSL_SIGDET_NR_OF_CHANNELS][PTX_PSL_SIGDET_NR_OF_DATA_POINTS];
        memset(&channel_data[0][0], 0x00, PTX_PSL_SIGDET_NR_OF_CHANNELS * PTX_PSL_SIGDET_NR_OF_DATA_POINTS * sizeof(uint16_t));

        /*
         * Extract all 4 channels from the Dscope.
         * Select x_sub_y_int_s on digrx debug bus.
         */
        uint8_t channel_idx;
        if (PTX_STATUS_SUCCESS == status)
        {
            for (channel_idx = 0; (PTX_STATUS_SUCCESS == status) && (channel_idx < PTX_PSL_SIGDET_NR_OF_CHANNELS); channel_idx++)
            {
                status = ptx_PSL_WriteRegister(psl, CL_DRX_DBG_DAC0_SEL_REG, channel_idx);

                (void) ptx_RUL_Sleep(psl->Rul, 5u);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_PSL_DScopeCaptureImmediate(psl, (uint16_t*) & (channel_data[channel_idx][0]));
                }
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Stop DScope after measuring. */
            status = ptx_PSL_DScopeDeInit(psl);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Stop transmitter after measuring. */
            status = ptx_PSL_StopTx(psl);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_RX_FE_CONFIG_REG, rx_fe_reg_value);
            status |= ptx_PSL_WriteRegister(psl, ANA_RX_BB_CONFIG_REG, rx_bb_reg_value);
            status |= ptx_PSL_WriteRegister(psl, CL_DRX_DF_CTRL_REG, drx_df_ctrl_reg_value);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /*
             * Calculate signal statistics.
             * array[][0] = average/mean
             * array[][1] = variance
             * array[][2] = stddef
             */
            float channel_statistics[PTX_PSL_SIGDET_NR_OF_CHANNELS][PTX_PSL_SIGDET_NR_OF_STAT_ITEMS];
            memset(&channel_statistics[0][0], 0x00, sizeof(float) * PTX_PSL_SIGDET_NR_OF_CHANNELS * PTX_PSL_SIGDET_NR_OF_STAT_ITEMS);

            uint16_t array_idx;
            for (channel_idx = 0; channel_idx < PTX_PSL_SIGDET_NR_OF_CHANNELS; channel_idx++)
            {
                float average = 0, variance = 0, std_deviation = 0, sum = 0, sum1 = 0;

                uint16_t start_idx = (PTX_PSL_SIGDET_NR_OF_DATA_POINTS / 2);
                /* Compute the sum of all elements. */
                for (array_idx = start_idx; array_idx < PTX_PSL_SIGDET_NR_OF_DATA_POINTS; array_idx++)
                {
                    sum = sum + channel_data[channel_idx][array_idx];
                }
                average = sum / ((float) (PTX_PSL_SIGDET_NR_OF_DATA_POINTS - start_idx));

                /* Compute variance and standard deviation. */
                for (array_idx = start_idx; array_idx < PTX_PSL_SIGDET_NR_OF_DATA_POINTS; array_idx++)
                {
                    sum1 = sum1 + (float) pow((((float) channel_data[channel_idx][array_idx]) - average), 2);
                }
                variance = sum1 / ((float) (PTX_PSL_SIGDET_NR_OF_DATA_POINTS - start_idx));
                std_deviation = (float) sqrt(variance);

                channel_statistics[channel_idx][PTX_PSL_SIGDET_AVG_IDX] = average;
                channel_statistics[channel_idx][PTX_PSL_SIGDET_VAR_IDX] = variance;
                channel_statistics[channel_idx][PTX_PSL_SIGDET_STDDEV_IDX] = std_deviation;
            }

            float max_sigdet = 0;
            float temp_sigdet = 0;
            /* Use the max value out of the 4 channels for the final SigDet value. */
            for (channel_idx = 0; channel_idx < PTX_PSL_SIGDET_NR_OF_CHANNELS; channel_idx++)
            {
                temp_sigdet = (channel_statistics[channel_idx][PTX_PSL_SIGDET_AVG_IDX] > 0 ? channel_statistics[channel_idx][PTX_PSL_SIGDET_AVG_IDX] : -channel_statistics[channel_idx][PTX_PSL_SIGDET_AVG_IDX]);
                temp_sigdet = temp_sigdet + (inParams->SigmaFactor * channel_statistics[channel_idx][PTX_PSL_SIGDET_STDDEV_IDX]);
                if (max_sigdet < temp_sigdet)
                {
                    max_sigdet = temp_sigdet;
                }
            }

            if (PTX_PSL_DSCOPE_VALID_RANGE > max_sigdet)
            {
                *sigDetValue = (uint32_t) max_sigdet;
            }
            else
            {
                *sigDetValue = 0;
                status = PTX_STATUS_E_GEN_ADCCLIPPING;
            }
        }

        /* Restore the CMEM content in any case. */
        (void) ptx_HIP_WrCMem(&(psl->Rul->Hip), DSCOPE_RAM0_READ_ADDR, cmem0_section_to_restore, sizeof(cmem0_section_to_restore) / sizeof(cmem0_section_to_restore[0]));
        (void) ptx_HIP_WrCMem(&(psl->Rul->Hip), DSCOPE_RAM1_FW_VERSION_ADDR, cmem1_section_to_restore, sizeof(cmem1_section_to_restore) / sizeof(cmem1_section_to_restore[0]));
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_PSL_StartTx(ptx_PSL_t *psl, uint8_t txMultiplier)
{
    /* Set multiplier */
    ptx_Status_t status = ptx_PSL_WriteRegister(psl, TX_WAVE_CW_REG, txMultiplier);

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Enable PA block */
        status =  ptx_PSL_ModifyRegister(psl, ANA_TX_PA_REG, 1u, ANA_TX_PA_REG_TX_PA_BIAS_EN_MASK);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Enable standard mode (3V - 5.5V) */
        status = ptx_PSL_WriteRegister(psl, ANA_TX_PA_CONTROL_REG, 0x00);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Enable PA block */
        status = ptx_PSL_ModifyRegister(psl, ANA_TX_PA_REG, 3u, ANA_TX_PA_REG_TX_PA_EN_MASK);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Enable the TX CLOCK */
        status = ptx_PSL_ModifyRegister(psl, SYS_CLOCK_CONTROL0_REG, 1u, SYS_CLOCK_CONTROL0_REG_CLOCK_TX_EN_MASK);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Enable TX WAVEGEN */
        status = ptx_PSL_ModifyRegister(psl, TX_WAVEGEN_CONFIG_REG, 1u, TX_WAVEGEN_CONFIG_REG_TX_WAVEGEN_EN_MASK);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Enable RF Field */
        status = ptx_PSL_WriteRegister(psl, CL_RF_FIELD_REG, CL_RF_FIELD_REG_RFF_ACT_EN_MASK);
    }

    return status;
}

static ptx_Status_t ptx_PSL_StopTx(ptx_PSL_t *psl)
{
    return ptx_PSL_ModifyRegister(psl, CL_RF_FIELD_REG, 0x00, CL_RF_FIELD_REG_RFF_ACT_EN_MASK);
}

static ptx_Status_t ptx_PSL_ApplyStaticConfiguration(ptx_PSL_t *psl)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Set reader threshold to max to keep DigRX alive. */
    status = ptx_PSL_WriteRegister(psl, CL_DRX_RD_TH0_REG, 0xFF);

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_PSL_WriteRegister(psl, CL_DRX_RD_TH1_REG, 0xFF);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_PSL_WriteRegister(psl, CL_DRX_RD_TH2_REG, 0x1F);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Enable the necessary clocks */
        status = ptx_PSL_WriteRegister(psl, SYS_CLOCK_CONTROL0_REG, 0x3F);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_PSL_WriteRegister(psl, ANA_RX_CONTROL0_REG, 0xFF);
    }

    /*
     * Enable RX-ADC
     */
    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_PSL_WriteRegister(psl, CONTROL_ADC_I0_REG, 0x36);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_PSL_WriteRegister(psl, CONTROL_ADC_I1_REG, 0x03);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_PSL_WriteRegister(psl, CONTROL_ADC_Q0_REG, 0x36);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_PSL_WriteRegister(psl, CONTROL_ADC_Q1_REG, 0x03);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Select i_sub_i_int_s on digrx debug bus */
        status = ptx_PSL_WriteRegister(psl, CL_DRX_DBG_DAC0_SEL_REG, 0x00);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        status = ptx_PSL_WriteRegister(psl, CL_DRX_DBG_SCAL_SEL_REG, 0x00);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Set CLI to receive mode */
        status = ptx_PSL_WriteRegister(psl, CL_CONTROL_REG, 0x04);
    }

    return status;
}

static ptx_Status_t ptx_PSL_ConfigureFrontEnd(ptx_PSL_t *psl, ptx_PSL_NfcType_t nfcType)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_NSC_RfConfig_ParamId_t rx_fe_reg_section = ParamId_Max;
    ptx_NSC_RfConfig_ParamId_t rx_bb_reg_section = ParamId_Max;
    ptx_NSC_RfConfig_ParamId_t drx_df_ctrl_section = ParamId_Max;

    uint8_t rx_fe_reg_value = 0;
    uint8_t rx_bb_reg_value = 0;
    uint8_t drx_df_ctrl_reg_value = 0;

    uint8_t config_regs_valid = 1u;

    switch (nfcType)
    {
        case NfcType_A106:
            rx_fe_reg_section = ParamId_AnaRxFeConfig_A106;
            rx_bb_reg_section = ParamId_AnaRxBbConfig_A106;
            drx_df_ctrl_section = ParamId_ClDrxDf_A106;
            break;
        case NfcType_A212:
            rx_fe_reg_section = ParamId_AnaRxFeConfig_A212;
            rx_bb_reg_section = ParamId_AnaRxBbConfig_A212;
            drx_df_ctrl_section = ParamId_ClDrxDf_A212;
            break;
        case NfcType_A424:
            rx_fe_reg_section = ParamId_AnaRxFeConfig_A424;
            rx_bb_reg_section = ParamId_AnaRxBbConfig_A424;
            drx_df_ctrl_section = ParamId_ClDrxDf_A424;
            break;
        case NfcType_A848:
            rx_fe_reg_section = ParamId_AnaRxFeConfig_A848;
            rx_bb_reg_section = ParamId_AnaRxBbConfig_A848;
            drx_df_ctrl_section = ParamId_ClDrxDf_A848;
            break;
        case NfcType_B106:
            rx_fe_reg_section = ParamId_AnaRxFeConfig_B106;
            rx_bb_reg_section = ParamId_AnaRxBbConfig_B106;
            drx_df_ctrl_section = ParamId_ClDrxDf_B106;
            break;
        case NfcType_B212:
            rx_fe_reg_section = ParamId_AnaRxFeConfig_B212;
            rx_bb_reg_section = ParamId_AnaRxBbConfig_B212;
            drx_df_ctrl_section = ParamId_ClDrxDf_B212;
            break;
        case NfcType_B424:
            rx_fe_reg_section = ParamId_AnaRxFeConfig_B424;
            rx_bb_reg_section = ParamId_AnaRxBbConfig_B424;
            drx_df_ctrl_section = ParamId_ClDrxDf_B424;
            break;
        case NfcType_B848:
            rx_fe_reg_section = ParamId_AnaRxFeConfig_B848;
            rx_bb_reg_section = ParamId_AnaRxBbConfig_B848;
            drx_df_ctrl_section = ParamId_ClDrxDf_B848;
            break;
        case NfcType_F212:
            rx_fe_reg_section = ParamId_AnaRxFeConfig_F212;
            rx_bb_reg_section = ParamId_AnaRxBbConfig_F212;
            drx_df_ctrl_section = ParamId_ClDrxDf_F212;
            break;
        case NfcType_F424:
            rx_fe_reg_section = ParamId_AnaRxFeConfig_F424;
            rx_bb_reg_section = ParamId_AnaRxBbConfig_F424;
            drx_df_ctrl_section = ParamId_ClDrxDf_F424;
            break;
        case NfcType_V026:
            rx_fe_reg_section = ParamId_AnaRxFeConfig_V26;
            rx_bb_reg_section = ParamId_AnaRxBbConfig_V26;
            drx_df_ctrl_section = ParamId_ClDrxDf_V26;
            break;
        default:
            config_regs_valid = 0;
            break;
    }

    if (1u == config_regs_valid)
    {
        status = ptx_NSC_RfConfig_GetParam(&(psl->Rul->Nsc), rx_fe_reg_section, &rx_fe_reg_value);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RfConfig_GetParam(&(psl->Rul->Nsc), rx_bb_reg_section, &rx_bb_reg_value);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RfConfig_GetParam(&(psl->Rul->Nsc), drx_df_ctrl_section, &drx_df_ctrl_reg_value);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_RX_FE_CONFIG_REG, rx_fe_reg_value);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, ANA_RX_BB_CONFIG_REG, rx_bb_reg_value);
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_PSL_WriteRegister(psl, CL_DRX_DF_CTRL_REG, drx_df_ctrl_reg_value);
        }
    }

    return status;
}

static ptx_Status_t ptx_PSL_DScopeConfigureSigDetMeas(ptx_PSL_t *psl)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Disable Dscope */
    status = ptx_PSL_DScopeDeInit(psl);

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Select DigRx as source for CH0 & CH1 */
        status = ptx_PSL_DScopeSource(psl, DScopeSource_DigRx, DScopeSource_DigRx);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Set sampling rate to 3.39MHz for CH0 & CH1 */
        status = ptx_PSL_DScopeStrobe(psl, DScopeStrobe_3MHz39, DScopeStrobe_3MHz39);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Set RAM0 & RAM1 capturing start positions */
        status = ptx_PSL_DScopeCaptureOffset(psl, DScopeCaptureOffset_0KB, DScopeCaptureOffset_15KB);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Set capture size */
        status = ptx_PSL_DScopeCaptureSize(psl, DScopeCaptureSize_256B, DScopeCaptureSize_256B);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Enable Dscope */
        status = ptx_PSL_DScopeEnable(psl);
    }

    return status;
}

static ptx_Status_t ptx_PSL_DScopeEnable(ptx_PSL_t *psl)
{
    return ptx_PSL_WriteRegister(psl, DSCOPE_CTRL_CONTROL_REG, 0x01);
}

static ptx_Status_t ptx_PSL_DScopeSource(ptx_PSL_t *psl, ptx_PSL_DScopeSource_t ch0Source, ptx_PSL_DScopeSource_t ch1Source)
{
    return ptx_PSL_WriteRegister(psl, DSCOPE_CTRL_SRC_SEL_REG, (uint8_t)((ch1Source << PTX_PSL_DSCOPE_CHANNEL1_SHIFT) |
                                                                         (ch0Source << PTX_PSL_DSCOPE_CHANNEL0_SHIFT)));
}


static ptx_Status_t ptx_PSL_DScopeStrobe(ptx_PSL_t *psl, ptx_PSL_DScopeStrobe_t ch0Strobe, ptx_PSL_DScopeStrobe_t ch1Strobe)
{
    return ptx_PSL_WriteRegister(psl, DSCOPE_CTRL_STR_SEL_REG, (uint8_t)((ch1Strobe << PTX_PSL_DSCOPE_CHANNEL1_SHIFT) |
                                                                         (ch0Strobe << PTX_PSL_DSCOPE_CHANNEL0_SHIFT)));
}


static ptx_Status_t ptx_PSL_DScopeCaptureSize(ptx_PSL_t *psl, ptx_PSL_DScopeCaptureSize_t ch0Size, ptx_PSL_DScopeCaptureSize_t ch1Size)
{
    return ptx_PSL_WriteRegister(psl, DSCOPE_CTRL_MAX_LEN_REG, (uint8_t)((ch1Size << PTX_PSL_DSCOPE_CHANNEL1_SHIFT) |
                                                                         (ch0Size << PTX_PSL_DSCOPE_CHANNEL0_SHIFT)));
}


static ptx_Status_t ptx_PSL_DScopeCaptureOffset(ptx_PSL_t *psl, ptx_PSL_DScopeCaptureOffset_t ch0Offset, ptx_PSL_DScopeCaptureOffset_t ch1Offset)
{
    return ptx_PSL_WriteRegister(psl, DSCOPE_CTRL_OFFSET_REG, (uint8_t)((ch1Offset << PTX_PSL_DSCOPE_CHANNEL1_SHIFT) |
                                                                        (ch0Offset << PTX_PSL_DSCOPE_CHANNEL0_SHIFT)));
}

static ptx_Status_t ptx_PSL_DScopeDeInit(ptx_PSL_t *psl)
{
    return ptx_PSL_WriteRegister(psl, DSCOPE_CTRL_CONTROL_REG, 0x00);
}

static ptx_Status_t ptx_PSL_DScopeStartSampling(ptx_PSL_t *psl, uint8_t startCh0, uint8_t startCh1)
{
    return ptx_PSL_WriteRegister(psl, DSCOPE_CTRL_CONTROL_REG, (uint8_t)(0x01 | (startCh1 != 0 ? 0x04 : 0) | (startCh0 != 0 ? 0x02 : 0)));
}

static ptx_Status_t ptx_PSL_DScopeStopSampling(ptx_PSL_t *psl, uint8_t stopCh0, uint8_t stopCh1)
{
    return ptx_PSL_WriteRegister(psl, DSCOPE_CTRL_CONTROL_REG, (uint8_t)(0x01 | (stopCh1 != 0 ? 0x10 : 0) | (stopCh0 != 0 ? 0x08 : 0)));
}

static uint16_t ptx_PSL_DScopeCaptureSizeToCmemWords(ptx_PSL_DScopeCaptureSize_t size)
{
    uint16_t words = 0;

    switch (size)
    {
        case DScopeCaptureSize_8B:
            words = 4;
            break;
        case DScopeCaptureSize_16B:
            words = 8;
            break;
        case DScopeCaptureSize_32B:
            words = 16;
            break;
        case DScopeCaptureSize_64B:
            words = 32;
            break;
        case DScopeCaptureSize_128B:
            words = 64;
            break;
        case DScopeCaptureSize_256B:
            words = 128;
            break;
        case DScopeCaptureSize_512B:
            words = 256;
            break;
        case DScopeCaptureSize_1KB:
            words = 512;
            break;
        case DScopeCaptureSize_2KB:
            words = 1024;
            break;
        case DScopeCaptureSize_4KB:
            words = 2048;
            break;
        case DScopeCaptureSize_6KB:
            words = 3072;
            break;
        case DScopeCaptureSize_8KB:
            words = 4096;
            break;
        case DScopeCaptureSize_10KB:
            words = 5120;
            break;
        case DScopeCaptureSize_12KB:
            words = 6144;
            break;
        case DScopeCaptureSize_14KB:
            words = 7168;
            break;
        case DScopeCaptureSize_16KB:
            words = 8192;
            break;
    }

    return words;
}

static ptx_Status_t ptx_PSL_DScopeReadCapture(ptx_PSL_t *psl, uint16_t *destinationData, ptx_PSL_DScopeRamModule_t module, ptx_PSL_DScopeCaptureOffset_t offset, ptx_PSL_DScopeCaptureSize_t size)
{
    ptx_Status_t status = PTX_STATUS_SUCCESS;

    uint16_t start_address = (uint16_t)((module == DScopeRamModule0 ? PTX_PSL_RAM0_START_ADDRESS : PTX_PSL_RAM1_START_ADDRESS) +  (offset * 512));
    uint16_t words_to_read = ptx_PSL_DScopeCaptureSizeToCmemWords(size);

    uint16_t full_chunks = words_to_read / PTX_PSL_CMEM_CHUNK_SIZE;
    uint16_t partial_chunk_len = words_to_read % PTX_PSL_CMEM_CHUNK_SIZE;

    uint8_t i;
    for (i = 0; (i < full_chunks) && (PTX_STATUS_SUCCESS == status); ++i)
    {
        status = ptx_HIP_RdCMem(&(psl->Rul->Hip), start_address, &destinationData[i * PTX_PSL_CMEM_CHUNK_SIZE], PTX_PSL_CMEM_CHUNK_SIZE);
        start_address = start_address + PTX_PSL_CMEM_CHUNK_SIZE;
    }

    if ((PTX_STATUS_SUCCESS == status) && (0 != partial_chunk_len))
    {
        status = ptx_HIP_RdCMem(&(psl->Rul->Hip), start_address, &destinationData[i * PTX_PSL_CMEM_CHUNK_SIZE], partial_chunk_len);
        start_address = start_address + partial_chunk_len;
    }

    return status;
}

static ptx_Status_t ptx_PSL_DScopeCaptureImmediate(ptx_PSL_t *psl, uint16_t *data)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Return error code if either PSL or inParams pointer is invalid. */
    if (PTX_COMP_CHECK(psl, Comp_Support) && (NULL != data))
    {
        /* Start capturing. */
        status = ptx_PSL_DScopeStartSampling(psl, 1, 1);

        (void) ptx_RUL_Sleep(psl->Rul, 5u);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Stop capturing by writing to DSCOPE_CTRL_CONTROL_REG. */
            status = ptx_PSL_DScopeStopSampling(psl, 1, 1);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Read data from RAM and return raw data. */
            status = ptx_PSL_DScopeReadCapture(psl, data, DScopeRamModule0, DScopeCaptureOffset_0KB, DScopeCaptureSize_256B);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Read data from RAM and return raw data. */
            uint16_t words_to_read = ptx_PSL_DScopeCaptureSizeToCmemWords(DScopeCaptureSize_256B);
            status = ptx_PSL_DScopeReadCapture(psl, &data[words_to_read], DScopeRamModule1, DScopeCaptureOffset_15KB, DScopeCaptureSize_256B);
        }
    }

    return status;
}
