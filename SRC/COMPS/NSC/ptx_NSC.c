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
    Module      : NFC Soft Controller (NSC) Basic Interface
    File        : ptx_NSC.c

    Description : Implementation of NFC Soft Controller Basic Interface.
                  Contains function(s) to handle NSC initialization, de-initialization and reset.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_HIP.h"
#include "ptx_HAL.h"
#include "ptx_NSC_Base.h"
#include "ptx_NSC_Interface.h"
#include <string.h>
#if !defined(PTX_DISABLE_LISTEN_MODE)
    #include "ptx_NSC_RfSetRoutingTable.h"
#endif

/*
 * ####################################################################################################################
 * LOCAL FUNCTIONS
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NSC_RESET_PIN           PTX_HAL_DRV_GPIO_RESET_PIN  /* Platform pin used to toggle NSC SEN pin. */
#define PTX_NSC_RESET_PIN_ACTIVE    0                           /* NSC reset pin active state. */
#define PTX_NSC_RESET_PIN_IDLE      1U                          /* NSC reset pin inactive state. */
#define PTX_NSC_RESET_RETRIES       2U                          /* Amount of retries for sending the HIP_RST_CMD. */

#define PTX_NSC_RESETNTF_HWVER_IDX  1U                          /* HW version parameter position in NSC_RESET_NTF. */
#define PTX_NSC_RESETNTF_FWVER_IDX  2U                          /* FW version parameter position in NSC_RESET_NTF. */
#define PTX_NSC_RESETNTF_DERIV_IDX  4U                          /* Commercial derivate parameter position in NSC_RESET_NTF. */

#define PTX_NSC_HIGH_BYTE_MASK      0xFF00
#define PTX_NSC_LOW_BYTE_MASK       0x00FF


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NSC_ToggleResetPin(ptx_NSC_t *nsc);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_Init (ptx_NSC_t *nscCtx, ptx_NSC_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_NSC_t *nsc = nscCtx;
    /* Expect valid initialisation parameters. Otherwise, no initialisation can take place. */

    if ( (NULL != nsc) && (NULL != initParams) && (NULL != initParams->Hal) && (NULL != initParams->Hip) && (NULL != initParams->Log))
    {
        PTX_LOG_I(initParams->Log, "ptx_NSC_Init");

        memset(nsc, 0, sizeof(ptx_NSC_t));

        nsc->ResidualRxBits = PTX_NSC_DEFAULT_NUMOF_RESIDUAL_BITS;
        nsc->ResidualTxBits = PTX_NSC_DEFAULT_NUMOF_RESIDUAL_BITS;
        nsc->MtU = PTX_NSC_MAX_TRANSFER_UNIT_SIZE;
        nsc->HAL = initParams->Hal;
        nsc->HIP = initParams->Hip;

        nsc->State     = ptx_NSC_State_Off;
        nsc->CompId    = Comp_NSC;

        ptx_NSC_Interface_InitParams_t nsc_intf_init_params;
        (void)memset(&nsc_intf_init_params, 0, sizeof(ptx_NSC_Interface_InitParams_t));
        nsc_intf_init_params.Param = initParams->ExtParams;
        nsc->Log = initParams->Log;

        status = ptx_NSC_Interface_Init(nsc, &nsc_intf_init_params);



#if !defined(PTX_DISABLE_LISTEN_MODE)
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_RfSetRoutingTable_Init(nsc);
        }
#endif

        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NSC_System_LoadDefaultParam(nsc);
            if (PTX_STATUS_SUCCESS != status)
            {
                /* Call ptx_NSC_Interface_DeInit to free up all the occupied resources */
                (void) ptx_NSC_Interface_Deinit(nsc);
            }
        }

        if (PTX_STATUS_SUCCESS != status)
        {
            memset(nsc, 0, sizeof(ptx_NSC_t));
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_DeInit (ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_DeInit");

        /* Interface specific DeInitialization. */
        status = ptx_NSC_Interface_Deinit(nsc);

        if (PTX_STATUS_SUCCESS == status)
        {
#if !defined(PTX_DISABLE_LISTEN_MODE)
            status = ptx_NSC_RfSetRoutingTable_Deinit(nsc);
#endif
            memset(nsc, 0, sizeof(ptx_NSC_t));
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_Reset(ptx_NSC_t *nsc, ptx_NSC_Reset_Types_t resetType)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_Reset");
        uint8_t retries = PTX_NSC_RESET_RETRIES;
        do
        {
            /*
             * Note: If the PTX2K is in standby state when ptx_NSC_Reset() is being called, the reset may fail.
             * However, the PTX2K will wake up due to the sent HIP_RST command, though it will not
             * execute the reset. Because of this behavior, the reset command must be sent again once the
             * chip has woken up to perform the actual system reset.
             */
            switch (resetType)
            {
                case ptx_NSC_Reset_Soft:
                    status = ptx_HIP_SysRst(nsc->HIP, HIP_Cmd_SysRst_RstEnDfy);
                    break;
                case ptx_NSC_Reset_Hard:
                    status = ptx_NSC_ToggleResetPin(nsc);
                    break;
                default:
                    /* Nothing to do. Error already set: Invalid parameter */
                    break;
            }

    #if defined(PTX_ENABLE_UART)
            if ((PTX_STATUS_SUCCESS == status) && (HAL_Interface_Type_UART == nsc->HAL->Interface.Type))
            {
                /*
                 * Special handling is required for UART HAL Communication Interface types, since there is speed change
                 * in HIP after the chip has been reset. HIP UART speed changes to default.
                 * Adjust HAL UART device speed here so that HIP and HAL stay synchronised.
                 */
                ptx_HAL_DRV_Interface_Parameter_t hal_param;
                hal_param.type = HAL_DRV_Interface_Parameter_Type_Transmission_Speed;
                hal_param.value.TransmissionSpeed = PTX_HAL_DRV_INTERFACE_UART_DEFAULT_BAUDRATE;
                status = ptx_HAL_Interface_SetParameter(nsc->HAL, &hal_param);
            }
    #endif

            if (PTX_STATUS_SUCCESS == status)
            {
                /* Add a delay for the system to reset and to set IRQ line low. Otherwise, we might start reading
                    * a message due to IRQ being set high from the previous system run.
                    */
                status = ptx_HAL_Sleep(nsc->HAL, 1U);
#if defined(PTX_ENABLE_I3C)
                if (PTX_STATUS_SUCCESS == status)
                {
                    ptx_HAL_DRV_Interface_Parameter_t hal_param;
                    hal_param.type = HAL_DRV_Interface_Parameter_Type_I3C_DynamicAddressAssignment;
                    hal_param.value.I3C_DynamicAddressAssignment = PTX_HAL_DRV_INTERFACE_I3C_DAA;
                    status = ptx_HAL_Interface_SetParameter(nsc->HAL, &hal_param);
                }
#endif
                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NSC_GetResetNtf(nsc);
                }
            }
            retries--;
        } while((PTX_STATUS_SUCCESS != status) && (ptx_NSC_Reset_Soft == resetType) && (0 != retries));
    }

    return status;
}

ptx_Status_t ptx_NSC_GetResetNtf(ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_GetResetNtf");

        uint8_t rst_ntf[PTX_NSC_CMD_PROCESSOR_RESET_NTFLEN_MAX];
        uint32_t rst_ntf_len = PTX_NSC_CMD_PROCESSOR_RESET_NTFLEN_MAX;

        status = ptx_NSC_CmdProcessor_FetchMsg(nsc, rst_ntf, &rst_ntf_len, PTX_NSC_TIMEOUT_DEFAULT, PTX_NSC_CMD_PROCESSOR_RESET_NTF);
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Received message has been processed in the previous call to FetchMsg.
             * Here we only check whether the processed message was NSC_RESET_NTF.
             * If not, set protocol error.
             */
            if (PTX_NSC_CMD_PROCESSOR_RESET_NTF != rst_ntf[0])
            {
                status = PTX_STATUS_E_NSC_PROTOCOL;
            }
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Extract NTF data. */
            nsc->HwVersion = rst_ntf[PTX_NSC_RESETNTF_HWVER_IDX];
            nsc->FwVersion = (uint16_t)(
                                 (((uint16_t)rst_ntf[PTX_NSC_RESETNTF_FWVER_IDX] << 8) & PTX_NSC_HIGH_BYTE_MASK) |
                                 (((uint16_t)rst_ntf[PTX_NSC_RESETNTF_FWVER_IDX + 1]) & PTX_NSC_LOW_BYTE_MASK)   );
            nsc->Derivate =  (uint16_t)(
                                 (((uint16_t)rst_ntf[PTX_NSC_RESETNTF_DERIV_IDX] << 8) & PTX_NSC_HIGH_BYTE_MASK) |
                                 (((uint16_t)rst_ntf[PTX_NSC_RESETNTF_DERIV_IDX + 1]) & PTX_NSC_LOW_BYTE_MASK)  );
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_ToggleResetPin(ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_ToggleResetPin");

        /*  Assert reset pin
         *  Set Reset pin direction Output, low state
         */
#if !defined(PTX_APP_DIVERSITY_RA0E2)
        const uint32_t reset_pin_asserted_time = 10U;

        status = ptx_HAL_GPIO_WritePin (nsc->HAL, (uint16_t)PTX_NSC_RESET_PIN, PTX_NSC_RESET_PIN_ACTIVE);
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Keep Reset pin low for at least minimum expected time. */
            status = ptx_HAL_Sleep (nsc->HAL, reset_pin_asserted_time);
        }

        /* Always try to return Reset pin to its original state (inactive). */

        /* De-assert reset pin.
         * Set Reset pin to Output, high state.
         */
        ptx_Status_t st = ptx_HAL_GPIO_WritePin (nsc->HAL, (uint16_t)PTX_NSC_RESET_PIN, PTX_NSC_RESET_PIN_IDLE);

        /* Forward the first error that occurred. */
        status = (PTX_STATUS_SUCCESS == status) ? st : status;
#else
        status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;
#endif
    }

    return status;
}
