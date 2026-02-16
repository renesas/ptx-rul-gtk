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
    Module      : HAL Communication Interface for SPI
    File        : ptx_HAL_Interface_SPI.h

    Description : Implementation of the SPI Communication Interface
*/

/**
 * \addtogroup DEV_INTERFACE_SPI
 *
 * @{
 */

#if !defined(PTX_HAL_INTERFACE_SPI_H)
#define PTX_HAL_INTERFACE_SPI_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_DRV_Interface_SPI.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \brief HAL GPIO Forward declaration.
 */
typedef struct ptx_HAL_GPIO ptx_HAL_GPIO_t;

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Async Receive Callback Type
 *
 * This callback must be implemented if async communication from the PTX2xxR/W to the host MCU is required.
 * Typically this callback will be implemented by the NSC layer in a threaded application.
 *
 * \param[in]   ctx             Initialised context for the callback
 */
typedef void (*ptx_HAL_Interface_SPI_AsyncRxCb_t)(void *ctx);

/**
 * \brief Async Rx Context
 *
 * This context must for the requirements for the \ref ptx_HAL_Interface_AsyncRxCb_t callback!
 *
 * \note This callback must be at least \ref ptx_HAL_t!
 */
typedef void *ptx_HAL_Interface_SPI_AsyncRxCbCtx_t;

/**
 * \brief Main Component Struture
 */
typedef struct ptx_HAL_Interface_SPI
{
    ptx_Comps_t                                 CompId;         /**< Component ID. */
    ptx_HAL_DRV_Interface_SPI_t                 Drv;            /**< Hardware Interface Driver. */

    ptx_HAL_Interface_SPI_AsyncRxCb_t           AsyncRxCb;      /**< Async Receive Callback. */
    ptx_HAL_Interface_SPI_AsyncRxCbCtx_t        AsyncRxCbCtx;   /**< Async Receive Callback Context. */
} ptx_HAL_Interface_SPI_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_Interface_SPI_InitParams
{
    ptx_HAL_DRV_Interface_Port_Type_t   PortType;           /**< Implementation Type of the hardware interface port. */
    uint8_t                             PortID;             /**< ID of the hardware interface port implementation. */

    uint32_t                            Speed;              /**< Communication interface speed. */
    ptx_Log_t                           *Log;               /**< Initialised Logger Context. */
    ptx_HAL_GPIO_t                      *Gpio;              /**< Initialized GPIO Context. */
#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
    void                                *OS;                /**< Pointer to initialized Osal context. */
#endif
} ptx_HAL_Interface_SPI_InitParams_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief SPI Communication Interface Initialization.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_Init (ptx_HAL_Interface_SPI_t *ctx, ptx_HAL_Interface_SPI_InitParams_t *initParams);

/**
 * \brief SPI Communication Interface De-Initialization.
 *
 * \param[in]       ctx   Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_DeInit (ptx_HAL_Interface_SPI_t *ctx);

/**
 * \brief SPI Communication Interface Open.
 *
 * Opens the initialised device.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_Open (void *ctx);

/**
 * \brief SPI Communication Interface Close.
 *
 * Closes the initialised device.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_Close (void *ctx);

/**
 * \brief SPI Communication Interface TRx.
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
ptx_Status_t ptx_HAL_Interface_SPI_TRx (void *ctx, uint8_t *txBuf[], uint32_t txLen[], uint32_t txBufferCnt,
                                        uint8_t *rxBuf[], uint32_t rxLen[], uint32_t rxBufferCnt);

/**
 * \brief SPI Communication Interface Enable Async Rx
 *
 * Enables the async rx functionality.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_EnableAsyncRx (void *ctx);

/**
 * \brief SPI Communication Interface Disable Async Rx
 *
 * Disables the async rx functionality.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_DisableAsyncRx (void *ctx);

/**
 * \brief SPI Communication Interface Register Async Rx Callback.
 *
 * Registers the callback \see rxCb. The callback is called if \see ptx_HAL_Interface_AsyncRx detects an transfer.
 * \see rxCbCtx is the corresponding context for the callback to work with.
 *
 * Depending on the intended usage of this function, it may not be implemented!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       rxCb            Callback function.
 * \param[in]       rxCbCtx         Callback context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_RegAsyncRxCb(void *ctx, ptx_HAL_Interface_SPI_AsyncRxCb_t rxCb, ptx_HAL_Interface_SPI_AsyncRxCbCtx_t rxCbCtx);

/**
 * \brief SPI Communication Interface Rx Pending.
 *
 * Checks if the attached device wants to communicate with the host. The device signals this by either an
 * GPIO pin, or RTS, etc.
 *
 * Depending on the intended usage of this function, it may not be implemented!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       rxPending       Pointer to store the information if a receive operation is pending.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_RxPending(void* ctx, uint8_t* rxPending);

/**
 * \brief SPI Communication Interface Async Rx Function. NoN Blocking.
 *
 * Checks if the attached device wants to communicate with the host. The device signals this by either an
 * GPIO pin, or RTS, etc. If this function detects the signal, it executes a \see ptx_HAL_Interface_AsyncRxCb_t
 * callback.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_AsyncRxNoNBlocking(void *ctx);

/**
 * \brief SPI Communication Interface Async Rx Function. Blocking.
 *
 *
 * Checks if the attached device wants to communicate with the host. The device signals this by either an
 * GPIO pin, or RTS, etc. If this function detects the signal, it executes a \see ptx_HAL_Interface_AsyncRxCb_t
 * callback.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_AsyncRxBlocking(void *ctx);

/**
 * \brief HAL SPI Communication Interface Set Parameter function.
 *
 * Sets a HAL SPI Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      Parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_SetParameter(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief HAL SPI Communication Interface Get Parameter function.
 *
 * Gets a HAL SPI Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      Parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SPI_GetParameter(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

