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
    Module      : FELICA DTE
    File        : ptx_FeliCa_DTE.h

    Description : FeliCa DTE component to execute FeliCa-related tests for passing FeliCa M-class certification and test-sequences defined in Reader/Writer
                  Digital Protocol Requirements Specification.
*/

/**
 * \addtogroup FELICA_DTE
 *
 * @{
 */

#if !defined(PTX_FELICA_DTE_H)
#define PTX_FELICA_DTE_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>
#include "ptx_RUL.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief FeliCa-DTE Test Type
 */
typedef enum ptx_FeliCaDTE_BitRate
{
    FELICA_DTE_BITRATE_212 = 1,                             /**< RF Bitrate 212 kBit/s */
    FELICA_DTE_BITRATE_424 = 2,                             /**< RF Bitrate 424 kBit/s */

} ptx_FeliCaDTE_BitRate_t;

/**
 * \brief FeliCa-DTE Test Type
 */
typedef enum ptx_FeliCaDTE_TestID
{
    FELICA_DTE_TestID_PerformanceTest,                      /**< RF-Performance Tests */
    FELICA_DTE_TestID_RWDigProtTest,                        /**< Reader/Writer Digital Protocol Test */

} ptx_FeliCaDTE_TestID_t;

/**
 * \brief FeliCa-DTE Reader/Writer Digital Protocol (Sub-)Test Type
 */
typedef enum ptx_FeliCaDTE_RWDigProt_TestID
{
    FELICA_DTE_SubTestID_RWDigProt_eMoney_Variant1,            /**< Reader/Writer Digital Protocol Test - Only enabling eMoney payments, Variant 1: SC = 0x0000, RC = 0x00 */
    FELICA_DTE_SubTestID_RWDigProt_eMoney_Variant2,            /**< Reader/Writer Digital Protocol Test - Only enabling eMoney payments, Variant 2: SC = 0xFFFF, RC = 0x01 */
    FELICA_DTE_SubTestID_RWDigProt_eMoney_NFCDEP,              /**< Reader/Writer Digital Protocol Test - Enabling eMoney payments and NFC-DEP / P2P */
    FELICA_DTE_SubTestID_RWDigProt_FrameStructure,             /**< Reader/Writer Digital Protocol Test - Frame Structure sequences */
    FELICA_DTE_SubTestID_RWDigProt_FrameStructure1,            /**< Reader/Writer Digital Protocol Test - Frame Structure sequence 1 */
    FELICA_DTE_SubTestID_RWDigProt_FrameStructure2,            /**< Reader/Writer Digital Protocol Test - Frame Structure sequence 2 */
    FELICA_DTE_SubTestID_RWDigProt_FrameStructure3,            /**< Reader/Writer Digital Protocol Test - Frame Structure sequence 3 */
    FELICA_DTE_SubTestID_RWDigProt_FrameStructure4,            /**< Reader/Writer Digital Protocol Test - Frame Structure sequence 4 */
    FELICA_DTE_SubTestID_RWDigProt_FrameStructure5,            /**< Reader/Writer Digital Protocol Test - Frame Structure sequence 5 */
    FELICA_DTE_SubTestID_RWDigProt_FrameStructure6,            /**< Reader/Writer Digital Protocol Test - Frame Structure sequence 6 */
    FELICA_DTE_SubTestID_RWDigProt_Setup_DP_POS,               /**< Reader/Writer Digital Protocol Test - Setup DP Test-board - positive Encoding */
    FELICA_DTE_SubTestID_RWDigProt_Setup_DP_NEG,               /**< Reader/Writer Digital Protocol Test - Setup DP Test-board - negative Encoding */
    FELICA_DTE_SubTestID_RWDigProt_Generic,                    /**< Reader/Writer Digital Protocol Test - Generic Test to send any (test-)command from application layer */

} ptx_FeliCaDTE_RWDigProt_TestID_t;

/**
 * \brief FeliCa-DTE Test Parameters for Performance-Tests
 */
typedef struct ptx_FeliCaDTE_PerformanceTest
{
    uint8_t                             NrTests;            /**< Number of Tests/Repetitions */
    uint8_t                             *ResultBuffer;      /**< Pointer to Result-Buffer */
    uint8_t                             ResultBufferSize;   /**< Size of Result-Buffer (must be >= NrTests) */
    ptx_FeliCaDTE_BitRate_t             Bitrate;            /**< Desired Bitrate */
    uint32_t                            TimeOutMS;          /**< Timeout value in [ms] for a single test-run (MUST be set to a value != 0!) */

} ptx_FeliCaDTE_PerformanceTest_t;

/**
 * \brief FeliCa-DTE Test Parameters for Protocol-Tests
 */
typedef struct ptx_FeliCaDTE_RWDigProtTest
{
    ptx_FeliCaDTE_RWDigProt_TestID_t    SubTestID;           /**< Sub-test Identifier (Reader / Writer Digital Protocol sequence ID) */
    const uint32_t                      *T_OffGuardTime;     /**< Guard Time between Field-Off and Field-On (default value = 30ms) */
    const uint32_t                      *T_OnGuardTime;      /**< Guard Time between Field-On first Command (default value = 25ms) */
    uint8_t                             *GenericCmdBuffer;   /**< Pointer to Buffer containing generic (application-)command */
    uint32_t                            GenericCmdBufferLen; /**< Length of generic (application-)command */
    uint32_t                            TimeOutMS;           /**< Timeout value in [ms] for a single RF data exchange (MUST be set to a value != 0!) */
    uint8_t                             *ResultBuffer;       /**< Pointer to Result-Buffer */
    uint32_t                            ResultBufferSize;    /**< Size of Result-Buffer (must be >= 256 Bytes) */
} ptx_FeliCaDTE_RWDigProtTest_t;

/**
 * \brief FeliCa-DTE Test Parameters
 */
typedef union ptx_FeliCaDTE_Params
{
    ptx_FeliCaDTE_PerformanceTest_t     PerformanceTest;    /**< FeliCa Performance Test parameters */
    ptx_FeliCaDTE_RWDigProtTest_t       RWDigProtTest;      /**< FeliCa Reader/Writer Digital Protocol Test parameters */

} ptx_FeliCaDTE_Params_t;

/**
 * \brief FeliCa-DTE Test Progress Parameters
 */
typedef struct ptx_FeliCaDTE_TestProgressParams
{
    uint8_t                             ExitProcessing;     /**< Flag to abort an ongoing Test-execution (if applicable) */
    uint8_t                             NrTestsProcessed;   /**< Info how many tests / subsequences were executed so far (if applicable) */

} ptx_FeliCaDTE_TestProgressParams_t;

/**
 * \brief Progress Callback Function
 */
typedef void (*ptx_FeliCaDTE_ProgressFn_t)(ptx_FeliCaDTE_TestProgressParams_t *progressParams);

/**
 * \brief FeliCa-DTE Test Parameters
 */
typedef struct ptx_FeliCaDTE_TestParams
{
    ptx_FeliCaDTE_TestID_t              ID;                 /**< Compliance Test-Identifier */
    ptx_FeliCaDTE_Params_t              Params;             /**< Test Parameter */
    ptx_FeliCaDTE_ProgressFn_t          ProgressCB;         /**< Optional Callback-Function to get notified about the current progress */
} ptx_FeliCaDTE_TestParams_t;

/**
 * \brief FeliCa-DTE Initialization Parameters
 */
typedef struct ptx_FeliCaDTE_InitParams
{
    ptx_RUL_t               *Rul;                /**< Pointer to initialised RUL component. */
    uint8_t                 EnableDDPC;          /**< If enabled, performs a DDPC cycle after every test. */
} ptx_FeliCaDTE_InitParams_t;

/**
 * \brief FeliCa-DTE Component
 */
typedef struct ptx_FeliCa_DTE
{
    /* Components */
    ptx_Comps_t              CompId;             /**< Component Id */

    ptx_RUL_t                *Rul;               /**< Pointer to initialised RUL component. */
    uint8_t                  EnableDDPC;         /**< If enabled, performs a DDPC cycle after every test. */

} ptx_FeliCaDTE_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initialize the FeliCa-DTE Component.
 *
 * \param[in]   feliCaDTE               Pointer to an existing instance of the FeliCa-DTE component.
 * \param[in]   initParams              Pointer to initialisation parameters.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_FeliCaDTE_Open (ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_InitParams_t *initParams);

/**
 * \brief Deinitialize the FeliCa-DTE Component.
 *
 * \param[in]   feliCaDTE               Pointer to an existing instance of the FeliCa-DTE component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_FeliCaDTE_Close (ptx_FeliCaDTE_t *feliCaDTE);

/**
 * \brief Enables/Disables the FeliCa-DTE mode in the system and configures the test parameters.
 *
 * \param[in]   feliCaDTE               Pointer to an existing instance of the FeliCa-DTE component.
 * \param[in]   enableMode              Enables (!= 0) or Disables (0) the FeliCa-DTE mode.
 * \param[in]   testParams              Various test parameters for FeliCa compliance tests.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_FeliCaDTE_EnableMode (ptx_FeliCaDTE_t *feliCaDTE, uint8_t enableMode, ptx_FeliCaDTE_TestParams_t *testParams);

/**
 * \brief Performs a FeliCa compliance test with given test parameters.
 *
 * \param[in]   feliCaDTE               Pointer to an existing instance of the FeliCa-DTE component.
 * \param[in]   testParams              Various test parameters for FeliCa compliance tests.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_FeliCaDTE_RunTest(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TestParams_t *testParams);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
