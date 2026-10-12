/** \file
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
    Module      : NFC Soft Controller (NSC) Interface Header for Testing
    File        : ptx_NSC_RfTest.h

    Description : Definition of NFC Soft Controller Interface for Testing.
                  Contains function(s) to evaluate system RF performance.
*/


/**
 * \addtogroup NSC_RF_TEST
 *
 * @{
 */

#if !defined(PTX_NSC_RFTEST_H)
#define PTX_NSC_RFTEST_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Int.h"


#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name RF Test Parameter definition.
 * @{
 */
#define PTX_NSC_RFTEST_ENAB_SOFEOF_B        (uint8_t) 0x01  /**< Enable generation of SoF and EoF */
#define PTX_NSC_RFTEST_DISAB_SOFEOF_B       (uint8_t) 0x00  /**< Disable generation of SoF and EoF */

#define PTX_NSC_RFTEST_ENAB_STARTSTOP_B     (uint8_t) 0x01  /**< Enable generation of Start- and Stop- Bits */
#define PTX_NSC_RFTEST_DISAB_STARTSTOP_B    (uint8_t) 0x00  /**< Disable generation of Start- and Stop- Bits */

#define PTX_NSC_RFTEST_ENAB_PARITY_A        (uint8_t) 0x01  /**< Enable generation of Parity-Bits */
#define PTX_NSC_RFTEST_DISAB_PARITY_A       (uint8_t) 0x00  /**< Disable generation of Parity-Bits */

#define PTX_NSC_RFTEST_ENAB_CRC             (uint8_t) 0x01  /**< Enable generation of CRC */
#define PTX_NSC_RFTEST_DISAB_CRC            (uint8_t) 0x00  /**< Disable generation of CRC */
/** @} */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Test Sequence ID
 */
typedef enum ptx_NSC_RfTest_SeqId
{
    NSC_RfTest_SeqId_None       = 0,
    NSC_RfTest_SeqId_DDPC       = 1U,
    NSC_RfTest_SeqId_MeasRFAmp  = 2U,
    NSC_RfTest_SeqId_Carrier    = 3U,
    NSC_RfTest_SeqId_PRBS_9     = 4U,
    NSC_RfTest_SeqId_PRBS_15    = 5U,
    NSC_RfTest_SeqId_Undefined  = 6U,

} ptx_NSC_RfTest_SeqId_t;

/**
 * \brief RF-Test Test Type
 */
typedef enum ptx_RfTest_TechType
{
    RF_TEST_TECH_A_POLL     = 0x01,       /**< NFC-A Technology Poll */
    RF_TEST_TECH_B_POLL     = 0x02,       /**< NFC-B Technology Poll */
    RF_TEST_TECH_F_POLL     = 0x03,       /**< NFC-F Technology Poll */
    RF_TEST_TECH_V_POLL     = 0x04,       /**< NFC-V Technology Poll */
} ptx_RfTest_TechType_t;

/**
 * \brief RF-Test PRBS Test Type
 */
typedef enum ptx_RfTest_BitRate
{
    RF_TEST_BITRATE_106     = 0x00,      /**< RF Bit-rate 106 kBit/s */
    RF_TEST_BITRATE_212     = 0x01,      /**< RF Bit-rate 212 kBit/s */
    RF_TEST_BITRATE_424     = 0x02,      /**< RF Bit-rate 424 kBit/s */
    RF_TEST_BITRATE_848     = 0x03,      /**< RF Bit-rate 848 kBit/s */
    RF_TEST_BITRATE_26_5    = 0x06       /**< RF Bit-rate 26.6 kBit/s */
} ptx_RfTest_BitRate_t;

/**
 * \brief RF-Test parameters for the Carrier test
 */
typedef struct ptx_NSC_RfTest_Params_Carrier
{
    uint8_t         RffAmp;                  /**< Rf-Field amplitude */
} ptx_NSC_RfTest_Params_Carrier_t;

/**
 * \brief RF-Test parameters for the DDPC test
 */
typedef struct ptx_NSC_RfTest_Params_DDPC
{
    uint8_t         DDPC_Period;    /**< Period of DDPC procedure in milliseconds. [1, 255] */
} ptx_NSC_RfTest_Params_DDPC_t;

/**
 * \brief RF-Test parameters for the RF Amplitude test
 */
typedef struct ptx_NSC_RfTest_Params_RfAmp
{
    uint8_t         XtalDiv2En;     /**< XTAL output divider enable. */
    uint16_t        GatePw;         /**< Number of reference clock periods the sys_clk counter should be enabled for during PLL calibration. */
    uint16_t        SysCntTg;       /**< Expected number of clock pulses during PLL calibration. */
    uint32_t        NHost;          /**< PLL division factor for the selected RF field frequency. */
    uint8_t         AvgFactorInt;   /**< RF amplitude measurement averaging factor. */
} ptx_NSC_RfTest_Params_RfAmp_t;

/**
 * \brief RF-Test parameters for the PRBS test
 */
typedef struct ptx_NSC_RfTest_Params_PRBS
{
    ptx_RfTest_TechType_t       Technology;             /**< RF-Technology */
    ptx_RfTest_BitRate_t        Bitrate;                /**< RF-Bitrate */
    uint8_t                     EnabSofEof;             /**< PRBS-specific flag, set 0 to disable NFC-B SoF/EoF */
    uint8_t                     EnabStartStopBits;      /**< PRBS-specific flag, set 0 to disable NFC-B start/stop bits */
    uint8_t                     EnabParityBits;         /**< PRBS-specific flag, set 0 to disable NFC-A parity bits */
    uint8_t                     EnabCRC;                /**< PRBS-specific flag, set 0 to disable generation of CRC */
} ptx_NSC_RfTest_Params_PRBS_t;

/**
 * \brief Union for Test specific parameters structures
*/
typedef union ptx_NSC_RfTest_Param
{
    ptx_NSC_RfTest_Params_DDPC_t    DDPCParams;         /**< DDPC Test parameters. */
    ptx_NSC_RfTest_Params_RfAmp_t   RfAmpParams;        /**< RF Amplitude Test parameters. */
    ptx_NSC_RfTest_Params_PRBS_t    PRBSParams;         /**< PRBS Test parameters. */
    ptx_NSC_RfTest_Params_Carrier_t CarrierParams;      /**< Carrier Test parameters. */
} ptx_NSC_RfTest_Param_t;

/**
 * \brief Structure for general Rf-Test input parameters
*/
typedef struct ptx_NSC_RfTest_InParams
{
    ptx_NSC_RfTest_SeqId_t          SeqId;              /**< Test sequence identifier. */
    ptx_NSC_RfTest_Param_t          Params;             /**< Test parameters. */
} ptx_NSC_RfTest_InParams_t;

/**
 * \brief Test sequence specific output structures
*/
typedef struct ptx_NSC_RfTest_RfAmp_Out
{
    uint32_t        RfAmplitude;                        /**< Measured RF Amplitude. */
} ptx_NSC_RfTest_RfAmp_Out_t;

/**
 * \brief Test sequence specific result structure
*/
typedef union ptx_NSC_RfTest_Out
{
    ptx_NSC_RfTest_RfAmp_Out_t      RfAmpOut;           /**< RF Amplitude test result parameter. */
} ptx_NSC_RfTest_Output_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Function used to run NSC_RF_RUN_CMD.
 *
 * \param[in]       nsc         Pointer to the component structure.
 * \param[in]       inParams    Pointer to the structure holding test sequence ID and specific parameters.
 * \param[out]      outRes      Pointer to the structure holding test sequence results.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_RfTest(ptx_NSC_t *nsc, ptx_NSC_RfTest_InParams_t *inParams, ptx_NSC_RfTest_Output_t *outRes);

/**
 * \brief Function to stop internal Rf-Tests
 *
 * \param[in] nsc Pointer to the component structure.
*/
ptx_Status_t ptx_NSC_RfTestStop(ptx_NSC_t *nsc);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
