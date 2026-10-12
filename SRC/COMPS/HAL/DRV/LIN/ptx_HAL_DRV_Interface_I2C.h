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
    Module      : Linux I2C Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_I2C.h

    Description : Implementation of the I2C Communication Interface Driver for the Linux Reference Platform
*/

/**
 * \addtogroup DRV_LIN_IMPL_INTERFACE_I2C
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
#include <string.h>

#include <linux/i2c.h>

#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_OSAL.h"
#include "ptx_Log.h"
#include "ptx_HAL_DRV_Interface_Int.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
* \brief Default device specific I2C definitions (Address / Speed).
 * @{
*/
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4C          (uint8_t)(0x4Cu)        /**< I2C Address 0x4C -> ADDR1 = 0 and ADDR0 = 0 */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4D          (uint8_t)(0x4Du)        /**< I2C Address 0x4D -> ADDR1 = 0 and ADDR0 = 1 */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4E          (uint8_t)(0x4Eu)        /**< I2C Address 0x4E -> ADDR1 = 1 and ADDR0 = 0 */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4F          (uint8_t)(0x4Fu)        /**< I2C Address 0x4F -> ADDR1 = 1 and ADDR0 = 1 */

#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_DEFAULT       (100000u)               /**< 100  kHz */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_FAST          (400000u)               /**< 400  kHz */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_FASTPLUS      (1000000u)              /**< 1    MHz */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_HIGHSPEED     (3400000u)              /**< 3.4  MHz */

#define PTX_HAL_DRV_INTERFACE_I2C_KERNEL              (2u)                    /**< Number of I2C data structures required by the Linux kernel */
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
 * This structure contains all variables required to operate the I2C driver from the Linux Platform.
 *
 * \note Integrators must initialise this structure according to their needs! A working example can be found in the
 * corresponding c file.
 */
typedef struct ptx_HAL_DRV_Interface_I2C_PortConfig
{
    ptx_HAL_DRV_Interface_Port_Type_t   Type;                                            /**< Implementation Type of the hardware interface port. */
    uint8_t                             ID;                                              /**< ID of the hardware interface port implementation. */

    int                                 Dev;                                             /**< fd ID. */
    uint32_t                            PortNr;                                          /**< I2C Port Number. */

    int                                 IrqFd;                                           /**< I2C IRQ GPIO pin file descriptor. */
    int                                 IrqFdCancel;                                     /**< I2C IRQ GPIO pin cancel file descriptor. */

    uint32_t                            IrqPin;                                          /**< I2C IRQ Pin Number of the predefined gpiochip device. */

    struct i2c_msg                      Msgs[PTX_HAL_DRV_INTERFACE_I2C_KERNEL];          /**< Pointer to Linux i2c_msg structure. */
} ptx_HAL_DRV_Interface_I2C_PortConfig_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL_DRV_Interface_I2C
{
    ptx_Comps_t                             CompId;         /**< Component ID. */

    ptx_HAL_DRV_Interface_I2C_PortConfig_t  *Port;          /**< I2C Default internal parameters. */
    ptx_Log_t                               *Log;           /**< Pointer to an initialized logger. */
    ptx_OSAL_t                              *OS;            /**< Pointer to an initialized Osal. */
    ptx_Mutex_t                             Mutex;          /**< Mutex for receiving function. */
    uint32_t                                Speed;          /**< I2C Communication Speed. */
    uint8_t                                 BusAddr;        /**< Bus Address. */
    
} ptx_HAL_DRV_Interface_I2C_t;
/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Interface_I2C_InitParams
{
    ptx_HAL_DRV_Interface_Port_Type_t   PortType;           /**< Implementation Type of the hardware interface port. */
    uint8_t                             PortID;             /**< ID of the hardware interface port implementation. */

    uint32_t                            Speed;          /**< I2C Communication Speed */
    uint32_t                            PortNr;         /**< I2C Port Number */
    uint8_t                             Address;        /**< Bus Address */
    ptx_Log_t                           *Log;           /**< Pointer to an initialized logger. */
    void                                *OS;            /**< Osal component */
    ptx_HAL_GPIO_t                      *Gpio;          /**< Initialized GPIO Context. */


} ptx_HAL_DRV_Interface_I2C_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * @brief Component Initialization.
 *
 * Initializes the Linux I2C Communication Interface Driver context.
 * The function prepares the driver for use by selecting a matching
 * internal port configuration, storing runtime parameters such as
 * bus speed and slave address, and initializing internal state.
 *
 * @details
 * - This function initializes the driver context only.
 * - It does not open the I2C device node or perform any bus access.
 * - The I2C device is opened later via @ref ptx_HAL_DRV_Interface_I2C_Open.
 * - Default values are applied if speed or address are set to 0.
 *
 * @param[out] ctx
 *   Pointer to the I2C driver component context.
 *
 * @param[in] initParams
 *   Pointer to the initialization parameter structure containing
 *   port selection, bus speed, slave address, logger, and OS context.
 *
 * @return
 *   Status, indicating whether the operation was successful.
 *   See @ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Init (ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_I2C_InitParams_t *initParams);

/**
 * @brief Component De-Initialization.
 *
 * De-initializes the Linux I2C Communication Interface Driver.
 * This function releases all resources associated with the driver
 * context and invalidates the component.
 *
 * @details
 * - This function closes any open I2C device handles.
 * - The driver context is cleared and must not be used afterwards
 *   unless re-initialized via Init().
 *
 * @param[in,out] ctx
 *   Pointer to the I2C driver component context.
 *
 * @return
 *   Status, indicating whether the operation was successful.
 *   See @ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_DeInit (ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * @brief Driver I2C Open.
 *
 * Opens the Linux I2C device and configures it for communication with
 * the configured slave address and bus speed.
 *
 * @details
 * - This function opens the Linux I2C device node (e.g. /dev/i2c-X).
 * - The slave address stored in the driver context is applied via ioctl.
 * - Must be called after Init() and before any Tx/Rx/TRx operations.
 *
 * @param[in] ctx
 *   Pointer to the I2C driver component context.
 *
 * @return
 *   Status, indicating whether the operation was successful.
 *   See @ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Open(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * @brief Driver I2C Close.
 *
 * Closes the Linux I2C device and releases logical access to the I2C bus.
 *
 * @details
 * - This function closes the file descriptor associated with the I2C device.
 * - The driver context remains valid and can be re-opened via Open().
 * - No driver configuration data is cleared.
 *
 * @param[in] ctx
 *   Pointer to the I2C driver component context.
 *
 * @return
 *   Status, indicating whether the operation was successful.
 *   See @ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Close(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * @brief Driver I2C Tx.
 *
 * Sends data to the configured I2C slave device.
 *
 * @details
 * - If restart is set to 0, the transmit operation is executed immediately
 *   as a standalone I2C transaction (START + STOP).
 * - If restart is non-zero, the transmit message is stored internally
 *   and combined with a subsequent Rx call to form a single I2C transaction
 *   without issuing a STOP condition (repeated START).
 * - This behavior enables register-based I2C accesses on Linux using
 *   the I2C_RDWR ioctl interface.
 *
 * @param[in] ctx
 *   Pointer to the I2C driver component context.
 * @param[in] txBuf
 *   Array of bytes to be transmitted.
 * @param[in] txLen
 *   Length of the transmit buffer.
 * @param[in] restart
 *   Restart control flag:
 *   - 0: send Tx immediately with STOP
 *   - non-zero: defer STOP and combine with Rx
 *
 * @return
 *   Status, indicating whether the operation was successful.
 *   See @ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Tx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *txBuf, uint32_t txLen, uint8_t restart);

/**
 * @brief Driver I2C Rx.
 *
 * Receives data from the configured I2C slave device.
 *
 * @details
 * - This function is designed to work in conjunction with a preceding
 *   Tx operation that was called with restart=1.
 * - It uses the previously stored Tx message (from Tx with restart=1)
 *   and combines it with the Rx operation to form a single I2C transaction
 *   using repeated START (no STOP between Tx and Rx).
 * - The function sends both the stored Tx message and the Rx message
 *   to the Linux kernel via I2C_RDWR ioctl with nmsgs=2.
 * - The Rx message uses I2C_M_RD | I2C_M_NOSTART flags to indicate
 *   a read operation without issuing a START condition.
 * - This behavior enables register-based I2C accesses on Linux.
 *
 * @param[in] ctx
 *   Pointer to the I2C driver component context.
 * @param[out] rxBuf
 *   Buffer to store received bytes.
 * @param[in] rxLen
 *   Length of the receive buffer and number of bytes to receive.
 * @param[in] restart
 *   Restart control flag (currently unused in this implementation).
 *
 * @return
 *   Status, indicating whether the operation was successful.
 *   See @ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Rx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *rxBuf, uint32_t rxLen, uint8_t restart);

/**
 * \brief Driver I2C Rx IRQ Level
 *
 * Reads the current IRQ level from the configured GPIO pin by checking if the
 * IRQ is already pending on the GPIO line.
 *
 * @details
 * - Validates that the provided context is a valid HAL component.
 * - Calls the internal GPIO helper function to check the pending IRQ state.
 * - Returns the current IRQ pin level (0 or 1) via the level output parameter.
 * - The GPIO pin must be properly configured in the port configuration structure.
 * - This function performs a synchronous read of the GPIO line state.
 *
 * \param[in]       ctx             Pointer to the I2C driver component context.
 * \param[out]      level           Pointer to store the current IRQ pin level (0 or 1).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_RxIrqLevel(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *level);

/**
 * \brief Driver I2C Set Parameter function.
 *
 * Sets a HAL I2C Communication Interface parameter selected by type.
 *
 * @details
 * Configures I2C interface parameters based on the parameter type specified.
 * Supported parameters include:
 * - HAL_DRV_Interface_Parameter_Type_Transmission_Speed: Sets the I2C bus speed
 * - HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address: Sets the I2C slave address
 * - HAL_DRV_Interface_Parameter_Type_UART_Enable_Fast_Mode: Not available for I2C
 *
 * The function validates the context and parameter pointer before processing.
 * Invalid parameter types are silently ignored and return the default status.
 *
 * \param[in]       ctx            Pointer to the I2C driver component context.
 * \param[in]       parameter      Pointer to parameter structure containing type and value.
 *                                 See \ref ptx_HAL_DRV_Interface_Parameter_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetParameter(ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief Driver I2C Get Parameter function.
 *
 * Retrieves the current value of a HAL I2C Communication Interface parameter.
 *
 * @details
 * Reads the current I2C interface parameter values based on the parameter type specified.
 * Supported parameters include:
 * - HAL_DRV_Interface_Parameter_Type_Transmission_Speed: Retrieves the current I2C bus speed
 * - HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address: Retrieves the current I2C slave address
 *
 * The function validates the context and parameter pointer before processing.
 * The retrieved value is stored in the parameter->value field corresponding to the type.
 * Unsupported parameter types return the default status without modification.
 *
 * \param[in]       ctx            Pointer to the I2C driver component context.
 * \param[in,out]   parameter      Pointer to parameter structure. On input, contains the type
 *                                 to retrieve. On output, contains the retrieved value.
 *                                 See \ref ptx_HAL_DRV_Interface_Parameter_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_GetParameter(ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

