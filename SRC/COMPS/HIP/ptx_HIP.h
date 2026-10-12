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
    Module      : Host Interface Protocol (HIP)
    File        : ptx_HIP.h

    Description : Host Interface Protocol (HIP) implements the PTX2xxR/W NFC hardware
                  specific transport mappings for SPI, I2C and UART, as well as low-level commands
                  to download the FW image into the code memory, access command buffers and
                  read/write from/to memory or registers.
*/

/**
 * \addtogroup HIP
 *
 * @{
 */

#if !defined(PTX_HIP_H)
#define PTX_HIP_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */


#include "ptx_HIP_Frame.h"
#include "ptx_Components.h"
#include "ptx_Status.h"
#include "ptx_HIP_Int.h"

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
 * \brief HAL Forward declaration.
 */
typedef struct ptx_HAL ptx_HAL_t;


/**
 * \brief HIP Frame Encoding Interfaces
 */
typedef enum ptx_HIP_FrameEncodeInterface
{
    HIP_FrameEncodeInterface_SPI,    /**< Frame Encoding for SPI Interface */
    HIP_FrameEncodeInterface_UART,   /**< Frame Encoding for UART Interface */
    HIP_FrameEncodeInterface_I2C,    /**< Frame Encoding for I2C Interface */
} ptx_HIP_FrameEncodeInterface_t;

/**
 * \brief Main Component Structure
 */
typedef struct ptx_HIP
{
    ptx_Comps_t         CompId; /**< Component ID */

    ptx_HIP_Frame_t     Frame;  /**< Pointer to HIP frame structure */
    ptx_Log_t           *Log;   /**< Pointer to logger context. */

} ptx_HIP_t;
/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_HIP_InitParams
{
    ptx_HAL_t                       *Hal;               /**< Pointer to HAL */
    ptx_Log_t                       *Log;               /**< Pointer to logger context. */

} ptx_HIP_InitParams_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[in]       hip             Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_Open (ptx_HIP_t *hip, ptx_HIP_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       hip             Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_Close (ptx_HIP_t *hip);

/**
 * \brief System Reset
 *
 * Resets the system based on the given method. See \see ptx_HIP_Cmd_SysRst_t for available methods.
 *
 * \param[in]       hip             Pointer to component.
 * \param[in]       method          System reset method.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_SysRst (ptx_HIP_t *hip, ptx_HIP_Cmd_SysRst_t method);

/**
 * \brief Read System Status
 *
 * \param[in]       hip             Pointer to component.
 * \param[in]       bytes           Number of bytes to read.
 * \param[out]      byteBuf         Byte buffer for system status. Must be at least \see bytes long.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_RdSysStat (ptx_HIP_t *hip, uint8_t bytes, uint8_t *byteBuf);

/**
 * \brief Write Code Memory
 *
 * \param[in]       hip             Pointer to component.
 * \param[in]       addr            Address to write to.
 * \param[in]       data            Array of data to write.
 * \param[in]       dataLen         Length of data to write.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_WrCMem (ptx_HIP_t *hip, uint16_t addr, uint16_t *data, uint32_t dataLen);

/**
 * \brief Read Code Memory
 *
 * \param[in]       hip             Pointer to component.
 * \param[in]       addr            Address to read to.
 * \param[out]      data            Array of data to read.
 * \param[in]       dataLen         Length of data ro read
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_RdCMem (ptx_HIP_t *hip, uint16_t addr, uint16_t *data, uint32_t dataLen);

/**
 * \brief Write Data Memory
 *
 * \param[in]       hip             Pointer to component.
 * \param[in]       addr            Address to write to.
 * \param[in]       data            Array of data to write.
 * \param[in]       dataLen         Length of data ro write
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_WrDMem (ptx_HIP_t *hip, uint16_t addr, uint8_t *data, uint32_t dataLen);

/**
 * \brief Read Data Memory
 *
 * \param[in]       hip             Pointer to component.
 * \param[in]       addr            Address to read to.
 * \param[out]      data            Array of data to read.
 * \param[in]       dataLen         Length of data ro read
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_RdDMem (ptx_HIP_t *hip, uint16_t addr, uint8_t *data, uint32_t dataLen);

/**
 * \brief Write Message
 *
 * \param[in]       hip             Pointer to component.
 * \param[in]       addr            Address to write to.
 * \param[out]      data            Array of data to write.
 * \param[in]       dataLen         Length of data ro write
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_WrMsg (ptx_HIP_t *hip, uint8_t addr, uint8_t *data, uint32_t dataLen);

/**
 * \brief Read Message Length
 *
 * \param[in]       hip             Pointer to component.
 * \param[out]      len             Pointer to message length.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_RdMsgLen (ptx_HIP_t *hip, uint16_t *len);

/**
 * \brief Read Message
 *
 * \param[in]       hip             Pointer to component.
 * \param[out]      msg             Array of data to read.
 * \param[in]       msgLen          Length of data ro read
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_RdMsg (ptx_HIP_t *hip, uint8_t *msg, uint16_t msgLen);

/**
 * \brief Configure HIP Frame
 *
 * \param[in]       hip             Pointer to component.
 * \param[in]       var             Configuration parameters
 * \param[in]       val             Configuration values
 * \param[in]       cnt             Number of configuration parameter
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HIP_CfgFrame (ptx_HIP_t *hip, ptx_HIP_Frame_Cfg_t *var, uint8_t *val, uint32_t cnt);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
