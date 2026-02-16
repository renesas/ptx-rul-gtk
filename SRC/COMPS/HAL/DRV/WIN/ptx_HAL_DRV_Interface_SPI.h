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
    Module      : Windows SPI Communication Interface Driver
    File        : ptx_HAL_DRV_Interface_SPI.h

    Description : Implementation of the SPI Communication Interface Driver for the Windows Reference Platform using FTDI
*/

/**
 * \addtogroup DRV_WIN_IMPL_INTERFACE_SPI
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
* \brief Communication Interface specific SPI definitions (Config / Speed) for Windows Platform.
 * @{
*/
#define PTX_HAL_DRV_INTERFACE_SPI_DEFAULT_SPEED           (1000000u)      /**< Default SPI speed, set during device initialisation. */
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
 * This structure contains all variables required to operate the SPI driver from the Windows Platform.
 *
 * \note Integrators must initialise this structure according to their needs! A working example can be found in the
 * corresponding c file.
 */
typedef struct ptx_HAL_DRV_Interface_SPI_PortConfig
{
    ptx_HAL_DRV_Interface_Port_Type_t Type;             /**< Implementation Type of the hardware interface port. */
    uint8_t                           ID;               /**< ID of the hardware interface port implementation. */

    char                             *ModuleDescriptor; /**< FTDI module descriptor. */

    ptx_HAL_FTD2XX_GPIO_t             IRQPin_Low;       /**< SPI Default IRQ pin low byte. */
    ptx_HAL_FTD2XX_GPIO_t             IRQPin_High;      /**< SPI Default IRQ pin high byte. */
} ptx_HAL_DRV_Interface_SPI_PortConfig_t;

/**
 * \brief Main Component Struture
 */
typedef struct ptx_HAL_DRV_Interface_SPI
{
    ptx_Comps_t                             CompId;         /**< Component ID. */

    ptx_HAL_FTD2XXCtx_t                     *Ftd2xx_ctx;    /**< FTDI context. */
    ptx_HAL_DRV_Interface_SPI_PortConfig_t  *Port;          /**< Pointer to Port config. */
    ptx_Log_t                               *Log;           /**< Pointer to an initialized logger. */
    ptx_OSAL_t                              *OS;            /**< Pointer to an initialized Osal. */
    uint32_t                                Speed;          /**< SPI Communication Speed. */

} ptx_HAL_DRV_Interface_SPI_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_DRV_Interface_SPI_InitParams
{
    ptx_HAL_DRV_Interface_Port_Type_t   PortType;           /**< Implementation Type of the hardware interface port. */
    uint8_t                             PortID;             /**< ID of the hardware interface port implementation. */

    uint32_t                            Speed;              /**< SPI Communication Speed. */
    uint32_t                            IrqPin;             /**< IRQ Pin. */
    ptx_Log_t                           *Log;               /**< Pointer to an initialized logger. */
    void                                *OS;                /**< Pointer to an initialized OS. */
    ptx_HAL_GPIO_t                      *Gpio;              /**< Initialized GPIO Context. */

} ptx_HAL_DRV_Interface_SPI_InitParams_t;


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
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Init (ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_SPI_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_DeInit (ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
 * \brief Driver SPI Open
 *
 * Opens the FTDI SPI hardware block and configures it according to the given configuration.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Open(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
 * \brief Driver SPI Close
 *
 * Closes the FTDI SPI hardware block.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Close(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
 * \brief Driver SPI Tx
 *
 * Sends data to the SPI interface, to communicate with a slave endpoint.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       txBuf           Array of bytes to be transmitted.
 * \param[in]       txLen           Length of \see txBuf.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Tx(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *txBuf, uint32_t txLen);

/**
 * \brief Driver SPI Rx
 *
 * Receives data from the SPI interface.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       rxBuf           Array of bytes to be received.
 * \param[in]       rxLen           Length of \see rxBuf. Must match the bytes received!
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Rx(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *rxBuf, uint32_t rxLen);

/**
 * \brief Driver SPI Set NSS (Chip Select)
 *
 * Sets the configured NSS pin to select the slave device. The NSS pin is configured
 * in \ref ptx_HAL_DRV_Interface_SPI_t.Port.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetNss(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
 * \brief Driver SPI Reset NSS (Chip Select)
 *
 * Resets the configured NSS pin to deselect the slave device. The NSS pin is configured
 * in \ref ptx_HAL_DRV_Interface_SPI_t.Port.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_ResetNss(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
 * \brief Driver SPI Activate Rx IRQ.
 *
 * Activates the Rx IRQ. The interrupt needs to be configured in \ref ptx_HAL_DRV_Interface_SPI_t.Port.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_ActivateRxIrq(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
 * \brief Driver SPI Deactivate Rx IRQ.
 *
 * Deactivates the Rx IRQ. The interrupt needs to be configured in \ref ptx_HAL_DRV_Interface_SPI_t.Port.
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_DeactivateRxIrq(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
 * \brief Driver SPI Rx IRQ Level
 *
 * Reads the Rx IRQ Level from the configured pin. The pin must be configured in \ref ptx_HAL_DRV_Interface_SPI_t.Port.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[out]      level           Pointer to the current value of the IRQ pin
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_RxIrqLevel(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *level);


/**
 * \brief Driver SPI Wait for IRQ to be triggered
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_WaitForIRQ(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
 * \brief Driver SPI Set Parameter function.
 *
 * Sets a HAL SPI Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetParameter(ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief Driver SPI Get Parameter function.
 *
 * Gets a HAL SPI Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value
 * ( \see ptx_HAL_DRV_Interface_Parameter_Value_t) through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \param[in]       ctx            Pointer to component.
 * \param[in]       parameter      Parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_GetParameter(ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

