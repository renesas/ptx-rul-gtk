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
    Module      : Linux UART Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_UART.h

    Description : Implementation of the UART Communication Interface Driver for the Linux Reference Platform

*/

/**
 * \addtogroup DRV_LIN_IMPL_INTERFACE_UART
 *
 * @{
 */

#if !defined(PTX_HAL_DRV_INTERFACE_UART_H)
#define PTX_HAL_DRV_INTERFACE_UART_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_Log.h"
#include "ptx_HAL_DRV_Interface_Int.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/eventfd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include "ptx_OSAL.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
* \brief Default device specific UART definitions (Config / Speed) for Linux.
 * @{
*/
#define PTX_HAL_DRV_INTERFACE_UART_DEFAULT_BAUDRATE                           (115200u)   /**< Default Uart Baudrate, set during device initialisation. */
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
 * This structure contains all variables required to operate the UART driver from the Linux Platform.
 *
 * \note Integrators must initialise this structure according to their needs! A working example can be found in the
 * corresponding c file.
 */
typedef struct ptx_HAL_DRV_Interface_UART_PortConfig
{
    ptx_HAL_DRV_Interface_Port_Type_t   Type;               /**< Implementation Type of the hardware interface port. */
    uint8_t                             ID;                 /**< ID of the hardware interface port implementation. */

    int                                 Dev;                /**< fd ID. */
    struct termios                      Options;            /**< Terminal options. */

    int                                 IrqFd;              /**< UART IRQ GPIO pin file descriptor. */
    int                                 IrqFdCancel;        /**< UART IRQ GPIO pin cancel file descriptor. */

    uint32_t                            IrqPin;             /**< UART IRQ Pin Number of the predefined gpiochip device. */


} ptx_HAL_DRV_Interface_UART_PortConfig_t;


/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL_DRV_Interface_UART
{
    ptx_Comps_t                                 CompId;             /**< Component ID. */

    ptx_HAL_DRV_Interface_UART_PortConfig_t     *Port;              /**< UART Port Implementation. */
    ptx_Log_t                                   *Log;               /**< Pointer to an initialized logger. */
    ptx_OSAL_t                                  *OS;                /**< Pointer to an initialized Osal. */
    ptx_Mutex_t                                 Mutex;              /**< Mutex for receiving function. */
    uint32_t                                    Speed;              /**< UART Default driver speed. */
    uint8_t                                     EnableFastMode;     /**< Enable fast mode using 2 stop bits instead of tcdrain. */
} ptx_HAL_DRV_Interface_UART_t;

/**
 * \brief Init parameters to Open Uart Port.
 */
typedef struct ptx_HAL_DRV_Interface_UART_InitParams
{
    ptx_HAL_DRV_Interface_Port_Type_t   PortType;           /**< Implementation Type of the hardware interface port. */
    uint8_t                             PortID;             /**< ID of the hardware interface port implementation. */

    uint8_t                             *InterfaceName;     /**< Device name. */
    uint32_t                            InterfaceNameLen;   /**< Device name length. */
    uint32_t                            Speed;              /**< UART Default driver speed. */
    ptx_Log_t                           *Log;               /**< Initialised Logger Context. */
    void                                *OS;                /**< Osal component. */
    ptx_HAL_GPIO_t                      *Gpio;              /**< Initialized GPIO Context. */
    uint32_t                            Timeout;            /**< UART Default Tx/Rx Transaction Timeout. */
} ptx_HAL_DRV_Interface_UART_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Driver UART Init
 *
 * Initializes the Linux UART driver according to the given parameters. Searches the global
 * port configuration table for a matching port type and ID, initializes the synchronization
 * mutex, and opens the serial port device with the specified baud rate.
 *
 * \param[in]       ctx             Pointer to the component structure to be initialized.
 *                                  Will be populated with port configuration, baud rate,
 *                                  OS context, and logger references.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Init(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_UART_InitParams_t *initParams);

/**
 * \brief Driver UART DeInit
 *
 * De-initializes the Linux UART driver by closing the serial port device, releasing the
 * synchronization mutex, and clearing the component structure. Ensures thread-safe cleanup
 * by acquiring the mutex before closing the port.
 *
 * \param[in]       ctx             Pointer to an initialized component (CompId must be Comp_HAL).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 *
 * \note Acquires mutex lock before closing the port to prevent concurrent access.
 * \note Preserves error status from port close operation if mutex unlock fails.
 * \note On successful completion, CompId is set to Comp_None and context is zeroed.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_DeInit(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Open
 *
 * Opens and prepares the Linux UART interface for communication. Discards any pending
 * data in the receive buffer and configures interrupt handling for asynchronous receive
 * operations.
 *
 * \param[in]       ctx             Pointer to an initialized component (CompId must be Comp_HAL).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 *
 * \note This function must be called after Init() and before any Tx/Rx operations.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Open(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Close
 *
 * Closes the Linux UART interface by releasing associated interrupt file descriptors.
 * Disables interrupt-driven receive operations and cleans up event notification resources.
 *
 * \param[in]       ctx             Pointer to an initialized component (CompId must be Comp_HAL).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 *
 * \note Closes both IrqFd (interrupt event descriptor) and IrqFdCancel (cancellation descriptor).
 * \note Does not close the main serial port device (ctx->Port->Dev); use DeInit() for full cleanup.
 * \note Always returns PTX_STATUS_SUCCESS if validation passes, regardless of close() results.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Close(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Tx
 *
 * Transmits data to the UART interface to communicate with an endpoint. Handles partial
 * writes by retrying until all bytes are sent.
 *
 * \param[in]       ctx             Pointer to an initialized component (CompId must be Comp_HAL).
 * \param[in]       txBuf           Pointer to array of bytes to be transmitted. Must not be NULL.
 * \param[in]       txLen           Length of txBuf in bytes. Must be greater than 0.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 *
 * \note Thread-safe: Acquires mutex lock during write operation to prevent concurrent access.
 * \note Handles partial writes: Retries write() calls until all bytes are transmitted, as
 *       write() may not send all requested bytes in a single call.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Tx(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *txBuf, uint32_t txLen);

/**
 * \brief Driver UART Rx
 *
 * Receives data from the UART interface. Handles partial reads by retrying until the
 * requested number of bytes are received or a timeout occurs. Supports zero-length reads
 * as a valid operation.
 *
 * \param[in]       ctx             Pointer to an initialized component (CompId must be Comp_HAL).
 * \param[in]       rxBuf           Pointer to array of bytes to receive data into. Must not be NULL.
 * \param[in]       rxLen           Length of rxBuf in bytes. Expected number of bytes to receive.
 *                                  May be 0 for no-op receive operation.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 *
 * \note Handles partial reads: Retries read() calls until all requested bytes are received,
 *       as read() may return fewer bytes than requested. In worst case, reads one byte at a time.
 * \note Retry mechanism: Uses PTX_HAL_DRV_INTERFACE_UART_MAX_WRRD_RETRIES limit to prevent
 *       infinite blocking. Each read() call may return up to PTX_HAL_DRV_INTERFACE_UART_MAX_ALLOWED_FRAME_LENGTH bytes.
 * \note Zero-length receive: If rxLen is 0, returns PTX_STATUS_SUCCESS without reading data.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Rx(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *rxBuf, uint32_t rxLen);

/**
 * \brief Driver Rx IRQ Level Callback
 *
 * Queries the current state of the receive IRQ GPIO pin to determine if an interrupt is
 * pending. Checks the GPIO line level to detect if a rising edge event has occurred.
 *
 * \param[in]       ctx             Pointer to an initialized component (CompId must be Comp_HAL).
 * \param[out]      level           Pointer to uint8_t to receive the IRQ line state:
 *                                  - 0: IRQ line is low (no pending interrupt)
 *                                  - 1: IRQ line is high (interrupt pending)
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 *
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_RxIrqLevel(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *level);

/**
 * \brief Driver UART Set Parameter
 *
 * Configures HAL UART communication interface parameters. Supports dynamic reconfiguration
 * of transmission speed and fast mode settings. Parameters are specified via a type-value pair
 * structure that selects which parameter to modify and provides the corresponding configuration value.
 *
 * \param[in]       ctx            Pointer to an initialized component (CompId must be Comp_HAL).
 * \param[in]       parameter      Pointer to parameter structure containing:
 *                                 - type: Parameter type selector (see ptx_HAL_DRV_Interface_Parameter_Type_t)
 *                                 - value: Union containing parameter-specific data
 *                                   (see ptx_HAL_DRV_Interface_Parameter_Value_t)
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 * 
 * \note Supported parameters:
 *       - HAL_DRV_Interface_Parameter_Type_Transmission_Speed: Changes UART baud rate via
 *         ptx_HAL_DRV_Interface_UART_OpenAltBaudrate()
 *       - HAL_DRV_Interface_Parameter_Type_UART_Enable_Fast_Mode: Enables/disables 2 stop bit
 *         mode for improved performance. Sets stop bits on PTX2K device and updates internal
 *         EnableFastMode flag. Fast mode reduces inter-frame delays by using 2 stop bits instead
 *         of tcdrain() blocking.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_SetParameter(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief Driver UART Get Parameter
 *
 * Retrieves current HAL UART communication interface parameter values. Parameters are
 * queried via a type selector that specifies which parameter to read, and the current value
 * is returned in the corresponding value field of the parameter structure.
 *
 * \param[in]       ctx            Pointer to an initialized component (CompId must be Comp_HAL).
 * \param[in,out]   parameter      Pointer to parameter structure:
 *                                 - [in] type: Parameter type selector (see ptx_HAL_DRV_Interface_Parameter_Type_t)
 *                                 - [out] value: Union populated with current parameter value
 *                                   (see ptx_HAL_DRV_Interface_Parameter_Value_t)
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 *
 * \note Supported parameters:
 *       - HAL_DRV_Interface_Parameter_Type_Transmission_Speed: Returns current UART baud rate
 *         stored in ctx->Speed
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_GetParameter(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
