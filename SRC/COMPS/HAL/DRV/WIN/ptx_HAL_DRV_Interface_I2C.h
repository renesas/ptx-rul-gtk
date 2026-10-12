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
    Module      : Windows I2C Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_I2C.h

    Description : Implementation of the I2C Communication Interface Driver for the Windows Reference Platform using FTDI
*/

/**
 * \addtogroup DRV_WIN_IMPL_INTERFACE_I2C
 *
 * @{
 */

#if !defined(PTX_HAL_DRV_INTERFACE_I2C_H)
#define PTX_HAL_DRV_INTERFACE_I2C_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>

#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_HAL_DRV_Interface_Int.h"
#include "ptx_OSAL.h"
#include "ptx_HAL_Ftd2xx.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
* \brief Communication Interface specific I2C definitions (Config / Speed) for Windows Platform.
 * @{
*/
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4C          (uint8_t)(0x4Cu)        /**< I2C Address 0x4C -> ADDR1 = 0 and ADDR0 = 0 */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4D          (uint8_t)(0x4Du)        /**< I2C Address 0x4C -> ADDR1 = 0 and ADDR0 = 1 */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4E          (uint8_t)(0x4Eu)        /**< I2C Address 0x4C -> ADDR1 = 1 and ADDR0 = 0 */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4F          (uint8_t)(0x4Fu)        /**< I2C Address 0x4C -> ADDR1 = 1 and ADDR0 = 1 */

#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_DEFAULT       (100000u)               /**< 100  kHz */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_FAST          (400000u)               /**< 400  kHz */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_FASTPLUS      (1000000u)              /**< 1    MHz */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_HIGHSPEED     (3400000u)              /**< 3.4  MHz */
/** @} */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */


/**
 * \brief HAL GPIO Forward declaration.
 */
typedef struct ptx_HAL_GPIO ptx_HAL_GPIO_t;

/**
 * \brief Port Configuration
 *
 * This structure contains all variables required to operate the I2C driver from the Windows Platform.
 *
 * \note Integrators must initialise this structure according to their needs! A working example can be found in the
 * corresponding c file.
 */
typedef struct ptx_HAL_DRV_Interface_I2C_PortConfig
{
    ptx_HAL_DRV_Interface_Port_Type_t Type;             /**< Implementation Type of the hardware interface port. */
    uint8_t                           ID;               /**< ID of the hardware interface port implementation. */

    char                             *ModuleDescriptor; /**< FTDI module descriptor. */

    ptx_HAL_FTD2XX_GPIO_t             IRQPin_Low;       /**< SPI Default IRQ pin low byte. */
    ptx_HAL_FTD2XX_GPIO_t             IRQPin_High;      /**< SPI Default IRQ pin high byte.*/
} ptx_HAL_DRV_Interface_I2C_PortConfig_t;

/**
 * \brief Wait for IRQ flag.
 *
 */
typedef enum ptx_HAL_Interrupt_Flag
{
    INTERRUPT_FALSE, /**< Interrupt is not expected */
    INTERRUPT_TRUE,  /**< Interrupt is expected and is waited for */
} ptx_HAL_Interrupt_Flag_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL_DRV_Interface_I2C
{
    ptx_Comps_t                            CompId;              /**< Component ID. */

    ptx_HAL_FTD2XXCtx_t                    *Ftd2xx_ctx;         /**< FTDI context. */
    ptx_HAL_DRV_Interface_I2C_PortConfig_t *Port;               /**< Pointer to Port config. */
    ptx_Log_t                              *Log;                /**< Pointer to an initialized logger. */
    ptx_OSAL_t                             *OS;                 /**< Pointer to an initialized Osal. */
    uint32_t                               Speed;               /**< I2C Communication Speed. */
    uint8_t                                BusAddr;             /**< Bus Address. */
    ptx_HAL_Interrupt_Flag_t               WaitForInterrupt;    /**< Wait for interrupt flag, can be set interrupt is expected to wait for it. */
} ptx_HAL_DRV_Interface_I2C_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Interface_I2C_InitParams
{
    ptx_HAL_DRV_Interface_Port_Type_t   PortType;           /**< Implementation Type of the hardware interface port. */
    uint8_t                             PortID;             /**< ID of the hardware interface port implementation. */

    uint32_t                            Speed;              /**< I2C Communication Speed. */
    uint8_t                             Address;            /**< Bus Address. */
    ptx_Log_t                           *Log;               /**< Pointer to an initialized logger. */
    void                                *OS;                /**< Pointer to initialized Osal context. */
    ptx_HAL_GPIO_t                      *Gpio;              /**< Initialized GPIO Context. */
} ptx_HAL_DRV_Interface_I2C_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * @brief Initialize the Windows I2C interface driver (FTDI-backed).
 *
 * This function prepares the driver context for use on the Windows reference platform
 * using an FTDI device as I2C master. It selects a matching internal port configuration
 * based on @p initParams->PortType and @p initParams->PortID, opens the underlying FTDI
 * library context, and stores runtime configuration (I2C speed and 7-bit slave address)
 * into the driver context.
 *
 * @details
 * - The function clears the provided @p ctx and then populates it on success.
 * - If @p initParams->Speed is 0, the driver uses @ref PTX_HAL_DRV_INTERFACE_I2C_SPEED_DEFAULT.
 * - If @p initParams->Address is 0, the driver uses @ref PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4C.
 * - The FTDI library context is opened during Init; the I2C channel itself is not opened
 *   here. Call @ref ptx_HAL_DRV_Interface_I2C_Open afterwards to open/configure the I2C block.
 *
 * @param[in,out] ctx
 *   Pointer to the driver context to initialize. On success, the context is marked as a valid
 *   HAL component and can be used with the other API functions.
 *
 * @param[in] initParams
 *   Initialization parameters:
 *   - PortType / PortID: select which internal port configuration to use.
 *   - Speed: I2C bus speed in Hz (0 => default).
 *   - Address: 7-bit I2C slave address (0 => default 0x4C).
 *   - Log: must be non-NULL (used for diagnostics).
 *   - OS: stored as OSAL context pointer.
 *   - Gpio: not used by this implementation.
 *
 * @return
 *   - @ref PTX_STATUS_SUCCESS on success.
 *   - @ref PTX_STATUS_E_GEN_INVALID_PARAMETER if @p ctx, @p initParams, or @p initParams->Log is NULL,
 *     or if no matching port configuration exists.
 *   - Any error propagated from the underlying FTDI library open call.
 *
 * @pre
 * - @p initParams->Log must be initialized.
 *
 * @post
 * - On success, @p ctx contains a valid FTDI context and stored configuration (speed/address).
 * - To start I2C transfers, @ref ptx_HAL_DRV_Interface_I2C_Open must be called.
 *
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Init(ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_I2C_InitParams_t *initParams);

/**
 * @brief De-initialize the Windows I2C interface driver (FTDI-backed).
 *
 * De-initializes the driver context and releases the underlying FTDI library context
 * that was opened during @ref ptx_HAL_DRV_Interface_I2C_Init.
 *
 * @details
 * - This function is the counterpart to @ref ptx_HAL_DRV_Interface_I2C_Init and is intended
 *   to free resources and invalidate the driver context.
 * - The implementation closes the FTDI library context via ptx_HAL_FTD2XX_LibClose().
 * - On success, the complete @p ctx memory is cleared and the component ID is reset.
 * - If closing the FTDI context fails, the function returns the error and leaves @p ctx
 *   unchanged (still valid/initialized).
 *
 * @param[in,out] ctx
 *   Pointer to an initialized @ref ptx_HAL_DRV_Interface_I2C_t context.
 *
 * @return
 *   - @ref PTX_STATUS_SUCCESS on success.
 *   - @ref PTX_STATUS_E_GEN_INVALID_PARAMETER if @p ctx is NULL or not a valid HAL component
 *     (i.e., not initialized / component check fails).
 *   - Any error propagated from the underlying FTDI library close call.
 *
 * @pre
 * - @p ctx must have been successfully initialized with @ref ptx_HAL_DRV_Interface_I2C_Init.
 *
 * @post
 * - On success, @p ctx is cleared and must not be used again unless re-initialized.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_DeInit(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * @brief Open and configure the I2C hardware interface.
 *
 * Opens the FTDI-based I2C controller and configures it with the transmission speed
 * stored in the driver context during initialization or via SetParameter.
 *
 * @details
 * - This function enables logical access to the I2C bus.
 * - It must be called after @ref ptx_HAL_DRV_Interface_I2C_Init and before any
 *   Tx/Rx/TRx operations.
 * - The I2C slave address is not configured here; it is applied per transfer.
 *
 * @param[in] ctx
 *   Pointer to an initialized HAL I2C driver context.
 *
 * @return
 *   - @ref PTX_STATUS_SUCCESS on success.
 *   - @ref PTX_STATUS_E_GEN_INVALID_PARAMETER if @p ctx is NULL or not a valid HAL component.
 *   - Any error propagated from the underlying FTDI I2C open call.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Open(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * @brief Close the I2C hardware interface.
 *
 * Closes the FTDI-based I2C controller and releases logical access to the I2C bus.
 *
 * @details
 * - This function disables I2C communication on the underlying FTDI device.
 * - It does not de-initialize or clear the driver context.
 * - After calling Close(), the driver context remains valid and may be
 *   re-opened using @ref ptx_HAL_DRV_Interface_I2C_Open.
 *
 * @param[in] ctx
 *   Pointer to an initialized HAL I2C driver context.
 *
 * @return
 *   - @ref PTX_STATUS_SUCCESS on success.
 *   - @ref PTX_STATUS_E_GEN_INVALID_PARAMETER if @p ctx is NULL or not a valid HAL component.
 *   - Any error propagated from the underlying FTDI I2C close call.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Close(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * @brief Transmit bytes to the configured I2C slave.
 *
 * Sends @p txLen bytes from @p txBuf to the 7-bit slave address stored in @c ctx->BusAddr.
 *
 * @details
 * - The effective slave address is taken from the driver context (Init default or SetParameter).
 * - The @p restart parameter is forwarded to the underlying transfer routine to control
 *   whether a (re-)START condition is issued as part of the operation.
 * - This function performs transmit-only; use TRx for combined write-then-read transactions.
 *
 * @param[in] ctx     Pointer to an initialized HAL I2C driver context.
 * @param[in] txBuf   Buffer containing bytes to transmit.
 * @param[in] txLen   Number of bytes to transmit.
 * @param[in] restart Restart/repeated-start control forwarded to the lower layer.
 *
 * @return
 *   - @ref PTX_STATUS_SUCCESS on success.
 *   - @ref PTX_STATUS_E_GEN_INVALID_PARAMETER if @p ctx is NULL / invalid component.
 *   - Any error propagated from the underlying FTDI I2C transfer call.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Tx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *txBuf, uint32_t txLen, uint8_t restart);

/**
 * @brief Receive bytes from the configured I2C slave.
 *
 * Reads @p rxLen bytes from the 7-bit slave address stored in @c ctx->BusAddr and writes
 * the received data into @p rxBuf.
 *
 * @details
 * - The effective slave address is taken from the driver context (Init default or SetParameter).
 * - The @p restart parameter is forwarded to the underlying transfer routine to control
 *   whether a (re-)START condition is issued as part of the operation.
 * - This function performs receive-only; use TRx for combined write-then-read transactions.
 *
 * @param[in] ctx     Pointer to an initialized HAL I2C driver context.
 * @param[out] rxBuf  Receive buffer to store incoming bytes.
 * @param[in] rxLen   Number of bytes to read.
 * @param[in] restart Restart/repeated-start control forwarded to the lower layer.
 *
 * @return
 *   - @ref PTX_STATUS_SUCCESS on success.
 *   - @ref PTX_STATUS_E_GEN_INVALID_PARAMETER if @p ctx is NULL / invalid component.
 *   - Any error propagated from the underlying FTDI I2C transfer call.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Rx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *rxBuf, uint32_t rxLen, uint8_t restart);

/**
 * @brief Read the current RX IRQ pin level.
 *
 * Reads the current logic level of the IRQ line associated with the interface and returns it
 * as a boolean-like value (0 or 1) via @p level. This is typically used by upper layers to
 * detect whether the device signals pending data to be read.
 *
 * @details
 * - The implementation queries the IRQ line via the FTDI layer and converts the returned
 *   raw value to 0/1.
 *
 * @param[in]  ctx   Pointer to an initialized HAL I2C driver context.
 * @param[out] level Output pointer receiving the current IRQ level (0 = low, 1 = high).
 *
 * @return
 *   - @ref PTX_STATUS_SUCCESS on success.
 *   - @ref PTX_STATUS_E_GEN_INVALID_PARAMETER if @p ctx is NULL / invalid component.
 *   - Any error propagated from the underlying IRQ read call.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_RxIrqLevel(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *level);

/**
 * @brief Set a runtime parameter of the I2C interface driver.
 *
 * Updates a driver parameter selected by @p parameter->type using the corresponding value
 * in @p parameter->value.
 *
 * @details
 * Supported parameter types in this implementation:
 * - HAL_DRV_Interface_Parameter_Type_Transmission_Speed:
 *   Updates @c ctx->Speed and re-opens the I2C controller with the new speed.
 * - HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address:
 *   Updates @c ctx->BusAddr (used for subsequent transfers).
 *
 * Unsupported / not available:
 * - HAL_DRV_Interface_Parameter_Type_UART_Enable_Fast_Mode returns NOT_AVAILABLE.
 * - Any other type is ignored and results in an error return.
 *
 * @param[in]     ctx       Pointer to an initialized HAL I2C driver context.
 * @param[in,out] parameter Pointer to parameter selector/value container.
 *
 * @return
 *   - @ref PTX_STATUS_SUCCESS on success.
 *   - @ref PTX_STATUS_E_GEN_INVALID_PARAMETER if @p ctx is invalid or @p parameter is NULL.
 *   - @ref PTX_STATUS_E_GEN_NOT_AVAILABLE for unsupported parameter types (e.g. UART fast mode).
 *   - Any error propagated from re-opening the I2C controller when changing speed.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetParameter(ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * @brief Get the current runtime parameter value of the I2C interface driver.
 *
 * Reads back a driver parameter selected by @p parameter->type and writes the corresponding
 * value into @p parameter->value.
 *
 * @details
 * Supported parameter types in this implementation:
 * - HAL_DRV_Interface_Parameter_Type_Transmission_Speed: returns @c ctx->Speed
 * - HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address: returns @c ctx->BusAddr
 *
 * @param[in]     ctx       Pointer to an initialized HAL I2C driver context.
 * @param[in,out] parameter Pointer to parameter selector/value container.
 *
 * @return
 *   - @ref PTX_STATUS_SUCCESS on success.
 *   - @ref PTX_STATUS_E_GEN_INVALID_PARAMETER if @p ctx is invalid or @p parameter is NULL.
 *   - @ref PTX_STATUS_E_GEN_INVALID_PARAMETER for unsupported/unknown parameter types.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_GetParameter(ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

