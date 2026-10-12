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
    Module      : Windows UART Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_UART.h

    Description : Implementation of the UART Communication Interface Driver for the Windows Reference Platform using FTDI
*/

/**
 * \addtogroup DRV_WIN_IMPL_INTERFACE_UART
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
#include "ptx_HAL_DRV_Interface_Int.h"
#include "ptx_OSAL.h"
#include "ptx_HAL_Ftd2xx.h"
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
* \brief Communication Interface specific UART definitions (Config / Speed) for Windows Platform.
 * @{
*/

#define PTX_HAL_DRV_INTERFACE_UART_DEFAULT_BAUDRATE                        (115200u)   /**< Default Uart Baudrate, set during device initialisation. */
#define PTX_HAL_DRV_INTERFACE_UART_FT232H_MODEM_STATUS_CTS_OFFSET          (0x10)      /**< Offset of the CTS pin state in the ModemInfo */

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
 * This structure contains all variables required to operate the UART driver from the Windows Platform.
 *
 * \note Integrators must initialise this structure according to their needs! A working example can be found in the
 * corresponding c file.
 */
typedef struct ptx_HAL_DRV_Interface_UART_PortConfig
{
    ptx_HAL_DRV_Interface_Port_Type_t           Type;               /**< Implementation Type of the hardware interface port. */
    uint8_t                                     ID;                 /**< ID of the hardware interface port implementation. */

    char                                       *ModuleDescriptor;   /**< FTDI module descriptor. */
} ptx_HAL_DRV_Interface_UART_PortConfig_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL_DRV_Interface_UART
{
    ptx_Comps_t                                 CompId;             /**< Component ID. */

    ptx_HAL_FTD2XXCtx_t                         *Ftd2xx_ctx;        /**< FTDI context. */
    ptx_HAL_DRV_Interface_UART_PortConfig_t     *Port;              /**< Pointer to Port config. */
    ptx_Log_t                                   *Log;               /**< Pointer to an initialized logger. */
    ptx_OSAL_t                                  *OS;                /**< Pointer to an initialized Osal. */
    uint32_t                                    Speed;              /**< UART Communication Speed. */
} ptx_HAL_DRV_Interface_UART_t;

/**
 * \brief Init parameters to Open Uart Port.
 */
typedef struct ptx_HAL_DRV_Interface_UART_InitParams
{
    ptx_HAL_DRV_Interface_Port_Type_t           PortType;           /**< Implementation Type of the hardware interface port. */
    uint8_t                                     PortID;             /**< ID of the hardware interface port implementation. */

    uint8_t                                     *InterfaceName;     /**< Pointer to the device name. RFU.*/
    uint32_t                                    InterfaceNameLen;   /**< Length of the device name. RFU.*/
    uint32_t                                    Speed;              /**< Speed used by peripheral before being configured by application. */
    void                                        *OS;                /**< Initialized Osal context. */
    ptx_Log_t                                   *Log;               /**< Initialized Logger Context. */
    ptx_HAL_GPIO_t                              *Gpio;              /**< Initialized GPIO Context. */
} ptx_HAL_DRV_Interface_UART_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Driver UART Init
 *
 * Initializes the Windows UART driver by opening the FTDI library context and configuring
 * the component with the provided initialization parameters. The driver supports a single
 * default UART port (FT232H module).
 *
 * \param[in]       ctx             Pointer to the component structure to be initialized.
 *                                  Will be populated with driver context and configuration.
 * \param[in]       initParams      Pointer to initialization parameters containing port type,
 *                                  port ID, communication speed, OS context, logger, and GPIO context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 * \note The Speed field in initParams defaults to PTX_HAL_DRV_INTERFACE_UART_DEFAULT_BAUDRATE
 *       (115200 baud) if set to 0.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Init(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_UART_InitParams_t *initParams);

/**
 * \brief Driver UART DeInit
 *
 * De-initializes the Windows UART driver by closing the FTDI library context and
 * clearing the component structure.
 *
 * \param[in]       ctx             Pointer to an initialized component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_DeInit(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Open
 *
 * Opens and configures the UART interface using the FTDI library. Applies the configured
 * communication speed to the UART port.
 *
 * \param[in]       ctx             Pointer to an initialized component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Open(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Close
 *
 * Closes the UART interface and releases the FTDI device handle.
 *
 * \param[in]       ctx             Pointer to an initialized component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Close(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Tx (Transmit only)
 *
 * Sends data to the UART interface.
 *
 * \param[in]       ctx             Pointer to an initialized component.
 * \param[in]       txBuf           Pointer to array of bytes to be transmitted.
 * \param[in]       txLen           Length of txBuf in bytes.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Tx(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *txBuf, uint32_t txLen);

/**
 * \brief Driver UART Rx (Receive only)
 *
 * Receives data from the UART interface.
 *
 * \param[in]       ctx             Pointer to an initialized component.
 * \param[in]       rxBuf           Pointer to array of bytes to receive data into.
 * \param[in]       rxLen           Length of rxBuf in bytes. Must match the number of bytes expected.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Rx(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *rxBuf, uint32_t rxLen);

/**
 * \brief Driver UART Rx IRQ Level
 *
 * Checks the Clear-To-Send (CTS) modem status line to determine if data is available
 * for reception. The CTS line level is inverted (active low).
 *
 * \param[in]       ctx             Pointer to an initialized component.
 * \param[out]      level           Pointer to level value. Set to 1 if CTS is active (low),
 *                                  0 if CTS is inactive (high).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_RxIrqLevel(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *level);

/**
 * \brief Driver UART Set Parameter
 *
 * Sets a UART communication interface parameter selected by type. Supported parameters include
 * transmission speed and other interface-specific settings.
 *
 * \param[in]       ctx             Pointer to an initialized component.
 * \param[in]       parameter       Pointer to parameter structure containing type and value.
 *                                  See \ref ptx_HAL_DRV_Interface_Parameter_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 * \note Supported parameter types:
 *       - HAL_DRV_Interface_Parameter_Type_Transmission_Speed: Updates UART baud rate
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_SetParameter(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief Driver UART Get Parameter
 *
 * Retrieves a UART communication interface parameter selected by type. Supported parameters include
 * transmission speed and other interface-specific settings.
 *
 * \param[in]       ctx             Pointer to an initialized component.
 * \param[in,out]   parameter       Pointer to parameter structure. Type field specifies which parameter
 *                                  to retrieve; value field is populated with the result.
 *                                  See \ref ptx_HAL_DRV_Interface_Parameter_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 * \note Supported parameter types:
 *       - HAL_DRV_Interface_Parameter_Type_Transmission_Speed: Returns current UART baud rate
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_GetParameter(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief Driver UART Discard Data
 *
 * Discards any data currently held in the UART receive buffer.
 *
 * \param[in]       ctx             Pointer to an initialized component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_DiscardData(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Cancel
 *
 * Cancels the current UART operation (transmission or reception).
 *
 * \param[in]       ctx             Pointer to an initialized component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Cancel(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Clear Cancel
 *
 * Cancels the current UART operation and clears the receive buffer.
 *
 * \param[in]       ctx             Pointer to an initialized component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_ClearCancel(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Init Comport
 *
 * Initializes the COM port for UART communication. This is an internal helper function
 * called during driver initialization to configure the serial port parameters.
 *
 * \param[in]       ctx             Pointer to the component structure.
 * \param[in]       initParams      Pointer to initialization parameters containing port configuration.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_InitComPort(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_UART_InitParams_t *initParams);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
