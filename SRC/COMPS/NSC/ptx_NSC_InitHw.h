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
    Module      : NFC Soft Controller (NSC) Interface Header for Hardware Initialization
    File        : ptx_NSC_InitHw.h

    Description : Definition of NFC Soft Controller Interface for Hardware Initialization.
                  Contains function(s) to set-up the hardware after successful boot up and firmware activation.
*/

/**
 * \addtogroup NSC_HARDWARE
 *
 * @{
 */

#if !defined(PTX_NSC_INITHW_H)
#define PTX_NSC_INITHW_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NSC_Int.h"

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

/**
 * \brief UART baud-rate configuration structure
 */
typedef struct ptx_NSC_CustomBaudrates
{
    uint32_t    uartBaudrate;           /**< Required baud-rate */
    uint8_t     uart_br0_div;           /**< Baud-rate divisor */
    uint8_t     uart_br1_div;           /**< Baud-rate divisor */
    uint8_t     uart_dither_en;         /**< Dithering enabler */
} ptx_NSC_CustomBaudrates_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initialization of the hardware.
 *
 * \param[in] nsc           Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_InitHw(ptx_NSC_t *nsc);

/**
 * \brief Calculate PLL parameters used for NSC clock/frequency setup.
 *
 * \param[in] nsc           Pointer to component.
 * \param[in] sysCfg        Configuration parameters used in calculations.
 * \param[out] pllSettings  Resulting parameters.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_CalculatePllParameters(ptx_NSC_t *nsc, ptx_NSC_SystemConfig_t *sysCfg, ptx_NSC_PllConfig_t *pllSettings);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
