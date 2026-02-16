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
    Module      : HAL Communication Interface for UART
    File        : ptx_HAL_Interface_UART.h

    Description : Implementation of the UART Communication Interface
*/

/**
 * \addtogroup DEV_INTERFACE_UART
 *
 * @{
 */

#if !defined(PTX_HAL_INTERFACE_UART_H)
#define PTX_HAL_INTERFACE_UART_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_HAL_DRV_Interface_UART.h"

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
 * \brief Async Receive Callback Type
 *
 * This callback must be implemented if async communication from the PTX2xxR/W to the host MCU is required.
 * Usually this callback will be implemented by the HIP layer.
 *
 * \param[in]   ctx             Initialised context for the callback
 */
typedef void (*ptx_HAL_Interface_UART_AsyncRxCb_t)(void *ctx);

/**
 * \brief Async Rx Context
 *
 * This context must for the requirements for the \ref ptx_HAL_Interface_AsyncRxCb_t callback!
 *
 * \note This callback must be at least \ref ptx_HAL_t!
 */
typedef void *ptx_HAL_Interface_UART_AsyncRxCbCtx_t;

/**
 * \brief Main Component Struture
 */
typedef struct ptx_HAL_Interface_UART
{
    ptx_Comps_t                                  CompId;         /**< Component ID. */
    ptx_HAL_DRV_Interface_UART_t                 Drv;            /**< Hardware Interface Driver. */

    uint32_t                                     CfgBaudrate;    /**< Initial Configured BaudRate. */

    ptx_HAL_Interface_UART_AsyncRxCb_t           AsyncRxCb;      /**< Async Receive Callback. */
    ptx_HAL_Interface_UART_AsyncRxCbCtx_t        AsyncRxCbCtx;   /**< Async Receive Callback Context. */
} ptx_HAL_Interface_UART_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_Interface_UART_InitParams
{
    uint8_t                             *InterfaceName;       /**< Device name. */
    uint32_t                            InterfaceNameLen;     /**< Device name string length. */

    ptx_HAL_DRV_Interface_Port_Type_t   PortType;             /**< Implementation Type of the hardware interface port. */
    uint8_t                             PortID;               /**< ID of the hardware interface port implementation. */

    uint32_t                            Speed;                /**< Communication interface speed. */
    ptx_Log_t                           *Log;                 /**< Initialised Logger Context. */
    ptx_HAL_GPIO_t                      *Gpio;                /**< Initialized GPIO Context. */
#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
    void                                *OS;                  /**< Pointer to initialized Osal context. */
#endif
} ptx_HAL_Interface_UART_InitParams_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief UART Communication Interface Initialization.
 *
 * \param[in]       ctx         Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_Init (ptx_HAL_Interface_UART_t *ctx, ptx_HAL_Interface_UART_InitParams_t *initParams);

/**
 * \brief UART Communication Interface De-Initialization.
 *
 * \param[in]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_DeInit (ptx_HAL_Interface_UART_t *ctx);

/**
 * \brief UART Communication Interface Open.
 *
 * Opens the initialised device.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_Open (void *ctx);

/**
 * \brief UART Communication Interface Close.
 *
 * Closes the initialised device.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_Close (void *ctx);

/**
 * \brief UART Communication Interface TRx.
 *
 * Transceive function for data. First the data in the \see txBuf array is transmitted. Second the data is received
 * in the \see rxBuf array. If \see txBuf is left empty, the function will not transmit any data. If \see rxBuf is left
 * empty, the function will not receive any data.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       txBuf           Pointer to array of data to be transmitted.
 * \param[in]       txLen           Length of each of the arrays in \see txBuf.
 * \param[in]       txBufferCnt     Number of buffers to be transmitted.
 * \param[inout]    rxBuf           Pointer to array of data to be received. The array must have the right size!
 * \param[inout]    rxLen           Length of each array to be received.
 * \param[in]       rxBufferCnt     Number of buffers to be transmitted.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_TRx (void *ctx, uint8_t *txBuf[], uint32_t txLen[], uint32_t txBufferCnt,
                                         uint8_t *rxBuf[], uint32_t rxLen[], uint32_t rxBufferCnt);

/**
 * \brief UART Communication Interface Tx.
 *
 * Transfer function for data. The data in the \see txBuf array is transmitted.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       txBuf           Array of bytes to be transmitted.
 * \param[in]       txLen           Length of \see txBuf.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_Tx (void *ctx, uint8_t *txBuf, uint32_t txLen);

/**
 * \brief UART Communication Interface Rx.
 *
 * Receive function for data. The data in the \see rxBuf array is transmitted.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       rxBuf           Array of bytes to be received.
 * \param[in]       rxLen           Length of \see rxBuf.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_Rx (void *ctx, uint8_t *rxBuf, uint32_t rxLen);

/**
 * \brief UART Communication Interface Enable Async Rx
 *
 * Enables the async rx functionality.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_EnableAsyncRx (void *ctx);

/**
 * \brief UART Communication Interface Disable Async Rx
 *
 * Disables the async rx functionality.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_DisableAsyncRx (void *ctx);

/**
 * \brief UART Communication Interface Register Async Rx Callback.
 *
 * Registers the callback \see rxCb. The callback is called if \see ptx_HAL_Interface_AsyncRx detects an transfer.
 * \see rxCbCtx is the corresponding context for the callback to work with.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       rxCb            Callback function.
 * \param[in]       rxCbCtx         Callback context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_RegAsyncRxCb(void *ctx, ptx_HAL_Interface_UART_AsyncRxCb_t rxCb, ptx_HAL_Interface_UART_AsyncRxCbCtx_t rxCbCtx);

/**
 * \brief UART Communication Interface Rx Pending.
 *
 * Checks if the attached device wants to communicate with the host. The device signals this by either an
 * GPIO pin, or RTS, etc.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       rxPending       Pointer to store the information if a receive operation is pending.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_RxPending(void* ctx, uint8_t* rxPending);

/**
 * \brief UART Communication Interface Async Rx Function. NoN Blocking.
 *
 * Checks if the attached device wants to communicate with the host. The device signals this by either an
 * GPIO pin, or RTS, etc. If this function detects the signal, it executes a \see ptx_HAL_Interface_AsyncRxCb_t
 * callback.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_AsyncRxNonBlocking(void *ctx);

/**
 * \brief UART Communication Interface Async Rx Function. Blocking.
 *
 * Checks if the attached device wants to communicate with the host. The device signals this by either an
 * GPIO pin, or RTS, etc. If this function detects the signal, it executes a \see ptx_HAL_Interface_AsyncRxCb_t
 * callback.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_AsyncRxBlocking(void *ctx);

/**
 * \brief HAL UART Communication Interface Set Parameter function.
 *
 * Sets a HAL UART Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      Parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_SetParameter(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief HAL UART Communication Interface Get Parameter function.
 *
 * Gets a HAL UART Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      Parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_UART_GetParameter(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

