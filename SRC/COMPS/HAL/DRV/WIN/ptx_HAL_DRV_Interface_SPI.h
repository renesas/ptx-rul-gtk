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
 * \brief Main Component Structure
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
* Initializes the SPI interface driver component for Windows platforms using the FTD2XX library.
* Searches the configured port list for a matching port type and ID, opens the corresponding
* FTD2XX device, and configures the component with the provided parameters.
*
* \param[out]      ctx             Pointer to uninitialized SPI component structure.
*                                  Must not be NULL. Will be zeroed and populated by this function.
* \param[in]       initParams      Pointer to initialization parameters structure.
*                                  Must not be NULL and must contain valid Log pointer.
*                                  PortType and PortID must match a configured port.
*                                  Speed of 0 uses default speed (PTX_HAL_DRV_INTERFACE_SPI_DEFAULT_SPEED).
*
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*
* \note IRQ pin configuration is transferred from Port config to FTD2XX context.
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Init (ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_SPI_InitParams_t *initParams);

/**
* \brief Component De-Initialization.
*
* Closes the FTD2XX library connection and releases all resources associated with the SPI
* interface driver component. Clears the component structure and marks it as uninitialized.
*
* \param[in,out]       ctx             Pointer to initialized SPI component structure.
*                                  Must not be NULL and must have CompId == Comp_HAL.
*
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_DeInit (ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
* \brief Open SPI Interface.
*
* Opens and configures the FTDI SPI hardware interface for communication. The SPI bus is
* configured with the speed parameter previously set during component initialization.
* This function must be called after Init() and before any SPI transactions.
*
* \param[in]       ctx             Pointer to initialized SPI component structure.
*                                  Must not be NULL and must have CompId == Comp_HAL.
*                                  Must have been previously initialized via ptx_HAL_DRV_Interface_SPI_Init().
*
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*
* \note The SPI speed is configured using the value set during Init() or the default speed.
* \note Must be paired with ptx_HAL_DRV_Interface_SPI_Close() to properly release resources.
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Open(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
* \brief Close SPI Interface.
*
* Closes the FTDI SPI hardware interface and releases associated resources. After this
* function returns successfully, the SPI interface is no longer available for transactions
* until ptx_HAL_DRV_Interface_SPI_Open() is called again.
*
* \param[in]       ctx             Pointer to initialized SPI component structure.
*                                  Must not be NULL and must have CompId == Comp_HAL.
*
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
*
* \note This function should be called when SPI communication is complete or before
*       component de-initialization via ptx_HAL_DRV_Interface_SPI_DeInit().
* \note The component remains initialized after Close() and can be reopened with Open().
*/
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Close(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
* \brief Transmit Data via SPI.
*
* Sends data to the SPI interface to communicate with a slave endpoint. This is a transmit-only
* operation where the MISO line is ignored. The data is transmitted at the SPI speed configured
* during component initialization.
*
* \param[in]       ctx             Pointer to initialized SPI component structure.
*                                  Must not be NULL and must have CompId == Comp_HAL.
*                                  Must have been previously opened via ptx_HAL_DRV_Interface_SPI_Open().
* \param[in]       txBuf           Pointer to array of bytes to be transmitted.
*                                  Must not be NULL if txLen > 0.
* \param[in]       txLen           Number of bytes to transmit from txBuf.
*                                  Must be greater than 0.
*
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Tx(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *txBuf, uint32_t txLen);

/**
* \brief Receive Data via SPI.
*
* Receives data from the SPI interface. The data is received at the SPI speed configured
* during component initialization.
*
* \param[in]       ctx             Pointer to initialized SPI component structure.
*                                  Must not be NULL and must have CompId == Comp_HAL.
*                                  Must have been previously opened via ptx_HAL_DRV_Interface_SPI_Open().
* \param[out]      rxBuf           Pointer to array of bytes to receive data into.
*                                  Must not be NULL if rxLen > 0.
*                                  Must be large enough to hold rxLen bytes.
* \param[in]       rxLen           Number of bytes to receive into rxBuf.
*                                  Must be greater than 0 and must match expected data length.
*
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_Rx(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *rxBuf, uint32_t rxLen);

/**
* \brief Assert SPI Chip Select (NSS).
*
* Asserts (drives low) the configured NSS (Chip Select) pin to select the slave device for
* SPI communication. The NSS pin is configured in the Port configuration stored in the component
* context during initialization. This function must be called before SPI transactions with a
* specific slave device.
*
* \param[in]       ctx             Pointer to initialized SPI component structure.
*                                  Must not be NULL and must have CompId == Comp_HAL.
*                                  Must have been previously opened via ptx_HAL_DRV_Interface_SPI_Open().
*
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetNss(ptx_HAL_DRV_Interface_SPI_t *ctx);

/**
* \brief De-assert SPI Chip Select (NSS).
*
* De-asserts (drives high) the configured NSS (Chip Select) pin to deselect the slave device
* and end SPI communication. The NSS pin is configured in the Port configuration stored in the
* component context during initialization. This function must be called after SPI transactions
* with a specific slave device are complete.
*
* \param[in]       ctx             Pointer to initialized SPI component structure.
*                                  Must not be NULL and must have CompId == Comp_HAL.
*                                  Must have been previously opened via ptx_HAL_DRV_Interface_SPI_Open().
*
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_ResetNss(ptx_HAL_DRV_Interface_SPI_t *ctx);


/**
* \brief Read SPI Receive IRQ Pin Level.
*
* Reads the current logic level of the configured Rx IRQ pin. The IRQ pin is configured
* in the Port configuration stored in the component context during initialization. This function
* performs a non-blocking read of the IRQ pin state, useful for polling-based interrupt detection.
*
* \param[in]       ctx             Pointer to initialized SPI component structure.
*                                  Must not be NULL and must have CompId == Comp_HAL.
*                                  Must have been previously opened via ptx_HAL_DRV_Interface_SPI_Open().
* \param[out]      level           Pointer to uint8_t variable to receive the IRQ pin level.
*                                  Must not be NULL.
*                                  Will be set to 0 if IRQ pin is low (inactive).
*                                  Will be set to 1 if IRQ pin is high (active).
*
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_RxIrqLevel(ptx_HAL_DRV_Interface_SPI_t *ctx, uint8_t *level);

/**
* \brief Set SPI Interface Parameter.
*
* Configures a HAL SPI Communication Interface parameter by type and value. Supported parameters
* are defined in ptx_HAL_DRV_Interface_Parameter_Type_t. The parameter type and corresponding value
* are passed via the ptx_HAL_DRV_Interface_Parameter_t structure.
*
* \param[in]       ctx             Pointer to initialized SPI component structure.
*                                  Must not be NULL and must have CompId == Comp_HAL.
*                                  Must have been previously opened via ptx_HAL_DRV_Interface_SPI_Open().
* \param[in]       parameter       Pointer to parameter structure containing:
*                                  - parameter->type: Parameter type to set (see ptx_HAL_DRV_Interface_Parameter_Type_t).
*                                  - parameter->value: Union containing the parameter value corresponding to the type.
*                                  Must not be NULL.
*
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_SetParameter(ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);


/**
* \brief Get SPI Interface Parameter.
*
* Retrieves the current value of a HAL SPI Communication Interface parameter by type.
* Supported parameters are defined in ptx_HAL_DRV_Interface_Parameter_Type_t. The parameter type
* is passed via the ptx_HAL_DRV_Interface_Parameter_t structure, and the current value is returned
* in the same structure's value field.
*
* \param[in]       ctx             Pointer to initialized SPI component structure.
*                                  Must not be NULL and must have CompId == Comp_HAL.
*                                  Must have been previously opened via ptx_HAL_DRV_Interface_SPI_Open().
* \param[in,out]   parameter       Pointer to parameter structure containing:
*                                  - parameter->type: [in] Parameter type to retrieve (see ptx_HAL_DRV_Interface_Parameter_Type_t).
*                                  - parameter->value: [out] Union to receive the parameter value corresponding to the type.
*                                  Must not be NULL.
*
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DRV_Interface_SPI_GetParameter(ptx_HAL_DRV_Interface_SPI_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

