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
    Module      : NFC Soft Controller (NSC) Interface Header for Register Access
    File        : ptx_NSC_Registers.h

    Description : Definition of NFC Soft Controller Interface for Register Access.
                  Contains register description data used to access NSC registers.
*/

/**
 * \addtogroup NSC_REGISTERS
 *
 * @{
 */

#if !defined(PTX_NSC_REGISTERS_H)
#define PTX_NSC_REGISTERS_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name Defines for SYS_WU_CONFIG1_REG.
 * @{
 */
#define SYS_WU_CONFIG1_REG                                          (0x0004)
#define SYS_WU_CONFIG1_REG_RST                                      (0x00)
#define SYS_WU_CONFIG1_REG_WU_GPIO_EN_MASK                          (0x04)
/** @} */

/**
 * \name Defines for SYS_CLOCK_CONTROL0_REG.
 * @{
 */
#define SYS_CLOCK_CONTROL0_REG                                      (0x0031)
#define SYS_CLOCK_CONTROL0_REG_CLOCK_TSENS_EN_MASK                  (0x40)
#define SYS_CLOCK_CONTROL0_REG_CLOCK_TX_EN_MASK                     (0x10)
/** @} */

/**
 * \name Defines for SYS_CLOCK_CONTROL1_REG.
 * @{
 */
#define SYS_CLOCK_CONTROL1_REG                                      (0x0032)
#define SYS_CLOCK_CONTROL1_REG_RST                                  (0x08)
#define SYS_CLOCK_CONTROL1_REG_CLOCK_REF_GPIO_EN_MASK               (0x20)
#define SYS_CLOCK_CONTROL1_REG_CLOCK_TSENS_RATE_EN_MASK             (0x08)
#define SYS_CLOCK_CONTROL1_REG_CLOCK_LPADC_EN_MASK                  (0x01)
/** @} */

/**
 * \name Defines for SYS_DFT_TB_CONTROL_REG.
 * @{
 */
#define SYS_DFT_TB_CONTROL_REG                                      (0x0113)
#define SYS_DFT_TB_CONTROL_REG_RST                                  (0x00)
#define SYS_DFT_TB_CONTROL_REG_TB0_BIT_SWAP_SEL_MASK                (0x30)
#define SYS_DFT_TB_CONTROL_REG_TB1_BIT_SWAP_SEL_MASK                (0xC0)
#define SYS_DFT_TB_CONTROL_REG_TB_CLOCK_SEL_MASK                    (0x0F)
/** @} */

/**
 * \name Defines for SYS_DFT_TB0_SEL_REG.
 * @{
 */
#define SYS_DFT_TB0_SEL_REG                                         (0x0111)
#define SYS_DFT_TB0_SEL_REG_RST                                     (0x00)
#define SYS_DFT_TB0_SEL_REG_TB_SWAP_EN_MASK                         (0x80)
/** @} */

/**
 * \name Defines for SYS_DFT_TB1_SEL_REG.
 * @{
 */
#define SYS_DFT_TB1_SEL_REG                                         (0x0112)
#define SYS_DFT_TB1_SEL_REG_RST                                     (0x00)
/** @} */




/**
 * \name General defines for all PAD_DIGX_XXX registers handling GPIOs.
 * @{
 */
/* If Set, 50ns input glitch filter is enabled. */
#define PAD_DIGX_IOX_REG_FS_MASK                                    (0x80u)     /**< GPIO Input Glitch Filter Mask.                         (bit 7)     */
#define PAD_DIGX_IOX_REG_FS_MASK_INV                                (0x7Fu)     /**< GPIO Input Glitch Filter Mask Inverted.                (bit 7)     */
#define PAD_DIGX_IOX_REG_FS_POS                                     (7u)        /**< FS Starting bit position.                                          */

/* Drive strength selection: '00': weakest [...] '11': strongest. */
#define PAD_DIGX_IOX_REG_DS_MASK                                    (0x60u)     /**< GPIO Drive Strength Mask.                              (bits 6-5)  */
#define PAD_DIGX_IOX_REG_DS_WKST_MASK                               (0x00u)     /**< GPIO Drive Strength Weakest.                           (bits 6-5)  */
#define PAD_DIGX_IOX_REG_DS_WK_MASK                                 (0x20u)     /**< GPIO Drive Strength Weak.                              (bits 6-5)  */
#define PAD_DIGX_IOX_REG_DS_ST_MASK                                 (0x40u)     /**< GPIO Drive Strength Strong.                            (bits 6-5)  */
#define PAD_DIGX_IOX_REG_DS_STST_MASK                               (0x60u)     /**< GPIO Drive Strength Strongest.                         (bits 6-5)  */
#define PAD_DIGX_IOX_REG_DS_MASK_INV                                (0x9Fu)     /**< GPIO Drive Strength Mask Inverted.                     (bits 6-5)  */
#define PAD_DIGX_IOX_REG_DS_POS                                     (5u)        /**< DS Starting bit position.                                          */

/* If set, the IO pad is in HighZ mode, else a pull resistor is connected (Resistor Enable Not). */
#define PAD_DIGX_IOX_REG_REN_MASK                                   (0x10u)     /**< GPIO Resistor Enable Not Mask  (1 = HiZ / 0 = Pull).   (bit 4)     */
#define PAD_DIGX_IOX_REG_REN_MASK_INV                               (0xEFu)     /**< GPIO Resistor Enable Not Inverted.                     (bit 4)     */
#define PAD_DIGX_IOX_REG_REN_POS                                    (4u)        /**< REN Starting bit position.                                         */

/* If set, PAD is configured as GPIO. */
#define PAD_DIGX_IOX_REG_GPIO_MASK                                  (0x08)      /**< GPIO GPIO Config.                                      (bit 3)     */
#define PAD_DIGX_IOX_REG_GPIO_MASK_INV                              (0xF7u)     /**< GPIO GPIO Config Inverted.                             (bit 3)     */
#define PAD_DIGX_IOX_REG_GPIO_POS                                   (3u)        /**< GPIO Starting bit position.                                        */

/* If set, IO pad is an input else output (Output Enable Not). */
#define PAD_DIGX_IOX_REG_OEN_MASK                                   (0x04u)     /**< GPIO Output Enable Not     (1 = Input / 0 = Output).   (bit 2)     */
#define PAD_DIGX_IOX_REG_OEN_MASK_INV                               (0xFBu)     /**< GPIO Output Enable Not Inverted.                       (bit 2)     */
#define PAD_DIGX_IOX_REG_OEN_POS                                    (2u)        /**< OEN Starting bit position.                                         */

/* Input from Digital IO to digital core (Read Only). */
#define PAD_DIGX_IOX_REG_C_MASK                                     (0x02u)     /**< GPIO Input From Digital IO to digital core.            (bit 1)     */
#define PAD_DIGX_IOX_REG_C_MASK_INV                                 (0xFDu)     /**< GPIO Input From Digital IO to digital core Inverted.   (bit 1)     */
#define PAD_DIGX_IOX_REG_C_POS                                      (1u)        /**< C Starting bit position.                                           */

/* Output from digital core to Digital IO. */
#define PAD_DIGX_IOX_REG_I_MASK                                     (0x01u)     /**< GPIO Output from digital core to Digital IO.           (bit 0)     */
#define PAD_DIGX_IOX_REG_I_MASK_INV                                 (0xFEu)     /**< GPIO Output from digital core to Digital IO Inverted.  (bit 0)     */
#define PAD_DIGX_IOX_REG_I_POS                                      (0u)        /**< I Starting bit position.                                           */
/** @} */

/**
 * \name Defines for PAD_DIG1_IO1_REG.
 * @{
 */
#define PAD_DIG1_IO1_REG                                            (0x0013)
#define PAD_DIG1_IO1_REG_RST                                        (0x06)
/** @} */

/**
 * \name Defines for PAD_DIG2_IO1_REG.
 * @{
 */
#define PAD_DIG2_IO1_REG                                            (0x00DA)
#define PAD_DIG2_IO1_REG_RST                                        (0x0E)
/** @} */

/**
 * \name Defines for PAD_DIG2_IO1_REG.
 * @{
 */
#define PAD_DIG3_IO1_REG                                            (0x00DB)
#define PAD_DIG3_IO1_REG_RST                                        (0x06)
/** @} */

/**
 * \name Defines for PAD_DIG4_IO2_REG.
 * @{
 */
#define PAD_DIG4_IO2_REG                                            (0x0014)
#define PAD_DIG4_IO2_REG_RST                                        (0x0E)
/** @} */

/**
 * \name Defines for PAD_DIG5_IO2_REG.
 * @{
 */
#define PAD_DIG5_IO2_REG                                            (0x00DC)
#define PAD_DIG5_IO2_REG_RST                                        (0x04)
/** @} */

/**
 * \name Defines for PAD_DIG6_IO2_REG.
 * @{
 */
#define PAD_DIG6_IO2_REG                                            (0x0015)
#define PAD_DIG6_IO2_REG_RST                                        (0x0E)
/** @} */

/**
 * \name Defines for PAD_DIG7_IO2_REG.
 * @{
 */
#define PAD_DIG7_IO2_REG                                            (0x00DD)
#define PAD_DIG7_IO2_REG_RST                                        (0x0E)
/** @} */

/**
 * \name Defines for PAD_DIG8_IO2_REG.
 * @{
 */
#define PAD_DIG8_IO2_REG                                            (0x00DE)
#define PAD_DIG8_IO2_REG_RST                                        (0x0E)
/** @} */


/**
 * \name Defines for PAD_TMUX_CONTROL0_REG.
 * @{
 */
#define PAD_TMUX_CONTROL0_REG                                       (0x00E5)
#define PAD_TMUX_CONTROL0_REG_RST                                   (0x00)
#define PAD_TMUX_CONTROL0_REG_DIG8_TB_EN_MASK                       (0x80)
#define PAD_TMUX_CONTROL0_REG_DIG7_TB_EN_MASK                       (0x40)
#define PAD_TMUX_CONTROL0_REG_DIG6_TB_EN_MASK                       (0x20)
#define PAD_TMUX_CONTROL0_REG_DIG5_TB_EN_MASK                       (0x10)
#define PAD_TMUX_CONTROL0_REG_DIG4_TB_EN_MASK                       (0x08)
#define PAD_TMUX_CONTROL0_REG_DIG3_TB_EN_MASK                       (0x04)
#define PAD_TMUX_CONTROL0_REG_DIG2_TB_EN_MASK                       (0x02)
#define PAD_TMUX_CONTROL0_REG_DIG1_TB_EN_MASK                       (0x01)
/** @} */




/**
 * \name Defines for AUX_IF_CONFIG0_REG.
 * @{
 */
#define AUX_IF_CONFIG0_REG                                          (0x001D)
#define AUX_IF_CONFIG0_REG_RST                                      (0x00)
#define AUX_IF_CONFIG0_REG_AUX_IF_ENABLE_MASK                       (0x80)
/** @} */




/**
 * \name Defines for CL_CONTROL_REG.
 * @{
 */
#define CL_CONTROL_REG                                              (0x0070)
/** @} */

/**
 * \name Defines for CL_RF_FIELD_REG.
 * @{
 */
#define CL_RF_FIELD_REG                                             (0x0084)
#define CL_RF_FIELD_REG_RFF_ACT_EN_MASK                             (0x01)
/** @} */

/**
 * \name Defines for CL_RX_CONFIG1_REG.
 * @{
 */
#define CL_RX_CONFIG1_REG                                           (0x008Eu)
#define CL_RX_CONFIG1_REG_RX_B_PRIME_EN_MASK                        (0x04u)
#define CL_RX_CONFIG1_REG_RX_B_PRIME_EN_MASK_INV                    (0xFBu)
#define CL_RX_CONFIG1_REG_RX_SOF_ONLY_EN_MASK                       (0x02U)
#define CL_RX_CONFIG1_REG_RX_SOF_ONLY_EN_MASK_INV                   (0xFDU)
#define CL_RX_CONFIG1_REG_RX_SKIP_1ST_CRC_BYTE_EN_MASK              (0x80U)
#define CL_RX_CONFIG1_REG_RX_SKIP_1ST_CRC_BYTE_EN_MASK_INV          (0x7FU)
#define CL_RX_CONFIG1_REG_RX_SKIP_1ST_CRC_BYTE_EN_MASK_RST          (0x00U)
/** @} */

/**
 * \name Defines for CL_TX_CONFIG1_REG
 * @{
 */
#define CL_TX_CONFIG1_REG                                           (0x0089U)
#define CL_TX_CONFIG1_REG_RST                                       (0x00U)
#define CL_TX_CONFIG1_REG_RX_SKIP_1ST_CRC_BYTE_EN_MASK              (0x80U)
#define CL_TX_CONFIG1_REG_RX_SKIP_1ST_CRC_BYTE_EN_MASK_INV          (0x7FU)
#define CL_TX_CONFIG1_REG_RX_SKIP_1ST_CRC_BYTE_EN_MASK_RST          (0x00U)
/**
 * \name Defines for CL_DRX_RD_TH0_REG.
 * @{
 */
#define CL_DRX_RD_TH0_REG                                           (0x00BF)
/** @} */

/**
 * \name Defines for CL_DRX_RD_TH1_REG.
 * @{
 */
#define CL_DRX_RD_TH1_REG                                           (0x00C0)
/** @} */

/**
 * \name Defines for CL_DRX_DBG_DAC0_SEL_REG.
 * @{
 */
#define CL_DRX_DBG_DAC0_SEL_REG                                     (0x00CE)
/** @} */

/**
 * \name Defines for CL_DRX_DBG_SCAL_SEL_REG.
 * @{
 */
#define CL_DRX_DBG_SCAL_SEL_REG                                     (0x00D1)
/** @} */

/**
 * \name Defines for CL_DRX_RD_TH2_REG.
 * @{
 */
#define CL_DRX_RD_TH2_REG                                           (0x00D7)
/** @} */

/**
 * \name Defines for CL_DRX_DF_CTRL_REG.
 * @{
 */
#define CL_DRX_DF_CTRL_REG                                          (0x00D8)
/** @} */




/**
 * \name Defines for TX_WAVEGEN_CONFIG_REG.
 * @{
 */
#define TX_WAVEGEN_CONFIG_REG                                       (0x00A4)
#define TX_WAVEGEN_CONFIG_REG_TX_WAVEGEN_EN_MASK                    (0x01)
/** @} */

/**
 * \name Defines for TX_WAVE_CW_REG.
 * @{
 */
#define TX_WAVE_CW_REG                                              (0x00A5)
/** @} */




/**
 * \name Defines for TSENS_DECIM_REG.
 * @{
 */
#define TSENS_DECIM_REG                                             (0x00EA)
#define TSENS_DECIM_REG_RST                                         (0x00)
#define TSENS_DECIM_REG_TSENS_EN_MASK                               (0x01)
#define TSENS_DECIM_REG_TSENS_TEST_EN_MASK                          (0x02)
/** @} */




/**
 * \name Defines for TX_WAVEGEN_CONFIG_REG.
 * @{
 */
#define ANA_TUNE_REG_ADDRESS                                        (0x001F)
#define ANA_TUNE_REG_TUNE_MASK                                      (0x1F)
#define ANA_TUNE_REG_TUNE_MASK_INV                                  (0xE0)
#define ANA_TUNE_REG_TUNE_MAX_VALUE                                 (0x1F)
/** @} */

/**
 * \name Defines for ANA_TX_PA_CONTROL_REG.
 * @{
 */
#define ANA_TX_PA_CONTROL_REG                                       (0x0023)
/** @} */

/**
 * \name Defines for ANA_RX_CONTROL0_REG.
 * @{
 */
#define ANA_RX_CONTROL0_REG                                         (0x0119)
/** @} */

/**
 * \name Defines for ANA_RX_CONTROL1_REG.
 * @{
 */
#define ANA_RX_CONTROL1_REG                                        (0x011A)
#define ANA_RX_CONTROL1_REG_RST                                    (0x00)
/** @} */

/**
 * \name Defines for ANA_RX_FE_CONFIG_REG.
 * @{
 */
#define ANA_RX_FE_CONFIG_REG                                        (0x011B)
/** @} */

/**
 * \name Defines for ANA_RX_BB_CONFIG_REG.
 * @{
 */
#define ANA_RX_BB_CONFIG_REG                                        (0x011C)
/** @} */

/**
 * \name Defines for ANA_RX_BUF_REG.
 * @{
 */
#define ANA_RX_BUF_REG                                              (0x0120)
#define ANA_RX_BUF_REG_RST                                          (0x00)
/** @} */

/**
 * \name Defines for ANA_RX_LPADC_CONTROL_REG.
 * @{
 */
#define ANA_RX_LPADC_CONTROL_REG                                    (0x0124)
#define ANA_RX_LPADC_CONTROL_REG_RX_LPADCQ_DIG_EN_MASK              (0x08)
#define ANA_RX_LPADC_CONTROL_REG_RX_LPADCI_DIG_EN_MASK              (0x04)
#define ANA_RX_LPADC_CONTROL_REG_RX_LPADCQ_COMP_EN_MASK             (0x02)
#define ANA_RX_LPADC_CONTROL_REG_RX_LPADCI_COMP_EN_MASK             (0x01)
/** @} */

/**
 * \name Defines for ANA_RX_LPADC_PP_CTRL_REG.
 * @{
 */
#define ANA_RX_LPADC_PP_CTRL_REG                                    (0x0127)
#define ANA_RX_LPADC_PP_CTRL_REG_LPADC_PP_EN_MASK                   (0x01)
/** @} */

/**
 * \name Defines for ANA_RX_LPADC_PP_CTRL2_REG.
 * @{
 */
#define ANA_RX_LPADC_PP_CTRL2_REG                                   (0x0128)
#define ANA_RX_LPADC_PP_CTRL2_REG_RST                               (0x00)
/** @} */

/**
 * \name Defines for ANA_RX_LPADC_PP_OUT_REG.
 * @{
 */
#define ANA_RX_LPADC_PP_OUT_REG                                     (0x012A)
/** @} */

/**
 * \name Defines for ANA_TX_PA_REG.
 * @{
 */
#define ANA_TX_PA_REG                                               (0x012B)
#define ANA_TX_PA_REG_TX_PA_BIAS_EN_MASK                            (0x10)
#define ANA_TX_PA_REG_TX_PA_EN_MASK                                 (0x03)
/** @} */

/**
 * \name Defines for ANA_TX_CURLIM_CTRL_REG.
 * @{
 */
#define ANA_TX_CURLIM_CTRL_REG                                      (0x012C)
#define ANA_TX_CURLIM_CTRL_REG_TX_CURLIM_EN_MASK                    (0x01)
/** @} */

/**
 * \name Defines for ANA_RX_TMUXA_REG.
 * @{
 */
#define ANA_RX_TMUXA_REG                                            (0x01D7)
#define ANA_RX_TMUXA_REG_RST                                        (0x00)
/** @} */

/**
 * \name Defines for ANA_RX_TEST_REG.
 * @{
 */
#define ANA_RX_TEST_REG                                             (0x01D9)
#define ANA_RX_TEST_REG_RST                                         (0x00)
#define ANA_RX_TEST_REG_RX_LDO_TST_EN_MASK                          (0x01)
#define ANA_RX_TEST_REG_RX_REF_TST_EN_MASK                          (0x04)
#define ANA_RX_TEST_REG_RX_CLOCKEXT_TST_EN_MASK                     (0x08)
#define ANA_RX_TEST_REG_RX_IBANK_TST_EN_MASK                        (0x40)
#define ANA_RX_TEST_REG_RX_VMIX_TST_EN_MASK                         (0x20)
/** @} */

/**
 * \name Defines for ANA_RX_TMUXB_REG.
 * @{
 */
#define ANA_RX_TMUXB_REG                                            (0x01D8)
#define ANA_RX_TMUXB_REG_RST                                        (0x00)
/** @} */

/**
 * \name Defines for ANA_SUPPLY_TEST_REG.
 * @{
 */
#define ANA_SUPPLY_TEST_REG                                         (0x01DE)
#define ANA_SUPPLY_TEST_REG_RST                                     (0x00)
#define ANA_SUPPLY_TEST_REG_BIAS_TEST_EN_MASK                       (0x01)
#define ANA_SUPPLY_TEST_REG_HPLDO_TEST_EN_MASK                      (0x02)
#define ANA_SUPPLY_TEST_REG_LDO2V5_TEST_EN_MASK                     (0x04)
#define ANA_SUPPLY_TEST_REG_VDPA_TEST_EN_MASK                       (0x10)
#define ANA_SUPPLY_TEST_REG_VBAT_TEST_EN_MASK                       (0x20)
#define ANA_SUPPLY_TEST_REG_VDDIO1_LDO_TEST_EN_MASK                 (0x40)
#define ANA_SUPPLY_TEST_REG_VDDIO2_LDO_TEST_EN_MASK                 (0x80)
/** @} */

/**
 * \name Defines for ANA_PMU_TEST_REG.
 * @{
 */
#define ANA_PMU_TEST_REG                                            (0x01DD)
#define ANA_PMU_TEST_REG_RST                                        (0x00)
#define ANA_PMU_TEST_REG_PMU_TEST_SEL0_MASK                         (0x0F)
#define ANA_PMU_TEST_REG_PMU_TEST_SEL1_MASK                         (0xF0)
/** @} */

/**
 * \name Defines for ANA_SWP_TEST1_REG.
 * @{
 */
#define ANA_SWP_TEST1_REG                                           (0x01DC)
#define ANA_SWP_TEST1_REG_RST                                       (0x00)
#define ANA_SWP_TEST1_REG_SWP_LDO_TEST_EN_MASK                      (0x01)
/** @} */


/**
 * \name Defines for ANA_TEST_BUF_REG.
 * @{
 */
#define ANA_TEST_BUF_REG                                            (0x01DF)
#define ANA_TEST_BUF_REG_RST                                        (0x00)
#define ANA_TEST_BUF_REG_ANATEST_BUF_EN_MASK                        (0x30)
#define ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK                    (0x0F)
#define ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD4               (0x01)
#define ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD6               (0x02)
#define ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD7               (0x04)
#define ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD8               (0x08)
/** @} */


/**
 * \name Defines for ANA_TEST_SEL0_REG.
 * @{
 */
#define ANA_TEST_SEL0_REG                                           (0x01E0)
#define ANA_TEST_SEL0_REG_RST                                       (0x00)
/** @} */

/**
 * \name Defines for ANA_TEST_SEL1_REG.
 * @{
 */
#define ANA_TEST_SEL1_REG                                           (0x01E1)
#define ANA_TEST_SEL1_REG_RST                                       (0x00)
/** @} */

/**
 * \name Defines for ANA_TEST_SEL2_REG.
 * @{
 */
#define ANA_TEST_SEL2_REG                                           (0x01E2)
#define ANA_TEST_SEL2_REG_RST                                       (0x00)
/** @} */

/**
 * \name Defines for ANA_TEST_SEL3_REG.
 * @{
 */
#define ANA_TEST_SEL3_REG                                           (0x01E3)
#define ANA_TEST_SEL3_REG_RST                                       (0x00)
/** @} */

/**
 * \name Defines for ANA_TEST_SHORT_REG.
 * @{
 */
#define ANA_TEST_SHORT_REG                                          (0x01E4)
#define ANA_TEST_SHORT_REG_RST                                      (0x00)
/** @} */

/**
 * \name Defines for ANA_RX_ADC_TEST_REG.
 * @{
 */
#define ANA_RX_ADC_TEST_REG                                         (0x01E8)
#define ANA_RX_ADC_TEST_REG_RST                                     (0x00)
#define ANA_RX_ADC_TEST_REG_RX_ADC_BUF_LDO_TEST_SEL_I_MASK          (0x02)
#define ANA_RX_ADC_TEST_REG_RX_ADC_CORE_AMP_LDO_TEST_SEL_I_MASK     (0x04)
#define ANA_RX_ADC_TEST_REG_RX_ADC_TEST_EN_I_MASK                   (0x01)
#define ANA_RX_ADC_TEST_REG_RX_ADC_BUF_LDO_TEST_SEL_Q_MASK          (0x20)
#define ANA_RX_ADC_TEST_REG_RX_ADC_CORE_AMP_LDO_TEST_SEL_Q_MASK     (0x40)
#define ANA_RX_ADC_TEST_REG_RX_ADC_TEST_EN_Q_MASK                   (0x10)
/** @} */

/**
 * \name Defines for ANA_CLOCK_TEST_REG.
 * @{
 */
#define ANA_CLOCK_TEST_REG                                          (0x01E9)
#define ANA_CLOCK_TEST_REG_RST                                      (0x00)
#define ANA_CLOCK_TEST_REG_LFIN_TO_TEST_MASK                        (0x20)
#define ANA_CLOCK_TEST_REG_LFVS_TO_TEST_MASK                        (0x10)
#define ANA_CLOCK_TEST_REG_LDO_PLL_VCO_TEST_EN_MASK                 (0x01)
#define ANA_CLOCK_TEST_REG_LDO_PLL_ANA_TEST_EN_MASK                 (0x02)
#define ANA_CLOCK_TEST_REG_LDO_XTAL_TST_EN_MASK                     (0x40)
#define ANA_CLOCK_TEST_REG_PFD_TST_SEL_POS                          (0x03)
#define ANA_CLOCK_TEST_REG_XTAL_TEST_EN_MASK                        (0x80)
/** @} */

/**
 * \name Defines for ANA_TX_TEST_REG.
 * @{
 */
#define ANA_TX_TEST_REG                                             (0x01EA)
#define ANA_TX_TEST_REG_TX_TEST_SEL_MASK                            (0x03)
/** @} */




/**
 * \name Defines for CONTROL_ADC_I0_REG.
 * @{
 */
#define CONTROL_ADC_I0_REG                                          (0x017C)
/** @} */

/**
 * \name Defines for CONTROL_ADC_I1_REG.
 * @{
 */
#define CONTROL_ADC_I1_REG                                          (0x017D)
/** @} */

/**
 * \name Defines for CONTROL_ADC_Q0_REG.
 * @{
 */
#define CONTROL_ADC_Q0_REG                                          (0x01A9)
/** @} */

/**
 * \name Defines for CONTROL_ADC_Q1_REG.
 * @{
 */
#define CONTROL_ADC_Q1_REG                                          (0x01AA)
/** @} */




/**
 * \name Defines for DSCOPE_CTRL_CONTROL_REG.
 * @{
 */
#define DSCOPE_CTRL_CONTROL_REG                                     (0x010B)
/** @} */

/**
 * \name Defines for DSCOPE_CTRL_SRC_SEL_REG.
 * @{
 */
#define DSCOPE_CTRL_SRC_SEL_REG                                     (0x010C)
/** @} */

/**
 * \name Defines for DSCOPE_CTRL_STR_SEL_REG.
 * @{
 */
#define DSCOPE_CTRL_STR_SEL_REG                                     (0x010A)
/** @} */

/**
 * \name Defines for DSCOPE_CTRL_OFFSET_REG.
 * @{
 */
#define DSCOPE_CTRL_OFFSET_REG                                      (0x0104)
/** @} */

/**
 * \name Defines for DSCOPE_CTRL_MAX_LEN_REG.
 * @{
 */
#define DSCOPE_CTRL_MAX_LEN_REG                                     (0x0103)
/** @} */

/**
 * \name Defines for DSCOPE_CTRL0_LEN1_REG.
 * @{
 */
#define DSCOPE_CTRL0_LEN1_REG                                       (0x0107)
/** @} */

/**
 * \name Defines for DSCOPE_CTRL0_LEN0_REG.
 * @{
 */
#define DSCOPE_CTRL0_LEN0_REG                                       (0x0106)
/** @} */




/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
