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
    Module      : NFC Forum Device Test Application (DTA)
    File        : ptx_NFCForumDTA.c

    Description : NFC Forum Device Test Application (DTA) contains the implementation of the Test Application
                  defined by NFC Forum, which can be used for NFC Forum Compliance Testing.
 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NFCForumDTA.h"

#include "ptx_Status.h"

#include "ptx_NSC_RfSetParameter.h"


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NFCFORUMDTA_RESTART_MAX_RETRIES (5u)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 *  Initialize the DTA Configuration to the default values
 *
 *  dta         Pointer to NFC Forum DTA component.
 *  testCfg     Pointer to test parameters.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
static ptx_Status_t ptx_NFCForumDTA_InitConfig(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);

/*
 *  Restart the System in case of a recoverable System Error
 *
 *  dta         Pointer to NFC Forum DTA component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
static ptx_Status_t ptx_NFCForumDTA_RestartSystem(ptx_NFCForumDTA_t *dta);

/*
 *  Initialize the Logic Link Control Protocol (LLCP) component.
 *
 *  dta         Pointer to NFC Forum DTA component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
static ptx_Status_t ptx_NFCForumDTA_InitLlcp(ptx_NFCForumDTA_t *dta);

/*
 *  Initialize the Simple NDEF Exchange Protocol (SNEP) Server or Client components.
 *
 *  dta         Pointer to NFC Forum DTA component.
 *  testCfg     Pointer to the DTA test configuration.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
static ptx_Status_t ptx_NFCForumDTA_InitSnep(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);

/*
 *  De-Initialize the Simple NDEF Exchange Protocol (SNEP) Server or Client components.
 *
 *  dta         Pointer to NFC Forum DTA component.
 *  testCfg     Pointer to the DTA test configuration.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*/
static ptx_Status_t ptx_NFCForumDTA_DeInitSnep(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NFCForumDTA_GetVersion(uint8_t *version)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != version)
    {
        *version = 13;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NFCForumDTA_Init(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != dta) && (NULL != initParams))
    {
        memset(dta, 0, sizeof(ptx_NFCForumDTA_t));

        if (PTX_COMP_CHECK(initParams->rul, Comp_RUL))
        {
            dta->rul = initParams->rul;

#if !defined(PTX_DISABLE_LISTEN_MODE)
            ptx_RUL_HCE_InitParams_t hce_init_params;
            memset(&hce_init_params, 0, sizeof(ptx_RUL_HCE_InitParams_t));

            hce_init_params.Rul = dta->rul;
            hce_init_params.AppRxBuffer = initParams->HCEMessageBuffer;
            hce_init_params.AppRxBufferSize = initParams->HCEMessageBufferSize;
            hce_init_params.Log = &initParams->rul->Log;
            status = ptx_RUL_HCE_Init(&dta->hce, &hce_init_params);
#else // PTX_DISABLE_LISTEN_MODE
            status = PTX_STATUS_SUCCESS;
#endif // PTX_DISABLE_LISTEN_MODE
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            dta->cardRegistry = &dta->rul->CardRegistry;
            dta->NDEFMessageBuffer = initParams->NDEFMessageBuffer;
            dta->NDEFMessageBufferSize = initParams->NDEFMessageBufferSize;
            dta->NDEFMessageBufferLen = initParams->NDEFMessageBufferLen;
            dta->applicationCallback = initParams->applicationCallback;
            dta->CompId = Comp_NFC_Forum_DTA;
        }
    }

    return status;
}

ptx_Status_t ptx_NFCForumDTA_DeInit(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
#if !defined(PTX_DISABLE_LISTEN_MODE)
        // Deinitialize HCE component
        status = ptx_RUL_HCE_DeInit(&dta->hce);
#else // PTX_DISABLE_LISTEN_MODE
        status = PTX_STATUS_SUCCESS;
#endif //PTX_DISABLE_LISTEN_MODE

        if (PTX_STATUS_SUCCESS == status)
        {
            // Deinitialize LLCP component
            status = ptx_RUL_LLCP_DeInit(&dta->llcp);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            // Invalidate NFC Forum DTA component
            dta->CompId = Comp_None;
        }

    }

    return status;

}

ptx_Status_t ptx_NFCForumDTA_StartDTARequest(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        dta->State = ptx_NFCForumDTA_State_INIT;
        dta->quitDTA = 0;

        while (0 == dta->quitDTA)
        {
            switch (dta->State)
            {
                case ptx_NFCForumDTA_State_INIT:
                    status = ptx_NFCForumDTA_InitConfig(dta, testCfg);
                    break;

                case ptx_NFCForumDTA_State_START_RF_DISCOVERY:
                    (void) ptx_RUL_LLCP_DeInit(&dta->llcp);
                    (void) ptx_NFCForumDTA_DeInitSnep(dta, testCfg);
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        status = ptx_NFCForumDTA_InitLlcp(dta);
                    }

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        status = ptx_NFCForumDTA_InitSnep(dta, testCfg);
                    }

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        status = ptx_NFCForumDTA_Start_RF_Discovery(dta, testCfg);
                    }
                    break;

                case ptx_NFCForumDTA_State_WAIT_4_ACTIVATION:
                    status = ptx_NFCForumDTA_ProcessDiscovery(dta, testCfg);
                    break;

                case ptx_NFCForumDTA_State_POLL_SELECT_DEVICE:
                    status = ptx_NFCForumDTA_ProcessMultipleCards(dta, testCfg);
                    break;

                case ptx_NFCForumDTA_State_POLL_DATA_EXCHANGE:
                    status = ptx_NFCForumDTA_ProcessPollDataExchange(dta, testCfg);
                    break;

#if !defined(PTX_DISABLE_LISTEN_MODE)
                case ptx_NFCForumDTA_State_LISTEN_PROCESS_EVENTS:
                    status = ptx_NFCForumDTA_ProcessListenEvents(dta, testCfg);
                    break;
#endif // PTX_DISABLE_LISTEN_MODE

                case ptx_NFCForumDTA_State_PROCESS_LLCP:
                    /**
                     * We already detected the magic bytes for LLCP, hoewever at this
                     * point we still have to decide if we want to run the LLCP tests
                     * or the SNEP tests (which build upon LLCP).
                     */
                    if (testCfg->PATTERN_NUMBER < PTX_NFCFORUMDTA_SNEP_PATTERNNUMBER_BOUNDARY)
                    {
                        status = ptx_NFCForumDTA_RunLlcpTestOperation(dta, testCfg);
                    }
                    else
                    {
                        status = ptx_NFCForumDTA_RunSnepTestOperation(dta, testCfg);
                    }
                    break;

                case ptx_NFCForumDTA_State_DEACTIVATE_DISCOVER:     // FALL THROUGH
                case ptx_NFCForumDTA_State_DEACTIVATE_IDLE:         // FALL THROUGH
                case ptx_NFCForumDTA_State_DEACTIVATE_SLEEP:        // FALL THROUGH
                case ptx_NFCForumDTA_State_DEACTIVATE_DESLECT:      // FALL THROUGH
                case ptx_NFCForumDTA_State_DEACTIVATE_RELEASE:      // FALL THROUGH
                case ptx_NFCForumDTA_State_DEACTIVATE_LISTEN_SLEEP: // FALL THROUGH
                    status = ptx_NFCForumDTA_Deactivate(dta, dta->State);
                    break;

                case ptx_NFCForumDTA_State_WAIT_4_USER_INPUT:
                    // Waiting for User Input not implemented
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                    dta->quitDTA = 1;
                    break;

                case ptx_NFCForumDTA_State_RESTART_RF_DISCOVERY:
                    dta->State = ptx_NFCForumDTA_State_START_RF_DISCOVERY;
                    break;

                case ptx_NFCForumDTA_State_RESTART_SYSTEM:
                    status = ptx_NFCForumDTA_RestartSystem(dta);
                    break;

                case ptx_NFCForumDTA_State_FINISHED:
                    if (dta->DTAConfiguration.REACTIVATION)
                    {
                        dta->State = ptx_NFCForumDTA_State_RESTART_RF_DISCOVERY;
                    }
                    else
                    {
                        dta->quitDTA = 1;
                        status = PTX_STATUS_SUCCESS;
                    }
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                    break;
            }

            if (PTX_STATUS_SUCCESS != status)
            {
                dta->quitDTA = 1;
                (void)ptx_NFCForumDTA_Deactivate(dta, ptx_NFCForumDTA_State_DEACTIVATE_IDLE);
                (void)ptx_NFCForumDTA_DeInitSnep(dta, testCfg);
            }

            if (NULL != dta->applicationCallback)
            {
                dta->applicationCallback(dta);
            }
        }
    }

    return status;

}

ptx_Status_t ptx_NFCForumDTA_RepeatTest(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg, uint32_t repetitions, uint8_t endless)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        if (0 == endless)
        {
            for (uint32_t i = 0; i < repetitions; i++)
            {
                status = ptx_NFCForumDTA_StartDTARequest(dta, testCfg);

                if (NULL != dta->applicationCallback)
                {
                    dta->applicationCallback(dta);
                }
            }
        }
        else
        {
            dta->quitEndlessExecution = 0;
            while (!dta->quitEndlessExecution)
            {
                status = ptx_NFCForumDTA_StartDTARequest(dta, testCfg);

                if (NULL != dta->applicationCallback)
                {
                    dta->applicationCallback(dta);
                }
            }
        }
    }

    return status;

}

ptx_Status_t ptx_NFCForumDTA_RegisterApplicationCallback(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_ApplicationCallback_t cb)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != cb))
    {
        dta->applicationCallback = cb;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NFCForumDTA_InitConfig(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        memset(&dta->DTAConfiguration, 0x00, sizeof(ptx_NFCForumDTA_Config_t));

        dta->DTAConfiguration.CON_ANTICOLL = 1;

        // Poll Mode Parameters
        dta->DTAConfiguration.CON_POLL_ACM = 0;
        dta->DTAConfiguration.CON_POLL_A =   1;
        dta->DTAConfiguration.CON_POLL_F =   1;
        dta->DTAConfiguration.CON_POLL_V =   1;

        dta->DTAConfiguration.CON_EXT_SENSB_RES = 0;

        dta->DTAConfiguration.CON_BAIL_OUT_A = 0;
        dta->DTAConfiguration.CON_BAIL_OUT_B = 0;
        dta->DTAConfiguration.CON_BAIL_OUT_F = 0;

        dta->DTAConfiguration.CON_DEVICES_LIMIT_A = 5;
        dta->DTAConfiguration.CON_DEVICES_LIMIT_B = 5;
        dta->DTAConfiguration.CON_DEVICES_LIMIT_F = 5;
        dta->DTAConfiguration.CON_DEVICES_LIMIT_V = 2;

        dta->DTAConfiguration.T4AT_NFC_DEP_PRIORITY = 1;
        dta->DTAConfiguration.T3T_NFC_DEP_PRIORITY  = 1;
        dta->DTAConfiguration.CON_BITR              = 2;

        // Listen Mode parameters
        dta->DTAConfiguration.CON_LISTEN_ACM =   0;
        dta->DTAConfiguration.CON_LISTEN_DEP_A = 0;
        dta->DTAConfiguration.CON_LISTEN_T4ATP = 0;
        dta->DTAConfiguration.CON_LISTEN_T4BTP = 0;

        dta->DTAConfiguration.CON_SYS_CODE =  NULL;
        dta->DTAConfiguration.CON_SENSF_RES = NULL;

        // Activation Activity parameters
        dta->DTAConfiguration.CON_ATR =       NULL;
        dta->DTAConfiguration.CON_GB =           0;
        dta->DTAConfiguration.CON_ATTRIB =    NULL;
        dta->DTAConfiguration.CON_RATS =         0;
        dta->DTAConfiguration.CON_BITR_NFC_DEP = 0;


        // Pattern Number dependant parameters
        switch (testCfg->PATTERN_NUMBER)
        {
            case 0x0000:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 2;
                dta->DTAConfiguration.CON_POLL_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_NFC_DEP      = 0;
                dta->DTAConfiguration.T3T_NFC_DEP_PRIORITY  = 0;
                dta->DTAConfiguration.REACTIVATION          = 0;
                dta->DTAConfiguration.CON_LISTEN_DEP_F      = 1;
                dta->DTAConfiguration.CON_LISTEN_T3TP       = 0;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x0001:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 2;
                dta->DTAConfiguration.CON_POLL_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_ACM          = 2;
                dta->DTAConfiguration.CON_BITR_NFC_DEP      = 0;
                dta->DTAConfiguration.T3T_NFC_DEP_PRIORITY  = 1;
                dta->DTAConfiguration.REACTIVATION          = 0;
                dta->DTAConfiguration.CON_LISTEN_DEP_F      = 0;
                dta->DTAConfiguration.CON_LISTEN_T3TP       = 1;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x0011: /* FALL THROUGH */
            case 0x0012: /* FALL THROUGH */
            case 0x0021: /* FALL THROUGH */
            case 0x0031:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.REACTIVATION          = 0;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x0002:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 2;
                dta->DTAConfiguration.CON_POLL_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_ACM          = 3;
                dta->DTAConfiguration.CON_BITR_NFC_DEP      = 0;
                dta->DTAConfiguration.REACTIVATION          = 0;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x0003:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 2;
                dta->DTAConfiguration.CON_POLL_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_NFC_DEP      = 3;
                dta->DTAConfiguration.REACTIVATION          = 0;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x0004:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 2;
                dta->DTAConfiguration.CON_POLL_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_ACM          = 2;
                dta->DTAConfiguration.CON_BITR_NFC_DEP      = 3;
                dta->DTAConfiguration.REACTIVATION          = 0;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x0005:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 3;
                dta->DTAConfiguration.CON_POLL_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_NFC_DEP      = 0;
                dta->DTAConfiguration.T3T_NFC_DEP_PRIORITY  = 0;
                dta->DTAConfiguration.REACTIVATION          = 0;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x0006:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 3;
                dta->DTAConfiguration.CON_POLL_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_ACM          = 3;
                dta->DTAConfiguration.T3T_NFC_DEP_PRIORITY  = 1;
                dta->DTAConfiguration.REACTIVATION          = 0;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x0007:
                dta->DTAConfiguration.CON_POLL_B            = 0;
                dta->DTAConfiguration.CON_BITR              = 2;
                dta->DTAConfiguration.CON_POLL_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_ACM          = 1;
                dta->DTAConfiguration.REACTIVATION          = 1;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x0008:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 3;
                dta->DTAConfiguration.REACTIVATION          = 1;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x0009:
                dta->DTAConfiguration.CON_POLL_B            = 0;
                dta->DTAConfiguration.CON_BITR              = 2;
                dta->DTAConfiguration.CON_POLL_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_ACM          = 2;
                dta->DTAConfiguration.REACTIVATION          = 0;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x000A:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 3;
                dta->DTAConfiguration.CON_POLL_ACM          = 1;
                dta->DTAConfiguration.CON_BITR_ACM          = 2;
                dta->DTAConfiguration.REACTIVATION          = 0;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x000B:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 2;
                dta->DTAConfiguration.REACTIVATION          = 0;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x1200: /* FALL THROUGH */
            case 0x1240: /* FALL THROUGH */
            case 0x1280:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 2;
                dta->DTAConfiguration.CON_BITR_NFC_DEP      = 0;
                dta->DTAConfiguration.REACTIVATION          = 0;
                dta->DTAConfiguration.CON_LISTEN_DEP_F      = 1;
                dta->DTAConfiguration.CON_LISTEN_T3TP       = 0;
                dta->DTAConfiguration.CON_GB                = 1;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x1201: /* FALL THROUGH */
            case 0x1241: /* FALL THROUGH */
            case 0x1281:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 3;
                dta->DTAConfiguration.CON_BITR_NFC_DEP      = 0;
                dta->DTAConfiguration.REACTIVATION          = 0;
                dta->DTAConfiguration.CON_LISTEN_DEP_F      = 1;
                dta->DTAConfiguration.CON_LISTEN_T3TP       = 0;
                dta->DTAConfiguration.CON_GB                = 1;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x1300: /* FALL THROUGH */
            case 0x1301: /* FALL THROUGH */
            case 0x1302: /* FALL THROUGH */
            case 0x1303: /* FALL THROUGH */
            case 0x1340: /* FALL THROUGH */
            case 0x1341: /* FALL THROUGH */
            case 0x1342: /* FALL THROUGH */
            case 0x1343:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 2;
                dta->DTAConfiguration.CON_BITR_NFC_DEP      = 0;
                dta->DTAConfiguration.REACTIVATION          = 0;
                dta->DTAConfiguration.CON_LISTEN_DEP_F      = 1;
                dta->DTAConfiguration.CON_LISTEN_T3TP       = 0;
                dta->DTAConfiguration.CON_GB                = 1;
                status = PTX_STATUS_SUCCESS;
                break;

            case 0x1310: /* FALL THROUGH */
            case 0x1311: /* FALL THROUGH */
            case 0x1312: /* FALL THROUGH */
            case 0x1313: /* FALL THROUGH */
            case 0x1350: /* FALL THROUGH */
            case 0x1351: /* FALL THROUGH */
            case 0x1352: /* FALL THROUGH */
            case 0x1353:
                dta->DTAConfiguration.CON_POLL_B            = 1;
                dta->DTAConfiguration.CON_BITR              = 3;
                dta->DTAConfiguration.CON_BITR_NFC_DEP      = 0;
                dta->DTAConfiguration.REACTIVATION          = 0;
                dta->DTAConfiguration.CON_LISTEN_DEP_F      = 1;
                dta->DTAConfiguration.CON_LISTEN_T3TP       = 0;
                dta->DTAConfiguration.CON_GB                = 1;
                status = PTX_STATUS_SUCCESS;
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        dta->State = ptx_NFCForumDTA_State_START_RF_DISCOVERY;
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_InitLlcp(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status;

    ptx_RUL_LLCP_InitParameters_t llcp_init_params;
    memset(&llcp_init_params, 0, sizeof(ptx_RUL_LLCP_InitParameters_t));
    llcp_init_params.Rul = dta->rul;

    memset(&dta->llcp, 0, sizeof(ptx_RUL_LLCP_t));
    status = ptx_RUL_LLCP_Init(&dta->llcp, &llcp_init_params);

    /* Setup the link structure for the connection less inbound SAP. */
    if (PTX_STATUS_SUCCESS == status)
    {
        ptx_RUL_LLCP_Link_t *ClInbound;
        ptx_RUL_LLCP_Link_InitParams_t init_params;
        memset(&init_params, 0, sizeof(ptx_RUL_LLCP_Link_InitParams_t));
        init_params.Type            = LinkType_ConnectionLess;
        init_params.Ssap            = PTX_NFCFORUMDTA_LLCP_INBOUND_CL_SAP_ADDRESS;
        init_params.Window          = PTX_LLCP_DEFAULT_RW;
        init_params.Miux            = PTX_NFCFORUMDTA_LLCP_DEFAULT_MIUX;
        init_params.ServiceUri      = PTX_NFCFORUMDTA_LLCP_INBOUND_CL_ECHO_SERVICE_NAME;
        init_params.ServiceUriLen   = strlen(PTX_NFCFORUMDTA_LLCP_INBOUND_CL_ECHO_SERVICE_NAME);

        status = ptx_RUL_LLCP_RegisterSap(&dta->llcp, &ClInbound, &init_params);
    }

    /* Setup the link structure for the connection oriented inbound SAP. */
    if (PTX_STATUS_SUCCESS == status)
    {
        ptx_RUL_LLCP_Link_t *CoInbound;
        ptx_RUL_LLCP_Link_InitParams_t init_params;
        memset(&init_params, 0, sizeof(ptx_RUL_LLCP_Link_InitParams_t));
        init_params.Type            = LinkType_ConnectionOriented;
        init_params.Ssap            = PTX_NFCFORUMDTA_LLCP_INBOUND_CO_SAP_ADDRESS;
        init_params.Window          = PTX_LLCP_DEFAULT_RW;
        init_params.Miux            = PTX_NFCFORUMDTA_LLCP_DEFAULT_MIUX;
        init_params.ServiceUri      = PTX_NFCFORUMDTA_LLCP_INBOUND_CO_ECHO_SERVICE_NAME;
        init_params.ServiceUriLen   = strlen(PTX_NFCFORUMDTA_LLCP_INBOUND_CO_ECHO_SERVICE_NAME);

        status = ptx_RUL_LLCP_RegisterSap(&dta->llcp, &CoInbound, &init_params);
    }

    /* Setup the link structure for the connection less outbound SAP. */
    if (PTX_STATUS_SUCCESS == status)
    {
        ptx_RUL_LLCP_Link_t *ClOutbound;
        ptx_RUL_LLCP_Link_InitParams_t init_params;
        memset(&init_params, 0, sizeof(ptx_RUL_LLCP_Link_InitParams_t));
        init_params.Type            = LinkType_ConnectionLess;
        init_params.Ssap            = PTX_NFCFORUMDTA_LLCP_OUTBOUND_CL_SAP_ADDRESS;
        init_params.Window          = PTX_LLCP_DEFAULT_RW;
        init_params.Miux            = PTX_NFCFORUMDTA_LLCP_DEFAULT_MIUX;
        init_params.ServiceUri      = PTX_NFCFORUMDTA_LLCP_OUTBOUND_CL_ECHO_SERVICE_NAME;
        init_params.ServiceUriLen   = strlen(PTX_NFCFORUMDTA_LLCP_OUTBOUND_CL_ECHO_SERVICE_NAME);

        status = ptx_RUL_LLCP_RegisterSap(&dta->llcp, &ClOutbound, &init_params);
    }

    /* Setup the link structure for the connection oriented outbound SAP. */
    if (PTX_STATUS_SUCCESS == status)
    {
        ptx_RUL_LLCP_Link_t *CoOutbound;
        ptx_RUL_LLCP_Link_InitParams_t init_params;
        memset(&init_params, 0, sizeof(ptx_RUL_LLCP_Link_InitParams_t));
        init_params.Type            = LinkType_ConnectionOriented;
        init_params.Ssap            = PTX_NFCFORUMDTA_LLCP_OUTBOUND_CO_SAP_ADDRESS;
        init_params.Window          = PTX_LLCP_DEFAULT_RW;
        init_params.Miux            = PTX_NFCFORUMDTA_LLCP_DEFAULT_MIUX;
        init_params.ServiceUri      = PTX_NFCFORUMDTA_LLCP_OUTBOUND_CO_ECHO_SERVICE_NAME;
        init_params.ServiceUriLen   = strlen(PTX_NFCFORUMDTA_LLCP_OUTBOUND_CO_ECHO_SERVICE_NAME);

        status = ptx_RUL_LLCP_RegisterSap(&dta->llcp, &CoOutbound, &init_params);
    }


    return status;
}

static ptx_Status_t ptx_NFCForumDTA_InitSnep(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        memset(&dta->snepClient, 0, sizeof(ptx_RUL_SnepClient_t));
        memset(&dta->snepServer, 0, sizeof(ptx_RUL_SnepServer_t));

        uint8_t dta_mode;
        if (PTX_NFCFORUMDTA_SNEPSERVER_PATTERN == (testCfg->PATTERN_NUMBER & PTX_NFCFORUMDTA_SNEP_PATTERNNUMBER_MASK))
        {
            dta_mode = (PTX_NFCFORUMDTA_SNEPSERVER_DTA_MODE_MASK == (testCfg->PATTERN_NUMBER & PTX_NFCFORUMDTA_SNEPSERVER_DTA_MODE_MASK) ? 1 : 0);
            status = ptx_RUL_SnepServer_Init(&dta->snepServer, &dta->llcp, dta_mode);
        }
        else
        {
            dta_mode = (PTX_NFCFORUMDTA_SNEPCLIENT_PATTERN_GET_REQ == (testCfg->PATTERN_NUMBER & PTX_NFCFORUMDTA_SNEP_PATTERNNUMBER_MASK) ? 1 : 0);
            status = ptx_RUL_SnepClient_Init(&dta->snepClient, &dta->llcp, dta_mode);
        }
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_DeInitSnep(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        if (PTX_NFCFORUMDTA_SNEPSERVER_PATTERN == (testCfg->PATTERN_NUMBER & PTX_NFCFORUMDTA_SNEP_PATTERNNUMBER_MASK))
        {
            status = ptx_RUL_SnepServer_Stop(&dta->snepServer);
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_SnepServer_DeInit(&dta->snepServer);
            }
        }
        else
        {
            status = ptx_RUL_SnepClient_DeInit(&dta->snepClient);
        }
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_RestartSystem(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
        uint8_t retries = 0;

        do
        {
            /* Wait for 1s to give the chip time to cool down, after that time try to initialize the system again */
            status = ptx_RUL_Sleep(dta->rul, 1000u);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_InitHardware(dta->rul);
            }

            ++retries;

        }
        while ((status != PTX_STATUS_SUCCESS) && (PTX_NFCFORUMDTA_RESTART_MAX_RETRIES > retries));

        if (PTX_STATUS_SUCCESS == status)
        {
            dta->State = ptx_NFCForumDTA_State_INIT;
        }
    }

    return status;
}
