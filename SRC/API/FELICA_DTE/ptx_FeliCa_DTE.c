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
    Module      : FELICA
    File        : ptx_FeliCa_DTE.c

    Description : Implementation of FeliCa DTE.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_FeliCa_DTE.h"
#include "ptx_RUL.h"
#include "ptx_NSC_RfSetParameter.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_FELICADTE_RW_DP_T_OFF_DEFAULT_MS            (uint32_t)30
#define PTX_FELICADTE_RW_DP_T_ON_DEFAULT_MS             (uint32_t)25
#define PTX_FELICADTE_PERFORMANCE_T_ON_DEFAULT_MS       (uint32_t)25
#define PTX_FELICADTE_RW_MIN_RESULT_BUFFER_SIZE         (uint32_t)256
#define PTX_FELICADTE_RX_BUFFER_SIZE                    (uint32_t)32
#define PTX_FELICADTE_MAX_RFPARAM_LEN                   (uint32_t)10
#define PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_COUNT         (uint8_t)3
#define PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_0             (uint8_t)0
#define PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_1             (uint8_t)1
#define PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_2             (uint8_t)2
#define PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_LENGTH        (uint8_t)6
#define PTX_FELICADTE_EMONEY_TEST_SEQ_LENGTH            (uint8_t)6
#define PTX_FELICADTE_SETDP_TEST_SEQ_LENGTH             (uint8_t)4
#define PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_6     (uint8_t)6
#define PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_16    (uint8_t)16
#define PTX_FELICADTE_SENSF_RES_LENGTH                  (uint32_t)18
#define PTX_FELICADTE_SENSF_RES_CONST                   (uint8_t)0x01

/*
 *  Temporary Test-data for Reader/Writer Digital Protocol Tests
 */
typedef struct ptx_FeliCaDTE_TempTestData
{
    ptx_FeliCaDTE_TestParams_t          *TestParams;
    ptx_FeliCaDTE_TestProgressParams_t  ProgressParams;
    ptx_Status_t                         TestStatus;
    uint8_t                             *Tx;
    uint32_t                            TxLen;
    uint8_t                             Rx[PTX_FELICADTE_RX_BUFFER_SIZE];
    uint32_t                            RxLen;

} ptx_FeliCaDTE_TempTestData_t;

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_FeliCaDTE_RunPerformanceTest(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TestParams_t *testParams);
static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TestParams_t *testParams);
static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest_eMoney(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TempTestData_t *tempTestData);
static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest_eMoneyNFC(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TempTestData_t *tempTestData);
static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest_FrameStruct(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TempTestData_t *tempTestData);
static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest_SetDP(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TempTestData_t *tempTestData);
static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest_Generic(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TempTestData_t *tempTestData);
static ptx_Status_t ptx_FeliCaDTE_Data_Exchange(ptx_FeliCaDTE_t *feliCaDTE, uint8_t *tx, uint32_t txLength, uint8_t *rx, uint32_t *rxLength, uint32_t timeoutMs);
static ptx_Status_t ptx_FeliCaDTE_HandleCarrier (ptx_FeliCaDTE_t *feliCaDTE, uint8_t switchOn);
static ptx_Status_t ptx_FeliCaDTE_Reset(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TestParams_t *testParams);
static ptx_Status_t ptx_FeliCaDTE_SetParams (ptx_FeliCaDTE_t *feliCaDTE, ptx_NSC_RfSetParameter_Param_Rf_Tech_t rfTech, uint8_t rfBitrate, uint32_t timeout);
static ptx_Status_t ptx_FeliCaDTE_Sleep (ptx_FeliCaDTE_t *feliCaDTE, uint32_t timeMS);
static ptx_Status_t ptx_FeliCaDTE_StoreTestSequences(ptx_FeliCaDTE_TempTestData_t *tempTestData);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_FeliCaDTE_Open (ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != feliCaDTE) && (NULL != initParams) && (NULL != initParams->Rul))
    {
        /* clear component */
        (void)memset(feliCaDTE, 0, sizeof(ptx_FeliCaDTE_t));

        /* set members */
        feliCaDTE->Rul = initParams->Rul;
        feliCaDTE->EnableDDPC = initParams->EnableDDPC;

        /* assign Component ID */
        feliCaDTE->CompId = Comp_FeliCa_DTE;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_FeliCaDTE_Close (ptx_FeliCaDTE_t *feliCaDTE)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(feliCaDTE, Comp_FeliCa_DTE))
    {
        memset(feliCaDTE, 0, sizeof(ptx_FeliCaDTE_t));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_FeliCaDTE_EnableMode (ptx_FeliCaDTE_t *feliCaDTE, uint8_t enableMode, ptx_FeliCaDTE_TestParams_t *testParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t guard_time;

    if (PTX_COMP_CHECK(feliCaDTE, Comp_FeliCa_DTE))
    {
        if (0 != enableMode)
        {
            if (NULL != testParams)
            {
                if (testParams->ID == FELICA_DTE_TestID_RWDigProtTest)
                {
                    guard_time = (NULL != testParams->Params.RWDigProtTest.T_OffGuardTime) ? *testParams->Params.RWDigProtTest.T_OffGuardTime : PTX_FELICADTE_RW_DP_T_OFF_DEFAULT_MS;
                    status = ptx_FeliCaDTE_Sleep(feliCaDTE, guard_time);
                }
                else
                {
                    /* Set status SUCCESS just to proceed with processing in the next step */
                    status = PTX_STATUS_SUCCESS;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_FeliCaDTE_HandleCarrier(feliCaDTE, enableMode);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        switch (testParams->ID)
                        {
                            case FELICA_DTE_TestID_PerformanceTest:

                                status = ptx_FeliCaDTE_Sleep(feliCaDTE, PTX_FELICADTE_PERFORMANCE_T_ON_DEFAULT_MS);

                                if (PTX_STATUS_SUCCESS == status)
                                {
                                    status = ptx_FeliCaDTE_SetParams(feliCaDTE,
                                                                     NSC_RfSetParameter_RfTech_NFC_F,
                                                                     (uint8_t)testParams->Params.PerformanceTest.Bitrate,
                                                                     testParams->Params.PerformanceTest.TimeOutMS);
                                }
                                break;

                            case FELICA_DTE_TestID_RWDigProtTest:
                                status = ptx_FeliCaDTE_SetParams(feliCaDTE,
                                                                 NSC_RfSetParameter_RfTech_NFC_F,
                                                                 (uint8_t)FELICA_DTE_BITRATE_212,
                                                                 testParams->Params.RWDigProtTest.TimeOutMS);

                                if (PTX_STATUS_SUCCESS == status)
                                {
                                    guard_time = (NULL != testParams->Params.RWDigProtTest.T_OnGuardTime) ? *testParams->Params.RWDigProtTest.T_OnGuardTime : PTX_FELICADTE_RW_DP_T_ON_DEFAULT_MS;
                                    status = ptx_FeliCaDTE_Sleep(feliCaDTE, guard_time);
                                }
                                break;

                            default:
                                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                                break;
                        }
                    }
                }

            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }

        }
        else
        {
            status = ptx_FeliCaDTE_SetParams(feliCaDTE, 0, 0, 0);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_FeliCaDTE_HandleCarrier(feliCaDTE, enableMode);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_FeliCaDTE_RunTest(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TestParams_t *testParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((PTX_COMP_CHECK(feliCaDTE, Comp_FeliCa_DTE)) && (NULL != testParams))
    {
        switch (testParams->ID)
        {
            case FELICA_DTE_TestID_PerformanceTest:
                status = ptx_FeliCaDTE_RunPerformanceTest(feliCaDTE, testParams);
                break;

            case FELICA_DTE_TestID_RWDigProtTest:
                status = ptx_FeliCaDTE_RunRFDigProtTest(feliCaDTE, testParams);
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
static ptx_Status_t ptx_FeliCaDTE_RunPerformanceTest(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TestParams_t *testParams)
{
    uint8_t TX_SEQ_SENSF_REQ[] = {0x06, 0x00, 0xFF, 0xFF, 0x00, 0x00};

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t rx_buffer[PTX_FELICADTE_RX_BUFFER_SIZE];
    uint32_t rx_len;

    ptx_FeliCaDTE_TestProgressParams_t progress_params;
    (void)memset(&progress_params, 0, sizeof(ptx_FeliCaDTE_TestProgressParams_t));

    if ((PTX_COMP_CHECK(feliCaDTE, Comp_FeliCa_DTE)) && (NULL != testParams))
    {
        if ((testParams->Params.PerformanceTest.NrTests <= testParams->Params.PerformanceTest.ResultBufferSize) &&
            (NULL != (testParams->Params.PerformanceTest.ResultBuffer)))
        {
            for (uint8_t i = 0; i < testParams->Params.PerformanceTest.NrTests; i++)
            {
                (void)memset(&rx_buffer[0], 0, PTX_FELICADTE_RX_BUFFER_SIZE);
                rx_len = PTX_FELICADTE_RX_BUFFER_SIZE;

                status = ptx_FeliCaDTE_Data_Exchange(feliCaDTE,
                                                     &TX_SEQ_SENSF_REQ[0],
                                                     (uint32_t)sizeof(TX_SEQ_SENSF_REQ),
                                                     &rx_buffer[0],
                                                     &rx_len,
                                                     testParams->Params.PerformanceTest.TimeOutMS);

                if (PTX_STATUS_SUCCESS == status)
                {
                    if ((PTX_FELICADTE_SENSF_RES_LENGTH != rx_len) || (PTX_FELICADTE_SENSF_RES_CONST != rx_buffer[1]))
                    {
                        status = PTX_STATUS_E_FELICA_PROTOCOL_ERROR;
                    }
                }

                progress_params.NrTestsProcessed++;

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Test OK */
                    testParams->Params.PerformanceTest.ResultBuffer[i] = 'O';

                }
                else if ((PTX_STATUS_E_GEN_TIMEOUT == status) || (PTX_STATUS_E_NSC_RFERROR == status))
                {
                    /* Test FAIL */
                    testParams->Params.PerformanceTest.ResultBuffer[i] = '.';

                    /* reset status */
                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    /* cancel test */
                    break;
                }

                /* inform upper layer about progress and provide option to quit (both optional) */
                if (NULL != testParams->ProgressCB)
                {
                    testParams->ProgressCB(&progress_params);
                }

                if (0 != progress_params.ExitProcessing)
                {
                    /* cancel test */
                    break;
                }

                if ((PTX_STATUS_SUCCESS == status) && (0u != feliCaDTE->EnableDDPC))
                {
                    status = ptx_FeliCaDTE_Reset(feliCaDTE, testParams);

                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TestParams_t *testParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_FeliCaDTE_TempTestData_t temp_test_data;
    (void)memset(&temp_test_data, 0, sizeof(ptx_FeliCaDTE_TempTestData_t));

    if ((PTX_COMP_CHECK(feliCaDTE, Comp_FeliCa_DTE)) && (NULL != testParams))
    {
        if ((PTX_FELICADTE_RW_MIN_RESULT_BUFFER_SIZE <= testParams->Params.RWDigProtTest.ResultBufferSize) &&
            (NULL != (testParams->Params.RWDigProtTest.ResultBuffer)))
        {
            (void)memset(&testParams->Params.RWDigProtTest.ResultBuffer[0], 0, testParams->Params.RWDigProtTest.ResultBufferSize);
            testParams->Params.RWDigProtTest.ResultBufferSize = 0;

            temp_test_data.TestParams = testParams;
            (void)memset(&temp_test_data.ProgressParams, 0, sizeof(ptx_FeliCaDTE_TestProgressParams_t));

            switch (testParams->Params.RWDigProtTest.SubTestID)
            {
                case FELICA_DTE_SubTestID_RWDigProt_eMoney_Variant1:
                case FELICA_DTE_SubTestID_RWDigProt_eMoney_Variant2:
                    status = ptx_FeliCaDTE_RunRFDigProtTest_eMoney(feliCaDTE, &temp_test_data);
                    break;

                case FELICA_DTE_SubTestID_RWDigProt_eMoney_NFCDEP:
                    status = ptx_FeliCaDTE_RunRFDigProtTest_eMoneyNFC(feliCaDTE, &temp_test_data);
                    break;

                case FELICA_DTE_SubTestID_RWDigProt_FrameStructure:
                case FELICA_DTE_SubTestID_RWDigProt_FrameStructure1:
                case FELICA_DTE_SubTestID_RWDigProt_FrameStructure2:
                case FELICA_DTE_SubTestID_RWDigProt_FrameStructure3:
                case FELICA_DTE_SubTestID_RWDigProt_FrameStructure4:
                case FELICA_DTE_SubTestID_RWDigProt_FrameStructure5:
                case FELICA_DTE_SubTestID_RWDigProt_FrameStructure6:
                    status = ptx_FeliCaDTE_RunRFDigProtTest_FrameStruct(feliCaDTE, &temp_test_data);
                    break;

                case FELICA_DTE_SubTestID_RWDigProt_Setup_DP_POS:
                case FELICA_DTE_SubTestID_RWDigProt_Setup_DP_NEG:
                    status = ptx_FeliCaDTE_RunRFDigProtTest_SetDP(feliCaDTE, &temp_test_data);
                    break;

                case FELICA_DTE_SubTestID_RWDigProt_Generic:
                    if ((NULL != testParams->Params.RWDigProtTest.GenericCmdBuffer) && (0 != testParams->Params.RWDigProtTest.GenericCmdBufferLen))
                    {
                        status = ptx_FeliCaDTE_RunRFDigProtTest_Generic(feliCaDTE, &temp_test_data);
                    }
                    else
                    {
                        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    }
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }

        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

    }

    if ((PTX_STATUS_SUCCESS == status) && (0u != feliCaDTE->EnableDDPC))
    {
        status = ptx_FeliCaDTE_Reset(feliCaDTE, testParams);
    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest_eMoney(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TempTestData_t *tempTestData)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != tempTestData)
    {
        uint8_t TX_SEQ_EMONEY_VARIANT_1[PTX_FELICADTE_EMONEY_TEST_SEQ_LENGTH] =  {0x06, 0x00, 0x12, 0xFC, 0x01, 0x00};
        uint8_t TX_SEQ_EMONEY_VARIANT_2[PTX_FELICADTE_EMONEY_TEST_SEQ_LENGTH] =  {0x06, 0x00, 0xFF, 0xFF, 0x01, 0x00};

        if (tempTestData->TestParams->Params.RWDigProtTest.SubTestID == FELICA_DTE_SubTestID_RWDigProt_eMoney_Variant1)
        {
            tempTestData->Tx          = &TX_SEQ_EMONEY_VARIANT_1[0];
            tempTestData->TxLen       = PTX_FELICADTE_EMONEY_TEST_SEQ_LENGTH;
        }
        else
        {
            tempTestData->Tx          = &TX_SEQ_EMONEY_VARIANT_2[0];
            tempTestData->TxLen       = PTX_FELICADTE_EMONEY_TEST_SEQ_LENGTH;
        }

        (void)memset(&tempTestData->Rx[0], 0, PTX_FELICADTE_RX_BUFFER_SIZE);
        tempTestData->RxLen = PTX_FELICADTE_RX_BUFFER_SIZE;

        tempTestData->TestStatus = ptx_FeliCaDTE_Data_Exchange(feliCaDTE, &tempTestData->Tx[0], (uint32_t)(tempTestData->TxLen), &tempTestData->Rx[0], &tempTestData->RxLen, tempTestData->TestParams->Params.RWDigProtTest.TimeOutMS);

        status = ptx_FeliCaDTE_StoreTestSequences(tempTestData);
    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest_eMoneyNFC(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TempTestData_t *tempTestData)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t TX_SEQ_EMONEY_NFC_SEQ_0[PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_LENGTH] = {0x06, 0x00, 0xFF, 0xFF, 0x00, 0x00};
    uint8_t TX_SEQ_EMONEY_NFC_SEQ_1[PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_LENGTH] = {0x06, 0x00, 0xFF, 0xFF, 0x00, 0x00};
    uint8_t TX_SEQ_EMONEY_NFC_SEQ_2[PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_LENGTH] = {0x06, 0x00, 0xFF, 0xFF, 0x01, 0x00};

    if (NULL != tempTestData)
    {
        for (int i = 0; i < PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_COUNT; i++)
        {
            switch (i)
            {
                case PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_1:
                    tempTestData->Tx          = &TX_SEQ_EMONEY_NFC_SEQ_1[0];
                    tempTestData->TxLen       = PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_LENGTH;
                    break;

                case PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_2:
                    tempTestData->Tx          = &TX_SEQ_EMONEY_NFC_SEQ_2[0];
                    tempTestData->TxLen       = PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_LENGTH;
                    break;

                case PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_0:
                /* FALL THROUGH */
                default:
                    tempTestData->Tx          = &TX_SEQ_EMONEY_NFC_SEQ_0[0];
                    tempTestData->TxLen       = PTX_FELICADTE_EMONEY_NFC_TEST_SEQ_LENGTH;
                    break;
            }

            (void)memset(&tempTestData->Rx[0], 0, PTX_FELICADTE_RX_BUFFER_SIZE);
            tempTestData->RxLen = PTX_FELICADTE_RX_BUFFER_SIZE;

            tempTestData->TestStatus = ptx_FeliCaDTE_Data_Exchange(feliCaDTE, &tempTestData->Tx[0], (uint32_t)(tempTestData->TxLen), &tempTestData->Rx[0], &tempTestData->RxLen, tempTestData->TestParams->Params.RWDigProtTest.TimeOutMS);

            status = ptx_FeliCaDTE_StoreTestSequences(tempTestData);

            tempTestData->ProgressParams.NrTestsProcessed++;

            /* inform upper layer about progress and provide option to quit (both optional) */
            if (NULL != tempTestData->TestParams->ProgressCB)
            {
                tempTestData->TestParams->ProgressCB(&tempTestData->ProgressParams);
            }

            if (0 != tempTestData->ProgressParams.ExitProcessing)
            {
                /* cancel test */
                break;
            }

            if (PTX_STATUS_SUCCESS != status)
            {
                break;
            }

            /* guard time before next Packet */
            (void)ptx_FeliCaDTE_Sleep(feliCaDTE, (uint32_t)5);
        }
    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest_FrameStruct(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TempTestData_t *tempTestData)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t TX_SEQ_FRAME_STRUCTURE_0[PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_6]  = {0x06, 0x00, 0xFF, 0xFF, 0x01, 0x03};
    uint8_t TX_SEQ_FRAME_STRUCTURE_1[PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_6]  = {0x06, 0x00, 0xFF, 0xFF, 0x01, 0x00};
    uint8_t TX_SEQ_FRAME_STRUCTURE_2[PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_16] = {0x10, 0x06, 0x02, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x05, 0x06, 0x01, 0x0B, 0x00, 0x01, 0x80, 0x04};
    uint8_t TX_SEQ_FRAME_STRUCTURE_3[PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_16] = {0x10, 0x06, 0x02, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x05, 0x06, 0x01, 0x0B, 0x00, 0x01, 0x80, 0x02};
    uint8_t TX_SEQ_FRAME_STRUCTURE_4[PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_16] = {0x10, 0x06, 0x02, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x05, 0x06, 0x01, 0x0B, 0x00, 0x01, 0x80, 0x05};
    uint8_t TX_SEQ_FRAME_STRUCTURE_5[PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_16] = {0x10, 0x06, 0x02, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x05, 0x06, 0x01, 0x0B, 0x00, 0x01, 0x80, 0x09};

    int sequence_start_nr;
    int sequence_end_nr;

    if (NULL != tempTestData)
    {
        /* determine which sequence to run */
        switch (tempTestData->TestParams->Params.RWDigProtTest.SubTestID)
        {
            case FELICA_DTE_SubTestID_RWDigProt_FrameStructure1:
                sequence_start_nr = 0;
                sequence_end_nr   = sequence_start_nr + 1;
                break;

            case FELICA_DTE_SubTestID_RWDigProt_FrameStructure2:
                sequence_start_nr = 1;
                sequence_end_nr   = sequence_start_nr + 1;
                break;

            case FELICA_DTE_SubTestID_RWDigProt_FrameStructure3:
                sequence_start_nr = 2;
                sequence_end_nr   = sequence_start_nr + 1;
                break;

            case FELICA_DTE_SubTestID_RWDigProt_FrameStructure4:
                sequence_start_nr = 3;
                sequence_end_nr   = sequence_start_nr + 1;
                break;

            case FELICA_DTE_SubTestID_RWDigProt_FrameStructure5:
                sequence_start_nr = 4;
                sequence_end_nr   = sequence_start_nr + 1;
                break;

            case FELICA_DTE_SubTestID_RWDigProt_FrameStructure6:
                sequence_start_nr = 5;
                sequence_end_nr   = sequence_start_nr + 1;
                break;

            /* all sequences */
            default:
                sequence_start_nr = 0;
                sequence_end_nr = 6;
                break;
        }

        for (int i = sequence_start_nr; i < sequence_end_nr; i++)
        {
            switch (i)
            {
                case 0:
                    tempTestData->Tx    = &TX_SEQ_FRAME_STRUCTURE_0[0];
                    tempTestData->TxLen = PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_6;
                    break;

                case 1:
                    tempTestData->Tx    = &TX_SEQ_FRAME_STRUCTURE_1[0];
                    tempTestData->TxLen = PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_6;
                    break;

                case 2:
                    tempTestData->Tx    = &TX_SEQ_FRAME_STRUCTURE_2[0];
                    tempTestData->TxLen = PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_16;
                    break;

                case 3:
                    tempTestData->Tx    = &TX_SEQ_FRAME_STRUCTURE_3[0];
                    tempTestData->TxLen = PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_16;
                    break;

                case 4:
                    tempTestData->Tx    = &TX_SEQ_FRAME_STRUCTURE_4[0];
                    tempTestData->TxLen = PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_16;
                    break;

                case 5:
                    tempTestData->Tx    = &TX_SEQ_FRAME_STRUCTURE_5[0];
                    tempTestData->TxLen = PTX_FELICADTE_FRAMESTRUCT_TEST_SEQ_LENGTH_16;
                    break;

                default:
                    /* not possible */
                    break;
            }

            (void)memset(&tempTestData->Rx[0], 0, PTX_FELICADTE_RX_BUFFER_SIZE);
            tempTestData->RxLen = PTX_FELICADTE_RX_BUFFER_SIZE;

            tempTestData->TestStatus = ptx_FeliCaDTE_Data_Exchange(feliCaDTE, &tempTestData->Tx[0], (uint32_t)(tempTestData->TxLen), &tempTestData->Rx[0], &tempTestData->RxLen, tempTestData->TestParams->Params.RWDigProtTest.TimeOutMS);

            status = ptx_FeliCaDTE_StoreTestSequences(tempTestData);

            tempTestData->ProgressParams.NrTestsProcessed++;

            /* inform upper layer about progress and provide option to quit (both optional) */
            if (NULL != tempTestData->TestParams->ProgressCB)
            {
                tempTestData->TestParams->ProgressCB(&tempTestData->ProgressParams);
            }

            if (0 != tempTestData->ProgressParams.ExitProcessing)
            {
                /* cancel test */
                break;
            }

            if (PTX_STATUS_SUCCESS != status)
            {
                break;
            }

            /* guard time before next Packet */
            (void)ptx_FeliCaDTE_Sleep(feliCaDTE, (uint32_t)5);
        }
    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest_SetDP(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TempTestData_t *tempTestData)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t TX_SEQ_SETP_DP_POS[PTX_FELICADTE_SETDP_TEST_SEQ_LENGTH] = {0x04, 0xFA, 0x01, 0x00};
    uint8_t TX_SEQ_SETP_DP_NEG[PTX_FELICADTE_SETDP_TEST_SEQ_LENGTH] = {0x04, 0xFA, 0x01, 0xFF};

    if (NULL != tempTestData)
    {
        if (tempTestData->TestParams->Params.RWDigProtTest.SubTestID == FELICA_DTE_SubTestID_RWDigProt_Setup_DP_POS)
        {
            tempTestData->Tx    = &TX_SEQ_SETP_DP_POS[0];
            tempTestData->TxLen = PTX_FELICADTE_SETDP_TEST_SEQ_LENGTH;

        }
        else
        {
            tempTestData->Tx    = &TX_SEQ_SETP_DP_NEG[0];
            tempTestData->TxLen = PTX_FELICADTE_SETDP_TEST_SEQ_LENGTH;
        }

        (void)memset(&tempTestData->Rx[0], 0, PTX_FELICADTE_RX_BUFFER_SIZE);
        tempTestData->RxLen = PTX_FELICADTE_RX_BUFFER_SIZE;

        tempTestData->TestStatus = ptx_FeliCaDTE_Data_Exchange(feliCaDTE, &tempTestData->Tx[0], (uint32_t)(tempTestData->TxLen), &tempTestData->Rx[0], &tempTestData->RxLen, tempTestData->TestParams->Params.RWDigProtTest.TimeOutMS);

        status = ptx_FeliCaDTE_StoreTestSequences(tempTestData);
    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_RunRFDigProtTest_Generic(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TempTestData_t *tempTestData)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != tempTestData)
    {
        tempTestData->Tx    = &tempTestData->TestParams->Params.RWDigProtTest.GenericCmdBuffer[0];
        tempTestData->TxLen = tempTestData->TestParams->Params.RWDigProtTest.GenericCmdBufferLen;

        (void)memset(&tempTestData->Rx[0], 0, PTX_FELICADTE_RX_BUFFER_SIZE);
        tempTestData->RxLen = PTX_FELICADTE_RX_BUFFER_SIZE;

        tempTestData->TestStatus = ptx_FeliCaDTE_Data_Exchange(feliCaDTE, &tempTestData->Tx[0], (uint32_t)(tempTestData->TxLen), &tempTestData->Rx[0], &tempTestData->RxLen, tempTestData->TestParams->Params.RWDigProtTest.TimeOutMS);

        status = ptx_FeliCaDTE_StoreTestSequences(tempTestData);
    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_Data_Exchange(ptx_FeliCaDTE_t *feliCaDTE, uint8_t *tx, uint32_t txLength, uint8_t *rx, uint32_t *rxLength, uint32_t timeoutMs)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((PTX_COMP_CHECK(feliCaDTE, Comp_FeliCa_DTE)) && (NULL != tx) && (0 != txLength) && (NULL != rxLength))
    {
        status = ptx_RUL_Exchange(feliCaDTE->Rul, tx, txLength, rx, rxLength, timeoutMs);
    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_HandleCarrier (ptx_FeliCaDTE_t *feliCaDTE, uint8_t switchOn)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != feliCaDTE)
    {
        if (0 != switchOn)
        {
            ptx_RUL_DiscoverConfig_t disc_cfg;
            memset(&disc_cfg, 0, sizeof(ptx_RUL_DiscoverConfig_t));
            disc_cfg.EnablePolling = 1;
            disc_cfg.PollMode = ptx_RUL_PollMode_ContinuousWave;
            disc_cfg.EnableDDPC = feliCaDTE->EnableDDPC;

            status = ptx_RUL_InitiateDiscovery(feliCaDTE->Rul, &disc_cfg);
        }
        else
        {
            ptx_RUL_DeactivateParams_t deactivateParams;
            memset(&deactivateParams, 0, sizeof(ptx_RUL_DeactivateParams_t));
            deactivateParams.Type = ptx_RUL_DeactivateType_Generic;
            deactivateParams.State = ptx_RUL_DeactivateState_Idle;
            status = ptx_RUL_Deactivate(feliCaDTE->Rul, &deactivateParams);
        }
    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_Reset(ptx_FeliCaDTE_t *feliCaDTE, ptx_FeliCaDTE_TestParams_t *testParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(feliCaDTE, Comp_FeliCa_DTE))
    {
        status = ptx_FeliCaDTE_EnableMode(feliCaDTE, 0u, testParams);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_FeliCaDTE_EnableMode(feliCaDTE, 1u, testParams);
        }
    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_SetParams (ptx_FeliCaDTE_t *feliCaDTE, ptx_NSC_RfSetParameter_Param_Rf_Tech_t rfTech, uint8_t rfBitrate, uint32_t timeout)
{
    /*
     * Internal uses a resolution of 128/fc per tick ~ 9,44us; 1 ms is roughly 106 ticks (~1.00059ms)
     * Note: The resolution is limited to 24-bits i.e. the max. value is roughly 158 s.
     */
    const uint32_t BASE_TIMEOUT_1_MS = (uint32_t)106;

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != feliCaDTE)
    {
        uint32_t rf_par_len = 0;
        ptx_NSC_RfSetParameter_Param_t rf_par[PTX_FELICADTE_MAX_RFPARAM_LEN];

        (void)memset(rf_par, 0, PTX_FELICADTE_MAX_RFPARAM_LEN * sizeof(ptx_NSC_RfSetParameter_Param_t));

        /* RF-Technology (default) */
        rf_par[rf_par_len].Type = NSC_RfSetParameter_Param_Type_RfTech;
        rf_par[rf_par_len].Value.RfTech = (ptx_NSC_RfSetParameter_Param_Rf_Tech_t)rfTech;
        rf_par_len++;

        /* FWT (default)*/
        uint32_t timeout_val = (uint32_t)((BASE_TIMEOUT_1_MS * timeout) & 0x00FFFFFF);
        rf_par[rf_par_len].Type = NSC_RfSetParameter_Param_Type_Fwt;
        rf_par[rf_par_len].Value.Fwt.FwtValue[2] = (uint8_t)((timeout_val >> 16) & (uint8_t)0xFF);
        rf_par[rf_par_len].Value.Fwt.FwtValue[1] = (uint8_t)((timeout_val >> 8)  & (uint8_t)0xFF);
        rf_par[rf_par_len].Value.Fwt.FwtValue[0] = (uint8_t)((timeout_val >> 0)  & (uint8_t)0xFF);
        rf_par_len++;

        /* TX-Bitrate */
        rf_par[rf_par_len].Type = NSC_RfSetParameter_Param_Type_TxBitRate;
        rf_par[rf_par_len].Value.TxBitRate.BitrateValue = rfBitrate;
        rf_par_len++;

        /* RX-Bitrate */
        rf_par[rf_par_len].Type = NSC_RfSetParameter_Param_Type_RxBitRate;
        rf_par[rf_par_len].Value.RxBitRate.BitrateValue = rfBitrate;
        rf_par_len++;

        /* TRX-CRC */
        rf_par[rf_par_len].Type = NSC_RfSetParameter_Param_Type_Crc;
        rf_par[rf_par_len].Value.Crc.TxCrc = (uint8_t)1;
        rf_par[rf_par_len].Value.Crc.RxCrc = (uint8_t)1;
        rf_par_len++;

        /* RES_LIMIT */
        rf_par[rf_par_len].Type = NSC_RfSetParameter_Param_Type_MaxRspLim;
        rf_par[rf_par_len].Value.MaxRspLim.MaxRspLimValue = (uint8_t)1;
        rf_par_len++;

        status = ptx_NSC_RfSetParameter(&(feliCaDTE->Rul->Nsc), rf_par, rf_par_len);
    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_Sleep (ptx_FeliCaDTE_t *feliCaDTE, uint32_t timeMS)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != feliCaDTE) && (0 != timeMS))
    {
        status = ptx_RUL_Sleep(feliCaDTE->Rul, timeMS);
    }

    return status;
}

static ptx_Status_t ptx_FeliCaDTE_StoreTestSequences(ptx_FeliCaDTE_TempTestData_t *tempTestData)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != tempTestData)
    {
        uint32_t index = (uint32_t)(tempTestData->TestParams->Params.RWDigProtTest.ResultBufferSize);

        tempTestData->TestParams->Params.RWDigProtTest.ResultBuffer[index] = (uint8_t)(tempTestData->TxLen + 1);
        index++;

        (void)memcpy(&tempTestData->TestParams->Params.RWDigProtTest.ResultBuffer[index], &tempTestData->Tx[0], tempTestData->TxLen);
        index = (uint32_t)(index + tempTestData->TxLen);

        if ((0 != tempTestData->RxLen) && (PTX_STATUS_SUCCESS == tempTestData->TestStatus))
        {
            uint32_t rx_len = (uint32_t)(tempTestData->RxLen - 1U);

            tempTestData->TestParams->Params.RWDigProtTest.ResultBuffer[index] = (uint8_t)(rx_len + 1U);
            index++;

            (void)memcpy(&tempTestData->TestParams->Params.RWDigProtTest.ResultBuffer[index], &tempTestData->Rx[0], rx_len);
            index = (uint32_t)(index + rx_len);

            status = PTX_STATUS_SUCCESS;
        }

        tempTestData->TestParams->Params.RWDigProtTest.ResultBufferSize = index;

        if (PTX_STATUS_SUCCESS != tempTestData->TestStatus)
        {
            status = tempTestData->TestStatus;
        }
    }

    return status;
}
