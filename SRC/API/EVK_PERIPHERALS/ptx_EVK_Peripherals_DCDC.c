/* \file
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
    Module      : EVK Peripherals - DC/DC Converter
    File        : ptx_EVK_Peripherals_DCDC.c

    Description : Submodule of EVK Peripherals, which allows the control of the DC/DC Converter
                  component on the RRQ35230-POLLER_EVB Board.

 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_EVK_Peripherals_DCDC.h"
#include <stdbool.h>
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_EVK_PERIPHERALS_DCDC_MAX_VOLTAGE_VALUE      (540u) /**< Max. voltage of the DC/DC converter limited to 5.4V. (Units of 10mV) */
#define PTX_EVK_PERIPHERALS_DCDC_STARTUP_VOLTAGE_VALUE  (400u) /**< Initial startup voltage of the system set to 4.0V. (Units of 10mV) */
#define PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_START_BYTE     (0x00) /**< Value of the Startbyte in the beginning of a set voltage message (written to the I2C configurable DC/DC converter). */
#define PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_REG_L_MASK     (0x07) /**< Bitmask for the lower 3 bits of the voltage setpoint (written to the I2C configurable DC/DC converter). */
#define PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_REG_H_MASK     (0xFF) /**< Bitmask for the higher 8 bits of the voltage setpoint (written to the I2C configurable DC/DC converter). */
#define PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_REG_H_SHIFT    (0x03) /**< Bitshift for the higher 8 bits of the voltage setpoint (written to the I2C configurable DC/DC converter). */
#define PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_GO_BIT         (0x01) /**< "GO" bit of the DC/DC converter. When the bit is written to the register, the DC/DC converter will adjust to the newly set voltage. */
#define PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_BUFFER_SIZE    (4u)   /**< Size of the Message to the DC/DC Converter to set a voltage. */
#define PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_START_BYTE_POS (0u)   /**< Position of the Starting Byte. */
#define PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_REG_L_POS      (1u)   /**< Position of the lower part of the voltage value. */
#define PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_REG_H_POS      (2u)   /**< Position of the higher part of the voltage value. */
#define PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_GO_BIT_POS     (3u)   /**< Position of the GO bit value. */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_EVK_Peripherals_DCDC_Init(ptx_EVK_Peripherals_DCDC_t *ctx, uint8_t i2cAddress)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != ctx)
    {
        ctx->CompId = Comp_WLC_PERIPHERALS;
        ctx->BusAddress = i2cAddress;

        status = ptx_EVK_Peripherals_DCDC_SetVoltage(ctx, PTX_EVK_PERIPHERALS_DCDC_STARTUP_VOLTAGE_VALUE);

        if (PTX_STATUS_SUCCESS != status)
        {
            memset(ctx, 0, sizeof(ptx_EVK_Peripherals_DCDC_t));
        }
    }

    return status;
}

ptx_Status_t ptx_EVK_Peripherals_DCDC_Deinit(ptx_EVK_Peripherals_DCDC_t *ctx)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC_PERIPHERALS))
    {
        (void)memset(ctx, 0, sizeof(ptx_EVK_Peripherals_DCDC_t));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_EVK_Peripherals_DCDC_SetVoltage(ptx_EVK_Peripherals_DCDC_t *ctx, uint16_t voltage)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ctx, Comp_WLC_PERIPHERALS) && (PTX_EVK_PERIPHERALS_DCDC_MAX_VOLTAGE_VALUE >= voltage))
    {
        ptx_HAL_DRV_Interface_Parameter_t addressParameter;
        addressParameter.type = HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address;
        addressParameter.value.I2CSlaveAddress = ctx->BusAddress;

        status = ptx_HAL_Interface_SetParameter((void *)ctx->i2cInterface, &addressParameter);

        if (PTX_STATUS_SUCCESS == status)
        {
            uint8_t tx_buf[PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_BUFFER_SIZE];
            tx_buf[PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_START_BYTE_POS] = PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_START_BYTE;
            tx_buf[PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_REG_L_POS] = (uint8_t)(voltage & PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_REG_L_MASK);
            tx_buf[PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_REG_H_POS] = (uint8_t)((voltage >> PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_REG_H_SHIFT) & PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_REG_H_MASK);
            tx_buf[PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_GO_BIT_POS] = PTX_EVK_PERIPHERALS_DCDC_VOLTAGE_GO_BIT;

            uint8_t *tx[1] = { tx_buf };
            uint32_t tx_len[1] = { sizeof(tx_buf) };

            status = ptx_HAL_Interface_TRx(ctx->i2cInterface, tx, tx_len, 1, false,  NULL, 0);
        }
    }

    return status;
}
