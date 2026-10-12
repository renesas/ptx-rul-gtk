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
    Module      : MCU-RA4M2 I2C Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_I2C.h

    Description : Implementation of the I2C Communication Interface Driver for the MCU-RA4M2 Reference Platform
*/

/**
 * \addtogroup DRV_RA4M2_IMPL_INTERFACE_I2C
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
#include "hal_data.h"
#include "ptx_HAL_DRV_Interface_Int.h"
#include "ptx_Log.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
* \brief Communication Interface specific I2C definitions (Address / Speed) for Renesas RA4M2 MCU.
 * @{
*/
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4C          (uint8_t)(0x4Cu)        /**< I2C Address 0x4C -> ADDR1 = 0 and ADDR0 = 0. */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4D          (uint8_t)(0x4Du)        /**< I2C Address 0x4D -> ADDR1 = 0 and ADDR0 = 1. */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4E          (uint8_t)(0x4Eu)        /**< I2C Address 0x4E -> ADDR1 = 1 and ADDR0 = 0. */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4F          (uint8_t)(0x4Fu)        /**< I2C Address 0x4F -> ADDR1 = 1 and ADDR0 = 1. */

#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_DEFAULT       (100000u)               /**< 100  kHz. */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_FAST          (400000u)               /**< 400  kHz. */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_FASTPLUS      (1000000u)              /**< 1    MHz. */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_HIGHSPEED     (3400000u)              /**< 3.4  MHz. */
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
 * \brief Transceiver Status.
*/
typedef enum ptx_HAL_DRV_Interface_I2C_TrxStatus
{
    TRX_NOT_STARTED     = 0,
    TRX_TX_INPROGRESS   = 1u,
    TRX_TX_DONE         = 2u,
    TRX_RX_INPROGRESS   = 3u,
    TRX_RX_DONE         = 4u,
    TRX_TIMEOUT         = 5u,
    TRX_ERROR           = 6u
} ptx_HAL_DRV_Interface_I2C_TrxStatus_t;

/**
 * \brief Port Configuration
 *
 * This structure contains all variables required to operate the I2C driver from the Renesas RA4M2 MCU.
 *
 * \note Integrators must initialise this structure according to their needs! A working example can be found in the
 * corresponding c file.
 */
typedef struct ptx_HAL_DRV_Interface_I2C_PortConfig
{
    ptx_HAL_DRV_Interface_Port_Type_t               Type;                       /**< Implementation Type of the hardware interface port. */
    uint8_t                                         ID;                         /**< ID of the hardware interface port implementation. */

    const i2c_master_instance_t                     *Instance;                  /**< I2C instance, generated by RENESAS Config Generator. */
    bsp_io_port_pin_t                               Irq;                        /**< I2C IRQ Pin, configured by RENESAS Config. */

    uint8_t                                         Channel;                    /**< I2C Channel, configured by RENESAS Config. */
    volatile ptx_HAL_DRV_Interface_I2C_TrxStatus_t  TransferState;              /**< Transfer State, required to determine if data is still in buffers. */
    volatile int32_t                                TransferWaitCnt;            /**< Transfer Wait Counter, acts as a build in timeout to cancel transfers. */
    const external_irq_instance_t                   *ExternalIrqInstance;       /**< IRQ instance, generated by RENESAS Config Generator. */
} ptx_HAL_DRV_Interface_I2C_PortConfig_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL_DRV_Interface_I2C
{
    ptx_Comps_t                                     CompId;             /**< Component ID. */

    ptx_HAL_DRV_Interface_I2C_PortConfig_t          *Port;              /**< Pointer to the I2C Port Configuration. */
    uint32_t                                        Speed;              /**< I2C Communication Speed. */
    uint8_t                                         Address;            /**< I2C Address. */
    ptx_Log_t                                       *Log;               /**< Log. */
    ptx_HAL_GPIO_t                                  *Gpio;              /**< Initialized GPIO Context. */
} ptx_HAL_DRV_Interface_I2C_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Interface_I2C_InitParams
{
    ptx_HAL_DRV_Interface_Port_Type_t   PortType;           /**< Implementation Type of the hardware interface port. */
    uint8_t                             PortID;             /**< ID of the hardware interface port implementation. */

    uint8_t                             Address;            /**< I2C Address. */
    uint32_t                            Speed;              /**< I2C Communication Speed. */
    ptx_Log_t                           *Log;               /**< Initialised Log Context. */
    ptx_HAL_GPIO_t                      *Gpio;              /**< Initialized GPIO Context. */
} ptx_HAL_DRV_Interface_I2C_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * Initializes the I2C interface driver component. Validates input parameters and searches for a matching
 * port configuration based on PortType and PortID. Sets up component references including Log context,
 * GPIO context, I2C communication speed, and slave device address. If Speed is 0, defaults to
 * PTX_HAL_DRV_INTERFACE_I2C_SPEED_DEFAULT (100 kHz). If Address is 0, defaults to
 * PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4C. The Log and GPIO contexts must be valid, initialized instances.
 *
 * \param[out]      ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Init (ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_I2C_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * Clears all component data and resets component ID to Comp_None. Does not close the I2C interface;
 * ptx_HAL_DRV_Interface_I2C_Close should be called separately if the interface is open.
 *
 * \param[in,out]       ctx              Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_DeInit (ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * \brief Opens the I2C interface.
 *
 * Opens the SCI I2C hardware block and configures it according to the port configuration. If the I2C interface
 * is already open (FSP_ERR_ALREADY_OPEN), it will be closed first and then reopened to ensure a clean state.
 * Configures the I2C communication speed by directly updating hardware registers (MDDR and BRR) as the FSP
 * does not provide an API to change speed at runtime. Supports 100 kHz (default) and 400 kHz (fast mode).
 * Other speeds (1 MHz, 3.4 MHz) are not supported. Opens and enables the external IRQ
 * interface and registers callbacks for both I2C transfer completion and external IRQ events.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Open(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * \brief Closes the I2C interface.
 *
 * Closes the SCI I2C hardware block and the external IRQ interface. Returns success if either
 * interface is successfully closed or if it was not open (FSP_ERR_NOT_OPEN).
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Close(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * \brief Transmits data over I2C.
 *
 * Sends data to the I2C slave device using the configured I2C interface. Initiates an asynchronous
 * write operation and waits for transfer completion via callback. The transfer state is monitored
 * with a timeout mechanism to detect transfer failures or timeouts.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       txBuf           Array of bytes to be transmitted (must not be NULL).
 * \param[in]       txLen           Length of txBuf in bytes.
 * \param[in]       restart         If non-zero, issues a repeated start condition after transfer (for continued communication).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Tx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *txBuf, uint32_t txLen, uint8_t restart);

/**
 * \brief Receives data over I2C.
 *
 * Receives data from the I2C slave device using the configured I2C interface. Initiates an asynchronous
 * read operation and waits for transfer completion via callback. The transfer state is monitored
 * with a timeout mechanism to detect transfer failures or timeouts.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[out]      rxBuf           Array of bytes to receive data into (must not be NULL).
 * \param[in]       rxLen           Length of rxBuf in bytes. Must match the number of bytes expected from slave.
 * \param[in]       restart         If non-zero, issues a repeated start condition after transfer (for continued communication).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Rx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *rxBuf, uint32_t rxLen, uint8_t restart);


/**
 * \brief Reads the current level of the Rx IRQ pin.
 *
 * Reads the current logic level (HIGH or LOW) of the configured IRQ pin using the GPIO interface.
 * The IRQ pin is configured in the port configuration structure (ptx_HAL_DRV_Interface_I2C_PortConfig_t.Irq).
 *
 * \param[in]       ctx             Pointer to component.
 * \param[out]      level           Pointer to variable for storing pin level (0 = LOW, 1 = HIGH).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_RxIrqLevel(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *level);


/**
 * \brief Sets a parameter for the I2C interface.
 *
 * Configures an I2C interface parameter selected by type. Supported parameters include:
 * - HAL_DRV_Interface_Parameter_Type_Transmission_Speed: Changes the I2C communication speed.
 *   The interface will be closed and reopened with the new speed. Supports 100 kHz and 400 kHz.
 *   Other speeds return PTX_STATUS_E_GEN_INVALID_BITRATE.
 * - HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address: Changes the I2C slave device address.
 *   Updates the address using R_SCI_I2C_SlaveAddressSet.
 * - HAL_DRV_Interface_Parameter_Type_UART_Enable_Fast_Mode: Not available for I2C (returns PTX_STATUS_E_GEN_NOT_AVAILABLE).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      Parameter (type, value) struct specifying the parameter to set.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetParameter(ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief Gets a parameter from the I2C interface.
 *
 * Retrieves the current value of an I2C interface parameter selected by type. Supported parameters include:
 * - HAL_DRV_Interface_Parameter_Type_Transmission_Speed: Returns the current I2C communication speed.
 * - HAL_DRV_Interface_Parameter_Type_I2C_Slave_Address: Returns the current I2C slave device address.
 *
 * \param[in]       ctx            Pointer to component.
 * \param[out]      parameter      Parameter (type, value) struct to receive the parameter value.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_GetParameter(ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
