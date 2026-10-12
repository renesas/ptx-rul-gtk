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
    Module      : ptx_RfTest
    File        : ptx_RfTest.c

    Description : Implementation of RF-Test API
 */

#include "ptx_RfTest.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_RfTest_LogParameters(ptx_RfTest_t *rfTestComp, ptx_RfTest_InParams_t *testParams);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_RfTest_Init (ptx_RfTest_t *rfTestComp, ptx_RfTest_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != rfTestComp) && (NULL != initParams))
    {
        if (NULL != initParams->Nsc)
        {
            /* clear component */
            (void)memset(rfTestComp, 0, sizeof(ptx_RfTest_t));

            /* set members */
            rfTestComp->Nsc = initParams->Nsc;
            rfTestComp->Logger = initParams->Logger;

            /* assign Component ID */
            rfTestComp->CompId = Comp_RF_TEST;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }


    return status;
}

ptx_Status_t ptx_RfTest_Deinit(ptx_RfTest_t *rfTestComp)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rfTestComp, Comp_RF_TEST))
    {
        /* nothing to do here */
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_RfTest_RunTest(ptx_RfTest_t *rfTestComp, ptx_RfTest_InParams_t *testParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rfTestComp, Comp_RF_TEST) && (NULL != testParams))
    {

        status = ptx_RfTest_LogParameters(rfTestComp, testParams);

        if ((rfTestComp->Nsc != NULL))
        {
            ptx_NSC_RfTest_InParams_t inParams;
            memset(&inParams, 0, sizeof(ptx_NSC_RfTest_InParams_t));

            switch (testParams->ID)
            {
                case RF_TEST_ID_PRBS_9:
                    inParams.SeqId = NSC_RfTest_SeqId_PRBS_9;
                    inParams.Params.PRBSParams = testParams->Params.PRBSParams;
                    break;
                case RF_TEST_ID_PRBS_15:
                    inParams.SeqId = NSC_RfTest_SeqId_PRBS_15;
                    inParams.Params.PRBSParams = testParams->Params.PRBSParams;
                    break;
                case RF_TEST_ID_Carrier:
                    inParams.SeqId = NSC_RfTest_SeqId_Carrier;
                    inParams.Params.CarrierParams = testParams->Params.CarrierParams;
                    break;

                default:
                    /* INVALID PARAMETER already set */
                    break;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_RfTest(rfTestComp->Nsc, &inParams, NULL);
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

ptx_Status_t ptx_RfTest_StopTest(ptx_RfTest_t *rfTestComp)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rfTestComp, Comp_RF_TEST))
    {
        status = ptx_NSC_RfTestStop(rfTestComp->Nsc);
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK(s)
 * ####################################################################################################################
 */

static ptx_Status_t ptx_RfTest_LogParameters(ptx_RfTest_t *rfTestComp, ptx_RfTest_InParams_t *testParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rfTestComp, Comp_RF_TEST) && (NULL != testParams))
    {
        status = PTX_STATUS_SUCCESS;

        switch (testParams->ID)
        {
            case RF_TEST_ID_PRBS_9:
            /* FALL THROUGH */
            case RF_TEST_ID_PRBS_15:
                PTX_LOG_I(rfTestComp->Logger, "SeqId: %d", testParams->ID);
                PTX_LOG_I(rfTestComp->Logger, "Bitrate: %d", testParams->Params.PRBSParams.Bitrate);
                PTX_LOG_I(rfTestComp->Logger, "EnabCRC: %d", testParams->Params.PRBSParams.EnabCRC);
                PTX_LOG_I(rfTestComp->Logger, "EnabParityBits: %d", testParams->Params.PRBSParams.EnabParityBits);
                PTX_LOG_I(rfTestComp->Logger, "EnabSofEof: %d", testParams->Params.PRBSParams.EnabSofEof);
                PTX_LOG_I(rfTestComp->Logger, "EnabStartStopBits: %d", testParams->Params.PRBSParams.EnabStartStopBits);
                PTX_LOG_I(rfTestComp->Logger, "Technology: %d", testParams->Params.PRBSParams.Technology);
                break;
            case RF_TEST_ID_Carrier:
                PTX_LOG_I(rfTestComp->Logger, "SeqId: %d", testParams->ID);
                PTX_LOG_I(rfTestComp->Logger, "RffAmp: %d", testParams->Params.CarrierParams.RffAmp);
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }
        (void)testParams; /* For compilation if PTX_APP_LOGFAC is None */
    }
    return status;
}
