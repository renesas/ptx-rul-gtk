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
    Module      : MCU-RA4M2 Interface Driver Common Types and Definitions
    File        : ptx_HAL_DRV_Interface_Int.h

    Description : Implementation of the Common Interface Driver Definitions and Types for the MCU-RA4M2 Reference Platform
*/

/**
 * \addtogroup DRV_RA4M2_IMPL_INTERFACE_INT
 *
 * @{
 */

#if !defined(PTX_HAL_DRV_INTERFACE_INT_H)
#define PTX_HAL_DRV_INTERFACE_INT_H

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

/**
 * \brief HAL Communication Interface Port Implementation
 */
typedef enum ptx_HAL_DRV_Interface_Port_Type
{
    HAL_DRV_Interface_Port_Type_Default = 0,      /**< Default Communication Interface Port Implementation. */
    HAL_DRV_Interface_Port_Type_Debug = 1,        /**< Debug (for e.g. Logging or Debug purposes) Communication Interface Port Implementation. */
    HAL_DRV_Interface_Port_Type_Evk = 2,          /**< RRQ35230-POLLER_EVB Board specific Communication Interface Port Implementation. */
    HAL_DRV_Interface_Port_Type_MAX = 3,          /**< MAX value of Enum. */
} ptx_HAL_DRV_Interface_Port_Type_t;

/**
 * \brief Types for all available parameters in the ptx_HAL_DRV_Interface_Parameter struct.
 */
typedef enum ptx_HAL_DRV_Interface_Parameter_Type
{
    HAL_DRV_Interface_Parameter_Type_Transmission_Speed,
    HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address,
    HAL_DRV_Interface_Parameter_Type_UART_Enable_Fast_Mode
} ptx_HAL_DRV_Interface_Parameter_Type_t;

/**
 * \brief Values for all available parameters in the ptx_HAL_DRV_Interface_Parameter struct.
 */
typedef union ptx_HAL_DRV_Interface_Parameter_Value
{
    uint32_t    TransmissionSpeed;              /**< Host interface speed. */
    uint8_t     I2CSlaveAddress;                /**< I2C slave address (if host interface = I2C). */
    uint8_t     LinuxUARTEnableFastMode;        /**< UART enable fast mode (2 stop bits) instead of tcdrain (only on Linux UART). */
} ptx_HAL_DRV_Interface_Parameter_Value_t;

/**
 * \brief Input struct for the SetParameter function.
 */
typedef struct ptx_HAL_DRV_Interface_Parameter
{
    ptx_HAL_DRV_Interface_Parameter_Type_t    type;
    ptx_HAL_DRV_Interface_Parameter_Value_t   value;
} ptx_HAL_DRV_Interface_Parameter_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
