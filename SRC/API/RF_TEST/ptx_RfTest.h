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
    Module      : Radio Frequency (RF) Test module
    File        : ptx_RfTest.h

    Description : RF-Test API allows execution of RF tests to optimize and verify the emitted RF-Field.

*/

/**
 * \addtogroup RF_TEST
 *
 * @{
 */

#if !defined(PTX_RFTEST_H)
#define PTX_RFTEST_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>
#include "ptx_Status.h"
#include "ptx_NSC_RfTest.h"
#include "ptx_Log.h"

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
 * \brief RF-Test Flags for PRBS
 */
#define PTX_RF_TEST_FLAGS_PRBS_TYPE_A_INCLUDE_PARITY    (uint8_t)0x01   /**< Enable/Disable Parity-Generation for Type-A */

/**
 * \brief RF-Test Initialization Parameters
 */
typedef struct ptx_RfTest_InitParams
{
    ptx_NSC_t               *Nsc;                       /**< Initialized instance of NSC component (member of IoT-/POS-component) */
    ptx_Log_t               *Logger;                    /**< Initialized instance of Logger Component */
} ptx_RfTest_InitParams_t;

/**
 * \brief RF-Test Component
 */
typedef struct ptx_RfTest
{
    /* Components */
    ptx_Comps_t             CompId;                     /**< Component Id */

    ptx_NSC_t              *Nsc;                        /**< Initialized instance of NSC component (member of IoT-/POS-component) */
    ptx_Log_t              *Logger;                     /**< Initialized instance of Logger Component */

} ptx_RfTest_t;

/**
 * \brief RF-Test Test ID
 */
typedef enum ptx_RfTest_TestID
{
    RF_TEST_ID_PRBS_9,                                  /**< PRBS-9 Test */
    RF_TEST_ID_PRBS_15,                                 /**< PRBS-15 Test */
    RF_TEST_ID_Carrier,                                 /**< Unmodulated Carrier-On Test */

} ptx_RfTest_TestID_t;

/**
 * \brief RF-Test specific Parameters
 */
typedef union ptx_RfTest_Params
{
    ptx_NSC_RfTest_Params_PRBS_t    PRBSParams;         /**< PRBS Parameters */
    ptx_NSC_RfTest_Params_Carrier_t CarrierParams;      /**< Carrier Parameters */
} ptx_RfTest_Params_t;


/**
 * \brief RF-Test generic input Parameters
 */
typedef struct ptx_RfTest_InParams
{
    ptx_RfTest_TestID_t         ID;                     /**< RF-Test Identifier */
    ptx_RfTest_Params_t         Params;                 /**< RF-Test Parameters */
} ptx_RfTest_InParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initializes the RF-Test Component.
 *
 * \param[in]   rfTestComp              Pointer to an existing instance of the RF-Test component.
 * \param[in]   initParams              Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptx_Status_t ptx_RfTest_Init (ptx_RfTest_t *rfTestComp, ptx_RfTest_InitParams_t *initParams);

/**
 * \brief Deinitializes the RF-Test Component.
 *
 * \param[in]   rfTestComp              Pointer to an existing instance of the RF-Test component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_RfTest_Deinit (ptx_RfTest_t *rfTestComp);

/**
 * \brief Performs a given RF-Test using the provided test parameters.
 *
 * \param[in]   rfTestComp              Pointer to an existing instance of the RF-Test component.
 * \param[in]   testParams              Various test parameters for RF-Test compliance tests.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_RfTest_RunTest(ptx_RfTest_t *rfTestComp, ptx_RfTest_InParams_t *testParams);

/**
 * \brief Stops an ongoing RF-Test.
 *
 * \param[in]   rfTestComp              Pointer to an existing instance of the RF-Test component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_RfTest_StopTest(ptx_RfTest_t *rfTestComp);

#if defined(__cplusplus)
}
#endif

/** @} */

#endif /* Guard */


