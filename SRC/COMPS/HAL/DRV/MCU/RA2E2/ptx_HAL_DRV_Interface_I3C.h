
/*
    ---------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause

    Copyright (c) 2024, Renesas Electronics Corporation and/or its affiliates


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
    Module      : MCU-RA2E2 I3C Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_I3C.h

    Description : Implementation of the I3C Communication Interface Driver for the MCU-RA2E2 Reference Platform
*/

/**
 * \addtogroup DRV_RA2E2_IMPL_INTERFACE_I3C
 * @{
 */

#ifndef PTX_HAL_DRV_INTERFACE_I3C_H
#define PTX_HAL_DRV_INTERFACE_I3C_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "hal_data.h"

#include "ptx_Components.h"
#include "ptx_Log.h"
#include "ptx_HAL_DRV_Interface_Int.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
* ####################################################################################################################
* DEFINES
* ####################################################################################################################
*/

#define PTX_HAL_DRV_INTERFACE_I3C_MAX_IBI_PAYLOAD_SIZE                      (32)
#define PTX_HAL_DRV_INTERFACE_I3C_MAX_WRITE_DATA_LEN                        (275)
#define PTX_HAL_DRV_INTERFACE_I3C_MAX_READ_DATA_LEN                         (32)
#define PTX_HAL_DRV_INTERFACE_I3C_MASTER_DEVICE_STATIC_ADDRESS              (0x47)
#define PTX_HAL_DRV_INTERFACE_I3C_MASTER_DEVICE_DYNAMIC_ADDRESS             (0x77)
#define PTX_HAL_DRV_INTERFACE_I3C_WAIT_TIME                                 (0x7FF)
#define PTX_HAL_DRV_INTERFACE_I3C_SLAVE_DEVICE_DYNAMIC_ADDRESS_START        (0x4C)

#define PTX_HAL_DRV_INTERFACE_I3C_SPEED_DEFAULT                             (1000000u)
#define PTX_HAL_DRV_INTERFACE_I3C_ADDRESS_4C                                (0x4C)

/* I3C Event Flags */
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_ENTDAA_ADDRESS_PHASE           0x00000001
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_IBI_READ_COMPLETE              0x00000002
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_IBI_READ_BUFFER_FULL           0x00000004
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_READ_BUFFER_FULL               0x00000008
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_IBI_WRITE_COMPLETE             0x00000010
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_HDR_EXIT_PATTERN_DETECTED      0x00000020
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_ADDRESS_ASSIGNMENT_COMPLETE    0x00000040
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_COMMAND_COMPLETE               0x00000080
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_WRITE_COMPLETE                 0x00000100
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_READ_COMPLETE                  0x00000200
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_TIMEOUT_DETECTED               0x00000400
#define PTX_HAL_DRV_INTERFACE_I3C_EVENT_FLAG_INTERNAL_ERROR                 0x00000800

#define PTX_HAL_DRV_INTERFACE_I3C_DAA                                       0x01            /* Dynamic Address Assignment Redo */

#define PTX_HAL_DRV_I3C_WORD_ALIGN              (4)
#define PTX_HAL_DRV_I3C_ONE                     (1)
#define PTX_HAL_DRV_I3C_RESET_VALUE             ((int32_t)0u)

/* The ammount of the events found in the i3c_event_t enum is the last event value + one for the array initialization */
#define PTX_HAL_DRV_I3C_EVENT_SIZE              (I3C_EVENT_SDA_WRITE_COMPLETE + PTX_HAL_DRV_I3C_ONE)

/**
 * \brief HAL GPIO Forward declaration.
 */
typedef struct ptx_HAL_GPIO ptx_HAL_GPIO_t;

/**
 * \brief Transceiver Status
 */
typedef enum ptx_HAL_DRV_Interface_TrxStatus
{
    TRX_NOT_STARTED      = 0,
    TRX_TX_INPROGRESS    = 1u,
    TRX_TX_DONE          = 2u,
    TRX_RX_INPROGRESS    = 3u,
    TRX_RX_DONE          = 4u,
    TRX_TIMEOUT          = 5u,
    TRX_ERROR            = 6u
} ptx_HAL_DRV_Interface_I3C_TrxStatus_t;

/**
 * \brief Port Configuration
 *
 * This structure contains all variables required to operate the I3C driver from Renesas.
 *
 * \note Integrators must initialise this structure according to their needs! A working example can be found in the
 * corresponding c file.
 */
typedef struct ptx_HAL_DRV_Interface_I3C_PortConfig
{
    ptx_HAL_DRV_Interface_Port_Type_t               Type;                   /**< Implementation Type of the hardware interface port. */
    uint8_t                                         ID;                     /**< ID of the hardware interface port implementation. */

    const i3c_instance_t                            *Instance;              /**< I3C instance, generated by RENESAS Config Generator */
    bsp_io_port_pin_t                               Irq;                    /**< I3C IRQ Pin, configured by RENESAS Config */

    uint8_t                                         Channel;                /**< I3C Default Channel, configured by RENESAS Config */
    volatile ptx_HAL_DRV_Interface_I3C_TrxStatus_t  TransferState;          /**< Transfer State, required to determine if data is still in buffers */
    volatile int32_t                                TransferWaitCnt;        /**< Transfer Wait Counter, acts as a build in timeout to cancel transfers */
    const external_irq_instance_t                   *ExternalIrqInstance;   /**< IRQ instance, generated by RENESAS Config Generator */
    uint8_t                                         IbiReadData[PTX_HAL_DRV_INTERFACE_I3C_MAX_IBI_PAYLOAD_SIZE] BSP_ALIGN_VARIABLE(PTX_HAL_DRV_I3C_WORD_ALIGN);
    uint8_t                                         WriteData[PTX_HAL_DRV_INTERFACE_I3C_MAX_WRITE_DATA_LEN] BSP_ALIGN_VARIABLE(PTX_HAL_DRV_I3C_WORD_ALIGN);
    i3c_device_cfg_t                                MasterDeviceCfg;
    i3c_device_table_cfg_t                          DeviceTableCfg;         /**< I3C bus/device management */

    volatile uint32_t                               I3cEventStatus;                             /**< Event Status */
    volatile uint32_t                               I3cEventCount[PTX_HAL_DRV_I3C_EVENT_SIZE];  /**< Event Status Counter */

} ptx_HAL_DRV_Interface_I3C_PortConfig_t;

typedef struct ptx_HAL_DRV_Interface_I3C
{
    ptx_Comps_t                                     CompId;         /**< Component ID */
    ptx_HAL_DRV_Interface_I3C_PortConfig_t          *Port;          /**< Pointer to the I3C Port Configuration */
    uint32_t                                        Speed;          /**< I3C Communication Speed */
    uint8_t                                         Address;        /**< I3C Address */
    ptx_Log_t                                       *Log;        /**< Logger */
    ptx_HAL_GPIO_t                                  *Gpio;          /**< Initialized GPIO Context. */
} ptx_HAL_DRV_Interface_I3C_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Interface_I3C_InitParams
{
    ptx_HAL_DRV_Interface_Port_Type_t   PortType;           /**< Implementation Type of the hardware interface port. */
    uint8_t                             PortID;             /**< ID of the hardware interface port implementation. */

    uint8_t                             Address;            /**< I3C Address. */
    uint32_t                            Speed;              /**< I3C Communication Speed. */
    ptx_Log_t                           *Log;               /**< Initialised Logger Context. */
    ptx_HAL_GPIO_t                      *Gpio;              /**< Initialized GPIO Context. */
} ptx_HAL_DRV_Interface_I3C_InitParams_t;

/*
    * ####################################################################################################################
    * TYPES
    * ####################################################################################################################
    */

/*
    * ####################################################################################################################
    * API FUNCTIONS
    * ####################################################################################################################
    */

/**
 * \brief Component Initialization.
 *
 * Initializes the I3C interface driver component. Validates input parameters and searches for a matching
 * port configuration based on PortType and PortID. Sets up component references including Log context,
 * GPIO context, I3C communication speed, and slave device address. The Log and GPIO contexts must be
 * valid, initialized instances.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I3C_Init(ptx_HAL_DRV_Interface_I3C_t *ctx, ptx_HAL_DRV_Interface_I3C_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * Clears all component data and resets component ID to Comp_None. Does not close the I3C interface;
 * ptx_HAL_DRV_Interface_I3C_Close should be called separately if the interface is open.
 *
 * \param[in]       ctx              Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I3C_DeInit(ptx_HAL_DRV_Interface_I3C_t *ctx);

/**
 * \brief Opens the I3C interface.
 *
 * Opens and initializes the I3C hardware module with the following sequence:
 * 1. Opens the I3C module via R_I3C_Open
 * 2. Opens and enables the external IRQ interface for I3C interrupt handling
 * 3. Registers the external IRQ callback
 * 4. Sets the master device configuration (static and dynamic addresses)
 * 5. Configures the I3C device table with slave device information
 * 6. Enables I3C mode on the bus
 * 7. Sets up IBI (In-Band Interrupt) read buffer
 * 8. Initiates Dynamic Address Assignment (ENTDAA) for slave devices
 * 9. Waits for address assignment completion
 * 10. Selects the device and configures SDR0 bitrate mode
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I3C_Open(ptx_HAL_DRV_Interface_I3C_t *ctx);

/**
 * \brief Closes the I3C interface.
 *
 * Closes the I3C hardware module and the external IRQ interface. Returns success if either
 * interface is successfully closed or if it was not open (FSP_ERR_NOT_OPEN).
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I3C_Close(ptx_HAL_DRV_Interface_I3C_t *ctx);

/**
 * \brief Transmits data over I3C.
 *
 * Sends data to the I3C slave device using the configured I3C interface. Copies the transmit data
 * to an internal write buffer, initiates an asynchronous write operation, and waits for transfer
 * completion via callback. The transfer state is monitored with a timeout mechanism to detect
 * transfer failures or timeouts.
 *
 * \param[in]       ctx         Pointer to component.
 * \param[in]       txBuf       Array of bytes to be transmitted (must not be NULL).
 * \param[in]       txLen       Length of txBuf in bytes (must not exceed PTX_HAL_DRV_INTERFACE_I3C_MAX_WRITE_DATA_LEN).
 * \param[in]       restart     If non-zero, issues a repeated start condition after transfer (for continued communication).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I3C_Tx(ptx_HAL_DRV_Interface_I3C_t *ctx, uint8_t txBuf[], uint32_t txLen, uint8_t restart);

/**
 * \brief Receives data over I3C.
 *
 * Receives data from the I3C slave device using the configured I3C interface. Initiates an asynchronous
 * read operation and waits for transfer completion via callback. The transfer state is monitored
 * with a timeout mechanism to detect transfer failures or timeouts.
 *
 * \param[in]       ctx         Pointer to component.
 * \param[out]      rxBuf       Array of bytes to receive data into (must not be NULL).
 * \param[in]       rxLen       Length of rxBuf in bytes (must not exceed PTX_HAL_DRV_INTERFACE_I3C_MAX_READ_DATA_LEN).
 * \param[in]       restart     If non-zero, issues a repeated start condition after transfer (for continued communication).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I3C_Rx(ptx_HAL_DRV_Interface_I3C_t *ctx, uint8_t *rxBuf, uint32_t rxLen, uint8_t restart);

/**
 * \brief Sets a parameter for the I3C interface.
 *
 * Configures an I3C interface parameter selected by type. Supported parameters include:
 * - HAL_DRV_Interface_Parameter_Type_I3C_DynamicAddressAssignment: Triggers a Dynamic Address Assignment (DAA)
 *   operation using the ENTDAA command. The function waits for address assignment completion.
 *
 * Other parameter types return PTX_STATUS_E_GEN_NOT_AVAILABLE.
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      Parameter (type, value) struct specifying the parameter to set.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I3C_SetParameter(ptx_HAL_DRV_Interface_I3C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief Retrieves a parameter value from the I3C interface.
 *
 * Queries the value of a specified parameter for the I3C device context using the provided parameter descriptor.
 * Currently not implemented and returns PTX_STATUS_E_GEN_NOT_IMPLEMENTED.
 *
 * \param[in]       ctx         Pointer to component.
 * \param[out]      parameter   Pointer to the parameter descriptor where the result will be stored.
 *                              See \ref ptx_HAL_DRV_Interface_Parameter_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I3C_GetParameter(ptx_HAL_DRV_Interface_I3C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief Reads the current level of the Rx IRQ pin.
 *
 * Reads the current logic level (HIGH or LOW) of the configured IRQ pin using the GPIO interface.
 * The IRQ pin is configured in the port configuration structure (ptx_HAL_DRV_Interface_I3C_PortConfig_t.Irq).
 *
 * \param[in]       ctx     Pointer to component.
 * \param[out]      level   Pointer to variable for storing pin level (0 = LOW, 1 = HIGH).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I3C_RxIrqLevel(ptx_HAL_DRV_Interface_I3C_t *ctx, uint8_t *level);


#ifdef __cplusplus
}
#endif

#endif /* Guard */

/** @} */
