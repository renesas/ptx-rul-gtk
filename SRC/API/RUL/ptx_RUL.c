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
    Module      : RUL
    File        : ptx_RUL.c

    Description : Reader Universal Library (RUL) API Implementation
                  Contains functions to use a PTX Device with RUL functionality.
 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL.h"
#include "ptx_RUL_Int.h"
#include "ptx_NSC_Registers.h"
#include "ptx_NSC_RfSetRoutingTable.h"
#include "ptx_Status.h"
#include "ptx_HAL.h"
#include <stdint.h>



/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_RUL_QMEAS_CENTER_FREQ       (13560000U)     /* Q-measurement center frequency in Hz (= 13.56MHz). */
#define PTX_RUL_ADC_MAXAMPLSB           (235U)          /* Max. amplitude LSB. */
#define PTX_RUL_AMP_MEAS_FACTOR         (1000U)         /* Amplitude measurement is scaled by a factor of 1000. */

/*
 * T3T SENSF-REQ defines.
 */
#define PTX_RUL_T3T_SENSFREQ_TXR_BUFFER_SIZE    253u        /* Maximum TRx buffer size for T3T-SensF-Req. */
#define PTX_RUL_T3T_SENSFREQ_COMMAND_CODE       0u          /* SensF-Req command code. */
#define PTX_RUL_T3T_SENSFREQ_COMMAND_LEN        6u          /* SensF-Req command length. */
#define PTX_RUL_T3T_SENSFREQ_MAX_TSN            16u         /* Maximum timeslot number. */
#define PTX_RUL_T3T_SENSFREQ_APPTIMEOUT_MS      500u        /* Application timeout. Must be larger than possible max. FWT (~21 ms). */
#define PTX_RUL_T3T_MAX_PARAMS                  (uint32_t)2 /* Maximum number of parameters used for T3T during NSC.RF_SET_PARAMETER */

/*
 * Frame Waiting Time lookup table for A10 (in ticks).
 *
 * Timeout of 2,44 + [1...15] * 1,22 => ~21ms (1 == min. timeslots; 15 == max. timeslots)
 *
 * Internally uses a resolution of 128/fc per tick ~ 9,44us; 1 ms is roughly 106 ticks (~1.00059ms)
 * Note: The resolution is limited to 24-bits i.e. the max. value is roughly 158 s.
 *
 * Calculated as follows: ceiling((2.44 + [1...16]*1.22 * 10^-3)/(64/fc)) [ticks]
 */

static const uint32_t ptx_RUL_SensFReqFWT[PTX_RUL_T3T_SENSFREQ_MAX_TSN] =
{
    776, 1034, 1293, 1551, 1810, 2068, 2327, 2585, 2844, 3102, 3361, 3619, 3878, 4136, 4395, 4653
};

/*
 * Bits Exchange definitions.
 */
#define PTX_RUL_BITSEXCHANGE_PARITYMASK             (uint8_t)0x80       /* Mask used in parity parsing of bitstream rsp. */

/**
 * \brief RUL Clock Configuration consisting of the reference frequency
 *          and the desired target frequency.
 */
typedef struct ptx_RUL_Clk_Config
{
    uint32_t            RefFreq;        /**< Reference frequency in Hz. */
    uint32_t            TargetFreq;     /**< Target/current frequency in Hz. */
} ptx_RUL_Clk_Config_t;

/**
 * \brief Calculated RUL Clock Parameters to achieve the target frequency with the given reference frequency.
 */
typedef struct ptx_RUL_Clk_Params
{
    uint8_t             XtalDiv2En;     /**< XTAL output divider enable. */
    uint16_t            GatePw;         /**< Number of reference clock periods the sys_clk counter should be enabled for during PLL calibration. */
    uint16_t            SysCntTg;       /**< Expected number of clock pulses during PLL calibration. */
    uint32_t            NHost;          /**< PLL division factor for the selected RF field frequency. */
} ptx_RUL_Clk_Params_t;

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
/*
 * Allocate and Initialise RUL Component.
 */
static ptx_Status_t ptx_RUL_AllocInit(ptx_RUL_t *rul, ptx_RUL_InitParams_t *initParams);

/*
* Internal function to map from RUL Hardware Config to NSC Rf Config.
*/
static ptx_Status_t ptx_RUL_MapRfConfig(ptx_RUL_HWConfig_t *hwConfig, ptx_NSC_RfConfig_Param_t *nscRfParams, uint8_t *rfParamsIdx);

/*
 * Internal function to map from RUL Hardware Config to NSC System Config.
 */
static ptx_Status_t ptx_RUL_MapSysConfig(ptx_RUL_HWConfig_t *hwConfig, ptx_NSC_SystemConfig_Param_t *nscSysParams, uint8_t *sysParamsIdx);

/*
 * Decode bitstream containing parity bits.
 */
static ptx_Status_t ptx_RUL_DecodeRawBits(ptx_RUL_t *rul, uint8_t *pldRawBits, uint32_t lengthRawBits, uint8_t residualBits,
                                          uint8_t *pldBytes, uint8_t *pldParityBits, uint32_t *lengthBytes, uint32_t *numTotalBits);

/*
 * Internal function to check the availability of the GPIO number.
*/
static ptx_Status_t ptx_RUL_CheckGpioAvailability(ptx_RUL_t *rul, ptx_RUL_GPIONbr_t gpioNbr);

/*
 * Internal function to retrieve the requested GPIO details (HW Address and Config Offset).
*/
static ptx_Status_t ptx_RUL_GetGpioAddress(ptx_RUL_GPIONbr_t gpioNbr, uint16_t *gpioHWAddr, uint8_t *gpioCfgOffset);

/*
 * Internal function to retrieve the requested GPIO Features set.
*/
static ptx_Status_t ptx_RUL_GetGpioFeatures(ptx_RUL_GPIONbr_t gpioNbr, uint8_t *gpioFeatures);

/*
 * Internal function to calculate PLL parameters.
 */
static ptx_Status_t ptx_RUL_CalculatePllParameters(ptx_RUL_t *rul, ptx_RUL_Clk_Config_t *clkConfig, ptx_RUL_Clk_Params_t *clkParams);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_RUL_Init(ptx_RUL_t *rul, ptx_RUL_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != rul) && (NULL != initParams))
    {
        memset(rul, 0, sizeof(ptx_RUL_t));

        status = ptx_RUL_AllocInit(rul, initParams);

        PTX_LOG_I(&rul->Log, "ptx_RUL_Init");

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_NSC_CmdProcessor_Cfg_t cmdproc_cfg_params;

            /* Set to NULL all parameters initially. The parameters which are NULL, do not get configured. */
            (void)memset(&cmdproc_cfg_params, 0, sizeof(ptx_NSC_CmdProcessor_Cfg_t));

            cmdproc_cfg_params.Ctx = rul;
            cmdproc_cfg_params.ProcessErrorNtf = ptx_RUL_ProcessErrorNtf;
            cmdproc_cfg_params.ProcessRfDiscoverNtf = ptx_RUL_ProcessRfDiscoverNtf;
            cmdproc_cfg_params.ProcessRfLpcdNtf = ptx_RUL_ProcessRfLpcdNtf;
            cmdproc_cfg_params.ProcessRfActivateNtf = ptx_RUL_ProcessRfActivateNtf;
            cmdproc_cfg_params.ProcessRfDeactivateNtf = ptx_RUL_ProcessRfDeactivateNtf;
            cmdproc_cfg_params.ProcessResetNtf = ptx_RUL_ProcessResetNtf;
            cmdproc_cfg_params.ProcessRfFieldNtf = ptx_RUL_ProcessRfFieldNtf;
            cmdproc_cfg_params.ProcessWptStopNtf = ptx_RUL_ProcessWlcWptStopNtf;
            cmdproc_cfg_params.ProcessDataMsg = ptx_RUL_ProcessDataMsg;

            status = ptx_NSC_CmdProcessor_Config (&(rul->Nsc), &cmdproc_cfg_params);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            rul->EmvCollisionFilter.Limit = initParams->EmvCollisionLimit;
            rul->SystemStatus = ptx_RUL_StatusInfoSystem_OK;
            rul->CompId = Comp_RUL;
            rul->ActiveExtensionEndpoint = NULL;
            status = ptx_RUL_ClearCardRegistry(rul);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_InitHardware(ptx_RUL_t* rul)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_InitHardware");

        /*
         * Reset DFY.
         */
        status = ptx_NSC_Reset(&(rul->Nsc), ptx_NSC_Reset_Soft);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_Firmware_Download(&(rul->Nsc), PTX_NSC_FIRMWARE_FORCE_DOWNLOAD_ON, NULL);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_Firmware_Activate(&(rul->Nsc), NULL);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_InitHw(&(rul->Nsc));
        }

#if defined(PTX_ENABLE_UART) && defined(PTX_APP_DIVERSITY_LINUX)
        if ((PTX_STATUS_SUCCESS == status) && (HAL_Interface_Type_UART == rul->Hal.Interface.Type))
        {
            /*
             * On Linux Uart, this workaround is needed to get faster timings as the tcdrain() function blocks for 8-12ms more than necessary.
             * For a detailed description see the comment in the ptx_HAL_DRV_Interface_UART.c file in the ptx_HAL_DRV_Interface_UART_Tx function.
             */
            ptx_HAL_DRV_Interface_Parameter_t hal_param;
            hal_param.type = HAL_DRV_Interface_Parameter_Type_UART_Enable_Fast_Mode;
            hal_param.value.LinuxUARTEnableFastMode = 1;
            status = ptx_HAL_Interface_SetParameter(&rul->Hal, &hal_param);
        }
#endif

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RfConfig(&(rul->Nsc), NULL, 0);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (ptx_RUL_StatusInfoSystem_NSCReset == rul->SystemStatus)
            {
                /* Overwrite system status change from Reset NTF's */
                rul->SystemStatus = ptx_RUL_StatusInfoSystem_OK;
            }
        }
        else
        {
           /*
            * Overtemperature is the only expected error during initial hardware configuration:
            * If the overtemperature threshold is set too low, the device may enter this state
            * already during NSC_INIT_CMD. In that situation, the NSC_INIT_CMD does not receive
            * its regular response; instead, an overtemperature error notification is sent and
            * the system powers down.
            *
            * Without this check below, the caller would only see PTX_STATUS_E_NSC_PROTOCOL.
            * Therefore, translate this case to PTX_STATUS_E_GEN_OVERTEMPERATURE.
            */
            if (ptx_RUL_StatusInfoSystem_ErrorTemperature == rul->SystemStatus)
            {
                status = PTX_STATUS_E_GEN_OVERTEMPERATURE;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_DeInit(ptx_RUL_t *rul)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_DeInit");
        /* Reset & Disable DFY to reduce power consumption. */
        status = ptx_HIP_SysRst(&(rul->Hip), HIP_Cmd_SysRst_RstDisDfy);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_DeInit(&(rul->Nsc));
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HIP_Close(&(rul->Hip));
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_HAL_DeInit(&(rul->Hal));
        }

#if defined(PTX_APP_DIVERSITY_WINDOWS) || defined(PTX_APP_DIVERSITY_LINUX)
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_OSAL_DeInit(&(rul->Osal));
        }
#endif

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_Log_DeInit(&rul->Log);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            memset(rul, 0, sizeof(ptx_RUL_t));
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_Reset(ptx_RUL_t* rul, ptx_RUL_ResetTypes_t resetType)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_Reset");
        /* Map the request to internal NSC reset types. */

        ptx_NSC_Reset_Types_t nsc_reset_type;

        switch (resetType)
        {
            case ptx_RUL_SystemResetType_Soft:
                nsc_reset_type = ptx_NSC_Reset_Soft;
                status = PTX_STATUS_SUCCESS;
                break;
            case ptx_RUL_SystemResetType_Hard:
                nsc_reset_type = ptx_NSC_Reset_Hard;
                status = PTX_STATUS_SUCCESS;
                break;
            default:
                /* Invalid request. Error is already set. */
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_Reset(&(rul->Nsc), nsc_reset_type);

            if ((PTX_STATUS_SUCCESS == status) && (ptx_RUL_StatusInfoSystem_NSCReset == rul->SystemStatus))
            {
                rul->SystemStatus = ptx_RUL_StatusInfoSystem_OK;
            }
        }
    }

    return status;

}

ptx_Status_t ptx_RUL_Deactivate(ptx_RUL_t *rul, ptx_RUL_DeactivateParams_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != params))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_Deactivate");
        ptx_NSC_RfDeact_Param_t deactivation_parameters;
        memset(&deactivation_parameters, 0, sizeof(deactivation_parameters));

        /* Set to Success, setting wrong parameter status will be done in one of default switch branches
         * if one of the parameters is not correct
         */
        status = PTX_STATUS_SUCCESS;

        switch (params->Type)
        {
            case ptx_RUL_DeactivateType_Generic:
                rul->DeactivateStatus = ptx_RUL_StatusInfoDeactivate_Pending;
                deactivation_parameters.Type = NSC_RfDeact_Type_Generic;
                break;
            case ptx_RUL_DeactivateType_Specific:
                rul->EmvRemovalStatus = ptx_RUL_StatusInfoEmvRemoval_Pending;
                deactivation_parameters.Type = NSC_RfDeact_Type_ProtSpecific;
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        switch (params->State)
        {
            case ptx_RUL_DeactivateState_Idle:
                deactivation_parameters.State = NSC_RfDeact_State_Idle;
                break;
            case ptx_RUL_DeactivateState_Poll:
                deactivation_parameters.State = NSC_RfDeact_State_Poll;
                break;
            case ptx_RUL_DeactivateState_Discovery:
                deactivation_parameters.State = NSC_RfDeact_State_Discovery;
                break;
            case ptx_RUL_DeactivateState_DiscoveryRfReset:
                deactivation_parameters.State = NSC_RfDeact_State_DiscoveryWithRfReset;
                break;
            case ptx_RUL_DeactivateState_ListenSleep:
                deactivation_parameters.State = NSC_RfDeact_State_ListenSleep;
                break;
            case ptx_RUL_DeactivateState_ListenIdle:
                deactivation_parameters.State = NSC_RfDeact_State_ListenIdle;
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        if (PTX_STATUS_E_GEN_INVALID_PARAMETER != status)
        {
            uint8_t ntf_to_follow = 0;

            status = ptx_NSC_RfDeactivate(&(rul->Nsc), &deactivation_parameters, &ntf_to_follow);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* In case no notification is pending, the card was deactivated immediately. */
                if (0 == ntf_to_follow)
                {
                    if (ptx_RUL_DeactivateState_Poll != params->State)
                    {
                        (void)ptx_RUL_ClearCardRegistry(rul);
                        rul->EmvRemovalStatus = ptx_RUL_StatusInfoEmvRemoval_Complete;
                    }
                    else
                    {
                        /* When deactivating to Poll state keep card registry only update state of activate card */
                        if ((NULL != rul->CardRegistry.ActiveCard) && (rul->CardRegistry.ActiveCard->State == ptx_RUL_CardRfState_Active))
                        {
                            rul->CardRegistry.ActiveCard->State = ptx_RUL_CardRfState_Sleep;
                        }
                    }
                    rul->DeactivateStatus = ptx_RUL_StatusInfoDeactivate_Complete;

                    rul->NfcMode = ptx_RUL_StatusInfoMode_Idle;
                }
            }
            else
            {
                rul->DeactivateStatus = ptx_RUL_StatusInfoDeactivate_Error;
                rul->EmvRemovalStatus = ptx_RUL_StatusInfoEmvRemoval_Error;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_GetStatusInfo(ptx_RUL_t *rul, ptx_RUL_Status_Params_t *statusParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != statusParams))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_GetStatusInfo");
        status = ptx_NSC_CmdProcessor_GetPendingMsg(&(rul->Nsc));

        switch (statusParams->Type)
        {
            case ptx_RUL_StatusType_System:
                statusParams->Info.System = rul->SystemStatus;

                /* StandbyError is not a critical system error. It is just the way NSC notifies the application
                 * about the transition from automatic Standby Mode. If this notification has been received,
                 * the system has exited DISCOVERY state and needs to initiate discovery all over again (if required).
                 * Since it is not a critical system error, report it back and reset the status for normal operation.
                 */
                if (ptx_RUL_StatusInfoSystem_ErrorStandby == rul->SystemStatus)
                {
                    rul->SystemStatus = ptx_RUL_StatusInfoSystem_OK;
                }
                break;

            case ptx_RUL_StatusType_Discover:
                statusParams->Info.Discover = rul->DiscoverStatus;

                /* Special case for EmvcoCollision error status which occurs in DISCOVERY state.
                 * The status and the error have to be reset here.
                 */
                if (ptx_RUL_StatusInfoDiscover_EmvCollision == rul->DiscoverStatus)
                {
                    rul->DiscoverStatus = ptx_RUL_StatusInfoDiscover_NoCard;
                    rul->RfErrorStatus = ptx_RUL_StatusInfoRfError_ErrorNone;
                }
                break;

            case ptx_RUL_StatusType_Lpcd:
                memcpy(&statusParams->Info.Lpcd, &rul->LpcdStatus, sizeof(ptx_RUL_StatusInfoLpcd_t));
                break;

            case ptx_RUL_StatusType_Deactivate:
                statusParams->Info.Deactivate = rul->DeactivateStatus;
                break;

            case ptx_RUL_StatusType_EmvRemoval:
                statusParams->Info.EmvRemoval = rul->EmvRemovalStatus;
                break;

            case ptx_RUL_StatusType_LastRfError:
                statusParams->Info.RfError = rul->RfErrorStatus;
                rul->RfErrorStatus = ptx_RUL_StatusInfoRfError_ErrorNone;
                break;

            case ptx_RUL_StatusType_NfcMode:
                statusParams->Info.NfcMode = rul->NfcMode;
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_BitsExchangeMode(ptx_RUL_t *rul, ptx_RUL_BitsExchangeMode_t enable)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (ptx_RUL_BitsExchangeMode_Max > enable))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_BitsExchangeMode");
        status = ptx_NSC_CltMsg_Enable(&(rul->Nsc), (ptx_NSC_CltMode_t)enable);
    }

    return status;
}

ptx_Status_t ptx_RUL_BitsExchange(ptx_RUL_t *rul, ptx_RUL_BitsExchangeParams_t *txParams,
                                  ptx_RUL_BitsExchangeParams_t *rxParams, uint32_t *numBits, uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != txParams) && (NULL != rxParams) && (NULL != numBits))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_BitsExchange");
        status = ptx_NSC_CltMsg(&(rul->Nsc), txParams->Data, txParams->Parity, txParams->DataLen);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t rx_buf[PTX_NSC_LENGTH_MAX];
            memset(rx_buf, 0, sizeof(uint8_t)*PTX_NSC_LENGTH_MAX);

            uint32_t rx_buf_len = PTX_NSC_LENGTH_MAX;

            status = ptx_NSC_DataRx(&(rul->Nsc), rx_buf, &rx_buf_len, msTimeout);

            /* Separate data from parity. */
            if ((0 != rx_buf_len) && ((PTX_STATUS_SUCCESS == status) || (PTX_STATUS_E_NSC_RESIDUAL_BITS == status)))
            {
                status = ptx_RUL_DecodeRawBits(rul, rx_buf, rx_buf_len, rul->Nsc.ResidualRxBits, &rxParams->Data[0], &rxParams->Parity[0], &rxParams->DataLen, numBits);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_Exchange(ptx_RUL_t *rul, uint8_t *tx, uint32_t txLen, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_Exchange");
        uint8_t continue_with_rx = 1U;
        if ((NULL != tx) && (0 < txLen))
        {
            status = ptx_NSC_DataTx (&(rul->Nsc), tx, txLen, 0U, PTX_NSC_DATA_TIMEOUT_DEFAULT);
            if (PTX_STATUS_SUCCESS != status)
            {
                continue_with_rx = 0;
            }
        }

        if (0 != continue_with_rx)
        {
            if ((NULL != rx) && (NULL != rxLen) && (0 < *rxLen))
            {
                status = ptx_NSC_DataRx (&(rul->Nsc), rx, rxLen, msTimeout);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_ConfigGPIO(ptx_RUL_t *rul, ptx_RUL_GPIONbr_t gpioNbr, ptx_RUL_GPIODir_t gpioDir, ptx_RUL_GPIOFlags_t gpioFlags)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_ConfigGPIO");

        uint16_t gpio_address;
        uint8_t gpio_cfg_index;

        status = ptx_RUL_GetGpioAddress(gpioNbr, &gpio_address, &gpio_cfg_index);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_CheckGpioAvailability(rul, gpioNbr);

            if (PTX_STATUS_SUCCESS == status)
            {
                uint8_t gpio_extended_features;
                status = ptx_RUL_GetGpioFeatures(gpioNbr, &gpio_extended_features);

                if (PTX_STATUS_SUCCESS == status)
                {
                    switch (gpioDir)
                    {
                        case ptx_RUL_GPIODir_Input:

                            /* Only Flags for the input direction are valid. */
                            if ((ptx_RUL_GPIOFlags_Weakest_Driver_Strength & (uint8_t)gpioFlags) || (ptx_RUL_GPIOFlags_Weak_Driver_Strength & (uint8_t)gpioFlags) || (ptx_RUL_GPIOFlags_Strong_Driver_Strength & (uint8_t)gpioFlags) || (ptx_RUL_GPIOFlags_Strongest_Driver_Strength & (uint8_t)gpioFlags))
                            {
                                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                            }
                            else
                            {
                                /* Configure PAD as Input GPIO */
                                rul->GpioCfg[gpio_cfg_index] = PAD_DIGX_IOX_REG_GPIO_MASK | PAD_DIGX_IOX_REG_OEN_MASK;

                                if (ptx_RUL_GPIOFlags_Internal_Pull_Resistor & (uint8_t)gpioFlags)
                                {
                                    /* Pullup/Pulldown resistor active. Only PAD5 has a pulldown resistor. */
                                    rul->GpioCfg[gpio_cfg_index] &= PAD_DIGX_IOX_REG_REN_MASK_INV;
                                }
                                else
                                {
                                    /* High Z mode active. */
                                    rul->GpioCfg[gpio_cfg_index] |= PAD_DIGX_IOX_REG_REN_MASK;
                                }

                                if (PTX_RUL_GPIO_EXTENDED_FEATURES == gpio_extended_features)
                                {
                                    /* Only PAD4, PAD6, PAD7 and PAD8 supports the extended features (Input Glitch Filter) */
                                    if (ptx_RUL_GPIOFlags_InputGlitchFilter & (uint8_t)gpioFlags)
                                    {
                                        rul->GpioCfg[gpio_cfg_index] |= PAD_DIGX_IOX_REG_FS_MASK;
                                    }
                                    else
                                    {
                                        rul->GpioCfg[gpio_cfg_index] &= PAD_DIGX_IOX_REG_FS_MASK_INV;
                                    }
                                }

                            }


                            break;

                        case ptx_RUL_GPIODir_Output:

                            /* Only Flags for the output direction are valid. */
                            if ((ptx_RUL_GPIOFlags_Internal_Pull_Resistor & (uint8_t)gpioFlags) || (ptx_RUL_GPIOFlags_InputGlitchFilter & (uint8_t)gpioFlags))
                            {
                                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                            }
                            else
                            {
                                /* Configure PAD as Output GPIO with Pull resistor deactivated to reduce current. */
                                rul->GpioCfg[gpio_cfg_index] = PAD_DIGX_IOX_REG_GPIO_MASK | PAD_DIGX_IOX_REG_REN_MASK;

                                if (PTX_RUL_GPIO_EXTENDED_FEATURES == gpio_extended_features)
                                {
                                    /* Only PAD4, PAD6, PAD7 and PAD8 support the extended features (Drive Strength) */
                                    if (ptx_RUL_GPIOFlags_Strongest_Driver_Strength & (uint8_t)gpioFlags)
                                    {
                                        rul->GpioCfg[gpio_cfg_index] |= PAD_DIGX_IOX_REG_DS_STST_MASK;
                                    }
                                    else if (ptx_RUL_GPIOFlags_Strong_Driver_Strength & (uint8_t)gpioFlags)
                                    {
                                        rul->GpioCfg[gpio_cfg_index] |= PAD_DIGX_IOX_REG_DS_ST_MASK;
                                    }
                                    else if (ptx_RUL_GPIOFlags_Weak_Driver_Strength & (uint8_t)gpioFlags)
                                    {
                                        rul->GpioCfg[gpio_cfg_index] |= PAD_DIGX_IOX_REG_DS_WK_MASK;
                                    }
                                    else
                                    {
                                        /* Defaults to Weakest Strength if flag not specified */
                                        rul->GpioCfg[gpio_cfg_index] |= PAD_DIGX_IOX_REG_DS_WKST_MASK;
                                    }


                                }
                            }

                            break;

                        default:
                            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                            break;
                    }
                }
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_Write(&(rul->Nsc), gpio_address, 1u, &rul->GpioCfg[gpio_cfg_index]);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_WriteGPIO(ptx_RUL_t *rul, ptx_RUL_GPIONbr_t gpioNbr, ptx_RUL_GPIOValue_t value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_WriteGPIO");
        uint16_t gpio_address;
        uint8_t gpio_cfg_index;

        status = ptx_RUL_GetGpioAddress(gpioNbr, &gpio_address, &gpio_cfg_index);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_CheckGpioAvailability(rul, gpioNbr);

            if (PTX_STATUS_SUCCESS == status)
            {
                if (    (0u == (rul->GpioCfg[gpio_cfg_index] & PAD_DIGX_IOX_REG_OEN_MASK))                              /* If configured as Output */
                        && (PAD_DIGX_IOX_REG_GPIO_MASK == (rul->GpioCfg[gpio_cfg_index] & PAD_DIGX_IOX_REG_GPIO_MASK)) )   /* If configured as GPIO */
                {
                    if (ptx_RUL_GPIOValue_High == value)
                    {
                        rul->GpioCfg[gpio_cfg_index] |= PAD_DIGX_IOX_REG_I_MASK;
                    }
                    else if (ptx_RUL_GPIOValue_Low == value)
                    {
                        rul->GpioCfg[gpio_cfg_index] &= PAD_DIGX_IOX_REG_I_MASK_INV;
                    }
                    else
                    {
                        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    }
                }
                else
                {
                    status = PTX_STATUS_E_GEN_NOT_PERMITTED;
                }
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_Write(&(rul->Nsc), gpio_address, 1u, &rul->GpioCfg[gpio_cfg_index]);
        }

    }

    return status;
}

ptx_Status_t ptx_RUL_ReadGPIO(ptx_RUL_t *rul, ptx_RUL_GPIONbr_t gpioNbr, ptx_RUL_GPIOValue_t *value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != value) )
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_ReadGPIO");
        uint16_t gpio_address;
        uint8_t gpio_cfg_index;

        status = ptx_RUL_GetGpioAddress(gpioNbr, &gpio_address, &gpio_cfg_index);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_CheckGpioAvailability(rul, gpioNbr);

            if (PTX_STATUS_SUCCESS == status)
            {
                if (PAD_DIGX_IOX_REG_GPIO_MASK == (rul->GpioCfg[gpio_cfg_index] & PAD_DIGX_IOX_REG_GPIO_MASK))          /* If configured as GPIO */
                {
                    uint8_t gpio_value = 0u;

                    status = ptx_NSC_Read(&(rul->Nsc), gpio_address, 1u, &gpio_value);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        *value = (uint8_t)((gpio_value & PAD_DIGX_IOX_REG_C_MASK) >> PAD_DIGX_IOX_REG_C_POS);
                    }
                }
                else
                {
                    status = PTX_STATUS_E_GEN_NOT_PERMITTED;
                }
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_Sleep(ptx_RUL_t* rul, uint32_t ms)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_Sleep");
        if (0 != ms)
        {
            status = ptx_HAL_Sleep(&(rul->Hal), ms);
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_Send(ptx_RUL_t* rul, uint8_t* tx, uint32_t txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != tx) && (0 < txLen))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_Send");
        status = ptx_NSC_DataTx (&(rul->Nsc), tx, txLen, 0U, PTX_NSC_DATA_TIMEOUT_DEFAULT);
    }

    return status;
}

ptx_Status_t ptx_RUL_Receive(ptx_RUL_t* rul, uint8_t* rx, uint32_t* rxLen, uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != rx) && (NULL != rxLen) && (0 < *rxLen))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_Receive");
        status = ptx_NSC_DataRx (&(rul->Nsc), rx, rxLen, msTimeout);
    }

    return status;
}

ptx_Status_t ptx_RUL_GetInfo(ptx_RUL_t* rul, ptx_RUL_InfoTypes_t infoType, uint8_t* infoBuf, uint8_t* infoBufLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != infoBuf) && (NULL != infoBufLen))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_GetInfo");
        uint8_t nscInfoType = ptx_NSC_Info_MAX;
        const uint8_t standardInfoSize = 1u;
        const uint8_t swRevisionInfoSize = 3u;
        const uint8_t fwRevisionInfoSize = 2u;
        const uint8_t platformRevisionSize = 8u;

        uint8_t expectedDataSize = 0;

        status = PTX_STATUS_SUCCESS;

        switch (infoType)
        {
            case ptx_RUL_InfoType_Stack_Revision:
                nscInfoType = ptx_NSC_Info_C_Stack;
                expectedDataSize = swRevisionInfoSize;
                break;
            case ptx_RUL_InfoType_HW_Version:
                nscInfoType = ptx_NSC_Info_HW_Version;
                expectedDataSize = standardInfoSize;
                break;
            case ptx_RUL_InfoType_FW_Revision:
                nscInfoType = ptx_NSC_Info_FW_Version;
                expectedDataSize = fwRevisionInfoSize;
                break;
            case ptx_RUL_InfoType_FW_Tool_Revision:
                nscInfoType = ptx_NSC_Info_DFY_Toolchain;
                expectedDataSize = fwRevisionInfoSize;
                break;
            case ptx_RUL_InfoType_RF_Config_Version:
                nscInfoType = ptx_NSC_Info_Rf_Config;
                expectedDataSize = fwRevisionInfoSize;
                break;
            case ptx_RUL_InfoType_Chip_Derivate:
                nscInfoType = ptx_NSC_Info_Derivate;
                expectedDataSize = fwRevisionInfoSize;
                break;
            case ptx_RUL_InfoType_Chip_Temperature:
                /* Data is retrieved via NSC GetStatus, therefore no nscInfoType is required */
                expectedDataSize = standardInfoSize;
                break;
            case ptx_RUL_InfoType_Mtu_Size:
                nscInfoType = ptx_NSC_Info_MtuSize;
                expectedDataSize = standardInfoSize;
                break;
            case ptx_RUL_InfoType_Local_Changes:
                nscInfoType = ptx_NSC_Info_Local_Changes;
                expectedDataSize = standardInfoSize;
                break;
            case ptx_RUL_InfoType_Platform_Revision:
                nscInfoType = ptx_NSC_Info_Platform_Revision;
                expectedDataSize = platformRevisionSize;
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Before the data is retrieved, check if enough space has been allocated for the expected information. */
            if (expectedDataSize > *infoBufLen)
            {
                /* Input buffer is too small for the requested data. */
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
            else if (ptx_RUL_InfoType_Chip_Temperature == infoType)
            {
                /* Handle special case for chip temperature info type. */
                ptx_NSC_ParamStatus_t param_status = {0};
                status = ptx_NSC_GetStatus(&(rul->Nsc), &param_status);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* 1By length */
                    *infoBuf = param_status.Temperature;
                    *infoBufLen = 1;
                }
            }
            else
            {
                /* Provided buffer is big enough to hold the requested data. */
                status = ptx_NSC_GetInfo(&(rul->Nsc), nscInfoType, infoBuf, infoBufLen);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_T3T_SensFReq(ptx_RUL_t* rul, uint16_t systemCode, uint8_t requestCode, uint8_t tsn, uint8_t* rx, uint32_t* rxLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t trx_buffer[PTX_RUL_T3T_SENSFREQ_TXR_BUFFER_SIZE];
    uint32_t rx_len = PTX_RUL_T3T_SENSFREQ_TXR_BUFFER_SIZE;
    uint32_t timeout = 0;
    uint8_t tsn_int = 0;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (rx != NULL) && (rxLength != NULL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_T3T_SensFReq");
        ptx_NSC_RfSetParameter_Param_t rf_Par[PTX_RUL_T3T_MAX_PARAMS];

        /* This function is only available if activated card / device is using the T3T-protocol */
        if (ptx_RUL_CardProtocolType_T3T == rul->CardRegistry.ActiveProtocol)
        {
            tsn_int = (uint8_t)(tsn & 0x0F); /* Mask away potentially too high tsn. */
            timeout = ptx_RUL_SensFReqFWT[tsn_int];
            rf_Par[0].Type = NSC_RfSetParameter_Param_Type_Fwt;
            rf_Par[0].Value.Fwt.FwtValue[2] = (uint8_t)((timeout >> 16) & (uint8_t)0xFF);
            rf_Par[0].Value.Fwt.FwtValue[1] = (uint8_t)((timeout >> 8)  & (uint8_t)0xFF);
            rf_Par[0].Value.Fwt.FwtValue[0] = (uint8_t)((timeout >> 0)  & (uint8_t)0xFF);
            rf_Par[1].Type = NSC_RfSetParameter_Param_Type_MaxRspLim;
            rf_Par[1].Value.MaxRspLim.MaxRspLimValue = 0;

            /*
             * Assemble T3T SENSF_REQ-command
             */
            trx_buffer[0] = PTX_RUL_T3T_SENSFREQ_COMMAND_LEN;
            trx_buffer[1] = PTX_RUL_T3T_SENSFREQ_COMMAND_CODE;
            trx_buffer[2] = (uint8_t)((systemCode >> 8) & (uint8_t)0xFF);
            trx_buffer[3] = (uint8_t)((systemCode >> 0) & (uint8_t)0xFF);
            trx_buffer[4] = requestCode;
            trx_buffer[5] = tsn;


            status = ptx_NSC_RfSetParameter(&(rul->Nsc), &rf_Par[0], PTX_RUL_T3T_MAX_PARAMS);

            if (PTX_STATUS_SUCCESS == status)
            {
                /* FeliCa-/T3T-response data is built up as follows
                 * Offset 0: Complete ATQC/SENSF_RES - 18 or 20 bytes, depending if RD requested) consisting of
                 *           - LEN-byte.....: 12h or 14h
                 *           - RESPONSE-code: 01h
                 *           - IDm..........: 8 bytes manufacturer ID
                 *           - PMm..........: 8 bytes manufacturer Parameters
                 *
                 *
                 * ... FeliCa-/T3T-response data of next card (if available)
                 *
                 */
                status = ptx_RUL_Exchange(rul, &trx_buffer[0], (uint32_t)PTX_RUL_T3T_SENSFREQ_COMMAND_LEN, &trx_buffer[0], &rx_len, PTX_RUL_T3T_SENSFREQ_APPTIMEOUT_MS);

                /*
                 * Read operation at APDU level correct? Let's forward up length if yes.
                 * If not, exchange status implicitly given via rxLength.
                 */
                if (PTX_STATUS_SUCCESS == status)
                {
                    *rxLength = rx_len;
                    memcpy(&rx[0], &trx_buffer[0], sizeof(uint8_t) * rx_len);
                }
                else
                {
                    *rxLength = 0;
                }
            }

            /* Reset Rf Parameters. */
            rf_Par[0].Type = NSC_RfSetParameter_Param_Type_Fwt;
            rf_Par[0].Value.Fwt.FwtValue[0] = 0;
            rf_Par[0].Value.Fwt.FwtValue[1] = 0;
            rf_Par[0].Value.Fwt.FwtValue[2] = 0;

            rf_Par[1].Type = NSC_RfSetParameter_Param_Type_MaxRspLim;
            rf_Par[1].Value.MaxRspLim.MaxRspLimValue = 1;

            status = ptx_NSC_RfSetParameter(&(rul->Nsc), &rf_Par[0], PTX_RUL_T3T_MAX_PARAMS);
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_T5T_Isolated_EoF (ptx_RUL_t* rul, uint8_t* rx, uint32_t* rxLength, uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (rx != NULL) && (rxLength != NULL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_T5T_Isolated_EoF");
        status = ptx_NSC_CtrlTx(&(rul->Nsc), PTX_NSC_DATA_MSG_EOF);

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_DataRx (&(rul->Nsc), rx, rxLength, msTimeout);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_UpdateHardwareConfiguration(ptx_RUL_t* rul, uint32_t nrConfigs, ptx_RUL_HWConfig_t* configs)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t update_rf_cfg = 0;
    uint8_t update_sys_cfg = 0;
    const uint8_t max_allowed_params = (PTX_NSC_RFCONFIG_PARAMID_MAX_PARAMID + PTX_NSC_SYSTEM_CONFIG_PARAM_ID_MAX_PARAMID);

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != configs) && (0 != nrConfigs) && (max_allowed_params >= nrConfigs))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_UpdateHardwareConfiguration");
        ptx_NSC_RfConfig_Param_t nsc_rf_params[PTX_NSC_RFCONFIG_PARAMID_MAX_PARAMID];
        uint8_t rfconfig_tlv_count = 0;

        ptx_NSC_SystemConfig_Param_t nsc_sys_params[PTX_NSC_SYSTEM_CONFIG_PARAM_ID_MAX_PARAMID];
        uint8_t sysconfig_tlv_count = 0;

        memset(&nsc_rf_params[0], 0, sizeof(ptx_NSC_RfConfig_Param_t) * PTX_NSC_RFCONFIG_PARAMID_MAX_PARAMID);
        memset(&nsc_sys_params[0], 0, sizeof(ptx_NSC_SystemConfig_Param_t) * PTX_NSC_SYSTEM_CONFIG_PARAM_ID_MAX_PARAMID);

        for (uint32_t i = 0; i < nrConfigs; i++)
        {
            ptx_RUL_HWConfig_t current_config = configs[i];

            if ((ptx_RUL_RF_CFG_MAX > current_config.ID) && (rfconfig_tlv_count < PTX_NSC_RFCONFIG_PARAMID_MAX_PARAMID))
            {
                status = ptx_RUL_MapRfConfig(&current_config, nsc_rf_params, &rfconfig_tlv_count);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* Updates the internal RF-config data structures, stored in the RAM of the MCU. */
                    status = ptx_NSC_RfConfig_SetSection(&(rul->Nsc), &nsc_rf_params[rfconfig_tlv_count - 1u]);
                }

                update_rf_cfg = 1u;
            }
            else if ((ptx_RUL_SYS_CFG_ClkSel <= current_config.ID) && (ptx_RUL_SYS_CFG_MAX > current_config.ID) && (sysconfig_tlv_count < PTX_NSC_SYSTEM_CONFIG_PARAM_ID_MAX_PARAMID))
            {
                /* Check lengths */
                status = ptx_RUL_MapSysConfig(&current_config, nsc_sys_params, &sysconfig_tlv_count);
                update_sys_cfg = 1u;
            }
            else
            {
                /* Not supported param ID */
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }

            if (PTX_STATUS_SUCCESS != status)
            {
                /* Parameter could not be loaded. Abort. */
                break;
            }
        }

        /* Update System Configuration if configured */
        if ((PTX_STATUS_SUCCESS == status) && (0 != update_sys_cfg))
        {
            status = ptx_NSC_System_UpdateConfiguration(&(rul->Nsc), nsc_sys_params, sysconfig_tlv_count);
        }

        /* Update RF Configuration if configured */
        if ((PTX_STATUS_SUCCESS == status) && (0 != update_rf_cfg))
        {
            status = ptx_NSC_RfConfig(&(rul->Nsc), nsc_rf_params, rfconfig_tlv_count);
        }

        if (ptx_RUL_StatusInfoSystem_NSCReset == rul->SystemStatus)
        {
            rul->SystemStatus = ptx_RUL_StatusInfoSystem_OK;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_SetPowerMode(ptx_RUL_t *rul, ptx_RUL_PowerMode_t powerMode)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_SetPowerMode");
        switch (powerMode)
        {
            case PowerMode_Standby:
                status = ptx_NSC_Standby(&(rul->Nsc));
                break;

            case PowerMode_Wakeup:
                status = ptx_NSC_Wakeup(&(rul->Nsc));
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_WriteDAC(ptx_RUL_t *rul, uint8_t value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_WriteDAC");
        if (ANA_TUNE_REG_TUNE_MAX_VALUE >= value)
        {
            value = value & ANA_TUNE_REG_TUNE_MASK;
            status = ptx_NSC_Write(&(rul->Nsc), ANA_TUNE_REG_ADDRESS, 1, &value);
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_GetRSSIValue(ptx_RUL_t *rul, uint8_t cwPowerLevel, ptx_RUL_AdcAveraging_t averaging, uint32_t *result)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != result) && (AdcAveraging_Max > averaging))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_GetRSSIValue");
        /* Set the result to zero. */
        *result = 0;

        /*
         *  Set both CW power levels to the one defined by the user.
         *  The call to ptx_NSC_RfConfig_SetParam() will update the NSC RF config data structures stored in the RAM.
         */
        status = ptx_NSC_RfConfig_SetParam(&(rul->Nsc), ParamId_PollPwrLvl0, cwPowerLevel);
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RfConfig_SetParam(&(rul->Nsc), ParamId_PollPwrLvl1, cwPowerLevel);
        }


        if (PTX_STATUS_SUCCESS == status)
        {
            /*
            * Write the updated RF config section to the device.
            */
            ptx_RUL_HWConfig_t rf_config;
            rf_config.ID        = ptx_RUL_RF_CFG_POLL;
            rf_config.Data.Ptr  = ptx_NSC_RfConfig_Poll;
            rf_config.Len       = ptx_NSC_RfConfig_Poll_len;
            status = ptx_RUL_UpdateHardwareConfiguration(rul, 1u, &rf_config);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_RUL_RfFieldMeasurement(rul, PTX_RUL_QMEAS_CENTER_FREQ, averaging, result);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Get out of RF test mode. */
            status = ptx_NSC_RfTestStop(&(rul->Nsc));
        }
    }

    return status;
}

ptx_Status_t ptx_RUL_CalibrateTxEsdPreventionThreshold(ptx_RUL_t *rul, uint8_t *results, uint8_t resultsLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != results))
    {
        /* Configure the Low-Power ADC for the RSSI measurement. */
        uint8_t regVal = ANA_RX_CONTROL1_REG_RST;
        status = ptx_NSC_Write(&rul->Nsc, ANA_RX_CONTROL1_REG, 1, &regVal);

        if (PTX_STATUS_SUCCESS == status)
        {
            regVal = ANA_RX_BUF_REG_RST;
            status = ptx_NSC_Write(&rul->Nsc, ANA_RX_BUF_REG, 1,  &regVal);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            regVal = (ANA_RX_LPADC_CONTROL_REG_RX_LPADCQ_DIG_EN_MASK | 
                      ANA_RX_LPADC_CONTROL_REG_RX_LPADCI_DIG_EN_MASK |
                      ANA_RX_LPADC_CONTROL_REG_RX_LPADCQ_COMP_EN_MASK | 
                      ANA_RX_LPADC_CONTROL_REG_RX_LPADCI_COMP_EN_MASK);
            status = ptx_NSC_Write(&rul->Nsc, ANA_RX_LPADC_CONTROL_REG, 1,  &regVal);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            regVal = (SYS_CLOCK_CONTROL1_REG_CLOCK_TSENS_RATE_EN_MASK | 
                      SYS_CLOCK_CONTROL1_REG_CLOCK_LPADC_EN_MASK);
            status = ptx_NSC_Write(&rul->Nsc, SYS_CLOCK_CONTROL1_REG, 1,  &regVal);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            regVal = ANA_RX_LPADC_PP_CTRL_REG_LPADC_PP_EN_MASK;
            status = ptx_NSC_Write(&rul->Nsc, ANA_RX_LPADC_PP_CTRL_REG, 1,  &regVal);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            regVal = ANA_RX_LPADC_PP_CTRL2_REG_RST;
            status = ptx_NSC_Write(&rul->Nsc, ANA_RX_LPADC_PP_CTRL2_REG, 1,  &regVal);
        }

        
        for(uint8_t power_level = 0; ((power_level < resultsLength) && (PTX_STATUS_SUCCESS == status)); ++power_level)
        {
            /*
             * Set both CW power levels defined by the calibration.
             */
            status = ptx_NSC_RfConfig_SetParam(&(rul->Nsc), ParamId_PollPwrLvl0, power_level);
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_RfConfig_SetParam(&(rul->Nsc), ParamId_PollPwrLvl1, power_level);
            }


            if (PTX_STATUS_SUCCESS == status)
            {
                /*
                * Write the updated RF config section to the device.
                */
                ptx_RUL_HWConfig_t rf_config;
                rf_config.ID        = ptx_RUL_RF_CFG_POLL;
                rf_config.Data.Ptr  = ptx_NSC_RfConfig_Poll;
                rf_config.Len       = ptx_NSC_RfConfig_Poll_len;
                status = ptx_RUL_UpdateHardwareConfiguration(rul, 1u, &rf_config);
            }

            /* 
             * Perform a RF Test RF Amplitude measurement to activate all components in the RF Frontend required for the 
             * RSSI measurement with the Low-Power ADC. 
             * 
             * The result of this measurement can be ignored!
             */
            ptx_RUL_Clk_Params_t clk_params;
            ptx_RUL_Clk_Config_t clk_config;
            ptx_NSC_RfTest_Output_t rf_test_result;
            if (PTX_STATUS_SUCCESS == status)
            {
                ptx_NSC_SystemConfig_t *system_cfg;
                (void) memset(&clk_params, 0, sizeof(ptx_RUL_Clk_Params_t));

                (void) memset(&clk_config, 0, sizeof(ptx_RUL_Clk_Config_t));

                /* Prepare the measurement results data structure. */
                (void) memset(&rf_test_result, 0, sizeof(ptx_NSC_RfTest_Output_t));

                system_cfg = &(rul->Nsc.SysCfg);
                clk_config.RefFreq = system_cfg->RefFreq;
                clk_config.TargetFreq = PTX_RUL_QMEAS_CENTER_FREQ;

                /* Calculate PLL parameters based on given reference frequency & target frequency. */
                status = ptx_RUL_CalculatePllParameters(rul, &clk_config, &clk_params);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Copy all the retrieved settings into the data structure for RF amplitude measurement. */
                ptx_NSC_RfTest_InParams_t rf_test_params;
                rf_test_params.SeqId                            = NSC_RfTest_SeqId_MeasRFAmp;
                rf_test_params.Params.RfAmpParams.XtalDiv2En    = clk_params.XtalDiv2En;
                rf_test_params.Params.RfAmpParams.GatePw        = clk_params.GatePw;
                rf_test_params.Params.RfAmpParams.SysCntTg      = clk_params.SysCntTg;
                rf_test_params.Params.RfAmpParams.NHost         = clk_params.NHost;
                rf_test_params.Params.RfAmpParams.AvgFactorInt  = 1;

                /* Run the NSC_RF_TEST_CMD. */
                status = ptx_NSC_RfTest(&(rul->Nsc), &rf_test_params, &rf_test_result);
            }

            /*
             * Read the result of the Low-Power ADC RSSI measurement for the calibration. 
             */
            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NSC_Read(&rul->Nsc, ANA_RX_LPADC_PP_OUT_REG, 1, &results[power_level]);
            }

            if ((PTX_STATUS_SUCCESS == status) && (PTX_RUL_TX_ESD_PREVENTION_THRESHOLD_MAX < results[power_level]))
            {
                status = PTX_STATUS_E_GEN_OUT_OF_RANGE;
            }


            if (PTX_STATUS_SUCCESS == status)
            {
                /* Get out of RF test mode. */
                status = ptx_NSC_RfTestStop(&(rul->Nsc));
            }
        }
    }

    return status;
}


ptx_Status_t ptx_RUL_RfFieldMeasurement(ptx_RUL_t *rul, uint32_t targetFrequency, ptx_RUL_AdcAveraging_t averaging, uint32_t *rfAmp)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != rfAmp) && (averaging < AdcAveraging_Max))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_RfFieldMeasurement");
        ptx_RUL_Clk_Params_t clk_params;
        (void) memset(&clk_params, 0, sizeof(ptx_RUL_Clk_Params_t));

        ptx_RUL_Clk_Config_t clk_config;
        (void) memset(&clk_config, 0, sizeof(ptx_RUL_Clk_Config_t));

        /* Prepare the measurement results data structure. */
        ptx_NSC_RfTest_Output_t rf_test_result;
        (void) memset(&rf_test_result, 0, sizeof(ptx_NSC_RfTest_Output_t));

        ptx_NSC_SystemConfig_t *system_cfg = &(rul->Nsc.SysCfg);
        clk_config.RefFreq = system_cfg->RefFreq;
        clk_config.TargetFreq = targetFrequency;

        /* Calculate PLL parameters based on given reference frequency & target frequency. */
        status = ptx_RUL_CalculatePllParameters(rul, &clk_config, &clk_params);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Copy all the retrieved settings into the data structure for RF amplitude measurement. */
            ptx_NSC_RfTest_InParams_t rf_test_params;
            rf_test_params.SeqId                            = NSC_RfTest_SeqId_MeasRFAmp;
            rf_test_params.Params.RfAmpParams.XtalDiv2En    = clk_params.XtalDiv2En;
            rf_test_params.Params.RfAmpParams.GatePw        = clk_params.GatePw;
            rf_test_params.Params.RfAmpParams.SysCntTg      = clk_params.SysCntTg;
            rf_test_params.Params.RfAmpParams.NHost         = clk_params.NHost;
            rf_test_params.Params.RfAmpParams.AvgFactorInt  = averaging;

            /* Run the NSC_RF_TEST_CMD. */
            status = ptx_NSC_RfTest(&(rul->Nsc), &rf_test_params, &rf_test_result);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint32_t measuredAmp = rf_test_result.RfAmpOut.RfAmplitude;

            /*
             *  In case we are above PTX_RUL_ADC_MAXAMPLSB the ADC got into the non-linear range!
             *  Hence, the measurement is invalid.
             */
            if (measuredAmp >= (PTX_RUL_ADC_MAXAMPLSB * PTX_RUL_AMP_MEAS_FACTOR))
            {
                *rfAmp = 0;
                status = PTX_STATUS_E_GEN_ADCCLIPPING;
            }
            else
            {
                *rfAmp = measuredAmp;
            }
        }
    }

    return status;
}

#if !defined(PTX_DISABLE_LISTEN_MODE)
ptx_Status_t ptx_RUL_RoutingTableConfig(ptx_RUL_t *rul, uint32_t length, ptx_RUL_RoutingList_t *rt_list)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_I(&rul->Log, "ptx_RUL_RoutingTableConfig");
        ptx_NSC_t nsc = rul->Nsc;
        uint32_t idx = 0;

        do
        {
            switch (rt_list[idx].Type)
            {
                case ptx_RUL_Technology_Based:
                    status = ptx_NSC_RfSetRoutingTable_AddEntryTech(&nsc, (ptx_NSC_RfTech_t)rt_list[idx].Entry.TechRouting.Target, (ptx_NSC_RoutingTarget_t)rt_list[idx].Entry.TechRouting.Target, 0, 0);
                    break;

                case ptx_RUL_Protocol_Based:
                    status = ptx_NSC_RfSetRoutingTable_AddEntryProtocol(&nsc, (ptx_NSC_RfProtocol_t)rt_list[idx].Entry.ProtocolRouting.Protocol, (ptx_NSC_RoutingTarget_t)rt_list[idx].Entry.ProtocolRouting.Target, 0, 0);
                    break;

                case ptx_RUL_Aid_Based:
                    if (PTX_NSC_RT_AID_MAX_LEN >= rt_list[idx].Entry.AidRouting.Length)
                    {
                        status = ptx_NSC_RfSetRoutingTable_AddEntryAid(&nsc, rt_list[idx].Entry.AidRouting.Aid, rt_list[idx].Entry.AidRouting.Length, (ptx_NSC_RoutingTarget_t)rt_list[idx].Entry.AidRouting.Target, (uint8_t)rt_list[idx].Entry.AidRouting.MatchType, 0, 0);
                    }
                    break;

                case ptx_RUL_RoutingType_Apdu_Based:
                /* FALL THROUGH */
                case ptx_RUL_RoutingType_System_Code_Based:
                /* FALL THROUGH */
                case ptx_RUL_RoutingType_NumOfTypes:
                    /* FALL THROUGH */
                    status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;
                    break;

                default:
                    break;
            }
            idx++;
        }
        while ((PTX_STATUS_SUCCESS == status) && (idx < length));

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RfSetRoutingTable(&(rul->Nsc));
        }
    }

    return status;
}
#endif

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
static ptx_Status_t ptx_RUL_AllocInit(ptx_RUL_t *rul, ptx_RUL_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != rul) && (NULL != initParams))
    {
        /*
         * Log component initialisation
         */

        status = ptx_Log_Init(&rul->Log, NULL);

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_V(&rul->Log, "ptx_RUL_AllocInit");

            ptx_HAL_InitParams_t hal_init;
            memset(&hal_init, 0, sizeof(hal_init));
            hal_init.InterfaceType = initParams->InterfaceInitParams.InterfaceType;
            hal_init.InterfacePortType = initParams->InterfaceInitParams.InterfacePortType;
            hal_init.InterfacePortID = initParams->InterfaceInitParams.InterfacePortID;
            hal_init.NumberOfTimers = initParams->InterfaceInitParams.NumberOfTimers;
            hal_init.Log = &rul->Log;

#if defined(PTX_APP_DIVERSITY_WINDOWS) || defined(PTX_APP_DIVERSITY_LINUX)
            /* The following 2 parameters are used only for UART interface on OS-based platforms. */
            hal_init.InterfaceName = initParams->InterfaceInitParams.InterfaceName;
            hal_init.InterfaceNameLen = initParams->InterfaceInitParams.InterfaceNameLen;

            /* Initialize OS context */
            status = ptx_OSAL_Init(&(rul->Osal), rul);
            if (PTX_STATUS_SUCCESS == status)
            {
                hal_init.Osal = &(rul->Osal);
            }
#endif

            if (PTX_STATUS_SUCCESS == status)
            {
                /* If initialisation was successful, return initialised component. Otherwise, keep invalid (NULL). */
                status = ptx_HAL_Init(&(rul->Hal), &hal_init);
            }

        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_HIP_InitParams_t hip_init;
            memset(&hip_init, 0, sizeof(hip_init));


            hip_init.Hal = &(rul->Hal);
            hip_init.Log = &rul->Log;
            status = ptx_HIP_Open(&(rul->Hip), &hip_init);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_NSC_InitParams_t nsc_init;
            memset(&nsc_init, 0, sizeof(ptx_NSC_InitParams_t));

            nsc_init.Hal = &(rul->Hal);
            nsc_init.Hip = &(rul->Hip);
            nsc_init.Log = &rul->Log;

            status = ptx_NSC_Init(&rul->Nsc, &nsc_init);

        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_MapRfConfig(ptx_RUL_HWConfig_t *hwConfig, ptx_NSC_RfConfig_Param_t *nscRfParams, uint8_t *rfParamsIdx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != hwConfig) && (NULL != nscRfParams) && (NULL != rfParamsIdx))
    {
        uint8_t idx = *rfParamsIdx;

        switch (hwConfig->ID)
        {
            case ptx_RUL_RF_CFG_POLL:
                if (ptx_NSC_RfConfig_Poll_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_Poll;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_POLL_A_106:
                if (ptx_NSC_RfConfig_PollA106_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_PollA106;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_POLL_B_106:
                if (ptx_NSC_RfConfig_PollB106_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_PollB106;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_POLL_F_212:
                if (ptx_NSC_RfConfig_PollF212_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_PollF212;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_POLL_F_424:
                if (ptx_NSC_RfConfig_PollF424_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_PollF424;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_POLL_V_26K5:
                if (ptx_NSC_RfConfig_PollV26_5_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_PollV_26K5;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_POLL_A_212:
                if (ptx_NSC_RfConfig_PollA212_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_PollA212;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_POLL_A_424:
                if (ptx_NSC_RfConfig_PollA424_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_PollA424;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_POLL_A_848:
                if (ptx_NSC_RfConfig_PollA848_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_PollA848;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_POLL_B_212:
                if (ptx_NSC_RfConfig_PollB212_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_PollB212;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_POLL_B_424:
                if (ptx_NSC_RfConfig_PollB424_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_PollB424;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_POLL_B_848:
                if (ptx_NSC_RfConfig_PollB848_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_PollB848;
                    status = PTX_STATUS_SUCCESS;
                }
                break;

            case ptx_RUL_RF_CFG_LISTEN:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                if (ptx_NSC_RfConfig_Listen_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_Listen;
                    status = PTX_STATUS_SUCCESS;
                }
                break;
#endif /* PTX_DISABLE_LISTEN_MODE */

            case ptx_RUL_RF_CFG_LISTEN_A_212:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                if (ptx_NSC_RfConfig_ListenA212_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_ListenA212;
                    status = PTX_STATUS_SUCCESS;
                }
                break;
#endif /* PTX_DISABLE_LISTEN_MODE */

            case ptx_RUL_RF_CFG_LISTEN_A_424:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                if (ptx_NSC_RfConfig_ListenA424_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_ListenA424;
                    status = PTX_STATUS_SUCCESS;
                }
                break;
#endif /* PTX_DISABLE_LISTEN_MODE */

            case ptx_RUL_RF_CFG_LISTEN_A_848:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                if (ptx_NSC_RfConfig_ListenA848_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_ListenA848;
                    status = PTX_STATUS_SUCCESS;
                }
                break;
#endif /* PTX_DISABLE_LISTEN_MODE */

            case ptx_RUL_RF_CFG_LISTEN_B_212:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                if (ptx_NSC_RfConfig_ListenB212_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_ListenB212;
                    status = PTX_STATUS_SUCCESS;
                }
                break;
#endif /* PTX_DISABLE_LISTEN_MODE */

            case ptx_RUL_RF_CFG_LISTEN_B_424:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                if (ptx_NSC_RfConfig_ListenB424_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_ListenB424;
                    status = PTX_STATUS_SUCCESS;
                }
                break;
#endif /* PTX_DISABLE_LISTEN_MODE */

            case ptx_RUL_RF_CFG_LISTEN_B_848:
#if !defined(PTX_DISABLE_LISTEN_MODE)
                if (ptx_NSC_RfConfig_ListenB848_len == hwConfig->Len)
                {
                    nscRfParams[idx].Id = NSC_RfConfig_ParamId_ListenB848;
                    status = PTX_STATUS_SUCCESS;
                }
                break;
#endif /* PTX_DISABLE_LISTEN_MODE */

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Parameter ID mapped. Check whether value and length are valid. */
            if ((NULL != hwConfig->Data.Ptr) && (0 != hwConfig->Len))
            {
                nscRfParams[idx].Value = hwConfig->Data.Ptr;
                nscRfParams[idx].Length = hwConfig->Len;
                *rfParamsIdx = (uint8_t)(*rfParamsIdx + 1);
            }
            else
            {
                /* Invalid length or value provided. */
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_MapSysConfig(ptx_RUL_HWConfig_t *hwConfig, ptx_NSC_SystemConfig_Param_t *nscSysParams, uint8_t *sysParamsIdx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != hwConfig) && (NULL != nscSysParams) && (NULL != sysParamsIdx))
    {
        uint8_t idx = *sysParamsIdx;

        switch (hwConfig->ID)
        {
            case ptx_RUL_SYS_CFG_ClkSel:
                if (PTX_NSC_SYSTEM_CONFIG_CLK_SEL_LEN == hwConfig->Len)
                {
                    nscSysParams[idx].Config = NSC_SystemConfig_Config_ClkSel;
                    nscSysParams[idx].Value.ClkSel = (uint8_t) hwConfig->Data.Value;
                    status = PTX_STATUS_SUCCESS;
                }
                break;
            case ptx_RUL_SYS_CFG_RefFreq:
                if (PTX_NSC_SYSTEM_CONFIG_REFFREQ_LEN == hwConfig->Len)
                {
                    nscSysParams[idx].Config = NSC_SystemConfig_Config_RefFreq;
                    nscSysParams[idx].Value.RefFreq = hwConfig->Data.Value;
                    status = PTX_STATUS_SUCCESS;
                }
                break;
            case ptx_RUL_SYS_CFG_TempErrTresh:
                if (PTX_NSC_SYSTEM_CONFIG_TEMP_ERR_TRESH_LEN == hwConfig->Len)
                {
                    nscSysParams[idx].Config = NSC_SystemConfig_Config_TempErrTresh;
                    nscSysParams[idx].Value.TempErrTresh = (uint8_t) hwConfig->Data.Value;
                    status = PTX_STATUS_SUCCESS;
                }
                break;
            case ptx_RUL_SYS_CFG_TxCurlimGain:
                if (PTX_NSC_SYSTEM_CONFIG_TX_CURLIMGAIN_LEN == hwConfig->Len)
                {
                    nscSysParams[idx].Config = NSC_SystemConfig_Config_TxCurlimGain;
                    nscSysParams[idx].Value.TxCurlimGain = (uint8_t) hwConfig->Data.Value;
                    status = PTX_STATUS_SUCCESS;
                }
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            *sysParamsIdx = (uint8_t)(*sysParamsIdx + 1);
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_DecodeRawBits(ptx_RUL_t *rul, uint8_t *pldRawBits, uint32_t lengthRawBits, uint8_t residualBits,
                                          uint8_t *pldBytes, uint8_t *pldParityBits, uint32_t *lengthBytes, uint32_t *numTotalBits)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != pldRawBits) && (NULL != pldBytes) && (lengthRawBits > 0) && (NULL != pldParityBits) && (NULL != lengthBytes) && (NULL != numTotalBits))
    {
        PTX_LOG_V(&rul->Log, "ptx_RUL_DecodeRawBits");
        /* Ensure that enough capacity for writing the bitstream */
        uint32_t num_pld_bytes = 0;
        uint32_t num_pld_bits = 0;

        /* Calculate the number of bits in the bitstream */
        if (0 == residualBits)
        {
            num_pld_bits = lengthRawBits * 8u;

            if (0 == (num_pld_bits % 9u))
            {
                num_pld_bytes = lengthRawBits / 9u;
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
        else
        {
            /* (length-1) full bytes + residuals */
            num_pld_bits = ((lengthRawBits - 1u) * 8u) + residualBits;

            if (0 == (num_pld_bits % 9u))
            {
                num_pld_bytes = num_pld_bits / 9u;
            }
            else
            {
                num_pld_bytes = (num_pld_bits / 9u) + 1u;
            }

            status = PTX_STATUS_SUCCESS;
        }

        /*
         * There are two options for processing bitstream, either smaller than 9 bits or
         * if larger than 9 bits this shall be multiple of 9
         */
        if (num_pld_bits >= 9u)
        {
            if (0 == (num_pld_bits % 9u))
            {
                /* If larger than 9 bits, It shall be multiple of 9u. */
            }
            else
            {
                status = PTX_STATUS_E_GEN_INTERNAL;
            }
        }


        if (PTX_STATUS_SUCCESS == status)
        {
            uint32_t length_bytes_input = *lengthBytes;

            if (length_bytes_input >= num_pld_bytes)
            {
                uint32_t index_pld = 0;
                uint32_t index_bits_bitstream = 0;
                uint32_t index_bytes_bitstream = 0;
                uint32_t index_bits_shift = 0;

                /* Enough capacity of output buffers, Let's write them. */
                (void)memset(pldBytes, 0, length_bytes_input);
                (void)memset(pldParityBits, 0, length_bytes_input);

                if (num_pld_bits < 9u)
                {
                    /* Less than 9 bits, */
                    pldBytes[0] = pldRawBits[0];
                    pldParityBits[0] = 0x00;
                    *lengthBytes = 1u;
                    *numTotalBits = num_pld_bits;

                }
                else
                {
                    do
                    {
                        index_bits_shift = 0;

                        /* Decode payload byte */
                        if (0 == (index_bits_bitstream % 8u))
                        {
                            pldBytes[index_pld] = pldRawBits[index_bytes_bitstream];
                        }
                        else
                        {
                            /* Some bits of the bitstream to current byte, some bits to the next one. */
                            index_bits_shift = index_bits_bitstream % 8u;
                            uint16_t two_bytes_temp = (uint16_t)(((((uint16_t)pldRawBits[index_bytes_bitstream + 1u]) << 8u) & 0xFF00) + (uint16_t)pldRawBits[index_bytes_bitstream]);
                            pldBytes[index_pld] = (uint8_t)(two_bytes_temp >> index_bits_shift);
                        }

                        index_bits_bitstream += 8u;
                        index_bytes_bitstream ++;

                        /* Decode parity bit */
                        index_bits_shift = index_bits_bitstream % 8u;
                        pldParityBits[index_pld] = (0x01 & (pldRawBits[index_bytes_bitstream] >> index_bits_shift));

                        index_bits_bitstream ++;
                        index_bytes_bitstream = (index_bits_bitstream >= ((index_bytes_bitstream + 1u) * 8u)) ? (index_bytes_bitstream + 1) : index_bytes_bitstream;

                        index_pld ++;

                    }
                    while (index_bits_bitstream < num_pld_bits);

                    /* Successful operation, let's update the number of bytes written, and the number of total bits */
                    *lengthBytes = index_pld;
                    *numTotalBits = index_pld * 9u;
                }
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_CheckGpioAvailability(ptx_RUL_t *rul, ptx_RUL_GPIONbr_t gpioNbr)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL))
    {
        PTX_LOG_V(&rul->Log, "ptx_RUL_CheckGpioAvailability");
        uint8_t gpio_value = 0u;

        /* Only allow GPIO configuration if the GPIO is not used for Digital Testbusses. */
        status = ptx_NSC_Read(&(rul->Nsc), PAD_TMUX_CONTROL0_REG, 1u, &gpio_value);

        if ((PTX_STATUS_SUCCESS == status) &&
            (((PAD_TMUX_CONTROL0_REG_DIG8_TB_EN_MASK == (PAD_TMUX_CONTROL0_REG_DIG8_TB_EN_MASK & gpio_value)) && (ptx_RUL_GPIONbr_8 == gpioNbr)) ||
             ((PAD_TMUX_CONTROL0_REG_DIG7_TB_EN_MASK == (PAD_TMUX_CONTROL0_REG_DIG7_TB_EN_MASK & gpio_value)) && (ptx_RUL_GPIONbr_7 == gpioNbr)) ||
             ((PAD_TMUX_CONTROL0_REG_DIG6_TB_EN_MASK == (PAD_TMUX_CONTROL0_REG_DIG6_TB_EN_MASK & gpio_value)) && (ptx_RUL_GPIONbr_6 == gpioNbr)) ||
             ((PAD_TMUX_CONTROL0_REG_DIG5_TB_EN_MASK == (PAD_TMUX_CONTROL0_REG_DIG5_TB_EN_MASK & gpio_value)) && (ptx_RUL_GPIONbr_5 == gpioNbr)) ||
             ((PAD_TMUX_CONTROL0_REG_DIG4_TB_EN_MASK == (PAD_TMUX_CONTROL0_REG_DIG4_TB_EN_MASK & gpio_value)) && (ptx_RUL_GPIONbr_4 == gpioNbr)) ||
             ((PAD_TMUX_CONTROL0_REG_DIG3_TB_EN_MASK == (PAD_TMUX_CONTROL0_REG_DIG3_TB_EN_MASK & gpio_value)) && (ptx_RUL_GPIONbr_3 == gpioNbr)) ||
             ((PAD_TMUX_CONTROL0_REG_DIG2_TB_EN_MASK == (PAD_TMUX_CONTROL0_REG_DIG2_TB_EN_MASK & gpio_value)) && (ptx_RUL_GPIONbr_2 == gpioNbr)) ||
             ((PAD_TMUX_CONTROL0_REG_DIG1_TB_EN_MASK == (PAD_TMUX_CONTROL0_REG_DIG1_TB_EN_MASK & gpio_value)) && (ptx_RUL_GPIONbr_1 == gpioNbr))))
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }

        /* Only allow GPIO configuration if the GPIO is not used for Analog Testbusses. */
        if ((PTX_STATUS_SUCCESS == status) && ((ptx_RUL_GPIONbr_4 == gpioNbr) || (ptx_RUL_GPIONbr_6 == gpioNbr) || (ptx_RUL_GPIONbr_7 == gpioNbr) || (ptx_RUL_GPIONbr_8 == gpioNbr)))
        {
            status = ptx_NSC_Read(&(rul->Nsc), ANA_TEST_BUF_REG, 1u, &gpio_value);

            if ((PTX_STATUS_SUCCESS == status) &&
                 (((ANA_TEST_BUF_REG_ANATEST_BUF_EN_MASK_PAD6 == (ANA_TEST_BUF_REG_ANATEST_BUF_EN_MASK_PAD6 & gpio_value)) && (ptx_RUL_GPIONbr_6 == gpioNbr)) ||
                  ((ANA_TEST_BUF_REG_ANATEST_BUF_EN_MASK_PAD4 == (ANA_TEST_BUF_REG_ANATEST_BUF_EN_MASK_PAD4 & gpio_value)) && (ptx_RUL_GPIONbr_4 == gpioNbr)) ||
                  ((ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD8 == (ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD8 & gpio_value)) && (ptx_RUL_GPIONbr_8 == gpioNbr)) ||
                  ((ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD7 == (ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD7 & gpio_value)) && (ptx_RUL_GPIONbr_7 == gpioNbr)) ||
                  ((ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD6 == (ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD6 & gpio_value)) && (ptx_RUL_GPIONbr_6 == gpioNbr)) ||
                  ((ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD4 == (ANA_TEST_BUF_REG_ANATEST_BUF_BYPASS_MASK_PAD4 & gpio_value)) && (ptx_RUL_GPIONbr_4 == gpioNbr))))
            {
                status = PTX_STATUS_E_GEN_NOT_PERMITTED;
            }
        }



        /* Only allow GPIO configuration if the Auxillary Interface is not active. Check only for GPIO4, GPIO5, GPIO6, GPIO7 and GPIO8. */
        if ((PTX_STATUS_SUCCESS == status) &&
            ((ptx_RUL_GPIONbr_4 == gpioNbr) || (ptx_RUL_GPIONbr_5 == gpioNbr) || (ptx_RUL_GPIONbr_6 == gpioNbr) || (ptx_RUL_GPIONbr_7 == gpioNbr) || (ptx_RUL_GPIONbr_8 == gpioNbr)))
        {
            status = ptx_NSC_Read(&(rul->Nsc), AUX_IF_CONFIG0_REG, 1u, &gpio_value);

            if ((PTX_STATUS_SUCCESS == status) && (AUX_IF_CONFIG0_REG_AUX_IF_ENABLE_MASK == (AUX_IF_CONFIG0_REG_AUX_IF_ENABLE_MASK & gpio_value)))
            {
                status = PTX_STATUS_E_GEN_NOT_PERMITTED;
            }
        }

        /* Only allow GPIO configuration if GPIO1 is not configured as wake-up source. */
        if ((PTX_STATUS_SUCCESS == status) && (ptx_RUL_GPIONbr_1 == gpioNbr))
        {
            status = ptx_NSC_Read(&(rul->Nsc), SYS_WU_CONFIG1_REG, 1u, &gpio_value);

            if ((PTX_STATUS_SUCCESS == status) && (SYS_WU_CONFIG1_REG_WU_GPIO_EN_MASK == (SYS_WU_CONFIG1_REG_WU_GPIO_EN_MASK & gpio_value)))
            {
                status = PTX_STATUS_E_GEN_NOT_PERMITTED;
            }
        }

        /* Only allow GPIO configuration if GPIO2 is not configured as reference clock input. */
        if ((PTX_STATUS_SUCCESS == status) && (ptx_RUL_GPIONbr_2 == gpioNbr))
        {
            status = ptx_NSC_Read(&(rul->Nsc), SYS_CLOCK_CONTROL1_REG, 1u, &gpio_value);

            if ((PTX_STATUS_SUCCESS == status) && (SYS_CLOCK_CONTROL1_REG_CLOCK_REF_GPIO_EN_MASK == (SYS_CLOCK_CONTROL1_REG_CLOCK_REF_GPIO_EN_MASK & gpio_value)))
            {
                status = PTX_STATUS_E_GEN_NOT_PERMITTED;
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_GetGpioAddress(ptx_RUL_GPIONbr_t gpioNbr, uint16_t *gpioHWAddr, uint8_t *gpioCfgOffset)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != gpioHWAddr) && (NULL != gpioCfgOffset))
    {
        switch (gpioNbr)
        {
            case ptx_RUL_GPIONbr_1:
                *gpioHWAddr = PAD_DIG1_IO1_REG;
                *gpioCfgOffset = PTX_RUL_GPIO_NR_01_CFG_INDEX;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_2:
                *gpioHWAddr = PAD_DIG2_IO1_REG;
                *gpioCfgOffset = PTX_RUL_GPIO_NR_02_CFG_INDEX;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_3:
                *gpioHWAddr = PAD_DIG3_IO1_REG;
                *gpioCfgOffset = PTX_RUL_GPIO_NR_03_CFG_INDEX;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_4:
                *gpioHWAddr = PAD_DIG4_IO2_REG;
                *gpioCfgOffset = PTX_RUL_GPIO_NR_04_CFG_INDEX;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_5:
                *gpioHWAddr = PAD_DIG5_IO2_REG;
                *gpioCfgOffset = PTX_RUL_GPIO_NR_05_CFG_INDEX;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_6:
                *gpioHWAddr = PAD_DIG6_IO2_REG;
                *gpioCfgOffset = PTX_RUL_GPIO_NR_06_CFG_INDEX;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_7:
                *gpioHWAddr = PAD_DIG7_IO2_REG;
                *gpioCfgOffset = PTX_RUL_GPIO_NR_07_CFG_INDEX;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_8:
                *gpioHWAddr = PAD_DIG8_IO2_REG;
                *gpioCfgOffset = PTX_RUL_GPIO_NR_08_CFG_INDEX;
                status = PTX_STATUS_SUCCESS;
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_GetGpioFeatures(ptx_RUL_GPIONbr_t gpioNbr, uint8_t *gpioFeatures)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != gpioFeatures)
    {
        switch (gpioNbr)
        {
            case ptx_RUL_GPIONbr_1:
                *gpioFeatures = PTX_RUL_GPIO01_FEATURES;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_2:
                *gpioFeatures = PTX_RUL_GPIO02_FEATURES;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_3:
                *gpioFeatures = PTX_RUL_GPIO05_FEATURES;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_4:
                *gpioFeatures = PTX_RUL_GPIO04_FEATURES;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_5:
                *gpioFeatures = PTX_RUL_GPIO05_FEATURES;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_6:
                *gpioFeatures = PTX_RUL_GPIO06_FEATURES;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_7:
                *gpioFeatures = PTX_RUL_GPIO07_FEATURES;
                status = PTX_STATUS_SUCCESS;
                break;

            case ptx_RUL_GPIONbr_8:
                *gpioFeatures = PTX_RUL_GPIO08_FEATURES;
                status = PTX_STATUS_SUCCESS;
                break;

            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

static ptx_Status_t ptx_RUL_CalculatePllParameters(ptx_RUL_t *rul, ptx_RUL_Clk_Config_t *clkConfig, ptx_RUL_Clk_Params_t *clkParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (PTX_COMP_CHECK(rul, Comp_RUL)) && (NULL != clkConfig) && (NULL != clkParams))
    {
        PTX_LOG_V(&rul->Log, "ptx_RUL_CalculatePllParameters");

        ptx_NSC_SystemConfig_t system_cfg = {0};
        ptx_NSC_PllConfig_t pll_config = {0};

        memcpy(&system_cfg, &(rul->Nsc.SysCfg), sizeof(ptx_NSC_SystemConfig_t));
        system_cfg.RefFreq = clkConfig->RefFreq;
        system_cfg.TargetFreq = clkConfig->TargetFreq;

        status = ptx_NSC_CalculatePllParameters(&(rul->Nsc), &system_cfg, &pll_config);
        if (PTX_STATUS_SUCCESS == status)
        {
            clkParams->GatePw = pll_config.GatePw;
            clkParams->NHost = pll_config.NHost;
            clkParams->SysCntTg = pll_config.SysCntTg;
            clkParams->XtalDiv2En = pll_config.XtalDiv2En;
        }
    }

    return status;
}
