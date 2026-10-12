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
    Module      : Linux SPI Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_SPI.h

    Description : Implementation of the SPI Communication Interface Driver for the Linux Reference Platform
*/

/**
 * \addtogroup DRV_LIN_IMPL_INTERFACE_SPI
 *
 * @{
 */

#if !defined(PTX_HAL_DRV_INTERFACE_SPI_H)
#define PTX_HAL_DRV_INTERFACE_SPI_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Status.h"
#include "ptx_Components.h"
#include "ptx_Log.h"
#include "ptx_HAL_DRV_Interface_Int.h"
#include "ptx_OSAL.h"
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
* \brief Default device specific SPI definitions (Config / Speed) for Linux.
 * @{
*/
#define PTX_HAL_DRV_INTERFACE_SPI_DEFAULT_SPEED           (1000000u)      /**< Default SPI speed. */
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
 * This structure contains all variables required to operate the SPI driver from the Linux Platform.
 *
 * \note Integrators must initialise this structure according to their needs! A working example can be found in the
 * corresponding c file.
 */
typedef struct ptx_HAL_DRV_Interface_SPI_PortConfig
{
    ptx_HAL_DRV_Interface_Port_Type_t   Type;           /**< Implementation Type of the hardware interface port. */
    uint8_t                             ID;             /**< ID of the hardware interface port implementation. */

    uint32_t                            PortNr;         /**< SPI Port Number. */
    int                                 PortFd;         /**< SPI Port file descriptor. */

    int                                 IrqFd;          /**< SPI IRQ GPIO pin file descriptor. */
    int                                 IrqFdCancel;    /**< SPI IRQ GPIO pin cancel file descriptor. */

    uint32_t                            IrqPin;         /**< SPI IRQ Pin Number of the predefined gpiochip device. */

    uint8_t                             *TxData;        /**< Tx-Buffer used for data transmissions. */
    uint32_t                            TxDataLen;      /**< Size of Tx-Buffer used for data transmissions. */

    uint8_t                             *RxData;        /**< Rx-Buffer used for data transmissions. */
    uint32_t                            RxDataLen;      /**< Size of Rx-Buffer used for data transmissions. */
} ptx_HAL_DRV_Interface_SPI_PortConfig_t;


/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL_DRV_Interface_SPI
{
    ptx_Comps_t                             CompId;         /**< Component ID. */

    ptx_HAL_DRV_Interface_SPI_PortConfig_t  *Port;          /**< SPI Default internal parameters */
    ptx_Log_t                               *Log;           /**< Pointer to an initialized logger. */
    ptx_OSAL_t                              *OS;            /**< Pointer to an initialized Osal. */
    uint32_t                                Speed;          /**< SPI Communication Speed */

} ptx_HAL_DRV_Interface_SPI_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Interface_SPI_InitParams
{
    ptx_HAL_DRV_Interface_Port_Type_t   PortType;       /**< Implementation Type of the hardware interface port. */
    uint8_t                             PortID;         /**< ID of the hardware interface port implementation. */

    uint32_t                            Speed;          /**< SPI Communication Speed. */
    ptx_Log_t                           *Log;           /**< Pointer to an initialized logger. */
    void                                *OS;            /**< Pointer to initialized Osal context. */
    ptx_HAL_GPIO_t                      *Gpio;          /**< Initialized GPIO Context. */
} ptx_HAL_DRV_Interface_SPI_InitParams_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
* \brief Initialize the SPI interface component.
*
* Searches for an SPI port configuration matching the specified PortType and PortID.
* Currently one default port configuration is provided, see gDrvSpiConfigs.
* Initializes the component context with the configuration. Sets the SPI
* communication speed (or uses default if not specified) and associates the logger
* and OS abstraction layer with the component.
*
* \param[out]      ctx             Pointer to SPI interface component context to initialize.
* \param[in]       initParams      Pointer to initialization parameters.
*
* \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Init (ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_SPI_InitParams_t *initParams);

/**
* \brief De-initialize the SPI interface component.
*
* Closes any open SPI resources associated with the component, clears the component
* context memory, and marks the component as uninitialized. Must only be called on
* a previously initialized component.
*
* \param[in,out]       ctx             Pointer to initialized SPI interface component context.
*
* \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
*
* \note Calls ptx_HAL_DRV_Interface_SPI_Close() internally before clearing context.
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_DeInit (ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
* \brief Open and configure the SPI interface.
*
* Opens the SPI device file and configures it with the following parameters:
* - SPI mode: MODE_0
* - Bits per word: 8
* - Speed: Uses configured speed or default if lower than minimum
* After SPI configuration, opens and configures the IRQ GPIO line.
*
* \param[in]       ctx             Pointer to initialized SPI interface component.
*
* \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
*
* \note Opens device file with flags: O_RDWR, O_NOCTTY, O_NDELAY, O_NONBLOCK
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Open(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
* \brief Close and release the SPI interface and IRQ resources.
*
* Closes all associated file descriptors: SPI port, IRQ GPIO, and IRQ cancel file descriptors.
* Close errors are silently ignored.
*
* \param[in]       ctx             Pointer to initialized SPI interface component.
*
* \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Close(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
* \brief Queue data for SPI transmission.
*
* Queues the provided transmit buffer for transmission over the SPI interface.
* The actual transmission is performed asynchronously. Only queues data if no
* transmission is already pending (TxData is NULL) and txLen is greater than 0.
*
* \param[in]       ctx             Pointer to initialized SPI interface component.
* \param[in]       txBuf           Pointer to array of bytes to be transmitted.
* \param[in]       txLen           Length of txBuf in bytes. If 0, no data is queued.
*
* \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
*
* \note This function only queues the data; actual transmission occurs asynchronously.
* \note txBuf must remain valid until transmission completes.
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Tx(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *txBuf, uint32_t txLen);


/**
* \brief Queue buffer for SPI reception.
*
* Queues the provided receive buffer to store data received over the SPI interface.
* The actual reception is performed asynchronously. The buffer size (rxLen) must be
* sufficient to hold the expected number of bytes.
*
* \param[in]       ctx             Pointer to initialized SPI interface component.
* \param[in]       rxBuf           Pointer to array of bytes to store received data.
* \param[in]       rxLen           Size of rxBuf in bytes. Must be at least as large
*                                  as the number of bytes expected to be received.
*
* \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
*
* \note This function only queues the buffer; actual reception occurs asynchronously.
* \note rxBuf must remain valid until reception completes.
* \note rxLen must match the actual number of bytes that will be received.
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Rx(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *rxBuf, uint32_t rxLen);

/**
* \brief Prepare for SPI transaction (NSS select).
*
* Validates the component and prepares for an SPI transaction. This function should be
* called before queuing Tx and Rx buffers. Does not actively control the NSS pin;
* actual NSS control and SPI communication occurs in ptx_HAL_DRV_Interface_SPI_ResetNss().
*
* \param[in]       ctx             Pointer to initialized SPI interface component.
*
* \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
*
* \note Call this function before ptx_HAL_DRV_Interface_SPI_Tx() and ptx_HAL_DRV_Interface_SPI_Rx().
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetNss(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
* \brief Execute SPI transaction and deselect slave (NSS reset).
*
* Executes the queued SPI transaction (transmit and receive) if Tx data is pending.
* Performs a full-duplex SPI transfer using the queued Tx and Rx buffers, then clears
* the buffers. If no Tx data is queued, returns SUCCESS without performing any transfer.
*
* \param[in]       ctx             Pointer to initialized SPI interface component.
*
* \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
*
* \note Tx and Rx buffers must be queued via ptx_HAL_DRV_Interface_SPI_Tx() and
*       ptx_HAL_DRV_Interface_SPI_Rx() before calling this function.
* \note Clears Tx and Rx buffer pointers after successful transaction.
* \note Logs Tx/Rx data unless PTX_ENABLE_TFCI is defined.
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_ResetNss(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
* \brief Read the current level of the SPI Rx IRQ GPIO pin.
*
* Reads and returns the current state of the configured IRQ GPIO pin.
*
* \param[in]       ctx             Pointer to initialized SPI interface component.
* \param[out]      level           Pointer to uint8_t to store the IRQ pin level.
*                                  0 = pin is low, 1 = pin is high.
*
* \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
*
* \note This function performs a non-blocking read of the GPIO pin state.
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_RxIrqLevel(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *level);

/**
* \brief Set SPI HAL device parameter.
*
* Sets a configurable SPI HAL parameter specified by the parameter type. Currently supports
* transmission speed configuration. Other parameter types (I2C, UART) return NOT_AVAILABLE
* as they are not applicable to SPI.
*
* \param[in]       ctx            Pointer to initialized SPI interface component.
* \param[in]       parameter      Pointer to parameter struct containing:
*                                 - type: Parameter type to set (see ptx_HAL_DRV_Interface_Parameter_Type_t)
*                                 - value: Parameter value union (see ptx_HAL_DRV_Interface_Parameter_Value_t)
*
* \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
*
* \note Currently only HAL_DRV_Interface_Parameter_Type_Transmission_Speed is supported.
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetParameter(ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
* \brief Get SPI HAL device parameter.
*
* Retrieves the current value of a configurable SPI HAL parameter specified by the parameter type.
* Currently supports reading transmission speed. Other parameter types (I2C, UART) return NOT_AVAILABLE
* as they are not applicable to SPI.
*
* \param[in]       ctx            Pointer to initialized SPI interface component.
* \param[in,out]   parameter      Pointer to parameter struct:
*                                 - [in] type: Parameter type to retrieve (see ptx_HAL_DRV_Interface_Parameter_Type_t)
*                                 - [out] value: Parameter value union populated with current value
*                                 (see ptx_HAL_DRV_Interface_Parameter_Value_t)
*
* \return Status, indicating whether the operation was successful. See \ref ptx_Status_t\.
*
* \note Currently only HAL_DRV_Interface_Parameter_Type_Transmission_Speed is supported.
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_GetParameter(ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
