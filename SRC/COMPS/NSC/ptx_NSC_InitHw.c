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
    Module      : NFC Soft Controller (NSC) Interface for Hardware Initialization
    File        : ptx_NSC_InitHw.c

    Description : Implementation of NFC Soft Controller Interface for Hardware Initialization.
                  Contains function(s) to set-up the hardware after successful boot up and firmware activation.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <math.h>
#include "ptx_NSC_InitHw.h"
#include "ptx_NSC_StateMachine.h"
#include "ptx_HAL.h"


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NSC_INIT_HW_CMD_LENGTH                  (36u)           /* NSC INIT Command length */

#define PTX_NSC_RSP_ERRCODE_INDEX                   (1u)            /* The index of the ErrorCode value in the RSP message. */

#define PTX_PLL_FRES                                (1000000.0)     /* Default frequency resolution (= 1MHz). */
#define PTX_PLL_MIN_REFFREQ                         (13560000U)     /* Minimum allowed reference frequency (= 13.56MHz). */
#define PTX_PLL_MAX_REFFREQ                         (54240000U)     /* Maximum allowed reference frequency (= 54.24MHz). */
#define PTX_PLL_MAX_INT_FREQ                        (27120000U)     /* Frequency threshold to enable frequency divider (= 27.12MHz). */
#define PTX_PLL_FREQ_DIVIDER                        (64U)           /* Internal frequency divider. */


#if defined(PTX_ENABLE_I3C)
    #define I3C_IBI_INTERRUPTS_ENABLED                  (1)         /* I3C in-bound interrupt enabled */
    #define I3C_IBI_INTERRUPTS_DISABLED                 (0)         /* I3C in-bound interrupt disabled */
#endif

#if defined(PTX_ENABLE_UART)
#define PTX_UART_DEFAULT_NSC_SPEED                  (115200u)
#define UART_BR_DITHER_EN_MASK                      (0x80)
#define UART_BR_DIV1_VAL_MASK                       (0x7F)
#define UART_BR_DIV0_VAL_MASK                       (0x07)

/*
 * The list of supported UART baud-rate configurations
 */
#define UART_CUSTOM_BR_NUM                                          (37u)

static ptx_NSC_CustomBaudrates_t sUartRegValue[UART_CUSTOM_BR_NUM] =
{
    {9600u, 7u, 43u, 0u},       {14400u, 6u, 58u, 0u},      {19200u, 6u, 43u, 0u},
    {28800u, 5u, 58u, 0u},      {38400u, 5u, 43u, 0u},      {57600u, 4u, 58u, 0u},
    {115200u, 3u, 58u, 0u},     {153600u, 3u, 43u, 0u},     {230400u, 2u, 58u, 0u},
    {460800u, 1u, 58u, 0u},     {921600u, 0u, 58u, 0u},     {1000000u, 0u, 54u, 1u},
    {1228800u, 0u, 43u, 0u},    {1382400u, 0u, 38u, 0u},    {1500000u, 0u, 35u, 0u},
    {1638400u, 0u, 32u, 0u},    {1843200u, 0u, 29u, 1u},    {2000000u, 0u, 26u, 0u},
    {2184533u, 0u, 24u, 0u},    {2500000u, 0u, 21u, 0u},    {2912711u, 0u, 18u, 1u},
    {3000000u, 0u, 17u, 0u},    {3276800u, 0u, 16u, 1u},    {3375000u, 0u, 15u, 0u},
    {3500000u, 0u, 15u, 1u},    {3686400u, 0u, 14u, 1u},    {4000000u, 0u, 13u, 1u},
    {4300000u, 0u, 12u, 1u},    {4500000u, 0u, 11u, 0u},    {4915200u, 0u, 10u, 0u},
    {5000000u, 0u, 10u, 0u},    {5100000u, 0u, 10u, 1u},    {5461333u, 0u, 9u, 0u},
    {6000000u, 0u, 8u, 0u},     {6300000u, 0u, 8u, 1u},     {6750000u, 0u, 0u, 0u},
    {6780000u, 0u, 0u, 0u}
};
#endif // PTX_ENABLE_UART

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

#if defined(PTX_ENABLE_UART)
    static ptx_Status_t ptx_NSC_InitUartConfiguration(ptx_NSC_t *nsc, uint8_t *reg0, uint8_t *reg1);
    static ptx_Status_t ptx_NSC_SwitchBaudrate(ptx_NSC_t *nsc);
#endif

#if defined(PTX_ENABLE_I3C)
    static void ptx_NSC_InitI3CConfiguration(ptx_NSC_t *nsc, uint8_t *reg);
#endif


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_InitHw(ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_InitHw");

        status = ptx_NSC_CheckRfStateCmdExecution(nsc, ptx_NSC_SM_CmdId_Init, NULL);
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        uint8_t idx = 0;
        uint8_t buffer[PTX_NSC_INIT_HW_CMD_LENGTH];
        memset(buffer, 0, PTX_NSC_INIT_HW_CMD_LENGTH);

        ptx_NSC_PllConfig_t pll_config;
        memset(&pll_config, 0x00, sizeof(ptx_NSC_PllConfig_t));
        status = ptx_NSC_CalculatePllParameters(nsc, &(nsc->SysCfg), &pll_config);
        if (PTX_STATUS_SUCCESS == status)
        {
            buffer[idx] = (uint8_t)PTX_NSC_INIT_HW_CMD_OPCODE;
            idx++;

            /** CLK_SEL: 1By */
            buffer[idx] = nsc->SysCfg.ClkSel;
            idx++;

            /** CLK_REQ_EN: 1By */
            buffer[idx] = nsc->SysCfg.ClkReqEn;
            idx++;
            /** CLK_REQ_GT: 1By */
            buffer[idx] = nsc->SysCfg.ClkReqGt;
            idx++;
            /** XTAL_TRIM: 1By */
            buffer[idx] = nsc->SysCfg.XtalTrim;
            idx++;

            /** XTAL_DIV2_EN: 1By */
            buffer[idx] = pll_config.XtalDiv2En;
            idx++;

            /** GATE_PW: 2By */
            buffer[idx] = (uint8_t) (pll_config.GatePw >> 8);
            idx++;
            buffer[idx] = (uint8_t) (pll_config.GatePw >> 0);
            idx++;

            /** SYS_CNT_TG: 2By */
            buffer[idx] = (uint8_t) (pll_config.SysCntTg >> 8);
            idx++;
            buffer[idx] = (uint8_t) (pll_config.SysCntTg >> 0);
            idx++;

            /** NHOST: 4By */
            buffer[idx] = (uint8_t) (pll_config.NHost >> 24);
            idx++;
            buffer[idx] = (uint8_t) (pll_config.NHost >> 16);
            idx++;
            buffer[idx] = (uint8_t) (pll_config.NHost >> 8);
            idx++;
            buffer[idx] = (uint8_t) (pll_config.NHost >> 0);
            idx++;

            /** SEED: 4By */
            buffer[idx] = (uint8_t) (nsc->SysCfg.PrngSeed >> 24);
            idx++;
            buffer[idx] = (uint8_t) (nsc->SysCfg.PrngSeed >> 16);
            idx++;
            buffer[idx] = (uint8_t) (nsc->SysCfg.PrngSeed >> 8);
            idx++;
            buffer[idx] = (uint8_t) (nsc->SysCfg.PrngSeed >> 0);
            idx++;

            /** NHOST_ALM: 4By */
            buffer[idx] = (uint8_t) (pll_config.NHostAlm >> 24);
            idx++;
            buffer[idx] = (uint8_t) (pll_config.NHostAlm >> 16);
            idx++;
            buffer[idx] = (uint8_t) (pll_config.NHostAlm >> 8);
            idx++;
            buffer[idx] = (uint8_t) (pll_config.NHostAlm >> 0);
            idx++;

            /** NALM_MAX: 4By */
            buffer[idx] = (uint8_t) (nsc->SysCfg.NalmMax >> 24);
            idx++;
            buffer[idx] = (uint8_t) (nsc->SysCfg.NalmMax >> 16);
            idx++;
            buffer[idx] = (uint8_t) (nsc->SysCfg.NalmMax >> 8);
            idx++;
            buffer[idx] = (uint8_t) (nsc->SysCfg.NalmMax >> 0);
            idx++;

            /** NALM_MIN: 4By */
            buffer[idx] = (uint8_t) (nsc->SysCfg.NalmMin >> 24);
            idx++;
            buffer[idx] = (uint8_t) (nsc->SysCfg.NalmMin >> 16);
            idx++;
            buffer[idx] = (uint8_t) (nsc->SysCfg.NalmMin >> 8);
            idx++;
            buffer[idx] = (uint8_t) (nsc->SysCfg.NalmMin >> 0);
            idx++;


            uint8_t uart_cfg_0 = 0;
            uint8_t uart_cfg_1 = 0;

#if defined(PTX_ENABLE_UART)
            if (HAL_Interface_Type_UART == nsc->HAL->Interface.Type)
            {
                status = ptx_NSC_InitUartConfiguration(nsc, &uart_cfg_0, &uart_cfg_1);
            }
#endif
            /** UART_CONFIG_0: 1By */
            buffer[idx] = uart_cfg_0;
            idx++;

            /** UART_CONFIG_1: 1By */
            buffer[idx] = uart_cfg_1;
            idx++;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t i3c_ibi_en = 0;
#if defined(PTX_ENABLE_I3C)
            ptx_NSC_InitI3CConfiguration(nsc, &i3c_ibi_en);
#endif
            /** I3C_IBI_EN: 1By */
            buffer[idx] = i3c_ibi_en;
            idx++;

            /** TSENS_TEMP_ERR_TH: 1By */
            buffer[idx] = nsc->SysCfg.TempErrTreshold;
            idx++;

            /** TX_CURLIM_GAIN: 1By */
            buffer[idx] = nsc->SysCfg.TxCurlimGain;
            idx++;

            status = ptx_NSC_CmdProcessor_DispatchMsg(nsc, PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0, buffer, idx);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t rsp_buffer[PTX_NSC_CMD_PROCESSOR_INIT_RSPLEN];
            uint32_t rsp_buffer_len = PTX_NSC_CMD_PROCESSOR_INIT_RSPLEN;
            memset(rsp_buffer, 0, sizeof(uint8_t)*PTX_NSC_CMD_PROCESSOR_INIT_RSPLEN);

            status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rsp_buffer, &rsp_buffer_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_INIT_RSP);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_CmdProcessor_ProcessRsp(nsc, ptx_NSC_CmdProcessor_RspId_Init, rsp_buffer, rsp_buffer_len, NULL);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                if (NSC_ErrorCode_NoErr == rsp_buffer[PTX_NSC_RSP_ERRCODE_INDEX])
                {
                    nsc->State = ptx_NSC_State_Idle;
#if defined(PTX_ENABLE_UART)
                    if (HAL_Interface_Type_UART == nsc->HAL->Interface.Type)
                    {
                        status = ptx_NSC_SwitchBaudrate(nsc);
                    }
#endif
                }
                else
                {
                    status = PTX_STATUS_E_NSC_PROTOCOL;
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_CalculatePllParameters(ptx_NSC_t *nsc, ptx_NSC_SystemConfig_t *sysCfg, ptx_NSC_PllConfig_t *pllSettings)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != sysCfg) && (NULL != pllSettings) &&
        (PTX_PLL_MIN_REFFREQ <= sysCfg->RefFreq) && (PTX_PLL_MAX_REFFREQ >= sysCfg->RefFreq))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_CalculatePllParameters");

        uint32_t internal_fref = sysCfg->RefFreq;

        /* Calculate the coarse tuner configuration values based on the reference clock frequency. */
        if (PTX_PLL_MAX_INT_FREQ < sysCfg->RefFreq)
        {
            pllSettings->XtalDiv2En = 1;
            internal_fref = internal_fref / 2;
        }

        /*
         * Calculate the internal VCO frequency.
         */
        float_t fvco_tg = (float_t) (sysCfg->TargetFreq * 256U);

        /*
         * Calculate the divider ratio.
         * NHOST = (256 * F_TARGET / F_REF)
         * FVCO_TG = 256 * F_TARGET
         * GATE_PW = round( (64 * F_REF) / (1E6 * 2 ^ XTAL_DIV_EN) )
         */
        float_t div = (fvco_tg / ((float_t) internal_fref));
        pllSettings->NHost = ((uint32_t) (div * (1 << 22)));
        pllSettings->NHostAlm = (uint32_t)(pllSettings->NHost + sysCfg->NAlmOffset);
        pllSettings->GatePw = (uint16_t) ((PTX_PLL_FREQ_DIVIDER * (internal_fref / PTX_PLL_FRES)) + 0.5);

        /*
         * SYS_CNT_TG = round( (GATE_PW * 54.24e6 * 2 ^ XTAL_DIV_EN) / F_REF );
         * Rounding --> add 0.5 and cast to int for rounding.
         * Do the calculation in floating point arithmetics.
         */
        div = (float_t)((float_t)(div * (float_t)pllSettings->GatePw / (float_t)PTX_PLL_FREQ_DIVIDER) + 0.5);
        pllSettings->SysCntTg = (uint16_t) div;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */

#if defined(PTX_ENABLE_I3C)
static void ptx_NSC_InitI3CConfiguration(ptx_NSC_t *nsc, uint8_t *reg)
{
    PTX_UNUSED_PARAMETER(nsc);

    if ( NULL != reg )
    {
        *reg = I3C_IBI_INTERRUPTS_DISABLED;
    }
}
#endif

#if defined(PTX_ENABLE_UART)
static ptx_Status_t ptx_NSC_InitUartConfiguration(ptx_NSC_t *nsc, uint8_t *reg0, uint8_t *reg1)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != reg0) && (NULL != reg1))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_InitUartConfiguration");

        status = PTX_STATUS_E_GEN_INVALID_BITRATE;

        /** If NscSpeed is set to 0, use the default UART application speed. */
        uint32_t req_baudrate = (0 == nsc->NscSpeed) ? PTX_UART_DEFAULT_NSC_SPEED : nsc->NscSpeed;

        for (uint8_t idx = 0; idx < UART_CUSTOM_BR_NUM; idx++)
        {
            if (sUartRegValue[idx].uartBaudrate == req_baudrate)
            {
                /** Baud-rate settings found in the array. Update output registers. */
                *reg1 = (uint8_t)(((0 != sUartRegValue[idx].uart_dither_en) ? UART_BR_DITHER_EN_MASK : 0) |
                                  (sUartRegValue[idx].uart_br1_div & UART_BR_DIV1_VAL_MASK));
                *reg0 = (uint8_t)(sUartRegValue[idx].uart_br0_div & UART_BR_DIV0_VAL_MASK);

                status = PTX_STATUS_SUCCESS;

                break;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_SwitchBaudrate(ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_SwitchBaudrate");

        /** If NscSpeed is set to 0, use the default UART application speed. */
        uint32_t speed = (0 == nsc->NscSpeed) ? PTX_UART_DEFAULT_NSC_SPEED : nsc->NscSpeed;

        ptx_HAL_DRV_Interface_Parameter_t hal_param;
        hal_param.type = HAL_DRV_Interface_Parameter_Type_Transmission_Speed;
        hal_param.value.TransmissionSpeed = speed;
        status = ptx_HAL_Interface_SetParameter(nsc->HAL, &hal_param);
    }

    return status;
}

#endif
