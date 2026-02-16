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
    Module      : NFC Soft Controller (NSC) Interface Header for System Configuration
    File        : ptx_NSC_SystemConfig.h

    Description : Definition of NFC Soft Controller Interface for System Configuration.
                  Contains the function(s) to set specific parameters for hardware initialization.
*/

/**
 * \addtogroup NSC_SYSTEM_CONFIG
 *
 * @{
 */

#if !defined(PTX_NSC_SYSTEM_CONFIG_H)
#define PTX_NSC_SYSTEM_CONFIG_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Status.h"


#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
/**
 * \name Length definitions.
 * @{
 */
#define PTX_NSC_SYSTEM_CONFIG_CLK_SEL_LEN           (1u)        /**< NSC System Config clock selection length. */
#define PTX_NSC_SYSTEM_CONFIG_REFFREQ_LEN           (4u)        /**< NSC System Config Reference frequency length. */
#define PTX_NSC_SYSTEM_CONFIG_TARGETFREQ_LEN        (4u)        /**< NSC System Config Target frequency length. */
#define PTX_NSC_SYSTEM_CONFIG_PRNG_SEED_LEN         (4U)        /**< NSC System Config PRNGSEED length. */
#define PTX_NSC_SYSTEM_CONFIG_TEMP_ERR_TRESH_LEN    (1u)        /**< NSC System Config Over Temperature Threshold length. */
#define PTX_NSC_SYSTEM_CONFIG_NALM_OFFSET_LEN       (4u)        /**< NSC System Config PLL division word offset length. */
#define PTX_NSC_SYSTEM_CONFIG_NALM_MAX_LEN          (4u)        /**< NSC System Config Maximum NALM value length. */
#define PTX_NSC_SYSTEM_CONFIG_NALM_MIN_LEN          (4u)        /**< NSC System Config Minimum NALM value length. */
#define PTX_NSC_SYSTEM_CONFIG_XTAL_TRIM_LEN         (1u)        /**< NSC System Config XTAL trimming value length. */
#define PTX_NSC_SYSTEM_CONFIG_TX_CURLIMGAIN_LEN     (1u)        /**< NSC System Config TxCurlimGain value length. */
#define PTX_NSC_SYSTEM_CONFIG_PARAM_ID_MAX_PARAMID  (11u)       /**< NSC System Config sets of parameters currently implemented */
/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Forward declaration of ptx_NSC structure.
 */
struct ptx_NSC;

/**
 * \brief Load SystemConfig parameters callback function type. Loads parameters into NSC internal System Configuration structure.
 *
 * \param[in]       nsc         Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_SystemConfig_LoadParamCb_t)(struct ptx_NSC *nsc);

/**
 * \brief System Config parameter type.
 */
typedef enum ptx_NSC_SystemConfig_Type
{
    NSC_SystemConfig_Config_ClkSel,         /*< Clock Selection 0 = XTAL, 1 = external clock using XTAL_IN, 2 = external clock using GPIO2.  */
    NSC_SystemConfig_Config_ClkReqEn,       /*< External Clock Request Enable 0 = disabled, 1 = enabled.  */
    NSC_SystemConfig_Config_ClkReqGt,       /*< External Clock Request Guard Time in microseconds.  */
    NSC_SystemConfig_Config_RefFreq,        /*< Frequency of the system's clock source. */
    NSC_SystemConfig_Config_TargetFreq,     /*< System's target Frequency. */
    NSC_SystemConfig_Config_PrngSeed,       /*< PRNG seed value. */
    NSC_SystemConfig_Config_NAlmOffset,     /*< Offset used for PLL division word calculation, for ALM operation. */
    NSC_SystemConfig_Config_NalmMax,        /*< Maximum valid NALM value. */
    NSC_SystemConfig_Config_NalmMin,        /*< Minimum valid NALM value. */
    NSC_SystemConfig_Config_XtalTrim,       /*< XTAL trimming value. */
    NSC_SystemConfig_Config_TempErrTresh,   /*< Over-temperature error treshold. */
    NSC_SystemConfig_Config_TxCurlimGain,   /*< Over-current gain selection. */
    NSC_SystemConfig_Config_MAX             /*< Maximum enumeration Value */
} ptx_NSC_SystemConfig_Type_t;

/**
 * \brief System Config parameter value.
 */
typedef union ptx_NSC_SystemConfig_Value
{
    uint8_t     ClkSel;             /**< Clock Selection 0 = XTAL, 1 = external clock using XTAL_IN, 2 = external clock using GPIO2.  */
    uint8_t     ClkReqEn;           /**< External Clock Request Enable 0 = disabled, 1 = enabled.  */
    uint8_t     ClkReqGt;           /**< External Clock Request Guard Time in microseconds.  */
    uint32_t    RefFreq;            /**< Frequency of the system's clock source. */
    uint32_t    TargetFreq;         /**< System's target frequency. */
    uint32_t    PrngSeed;           /**< PRNG seed value. */
    uint32_t    NAlmOffset;         /**< Offset used for PLL division word calculation, for ALM operation. */
    uint32_t    NalmMax;            /**< Maximum valid NALM value. */
    uint32_t    NalmMin;            /**< Minimum valid NALM value. */
    uint8_t     XtalTrim;           /**< XTAL trimming value. */
    uint8_t     TempErrTresh;       /**< Over-temperature error treshold value. */
    uint8_t     TxCurlimGain;       /**< Over-current error gain selection. */
} ptx_NSC_SystemConfig_Value_t;

/**
 * \brief System Config parameter.
 */
typedef struct ptx_NSC_SystemConfig_Param
{
    ptx_NSC_SystemConfig_Type_t     Config;             /**< System config type */
    ptx_NSC_SystemConfig_Value_t    Value;              /**< System config value */
} ptx_NSC_SystemConfig_Param_t;

/**
 * \brief System Config Structure.
 */
typedef struct ptx_NSC_SystemConfig
{
    uint8_t     ClkSel;                                 /**< Clock Selection 0 = XTAL, 1 = external clock (using XTAL_IN pad),
                                                             2 = external clock using GPIO2 pad. */
    uint8_t     ClkReqEn;                               /**< External clock request enable 0 = disabled */
    uint8_t     ClkReqGt;                               /**< External clock request guard time, measured in microseconds,
                                                             specifies the duration the PTX2K shall wait after a clock request
                                                             is made before the HOST provides a stable clock signal. */
    uint8_t     XtalTrim;                               /**< XTAL trimming value. */
    uint32_t    RefFreq;                                /**< Frequency of the system clock source. */
    uint32_t    TargetFreq;                             /**< System target frequency. */
    uint32_t    PrngSeed;                               /**< PRNG seed value. */
    uint32_t    NalmMax;                                /**< Maximum valid NALM value. */
    uint32_t    NalmMin;                                /**< Minimum valid NALM value. */
    uint32_t    NAlmOffset;                             /**< Offset used for PLL division word calculation, for ALM operation. */
    uint8_t     TempErrTreshold;                        /**< Over-temperature error treshold, in °C.
                                                             Value: 0-127 C. 0 - temperature protection disabled. */
    uint8_t     TxCurlimGain;                           /**< Over-current error gain selection.
                                                             Value:  0 - over-current protection disabled,
                                                                     1 - 12.5 (100mOhm resistor), 2 - 25 (50mOhm resistor). */
    ptx_NSC_SystemConfig_LoadParamCb_t LoadSystemParam; /**< System Parameter load callback. */
} ptx_NSC_SystemConfig_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Load default parameters into internal NSC System Configuration structure.
 *
 * \param[in]       nsc         Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_System_LoadDefaultParam(struct ptx_NSC *nsc);

/**
 * \brief Update Configuration
 *
 * Update the system configuration of the device. Updating these parameters requires the device to be in the INIT state.
 * If the device is not in the INIT state, a Reset will be performed in order to update these parameters.
 *
 * \param[in]       nsc         Pointer to component.
 * \param[in]       params      Pointer to parameter array.
 * \param[in]       cnt         Number of elements in params array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_System_UpdateConfiguration(struct ptx_NSC *nsc, ptx_NSC_SystemConfig_Param_t *params, uint8_t cnt);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
