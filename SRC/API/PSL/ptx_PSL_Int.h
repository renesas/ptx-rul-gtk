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
    File        : ptx_PSL_Int.h

    Description : PSL specific data structures and enumerations.

*/

/**
 * \addtogroup PSL
 * @{
 */

#if !defined(PTX_PSLINT_H)
#define PTX_PSLINT_H

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
 *  \brief Internal parameters.
 */
#define PTX_PSL_AMP_MEAS_FACTOR         (1000U)     /**< Amplitude measurement is scaled by a factor of 1000. */

/**
 *  \brief Default Q-Measurement configuration values.
 */
#define PTX_PSL_QMEAS_CENTER_FREQ       (13560000U) /**< Q-measurement center frequency in Hz (= 13.56MHz). */
#define PTX_PSL_QMEAS_BUFFERSIZE        (1000U)     /**< Buffer size of uint32_t's for Q-measurement results. */
#define PTX_PSL_QMEAS_MAX_FREQ_SPAN     (5000000U)  /**< Max frequency span for Q-measurement (= 5MHz). */

/**
 *  \brief RAM module addresses
 */
#define DSCOPE_RAM0_READ_ADDR           (0x2000)    /**< Start address in RAM will be 0x2000.*/
#define DSCOPE_RAM1_READ_ADDR           (0x5E00)    /**< Start address in RAM will be 0x5E00.*/
#define DSCOPE_RAM1_FW_VERSION_ADDR     (0x5FFE)    /**< Address of the firmware version information in the CMEM.*/

/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/
/**
 * Anamux test signal values.
 */
typedef enum ptx_PSL_AnaMuxTestSignals
{
    TestSignal_Vs0                              = 0x0000,   /**< VS_0 */
    TestSignal_RxAdcBufLdoTestI                 = 0x0100,   /**< rx_adc_buf_ldo_test_i */
    TestSignal_RxAdcCoreAmpLdoTestI             = 0x0200,   /**< rx_adc_core_amp_ldo_test_i */
    TestSignal_RxAdcTestPI                      = 0x0300,   /**< RX_ADC_TEST_P_i */
    TestSignal_RxAdcTestNI                      = 0x0400,   /**< RX_ADC_TEST_N_i */
    TestSignal_RxAdcBufLdoTestQ                 = 0x0500,   /**< rx_adc_buf_ldo_test_q */
    TestSignal_RxAdcCoreAmpLdoTestQ             = 0x0600,   /**< rx_adc_core_amp_ldo_test_q */
    TestSignal_RxAdcTestPQ                      = 0x0700,   /**< RX_ADC_TEST_P_q */
    TestSignal_RxAdcTestNQ                      = 0x0800,   /**< RX_ADC_TEST_N_q */
    TestSignal_LfInTest                         = 0x0900,   /**< lfin_test */
    TestSignal_LfVsTest                         = 0x0A00,   /**< lfvs_test */
    TestSignal_LdoPllVcoVoutTest                = 0x0B00,   /**< ldo_pll_vco_vout_test */
    TestSignal_LdoPllAnaVoutTest                = 0x0C00,   /**< ldo_pll_ana_vout_test */
    TestSignal_LdoXtalVoutTest                  = 0x0D00,   /**< ldo_xtal_vout_test */
    TestSignal_RxLdoTest                        = 0x0E00,   /**< rx_ldo_tst */
    TestSignal_RxRefTest                        = 0x0F00,   /**< rx_ref_tst */
    TestSignal_RxClockExtLdoTst                 = 0x1000,   /**< rx_clockext_ldo_tst */
    TestSignal_RxA0TestTiaIp                    = 0x1100,   /**< rx_A0_tst_TIA_Ip */
    TestSignal_RxA1TestTiaIn                    = 0x1200,   /**< rx_A1_tst_TIA_In */
    TestSignal_RxA0TestDccIp                    = 0x1102,   /**< rx_A0_tst_DCC_Ip */
    TestSignal_RxA1TestDccIn                    = 0x1202,   /**< rx_A1_tst_DCC_In */
    TestSignal_RxA0TestBBinIp                   = 0x1101,   /**< rx_A0_tst_BBin_Ip */
    TestSignal_RxA1TestBBinIn                   = 0x1201,   /**< rx_A1_tst_BBin_In */
    TestSignal_RxA0TestBBoutIp                  = 0x1103,   /**< rx_A0_tst_BBout_Ip */
    TestSignal_RxA1TestBBoutIn                  = 0x1203,   /**< rx_A1_tst_BBout_In */
    TestSignal_RxA0TestBuffer                   = 0x1107,   /**< rx_A0_tst_Buffer */
    TestSignal_RxA1TestBuffer                   = 0x1207,   /**< rx_A1_tst_Buffer */
    TestSignal_RxB0TestTiaQp                    = 0x1300,   /**< rx_B0_tst_TIA_Qp */
    TestSignal_RxB1TestTiaQn                    = 0x1400,   /**< rx_B1_tst_TIA_Qn */
    TestSignal_RxB0TestDCCQp                    = 0x1302,   /**< rx_B0_tst_DCC_Qp */
    TestSignal_RxB1TestDCCQn                    = 0x1402,   /**< rx_B1_tst_DCC_Qn */
    TestSignal_RxB0TestBBinQp                   = 0x1301,   /**< rx_B0_tst_BBin_Qp */
    TestSignal_RxB1TestBBinQn                   = 0x1401,   /**< rx_B1_tst_BBin_Qn */
    TestSignal_RxB0TestBBoutQp                  = 0x1303,   /**< rx_B0_tst_BBout_Qp */
    TestSignal_RxB1TestBBoutQn                  = 0x1403,   /**< rx_B1_tst_BBout_Qn */
    TestSignal_RxB0TestBuffer                   = 0x1307,   /**< rx_B0_tst_Buffer */
    TestSignal_RxB1TestBuffer                   = 0x1407,   /**< rx_B1_tst_Buffer */
    TestSignal_RxIBankIbc5uTest                 = 0x1500,   /**< rx_ibank_ibc_5u_tst */
    TestSignal_RxVmixTest                       = 0x1600,   /**< rx_vmix_tst */
    TestSignal_PmuTestOut0LL                    = 0x1700,   /**< pmu_test_out0_LL */
    TestSignal_PmuTestOut0IbRef0u4Test          = 0x1701,   /**< pmu_test_out0_ib_ref_0u4_tst */
    TestSignal_PmuTestOut0Ib0u1TuneTest         = 0x1702,   /**< pmu_test_out0_ib0u1_tune_tst */
    TestSignal_PmuTestOut0Ib1uRefTest           = 0x1703,   /**< pmu_test_out0_ib1u_ref_tst */
    TestSignal_PmuTestOut0Ib1uTuneTest          = 0x1704,   /**< pmu_test_out0_ib1u_tune_tst */
    TestSignal_PmuTestOut0Ib20uTuneTest         = 0x1705,   /**< pmu_test_out0_ib20u_tune_tst */
    TestSignal_PmuTestOut0VrefTest              = 0x1706,   /**< pmu_test_out0_vref_tst */
    TestSignal_PmuTestOut0V2iHpVrefTest         = 0x1707,   /**< pmu_test_out0_v2i_hp_vref_tst */
    TestSignal_PmuTestOut0V2iHpVrefTuneTest     = 0x1708,   /**< pmu_test_out0_v2i_hp_vref_tune_tst */
    TestSignal_PmuTestOut0V2iLpVrefTest         = 0x1709,   /**< pmu_test_out0_v2i_lp_vref_tst */
    TestSignal_PmuTestOut0V2iLpVrefTuneTest     = 0x170A,   /**< pmu_test_out0_v2i_lp_vref_tune_tst */
    TestSignal_PmuTestOut0Ldo2v5OutTest         = 0x170B,   /**< pmu_test_out0_ldo2v5_out_test */
    TestSignal_PmuTestOut0HpLdoVoutTest         = 0x170C,   /**< pmu_test_out0_hpldo_vout_test */
    TestSignal_PmuTestOut0VsFromLpBias          = 0x170D,   /**< pmu_test_out0_VS_from_lpbias */
    TestSignal_PmuTestOut0LL14                  = 0x170E,   /**< pmu_test_out0_LL_14 */
    TestSignal_PmuTestOut0LL15                  = 0x170F,   /**< pmu_test_out0_LL_15 */
    TestSignal_PmuTestOut1LL                    = 0x1800,   /**< pmu_test_out1_LL */
    TestSignal_PmuTestOut1IbRef0u4Test          = 0x1801,   /**< pmu_test_out1_ib_ref_0u4_tst */
    TestSignal_PmuTestOut1Ib0u1TuneTest         = 0x1802,   /**< pmu_test_out1_ib0u1_tune_tst */
    TestSignal_PmuTestOut1Ib1uRefTest           = 0x1803,   /**< pmu_test_out1_ib1u_ref_tst */
    TestSignal_PmuTestOut1Ib1uTuneTest          = 0x1804,   /**< pmu_test_out1_ib1u_tune_tst */
    TestSignal_PmuTestOut1Ib20uTuneTest         = 0x1805,   /**< pmu_test_out1_ib20u_tune_tst */
    TestSignal_PmuTestOut1VrefTest              = 0x1806,   /**< pmu_test_out1_vref_tst */
    TestSignal_PmuTestOut1V2iHpVrefTest         = 0x1807,   /**< pmu_test_out1_v2i_hp_vref_tst */
    TestSignal_PmuTestOut1V2iHpVrefTuneTest     = 0x1808,   /**< pmu_test_out1_v2i_hp_vref_tune_tst */
    TestSignal_PmuTestOut1V2iLpVrefTest         = 0x1809,   /**< pmu_test_out1_v2i_lp_vref_tst */
    TestSignal_PmuTestOut1V2iLpVrefTuneTest     = 0x180A,   /**< pmu_test_out1_v2i_lp_vref_tune_tst */
    TestSignal_PmuTestOut1Ldo2v5OutTest         = 0x180B,   /**< pmu_test_out1_ldo2v5_out_test */
    TestSignal_PmuTestOut1HpLdoVoutTest         = 0x180C,   /**< pmu_test_out1_hpldo_vout_test */
    TestSignal_PmuTestOut1VsFromLpBias          = 0x180D,   /**< pmu_test_out1_VS_from_lpbias */
    TestSignal_PmuTestOut1LL14                  = 0x180E,   /**< pmu_test_out1_LL_14 */
    TestSignal_PmuTestOut1LL15                  = 0x180F,   /**< pmu_test_out1_LL_15 */
    TestSignal_SwpLdoTestOut                    = 0x1900,   /**< swp_ldo_test_out */
    TestSignal_HiZ26                            = 0x1A00,   /**< HiZ_26 */
    TestSignal_HiZ27                            = 0x1B00,   /**< HiZ_27 */
    TestSignal_TxTestHiZ                        = 0x1C00,   /**< tx_tst_HiZ */
    TestSignal_TxTest05VsPmos                   = 0x1C01,   /**< tx_tst_05_VSpmos */
    TestSignal_TxTest05VdNmos                   = 0x1C02,   /**< tx_tst_05_VDnmos */
    TestSignal_TxTestAmpOutIsense               = 0x1C03,   /**< tx_tst_amp_out_Isense */
    TestSignal_TsensTestOutN                    = 0x1D00,   /**< tsens_tst_outn */
    TestSignal_TsensTestOutP                    = 0x1E00,   /**< tsens_tst_outp */
    TestSignal_XtalTestOut                      = 0x1F00,   /**< xtal_test_out */
    TestSignal_HiZ                              = 0x2000,   /**< HiZ */
    TestSignal_RxDacOut0                        = 0x2100,   /**< rx_dac_out0 */
    TestSignal_RxDacOut1                        = 0x2200,   /**< rx_dac_out1 */
    TestSignal_TestAdc0DacOut                   = 0x2300,   /**< tst_adc0_dac_out */
    TestSignal_TestAdc1DacOut                   = 0x2400,   /**< tst_adc1_dac_out */
    TestSignal_ScaledVbat                       = 0x2500,   /**< scaled_vbat */
    TestSignal_ScaledVdpa                       = 0x2600,   /**< scaled_vdpa */
    TestSignal_PolyResistor                     = 0x2700,   /**< poly_resistor */
} ptx_PSL_AnaMuxTestSignals_t;

/**
 * Anamux ADC modes.
 */
typedef enum ptx_PSL_AnaTestADC
{
    AnaTestAdc0_Sar,            /**< ADC0_SAR */
    AnaTestAdc0_SarAttenuated,  /**< ADC0_SAR_Attenuated */
    AnaTestAdc0_ManualMode,     /**< ADC0_manualMode */
    AnaTestAdc1_Sar,            /**< ADC1_SAR */
    AnaTestAdc1_SarAttenuated,  /**< ADC1_SAR_Attenuated */
    AnaTestAdc1_ManualMode,     /**< ADC1_manualMode */
} ptx_PSL_AnaTestADC_t;

/**
 * Anamux test pins.
 */
typedef enum ptx_PSL_AnaTestPin
{
    AnaTestPin_Atst0Buff,   /**< ATST0_buff */
    AnaTestPin_Atst1Buff,   /**< ATST1_buff */
    AnaTestPin_Atst0,       /**< ATST0 */
    AnaTestPin_Atst1,       /**< ATST1 */
    AnaTestPin_Atst2,       /**< ATST2 */
    AnaTestPin_Atst3,       /**< ATST3 */
} ptx_PSL_AnaTestPin_t;

/**
 * TB0 pin swap types.
 */
typedef enum ptx_PSL_TbSwapType
{
    SwapType_Direct,        /**< Directly, no swapping */
    SwapType_Nibbles,       /**< Full nibble swap*/
    SwapType_TuplePerNibble,/**< Tuple per nibble swap*/
    SwapType_InvTuple,      /**< Inverse tuple swapping */
} ptx_PSL_TbSwapType_t;

/**
 * TB0 test signals.
 */
typedef enum ptx_PSL_Tb0TestSignals
{
    Tb0_TbOff            = 0x00, /**< TB0_TB_Off */
    Tb0_DrxDbgDac_15_8   = 0x01, /**< TB0_drx_dbg_dac_15_8 */
    Tb0_DrxDbgBus        = 0x02, /**< TB0_drx_dbg_bus */
    Tb0_RxfdDbg0         = 0x03, /**< TB0_rxfd_dbg0 */
    Tb0_TxfgDbg0         = 0x04, /**< TB0_txfg_dbg0 */
    Tb0_ComDbg0          = 0x05, /**< TB0_com_dbg0 */
    Tb0_DmDbg0           = 0x06, /**< TB0_dm_dbg0 */
    Tb0_RffmDbg0         = 0x07, /**< TB0_rffm_dbg0 */
    Tb0_CltDbg0          = 0x08, /**< TB0_clt_dbg0 */
    Tb0_UartDbg0         = 0x09, /**< TB0_uart_dbg0 */
    Tb0_AdcSampleIch     = 0x0A, /**< TB0_adc_sample_ich */
    Tb0_PxiDbg0          = 0x0B, /**< TB0_pxi_dbg0 */
    Tb0_PxiDbg1          = 0x0C, /**< TB0_pxi_dbg1 */
    Tb0_PxiDbg2          = 0x0D, /**< TB0_pxi_dbg2 */
    Tb0_PxiDbg3          = 0x0E, /**< TB0_pxi_dbg3 */
    Tb0_DfyDbg0          = 0x10, /**< TB0_dfy_dbg0 */
    Tb0_AnaLpAdcIchDbg   = 0x11, /**< TB0_ana_lpadc_ich_dbg */
    Tb0_AnaDbg0          = 0x20, /**< TB0_ana_dbg0 */
    Tb0_AnaDbg1          = 0x21, /**< TB0_ana_dbg1 */
    Tb0_AnaDbg2          = 0x22, /**< TB0_ana_dbg2 */
    Tb0_AnaDbg3          = 0x23, /**< TB0_ana_dbg3 */
    Tb0_AnaDbg4          = 0x24, /**< TB0_ana_dbg4 */
    Tb0_BlmDbg0          = 0x25, /**< TB0_blm_dbg0 */
    Tb0_TsensDbg0        = 0x26, /**< TB0_tsens_dbg0 */
    Tb0_PllCtDbg0        = 0x27, /**< TB0_pll_ct_dbg0 */
    Tb0_HifTbI3C1        = 0x28, /**< TB0_hif_tb_i3c1 */
    Tb0_AuxIfDbgI3C1     = 0x29, /**< TB0_aux_if_dbg_i3c1 */
    Tb0_CmemCtrlDbg0     = 0x2a, /**< TB0_cmem_ctrl_dbg0 */
    Tb0_SwpMuxDbg0       = 0x2b, /**< TB0_swp_mux_dbg0 */
    Tb0_AuxIfCtrlDbg0    = 0x2c, /**< TB0_aux_if_ctrl_dbg0 */
    Tb0_AnaCtrlTopDbg0   = 0x2d, /**< TB0_ana_ctrl_top_dbg0 */
    Tb0_AnaCtrlTopDbg1   = 0x2e, /**< TB0_ana_ctrl_top_dbg1 */
    Tb0_Test0x55         = 0x3f, /**< TB0_tb0_test_0x55 */
} ptx_PSL_Tb0TestSignals_t;

/**
 * TB1 test signals.
 */
typedef enum ptx_PSL_Tb1TestSignals
{
    Tb1_TbOff           = 0x00, /**< TB1_TB_Off */
    Tb1_DrxDbgDac70     = 0x01, /**< TB1_drx_dbg_dac_7_0 */
    Tb1_DrxDbgBus       = 0x02, /**< TB1_drx_dbg_bus */
    Tb1_RxfdDbg0        = 0x03, /**< TB1_rxfd_dbg0 */
    Tb1_RxfdDbg1        = 0x04, /**< TB1_rxfd_dbg1 */
    Tb1_RxfdDbg2        = 0x05, /**< TB1_rxfd_dbg2 */
    Tb1_TxfgDbg1        = 0x06, /**< TB1_txfg_dbg1 */
    Tb1_ComDbg1         = 0x07, /**< TB1_com_dbg1 */
    Tb1_DmDbg1          = 0x08, /**< TB1_dm_dbg1 */
    Tb1_RffmDbg1        = 0x09, /**< TB1_rffm_dbg1 */
    Tb1_CltDbg0         = 0x0A, /**< TB1_clt_dbg0 */
    Tb1_UartDbg1        = 0x0B, /**< TB1_uart_dbg1 */
    Tb1_AdcSampleQCh    = 0x0C, /**< TB1_adc_sample_q_ch */
    Tb1_PxiDbg0         = 0x0D, /**< TB1_pxi_dbg0 */
    Tb1_PxiDbg1         = 0x0E, /**< TB1_pxi_dbg1 */
    Tb1_PxiDbg2         = 0x0F, /**< TB1_pxi_dbg2 */
    Tb1_PxiDbg3         = 0x10, /**< TB1_pxi_dbg3 */
    Tb1_DfyDbg1         = 0x11, /**< TB1_dfy_dbg1 */
    Tb1_AnaLpAdcQChDbg  = 0x12, /**< TB1_ana_lpadc_qch_dbg */
    Tb1_AnaDbg0         = 0x20, /**< TB1_ana_dbg0 */
    Tb1_AnaDbg1         = 0x21, /**< TB1_ana_dbg1 */
    Tb1_AnaDbg2         = 0x22, /**< TB1_ana_dbg2 */
    Tb1_AnaDbg3         = 0x23, /**< TB1_ana_dbg3 */
    Tb1_AnaDbg4         = 0x24, /**< TB1_ana_dbg4 */
    Tb1_UartDbg0        = 0x25, /**< TB1_uart_dbg0 */
    Tb1_TsensDbg1       = 0x26, /**< TB1_tsens_dbg1 */
    Tb1_PllCtDbg1       = 0x27, /**< TB1_pll_ct_dbg1 */
    Tb1_HifDbgI3C0      = 0x28, /**< TB1_hif_dbg_i3c0 */
    Tb1_AuxIfDbgI3C0    = 0x2a, /**< TB1_aux_if_dbg_i3c0 */
    Tb1_CmemCtrlDbg0    = 0x2b, /**< TB1_cmem_ctrl_dbg0 */
    Tb1_SwpMuxDbg0      = 0x2c, /**< TB1_swp_mux_dbg0 */
    Tb1_AuxIfCtrlDbg0   = 0x2d, /**< TB1_aux_if_ctrl_dbg0 */
    Tb1_AnaCtrlTopDbg0  = 0x2e, /**< TB1_ana_ctrl_top_dbg0 */
    Tb1_AnaCtrlTopDbg1  = 0x2f, /**< TB1_ana_ctrl_top_dbg1 */
    Tb1_Tb0Test0xAA     = 0x3f, /**< TB1_tb0_test_0xaa */
} ptx_PSL_Tb1TestSignals_t;

/**
 * Enumeration used for easier definition of NFC communication type and data rate.
 * The data rate is encoded with the type by intention.
 */
typedef enum ptx_PSL_NfcType
{
    NfcType_A106 = 0x10,    /**< NFC-A 106kBit/s  */
    NfcType_A212 = 0x11,    /**< NFC-A 212kBit/s  */
    NfcType_A424 = 0x12,    /**< NFC-A 424kBit/s  */
    NfcType_A848 = 0x13,    /**< NFC-A 848kBit/s  */
    NfcType_B106 = 0x20,    /**< NFC-B 106kBit/s  */
    NfcType_B212 = 0x21,    /**< NFC-B 212kBit/s  */
    NfcType_B424 = 0x22,    /**< NFC-B 424kBit/s  */
    NfcType_B848 = 0x23,    /**< NFC-B 848kBit/s  */
    NfcType_F212 = 0x41,    /**< NFC-F 212kBit/s  */
    NfcType_F424 = 0x42,    /**< NFC-F 424kBit/s  */
    NfcType_V026 = 0x80,    /**< NFC-V 26.5kBit/s */
    NfcType_Max
} ptx_PSL_NfcType_t;

/**
 * \brief SigDet measurement input parameters.
 */
typedef struct ptx_PSL_SigDet_InParams
{
    ptx_PSL_NfcType_t   NfcType;        /**< NFC interface type and data rate. */
    uint8_t             TxMultiplier;   /**< TX multiplier at which the measurement shall be carried out. Equal to TX_WAVE_CW_REG_PL1/0 in RF-config. */
    uint8_t             SigmaFactor;    /**< Defines how many times sigma is added. */
} ptx_PSL_SigDet_InParams_t;

/**
 * TB pins available.
 */
typedef enum ptx_PSL_TbTypes
{
    TbTypes_Tb0,                        /**< TB0 */
    TbTypes_Tb1,                        /**< TB1 */
} ptx_PSL_TbTypes_t;

/**
 * TB signals storage.
 */
typedef union ptx_PSL_TbSignals
{
    ptx_PSL_Tb0TestSignals_t SigTb0;    /**< TB0 signals */
    ptx_PSL_Tb1TestSignals_t SigTb1;    /**< TB1 signals */
} ptx_PSL_TbSignals_t;

/**
 * TB type and signal combination for setting TB signals.
 */
typedef struct ptx_PSL_TB
{
    ptx_PSL_TbTypes_t   Type;           /**< Type   */
    ptx_PSL_TbSignals_t Signal;         /**< Signal */
} ptx_PSL_TB_t;

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