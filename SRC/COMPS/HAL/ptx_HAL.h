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
    Module      : HAL Main Interface
    File        : ptx_HAL.h

    Description : Hardware Abstraction Layer (HAL) main interface.
                  Contains functions of top-level HAL interface functions
                  to interact with NFC HW.
*/

/**
 * \addtogroup HAL
 * @{
 */

#if !defined(PTX_HAL_H)
#define PTX_HAL_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_HAL_Interface.h"
#include "ptx_HAL_GPIO.h"
#include "ptx_HAL_Timer.h"
#include "ptx_Log.h"


#if defined(__cplusplus)
extern "C" {
#endif


/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

#define PTX_HAL_TMR_MAX (2)     /**< Max number of timers */
#define PTX_HAL_TMR_1   (0)     /**< Timer 1 */
#define PTX_HAL_TMR_2   (1)     /**< Timer 2 */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HAL
{
    ptx_Comps_t         CompId;                         /**< Component ID. */

    ptx_HAL_Interface_t Interface;                      /**< Hardware Interface. */

    ptx_HAL_GPIO_t      Gpio;                           /**< GPIO. */

    ptx_HAL_Tmr_t       Tmr[PTX_HAL_TMR_MAX];           /**< Timer. */
    uint8_t             NumberOfTimers;                 /**< Number of Timers. */

    ptx_Log_t           *Log;                           /**< Pointer to logger context. */

} ptx_HAL_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HAL_InitParams
{
    uint8_t                             *InterfaceName;         /**< Pointer to the hardware interface name. */
    uint32_t                            InterfaceNameLen;       /**< Length of the hardware interface name. */
    ptx_HAL_Interface_Type_t            InterfaceType;          /**< Type of the hardware interface. */
    ptx_HAL_DRV_Interface_Port_Type_t   InterfacePortType;      /**< Implementation Type of the hardware interface port. */
    uint8_t                             InterfacePortID;        /**< ID of the port for the hardware interface. */
    uint32_t                            InterfaceSpeed;         /**< Communication interface speed. The parameter is used only for testing
                                                                     and is not supposed to be used by the final application. */

    uint8_t                             NumberOfTimers;         /**< Number of requested Timers. */

    ptx_Log_t                           *Log;                   /**< Logger context. */
#if defined(PTX_APP_DIVERSITY_WINDOWS) || defined(PTX_APP_DIVERSITY_LINUX) || defined(PTX_APP_DIVERSITY_ANDROID)
    void                                *Osal;                  /**< OSAL context */
#endif
} ptx_HAL_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief HAL Component Initialization.
 *
 * \param[out]      ctx             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Init (ptx_HAL_t *ctx, ptx_HAL_InitParams_t *initParams);

/**
 * \brief HAL Component De-Initialization.
 *
 * \param[in,out]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DeInit (ptx_HAL_t *ctx);

/**
 * \brief HAL Communication Interface Open.
 *
 * Opens the initialised device.
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 *
 * \param[in,out]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_Open (ptx_HAL_t *ctx);

/**
 * \brief HAL Communication Interface Close.
 *
 * Closes the initialised device.
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 *
 * \param[in,out]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_Close (ptx_HAL_t *ctx);

/**
 * \brief HAL Communication Interface TRx.
 *
 * Transceive function for data. First the data in the \see txBuf array is transmitted. Second the data is received
 * in the \see rxBuf array. If \see txBuf is left empty, the function will not transmit any data. If \see rxBuf is left
 * empty, the function will not receive any data.
 *
 * Depending on the intended usage of this function, it may not be implemented!
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       txBuf           Pointer to array of data to be transmitted.
 * \param[in]       txLen           Length of each of the arrays in \see txBuf.
 * \param[in]       txBufferCnt     Number of buffers to be transmitted.
 * \param[out]      rxBuf           Pointer to array of data to be received. The array must have the right size!
 * \param[in]       rxLen           Length of each array to be received.
 * \param[in]       rxBufferCnt     Number of buffers to be received.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_TRx (ptx_HAL_t *ctx, uint8_t *txBuf[], uint32_t txLen[], uint32_t txBufferCnt,
                                    uint8_t *rxBuf[], uint32_t rxLen[], uint32_t rxBufferCnt);

/**
 * \brief HAL Communication Interface Tx.
 *
 * Transmits the given data in \see txBuf.
 *
 * Depending on the intended usage of this function, it may not be implemented!
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       txBuf           Array of data to be transmitted.
 * \param[in]       txLen           Number of bytes to be transmitted.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_Tx (ptx_HAL_t *ctx, uint8_t *txBuf, uint32_t txLen);

/**
 * \brief HAL Communication Interface Rx.
 *
 * Receives the given data in \see rxBuf.
 *
 * Depending on the intended usage of this function, it may not be implemented!
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[out]      rxBuf           Array with the length of data to be received.
 * \param[in]       rxLen           Number of bytes to be received.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_Rx (ptx_HAL_t *ctx, uint8_t *rxBuf, uint32_t rxLen);

/**
 * \brief HAL Communication Interface Receive Operation Pending.
 *
 * Checks if PTX2xxR/W signalled that a read operation (SPI, UART, etc.) is pending.
 * \see rxCbCtx is the corresponding context for the callback to work with.
 * *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       rxPending       Pointer to store the information if a receive operation is pending.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_RxPending(ptx_HAL_t *ctx, uint8_t* rxPending);

/**
 * \brief HAL Communication Interface Set Parameter function.
 *
 * Sets a HAL Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value ( \see ptx_HAL_DRV_Interface_Parameter_Value_t)
 * through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       parameter       Parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_SetParameter(ptx_HAL_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief HAL Communication Interface Get Parameter function.
 *
 * Gets a HAL Communication Interface parameter selected by a type ( \see ptx_HAL_DRV_Interface_Parameter_Type_t) with the corresponding data value ( \see ptx_HAL_DRV_Interface_Parameter_Value_t)
 * through the input value parameter ( \see ptx_HAL_DRV_Interface_Parameter_t).
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       parameter       Parameter (type, value) struct.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Interface_GetParameter(ptx_HAL_t *ctx, ptx_HAL_DRV_Interface_Parameter_t *parameter);

/**
 * \brief Opens access to the GPIO of the hardware
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in,out]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_GPIO_Open (ptx_HAL_t *ctx);

/**
 * \brief Close access to the GPIO of the hardware
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in,out]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_GPIO_Close (ptx_HAL_t *ctx);

/**
 * \brief Reads the given pin and stores the result
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       pin             Pin number to read from.
 * \param[out]      level           Level of the pin. Either HIGH or LOW.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_GPIO_ReadPin (ptx_HAL_t *ctx, uint16_t pin, uint8_t *level);

/**
 * \brief Writes the given pin
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       pin             Pin number to write to.
 * \param[in]       level           Level of the pin. Either HIGH or LOW.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_GPIO_WritePin (ptx_HAL_t *ctx, uint16_t pin, uint8_t level);

/**
 * \brief Reads the given pins associated to port and stores the results
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       port            Port number to read from.
 * \param[out]      data            Data from the port. Each pin is stored as either high or low value, encoded as bit.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_GPIO_ReadPort (ptx_HAL_t *ctx, uint32_t port, uint16_t *data);

/**
 * \brief Writes the given pins associated to port and stores the results
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       port            Port number to write to.
 * \param[in]       data            Data from the port. Each pin is stored as either high or low value, encoded as bit.
 * \param[in]       mask            Mask identifying which pins to write.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_GPIO_WritePort (ptx_HAL_t *ctx, uint32_t port, uint16_t data, uint16_t mask);

/**
 * \brief Configures the GPIO Port
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       cfg             Array containing configuration options.
 * \param[in]       cfgLen          Length of the array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_GPIO_SetCfgPin (ptx_HAL_t *ctx, ptx_HAL_DRV_GPIO_Cfg_t cfg[], uint32_t cfgLen);


/**
 * \brief Enables interrupts globally
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_EnableIrq (ptx_HAL_t *ctx);

/**
 * \brief Disables interrupts globally
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_DisableIrq (ptx_HAL_t *ctx);

/**
 * \brief Waits for interrupts
 *
 * \note It is required to initialise a device in \ref ptx_HAL_Init, otherwise this function will not work!
 * \note It is required that the device implements this function in order to work!
 *
 * \param[in]       ctx             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_WaitForIrq (ptx_HAL_t *ctx);

/**
 * \brief Timer Open.
 *
 * \param[in,out]   ctx             Pointer to component.
 * \param[in]       id              Timer Id
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Tmr_Open (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id);

/**
 * \brief Timer Close.
 *
 * \param[in,out]   ctx             Pointer to component.
 * \param[in]       id              Timer Id
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Tmr_Close (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id);

/**
 * \brief Timer Start.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       id              Timer Id
 * \param[in]       unit            Time unit (e.g. us, ms, s)
 * \param[in]       ticks           Timer ticks. A tick represents one unit, e.g. a tick of 5 and the unit us result in a timer for 5us.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Tmr_Start (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id, ptx_HAL_DRV_Tmr_Unit_t unit, uint32_t ticks);

/**
 * \brief Timer Stop.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       id              Timer Id
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Tmr_Stop (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id);

/**
 * \brief Timer IsElapsed.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       id              Timer Id
 * \param[in]       isElapsed       Pointer to information storage.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Tmr_IsElapsed (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id, uint8_t *isElapsed);

/**
 * \brief Timer RegExecCb.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       id              Timer Id
 * \param[in]       cb              Callback to be registered for the Timer ISR.
 * \param[in]       cbCtx           Context used by the callback.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Tmr_RegExecCb (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id, ptx_HAL_DRV_Tmr_ExecCb_t cb, ptx_HAL_DRV_Tmr_ExecCbCtx_t cbCtx);

/**
 * \brief Timer Configuration.
 *
 * \deprecated This function will be removed in a future version. All relevant timer functions can be configured in ptx_HAL_Tmr_Start.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       id              Timer Id
 * \param[in]       cfg             Array of \ref ptx_HAL_DRV_Tmr_Cfg_t configurations.
 * \param[in]       len             Length of cfg array.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Tmr_Cfg (ptx_HAL_t *ctx, ptx_HAL_Tmr_Id_t id, ptx_HAL_DRV_Tmr_Cfg_t cfg[], uint32_t len);

/**
 * \brief Implements system sleep (delay) functionality.
 *
 * \note This is a blocking call: calling thread (or application) shall stop execution for
 * a given \p msTimeout period of time.
 *
 * \note The API is typically called when there is the need to wait for
 * HW peripheral to finish its current operation completely, e.g. after reset, or
 * to add execution delays.
 *
 * \param[in]       ctx             Pointer to component.
 * \param[in]       msTimeout       Sleeping time in milliseconds.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HAL_Sleep (ptx_HAL_t *ctx, uint32_t msTimeout);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

