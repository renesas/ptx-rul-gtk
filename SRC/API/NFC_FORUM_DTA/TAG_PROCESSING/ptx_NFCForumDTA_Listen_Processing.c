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
    Module      : NFC Forum Device Test Application (DTA) Listen Processing
    File        : ptx_NFCForumDTA_Listen_Processing.c

    Description : Contains the implementation to handle the processing of Listen Mode in the NFC Forum DTA. It controls the
                  processing if the NFC Forum DTA is running in Listen Mode.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NFCForumDTA_Listen_Processing.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NFCFORUMDTA_LISTEN_SENSF_RES_PAD01_LEN          (5u)
#define PTX_NFCFORUMDTA_LISTEN_SENSF_RES_SYS_CODE_LEN       (2u)
#define PTX_NFCFORUMDTA_LISTEN_SENSF_REQ_SYS_CODE_OFFSET    (1u)
#define PTX_NFCFORUMDTA_LISTEN_T3T_CMD_NFCID2_OFFSET        (2u)
#define PTX_NFCFORUMDTA_LISTEN_T3T_BLOCK_DATA_SIZE          (16u)
#define PTX_NFCFORUMDTA_LISTEN_T3T_CHECK_RESPONSE_CONSTANT  (0x07)
#define PTX_NFCFORUMDTA_LISTEN_T3T_UPDATE_RESPONSE_CONSTANT (0x09)

#define PTX_NFCFORUMDTA_LISTEN_SELECT_APDU_LEN              (4u)
#define PTX_NFCFORUMDTA_LISTEN_LOOPBACK_CAPDU_LEN           (4u)
#define PTX_NFCFORUMDTA_LISTEN_SELECT_DTA_APDU_LEN          (16u)
#define PTX_NFCFORUMDTA_LISTEN_SEL_RES_LEN                  (2u)
#define PTX_NFCFORUMDTA_LISTEN_SEL_RES_DTA_LEN              (4u)
#define PTX_NFCFORUMDTA_LISTEN_STATUS_WORD_LEN              (2u)

#define PTX_NFCFORUMDTA_LISTEN_WAIT_CMD_LEN                 (5u)
#define PTX_NFCFORUMDTA_LISTEN_WAIT_CMD_RWT                 (310u)
#define PTX_NFCFORUMDTA_LISTEN_WAIT_CMD_RWT_P2PACM          (5u)
#define PTX_NFCFORUMDTA_LISTEN_PATTERN_NUMBER_CMD_LEN       (4u)
#define PTX_NFCFORUMDTA_LISTEN_SELECT_APDU_DF_LEN           (14u)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_CommandProcess_T3T(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen);

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_CommandProcess_T4T(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen);

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_CommandProcess_NFCDEP(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen, ptx_NFCForumDTA_Test_Config_t *testCfg);

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendSensFRes(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen);
static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendCheckRes(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen);
static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendUpdateRes(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen);
static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendLoopbackRes(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen);


static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendSelRes(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen);
static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendLoopbackAPDU(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen);
static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendUnknownCommand(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen);

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendLoopbackAppData(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen);
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NFCForumDTA_Listen_Processing_CommandProcess(ptx_NFCForumDTA_t *dta, uint8_t protocol, uint8_t *rx, uint32_t rxLen, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != rx) && (0 != rxLen) && (0 != protocol))
    {
        switch (protocol)
        {
            case ptx_RUL_CardProtocolType_T2T:
                status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;
                break;
            case ptx_RUL_CardProtocolType_T3T:
                status = ptx_NFCForumDTA_Listen_Processing_CommandProcess_T3T(dta, rx, rxLen);
                break;
            case ptx_RUL_CardProtocolType_T4T:
                status = ptx_NFCForumDTA_Listen_Processing_CommandProcess_T4T(dta, rx, rxLen);
                break;
            case ptx_RUL_CardProtocolType_T5T:
                status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;
                break;
            case ptx_RUL_CardProtocolType_NfcDep:
                status = ptx_NFCForumDTA_Listen_Processing_CommandProcess_NFCDEP(dta, rx, rxLen, testCfg);
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_STATE;
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

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_CommandProcess_T3T (ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != dta) && (NULL != rx) && (0 != rxLen))
    {
        const uint8_t SENSF_REQ_LEN = 5;
        const uint8_t SENSF_REQ_CMP_LEN = 3;
        uint8_t SENSF_REQ_CMD[3] = {0x00, 0xFF, 0xFF};

        const uint8_t T3T_CHECK_CMD  = 0x06;
        const uint8_t T3T_UPDATE_CMD = 0x08;
        const uint8_t T3T_CMD_MIN_LEN = 9;
        const uint8_t T3T_LOOPBACK_CMD_0 = 0xD8;
        const uint8_t T3T_LOOPBACK_CMD_1 = 0x00;

        // Check if the received command is a SENSF_REQ command
        if ((rxLen >= SENSF_REQ_LEN) && (0 == memcmp(rx, SENSF_REQ_CMD, SENSF_REQ_CMP_LEN)))
        {
            // Send back SENSF_RES
            status = ptx_NFCForumDTA_Listen_Processing_SendSensFRes(dta, rx, rxLen);
        }
        // Check if the received command is a CHECK command
        else if ((rxLen >= T3T_CMD_MIN_LEN) && rx[1] == T3T_CHECK_CMD)
        {
            // Send response to CHECK command
            status = ptx_NFCForumDTA_Listen_Processing_SendCheckRes(dta, rx, rxLen);
        }
        // Check if the recevied command is a UPDATE command
        else if ((rxLen >= T3T_CMD_MIN_LEN) && rx[1] == T3T_UPDATE_CMD)
        {
            // Send response to UPDATE command
            status = ptx_NFCForumDTA_Listen_Processing_SendUpdateRes(dta, rx, rxLen);
        }
        // Check if the received command is the Loopback command
        else if (rx[1] == T3T_LOOPBACK_CMD_0 && rx[2] == T3T_LOOPBACK_CMD_1)
        {
            // Send response to LOOPBACK command
            status = ptx_NFCForumDTA_Listen_Processing_SendLoopbackRes(dta, rx, rxLen);
        }


    }

    return status;
}


static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_CommandProcess_T4T (ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != rx) && (0 != rxLen))
    {
        const uint8_t SELECT_APDU[PTX_NFCFORUMDTA_LISTEN_SELECT_APDU_LEN] = {0x00, 0xA4, 0x04, 0x00};
        const uint8_t LOOPBACK_CAPDU[PTX_NFCFORUMDTA_LISTEN_LOOPBACK_CAPDU_LEN] = {0x80, 0xEE, 0x00, 0x00};

        // Check if received command is a SELECT APDU
        if ((rxLen >= PTX_NFCFORUMDTA_LISTEN_SELECT_APDU_LEN) && (0 == memcmp(rx, SELECT_APDU, sizeof(uint8_t) * PTX_NFCFORUMDTA_LISTEN_SELECT_APDU_LEN)))
        {
            // respond with a Select Response
            status = ptx_NFCForumDTA_Listen_Processing_SendSelRes(dta, rx, rxLen);
        }
        // Check if received command is a Loopback command
        else if ((rxLen >= PTX_NFCFORUMDTA_LISTEN_LOOPBACK_CAPDU_LEN) && (0 == memcmp(rx, LOOPBACK_CAPDU, sizeof(uint8_t) * PTX_NFCFORUMDTA_LISTEN_LOOPBACK_CAPDU_LEN)))
        {
            // respond with loopback APDU
            status  = ptx_NFCForumDTA_Listen_Processing_SendLoopbackAPDU(dta, rx, rxLen);
        }
        // Respond with Status Word command not allowed in case of unknown command
        else
        {
            // respond with unknown command
            status = ptx_NFCForumDTA_Listen_Processing_SendUnknownCommand(dta, rx, rxLen);
        }

    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_CommandProcess_NFCDEP (ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != rx) && (0 != rxLen))
    {

        uint8_t WAIT_CMD[PTX_NFCFORUMDTA_LISTEN_WAIT_CMD_LEN] = {0xFF, 0xFF, 0xFF, 0x01, 0x03};
        uint8_t PATTERN_NUM_CMD[PTX_NFCFORUMDTA_LISTEN_PATTERN_NUMBER_CMD_LEN] = {0xFF, 0x00, 0x00, 0x00};

        // Check if the received command is a wait command
        if ((rxLen >= PTX_NFCFORUMDTA_LISTEN_WAIT_CMD_LEN) && (0 == memcmp(rx, WAIT_CMD, PTX_NFCFORUMDTA_LISTEN_WAIT_CMD_LEN)))
        {
            // Wait for RWT + delta RWT + 128/fc

            // uint32_t rwt = 0; //(256 * 16/fc)* 2 ** WT // (4096/fc) * 2**WT // ~(3 microseconds) * 2**WT
            // uint8_t wt = 10;
            // uint32_t pow = 1;
            // for (size_t i = 0; i < wt; i++)
            // {
            //     pow *= 2;
            // }

            // pow = 2 ** WT
            // 256*16/fc ~ 302µs
            // + delta ->

            //uint32_t rwt = (303 * 1024) / 1000;

            if (ptx_NSC_RfTech_Active_Listen == dta->hce.ActiveTech)
            {
                status = ptx_RUL_Sleep(dta->rul, PTX_NFCFORUMDTA_LISTEN_WAIT_CMD_RWT_P2PACM);
            }
            else
            {
                status = ptx_RUL_Sleep(dta->rul, PTX_NFCFORUMDTA_LISTEN_WAIT_CMD_RWT);
            }




            if (PTX_STATUS_SUCCESS == status)
            {
                // respond with application data
                status  = ptx_NFCForumDTA_Listen_Processing_SendLoopbackAppData(dta, rx, rxLen);
            }

        }
        // Check if the received command is a Pattern Number command
        else if ((rxLen >= PTX_NFCFORUMDTA_LISTEN_PATTERN_NUMBER_CMD_LEN) && (0 == memcmp(rx, PATTERN_NUM_CMD, sizeof(uint8_t) * PTX_NFCFORUMDTA_LISTEN_PATTERN_NUMBER_CMD_LEN)))
        {
            testCfg->PATTERN_NUMBER = 0x0000;
            status = PTX_STATUS_SUCCESS;
        }
        // Perform loopback if neither wait nor pattern number command was received
        else
        {
            // respond with loopback of application data
            if ((testCfg->PATTERN_NUMBER != 0x1200) && (testCfg->PATTERN_NUMBER != 0x1201))
            {
                status = ptx_NFCForumDTA_Listen_Processing_SendLoopbackAppData(dta, rx, rxLen);
            }
            else
            {
                status = PTX_STATUS_SUCCESS;
            }
        }

    }

    return status;
}


static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendSensFRes (ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != rx) && (0 != rxLen))
    {
        const uint8_t SENSF_REQ_LEN = 5;
        if (rxLen >= SENSF_REQ_LEN)
        {
            uint8_t *tx = dta->NDEFMessageBuffer;
            *dta->NDEFMessageBufferLen = dta->NDEFMessageBufferSize;
            uint32_t *txLen = dta->NDEFMessageBufferLen;

            // extract system code and request code from SENSF_REQ
            uint8_t sys_code[PTX_NFCFORUMDTA_LISTEN_SENSF_RES_SYS_CODE_LEN];
            memcpy(sys_code, &rx[PTX_NFCFORUMDTA_LISTEN_SENSF_REQ_SYS_CODE_OFFSET], sizeof(uint8_t) * PTX_NFCFORUMDTA_LISTEN_SENSF_RES_SYS_CODE_LEN);
            uint8_t rc_requested  = rx[3];

            const uint8_t NFCID2[PTX_RUL_NFCID2_LEN] = {0x02, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            const uint8_t PAD01[PTX_NFCFORUMDTA_LISTEN_SENSF_RES_PAD01_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            const uint8_t MRTI_CHECK = 0xFF;
            const uint8_t MRTI_UPDATE = 0xFF;
            const uint8_t PAD2 = 0xFF;




            // Assemble SENSF_RES
            uint8_t idx = 0;

            // SENSF_RES Constant
            tx[idx] = 0x01;
            idx++;

            // NFCID2
            memcpy(&tx[idx], NFCID2, sizeof(uint8_t) * PTX_RUL_NFCID2_LEN);
            idx += PTX_RUL_NFCID2_LEN;

            // PAD 0 & PAD 1
            memcpy(&tx[idx], PAD01, sizeof(uint8_t) * PTX_NFCFORUMDTA_LISTEN_SENSF_RES_PAD01_LEN);
            idx += PTX_NFCFORUMDTA_LISTEN_SENSF_RES_PAD01_LEN;

            // MRTI CHECK
            tx[idx] = MRTI_CHECK;
            idx++;

            // MRTI UPDATE
            tx[idx] = MRTI_UPDATE;
            idx++;

            // PAD2
            tx[idx] = PAD2;
            idx++;

            switch (rc_requested)
            {
                case 0x01:
                    // System Code
                    memcpy(&tx[idx], sys_code, sizeof(uint8_t) * PTX_NFCFORUMDTA_LISTEN_SENSF_RES_SYS_CODE_LEN);
                    idx += PTX_NFCFORUMDTA_LISTEN_SENSF_RES_SYS_CODE_LEN;
                    break;

                case 0x02:
                    // Advanced protocol features
                    tx[idx] = 0x00;
                    idx++;
                    tx[idx] = 0x00;
                    idx++;
                    break;
                default:
                    break;
            }

            *txLen = idx;

            status = ptx_RUL_HCE_SendData(&dta->hce, tx, *txLen);
        }
    }


    return status;
}

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendCheckRes (ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != rx) && (0 != rxLen))
    {
        const uint8_t T3T_CMD_MIN_LEN = 9;
        if (rxLen >= T3T_CMD_MIN_LEN)
        {
            uint8_t *tx = dta->NDEFMessageBuffer;
            *dta->NDEFMessageBufferLen = dta->NDEFMessageBufferSize;
            uint32_t *txLen = dta->NDEFMessageBufferLen;

            // Get NFCID2 from received data
            uint8_t nfcid2[PTX_RUL_NFCID2_LEN];
            memcpy(nfcid2, &rx[PTX_NFCFORUMDTA_LISTEN_T3T_CMD_NFCID2_OFFSET], sizeof(uint8_t) * PTX_RUL_NFCID2_LEN);

            const uint8_t BLOCK_DATA[PTX_NFCFORUMDTA_LISTEN_T3T_BLOCK_DATA_SIZE] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

            uint8_t idx = 1;
            tx[idx] = PTX_NFCFORUMDTA_LISTEN_T3T_CHECK_RESPONSE_CONSTANT;   // Constant for CHECK response
            idx++;

            // NFCID2
            memcpy(&tx[idx], nfcid2, sizeof(uint8_t) * PTX_RUL_NFCID2_LEN);
            idx += PTX_RUL_NFCID2_LEN;

            // Constants according to DTA
            tx[idx] = 0x00;
            idx++;
            tx[idx] = 0x00;
            idx++;
            tx[idx] = 0x01;
            idx++;

            // Block Data
            memcpy(&tx[idx], BLOCK_DATA, sizeof(uint8_t) * PTX_NFCFORUMDTA_LISTEN_T3T_BLOCK_DATA_SIZE);
            idx += PTX_NFCFORUMDTA_LISTEN_T3T_BLOCK_DATA_SIZE;

            tx[0] = idx;

            *txLen = idx;

            status = ptx_RUL_HCE_SendData(&dta->hce, tx, *txLen);
        }
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendUpdateRes (ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != rx) && (0 != rxLen))
    {
        const uint8_t T3T_CMD_MIN_LEN = 9;
        if (rxLen >= T3T_CMD_MIN_LEN)
        {
            uint8_t *tx = dta->NDEFMessageBuffer;
            *dta->NDEFMessageBufferLen = dta->NDEFMessageBufferSize;
            uint32_t *txLen = dta->NDEFMessageBufferLen;

            // Get NFCID2 from received data
            uint8_t nfcid2[PTX_RUL_NFCID2_LEN];
            memcpy(nfcid2, &rx[PTX_NFCFORUMDTA_LISTEN_T3T_CMD_NFCID2_OFFSET], sizeof(uint8_t) * PTX_RUL_NFCID2_LEN);

            uint8_t idx = 1;
            tx[idx] = PTX_NFCFORUMDTA_LISTEN_T3T_UPDATE_RESPONSE_CONSTANT;   // Constant for UPDATE response
            idx++;

            // NFCID2
            memcpy(&tx[idx], nfcid2, sizeof(uint8_t) * PTX_RUL_NFCID2_LEN);
            idx += PTX_RUL_NFCID2_LEN;

            // Constants according to DTA
            tx[idx] = 0x00;
            idx++;
            tx[idx] = 0x00;
            idx++;

            tx[0] = idx;

            *txLen = idx;

            status = ptx_RUL_HCE_SendData(&dta->hce, tx, *txLen);
        }
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendLoopbackRes(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != rx) && (0 != rxLen))
    {

        uint8_t *tx = dta->NDEFMessageBuffer;
        *dta->NDEFMessageBufferLen = dta->NDEFMessageBufferSize;
        uint32_t *txLen = dta->NDEFMessageBufferLen;

        const uint8_t T3T_LOOPBACK_RES_0 = 0xD9;
        const uint8_t T3T_LOOPBACK_RES_1 = 0x01;

        uint32_t idx = 1;
        tx[idx] = T3T_LOOPBACK_RES_0;   // Constant for Loopback response
        idx++;
        tx[idx] = T3T_LOOPBACK_RES_1;   // Constant for Loopback response
        idx++;

        memcpy(&tx[idx], &rx[3], (rx[0] * sizeof(uint8_t) - 3));
        idx += rx[0] - 3u;

        tx[0] = (uint8_t)idx;

        *txLen = idx;

        status = ptx_RUL_HCE_SendData(&dta->hce, tx, *txLen);

    }

    return status;
}


static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendSelRes(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != rx) && (0 != rxLen))
    {
        uint8_t SEL_RES[PTX_NFCFORUMDTA_LISTEN_SEL_RES_LEN] = {0x90, 0x00};
        uint8_t SEL_RES_DTA[PTX_NFCFORUMDTA_LISTEN_SEL_RES_DTA_LEN] = {0x01, 0x00, 0x90, 0x00};

        /* DTA DF name variants  */
        const uint8_t SELECT_APDU_DFNAME_1[PTX_NFCFORUMDTA_LISTEN_SELECT_APDU_DF_LEN] = {0x31, 0x4E, 0x46, 0x43, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31}; /* "1NFC.SYS.DDF01" */
        const uint8_t SELECT_APDU_DFNAME_2[PTX_NFCFORUMDTA_LISTEN_SELECT_APDU_DF_LEN] = {0x32, 0x4E, 0x46, 0x43, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31}; /* "2NFC.SYS.DDF01" */

        uint8_t isDtaSelect = 0u;

        // Search the whole received APDU for either DF name
        if (rxLen >= sizeof(SELECT_APDU_DFNAME_1))
        {
            uint32_t i;
            for (i = 0u; i <= (rxLen - (uint32_t)sizeof(SELECT_APDU_DFNAME_1)); i++)
            {
                if ((0 == memcmp(&rx[i], SELECT_APDU_DFNAME_1, sizeof(SELECT_APDU_DFNAME_1))) ||
                    (0 == memcmp(&rx[i], SELECT_APDU_DFNAME_2, sizeof(SELECT_APDU_DFNAME_2))))
                {
                    isDtaSelect = 1u;
                    break;
                }
            }
        }

        if (isDtaSelect != 0u)
        {
            // Send back the expected SEL_RES for the Select DTA command
            status = ptx_RUL_HCE_SendData(&dta->hce, SEL_RES_DTA, PTX_NFCFORUMDTA_LISTEN_SEL_RES_DTA_LEN);
        }
        else
        {
            // Send back the standard SEL_RES response for other select commands
            status = ptx_RUL_HCE_SendData(&dta->hce, SEL_RES, PTX_NFCFORUMDTA_LISTEN_SEL_RES_LEN);
        }
    }
    return status;
}

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendLoopbackAPDU(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != rx) && (0 != rxLen))
    {
        uint8_t *tx = dta->NDEFMessageBuffer;
        *dta->NDEFMessageBufferLen = dta->NDEFMessageBufferSize;
        uint32_t *txLen = dta->NDEFMessageBufferLen;

        // Create Response APDU from received APDU
        *txLen = rxLen + 2;
        memcpy(tx, rx, sizeof(uint8_t) * rxLen);

        // Add Successful Status Word
        const uint8_t STATUS_WORD_OK[PTX_NFCFORUMDTA_LISTEN_STATUS_WORD_LEN] = {0x90, 0x00};
        memcpy(&tx[rxLen], STATUS_WORD_OK, sizeof(uint8_t) * PTX_NFCFORUMDTA_LISTEN_STATUS_WORD_LEN);

        status = ptx_RUL_HCE_SendData(&dta->hce, tx, *txLen);
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendUnknownCommand(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != rx) && (0 != rxLen))
    {
        uint8_t STATUS_WORD_COMMAND_NOT_ALLOWED[PTX_NFCFORUMDTA_LISTEN_STATUS_WORD_LEN] = {0x69, 0x86};

        status = ptx_RUL_HCE_SendData(&dta->hce, STATUS_WORD_COMMAND_NOT_ALLOWED, PTX_NFCFORUMDTA_LISTEN_SEL_RES_LEN);
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_Listen_Processing_SendLoopbackAppData(ptx_NFCForumDTA_t *dta, uint8_t *rx, uint32_t rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != rx) && (0 != rxLen))
    {
        uint8_t *tx = dta->NDEFMessageBuffer;
        *dta->NDEFMessageBufferLen = dta->NDEFMessageBufferSize;
        uint32_t *txLen = dta->NDEFMessageBufferLen;

        // Create Response from received Application data
        *txLen = rxLen;
        memcpy(tx, rx, sizeof(uint8_t) * rxLen);

        status = ptx_RUL_HCE_SendData(&dta->hce, tx, *txLen);
    }

    return status;
}
