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
    Module      : MCU-RA4M2 MCU Driver
    File        : ptx_HAL_DRV_MCU.h

    Description : Implementation of the MCU Driver for the MCU-RA4M2 Reference Platform
*/

/**
 * \addtogroup DRV_RA4M2_IMPL_MCU
 *
 * @{
 */

#if !defined(PTX_HAL_DRV_MCU_H)
#define PTX_HAL_DRV_MCU_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Status.h"

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
 * \brief Enables MCU interrupts.
 *
 * Enables all maskable interrupts on the MCU by calling the ARM Cortex-M intrinsic function __enable_irq().
 * This allows interrupt handlers to execute when interrupt events occur. Typically called after critical
 * initialization sections where interrupts must be disabled.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_MCU_EnableIrq ();

/**
 * \brief Disables MCU interrupts.
 *
 * Disables all maskable interrupts on the MCU by calling the ARM Cortex-M intrinsic function __disable_irq().
 * This prevents interrupt handlers from executing, allowing critical sections of code to run without
 * interruption. Interrupts remain disabled until ptx_HAL_DRV_MCU_EnableIrq is called.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_MCU_DisableIrq ();

/**
 * \brief Puts the MCU into a low-power wait-for-interrupt state.
 *
 * Executes a sequence of ARM Cortex-M intrinsic functions to place the MCU into a low-power state:
 * - __DSB(): Data Synchronization Barrier - ensures all memory operations complete before sleep
 * - __WFI(): Wait For Interrupt - puts the MCU into sleep mode until an interrupt occurs
 * - __ISB(): Instruction Synchronization Barrier - ensures instruction pipeline is synchronized after wakeup
 *
 * This function is typically used in idle loops or power management routines to reduce power consumption
 * while waiting for interrupt events. The MCU will wake up when any enabled interrupt occurs.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_MCU_WaitForIrq ();

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
