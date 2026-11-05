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

    Description : Implementation of the print module. The messages are printed to the standard output console by using the
                  native printf API, if PTX_UTILITY_PRINTING_CONSOLE is defined. Otherwise, printing is disabled.
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




/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_MAX_PRINTBUF_SIZE                     (256U)      /* Maximum message buffer length. Note: last byte shall be 0. */
#define PTX_ASCII_SPACE                           (0x20U)     /* ASCII table SPACE character. */
#define PTX_PRINT_LINE_LENGTH                     80U


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

#if defined(PTX_UTILITY_PRINTING_CONSOLE)

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

