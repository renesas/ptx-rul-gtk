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
    Module      : NFC Forum Device Test Application (DTA) SNEP Processing
    File        : ptx_NFCForumDTA_SNEP_Processing.c

    Description : Contains the implementation to handle SNEP in the NFC Forum DTA. It controls the processing if the
                  NFC Forum DTA is running in SNEP Mode for P2P communication.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NFCForumDTA_SNEP_Processing.h"

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static const uint8_t SNEP_NORMAL_RECORD[PTX_SNEP_DTA_NORMAL_NDEF_RECORD_LENGTH] =
{
    0xC1, 0x01, 0x00, 0x00, 0x00, 0x1E, 0x54, 0x02, 0x6C, 0x61, 0x4C, 0x6F, 0x72,
    0x65, 0x6D, 0x20, 0x69, 0x70, 0x73, 0x75, 0x6D, 0x20, 0x64, 0x6F, 0x6C, 0x6F,
    0x72, 0x20, 0x73, 0x69, 0x74, 0x20, 0x61, 0x6D, 0x65, 0x74, 0x2E
};
static const uint8_t SNEP_LONG_RECORD[PTX_SNEP_DTA_LONG_NDEF_RECORD_LENGTH] =
{
    0xC1, 0x01, 0x00, 0x00, 0x01, 0xEF, 0x54, 0x02, 0x6C, 0x61, 0x4C, 0x6F, 0x72,
    0x65, 0x6D, 0x20, 0x69, 0x70, 0x73, 0x75, 0x6D, 0x20, 0x64, 0x6F, 0x6C, 0x6F,
    0x72, 0x20, 0x73, 0x69, 0x74, 0x20, 0x61, 0x6D, 0x65, 0x74, 0x2C, 0x20, 0x63,
    0x6F, 0x6E, 0x73, 0x65, 0x63, 0x74, 0x65, 0x74, 0x75, 0x72, 0x20, 0x61, 0x64,
    0x69, 0x70, 0x69, 0x73, 0x63, 0x69, 0x6E, 0x67, 0x20, 0x65, 0x6C, 0x69, 0x74,
    0x2E, 0x20, 0x50, 0x68, 0x61, 0x73, 0x65, 0x6C, 0x6C, 0x75, 0x73, 0x20, 0x61,
    0x74, 0x20, 0x6C, 0x6F, 0x72, 0x65, 0x6D, 0x20, 0x6E, 0x75, 0x6E, 0x63, 0x2C,
    0x20, 0x75, 0x74, 0x20, 0x76, 0x65, 0x6E, 0x65, 0x6E, 0x61, 0x74, 0x69, 0x73,
    0x20, 0x71, 0x75, 0x61, 0x6D, 0x2E, 0x20, 0x45, 0x74, 0x69, 0x61, 0x6D, 0x20,
    0x69, 0x64, 0x20, 0x64, 0x6F, 0x6C, 0x6F, 0x72, 0x20, 0x71, 0x75, 0x61, 0x6D,
    0x2C, 0x20, 0x61, 0x74, 0x20, 0x76, 0x69, 0x76, 0x65, 0x72, 0x72, 0x61, 0x20,
    0x64, 0x6F, 0x6C, 0x6F, 0x72, 0x2E, 0x20, 0x50, 0x68, 0x61, 0x73, 0x65, 0x6C,
    0x6C, 0x75, 0x73, 0x20, 0x65, 0x75, 0x20, 0x6C, 0x61, 0x63, 0x75, 0x73, 0x20,
    0x6C, 0x69, 0x67, 0x75, 0x6C, 0x61, 0x2C, 0x20, 0x71, 0x75, 0x69, 0x73, 0x20,
    0x65, 0x75, 0x69, 0x73, 0x6D, 0x6F, 0x64, 0x20, 0x65, 0x72, 0x61, 0x74, 0x2E,
    0x20, 0x53, 0x65, 0x64, 0x20, 0x66, 0x65, 0x75, 0x67, 0x69, 0x61, 0x74, 0x2C,
    0x20, 0x6C, 0x69, 0x67, 0x75, 0x6C, 0x61, 0x20, 0x61, 0x74, 0x20, 0x6D, 0x6F,
    0x6C, 0x6C, 0x69, 0x73, 0x20, 0x61, 0x6C, 0x69, 0x71, 0x75, 0x65, 0x74, 0x2C,
    0x20, 0x6A, 0x75, 0x73, 0x74, 0x6F, 0x20, 0x6C, 0x61, 0x63, 0x75, 0x73, 0x20,
    0x63, 0x6F, 0x6E, 0x64, 0x69, 0x6D, 0x65, 0x6E, 0x74, 0x75, 0x6D, 0x20, 0x65,
    0x72, 0x6F, 0x73, 0x2C, 0x20, 0x6E, 0x6F, 0x6E, 0x20, 0x74, 0x69, 0x6E, 0x63,
    0x69, 0x64, 0x75, 0x6E, 0x74, 0x20, 0x6E, 0x65, 0x71, 0x75, 0x65, 0x20, 0x69,
    0x70, 0x73, 0x75, 0x6D, 0x20, 0x65, 0x75, 0x20, 0x72, 0x69, 0x73, 0x75, 0x73,
    0x2E, 0x20, 0x53, 0x65, 0x64, 0x20, 0x61, 0x64, 0x69, 0x70, 0x69, 0x73, 0x63,
    0x69, 0x6E, 0x67, 0x20, 0x64, 0x75, 0x69, 0x20, 0x65, 0x75, 0x69, 0x73, 0x6D,
    0x6F, 0x64, 0x20, 0x74, 0x65, 0x6C, 0x6C, 0x75, 0x73, 0x20, 0x75, 0x6C, 0x6C,
    0x61, 0x6D, 0x63, 0x6F, 0x72, 0x70, 0x65, 0x72, 0x20, 0x6F, 0x72, 0x6E, 0x61,
    0x72, 0x65, 0x2E, 0x20, 0x50, 0x68, 0x61, 0x73, 0x65, 0x6C, 0x6C, 0x75, 0x73,
    0x20, 0x6D, 0x61, 0x74, 0x74, 0x69, 0x73, 0x20, 0x72, 0x69, 0x73, 0x75, 0x73,
    0x20, 0x65, 0x74, 0x20, 0x6C, 0x65, 0x63, 0x74, 0x75, 0x73, 0x20, 0x65, 0x75,
    0x69, 0x73, 0x6D, 0x6F, 0x64, 0x20, 0x65, 0x75, 0x20, 0x66, 0x65, 0x72, 0x6D,
    0x65, 0x6E, 0x74, 0x75, 0x6D, 0x20, 0x73, 0x65, 0x6D, 0x20, 0x63, 0x75, 0x72,
    0x73, 0x75, 0x73, 0x2E, 0x20, 0x50, 0x68, 0x61, 0x73, 0x65, 0x6C, 0x6C, 0x75,
    0x73, 0x20, 0x74, 0x72, 0x69, 0x73, 0x74, 0x69, 0x71, 0x75, 0x65, 0x20, 0x63,
    0x6F, 0x6E, 0x73, 0x65, 0x63, 0x74, 0x65, 0x74, 0x75, 0x72, 0x20, 0x6D, 0x61,
    0x75, 0x72, 0x69, 0x73, 0x20, 0x65, 0x75, 0x20, 0x70, 0x6F, 0x72, 0x74, 0x74,
    0x69, 0x74, 0x6F, 0x72, 0x2E, 0x20, 0x53, 0x65, 0x64, 0x20, 0x6C, 0x6F, 0x62,
    0x6F, 0x72, 0x74, 0x69, 0x73, 0x20, 0x70, 0x6F, 0x72, 0x74, 0x74, 0x69, 0x74,
    0x6F, 0x72, 0x20, 0x6F, 0x72, 0x63, 0x69, 0x2E
};

ptx_Status_t ptx_NFCForumDTA_RunSnepServerOperation(ptx_NFCForumDTA_t *dta);
ptx_Status_t ptx_NFCForumDTA_RunSnepClientOperation(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NFCForumDTA_RunSnepTestOperation(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
        /* In case the PATTERN_NUMBER is higher than NFC_FORUM_DTA_SNEP_SERVER_PATTERNNUMBER
            it's an indication that we need to run the SNEP_CLIENT DTA. */
        if (PTX_NFCFORUMDTA_SNEPSERVER_PATTERN == (testCfg->PATTERN_NUMBER & PTX_NFCFORUMDTA_SNEP_PATTERNNUMBER_MASK))
        {
            status = ptx_NFCForumDTA_RunSnepServerOperation(dta);
        }
        else
        {
            status = ptx_NFCForumDTA_RunSnepClientOperation(dta, testCfg);
        }
    }

    dta->State = ptx_NFCForumDTA_State_FINISHED;

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

ptx_Status_t ptx_NFCForumDTA_RunSnepServerOperation(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status;

    status = ptx_RUL_SnepServer_WaitForClient(&dta->snepServer);

    /* Loop until the ServerState is either DOWN (=client disconnected), or an error happened. */
    while ((PTX_STATUS_SUCCESS == status) && (ServerState_ServerDown != dta->snepServer.ServerState) && (!dta->quitDTA))
    {
        status = ptx_RUL_SnepServer_Handle(&dta->snepServer);
        (void) dta->applicationCallback(dta);
    }

    /* No other action/request necessary for the DTA --> deactivate! */
    ptx_RUL_DeactivateParams_t deactivate_params;
    deactivate_params.Type = ptx_RUL_DeactivateType_Generic;
    deactivate_params.State = ptx_RUL_DeactivateState_Idle;
    status = ptx_RUL_Deactivate(dta->rul, &deactivate_params);

    /* We deinitialize the component in any case! */
    (void) ptx_RUL_SnepServer_DeInit(&dta->snepServer);

    return status;
}

ptx_Status_t ptx_NFCForumDTA_RunSnepClientOperation(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    /* Connect to the server. */
    ptx_Status_t status = ptx_RUL_SnepClient_ConnectToServer(&dta->snepClient);

    if (PTX_STATUS_SUCCESS == status)
    {
        /* Differentiate the different DTA scenarios depending on the provided pattern number. */
        switch (testCfg->PATTERN_NUMBER & PTX_NFCFORUMDTA_SNEP_PATTERNNUMBER_MASK)
        {
            case PTX_NFCFORUMDTA_SNEPCLIENT_PATTERN_PUT_REQ_NORMAL:
            {
                status = ptx_RUL_SnepClient_PutMessage(&dta->snepClient, (uint8_t*) SNEP_NORMAL_RECORD, sizeof(SNEP_NORMAL_RECORD));
            }
            break;

            case PTX_NFCFORUMDTA_SNEPCLIENT_PATTERN_PUT_REQ_LONG:
            {
                status = ptx_RUL_SnepClient_PutMessage(&dta->snepClient, (uint8_t*) SNEP_LONG_RECORD, sizeof(SNEP_LONG_RECORD));
            }
            break;
            case PTX_NFCFORUMDTA_SNEPCLIENT_PATTERN_GET_REQ:
            {
                uint8_t temporary_buffer[sizeof(SNEP_NORMAL_RECORD)];
                status = ptx_RUL_SnepClient_GetMessage(&dta->snepClient, temporary_buffer, sizeof(SNEP_NORMAL_RECORD));
            }
            break;
            default:
                break;
        }
    }

    /* Loop until the ClientState is either IDLE (=operation finished), or an error happened. */
    while ((PTX_STATUS_SUCCESS == status) && (ClientState_Idle != dta->snepClient.ClientState) && (!dta->quitDTA))
    {
        status = ptx_RUL_SnepClient_Handle(&dta->snepClient);
        (void) dta->applicationCallback(dta);
    }

    /* No other action/request necessary for the DTA --> deactivate! */
    ptx_RUL_DeactivateParams_t deactivate_params;
    deactivate_params.Type = ptx_RUL_DeactivateType_Generic;
    deactivate_params.State = ptx_RUL_DeactivateState_Idle;
    status = ptx_RUL_Deactivate(dta->rul, &deactivate_params);

    /* We deinitialize the component in any case! */
    (void) ptx_RUL_SnepClient_DeInit(&dta->snepClient);

    return status;
}
