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
    Module      : NFC Forum Device Test Application (DTA) T3T Processing
    File        : ptx_NFCForumDTA_T3T_Processing.c

    Description : Contains the implementation to handle the processing of T3T in the NFC Forum DTA. It controls the
                  processing if the NFC Forum DTA is communicating with a card in T3T Mode.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NFCForumDTA_T3T_Processing.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NFCFORUMDTA_T3T_SENSFREQ_SIZE       (6u)
#define PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE       (20u)

#define PTX_NFCFORUMDTA_T3T_LOOPBACK_PAYLOAD_SIZE   (255u)

#define PTX_NFCFORUMDTA_T3T_LOOPBACK_WRITE_CMD  (0xD8u)
#define PTX_NFCFORUMDTA_T3T_LOOPBACK_WRITE_RSP  (0xD9u)

#define PTX_NFCFORUMDTA_T3T_RDCODE_NEG_OFFSET   (2u)
#define PTX_NFCFORUMDTA_T3T_RDCODE_SIZE         (2u)


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NFCForumDTA_Loopback(ptx_NFCForumDTA_t *dta);
static ptx_Status_t ptx_NFCForumDTA_SysCapabilityDetection(ptx_NFCForumDTA_t *dta);
static ptx_Status_t ptx_NFCForumDTA_ServiceIndepApplication(ptx_NFCForumDTA_t *dta);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NFCForumDTA_ProcessT3T(ptx_NFCForumDTA_t *dta, ptx_NFCForumDTA_Test_Config_t *testCfg)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint16_t sysCode = 0;
    uint8_t rc = 0;
    uint8_t tsn = 0;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA) && (NULL != testCfg))
    {
        status = PTX_STATUS_SUCCESS;
    }

    if (PTX_STATUS_SUCCESS == status)
    {
        switch (testCfg->PATTERN_NUMBER)
        {
            case 0x0007: // FALL THROUGH
            case 0x0008: // FALL THROUGH
            case 0x000A: // FALL THROUGH
            case 0x000B:
                status = PTX_STATUS_SUCCESS;
                break;

            default:
                sysCode = 0x12FC;
                rc = 0;
                tsn = 0;
                uint8_t rx[PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE];
                uint32_t rxLen = PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE;
                status = ptx_RUL_T3T_SensFReq(dta->rul, sysCode, rc, tsn, rx, &rxLen);
                (void)rxLen;
                break;
        }
    }

    if (PTX_STATUS_SUCCESS == status)
    {

        switch (testCfg->PATTERN_NUMBER)
        {
            case 0x0000: // FALL THROUGH
            case 0x0001:
                // Read NDEF Message
                memset(dta->NDEFMessageBuffer, 0x00, sizeof(uint8_t) * dta->NDEFMessageBufferSize);
                status = ptx_NDEF_CheckMessage(&dta->ndef);
                if (PTX_STATUS_SUCCESS == status)
                {
                    *dta->NDEFMessageBufferLen = dta->NDEFMessageBufferSize;
                    status = ptx_NDEF_ReadMessage(&dta->ndef, dta->NDEFMessageBuffer, dta->NDEFMessageBufferLen);
                }
                break;
            case 0x0002:
                // Write NDEF Message
                status = ptx_NDEF_CheckMessage(&dta->ndef);
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_WriteMessage(&dta->ndef, dta->NDEFMessageBuffer, *dta->NDEFMessageBufferLen);
                }
                break;
            case 0x0003:
                // Lock NDEF Tag
                status = ptx_NDEF_CheckMessage(&dta->ndef);
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_LockTag(&dta->ndef);
                }
                break;
            case 0x0005: // FALL THROUGH
            case 0x0006:
                // NDEF Loopback
                status = ptx_NFCForumDTA_Loopback(dta);
                break;
            case 0x0007:
            case 0x0008:
                // Capability Detection
                status = ptx_NFCForumDTA_SysCapabilityDetection(dta);
                break;
            case 0x000A: // FALL THROUGH
            case 0x000B:
                // Service Independent Application
                status = ptx_NFCForumDTA_ServiceIndepApplication(dta);
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

static ptx_Status_t ptx_NFCForumDTA_GeneratePayload(uint8_t *payload, uint8_t *nfcid2)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != payload) && (NULL != nfcid2))
    {
        for (uint32_t i = 0; i < 31; i++)
        {
            memcpy(&payload[i * PTX_T3T_NFCID2_SIZE], nfcid2, PTX_T3T_NFCID2_SIZE);
        }

        payload[248] = nfcid2[0];
        payload[249] = nfcid2[1];
        payload[250] = nfcid2[2];
        payload[251] = nfcid2[3];

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_Loopback(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
        uint16_t sysCode = 0x12FC;
        uint8_t rc = 0;
        uint8_t tsn = 0;

        uint8_t rx[PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE];
        uint32_t rxLen = PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE;

        status = ptx_RUL_T3T_SensFReq(dta->rul, sysCode, rc, tsn, rx, &rxLen);

        /*if (0 == rxLen)
        {
            status = ptx_RUL_T3T_SensFReq(dta->rul, sysCode, rc, tsn, rx, &rxLen);
        }*/

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t payload[PTX_NFCFORUMDTA_T3T_LOOPBACK_PAYLOAD_SIZE];
            memset(payload, 0, sizeof(uint8_t) * PTX_NFCFORUMDTA_T3T_LOOPBACK_PAYLOAD_SIZE);

            uint8_t nfcid2[PTX_T3T_NFCID2_SIZE];
            memcpy(nfcid2, &rx[1], sizeof(uint8_t) * PTX_T3T_NFCID2_SIZE);


            payload[0] = PTX_NFCFORUMDTA_T3T_LOOPBACK_WRITE_CMD;
            payload[1] = 0x00;
            status = ptx_NFCForumDTA_GeneratePayload(&payload[2], nfcid2);

            if ((PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE >= sizeof(rx)) & (PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE >= sizeof(payload)))
            {
                do
                {
                    rxLen = PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE;
                    status = ptx_RUL_T3T_SensFReq(dta->rul, sysCode, rc, tsn, rx, &rxLen);

                    if (NULL != dta->applicationCallback)
                    {
                        dta->applicationCallback(dta);
                    }

                }
                while ((PTX_NFCFORUMDTA_T3T_LOOPBACK_WRITE_RSP == rx[0]) &&
                       (0 == rx[1]) && (PTX_STATUS_SUCCESS == status) &&
                       (0 == memcmp(&rx[2], &payload[2], PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE - 2)));
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_SysCapabilityDetection(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
        uint16_t sysCode = 0xFFFF;
        uint8_t rc = 1;
        uint8_t tsn = 0;

        uint8_t rx[PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE];
        uint32_t rxLen = PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE;

        status = ptx_RUL_T3T_SensFReq(dta->rul, sysCode, rc, tsn, rx, &rxLen);

        if ((PTX_STATUS_SUCCESS == status) && (0 != rxLen))
        {
            uint8_t RDCode[PTX_NFCFORUMDTA_T3T_RDCODE_SIZE];
            memcpy(RDCode, &rx[rxLen - PTX_NFCFORUMDTA_T3T_RDCODE_NEG_OFFSET], sizeof(uint8_t) * PTX_NFCFORUMDTA_T3T_RDCODE_SIZE);

            uint8_t option1[2] = {0x12, 0xFC};
            uint8_t option2[2] = {0x8E, 0xFC};

            if (0 == memcmp(RDCode, option1, PTX_NFCFORUMDTA_T3T_RDCODE_SIZE))
            {
                rxLen = PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE;
                sysCode = 0x12FC;
                rc = 0;
                tsn = 0;
                status = ptx_RUL_T3T_SensFReq(dta->rul, sysCode, rc, tsn, rx, &rxLen);
            }
            else if (0 == memcmp(RDCode, option2, PTX_NFCFORUMDTA_T3T_RDCODE_SIZE))
            {
                rxLen = PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE;
                sysCode = 0x8EFC;
                rc = 0;
                tsn = 0;
                status = ptx_RUL_T3T_SensFReq(dta->rul, sysCode, rc, tsn, rx, &rxLen);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                uint8_t nfcid2[PTX_T3T_NFCID2_SIZE];// = {0x02, 0xFE, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
                memcpy(nfcid2, &rx[2], sizeof(uint8_t) * PTX_T3T_NFCID2_SIZE);

                uint8_t payload[PTX_NFCFORUMDTA_T3T_LOOPBACK_PAYLOAD_SIZE];
                memset(payload, 0, sizeof(uint8_t) * PTX_NFCFORUMDTA_T3T_LOOPBACK_PAYLOAD_SIZE);

                uint8_t payload_rx[300];
                memset(payload_rx, 0, sizeof(uint8_t) * 300);
                uint32_t payload_rx_len = 300;

                payload[0] = 0xFF;
                payload[1] = PTX_NFCFORUMDTA_T3T_LOOPBACK_WRITE_CMD;
                payload[2] = 0x00;
                status = ptx_NFCForumDTA_GeneratePayload(&payload[3], nfcid2);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_RUL_Exchange(dta->rul, payload, PTX_NFCFORUMDTA_T3T_LOOPBACK_PAYLOAD_SIZE, payload_rx, &payload_rx_len, PTX_T3T_DEFAULT_TIMEOUT_MS);
                }

                if ((PTX_STATUS_SUCCESS == status) &&
                    (PTX_NFCFORUMDTA_T3T_LOOPBACK_PAYLOAD_SIZE == payload_rx[0]) &&
                    (PTX_NFCFORUMDTA_T3T_LOOPBACK_WRITE_RSP == payload_rx[1]) &&
                    (0 == memcmp(&payload[3], &payload_rx[3], sizeof(uint8_t) * PTX_NFCFORUMDTA_T3T_LOOPBACK_PAYLOAD_SIZE - 3)))
                {
                    payload_rx_len = 300;
                    status = ptx_RUL_Exchange(dta->rul, payload, PTX_NFCFORUMDTA_T3T_LOOPBACK_PAYLOAD_SIZE, payload_rx, &payload_rx_len, PTX_T3T_DEFAULT_TIMEOUT_MS);
                }


            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NFCForumDTA_ServiceIndepApplication(ptx_NFCForumDTA_t *dta)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(dta, Comp_NFC_Forum_DTA))
    {
        uint16_t sysCode = 0xFFFF;
        uint8_t rc = 1;
        uint8_t tsn = 0;

        uint8_t rx[PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE];
        uint32_t rxLen = PTX_NFCFORUMDTA_T3T_SENSFRES_SIZE;

        status = ptx_RUL_T3T_SensFReq(dta->rul, sysCode, rc, tsn, rx, &rxLen);

        if ((PTX_STATUS_SUCCESS == status) && (0 != rxLen))
        {
            uint8_t RDCode[PTX_NFCFORUMDTA_T3T_RDCODE_SIZE];
            memcpy(RDCode, &rx[rxLen - PTX_NFCFORUMDTA_T3T_RDCODE_NEG_OFFSET], sizeof(uint8_t) * PTX_NFCFORUMDTA_T3T_RDCODE_SIZE);

            uint8_t empty[2] = {0x00, 0x00};

            sysCode = (uint16_t)(RDCode[0] << 8);
            sysCode = (uint16_t)(sysCode + RDCode[1]);

            if (0 == memcmp(RDCode, empty, PTX_NFCFORUMDTA_T3T_RDCODE_SIZE))
            {
                sysCode = 0x12FC;
            }

            rc = 0;
            tsn = 0;
            status = ptx_RUL_T3T_SensFReq(dta->rul, sysCode, rc, tsn, rx, &rxLen);

        }
    }

    return status;
}
