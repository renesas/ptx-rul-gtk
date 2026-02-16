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
    Module      : PSL
    File        : ptx_PSL.h

    Description : Implementation of Product Support Library (PSL), to alleviate the integration of the PTX2xxR/W
                  into the final target application by providing a set of functions to identify and debug potential
                  performance issues during the design-in phase.

*/

/**
 * \addtogroup PSL
 * @{
 */

#if !defined(PTX_PSL_H)
#define PTX_PSL_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_RUL.h"

#include "ptx_PSL_Int.h"


#if defined(__cplusplus)
extern "C" {
#endif

/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/

/**
 * \brief Antenna Q measurement input parameters.
 */
typedef struct ptx_PSL_QMeas_Parameters
{
    uint32_t                FreqSpan;   /**< Frequency span (CenterFrequency = 13.56MHz). */
    uint32_t                FreqStep;   /**< Frequency step in Hz. */
    ptx_RUL_AdcAveraging_t  AvgFactor;  /**< Averaging factor. */
} ptx_PSL_QMeas_Parameters_t;

/**
 * \brief Antenna Q measurement input parameters.
 */
typedef struct ptx_PSL_QMeas_Result
{
    uint32_t            QFactor;            /**< Calculated Q-Factor multiplied by the factor of 100. */
    uint32_t            ResonaceFreq;       /**< Resonance frequency. 0 if not found */
    uint32_t            LowerCornerFreq;    /**< Lower -3dB corner frequency. 0 if not found */
    uint32_t            UpperCornerFreq;    /**< Upper -3dB corner frequency. 0 if not found */
} ptx_PSL_QMeas_Result_t;

/**
 * \brief PSL Clock Configuration consisting of the reference frequency
 *          and the desired target frequency.
 */
typedef struct ptx_PSL_Clk_Config
{
    uint32_t            RefFreq;        /**< Reference frequency in Hz. */
    uint32_t            TargetFreq;     /**< Target/current frequency in Hz. */
} ptx_PSL_Clk_Config_t;

/**
 * \brief Calculated PSL Clock Parameters to achieve the target frequency with the given reference frequency.
 */
typedef struct ptx_PSL_Clk_Params
{
    uint8_t             XtalDiv2En;     /**< XTAL output divider enable. */
    uint16_t            GatePw;         /**< Number of reference clock periods the sys_clk counter should be enabled for during PLL calibration. */
    uint16_t            SysCntTg;       /**< Expected number of clock pulses during PLL calibration. */
    uint32_t            NHost;          /**< PLL division factor for the selected RF field frequency. */
} ptx_PSL_Clk_Params_t;

/**
 * \brief PSL DDPC power levels.
 */
typedef struct ptx_PSL_Ddpc_Measurement_Params
{
    uint8_t                 PowerLevel0;    /**< Low CW power level. */
    uint8_t                 PowerLevel1;    /**< High CW power level. */
    ptx_RUL_AdcAveraging_t  AvgFactor;      /**< ADC Averaging factor. */
} ptx_PSL_Ddpc_Measurement_Params_t;

/**
 * \brief PSL DDPC RSSI results.
 */
typedef struct ptx_PSL_Ddpc_Results
{
    uint32_t            RssiPowerLevel0;   /**< Measured RSSI for low CW power level. */
    uint32_t            RssiPowerLevel1;   /**< Measured RSSI for high CW power level. */
} ptx_PSL_Ddpc_Results_t;

/**
 * \brief PSL DDPC settings.
 */
typedef struct ptx_PSL_Ddpc_Params
{
    uint8_t                 PowerLevel0;    /**< Low CW power level. */
    uint8_t                 PowerLevel1;    /**< High CW power level. */
    ptx_RUL_AdcAveraging_t  AvgFactor;      /**< ADC Averaging factor. */
    uint16_t                LowThreshold;   /**< Threshold for switching to the high power level (PL1) (= 9-Bit value). */
    uint16_t                HighThreshold;  /**< Threshold for switching to the low power level (PL0) (= 9-Bit value). */
} ptx_PSL_Ddpc_Params_t;


/**
 * \brief PSL DDPC Status.
 */
typedef struct ptx_PSL_Ddpc_Status
{
    uint8_t     PowerLevel; /**< The power level currently in use (either 0 or 1). */
    uint32_t    Rssi;       /**< Current RSSI value (used to decide, which powerlevel to use). */
} ptx_PSL_Ddpc_Status_t;


/**
 * DScope source selection.
 */
typedef enum ptx_PSL_DScopeSource
{
    DScopeSource_AdcIQChannel,      /**< DSCOPE channel 0 samples I channel, whereas DSCOPE channel 1 samples Q-channel. */
    DScopeSource_DigRx,             /**< DSCOPE samples DigRx. */
    DScopeSource_TestBus0,          /**< DSCOPE samples TestBus0. */
    DScopeSource_TestBus1,          /**< DSCOPE samples TestBus1. */
    DScopeSource_Tsense,            /**< DSCOPE samples Tsense. */
    DScopeSource_LpAdc,             /**< DSCOPE samples LpADC. */
    DScopeSource_Anatest,           /**< DSCOPE samples Anatest signal. */
    DScopeSource_TsenseRaw,         /**< DSCOPE samples TsenseRaw signal. */
    DScopeSource_TsenseRawLatched   /**< DSCOPE samples TsenseRawLatched signal. */
} ptx_PSL_DScopeSource_t;

/**
 * DScope strobe selection.
 */
typedef enum ptx_PSL_DScopeStrobe
{
    DScopeStrobe_27MHz12,               /**< DSCOPE strobe set to 27.12 MHz. */
    DScopeStrobe_13MHz56,               /**< DSCOPE strobe set to 13.56 MHz. */
    DScopeStrobe_6MHz78,                /**< DSCOPE strobe set to 6.78 MHz. */
    DScopeStrobe_3MHz39,                /**< DSCOPE strobe set to 3.39 MHz. */
    DScopeStrobe_1MHz69,                /**< DSCOPE strobe set to 1.69 MHz. */
    DScopeStrobe_848kHz,                /**< DSCOPE strobe set to 848 kHz. */
    DScopeStrobe_424kHz,                /**< DSCOPE strobe set to 424 kHz. */
    DScopeStrobe_212kHz,                /**< DSCOPE strobe set to 212 kHz. */
    DScopeStrobe_106kHz,                /**< DSCOPE strobe set to 106 kHz. */
    DScopeStrobe_TempValid = 0b1111,    /**< DSCOPE strobe set to TempValid signal. */
} ptx_PSL_DScopeStrobe_t;

/**
 * DScope capture size selection.
 */
typedef enum ptx_PSL_DScopeCaptureSize
{
    DScopeCaptureSize_8B,   /**< DSCOPE capture size set to 8 Byte. */
    DScopeCaptureSize_16B,  /**< DSCOPE capture size set to 16 Byte. */
    DScopeCaptureSize_32B,  /**< DSCOPE capture size set to 32 Byte. */
    DScopeCaptureSize_64B,  /**< DSCOPE capture size set to 64 Byte. */
    DScopeCaptureSize_128B, /**< DSCOPE capture size set to 128 Byte. */
    DScopeCaptureSize_256B, /**< DSCOPE capture size set to 256 Byte. */
    DScopeCaptureSize_512B, /**< DSCOPE capture size set to 512 Byte. */
    DScopeCaptureSize_1KB,  /**< DSCOPE capture size set to 1 KByte. */
    DScopeCaptureSize_2KB,  /**< DSCOPE capture size set to 2 KByte. */
    DScopeCaptureSize_4KB,  /**< DSCOPE capture size set to 4 KByte. */
    DScopeCaptureSize_6KB,  /**< DSCOPE capture size set to 6 KByte. */
    DScopeCaptureSize_8KB,  /**< DSCOPE capture size set to 8 KByte. */
    DScopeCaptureSize_10KB, /**< DSCOPE capture size set to 10 KByte. */
    DScopeCaptureSize_12KB, /**< DSCOPE capture size set to 12 KByte. */
    DScopeCaptureSize_14KB, /**< DSCOPE capture size set to 14 KByte. */
    DScopeCaptureSize_16KB  /**< DSCOPE capture size set to 16 KByte. */
} ptx_PSL_DScopeCaptureSize_t;

/**
 * DScope capture position selection.
 */
typedef enum ptx_PSL_DScopeCaptureOffset
{
    DScopeCaptureOffset_0KB,    /**< DSCOPE capture offset set to 0 KByte. */
    DScopeCaptureOffset_1KB,    /**< DSCOPE capture offset set to 1 KByte. */
    DScopeCaptureOffset_2KB,    /**< DSCOPE capture offset set to 2 KByte. */
    DScopeCaptureOffset_3KB,    /**< DSCOPE capture offset set to 3 KByte. */
    DScopeCaptureOffset_4KB,    /**< DSCOPE capture offset set to 4 KByte. */
    DScopeCaptureOffset_5KB,    /**< DSCOPE capture offset set to 5 KByte. */
    DScopeCaptureOffset_6KB,    /**< DSCOPE capture offset set to 6 KByte. */
    DScopeCaptureOffset_7KB,    /**< DSCOPE capture offset set to 7 KByte. */
    DScopeCaptureOffset_8KB,    /**< DSCOPE capture offset set to 8 KByte. */
    DScopeCaptureOffset_9KB,    /**< DSCOPE capture offset set to 9 KByte. */
    DScopeCaptureOffset_10KB,   /**< DSCOPE capture offset set to 10 KByte. */
    DScopeCaptureOffset_11KB,   /**< DSCOPE capture offset set to 11 KByte. */
    DScopeCaptureOffset_12KB,   /**< DSCOPE capture offset set to 12 KByte. */
    DScopeCaptureOffset_13KB,   /**< DSCOPE capture offset set to 13 KByte. */
    DScopeCaptureOffset_14KB,   /**< DSCOPE capture offset set to 14 KByte. */
    DScopeCaptureOffset_15KB    /**< DSCOPE capture offset set to 15 KByte. */
} ptx_PSL_DScopeCaptureOffset_t;

/**
 * DScope RAM module selection.
 */
typedef enum ptx_PSL_DScopeRamModule
{
    DScopeRamModule0,   /**< DSCOPE stores the captured data to RAM module 0. */
    DScopeRamModule1    /**< DSCOPE stores the captured data to RAM module 1. */
} ptx_PSL_DScopeRamModule_t;

/**
 * \brief PSL Initialisation Parameters.
 */
typedef struct ptx_PSL_InitParams
{
    ptx_RUL_t           *Rul;           /**< RUL component reference */
    ptx_Log_t           *Log;           /**< LOG component reference */
} ptx_PSL_InitParams_t;

/**
 * \brief PSL Type.
 */
typedef struct ptx_PSL
{
    ptx_Comps_t         CompId;         /**< Component Id */
    ptx_RUL_t           *Rul;           /**< RUL component reference */
    ptx_Log_t           *Log;           /**< LOG component reference */
    uint8_t             IgnoreGpio2;    /**< Flag to ignore certain functionalities for GPIO2 (can be used as CLK source). */
} ptx_PSL_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief PSL Component Initialisation.
 *
 * \param[in,out]   psl             Pointer to PSL component.
 * \param[in]       initParams      Pointer to initialisation parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_Init(ptx_PSL_t *psl, ptx_PSL_InitParams_t *initParams);

/**
 * \brief PSL RF Field Amplitude Measurement at the provided frequency.
 * *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       targetFrequency Target frequency for the amplitude measurement.
 * \param[out]      averaging       Number of measurement to average.
 * \param[out]      rfAmp           Pointer to the measurement result.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_RfFieldMeasurement(ptx_PSL_t *psl, uint32_t targetFrequency, ptx_RUL_AdcAveraging_t averaging, uint32_t *rfAmp);

/**
 * \brief PSL RSSI Measurement at 13.56MHz using the given CW power level.
 *
 * \param[in]       psl                 Pointer to PSL component.
 * \param[in]       cwPowerLevel        CW multiplier value used for the measurement.
 * \param[in]       averaging           The amount of averaged measurements.
 * \param[out]      result              Pointer to store the measurement result.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_RssiMeasurement(ptx_PSL_t *psl, uint8_t cwPowerLevel, ptx_RUL_AdcAveraging_t averaging, uint32_t *result);

/**
 * \brief PSL Q-measurement.
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       measParams      Pointer to the structure holding the measurement parameters.
 * \param[out]      measResult      Pointer to store the measurement result.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_QMeasurement(ptx_PSL_t *psl, ptx_PSL_QMeas_Parameters_t *measParams, ptx_PSL_QMeas_Result_t *measResult);

/**
 * \brief Writes the DDPC measurement parameters to the PTX2xxR/W.
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       params          Pointer to the DDPC measurement parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_SetDdpcParameters(ptx_PSL_t *psl, ptx_PSL_Ddpc_Params_t *params);

/**
 * \brief Evaluates the RSSI using the given DDPC parameters.
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       measParams      Pointer to the measurement parameters for the DDPC evaluation.
 * \param[out]      results         Pointer to store the DDPC measurement results for the given power levels.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_RunDdpcEvaluation(ptx_PSL_t *psl, ptx_PSL_Ddpc_Measurement_Params_t *measParams, ptx_PSL_Ddpc_Results_t *results);

/**
 * \brief Continuously runs the DDPC test procedure with a defined period.
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       measIntervalMs  DDPC measurement interval in milliseconds.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_RunDdpcTest(ptx_PSL_t *psl, uint8_t measIntervalMs);

/**
 * \brief Retrieves the DDPC status (only valid if "DDPC Test" is running).
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       ddpcStatus      Pointer to store the DDPC measurement status.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_GetDdpcStatus(ptx_PSL_t *psl, ptx_PSL_Ddpc_Status_t *ddpcStatus);

/**
 * \brief Stops the DDPC test procedure.
 *
 * \param[in]       psl             Pointer to PSL component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_StopDdpcTest(ptx_PSL_t *psl);

/**
 * \brief Write a single memory register.
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       address         Memory address.
 * \param[in]       value           Value to write to register.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_WriteRegister(ptx_PSL_t *psl, uint16_t address, uint8_t value);

/**
 * \brief Read a single memory register.
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       address         Memory address.
 * \param[out]      value           Value in register.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_ReadRegister(ptx_PSL_t *psl, uint16_t address, uint8_t *value);

/**
 * \brief Read a single memory register, apply a new value via a mask, and write it to the register.
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       address         Memory address.
 * \param[in]       bitValue        Value to write to register.
 * \param[in]       mask            Mask to apply to register
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_ModifyRegister(ptx_PSL_t *psl, uint16_t address, uint8_t bitValue, uint8_t mask);

/**
 * \brief Reset all registers involved in analog MUX and digital Mux configuration to default values.
 *
 * \param[in]       psl             Pointer to PSL component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_ResetAllRegs(ptx_PSL_t *psl);

/**
 * \brief Map any analog signal to specific I/O pad.
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       testPinNr       Test pin number (Atst0 - Atst3).
 * \param[in]       signal          Test signal to be muxed to the specified pin.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_AnaMuxSelectSignal(ptx_PSL_t *psl, ptx_PSL_AnaTestPin_t testPinNr, ptx_PSL_AnaMuxTestSignals_t signal);

/**
 * \brief Reset all the short switches to open state.
 *
 * \param[in]       psl             Pointer to PSL component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_ResetShortOutputSignal(ptx_PSL_t *psl);

/**
 * \brief Connect first pad with second pad via a short. Order is not critical.
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       pin1            First pad.
 * \param[in]       pin2            Second pad.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_ShortOutputSignal(ptx_PSL_t *psl, ptx_PSL_AnaTestPin_t pin1, ptx_PSL_AnaTestPin_t pin2);

/**
 * \brief Configure the digital multiplexer with the provided configuration to analyze internal digital test signals.
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       swapTestMux     Enables swapping for complete TB0, TB1.
 * \param[in]       swapTB1         Defines the bit swapping configuration within TB1.
 * \param[in]       swapTB0         Defines the bit swapping configuration within TB0.
 * \param[in]       sysClkPinNbr    Defines the pin number for the system clock output.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_DigMuxConfigure(ptx_PSL_t *psl, uint8_t swapTestMux, ptx_PSL_TbSwapType_t swapTB1, ptx_PSL_TbSwapType_t swapTB0, uint8_t sysClkPinNbr);

/**
 * \brief Resets the digital multiplexer.
 *
 * \param[in]       psl             Pointer to PSL component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_DigMuxReset(ptx_PSL_t *psl);

/**
 * \brief Selects a test vector signal.
 *
 * \param[in]       psl             Pointer to PSL component.
 * \param[in]       signal          Test vector to be muxed out.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_DigMuxSelectSignal(ptx_PSL_t *psl, ptx_PSL_TB_t signal);

/**
 * \brief Measures the system inherent noise seen by the receiver to determine the signal detector threshold necessary to
 *        prevent unintended activation of the digital receiver. The measurement dependend on the data rate and the type.
 *
 * \param[in]     psl               Pointer to PSL component.
 * \param[in]     inParams          Structure defining the measurement paramters. See \ref ptx_PSL_SigDet_InParams_t
 * \param[out]    sigDetValue       Resulting SigDet value. The value can be written directly to the RD_THR registers.
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_SigDetMeasurement(ptx_PSL_t *psl, ptx_PSL_SigDet_InParams_t *inParams, uint32_t *sigDetValue);

/**
 * \brief Deinitialized the PSL component and resets all the devices' registers to their reset states.
 *
 * \param[in]     psl               Pointer to PSL component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_PSL_DeInit(ptx_PSL_t *psl);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
