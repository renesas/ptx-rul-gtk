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
    Module      : Utils
    File        : ptx_Utils_Print.c

    Description : Implementation of the print module. There are 2 different implementations specific for underlying platforms (system).
                  Those 2 versions are included/excluded by specific compile switches.

                  To enable the version for Renesas RA4M2 MCU platform, PTX_PLATFORM_MCU must be defined. This version relies on specific
                  project MCU configuration, i.e. it uses specific predefined UART port and FSP library to print out the messages.

                  To enable the version for Windows/Linux platform, PTX_PLATFORM_OS must be defined. This version prints out the messages
                  to the standard output console by using native printf API.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Utils_Print.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>


#if defined (PTX_UTILITY_PRINTING_SERIAL)
    #include "hal_data.h"
#endif


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_MAX_PRINTBUF_SIZE                     (256U)      /* Maximum message buffer length. Note: last byte shall be 0. */
#define PTX_ASCII_SPACE                           (0x20U)     /* ASCII table SPACE character. */
#define PTX_PRINT_LINE_LENGTH                     80U

#if defined (PTX_UTILITY_PRINTING_SERIAL)

#define PTX_UTILS_UART_DEFAULT_BAUDRATE           (256000u)   /* Baudrate */
#define PTX_UTILS_UART_DEFAULT_BAUDRATE_ERROR     (3000u)     /* Baudrate error, 3% Error, Calculation: x / 1000 */
/* "SCIU" in ASCII.  Used to determine if the control block is open. */
#define SCI_UART_OPEN                             (0x53434955U)

/*
 * Uart states.
 */
typedef enum ptx_Utils_UartState
{
    Uart_State_Idle,
    Uart_State_TxOngoing,
    Uart_State_TxDone,
    Uart_State_TxError
} ptx_Utils_UartState_t;

void ptx_Utils_Print_Callback(uart_callback_args_t *p_args);

static volatile ptx_Utils_UartState_t sUartState = Uart_State_Idle;

#endif

static void ptx_Utils_Write(char *msgBuffer, uint32_t msgLen);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

void ptx_Utils_Print(const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);

    char buffer[PTX_MAX_PRINTBUF_SIZE];
    memset(buffer, 0, PTX_MAX_PRINTBUF_SIZE);

    uint32_t num_written = (uint32_t)vsnprintf(buffer, PTX_MAX_PRINTBUF_SIZE, format, argptr);
    if (num_written >= PTX_MAX_PRINTBUF_SIZE)
    {
        /* The output was truncated. Set terminating null character since it has been overwritten. */
        buffer[PTX_MAX_PRINTBUF_SIZE - 1] = '\0';
        num_written = PTX_MAX_PRINTBUF_SIZE - 1;
    }

    /* The following call uses only a null-byte terminated string. */
    ptx_Utils_Write(buffer, num_written);

    va_end(argptr);
}

void ptx_Utils_Print_Buffer(uint8_t *buffer, uint16_t bufferLength, uint8_t addNewLine, uint8_t printASCII)
{
    uint8_t character_to_print;

    if ((NULL != buffer) && (0 != bufferLength))
    {
        for (uint16_t i = 0; (i < bufferLength) && (i < PTX_MAX_PRINTBUF_SIZE); i++)
        {
            if ((i > 0) && ((i % (PTX_PRINT_LINE_LENGTH - 5) == 0)))
            {
                ptx_Utils_Print("\n     ");
            }

            if (0 == printASCII)
            {
                ptx_Utils_Print("%02X", (uint8_t)buffer[i]);
            }
            else
            {
                character_to_print = (uint8_t)buffer[i];

                /* Avoid unintentional interpretation of ascii-commands */
                if (character_to_print < PTX_ASCII_SPACE)
                {
                    ptx_Utils_Print(".");
                }
                else
                {
                    ptx_Utils_Print("%c", character_to_print);
                }
            }
        }

        if (0 != addNewLine)
        {
            ptx_Utils_Print("\n");
        }
    }
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */

#if defined (PTX_UTILITY_PRINTING_SERIAL)

static void ptx_Utils_Write(char *msgBuffer, uint32_t msgLen)
{
    /* Send message to MCU UART. Direct access to FSP API. */

    /*
     * NOTE: it is expected here that ptx_uart_dbg uart module instance is valid. No special checking done here.
     *       The instance is typically defined in e2studio project configuration and made available through
     *       automatically generated hal_data module.
     *
     *       This example uses r_sci_uart module. If another UART module is used e.g. r_sci_b_uart, adjust the names if needed
     *       for the call to calculate baudrate API.
     */

    if ((NULL != msgBuffer) && (0 != msgLen))
    {
        /* First, check if interface is open. */
        fsp_err_t err = FSP_ERR_NOT_ENABLED;
        sci_uart_instance_ctrl_t *p_ctrl = (sci_uart_instance_ctrl_t *) ptx_uart_dbg.p_ctrl;
        if (SCI_UART_OPEN != p_ctrl->open)
        {
            /* It is not open. Open and configure it here. */
            err = R_SCI_UART_Open(ptx_uart_dbg.p_ctrl, ptx_uart_dbg.p_cfg);

            if (FSP_SUCCESS == err)
            {
                err = R_SCI_UART_CallbackSet(ptx_uart_dbg.p_ctrl, ptx_Utils_Print_Callback, NULL, NULL);
            }

            if (FSP_SUCCESS == err)
            {
                baud_setting_t baud_setting;
                err = R_SCI_UART_BaudCalculate(PTX_UTILS_UART_DEFAULT_BAUDRATE, 0, PTX_UTILS_UART_DEFAULT_BAUDRATE_ERROR, &baud_setting);
                if (FSP_SUCCESS == err)
                {
                    err = R_SCI_UART_BaudSet(ptx_uart_dbg.p_ctrl, (void *) &baud_setting);
                }
            }
        }
        else
        {
            /* Uart interface is already open. Just proceed. */
            err = FSP_SUCCESS;
        }

        if (FSP_SUCCESS == err)
        {
            /* Interface is ready. */
            sUartState = Uart_State_TxOngoing;
            err = R_SCI_UART_Write(ptx_uart_dbg.p_ctrl, (uint8_t*)msgBuffer, (uint32_t)msgLen);

            if (FSP_SUCCESS == err)
            {
                while (Uart_State_TxOngoing == sUartState)
                {
                    /* Wait for the transfer to finish. */
                }
            }
        }
    }
}

void ptx_Utils_Print_Callback(uart_callback_args_t * p_args)
{
    /*
     * This callback (name) is predefined in e2studio configuration.
     */
    if (NULL != p_args)
    {
        switch (p_args->event)
        {
            case UART_EVENT_TX_COMPLETE:
                sUartState = Uart_State_TxDone;
                break;

            default:
                /* Transfer error. */
                break;
        }
    }
}

#elif defined(PTX_UTILITY_PRINTING_CONSOLE)

static void ptx_Utils_Write(char *msgBuffer, uint32_t msgLen)
{

    /* Send message to standard output (console). */
    printf("%s", msgBuffer);

    (void)msgLen;
}

#else

static void ptx_Utils_Write(char *msgBuffer, uint32_t msgLen)
{
    (void)msgBuffer;
    (void)msgLen;
}

#endif

