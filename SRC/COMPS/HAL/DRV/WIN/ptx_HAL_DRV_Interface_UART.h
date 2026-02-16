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
 * \brief Main Component Struture
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
 * Initializes the Windows Default Uart driver according to the given parameters.
 *
 * \param[in]       ctx             Pointer to pointer of the component (The address of the component will be replaced by one allocated by OS)
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Init(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_UART_InitParams_t *initParams);

/**
 * \brief Driver UART DeInit
 *
 * De-Initializes the Windows Default Uart driver.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_DeInit(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Open
 *
 * Opens the Windows UART Default interface and configures it according to the parameters in \see ptx_HAL_DRV_Interface_UART.Port.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Open(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Close
 *
 * Closes the Windows UART Default interface.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Close(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Activate Rx IRQ.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_ActivateRxIrq(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Deactivate Rx IRQ.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_DeactivateRxIrq(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver Rx IRQ Level Callback Datatype
 *
 * Checks if any bytes have been received since the last Reading operation.
 *
 * \param[in]       ctx          Pointer to component.
 * \param[out]      level        Pointer to level value.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_RxIrqLevel(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *level);

/**
 * \brief Driver UART Wait for IRQ to be triggered
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_WaitForIRQ(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART TRx
 *
 * Sends and/or receives data to/from the UART interface, to communicate with an endpoint.
 *
 * \param[in]       ctx           Pointer to component.
 * \param[in]       txBuf         Array of bytes to be transmitted.
 * \param[in]       txLen         Length of \see tx.
 * \param[in]       rxBuf         Array of bytes to be received.
 * \param[in]       rxLen         Length of \see rxBuf. Must match the bytes received!
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_TRx(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *txBuf, uint32_t *txLen, uint8_t *rxBuf, uint32_t *rxLen);

/**
 * \brief Driver UART Tx
 *
 * Sends data to the UART interface, to communicate with an endpoint.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       txBuf           Array of bytes to be transmitted.
 * \param[in]       txLen           Length of \see txBuf.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Tx(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *txBuf, uint32_t txLen);

/**
 * \brief Driver UART Rx
 *
 * Receives data from the UART interface.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       rxBuf           Array of bytes to be received.
 * \param[in]       rxLen           Length of \see rxBuf. Must match the bytes received!
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Rx(ptx_HAL_DRV_Interface_UART_t *ctx, uint8_t *rxBuf, uint32_t rxLen);

/**
 * \brief Driver UART Set Parameter function.
 *
 * Sets a HAL UART Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_SetParameter(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief Driver UART Get Parameter function.
 *
 * Gets a HAL UART Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      Parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_GetParameter(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Driver UART Discard Data
 *
 * Discards data currently held in buffer.
 *
 * \param[in]       ctx         Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_DiscardData(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Cancel
 *
 * Cancels the current Uart event.
 *
 * \param[in]       ctx         Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_Cancel(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Clear Cancel
 *
 * Cancels the Uart event and clears the buffer.
 *
 * \param[in]       ctx         Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_ClearCancel(ptx_HAL_DRV_Interface_UART_t *ctx);

/**
 * \brief Driver UART Init Comport
 *
 * Initializes the comport to use for Uart communication.
 *
 * \param[in]       ctx         Pointer to component.
 * \param[in]       initParams      Pointer to Parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_UART_InitComPort(ptx_HAL_DRV_Interface_UART_t *ctx, ptx_HAL_DRV_Interface_UART_InitParams_t *initParams);

/**
 * \brief Driver UART Baudrate conversion
 *
 * Converts int to Windows internal speed definitions.
 *
 * \param[in]       rawBaud         speed as integer
 *
 * \return Speed, passable to WinAPI.
 */
uint32_t ptx_HAL_DRV_Interface_UART_UartBaudrateConv(uint32_t rawBaud);



#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
