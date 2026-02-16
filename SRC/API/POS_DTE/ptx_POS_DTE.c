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
    Module      : Point of Sale (POS) Device Test Environment (DTE) Add-on Application Interface (API)
    File        : ptx_POS_DTE.c

    Description : Implementation of POS DTE.
                  Contains the implementation of all the functions involved in POS DTE procedures, as well
                  as the internal helper functions used to perform certain actions via RUL API.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_POS_DTE.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_POS_DTE_INT_WORK_BUFFER_SIZE                    (32u)
#define PTX_POS_DTE_INT_MAX_RF_PARAMS                       (8u)
#define PTX_POS_DTE_INT_TYPE_A_UID_CL_LEN                   (5u)        /**< UID Length during AntiColl & Select = 4 Byte actual UID + 1 Byte BCC. */
#define PTX_POS_DTE_INT_TYPE_A_SAK_LEN                      (1u)
#define PTX_POS_DTE_MIN_APDU_LENGTH                         (6u)

/*
 * APDU Status-Words according to ISO 7816:
 */
#define PTX_POS_DTE_STATUS_SW1_OK                           (0x90u)
#define PTX_POS_DTE_STATUS_SW2_OK                           (0x00u)

#define PTX_POS_DTE_STATUS_SW12_OK                          (0x01u)
#define PTX_POS_DTE_STATUS_SW12_FAIL                        (0x00u)

#define PTX_POS_DTE_WUPA_CMD                                (0x52u)
#define PTX_POS_DTE_HLTA_CMD                                (0x50u)
#define PTX_POS_DTE_AC_SEL_CL1_CMD                          (0x93u)
#define PTX_POS_DTE_AC_SEL_CL2_CMD                          (0x95u)
#define PTX_POS_DTE_AC_SEL_CL3_CMD                          (0x97u)
#define PTX_POS_DTE_RATS_CMD                                (0xE0u)
#define PTX_POS_DTE_WUPB_CMD                                (0x05u)
#define PTX_POS_DTE_ATTRIB_CMD                              (0x1Du)
#define PTX_POS_DTE_IBLOCK0_CMD                             (0x02u)
#define PTX_POS_DTE_IBLOCK1_CMD                             (0x03u)

/*
 * Defines the possible Rf Deactivation type commands.
 */
#define PTX_POS_DTE_RF_DEACTIVATE_RESTART_DISCOVERY         (1u)
#define PTX_POS_DTE_RF_DEACTIVATE_IDLE                      (0u)

/*
 * Received APDU values used to signal next action/status during EMVCo loop:
 *  -> start card removal
 *  -> handle power off: start reader inactivation and loop exit procedure.
 */
#define PTX_POS_DTE_CARD_REMOVAL_POWER_OFF_PATTERN_OFFSET   (1u)
#define PTX_POS_DTE_CARD_REMOVAL_PATTERN                    (0x70u)
#define PTX_POS_DTE_POWER_OFF_PATTERN                       (0x72u)

/*
 * Defines maximum buffer size for the data exchange.
 */
#define PTX_POS_DTE_EXCHANGE_BUFFER_SIZE                    (266u)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_POS_DTE_INT_State_StartPolling(ptx_POS_DTE_t* ctx, ptx_POS_DTE_TargetRFTech_t rfTech);
static ptx_Status_t ptx_POS_DTE_INT_State_WaitForActivation(ptx_POS_DTE_t* ctx);
static ptx_Status_t ptx_POS_DTE_INT_State_ExchangePPSE(ptx_POS_DTE_t* ctx, uint8_t* rxData, uint32_t* rxDataLength);
static ptx_Status_t ptx_POS_DTE_INT_State_ExchangeAPDU(ptx_POS_DTE_t* ctx, uint8_t* rxData, uint32_t* rxDataLength);
static ptx_Status_t ptx_POS_DTE_INT_State_StartCardRemoval(ptx_POS_DTE_t* ctx);
static ptx_Status_t ptx_POS_DTE_INT_State_WaitCardRemoval(ptx_POS_DTE_t* ctx);
static ptx_Status_t ptx_POS_DTE_INT_State_RestartPolling(ptx_POS_DTE_t* ctx);
static ptx_Status_t ptx_POS_DTE_INT_State_DeactivateReader(ptx_POS_DTE_t* ctx);
static ptx_Status_t ptx_POS_DTE_INT_State_SystemError(ptx_POS_DTE_t* ctx, ptx_RUL_StatusInfo_t info);
static void ptx_POS_DTE_INT_HandlePPSE(ptx_POS_DTE_t* ctx, uint8_t* rxData, uint32_t rxDataLength);
static void ptx_POS_DTE_INT_HandleAPDU(ptx_POS_DTE_t* ctx, uint8_t* rxData, uint32_t rxDataLength);
static ptx_Status_t ptx_POS_DTE_INT_HandleRFStatus(ptx_POS_DTE_t* ctx, ptx_Status_t currentStatus);
static void ptx_POS_DTE_INT_CheckActiveCard(ptx_POS_DTE_t* ctx);
static uint8_t ptx_POS_DTE_INT_IsStatusWordOk(uint8_t* rxData, uint32_t rxDataLength);
static ptx_Status_t ptx_POS_DTE_INT_DeactivateReader(ptx_POS_DTE_t* ctx, uint8_t restartDiscovery);
static ptx_Status_t ptx_POS_DTE_INT_ConfigPolling(ptx_POS_DTE_t* ctx, ptx_POS_DTE_TargetRFTech_t rfTech);

static ptx_Status_t ptx_POS_DTE_ChangeRfSettings(ptx_POS_DTE_t *ctx, uint8_t useCrc, uint8_t nrTxBits, uint8_t rfTech);
static ptx_Status_t ptx_POS_DTE_Set_A_Short_Frame(ptx_POS_DTE_t *ctx);
static ptx_Status_t ptx_POS_DTE_Set_A_Standard_Frame(ptx_POS_DTE_t *ctx, uint8_t crcEnabled);
static ptx_Status_t ptx_POS_DTE_Set_B_Standard_Frame(ptx_POS_DTE_t *ctx);

static ptx_Status_t ptx_POS_DTE_Anticollision(ptx_POS_DTE_t *ctx, uint8_t cascadeLevelCode, uint8_t *uid);
static ptx_Status_t ptx_POS_DTE_Activate(ptx_POS_DTE_t *ctx, uint8_t cascadeLevelCode, uint8_t *uid, uint8_t *selRes);

static ptx_Status_t ptx_POS_DTE_TA_WupA_Cmd(ptx_POS_DTE_t *ctx);
static ptx_Status_t ptx_POS_DTE_TA_HltA_Cmd(ptx_POS_DTE_t *ctx);
static ptx_Status_t ptx_POS_DTE_TA_AntiColl_Cmd(ptx_POS_DTE_t *ctx);
static ptx_Status_t ptx_POS_DTE_TA_Select_Cmd(ptx_POS_DTE_t *ctx);
static ptx_Status_t ptx_POS_DTE_TA_Rats_Cmd(ptx_POS_DTE_t *ctx);
static ptx_Status_t ptx_POS_DTE_TA_WupB_Cmd(ptx_POS_DTE_t *ctx);
static ptx_Status_t ptx_POS_DTE_TA_Attrib_Cmd(ptx_POS_DTE_t *ctx);
static ptx_Status_t ptx_POS_DTE_TA_IBlock1_Cmd(ptx_POS_DTE_t *ctx);
static ptx_Status_t ptx_POS_DTE_TA_IBlock2_Cmd(ptx_POS_DTE_t *ctx);

const uint8_t ptx_POS_DTE_WUPA_CMD_BUF[]    = {PTX_POS_DTE_WUPA_CMD};
const uint8_t ptx_POS_DTE_HLTA_CMD_BUF[]    = {PTX_POS_DTE_HLTA_CMD,       0x00};
const uint8_t ptx_POS_DTE_ANTICOL_CMD_BUF[] = {PTX_POS_DTE_AC_SEL_CL1_CMD, 0x20u};
const uint8_t ptx_POS_DTE_SELECT_CMD_BUF[]  = {PTX_POS_DTE_AC_SEL_CL1_CMD, 0x70u, 0x27u, 0xE9u, 0x3Bu, 0x11u, 0xE4u};
const uint8_t ptx_POS_DTE_RATS_CMD_BUF[]    = {PTX_POS_DTE_RATS_CMD,       0x80};
const uint8_t ptx_POS_DTE_WUPB_CMD_BUF[]    = {PTX_POS_DTE_WUPB_CMD,       0x00, 0x08};
const uint8_t ptx_POS_DTE_ATTRIB_CMD_BUF[]  = {PTX_POS_DTE_ATTRIB_CMD,     0x46, 0xB5, 0xC7, 0xA0, 0x00, 0x08, 0x01, 0x00};
const uint8_t ptx_POS_DTE_IBLOCK1_CMD_BUF[] = {PTX_POS_DTE_IBLOCK0_CMD,    0x00u, 0xA4u, 0x04u, 0x00u, 0x0Eu, 0x32u, 0x50u, 0x41u, 0x59u, 0x2Eu, 0x53u, 0x59u, 0x53u, 0x2Eu, 0x44u, 0x44u, 0x46u, 0x30u, 0x31u, 0x00u};
const uint8_t ptx_POS_DTE_IBLOCK2_CMD_BUF[] = {PTX_POS_DTE_IBLOCK1_CMD,    0x00u, 0xA4u, 0x04u, 0x00u, 0x0Cu, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u, 0x08u, 0x09u, 0x0Au, 0x0Bu, 0x0Cu, 0x00u};
const uint8_t ptx_POS_DTE_C_APDU_PPSE[]     = {0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_POS_DTE_Init(ptx_POS_DTE_t* ctx, ptx_POS_DTE_InitParams_t* initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams) && (NULL != initParams->Rul))
    {
        (void)memset(ctx, 0, sizeof(ptx_POS_DTE_t));

        ctx->Rul                = initParams->Rul;
        ctx->Logger             = &(initParams->Rul->Log);
        ctx->ResetDelay         = initParams->ResetDelay;
        ctx->IopLoopback        = initParams->IopLoopback;
        ctx->IgnoreRFError      = initParams->IgnoreRFError;
        ctx->CbSuccess          = initParams->CbSuccess;
        ctx->CbError            = initParams->CbError;
        ctx->CbApdu             = initParams->CbApdu;
        ctx->EnableDDPC         = initParams->EnableDDPC == 0 ? 0 : 1;

        ctx->CompId = Comp_POS_DTE;

        status = ptx_POS_DTE_ResetState(ctx);

        if (PTX_STATUS_SUCCESS != status)
        {
            ctx->CompId = Comp_None;
        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_DeInit(ptx_POS_DTE_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_NSC_DataSetNrResidualTxBits(&(ctx->Rul->Nsc), 0);
        if ((PTX_STATUS_SUCCESS == status) && (ptx_POS_DTE_Carrier_ON == ctx->CarrierState))
        {
            status = ptx_POS_DTE_INT_DeactivateReader(ctx, PTX_POS_DTE_RF_DEACTIVATE_IDLE);
            ctx->CarrierState = ptx_POS_DTE_Carrier_OFF;
        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_ResetState(ptx_POS_DTE_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        ctx->TransacAState      = ptx_POS_DTE_TransacA_Init;
        ctx->TransacBState      = ptx_POS_DTE_TransacB_Init;
        ctx->LoopbackState      = ptx_POS_DTE_State_StartPolling;
        ctx->CarrierState       = ptx_POS_DTE_Carrier_OFF;

        status = ptx_NSC_DataSetNrResidualTxBits(&(ctx->Rul->Nsc), 0);
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_Carrier(ptx_POS_DTE_t* ctx, ptx_POS_DTE_CarrierState_t carrierReq)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        if (ctx->CarrierState != carrierReq)
        {
            if (ptx_POS_DTE_Carrier_ON == carrierReq)
            {
                ptx_RUL_DiscoverConfig_t discoveryCfg;
                memset(&discoveryCfg, 0, sizeof(discoveryCfg));

                discoveryCfg.EnablePolling = 1u;
                discoveryCfg.PollMode = ptx_RUL_PollMode_ContinuousWave;
                discoveryCfg.DisablePollIsoDep = 1u;
                discoveryCfg.EnableDDPC = ctx->EnableDDPC;

                /* Initiating a discovery loop with PollMode = Continuous-Wave will activate the Carrier and move to IDLE */
                status = ptx_RUL_InitiateDiscovery(ctx->Rul, &discoveryCfg);

                ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_TRANSAC_CARRIER_RESET_WAIT_MS);
            }
            else
            {
                if (ptx_POS_DTE_Carrier_OFF == carrierReq)
                {
                    status = ptx_POS_DTE_INT_DeactivateReader(ctx, PTX_POS_DTE_RF_DEACTIVATE_IDLE);
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->CarrierState = carrierReq;

                PTX_LOG_I(ctx->Logger, "RF-Carrier = %d", (int)carrierReq);
            }
        }
        else
        {
            // same state - nothing to do
            status = PTX_STATUS_SUCCESS;

        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_Polling(ptx_POS_DTE_t* ctx, ptx_POS_DTE_TargetRFTech_t rfTech)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        /* Stop any previously running polling loop (if any) by going to RF_IDLE */
        status = ptx_POS_DTE_Carrier(ctx, ptx_POS_DTE_Carrier_OFF);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_POS_DTE_INT_ConfigPolling(ctx, rfTech);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_InitiateDiscovery(ctx->Rul, &ctx->DiscoveryCfg);

                if (PTX_STATUS_SUCCESS == status)
                {
                    ctx->CarrierState = ptx_POS_DTE_Carrier_ON;

                    PTX_LOG_I(ctx->Logger, "RF-Polling Started");
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_Reset(ptx_POS_DTE_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_Carrier(ctx, ptx_POS_DTE_Carrier_OFF);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_RUL_Sleep(ctx->Rul, ctx->ResetDelay);

            status = ptx_POS_DTE_Carrier(ctx, ptx_POS_DTE_Carrier_ON);
        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_PowerOff(ptx_POS_DTE_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_Carrier(ctx, ptx_POS_DTE_Carrier_OFF);
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_WupA(ptx_POS_DTE_t* ctx, uint8_t *rx, uint32_t *rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
    uint32_t tx_buf_len;
    uint8_t rx_buf[PTX_POS_DTE_EXCHANGE_BUFFER_SIZE];
    uint32_t rx_buf_Len = PTX_POS_DTE_EXCHANGE_BUFFER_SIZE;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        if (ptx_POS_DTE_Carrier_ON == ctx->CarrierState)
        {
            status = ptx_POS_DTE_Set_A_Short_Frame(ctx);

            if (PTX_STATUS_SUCCESS == status)
            {
                (void)memcpy(&tx_buf[0], &ptx_POS_DTE_WUPA_CMD_BUF[0], sizeof(ptx_POS_DTE_WUPA_CMD_BUF));
                tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_WUPA_CMD_BUF);

                status = ptx_RUL_Exchange(ctx->Rul, &tx_buf[0], tx_buf_len, rx_buf, &rx_buf_Len, PTX_POS_DTE_DEFAULT_ACTIVATION_TIMEOUT_MS);

                if (PTX_STATUS_SUCCESS == status)
                {
                    PTX_LOG_I(ctx->Logger, "WUP-A OK");

                    if ((NULL != rx) && (NULL != rxLen))
                    {
                        if (2u == rx_buf_Len)
                        {
                            /* Store ATQA/SENS_RES */
                            *rxLen = rx_buf_Len;
                            (void)memcpy(&rx[0], &rx_buf[0], *rxLen);
                        }
                        else
                        {
                            status = PTX_STATUS_E_NSC_RFERROR;
                        }
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_RATS(ptx_POS_DTE_t *ctx, uint8_t *rx, uint32_t *rxLen)
{
    const uint8_t CASCADE_LEVEL_CMD[] = {PTX_POS_DTE_AC_SEL_CL1_CMD, PTX_POS_DTE_AC_SEL_CL2_CMD, PTX_POS_DTE_AC_SEL_CL3_CMD};

    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
    uint32_t tx_buf_len;
    uint32_t rx_buf_Len;
    uint8_t cascade_level;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        uint8_t sens_res[2];
        uint8_t nfcid1[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
        uint8_t sel_res = 0;
        uint8_t rats_res[128];

        if (ptx_POS_DTE_Carrier_ON == ctx->CarrierState)
        {
            status = ptx_POS_DTE_WupA(ctx, &sens_res[0], &rx_buf_Len);

            if (PTX_STATUS_SUCCESS == status)
            {
                for (uint8_t i = 0; i < 3 ; i++)
                {
                    cascade_level = CASCADE_LEVEL_CMD[i];

                    status = ptx_POS_DTE_Anticollision(ctx, cascade_level, &nfcid1[i * PTX_POS_DTE_INT_TYPE_A_UID_CL_LEN]);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        status = ptx_POS_DTE_Activate(ctx, cascade_level, &nfcid1[i * PTX_POS_DTE_INT_TYPE_A_UID_CL_LEN], &sel_res);
                    }

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* UID/NFCID1 complete ? Yes - exit loops */
                        if ((sel_res & 0x04u) == 0x00u)
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    (void)memcpy(&tx_buf[0], &ptx_POS_DTE_RATS_CMD_BUF[0], sizeof(ptx_POS_DTE_RATS_CMD_BUF));
                    tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_RATS_CMD_BUF);

                    rx_buf_Len = PTX_POS_DTE_EXCHANGE_BUFFER_SIZE;

                    status = ptx_RUL_Exchange(ctx->Rul, &tx_buf[0], tx_buf_len, &rats_res[0], &rx_buf_Len, PTX_POS_DTE_DEFAULT_ACTIVATION_TIMEOUT_MS);
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    PTX_LOG_I(ctx->Logger, "RATS OK");

                    if ((NULL != rx) && (NULL != rxLen))
                    {
                        uint32_t index = 0;

                        /* Store ATQA/SENS_RES */
                        (void)memcpy(&rx[index], &sens_res[0], 2u);
                        index = (uint32_t)(index + 2u);

                        /* Store UID/NFCID1 length + actual UID/NFCID1 */
                        switch (cascade_level)
                        {
                            case PTX_POS_DTE_AC_SEL_CL1_CMD:
                            {
                                rx[index + 0] = 4u;
                                (void)memcpy(&rx[index + 1], &nfcid1[0], 4u);
                                index = (uint32_t)(index + 1u + 4u);
                                break;
                            }

                            case PTX_POS_DTE_AC_SEL_CL2_CMD:
                            {
                                rx[index + 0] = 7u;
                                (void)memcpy(&rx[index + 1], &nfcid1[1], 3u);
                                (void)memcpy(&rx[index + 4], &nfcid1[5], 4u);
                                index = (uint32_t)(index + 1u + 7u);
                                break;
                            }

                            case PTX_POS_DTE_AC_SEL_CL3_CMD:
                            {
                                rx[index + 0] = 10u;
                                (void)memcpy(&rx[index + 1], &nfcid1[1], 3u);
                                (void)memcpy(&rx[index + 4], &nfcid1[6], 3u);
                                (void)memcpy(&rx[index + 7], &nfcid1[10], 4u);
                                index = (uint32_t)(index + 1u + 10u);
                                break;
                            }

                            default:
                                /* Not possible here */
                                break;
                        }

                        /* Store SAK/SEL_RES */
                        rx[index] = sel_res;
                        index++;

                        /* Store ATS/RATS_RES */
                        (void)memcpy(&rx[index], &rats_res[0], rx_buf_Len);
                        index = (uint32_t)(index + rx_buf_Len);

                        *rxLen = index;
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_WupB(ptx_POS_DTE_t *ctx, uint8_t *rx, uint32_t *rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
    uint32_t tx_buf_len;
    uint8_t rx_buf[PTX_POS_DTE_EXCHANGE_BUFFER_SIZE];
    uint32_t rx_buf_Len = PTX_POS_DTE_EXCHANGE_BUFFER_SIZE;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        if (ptx_POS_DTE_Carrier_ON == ctx->CarrierState)
        {
            status = ptx_POS_DTE_Set_B_Standard_Frame(ctx);

            if (PTX_STATUS_SUCCESS == status)
            {
                (void)memcpy(&tx_buf[0], &ptx_POS_DTE_WUPB_CMD_BUF[0], sizeof(ptx_POS_DTE_WUPB_CMD_BUF));
                tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_WUPB_CMD_BUF);

                status = ptx_RUL_Exchange(ctx->Rul, &tx_buf[0], tx_buf_len, rx_buf, &rx_buf_Len, PTX_POS_DTE_DEFAULT_ACTIVATION_TIMEOUT_MS);

                if (PTX_STATUS_SUCCESS == status)
                {
                    PTX_LOG_I(ctx->Logger, "WUP-B OK");

                    /* Store Card-ID in any case - need for potential next call to ptx_POS_DTE_ATTRIB */
                    (void)memcpy(&ctx->CardID[0], &rx_buf[1], (size_t)4);

                    if ((NULL != rx) && (NULL != rxLen))
                    {
                        /* Store ATQB/SENSB_RES */
                        *rxLen = rx_buf_Len;
                        (void)memcpy(&rx[0], &rx_buf[0], *rxLen);
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_STATE;
        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_ATTRIB(ptx_POS_DTE_t *ctx, uint8_t *rx, uint32_t *rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
    uint32_t tx_buf_len;
    uint8_t rx_buf[PTX_POS_DTE_EXCHANGE_BUFFER_SIZE];
    uint32_t rx_buf_Len = PTX_POS_DTE_EXCHANGE_BUFFER_SIZE;
    uint8_t sensb_res[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
    uint32_t sensb_res_len = 0;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_WupB(ctx, &sensb_res[0], &sensb_res_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            (void)memcpy(&tx_buf[0], &ptx_POS_DTE_ATTRIB_CMD_BUF[0], sizeof(ptx_POS_DTE_ATTRIB_CMD_BUF));
            (void)memcpy(&tx_buf[1], &ctx->CardID[0], (size_t)4);
            tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_ATTRIB_CMD_BUF);

            status = ptx_RUL_Exchange(ctx->Rul, &tx_buf[0], tx_buf_len, rx_buf, &rx_buf_Len, PTX_POS_DTE_DEFAULT_ACTIVATION_TIMEOUT_MS);

            if (PTX_STATUS_SUCCESS == status)
            {
                PTX_LOG_I(ctx->Logger, "Attrib OK");

                if ((NULL != rx) && (NULL != rxLen))
                {
                    uint32_t index = 0;

                    /* Store ATQB/SENSB_RES Length */
                    rx[index] = (uint8_t)sensb_res_len;
                    index++;

                    /* Store ATQB/SENSB_RES */
                    (void)memcpy(&rx[index], &sensb_res[0], sensb_res_len);
                    index = (uint32_t)(index + sensb_res_len);

                    /* Store ATTRIB_RES Length */
                    rx[index] = (uint8_t)rx_buf_Len;
                    index++;

                    (void)memcpy(&rx[index], &rx_buf[0], rx_buf_Len);
                    index = (uint32_t)(index + rx_buf_Len);

                    *rxLen = index;
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_TransacA(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        switch (ctx->TransacAState)
        {
            case ptx_POS_DTE_TransacA_Init:
            {
                if (ptx_POS_DTE_Carrier_ON == ctx->CarrierState)
                {
                    ctx->DisableFwt = 1u;
                    ctx->TransacAState = ptx_POS_DTE_TransacA_CMD_WUPA_1;
                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                }
                break;
            }

            case ptx_POS_DTE_TransacA_CMD_WUPA_1:
            {
                status = ptx_POS_DTE_TA_WupA_Cmd(ctx);
                ctx->TransacAState = ptx_POS_DTE_TransacA_CMD_HLTA;
                break;
            }

            case ptx_POS_DTE_TransacA_CMD_HLTA:
            {
                status = ptx_POS_DTE_TA_HltA_Cmd(ctx);
                ctx->TransacAState = ptx_POS_DTE_TransacA_CMD_WUPB;
                break;
            }

            case ptx_POS_DTE_TransacA_CMD_WUPB:
            {
                status = ptx_POS_DTE_TA_WupB_Cmd(ctx);
                ctx->TransacAState = ptx_POS_DTE_TransacA_CMD_WUPA_2;
                break;
            }

            case ptx_POS_DTE_TransacA_CMD_WUPA_2:
            {
                status = ptx_POS_DTE_TA_WupA_Cmd(ctx);
                ctx->TransacAState = ptx_POS_DTE_TransacA_CMD_ANTICOLLISION;
                break;
            }

            case ptx_POS_DTE_TransacA_CMD_ANTICOLLISION:
            {
                status = ptx_POS_DTE_TA_AntiColl_Cmd(ctx);
                ctx->TransacAState = ptx_POS_DTE_TransacA_CMD_SELECT;
                break;
            }

            case ptx_POS_DTE_TransacA_CMD_SELECT:
            {
                status = ptx_POS_DTE_TA_Select_Cmd(ctx);
                ctx->TransacAState = ptx_POS_DTE_TransacA_CMD_RATS;
                break;
            }

            case ptx_POS_DTE_TransacA_CMD_RATS:
            {
                status = ptx_POS_DTE_TA_Rats_Cmd(ctx);
                ctx->TransacAState = ptx_POS_DTE_TransacA_CMD_IBLOCK_1;
                break;
            }

            case ptx_POS_DTE_TransacA_CMD_IBLOCK_1:
            {
                status = ptx_POS_DTE_TA_IBlock1_Cmd(ctx);
                ctx->TransacAState = ptx_POS_DTE_TransacA_CMD_IBLOCK_2;
                break;
            }

            case ptx_POS_DTE_TransacA_CMD_IBLOCK_2:
            {
                status = ptx_POS_DTE_TA_IBlock2_Cmd(ctx);
                ctx->TransacAState = ptx_POS_DTE_TransacA_END;
                break;
            }

            case ptx_POS_DTE_TransacA_END:
            {
                ctx->DisableFwt = 0u;
                ctx->TransacAState = ptx_POS_DTE_TransacA_Init;
                status = ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_MIN_TIME_MS_TRANSAC_END);
                break;
            }

            default:
            {
                /* SHALL NEVER HAPPEN **/
                PTX_LOG_I(ctx->Logger, "Error - Invalid TRANSAC_A-State!");
                status = PTX_STATUS_E_GEN_INVALID_STATE;
                break;
            }
        }

        /* Transac function must ignore all data messages received.
        * Reset status code in case of an invalid length.
        * All other types of messages are already handled by NSC layer.
        */
        if ((PTX_STATUS_E_NSC_INVALID_DATA_LENGTH == status) || (PTX_STATUS_E_NSC_RX_ERROR_LENGTH == status))
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_GetTransacAState(ptx_POS_DTE_t *ctx, ptx_POS_DTE_TransacA_State_t *state)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((PTX_COMP_CHECK(ctx, Comp_POS_DTE)) && (NULL != state))
    {
        *state = ctx->TransacAState;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_TransacB(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        switch (ctx->TransacBState)
        {
            case ptx_POS_DTE_TransacB_Init:
            {
                if (ptx_POS_DTE_Carrier_ON == ctx->CarrierState)
                {
                    ctx->DisableFwt = 1u;
                    ctx->TransacBState = ptx_POS_DTE_TransacB_CMD_WUPB_1;
                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                }
                break;
            }

            case ptx_POS_DTE_TransacB_CMD_WUPB_1:
            {
                status = ptx_POS_DTE_TA_WupB_Cmd(ctx);
                ctx->TransacBState = ptx_POS_DTE_TransacB_CMD_WUPA;
                break;
            }

            case ptx_POS_DTE_TransacB_CMD_WUPA:
            {
                status = ptx_POS_DTE_TA_WupA_Cmd(ctx);
                ctx->TransacBState = ptx_POS_DTE_TransacB_CMD_WUPB_2;
                break;
            }

            case ptx_POS_DTE_TransacB_CMD_WUPB_2:
            {
                status = ptx_POS_DTE_TA_WupB_Cmd(ctx);
                ctx->TransacBState = ptx_POS_DTE_TransacB_CMD_ATTRIB;
                break;
            }

            case ptx_POS_DTE_TransacB_CMD_ATTRIB:
            {
                status = ptx_POS_DTE_TA_Attrib_Cmd(ctx);
                ctx->TransacBState = ptx_POS_DTE_TransacB_CMD_IBLOCK_1;
                break;
            }

            case ptx_POS_DTE_TransacB_CMD_IBLOCK_1:
            {
                status = ptx_POS_DTE_TA_IBlock1_Cmd(ctx);
                ctx->TransacBState = ptx_POS_DTE_TransacB_CMD_IBLOCK_2;
                break;
            }

            case ptx_POS_DTE_TransacB_CMD_IBLOCK_2:
            {
                status = ptx_POS_DTE_TA_IBlock2_Cmd(ctx);
                ctx->TransacBState = ptx_POS_DTE_TransacB_END;
                break;
            }

            case ptx_POS_DTE_TransacB_END:
            {
                ctx->DisableFwt = 0u;
                ctx->TransacBState = ptx_POS_DTE_TransacB_Init;
                status = ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_MIN_TIME_MS_TRANSAC_END);
                break;
            }

            default:
            {
                /* SHALL NEVER HAPPEN **/
                PTX_LOG_I(ctx->Logger, "Error - Invalid TRANSAC_B-State!");
                status = PTX_STATUS_E_GEN_INVALID_STATE;
                break;
            }
        }

        /* Transac function must ignore all data messages received.
        * Reset status code in case of an invalid length.
        * All other types of messages are already handled by NSC layer.
        */
        if ((PTX_STATUS_E_NSC_INVALID_DATA_LENGTH == status) || (PTX_STATUS_E_NSC_RX_ERROR_LENGTH == status))
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_GetTransacBState(ptx_POS_DTE_t *ctx, ptx_POS_DTE_TransacB_State_t *state)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((PTX_COMP_CHECK(ctx, Comp_POS_DTE)) && (NULL != state))
    {
        *state = ctx->TransacBState;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_Transac_Stop(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        ctx->TransacAState = ptx_POS_DTE_TransacA_Init;
        ctx->TransacBState = ptx_POS_DTE_TransacB_Init;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_Loopback(ptx_POS_DTE_t *ctx, ptx_POS_DTE_TargetRFTech_t rfTech)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        static uint8_t apdu_buffer[PTX_POS_DTE_EXCHANGE_BUFFER_SIZE];
        static uint32_t apdu_buffer_len = PTX_POS_DTE_EXCHANGE_BUFFER_SIZE;

        ptx_RUL_Status_Params_t status_params;
        status_params.Type = ptx_RUL_StatusType_System;

        status = ptx_RUL_GetStatusInfo(ctx->Rul, &status_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            if (ptx_RUL_StatusInfoSystem_OK != status_params.Info.System)
            {
                /* Critical - Overwrite state */
                ctx->LoopbackState = ptx_POS_DTE_State_SystemError;
            }

            switch (ctx->LoopbackState)
            {
                case ptx_POS_DTE_State_StartPolling:
                {
                    status = ptx_POS_DTE_INT_State_StartPolling(ctx, rfTech);
                    break;
                }

                case ptx_POS_DTE_State_WaitForActivation:
                {
                    status = ptx_POS_DTE_INT_State_WaitForActivation(ctx);
                    break;
                }

                case ptx_POS_DTE_State_ExchangePPSE:
                {
                    status = ptx_POS_DTE_INT_State_ExchangePPSE(ctx, &apdu_buffer[0], &apdu_buffer_len);
                    break;
                }

                case ptx_POS_DTE_State_ExchangeAPDU:
                {
                    status = ptx_POS_DTE_INT_State_ExchangeAPDU(ctx, &apdu_buffer[0], &apdu_buffer_len);
                    break;
                }

                case ptx_POS_DTE_State_StartCardRemoval:
                {
                    status = ptx_POS_DTE_INT_State_StartCardRemoval(ctx);
                    break;
                }

                case ptx_POS_DTE_State_WaitCardRemoval:
                {
                    status = ptx_POS_DTE_INT_State_WaitCardRemoval(ctx);
                    break;
                }

                case ptx_POS_DTE_State_RestartPolling:
                {
                    status = ptx_POS_DTE_INT_State_RestartPolling(ctx);
                    break;
                }

                case ptx_POS_DTE_State_DeactivateReader:
                {
                    status = ptx_POS_DTE_INT_State_DeactivateReader(ctx);
                    break;
                }

                case ptx_POS_DTE_State_SystemError:
                {
                    (void)ptx_POS_DTE_INT_State_SystemError(ctx, status_params.Info);
                    (void)ptx_POS_DTE_INT_State_DeactivateReader(ctx);
                    break;
                }

                case ptx_POS_DTE_State_Exit:
                {
                    (void)ptx_POS_DTE_INT_State_DeactivateReader(ctx);
                    break;
                }

                default:
                {
                    /* SHALL NEVER HAPPEN **/
                    PTX_LOG_I(ctx->Logger, "Error - Invalid Loopback-State!");
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                    break;
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_GetLoopbackState(ptx_POS_DTE_t *ctx, ptx_POS_DTE_LoopbackState_t *state)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((PTX_COMP_CHECK(ctx, Comp_POS_DTE)) && (NULL != state))
    {
        *state = ctx->LoopbackState;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_POS_DTE_Loopback_Stop(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        (void)ptx_POS_DTE_INT_DeactivateReader(ctx, PTX_POS_DTE_RF_DEACTIVATE_IDLE);
        ctx->LoopbackState = ptx_POS_DTE_State_StartPolling;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
static ptx_Status_t ptx_POS_DTE_INT_DeactivateReader(ptx_POS_DTE_t* ctx, uint8_t restartDiscovery)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        ptx_RUL_DeactivateParams_t deactParams;

        deactParams.Type = ptx_RUL_DeactivateType_Generic;
        deactParams.State = (PTX_POS_DTE_RF_DEACTIVATE_RESTART_DISCOVERY == restartDiscovery) ? ptx_RUL_DeactivateState_DiscoveryRfReset : ptx_RUL_DeactivateState_Idle;

        status = ptx_RUL_Deactivate(ctx->Rul, &deactParams);

        if (ptx_RUL_DeactivateState_Idle == deactParams.State)
        {
            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->CarrierState = ptx_POS_DTE_Carrier_OFF;
            }
            else if (PTX_STATUS_E_GEN_INVALID_STATE == status)
            {
                /* nothing wrong, state transition not permitted (IDLE -> IDLE), return success because reader is already Deactivated */
                status = PTX_STATUS_SUCCESS;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_INT_ConfigPolling(ptx_POS_DTE_t* ctx, ptx_POS_DTE_TargetRFTech_t rfTech)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        memset(&ctx->DiscoveryCfg, 0, sizeof(ptx_RUL_DiscoverConfig_t));

        ctx->DiscoveryCfg.EnablePolling = 1u;
        ctx->DiscoveryCfg.PollMode = ptx_RUL_PollMode_Emvco;
        ctx->DiscoveryCfg.PollIsoDepHbr = ptx_RUL_PollIsoDepHBR_Standard;
        ctx->DiscoveryCfg.DisableNfcDepProtocol = 1;
        ctx->DiscoveryCfg.EnableDDPC = ctx->EnableDDPC;

        if ((ptx_POS_DTE_TargetRFTech_A == rfTech) || (ptx_POS_DTE_TargetRFTech_A_B == rfTech))
        {
            ctx->DiscoveryCfg.PollTypeA = 1u;
            ctx->DiscoveryCfg.PollTypeAPollFrequency = 1u;
            ctx->DiscoveryCfg.PollTypeADeviceLimit = 1u;
        }

        if ((ptx_POS_DTE_TargetRFTech_B == rfTech) || (ptx_POS_DTE_TargetRFTech_A_B == rfTech))
        {
            ctx->DiscoveryCfg.PollTypeB = 1u;
            ctx->DiscoveryCfg.PollTypeBPollFrequency = 1u;
            ctx->DiscoveryCfg.PollTypeBDeviceLimit = 1u;
        }

        ctx->DiscoveryCfg.IdleTime = 0;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_INT_State_StartPolling(ptx_POS_DTE_t* ctx, ptx_POS_DTE_TargetRFTech_t rfTech)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_INT_ConfigPolling(ctx, rfTech);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_InitiateDiscovery(ctx->Rul, &ctx->DiscoveryCfg);

            if (PTX_STATUS_SUCCESS == status)
            {
                ctx->LoopbackState = ptx_POS_DTE_State_WaitForActivation;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_INT_State_WaitForActivation(ptx_POS_DTE_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Retrieve information from system. */
    ptx_RUL_Status_Params_t status_params;
    status_params.Type = ptx_RUL_StatusType_Discover;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_RUL_GetStatusInfo(ctx->Rul, &status_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (status_params.Info.Discover)
            {
                case ptx_RUL_StatusInfoDiscover_NoCard:
                {
                    ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_RF_DETECTION_WAIT_TIME_MS);
                    break;
                }

                case ptx_RUL_StatusInfoDiscover_Active:
                {
                    ptx_POS_DTE_INT_CheckActiveCard(ctx);
                    break;
                }

                case ptx_RUL_StatusInfoDiscover_CardDetectionOngoing:
                {
                    /* Do nothing, the state will be handled once the FW has reported all cards in the field */
                    break;
                }

                case ptx_RUL_StatusInfoDiscover_CardsAvailable:
                case ptx_RUL_StatusInfoDiscover_MultiProtocol:
                case ptx_RUL_StatusInfoDiscover_EmvCollision:
                {
                    PTX_LOG_I(ctx->Logger, "Collision Detected - Please remove additional Cards from the RF-Field or use only Cards supporting ISO-DEP!");

                    /* FW automatically restarts Polling - nothing else to do here. */
                    break;
                }

                default:
                {
                    /* SHALL NEVER HAPPEN **/
                    PTX_LOG_I(ctx->Logger, "Error - Unknown/Unsupported Discover-Status! %02X", (uint8_t)status_params.Info.Discover);
                    status = PTX_STATUS_E_GEN_INVALID_STATE;
                    break;
                }
            }
        }
        else
        {
            ctx->LoopbackState = ptx_POS_DTE_State_SystemError;
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_INT_State_ExchangePPSE(ptx_POS_DTE_t* ctx, uint8_t* rxData, uint32_t* rxDataLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t tx_data[PTX_POS_DTE_EXCHANGE_BUFFER_SIZE];
    uint32_t tx_data_len = (uint32_t)sizeof(ptx_POS_DTE_C_APDU_PPSE);

    if ((PTX_COMP_CHECK(ctx, Comp_POS_DTE)) && (NULL != rxData) && (NULL != rxDataLength))
    {
        memcpy(&tx_data[0], &ptx_POS_DTE_C_APDU_PPSE[0], tx_data_len);

        ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_EXCHANGE_PPSE_WAIT_TIME_MS);

        PTX_LOG_I_MEM(ctx->Logger, tx_data, tx_data_len, "TX = ");

        *rxDataLength = PTX_POS_DTE_EXCHANGE_BUFFER_SIZE;

        status = ptx_RUL_Exchange(ctx->Rul, &tx_data[0], tx_data_len, rxData, rxDataLength, PTX_POS_DTE_DEFAULT_PROTOCOL_TIMEOUT_MS);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_POS_DTE_INT_HandlePPSE(ctx, rxData, *rxDataLength);
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "PPSE Exchange failed due to transmission error! Status: %d", status);

            /* RF-Errors may happen - restart Polling, everything else needs to be handled by the Application */
            ctx->LoopbackState = ptx_POS_DTE_State_RestartPolling;

            status = ptx_POS_DTE_INT_HandleRFStatus(ctx, status);

            if (ctx->IopLoopback && NULL != ctx->CbError)
            {
                ctx->CbError();
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_INT_State_ExchangeAPDU(ptx_POS_DTE_t* ctx, uint8_t* rxData, uint32_t* rxDataLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != rxData) && (NULL != rxDataLength))
    {
        uint8_t tx_data[PTX_POS_DTE_EXCHANGE_BUFFER_SIZE];
        uint32_t tx_data_len = *rxDataLength - 2u;

        ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_EXCHANGE_APDU_WAIT_TIME_MS);

        if (tx_data_len > PTX_POS_DTE_EXCHANGE_BUFFER_SIZE)
        {
            tx_data_len = PTX_POS_DTE_EXCHANGE_BUFFER_SIZE;
        }

        (void)memcpy(tx_data, rxData, tx_data_len);

        PTX_LOG_I_MEM(ctx->Logger, tx_data, tx_data_len, "TX = ");

        *rxDataLength = PTX_POS_DTE_EXCHANGE_BUFFER_SIZE;

        status = ptx_RUL_Exchange(ctx->Rul, tx_data, tx_data_len, rxData, rxDataLength, PTX_POS_DTE_DEFAULT_PROTOCOL_TIMEOUT_MS);

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_POS_DTE_INT_HandleAPDU(ctx, rxData, *rxDataLength);
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "APDU Exchange failed due to transmission error! Status: %d", status);

            /* RF-Errors may happen - restart Polling, everything else needs to be handled by the Application */
            ctx->LoopbackState = ptx_POS_DTE_State_RestartPolling;

            status = ptx_POS_DTE_INT_HandleRFStatus(ctx, status);

            if (ctx->IopLoopback && NULL != ctx->CbError)
            {
                ctx->CbError();
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_INT_State_StartCardRemoval(ptx_POS_DTE_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_DeactivateParams_t deact_params = {0};

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        deact_params.Type = ptx_RUL_DeactivateType_Specific;
        deact_params.State = ptx_RUL_DeactivateState_Idle;

        status = ptx_RUL_Deactivate(ctx->Rul, &deact_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I(ctx->Logger, "Waiting for card to be removed!");
            ctx->LoopbackState = ptx_POS_DTE_State_WaitCardRemoval;
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_INT_State_WaitCardRemoval(ptx_POS_DTE_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        ptx_RUL_Status_Params_t status_params = {0};
        status_params.Type = ptx_RUL_StatusType_EmvRemoval;

        status = ptx_RUL_GetStatusInfo(ctx->Rul, &status_params);

        if ((PTX_STATUS_SUCCESS == status))
        {
            switch (status_params.Info.EmvRemoval)
            {
                case ptx_RUL_StatusInfoEmvRemoval_Pending:
                {
                    PTX_LOG_I(ctx->Logger, "Please remove card from field!");
                    ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_CARD_REMOVAL_WAIT_TIME_MS);
                    break;
                }

                default:
                {
                    PTX_LOG_I(ctx->Logger, "Card Removed!");
                    ctx->LoopbackState = ptx_POS_DTE_State_StartPolling;
                    break;
                }
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_INT_State_RestartPolling(ptx_POS_DTE_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        ptx_RUL_DeactivateParams_t deact_params;
        deact_params.Type = ptx_RUL_DeactivateType_Generic;
        deact_params.State = ptx_RUL_DeactivateState_DiscoveryRfReset;

        status = ptx_RUL_Deactivate(ctx->Rul, &deact_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I(ctx->Logger, "Restart done. Waiting for discovered cards...");
            ctx->LoopbackState = ptx_POS_DTE_State_WaitForActivation;
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_INT_State_DeactivateReader(ptx_POS_DTE_t* ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        (void)ptx_POS_DTE_INT_DeactivateReader(ctx, PTX_POS_DTE_RF_DEACTIVATE_IDLE);

        ctx->LoopbackState = ptx_POS_DTE_State_Exit;
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_INT_State_SystemError(ptx_POS_DTE_t* ctx, ptx_RUL_StatusInfo_t info)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        switch (info.System)
        {
            case ptx_RUL_StatusInfoSystem_ErrorOvercurrent:
            {
                PTX_LOG_I(ctx->Logger, "Error - Critical System-Error (Overcurrent) occurred - Quit Application");
                break;
            }

            case ptx_RUL_StatusInfoSystem_ErrorTemperature:
            {
                PTX_LOG_I(ctx->Logger, "Error - Critical System-Error (Temperature) occurred - Quit Application");
                break;
            }

            case ptx_RUL_StatusInfoSystem_ErrorVddTx:
            {
                PTX_LOG_I(ctx->Logger, "Error - Critical System-Error (VDD Tx) occurred - Quit Application");
                break;
            }

            case ptx_RUL_StatusInfoSystem_ErrorOTP:
            {
                PTX_LOG_I(ctx->Logger, "Error - Critical System-Error (OTP configuration) occurred - Quit Application");
                break;
            }

            default:
            {
                PTX_LOG_I(ctx->Logger, "Error - Critical System-Error (Unknown) occurred - Quit Application");
                break;
            }
        }
    }

    return status;
}

static void ptx_POS_DTE_INT_HandlePPSE(ptx_POS_DTE_t* ctx, uint8_t *rxData, uint32_t rxDataLength)
{
    ptx_Status_t status;

    if ((PTX_COMP_CHECK(ctx, Comp_POS_DTE)) && (NULL != rxData))
    {
        PTX_LOG_I_MEM(ctx->Logger, rxData, rxDataLength, "RX = ");

        if (PTX_POS_DTE_STATUS_SW12_OK == ptx_POS_DTE_INT_IsStatusWordOk(rxData, (uint32_t)rxDataLength))
        {
            if (PTX_POS_DTE_CARD_REMOVAL_PATTERN == rxData[PTX_POS_DTE_CARD_REMOVAL_POWER_OFF_PATTERN_OFFSET])
            {
                PTX_LOG_I(ctx->Logger, "EMVCO Loopback-Test Card Removal requested!");

                if ( (ctx->IopLoopback) && (NULL != ctx->CbSuccess) )
                {
                    ctx->CbSuccess();
                }

                ctx->LoopbackState = ptx_POS_DTE_State_StartCardRemoval;
            }
            else if ((PTX_POS_DTE_POWER_OFF_PATTERN == rxData[PTX_POS_DTE_CARD_REMOVAL_POWER_OFF_PATTERN_OFFSET]))
            {
                PTX_LOG_I(ctx->Logger, "EMVCO Loopback-Test Power Off requested!");

                status = ptx_POS_DTE_INT_DeactivateReader(ctx, PTX_POS_DTE_RF_DEACTIVATE_IDLE);

                if (PTX_STATUS_SUCCESS == status)
                {
                    (void)ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_LOOPBACK_POWER_OFF_RESET_WAIT_MS);

                    ctx->LoopbackState = ptx_POS_DTE_State_StartPolling;
                }
                else
                {
                    PTX_LOG_I(ctx->Logger, "Internal Error during POWER_OFF-sequence; STOP TEST!");

                    ctx->LoopbackState = ptx_POS_DTE_State_Exit;
                }
            }
            else
            {
                PTX_LOG_I(ctx->Logger, "EMVCO compatible Card Detected!");

                ctx->LoopbackState = ptx_POS_DTE_State_ExchangeAPDU;
            }
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "Warning - Detected Card is not EMVCO compatible!");

            ctx->LoopbackState = ptx_POS_DTE_State_RestartPolling;
        }
    }
}

static void ptx_POS_DTE_INT_HandleAPDU(ptx_POS_DTE_t* ctx, uint8_t* rxData, uint32_t rxDataLength)
{
    ptx_Status_t status;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        PTX_LOG_I_MEM(ctx->Logger, rxData, rxDataLength, "RX = ");

        if (PTX_POS_DTE_STATUS_SW12_OK == ptx_POS_DTE_INT_IsStatusWordOk(rxData, (uint32_t)rxDataLength))
        {
            if (PTX_POS_DTE_MIN_APDU_LENGTH > rxDataLength)
            {
                ctx->LoopbackState = ptx_POS_DTE_State_ExchangePPSE;
            }
            else if (PTX_POS_DTE_CARD_REMOVAL_PATTERN == rxData[PTX_POS_DTE_CARD_REMOVAL_POWER_OFF_PATTERN_OFFSET])
            {
                PTX_LOG_I(ctx->Logger, "EMVCO Loopback-Test Card Removal requested!");

                if ( (ctx->IopLoopback) && (NULL != ctx->CbSuccess) )
                {
                    ctx->CbSuccess();
                }

                ctx->LoopbackState = ptx_POS_DTE_State_StartCardRemoval;
            }
            else if ((PTX_POS_DTE_POWER_OFF_PATTERN == rxData[PTX_POS_DTE_CARD_REMOVAL_POWER_OFF_PATTERN_OFFSET]))
            {
                PTX_LOG_I(ctx->Logger, "EMVCO Loopback-Test Power Off requested!");

                status = ptx_POS_DTE_INT_DeactivateReader(ctx, PTX_POS_DTE_RF_DEACTIVATE_IDLE);

                if (PTX_STATUS_SUCCESS == status)
                {
                    (void)ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_LOOPBACK_POWER_OFF_RESET_WAIT_MS);

                    ctx->LoopbackState = ptx_POS_DTE_State_StartPolling;
                }
                else
                {
                    PTX_LOG_I(ctx->Logger, "Internal Error during POWER_OFF-sequence; STOP TEST!");

                    ctx->LoopbackState = ptx_POS_DTE_State_Exit;
                }
            }
            else
            {
                /*
                * Nothing to do - just stay in this state and send next APDU
                */
                if (NULL != ctx->CbApdu)
                {
                    ctx->CbApdu(rxData, rxDataLength);
                }
            }
        }
        else
        {
            PTX_LOG_I(ctx->Logger, "EMVCO Loopback-Test finished with Application-Errors!");

            if ( (ctx->IopLoopback) && (NULL != ctx->CbError) )
            {
                ctx->CbError();
            }

            ctx->LoopbackState = ptx_POS_DTE_State_DeactivateReader;
        }
    }
}

static ptx_Status_t ptx_POS_DTE_INT_HandleRFStatus(ptx_POS_DTE_t* ctx, ptx_Status_t currentStatus)
{
    /* Default assignment here would return a new status in case Component-check would fail, but this must not happen anyway at this location. */
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = currentStatus;

        if (PTX_STATUS_E_NSC_RFERROR == status)
        {
            /* Reset status if corresponding ignore-flag is set */
            if (0 != ctx->IgnoreRFError)
            {
                status = PTX_STATUS_SUCCESS;
            }
        }
    }

    return status;
}

static void ptx_POS_DTE_INT_CheckActiveCard(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        ptx_RUL_CardRegistry_t *card_registry;

        status = ptx_RUL_GetCardRegistry(ctx->Rul, &card_registry);

        if (PTX_STATUS_SUCCESS == status)
        {
            switch (card_registry->ActiveProtocol)
            {
                case ptx_RUL_CardProtocolType_T4T:
                {
                    PTX_LOG_I(ctx->Logger, "ISO-DEP compatible Card Detected!");
                    ctx->LoopbackState = ptx_POS_DTE_State_ExchangePPSE;
                    break;
                }

                default:
                {
                    PTX_LOG_I(ctx->Logger, "Warning - Detected Card and / or Protocol is not supported by EMV/POS!");
                    ctx->LoopbackState = ptx_POS_DTE_State_RestartPolling;
                    break;
                }
            }
        }
    }
}

static uint8_t ptx_POS_DTE_INT_IsStatusWordOk(uint8_t* rxData, uint32_t rxDataLength)
{
    uint8_t status_word_ok = PTX_POS_DTE_STATUS_SW12_FAIL;

    if (NULL != rxData)
    {
        if (rxDataLength >= 2u)
        {
            if ((PTX_POS_DTE_STATUS_SW1_OK == rxData[rxDataLength - 2u]) && (PTX_POS_DTE_STATUS_SW2_OK == rxData[rxDataLength - 1u]))
            {
                status_word_ok = PTX_POS_DTE_STATUS_SW12_OK;
            }
        }
    }

    return status_word_ok;
}

static ptx_Status_t ptx_POS_DTE_TA_WupA_Cmd(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_Set_A_Short_Frame(ctx);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
            uint32_t tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_WUPA_CMD_BUF);
            memcpy(&tx_buf[0], &ptx_POS_DTE_WUPA_CMD_BUF[0], tx_buf_len);

            status = ptx_RUL_Send(ctx->Rul, &tx_buf[0], tx_buf_len);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_MIN_TIME_MS_TRANSAC_CMD);
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_TA_HltA_Cmd(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_Set_A_Standard_Frame(ctx, (uint8_t)1);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
            uint32_t tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_HLTA_CMD_BUF);
            memcpy(&tx_buf[0], &ptx_POS_DTE_HLTA_CMD_BUF[0], tx_buf_len);

            status = ptx_RUL_Send(ctx->Rul, &tx_buf[0], tx_buf_len);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_MIN_TIME_MS_TRANSAC_CMD);
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_TA_Rats_Cmd(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
        uint32_t tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_RATS_CMD_BUF);
        memcpy(&tx_buf[0], &ptx_POS_DTE_RATS_CMD_BUF[0], tx_buf_len);

        status = ptx_RUL_Send(ctx->Rul, &tx_buf[0], tx_buf_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_MIN_TIME_MS_TRANSAC_CMD);
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_TA_AntiColl_Cmd(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_Set_A_Standard_Frame(ctx, (uint8_t)0);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
            uint32_t tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_ANTICOL_CMD_BUF);
            memcpy(&tx_buf[0], &ptx_POS_DTE_ANTICOL_CMD_BUF[0], tx_buf_len);

            status = ptx_RUL_Send(ctx->Rul, &tx_buf[0], tx_buf_len);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_MIN_TIME_MS_TRANSAC_CMD);
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_TA_Select_Cmd(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_Set_A_Standard_Frame(ctx, (uint8_t)1);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
            uint32_t tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_SELECT_CMD_BUF);
            memcpy(&tx_buf[0], &ptx_POS_DTE_SELECT_CMD_BUF[0], tx_buf_len);

            status = ptx_RUL_Send(ctx->Rul, &tx_buf[0], tx_buf_len);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_MIN_TIME_MS_TRANSAC_CMD);
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_TA_WupB_Cmd(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_Set_B_Standard_Frame(ctx);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
            uint32_t tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_WUPB_CMD_BUF);
            memcpy(&tx_buf[0], &ptx_POS_DTE_WUPB_CMD_BUF[0], tx_buf_len);

            status = ptx_RUL_Send(ctx->Rul, &tx_buf[0], tx_buf_len);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_MIN_TIME_MS_TRANSAC_CMD);
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_TA_Attrib_Cmd(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
        uint32_t tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_ATTRIB_CMD_BUF);
        memcpy(&tx_buf[0], &ptx_POS_DTE_ATTRIB_CMD_BUF[0], tx_buf_len);

        status = ptx_RUL_Send(ctx->Rul, &tx_buf[0], tx_buf_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_MIN_TIME_MS_TRANSAC_CMD);
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_TA_IBlock1_Cmd(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
        uint32_t tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_IBLOCK1_CMD_BUF);
        memcpy(&tx_buf[0], &ptx_POS_DTE_IBLOCK1_CMD_BUF[0], tx_buf_len);

        status = ptx_RUL_Send(ctx->Rul, &tx_buf[0], tx_buf_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_MIN_TIME_MS_TRANSAC_CMD);
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_TA_IBlock2_Cmd(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
        uint32_t tx_buf_len = (uint32_t)sizeof(ptx_POS_DTE_IBLOCK2_CMD_BUF);
        memcpy(&tx_buf[0], &ptx_POS_DTE_IBLOCK2_CMD_BUF[0], tx_buf_len);

        status = ptx_RUL_Send(ctx->Rul, &tx_buf[0], tx_buf_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_Sleep(ctx->Rul, PTX_POS_DTE_MIN_TIME_MS_TRANSAC_CMD);
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_Anticollision(ptx_POS_DTE_t *ctx, uint8_t cascadeLevelCode, uint8_t *uid)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((PTX_COMP_CHECK(ctx, Comp_POS_DTE)) && (NULL != uid))
    {
        status = ptx_POS_DTE_Set_A_Standard_Frame(ctx, (uint8_t)0);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
            memset(&tx_buf, 0, PTX_POS_DTE_INT_WORK_BUFFER_SIZE);
            uint32_t tx_buf_len = 2u;

            tx_buf[0] = cascadeLevelCode;
            tx_buf[1] = 0x20u;

            uint8_t rx_buf[PTX_POS_DTE_EXCHANGE_BUFFER_SIZE];
            uint32_t rx_buf_Len = PTX_POS_DTE_EXCHANGE_BUFFER_SIZE;

            status = ptx_RUL_Exchange(ctx->Rul, &tx_buf[0], tx_buf_len, &rx_buf[0], &rx_buf_Len, PTX_POS_DTE_DEFAULT_ACTIVATION_TIMEOUT_MS);

            if ((PTX_STATUS_SUCCESS == status) && (PTX_POS_DTE_INT_TYPE_A_UID_CL_LEN == rx_buf_Len))
            {
                (void)memcpy(uid, &rx_buf[0], rx_buf_Len);
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_Activate(ptx_POS_DTE_t *ctx, uint8_t cascadeLevelCode, uint8_t *uid, uint8_t *selRes)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((PTX_COMP_CHECK(ctx, Comp_POS_DTE)) && (NULL != uid) && (0 != selRes))
    {
        status = ptx_POS_DTE_Set_A_Standard_Frame(ctx, (uint8_t)1);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t tx_buf[PTX_POS_DTE_INT_WORK_BUFFER_SIZE];
            memset(&tx_buf, 0, PTX_POS_DTE_INT_WORK_BUFFER_SIZE);
            uint32_t tx_buf_len = 2u + PTX_POS_DTE_INT_TYPE_A_UID_CL_LEN;

            tx_buf[0] = cascadeLevelCode;
            tx_buf[1] = 0x70u;
            memcpy(&tx_buf[2], uid, PTX_POS_DTE_INT_TYPE_A_UID_CL_LEN);

            uint8_t rx_buf[PTX_POS_DTE_EXCHANGE_BUFFER_SIZE];
            uint32_t rx_buf_Len = PTX_POS_DTE_EXCHANGE_BUFFER_SIZE;

            status = ptx_RUL_Exchange(ctx->Rul, &tx_buf[0], tx_buf_len, &rx_buf[0], &rx_buf_Len, PTX_POS_DTE_DEFAULT_ACTIVATION_TIMEOUT_MS);

            if ((PTX_STATUS_SUCCESS == status) && (PTX_POS_DTE_INT_TYPE_A_SAK_LEN == rx_buf_Len))
            {
                *selRes = rx_buf[0];
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_Set_A_Short_Frame(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_ChangeRfSettings(ctx, NSC_RfSetParameter_Crc_Disable, 0x07u, NSC_RfSetParameter_RfTech_NFC_A);
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_Set_A_Standard_Frame(ptx_POS_DTE_t *ctx, uint8_t crcEnabled)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_ChangeRfSettings(ctx, crcEnabled, 0x00u, NSC_RfSetParameter_RfTech_NFC_A);
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_Set_B_Standard_Frame(ptx_POS_DTE_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        status = ptx_POS_DTE_ChangeRfSettings(ctx, NSC_RfSetParameter_Crc_Enable, 0x00u, NSC_RfSetParameter_RfTech_NFC_B);
    }

    return status;
}

static ptx_Status_t ptx_POS_DTE_ChangeRfSettings(ptx_POS_DTE_t *ctx, uint8_t useCrc, uint8_t nrTxBits, uint8_t rfTech)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_POS_DTE))
    {
        uint32_t rf_par_len = 0u;
        ptx_NSC_RfSetParameter_Param_t rf_par_buf[PTX_POS_DTE_INT_MAX_RF_PARAMS];

        (void)memset(rf_par_buf, 0, PTX_POS_DTE_INT_MAX_RF_PARAMS * sizeof(ptx_NSC_RfSetParameter_Param_t));

        if (NSC_RfSetParameter_RfTech_NFC_A == rfTech)
        {
            /* [Param ID 0x03]: Parity Set: Tx:ON / Rx:ON; only applicable to NFC-A Tech */
            rf_par_buf[rf_par_len].Type = NSC_RfSetParameter_Param_Type_Parity;

            if (0x07u == nrTxBits)
            {
                /* Disable Parity for Tx short frame */
                rf_par_buf[rf_par_len].Value.Parity.TxParity = NSC_RfSetParameter_Parity_Disable;
            }
            else
            {
                rf_par_buf[rf_par_len].Value.Parity.TxParity = NSC_RfSetParameter_Parity_Enable;
            }

            rf_par_buf[rf_par_len].Value.Parity.RxParity = NSC_RfSetParameter_Parity_Enable;
            rf_par_len++;
        }

        /* [Param ID 0x04]: Tx/Rx CRC Set */
        rf_par_buf[rf_par_len].Type = NSC_RfSetParameter_Param_Type_Crc;
        rf_par_buf[rf_par_len].Value.Crc.TxCrc = (ptx_NSC_RfSetParameter_Crc_Cfg_t)useCrc;
        rf_par_buf[rf_par_len].Value.Crc.RxCrc = (ptx_NSC_RfSetParameter_Crc_Cfg_t)useCrc;
        rf_par_len++;

        /* [Param ID 0x06]: MAX_RSP_LIM */
        rf_par_buf[rf_par_len].Type = NSC_RfSetParameter_Param_Type_MaxRspLim;
        rf_par_buf[rf_par_len].Value.MaxRspLim.MaxRspLimValue = 0x01u;
        rf_par_len++;

        /* Default - send 8 bits per byte (value = 0) */
        (void)ptx_NSC_DataSetNrResidualTxBits(&(ctx->Rul->Nsc), 0);

        if ((NSC_RfSetParameter_RfTech_NFC_A == rfTech) && (0 != nrTxBits))
        {
            /* [Param ID 0x05]: Bit Align (Offset and TX Residual Bits); only applicable to NFC-A Tech */
            (void)ptx_NSC_DataSetNrResidualTxBits(&(ctx->Rul->Nsc), nrTxBits);
        }

        /* [Param ID 0x07]: FWT Field; has time units of 128/Fc . This is 9,43 microsec. */
        rf_par_buf[rf_par_len].Type = NSC_RfSetParameter_Param_Type_Fwt;

        if (ctx->DisableFwt)
        {
            rf_par_buf[rf_par_len].Value.Fwt.FwtValue[0u] = 0x00u;
            rf_par_buf[rf_par_len].Value.Fwt.FwtValue[1u] = 0x00u;
            rf_par_buf[rf_par_len].Value.Fwt.FwtValue[2u] = 0x00u;
        }
        else
        {
            rf_par_buf[rf_par_len].Value.Fwt.FwtValue[0u] = 0x4Cu;
            rf_par_buf[rf_par_len].Value.Fwt.FwtValue[1u] = 0x27u;
            rf_par_buf[rf_par_len].Value.Fwt.FwtValue[2u] = 0x01u;
        }
        rf_par_len++;

        /* [Param ID 0x01]: RfTech */
        rf_par_buf[rf_par_len].Type = NSC_RfSetParameter_Param_Type_RfTech;
        rf_par_buf[rf_par_len].Value.RfTech = (ptx_NSC_RfSetParameter_Param_Rf_Tech_t)rfTech;
        rf_par_len++;

        /* [Param ID 0x02]: BitRate */
        rf_par_buf[rf_par_len].Type = NSC_RfSetParameter_Param_Type_TxBitRate;
        rf_par_buf[rf_par_len].Value.TxBitRate.BitrateValue = PTX_NSC_RF_SET_BITRATE_106;
        rf_par_len++;

        /* [Param ID 0x02]: BitRate */
        rf_par_buf[rf_par_len].Type = NSC_RfSetParameter_Param_Type_RxBitRate;
        rf_par_buf[rf_par_len].Value.RxBitRate.BitrateValue = PTX_NSC_RF_SET_BITRATE_106;
        rf_par_len++;

        status = ptx_NSC_RfSetParameter(&(ctx->Rul->Nsc), &rf_par_buf[0], rf_par_len);
    }

    return status;
}

