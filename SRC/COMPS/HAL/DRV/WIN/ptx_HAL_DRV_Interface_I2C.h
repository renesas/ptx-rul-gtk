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
    Module      : Windows I2C Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_I2C.h

    Description : Implementation of the I2C Communication Interface Driver for the Windows Reference Platform using FTDI
*/

/**
 * \addtogroup DRV_WIN_IMPL_INTERFACE_I2C
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
#include "ptx_HAL_DRV_Interface_Int.h"
#include "ptx_OSAL.h"
#include "ptx_HAL_Ftd2xx.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
* \brief Communication Interface specific I2C definitions (Config / Speed) for Windows Platform.
 * @{
*/
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4C          (uint8_t)(0x4Cu)        /**< I2C Address 0x4C -> ADDR1 = 0 and ADDR0 = 0 */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4D          (uint8_t)(0x4Du)        /**< I2C Address 0x4C -> ADDR1 = 0 and ADDR0 = 1 */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4E          (uint8_t)(0x4Eu)        /**< I2C Address 0x4C -> ADDR1 = 1 and ADDR0 = 0 */
#define PTX_HAL_DRV_INTERFACE_I2C_ADDRESS_4F          (uint8_t)(0x4Fu)        /**< I2C Address 0x4C -> ADDR1 = 1 and ADDR0 = 1 */

#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_DEFAULT       (100000u)               /**< 100  kHz */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_FAST          (400000u)               /**< 400  kHz */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_FASTPLUS      (1000000u)              /**< 1    MHz */
#define PTX_HAL_DRV_INTERFACE_I2C_SPEED_HIGHSPEED     (3400000u)              /**< 3.4  MHz */
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
 * This structure contains all variables required to operate the I2C driver from the Windows Platform.
 *
 * \note Integrators must initialise this structure according to their needs! A working example can be found in the
 * corresponding c file.
 */
typedef struct ptx_HAL_DRV_Interface_I2C_PortConfig
{
    ptx_HAL_DRV_Interface_Port_Type_t Type;             /**< Implementation Type of the hardware interface port. */
    uint8_t                           ID;               /**< ID of the hardware interface port implementation. */

    char                             *ModuleDescriptor; /**< FTDI module descriptor. */

    ptx_HAL_FTD2XX_GPIO_t             IRQPin_Low;       /**< SPI Default IRQ pin low byte. */
    ptx_HAL_FTD2XX_GPIO_t             IRQPin_High;      /**< SPI Default IRQ pin high byte.*/
} ptx_HAL_DRV_Interface_I2C_PortConfig_t;

/**
 * \brief Wait for IRQ flag.
 *
 */
typedef enum ptx_HAL_Interrupt_Flag
{
    INTERRUPT_FALSE, /**< Interrupt is not expected */
    INTERRUPT_TRUE,  /**< Interrupt is expected and is waited for */
} ptx_HAL_Interrupt_Flag_t;

/**
 * \brief Main Component Struture
 */
typedef struct ptx_HAL_DRV_Interface_I2C
{
    ptx_Comps_t                            CompId;              /**< Component ID. */

    ptx_HAL_FTD2XXCtx_t                    *Ftd2xx_ctx;         /**< FTDI context. */
    ptx_HAL_DRV_Interface_I2C_PortConfig_t *Port;               /**< Pointer to Port config. */
    ptx_Log_t                              *Log;                /**< Pointer to an initialized logger. */
    ptx_OSAL_t                             *OS;                 /**< Pointer to an initialized Osal. */
    uint32_t                               Speed;               /**< I2C Communication Speed. */
    uint8_t                                BusAddr;             /**< Bus Address. */
    ptx_HAL_Interrupt_Flag_t               WaitForInterrupt;    /**< Wait for interrupt flag, can be set interrupt is expected to wait for it. */
} ptx_HAL_DRV_Interface_I2C_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Interface_I2C_InitParams
{
    ptx_HAL_DRV_Interface_Port_Type_t   PortType;           /**< Implementation Type of the hardware interface port. */
    uint8_t                             PortID;             /**< ID of the hardware interface port implementation. */

    uint32_t                            Speed;              /**< I2C Communication Speed. */
    uint8_t                             Address;            /**< Bus Address. */
    ptx_Log_t                           *Log;               /**< Pointer to an initialized logger. */
    void                                *OS;                /**< Pointer to initialized Osal context. */
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
 * \param[in]       ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Init (ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_I2C_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_DeInit (ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * \brief Driver I2C Open
 *
 * Opens the FTDI I2C hardware block and configures it according to the given configuration.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Open(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * \brief Driver I2C Close
 *
 * Closes the FTDI I2C hardware block.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Close(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * \brief Driver I2C TRx
 *
 * Sends data to the I2C hardware block, to communicate with a slave endpoint.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       tx              Array of bytes to be transmitted.
 * \param[in]       txLen           Length of \see tx.
 * \param[in]       rxBuf           Array to save response
 * \param[in]       rxLen           Length of \see rxBuf.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_TRx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *tx, uint32_t *txLen, uint8_t *rxBuf, uint32_t *rxLen);

/**
 * \brief Driver I2C Tx
 *
 * Sends data to the I2C hardware block, to communicate with a slave endpoint.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       txBuf           Array of bytes to be transmitted.
 * \param[in]       txLen           Length of \see txBuf.
 * \param[in]       restart         Restart required flag, unused here.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Tx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *txBuf, uint32_t txLen, uint8_t restart);

/**
 * \brief Driver I2C Rx
 *
 * Read data to the I2C hardware block, to communicate with a slave endpoint.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       rxBuf           Array of bytes to be transmitted.
 * \param[in]       rxLen           Length of \see rxBuf.
 * \param[in]       restart         Restart required flag, unused here.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_Rx(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *rxBuf, uint32_t rxLen, uint8_t restart);

/**
 * \brief Driver I2C Activate Rx IRQ.
 *
 * Activates the Rx IRQ. The interrupt needs to be configured in \ref ptx_HAL_DRV_Interface_I2C_t.Port.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_ActivateRxIrq(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * \brief Driver I2C Deactivate Rx IRQ.
 *
 * Deactivates the Rx IRQ. The interrupt needs to be configured in \ref ptx_HAL_DRV_Interface_I2C_t.Port.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_DeactivateRxIrq(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * \brief Driver I2C Rx IRQ Level
 *
 * Reads the Rx IRQ Level from the configured pin. The pin must be configured in \ref ptx_HAL_DRV_Interface_I2C_t.Port.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[out]      level           Pointer to the current value of the IRQ pin
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_RxIrqLevel(ptx_HAL_DRV_Interface_I2C_t *ctx, uint8_t *level);

/**
 * \brief Driver I2C Wait for IRQ to be triggered
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_WaitForIRQ(ptx_HAL_DRV_Interface_I2C_t *ctx);

/**
 * \brief Driver I2C Set Parameter function.
 *
 * Sets a HAL I2C Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_SetParameter(ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief Driver I2C Get Parameter function.
 *
 * Gets a HAL I2C Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      Parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_I2C_GetParameter(ptx_HAL_DRV_Interface_I2C_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

