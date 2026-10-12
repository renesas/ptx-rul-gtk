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
    Module      : Common Module Interface for Serial Logging
    File        : ptx_Log_Serial.c

    Description : Logging-implementation for Renesas RA-platforms supporting sending data via UART.
                  Contains the functions used to Initialize, de-initialize the UART logging
                  mechanism and the callback to print the data.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Log_Serial.h"
#include "ptx_Log_Interface.h"
#include <stdio.h>
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
static ptx_Log_Serial_t gLogSerial;

#define PTX_LOG_SERIAL_UART_INSTANCE            (uart_instance_t*)&ptx_uart_dbg
#define PTX_LOG_SERIAL_DEFAULT_UART_BAUDRATE    (115200u)
#define PTX_LOG_SERIAL_MAX_TX_BUFF_SIZE         (256U)
#define PTX_LOG_SERIAL_MAX_UART_BAUDRATE_ERROR  (3000)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

void ptx_Log_Serial_Write(ptx_Log_Serial_t *logCtx, char *buffer, uint32_t bufferLen);
void ptx_Log_Serial_Callback(uart_callback_args_t *p_args);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_Log_Interface_Open (ptx_Log_t *log, ptx_Log_InitParams_t *params)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != log) && (NULL != params))
    {
        (void)memset(&gLogSerial, 0, sizeof(ptx_Log_Serial_t));

        params->LogCtx  = &gLogSerial;
        params->LogType = ptx_Log_Type_Default;

        gLogSerial.UartInstance = PTX_LOG_SERIAL_UART_INSTANCE;

        uart_instance_t *uart_instance = (uart_instance_t*)gLogSerial.UartInstance;

        fsp_err_t err;

        err = R_SCI_UART_Open(uart_instance->p_ctrl, uart_instance->p_cfg);

        if (FSP_ERR_ALREADY_OPEN == err)
        {
            /* Close and re-open the interface. */
            (void)R_SCI_UART_Close(uart_instance->p_ctrl);

            err = R_SCI_UART_Open(uart_instance->p_ctrl, uart_instance->p_cfg);
        }

        if (FSP_SUCCESS == err)
        {
            err = R_SCI_UART_CallbackSet(uart_instance->p_ctrl, ptx_Log_Serial_Callback, &gLogSerial, NULL);
        }

        if (FSP_SUCCESS == err)
        {
            baud_setting_t baud_setting;

            err = R_SCI_UART_BaudCalculate(PTX_LOG_SERIAL_DEFAULT_UART_BAUDRATE, false, PTX_LOG_SERIAL_MAX_UART_BAUDRATE_ERROR, &baud_setting);

            if (FSP_SUCCESS == err)
            {
                err = R_SCI_UART_BaudSet(uart_instance->p_ctrl, (void *) &baud_setting);

                status = (FSP_SUCCESS == err) ? PTX_STATUS_SUCCESS : PTX_STATUS_E_GEN_INTERFACE_OPEN;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_Log_Interface_Close (ptx_Log_t *log)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(log, Comp_LOG))
    {
        ptx_Log_Serial_t *ctx = (ptx_Log_Serial_t*)log->LogCtx;
        uart_instance_t *uart_instance = (uart_instance_t*)ctx->UartInstance;

        fsp_err_t err = R_SCI_UART_Close(uart_instance->p_ctrl);

        if (FSP_SUCCESS == err)
        {
            status = (FSP_SUCCESS == err) ? PTX_STATUS_SUCCESS : PTX_STATUS_E_GEN_INTERFACE_CLOSE;
        }
    }

    return status;
}

void ptx_Log_Interface_WriteCB (ptx_Log_t *log, const char lvl, char *msg, uint32_t msgLen)
{
    if (PTX_COMP_CHECK(log, Comp_LOG) && (NULL != msg) && (0 != msgLen))
    {
        ptx_Log_Serial_t *ctx = (ptx_Log_Serial_t*)log->LogCtx;

        char lvl_prefix[2] = {lvl, ' '};

        ptx_Log_Serial_Write(ctx, &lvl_prefix[0], 2u);
        ptx_Log_Serial_Write(ctx, &msg[0], msgLen);
    }
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
void ptx_Log_Serial_Write(ptx_Log_Serial_t *logCtx, char *buffer, uint32_t bufferLen)
{
    if (NULL != logCtx)
    {
        uart_instance_t *uart_instance = (uart_instance_t*)logCtx->UartInstance;

        logCtx->UartTxState = LOG_SERIAL_TxState_Ongoing;

        fsp_err_t err = R_SCI_UART_Write(uart_instance->p_ctrl, (uint8_t*)buffer, bufferLen);

        if (FSP_SUCCESS == err)
        {
            while (LOG_SERIAL_TxState_Ongoing == logCtx->UartTxState)
            {
                /* Wait for on-going transfer to be finished. */
            }
        }
    }
}

void ptx_Log_Serial_Callback(uart_callback_args_t * p_args)
{
    if (NULL != p_args)
    {
        ptx_Log_Serial_t *ctx = (ptx_Log_Serial_t*)p_args->p_context;

        switch (p_args->event)
        {
            case UART_EVENT_TX_COMPLETE:
                ctx->UartTxState = LOG_SERIAL_TxState_Done;
                break;

            default:
                /* Transfer error; possible, ignore here */
                break;
        }
    }
}
