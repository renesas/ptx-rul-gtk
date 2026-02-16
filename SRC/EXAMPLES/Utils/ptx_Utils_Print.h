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
    Module      : Utils
    File        : ptx_Utils_Print.h

    Description : Print module API definition
*/

/**
 * \addtogroup SDK_EXAMPLE_UTILS
 *
 * @{
 */

#if !defined(PTX_UTILS_PRINT_H_)
#define PTX_UTILS_PRINT_H_


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>


#if defined(__cplusplus)
extern "C" {
#endif


/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Prints out formatted message. Depending on the underlying platform, output message shall be
 * written to a predefined Uart port (RA platform), or to standard output stream (Windows or Linux platform).
 *
 * \note The function accepts input arguments the same way as printf function.
 *
 * \param[in]   format      Format according to which the output message is created.
 * \param[in]   ...         Variable argument list.
 *
 */
void ptx_Utils_Print(const char *format, ...);

/**
 * \brief Formats and prints out provided data buffer. Each data byte is formatted according to requested
 * format specifiers and printed out by using ptx_Utils_Print API.
 *
 * \param[in]   buffer          Data buffer to be printed out.
 * \param[in]   bufferLength    The number of bytes to print out of data buffer.
 * \param[in]   addNewLine      If not set to 0, adds (prints) new line after the last data byte has been printed out.
 *                              Otherwise, no newline is added.
 * \param[in]   printASCII      If set to 0, print bytes in hex format. Otherwise, print bytes in ASCII format.
 *
 */
void ptx_Utils_Print_Buffer(uint8_t *buffer, uint16_t bufferLength, uint8_t addNewLine, uint8_t printASCII);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
