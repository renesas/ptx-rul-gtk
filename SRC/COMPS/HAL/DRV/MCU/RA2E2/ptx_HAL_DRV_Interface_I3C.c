/*
    ---------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause

    Copyright (c) 2024, Renesas Electronics Corporation and/or its affiliates


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
    Module      : MCU-RA2E2 I3C Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_I3C.c

    Description : Implementation of the I3C Communication Interface Driver for the MCU-RA2E2 Reference Platform
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_DRV_Interface_I3C.h"
#include "ptx_HAL_GPIO.h"
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_HAL_DRV_I3C_NUMBER_PORTS            (1u)
#define PTX_HAL_DRV_I3C_DEFAULT_CHANNEL         (0u)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

static ptx_HAL_DRV_Interface_I3C_PortConfig_t gDrvI3CPortConfigs[PTX_HAL_DRV_I3C_NUMBER_PORTS] =
{
    {
        .Type                       = HAL_DRV_Interface_Port_Type_Default,
        .ID                         = 0,
        .Instance                   = &g_i3c0,
        .Irq                        = BSP_IO_PORT_01_PIN_01,
        .Channel                    = PTX_HAL_DRV_I3C_DEFAULT_CHANNEL,
        .TransferState              = PTX_HAL_DRV_I3C_RESET_VALUE,
        .TransferWaitCnt            = 0u,
        .ExternalIrqInstance        = &g_external_irq0,
        .MasterDeviceCfg          =
            {
                /* This is the Static I3C / I2C Legacy address defined by the device manufacturer. */
                .static_address = PTX_HAL_DRV_INTERFACE_I3C_MASTER_DEVICE_STATIC_ADDRESS,
                /* The dynamic address will be automatically updated when the master configures this device using CCC ENTDAA. */
                .dynamic_address = PTX_HAL_DRV_INTERFACE_I3C_MASTER_DEVICE_DYNAMIC_ADDRESS
            },
        .I3cEventStatus             = PTX_HAL_DRV_I3C_RESET_VALUE,
        .I3cEventCount              = {PTX_HAL_DRV_I3C_RESET_VALUE}
    }
};

static ptx_Status_t ptx_HAL_DRV_Interface_I3C_Wait4TransferDone(ptx_HAL_DRV_Interface_I3C_t *ctx, uint32_t eventId, int32_t timeout);

void ptx_HAL_Drv_Default_IrqCallback(external_irq_callback_args_t *p_args);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_HAL_DRV_Interface_I3C_Init(ptx_HAL_DRV_Interface_I3C_t *ctx, ptx_HAL_DRV_Interface_I3C_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ctx) && (NULL != initParams) && (NULL != initParams->Log))
    {
        PTX_LOG_V(initParams->Log, "ptx_HAL_DRV_Interface_I3C_Init");

        memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_I3C_t));

        for (uint8_t config = 0; (PTX_HAL_DRV_I3C_NUMBER_PORTS > config) && (PTX_STATUS_SUCCESS != status); ++config)
        {
            if ((gDrvI3CPortConfigs[config].Type == initParams->PortType) && (gDrvI3CPortConfigs[config].ID == initParams->PortID))
            {
                ctx->Port = &gDrvI3CPortConfigs[config];
                ctx->Speed = initParams->Speed;
                ctx->Address = initParams->Address;
                ctx->Log = initParams->Log;
                ctx->Gpio = initParams->Gpio;
                ctx->CompId = Comp_HAL;
                status = PTX_STATUS_SUCCESS;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I3C_DeInit(ptx_HAL_DRV_Interface_I3C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_I3C_DeInit");

        memset(ctx, 0, sizeof(ptx_HAL_DRV_Interface_I3C_t));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I3C_Open(ptx_HAL_DRV_Interface_I3C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    fsp_err_t err = FSP_ERR_ASSERTION;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_I3C_Open");

        const i3c_instance_t *i3c = ctx->Port->Instance;

        /* Initializes the I3C module. */
        err = R_I3C_Open(i3c->p_ctrl, i3c->p_cfg);
        if (FSP_SUCCESS == err)
        {
            if (NULL != ctx->Port->ExternalIrqInstance)
            {
                const external_irq_instance_t *irq = ctx->Port->ExternalIrqInstance;
                err = R_ICU_ExternalIrqOpen(irq->p_ctrl, irq->p_cfg);
    
                if (FSP_SUCCESS == err)
                {
                    err = R_ICU_ExternalIrqEnable(irq->p_ctrl);
                }
    
                if (FSP_SUCCESS == err)
                {
                    err = R_ICU_ExternalIrqCallbackSet(ctx->Port->ExternalIrqInstance->p_ctrl, ptx_HAL_Drv_Default_IrqCallback, ctx, NULL);
                }
            }
        }

        if (FSP_SUCCESS == err)
        {
            /* Set the device configuration for this device. */
            err = R_I3C_DeviceCfgSet(i3c->p_ctrl, &(ctx->Port->MasterDeviceCfg));
        }

        if (FSP_SUCCESS == err)
        {
            /* Set the I3C devices information through device table entries */
            memset(&ctx->Port->DeviceTableCfg, PTX_HAL_DRV_I3C_RESET_VALUE, sizeof(i3c_device_table_cfg_t));
    
            ctx->Port->DeviceTableCfg.static_address = (uint8_t)0x4C;
            ctx->Port->DeviceTableCfg.dynamic_address = 0x4c;
            ctx->Port->DeviceTableCfg.device_protocol = I3C_DEVICE_PROTOCOL_I3C;
            ctx->Port->DeviceTableCfg.ibi_accept = false;
            ctx->Port->DeviceTableCfg.ibi_payload = false;
            ctx->Port->DeviceTableCfg.master_request_accept = false;
    
            err = R_I3C_MasterDeviceTableSet(i3c->p_ctrl, 0, &ctx->Port->DeviceTableCfg); /* I3C_DEVICE_INDEX_EXTENDED_DEVICE */
        }

        if (FSP_SUCCESS == err)
        {
            /* Enable I3C Mode. */
            err = R_I3C_Enable(i3c->p_ctrl);
        }

        if (FSP_SUCCESS == err)
        {
            /* Set the buffer for storing IBI data that is read from the slave. */
            err = R_I3C_IbiRead(i3c->p_ctrl, ctx->Port->IbiReadData, PTX_HAL_DRV_INTERFACE_I3C_MAX_IBI_PAYLOAD_SIZE);
        }

        if (FSP_SUCCESS == err)
        {
            /* Start assigning dynamic addresses to devices on the bus using the CCC ENTDAA command. */
            /* we have one slave device hence last argument is ONE */
            err = R_I3C_DynamicAddressAssignmentStart(i3c->p_ctrl, I3C_ADDRESS_ASSIGNMENT_MODE_ENTDAA, PTX_HAL_DRV_I3C_RESET_VALUE, PTX_HAL_DRV_I3C_RESET_VALUE);
        }

        if (FSP_SUCCESS == err)
        {
            /* waiting for the bus initialization */
            /* hold up the application until the DAA is completed */
            status = ptx_HAL_DRV_Interface_I3C_Wait4TransferDone(ctx, I3C_EVENT_ADDRESS_ASSIGNMENT_COMPLETE, PTX_HAL_DRV_INTERFACE_I3C_WAIT_TIME);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Bus configuration is completed successfully. */
            err = R_I3C_DeviceSelect(i3c->p_ctrl, 0, I3C_BITRATE_MODE_I3C_SDR0_STDBR);
        }
        else
        {
            err = FSP_ERR_ASSERTION;
        }
    }

    if (FSP_SUCCESS == err)
    {
        status = PTX_STATUS_SUCCESS;
    }
    else
    {
        status = PTX_STATUS_E_GEN_INTERFACE_OPEN;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I3C_Close(ptx_HAL_DRV_Interface_I3C_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_I3C_Close");

        const i3c_instance_t *i3c = ctx->Port->Instance;

        fsp_err_t r_status = R_I3C_Close(i3c->p_ctrl);

        if ((FSP_SUCCESS == r_status) || (FSP_ERR_NOT_OPEN == r_status))
        {
            if (NULL != ctx->Port->ExternalIrqInstance)
            {
                const external_irq_instance_t *irq = ctx->Port->ExternalIrqInstance;
                r_status = R_ICU_ExternalIrqClose(irq->p_ctrl);
            }
        }

        if ((FSP_SUCCESS == r_status) || (FSP_ERR_NOT_OPEN == r_status))
        {
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_CLOSE;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I3C_Tx(ptx_HAL_DRV_Interface_I3C_t *ctx, uint8_t txBuf[], uint32_t txLen, uint8_t restart)
{
    ptx_Status_t status = PTX_STATUS_SUCCESS;
    fsp_err_t err = FSP_SUCCESS;
    
    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != txBuf))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_I3C_Tx");

        const i3c_instance_t *i3c = ctx->Port->Instance;
        /* update buffer */
        for (uint32_t cnt = PTX_HAL_DRV_I3C_RESET_VALUE; cnt < txLen; cnt++)
        {
            ctx->Port->WriteData[cnt] = (uint8_t)txBuf[cnt];
        }

        /* perform write operation.*/
        err = R_I3C_Write(i3c->p_ctrl, ctx->Port->WriteData, txLen, restart);
        if (FSP_SUCCESS != err)
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            /* wait for write complete event.*/
            status = ptx_HAL_DRV_Interface_I3C_Wait4TransferDone(ctx, I3C_EVENT_WRITE_COMPLETE, PTX_HAL_DRV_INTERFACE_I3C_WAIT_TIME);
        }
#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I_MEM(ctx->Log, &txBuf[0], txLen, "TX: ");
        }
#endif
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I3C_Rx(ptx_HAL_DRV_Interface_I3C_t *ctx, uint8_t *rxBuf, uint32_t rxLen, uint8_t restart)
{
    ptx_Status_t status = PTX_STATUS_SUCCESS;
    fsp_err_t err = FSP_SUCCESS;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != rxBuf))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_I3C_Rx");

        const i3c_instance_t *i3c = ctx->Port->Instance;
        /* Start a read operation. */
        err = R_I3C_Read(i3c->p_ctrl, rxBuf, rxLen, restart);
        if (FSP_SUCCESS != err)
        {
            /* R_I3C_Read API FAILED. */ 
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            /* wait for read complete event.*/
            status = ptx_HAL_DRV_Interface_I3C_Wait4TransferDone(ctx, I3C_EVENT_READ_COMPLETE, PTX_HAL_DRV_INTERFACE_I3C_WAIT_TIME);
        }
#if !defined(PTX_ENABLE_TFCI)
        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_LOG_I_MEM(ctx->Log, &rxBuf[0], rxLen, "RX: ");
        }
#endif
    }
    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I3C_RxIrqLevel(ptx_HAL_DRV_Interface_I3C_t *ctx, uint8_t *level)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_I3C_RxIrqLevel");

        bsp_io_level_t value;
        memset(&value, 0, sizeof(value));
        
        fsp_err_t r_status = R_IOPORT_PinRead(ctx->Gpio->Drv.Port->Instance->p_ctrl, ctx->Port->Irq, &value);

        if (FSP_SUCCESS == r_status)
        {
            *level = value;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERFACE_IRQ_READ;
        }
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I3C_SetParameter(ptx_HAL_DRV_Interface_I3C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != parameter))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_I3C_SetParameter");
        
        if ((HAL_DRV_Interface_Parameter_Type_I3C_DynamicAddressAssignment == parameter->type) &&
            (PTX_HAL_DRV_INTERFACE_I3C_DAA == parameter->value.I3C_DynamicAddressAssignment)
        )
        (void)R_I3C_DynamicAddressAssignmentStart(ctx->Port->Instance->p_ctrl, I3C_ADDRESS_ASSIGNMENT_MODE_ENTDAA, PTX_HAL_DRV_I3C_RESET_VALUE, PTX_HAL_DRV_I3C_RESET_VALUE);
        status = ptx_HAL_DRV_Interface_I3C_Wait4TransferDone(ctx, I3C_EVENT_ADDRESS_ASSIGNMENT_COMPLETE, PTX_HAL_DRV_INTERFACE_I3C_WAIT_TIME);
    }
    else
    {
        status = PTX_STATUS_E_GEN_NOT_AVAILABLE;
    }

    return status;
}

ptx_Status_t ptx_HAL_DRV_Interface_I3C_GetParameter(ptx_HAL_DRV_Interface_I3C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (NULL != parameter))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_I3C_GetParameter");
        
        status = PTX_STATUS_E_GEN_NOT_IMPLEMENTED;
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_HAL_DRV_Interface_I3C_Wait4TransferDone(ptx_HAL_DRV_Interface_I3C_t *ctx, uint32_t eventId, int32_t timeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_HAL) && (PTX_HAL_DRV_I3C_EVENT_SIZE > eventId))
    {
        PTX_LOG_V(ctx->Log, "ptx_HAL_DRV_Interface_I3C_Wait4TransferDone");
        
        ctx->Port->TransferState = TRX_TX_INPROGRESS;
        ctx->Port->TransferWaitCnt = timeout;

        while ( (TRX_TX_DONE != ctx->Port->TransferState) && (TRX_RX_DONE != ctx->Port->TransferState) )
        {
            --ctx->Port->TransferWaitCnt;

            if (PTX_HAL_DRV_I3C_RESET_VALUE >= ctx->Port->TransferWaitCnt)
            {
                status = PTX_STATUS_E_GEN_INTERFACE_TIMEOUT;
                break;
            }
            else if (TRX_ERROR == ctx->Port->TransferState)
            {
                status = PTX_STATUS_E_GEN_TRANSFER_ABORTED;
                break;
            }
            else
            {
                /* Do nothing. Error already set. */
            }
        }

        if ( ((PTX_STATUS_E_GEN_INTERFACE_TIMEOUT != status) && (PTX_STATUS_E_GEN_TRANSFER_ABORTED != status)) &&
             ((TRX_TX_DONE == ctx->Port->TransferState) || (TRX_RX_DONE == ctx->Port->TransferState)) )
        {
            if (ctx->Port->I3cEventCount[eventId] > PTX_HAL_DRV_I3C_RESET_VALUE)
            {
                ctx->Port->I3cEventStatus = PTX_HAL_DRV_I3C_RESET_VALUE;
                ctx->Port->I3cEventCount[eventId] -= PTX_HAL_DRV_I3C_ONE;

                status = PTX_STATUS_SUCCESS;
            }
        }
    }

    return status;
}

void ptx_HAL_Drv_Default_IrqCallback(external_irq_callback_args_t *p_args)
{
    /*
     * Callback is left empty intentionally!
     * All actions regarding the interrupt and the pin level are handled by the device
     * implementation! The ISR callback is just used to wake up the MCU from sleep states
     */
    (void)p_args;
}

void ptx_HAL_DRV_Interface_I3C_TransferCallback(i3c_callback_args_t const *const p_args)
{
    /* 
     * The I3C stack from the v6.1.0 FSP does not provide an valid pointer to context 
     * The pointer to context found in I3C (the p_context from i3c_callback_args_t) is taken from it's static configuration and it points to NULL
     * .p_context = p_ctrl->p_cfg->p_context in callback_args, where p_cfg has .p_context = NULL in hal_data.c, and p_cfg is const, can't be changed at runtime
     * The I3C stack needs to be updated to provide the correct pointer to context
     * For now, this global variable workaround must be used in order to access the Event Status and Transfer State variables 
     */
    ptx_HAL_DRV_Interface_I3C_PortConfig_t *config = &gDrvI3CPortConfigs[0];

    /* Check if the event is a valid I3C event found in the i3c_event_t enum */
    if ((PTX_HAL_DRV_I3C_EVENT_SIZE) >  p_args->event)
    {
        /* update the event in the array and this will be used in ptx_HAL_DRV_Interface_I3C_Wait4TransferDone function.*/
        config->I3cEventStatus = p_args->event_status;
        /* Increase the counter for the specific event */
        config->I3cEventCount[p_args->event]++;

        switch (p_args->event)
        {
            case I3C_EVENT_ENTDAA_ADDRESS_PHASE:
            {
                config->TransferState = TRX_RX_DONE;
                break;
            }

            case I3C_EVENT_ADDRESS_ASSIGNMENT_COMPLETE:
            {
                config->TransferState = TRX_RX_DONE;
                break;
            }

            case I3C_EVENT_READ_COMPLETE:
            {
                config->TransferState = TRX_RX_DONE;
                break;
            }

            case I3C_EVENT_WRITE_COMPLETE:
            {
                config->TransferState = TRX_TX_DONE;
                break;
            }

            case I3C_EVENT_COMMAND_COMPLETE:
            {
                config->TransferState = TRX_TX_DONE;
                break;
            }

            case I3C_EVENT_IBI_READ_COMPLETE:
            {
                /* When an IBI is completed, the transfer_size, ibi_type, and ibi_address will be available in p_args. */
                switch (p_args->ibi_type)
                {
                    case I3C_IBI_TYPE_INTERRUPT:
                    {
                        config->TransferState = TRX_RX_DONE;
                        break;
                    }
                    case I3C_IBI_TYPE_HOT_JOIN:
                    {
                        config->TransferState = TRX_RX_DONE;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }    
}
