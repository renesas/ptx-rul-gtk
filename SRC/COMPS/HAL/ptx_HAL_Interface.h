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
    Module      : HAL Communication Interface
    File        : ptx_HAL_Interface.h

    Description : Implementation of the HAL Communication Interface for physical interfaces.
*/

/**
 * \addtogroup HAL_IF_COMM
 * @{
 */

#if !defined(PTX_HAL_INTERFACE_H)
#define PTX_HAL_INTERFACE_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stddef.h>
#include "ptx_Status.h"
#include "ptx_Components.h"
#include "ptx_HAL_GPIO.h"
#include "ptx_HAL_DRV_Interface_Int.h"
#include "ptx_Log.h"

#if defined(PTX_ENABLE_SPI)
    /* SPI */
    #include "ptx_HAL_Interface_SPI.h"
#endif

#if defined(PTX_ENABLE_UART)
    /* UART */
    #include "ptx_HAL_Interface_UART.h"
#endif

#if defined(PTX_ENABLE_I2C)
    /* I2C */
    #include "ptx_HAL_Interface_I2C.h"
#endif

#ifdef PTX_ENABLE_I3C
    /* I3C */
    #include "ptx_HAL_Interface_I3C.h"
#endif


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
 * \brief Async Receive Callback Datatype
 *
 * This callback must be implemented if async communication from the PTX2xxR/W to the host MCU is required.
 * Usually this callback will be implemented by the HIP layer.
 *
 * \param[in]   ctx             Initialised context for the callback
 */
typedef void (*ptx_HAL_Interface_AsyncRxCb_t)(void *ctx);

/**
 * \brief Async Rx Context
 *
 * This context must for the requirements for the \ref ptx_HAL_Interface_AsyncRxCb_t callback!
 *
 * \note This callback must be at least \ref ptx_HAL_t!
 */
typedef void *ptx_HAL_Interface_AsyncRxCbCtx_t;

/**
 * \brief HAL Communication Interface Open Callback Datatype
 *
 * The callback is used to open the device.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbOpen_t)(void *ctx);

/**
 * \brief HAL Communication Interface Close Callback Datatype
 *
 * The callback is used to close the device.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbClose_t)(void *ctx);

/**
 * \brief HAL Communication Interface TRx Callback Datatype
 *
 * Transceive function for data callback. First the data in the txBuf array is transmitted. Second the data is received
 * in the rxBuf array. If txBuf is left empty, the function will not transmit any data. If rxBuf is left
 * empty, the function will not receive any data.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       txBuf           Pointer to array of data to be transmitted.
 * \param[in]       txLen           Length of each of the arrays in txBuf.
 * \param[in]       txBufferCnt     Number of buffers to be transmitted.
 * \param[inout]    rxBuf           Pointer to array of data to be received. The array must have the right size!
 * \param[inout]    rxLen           Length of each array to be received.
 * \param[in]       rxBufferCnt     Number of buffers to be transmitted.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbTRx_t)(void *ctx, uint8_t *txBuf[], uint32_t txLen[], uint32_t txBufferCnt,
                                                  uint8_t *rxBuf[], uint32_t rxLen[], uint32_t rxBufferCnt);

/**
 * \brief HAL Communication Interface Tx Callback Datatype
 *
 * Callback to transmit the given data in tx.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       txBuf           Array of data to be transmitted.
 * \param[in]       txLen           Number of bytes to be transmitted.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbTx_t)(void *ctx, uint8_t *txBuf, uint32_t txLen);

/**
 * \brief HAL Communication Interface Rx Callback Datatype
 *
 * Callback to receive the given data in rx.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[inout]    rxBuf           Array with the length of data to be received.
 * \param[in]       rxLen           Number of bytes to be received.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbRx_t)(void *ctx, uint8_t *rxBuf, uint32_t rxLen);

/**
 * \brief HAL Communication Interface Enable Async Rx Callback
 *
 * Callback to enable the async rx functionality.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbEnableAsyncRx_t)(void *ctx);

/**
 * \brief HAL Communication Interface disable Async Rx Callback Datatype
 *
 * Callback to disable the async rx functionality.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbDisableAsyncRx_t)(void *ctx);

/**
 * \brief HAL Communication Interface Register Async Rx Callback Datatype
 *
 * Callback to register the callback rxCb. The callback is called if ptx_HAL_Interface_AsyncRx detects an transfer.
 * rxCbCtx is the corresponding context for the callback to work with.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       rxCb            Callback function.
 * \param[in]       rxCbCtx         Callback context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbRegAsyncRxCb_t)(void *ctx, ptx_HAL_Interface_AsyncRxCb_t rxCb, ptx_HAL_Interface_AsyncRxCbCtx_t rxCbCtx);

/**
 * \brief HAL Communication Interface Async Rx NoNBlocking Function Datatype
 *
 * Callback checks if the attached device wants to communicate with the host. The device signals this by either an
 * GPIO pin, or RTS, etc. If this function detects the signal, it executes a ptx_HAL_Interface_AsyncRxCb_t
 * callback.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbAsyncRxNonBlocking_t)(void *ctx);

/**
 * \brief HAL Communication Interface Async Rx Blocking Function Datatype
 *
 * This function waits for the IRQ to be triggered and when that happens it executes a ptx_HAL_Interface_AsyncRxCb_t
 * callback.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbAsyncRxBlocking_t)(void *ctx);


/**
 * \brief HAL Communication Interface Async Rx Pending Callback Datatype
 *
 * Callback checks if the attached device wants to communicate with the host. The device signals this by either an
 * GPIO pin, or RTS, etc.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       rxPending       Pointer to store the information if a receive operation is pending.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t(*ptx_HAL_Interface_CbRxPending_t)(void* ctx, uint8_t* rxPending);

/**
 * \brief HAL Communication Interface Set Parameter Callback function.
 *
 * This callback sets a HAL Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      Parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbSetParameter_t)(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief HAL Communication Interface Get Parameter Callback function.
 *
 * This callback gets a HAL Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      Parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_HAL_Interface_CbGetParameter_t)(void *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief HAL Communication Interface Types
 */
typedef enum ptx_HAL_Interface_Type
{
#if defined(PTX_ENABLE_SPI)
    HAL_Interface_Type_SPI = 0,    /**< SPI Communication Interface Type. */
#endif

#if defined(PTX_ENABLE_UART)
    HAL_Interface_Type_UART = 1,   /**< UART Communication Interface Type. */
#endif

#if defined(PTX_ENABLE_I2C)
    HAL_Interface_Type_I2C = 2,    /**< I2C Communication Interface Type. */
#endif

#if defined(PTX_ENABLE_I3C)
    HAL_Interface_Type_I3C = 3,    /**< I3C Communication Interface Type. */
#endif
    HAL_Interface_Type_MAX = 4,    /**< MAX value of Enum. */
} ptx_HAL_Interface_Type_t;


/**
 * \brief HAL Dev Structure
 * \note Integrators must implement the callbacks according to their specification!
 */
typedef struct ptx_HAL_Interface
{
    ptx_Comps_t                                   CompId;             /**< Component ID */

    ptx_HAL_Interface_Type_t                      Type;               /**< Type of the hardware interface. */

#if defined(PTX_ENABLE_SPI)
    ptx_HAL_Interface_SPI_t                       SpiInterface;       /**< SPI handware interface. */
#endif

#if defined(PTX_ENABLE_I2C)
    ptx_HAL_Interface_I2C_t                       I2cInterface;       /**< UART handware interface. */
#endif

#if defined(PTX_ENABLE_UART)
    ptx_HAL_Interface_UART_t                      UartInterface;       /**< I2C handware interface. */
#endif

#if defined(PTX_ENABLE_I3C)
    ptx_HAL_Interface_I3C_t                       I3cInterface;       /**< I3C handware interface. */
#endif

    ptx_HAL_Interface_CbOpen_t                    Open;               /**< Device Open Callback */
    ptx_HAL_Interface_CbClose_t                   Close;              /**< Device Close Callback */
    ptx_HAL_Interface_CbTRx_t                     TRx;                /**< Device Transceive Callback */
    ptx_HAL_Interface_CbTx_t                      Tx;                 /**< Device Transmit Callback */
    ptx_HAL_Interface_CbRx_t                      Rx;                 /**< Device Receive Callback */
    ptx_HAL_Interface_CbEnableAsyncRx_t           EnableAsyncRx;      /**< Device Enable Async Rx Callback */
    ptx_HAL_Interface_CbDisableAsyncRx_t          DisableAsyncRx;     /**< Device Enable Async Rx Callback */
    ptx_HAL_Interface_CbRegAsyncRxCb_t            RegAsyncRxCb;       /**< Device Async Rx callback register function with context Callback */
    ptx_HAL_Interface_CbAsyncRxNonBlocking_t      AsyncRxNoNBlocking; /**< Device Async Rx Callback ( if IRQ triggered ) */
    ptx_HAL_Interface_CbAsyncRxBlocking_t         AsyncRxBlocking;    /**< Device Async Rx Callback ( by blocking for IRQ to be triggered ) */
    ptx_HAL_Interface_CbRxPending_t               RxPending;          /**< Device Rx Pending Callback */
    ptx_HAL_Interface_CbSetParameter_t            SetParameter;       /**< Device Set Parameter Callback */
    ptx_HAL_Interface_CbGetParameter_t            GetParameter;       /**< Device Get Parameter Callback */
} ptx_HAL_Interface_t;


/**
 * \brief HAL Dev Initialisation Parameters
 */
typedef struct ptx_HAL_Interface_InitParams
{
    uint8_t                                     *Name;                  /**< Pointer to the hardware interface name. */
    uint32_t                                    NameLen;                /**< Length of the hardware interface name. */
    ptx_HAL_Interface_Type_t                    Type;                   /**< Type of the hardware interface. */
    ptx_HAL_DRV_Interface_Port_Type_t           PortType;               /**< Implementation Type of the hardware interface port. */
    uint8_t                                     PortID;                 /**< ID of the hardware interface port implementation. */
    uint32_t                                    Speed;                  /**< Communication interface speed. The parameter is used only for testing
                                                                            and is not supposed to be used by the final application. */

    ptx_Log_t                                   *Log;                   /**< Initialised Logger Context */
    ptx_HAL_GPIO_t                              *Gpio;                  /**< Initialized GPIO Context. */
#if defined(PTX_APP_DIVERSITY_WIN) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
    void                                        *Osal;                  /**< Pointer to OSAL context */
#endif
} ptx_HAL_Interface_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Communication Interface Initialization.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_Init (ptx_HAL_Interface_t *ctx, ptx_HAL_Interface_InitParams_t *initParams);

/**
 * \brief Communication Interface De-Initialization.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_DeInit (ptx_HAL_Interface_t *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

