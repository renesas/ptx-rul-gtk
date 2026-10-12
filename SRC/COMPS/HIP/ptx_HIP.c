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
    File        : ptx_HIP.c

    Description : Host Interface Protocol (HIP) implements the PTX2xxR/W NFC hardware
                  specific transport mappings for SPI, I2C and UART, as well as low-level commands
                  to download the FW image into the code memory, access command buffers and
                  read/write from/to memory or registers.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <string.h>

#include "ptx_HIP.h"
#include "ptx_HAL.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_HIP_Open (ptx_HIP_t *hip, ptx_HIP_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Accept only valid init params, i.e. hal and frame must also be valid. Otherwise, no point to continue. */

    if ((NULL != hip) && (NULL != initParams) && PTX_COMP_CHECK(initParams->Hal, Comp_HAL) && (NULL != initParams->Log))
    {
        PTX_LOG_V(initParams->Log, "ptx_HIP_Open");

        /* Set to invalid component until init finishes successfully. */
        (void)memset(hip, 0, sizeof(ptx_HIP_t));

        ptx_HIP_Frame_InitParams_t hip_init_params;
        memset(&hip_init_params, 0, sizeof(ptx_HIP_Frame_InitParams_t));
        
        hip_init_params.Log = initParams->Log;
        hip_init_params.Hal = initParams->Hal;



#if defined(PTX_ENABLE_SPI)
        if (HAL_Interface_Type_SPI == initParams->Hal->Interface.Type)
        {

            hip_init_params.EncodeCb = ptx_HIP_Frame_Encode_SPI;
        }
#endif
#if defined(PTX_ENABLE_UART)
        if (HAL_Interface_Type_UART == initParams->Hal->Interface.Type)
        {
            hip_init_params.EncodeCb = ptx_HIP_Frame_Encode_UART;
        }
#endif
#if defined(PTX_ENABLE_I2C)
        if (HAL_Interface_Type_I2C == initParams->Hal->Interface.Type)
        {
            hip_init_params.EncodeCb = ptx_HIP_Frame_Encode_I2C;
        }
#endif
#if defined(PTX_ENABLE_I3C)
        if (HAL_Interface_Type_I3C == initParams->Hal->Interface.Type)
        {
            hip_init_params.EncodeCb = ptx_HIP_Frame_Encode_I3C;
        }
#endif

        status = ptx_HIP_Frame_Open(&hip->Frame, &hip_init_params);
        if (PTX_STATUS_SUCCESS == status)
        {
            hip->CompId = Comp_HIP;
            hip->Log = initParams->Log;
        }
    }

    return status;
}

ptx_Status_t ptx_HIP_Close (ptx_HIP_t *hip)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hip, Comp_HIP))
    {
        PTX_LOG_V(hip->Log, "ptx_HIP_Close");

        status = ptx_HIP_Frame_Close(&hip->Frame);
        if (PTX_STATUS_SUCCESS == status)
        {
            (void)memset(hip, 0, sizeof(ptx_HIP_t));
        }
    }

    return status;
}

ptx_Status_t ptx_HIP_SysRst (ptx_HIP_t *hip, ptx_HIP_Cmd_SysRst_t method)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hip, Comp_HIP) && (HIP_Cmd_SysRst_MAX > method))
    {
        PTX_LOG_V(hip->Log, "ptx_HIP_SysRst");

        ptx_HIP_FrameData_t frame_data =
        /*    CMD    |        IS WRITE FLAG      | Addr | Addr Len |     txBuffer     |   txBufferLen  | rxBuffer | rxBufferLen */
        { HIP_Cmd_Rst, PTX_HIP_FRAME_CMD_IS_WRITE,  NULL,         0, (uint8_t*) &method, sizeof(uint8_t),      NULL,            0 };

        status = ptx_HIP_Frame_TRx(&hip->Frame, &frame_data);
    }

    return status;
}

ptx_Status_t ptx_HIP_RdSysStat (ptx_HIP_t *hip, uint8_t bytes, uint8_t *byteBuf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hip, Comp_HIP) && (NULL != byteBuf))
    {
        PTX_LOG_V(hip->Log, "ptx_HIP_RdSysStat");

        ptx_HIP_FrameData_t frame_data =
        /*    CMD    |        IS WRITE FLAG     | Addr | Addr Len |      txBuffer    |  txBufferLen  | rxBuffer | rxBufferLen */
        { HIP_Cmd_RSS, PTX_HIP_FRAME_CMD_IS_READ,  NULL,          0,           &bytes,  sizeof(bytes),   byteBuf,        bytes };

        status = ptx_HIP_Frame_TRx(&hip->Frame, &frame_data);
    }

    return status;
}

ptx_Status_t ptx_HIP_WrCMem (ptx_HIP_t *hip, uint16_t addr, uint16_t *data, uint32_t dataLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hip, Comp_HIP) && (NULL != data))
    {
        PTX_LOG_V(hip->Log, "ptx_HIP_WrCMem");

        if ( (PTX_HIP_MAX_PAYLOAD_LENGTH / sizeof(uint16_t)) < dataLen )
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            uint16_t tmp_addr = addr;

            uint16_t tmp_data[PTX_HIP_MAX_PAYLOAD_LENGTH / sizeof(uint16_t)];
            memcpy(tmp_data, data, dataLen * sizeof(uint16_t));

            PTX_HIP_BSWAP16(tmp_addr);
            PTX_HIP_BSWAP16_ARRAY(tmp_data, dataLen);

            ptx_HIP_FrameData_t frame_data =
            /*      CMD     |        IS WRITE FLAG      |        Addr        |     Addr Len     |     txBuffer     |         txBufferLen         | rxBuffer | rxBufferLen */
            { HIP_Cmd_WrCMem, PTX_HIP_FRAME_CMD_IS_WRITE, (uint8_t*) &tmp_addr, sizeof(tmp_addr), (uint8_t*)tmp_data,   (uint32_t)(dataLen * sizeof(uint16_t)),      NULL,            0 };

            status = ptx_HIP_Frame_TRx(&hip->Frame, &frame_data);
        }
    }

    return status;
}

ptx_Status_t ptx_HIP_RdCMem (ptx_HIP_t *hip, uint16_t addr, uint16_t *data, uint32_t dataLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hip, Comp_HIP) && (NULL != data))
    {
        PTX_LOG_V(hip->Log, "ptx_HIP_RdCMem");

        uint16_t tmp_len = (uint16_t) dataLen;
        uint16_t tmp_addr = addr;

        PTX_HIP_BSWAP16(tmp_addr);
        PTX_HIP_BSWAP16(tmp_len);

        ptx_HIP_FrameData_t frame_data =
        /*    CMD       |       IS WRITE FLAG      |        Addr        |     Addr Len     |      txBuffer     |   txBufferLen   |   rxBuffer    |          rxBufferLen        */
        { HIP_Cmd_RdCMem, PTX_HIP_FRAME_CMD_IS_READ, (uint8_t*) &tmp_addr,  sizeof(tmp_addr), (uint8_t*) &tmp_len,  sizeof(tmp_len), (uint8_t*)data, (uint32_t)(dataLen * sizeof(uint16_t)) };

        status = ptx_HIP_Frame_TRx(&hip->Frame, &frame_data);

        if (PTX_STATUS_SUCCESS == status)
        {
            PTX_HIP_BSWAP16_ARRAY(data, dataLen);
        }
    }

    return status;
}

ptx_Status_t ptx_HIP_WrDMem (ptx_HIP_t *hip, uint16_t addr, uint8_t *data, uint32_t dataLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * Do not accept dataLen parameter to be 0. This is an invalid value for the HIP protocol -
     * but it SHALL NOT cause HIP command to fail! Only NAK shall be stored in PTX2xxR/W.
     * In order to prevent the situation when WrDMem API returns SUCCESS, although HIP operation failed,
     * do not allow dataLen to be 0.
     */

    if (PTX_COMP_CHECK(hip, Comp_HIP) && (NULL != data) && (0 != dataLen))
    {
        PTX_LOG_V(hip->Log, "ptx_HIP_WrDMem");

        uint16_t tmp_addr = addr;
        PTX_HIP_BSWAP16(tmp_addr);

        ptx_HIP_FrameData_t frame_data =
        /*      CMD     |        IS WRITE FLAG      |         Addr      |     Addr Len     |  txBuffer   | txBufferLen |   rxBuffer    | rxBufferLen */
        { HIP_Cmd_WrDMem, PTX_HIP_FRAME_CMD_IS_WRITE, (uint8_t*) &tmp_addr, sizeof(tmp_addr),         data,      dataLen,           NULL,            0 };

        status = ptx_HIP_Frame_TRx(&hip->Frame, &frame_data);
    }

    return status;
}

ptx_Status_t ptx_HIP_RdDMem (ptx_HIP_t *hip, uint16_t addr, uint8_t *data, uint32_t dataLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * Do not accept dataLen parameter to be 0. This is an invalid value for the HIP protocol -
     * it shall cause HIP command to fail.
     * Still, on SPI bus there shall be a dummy attempt to read 3 Bytes (read as all 0s).
     * In order to prevent this dummy action in HIP, do not accept 0 dataLen.
     */

    if (PTX_COMP_CHECK(hip, Comp_HIP) && (NULL != data) && (0 != dataLen))
    {
        PTX_LOG_V(hip->Log, "ptx_HIP_RdDMem");

        uint16_t tmp_len = (uint16_t) dataLen;
        uint16_t tmp_addr = addr;

        PTX_HIP_BSWAP16(tmp_addr);
        PTX_HIP_BSWAP16(tmp_len);

        ptx_HIP_FrameData_t frame_data =
        /*    CMD       |      IS WRITE FLAG       |         Addr       |    Addr Len     |     txBuffer      |  txBufferLen   |   rxBuffer    | rxBufferLen */
        { HIP_Cmd_RdDMem, PTX_HIP_FRAME_CMD_IS_READ, (uint8_t*) &tmp_addr, sizeof(tmp_addr), (uint8_t*) &tmp_len, sizeof(tmp_len),           data,      dataLen };

        status = ptx_HIP_Frame_TRx(&hip->Frame, &frame_data);
    }

    return status;
}

ptx_Status_t ptx_HIP_WrMsg (ptx_HIP_t *hip, uint8_t addr, uint8_t *data, uint32_t dataLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hip, Comp_HIP) && (NULL != data))
    {
        PTX_LOG_V(hip->Log, "ptx_HIP_WrMsg");


        ptx_HIP_FrameData_t frame_data =
        /*     CMD     |       IS WRITE FLAG       |     Addr    |  Addr Len   |  txBuffer   | txBufferLen |   rxBuffer    | rxBufferLen */
        { HIP_Cmd_WrMsg, PTX_HIP_FRAME_CMD_IS_WRITE,        &addr, sizeof(addr),         data,      dataLen,           NULL,            0 };

        status = ptx_HIP_Frame_TRx(&hip->Frame, &frame_data);
    }

    return status;
}

ptx_Status_t ptx_HIP_RdMsgLen (ptx_HIP_t *hip, uint16_t *len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hip, Comp_HIP) && (len != NULL))
    {
        PTX_LOG_V(hip->Log, "ptx_HIP_RdMsgLen");

        ptx_HIP_FrameData_t frame_data =
        /*    CMD     |       IS WRITE FLAG      | Addr | Addr Len | txBuffer | txBufferLen |   rxBuffer    |   rxBufferLen   */
        { HIP_Cmd_RdML, PTX_HIP_FRAME_CMD_IS_READ,  NULL,         0,      NULL,            0,  (uint8_t*)len, sizeof(uint16_t) };

        status = ptx_HIP_Frame_TRx(&hip->Frame, &frame_data);

        PTX_HIP_BSWAP16(*len);
    }

    return status;
}

ptx_Status_t ptx_HIP_RdMsg (ptx_HIP_t *hip, uint8_t *msg, uint16_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hip, Comp_HIP) && (NULL != msg))
    {
        PTX_LOG_V(hip->Log, "ptx_HIP_RdMsg");

        uint16_t tmp_len = msgLen;
        PTX_HIP_BSWAP16(tmp_len);

        ptx_HIP_FrameData_t frame_data =
        /*    CMD      |       IS WRITE FLAG      | Addr | Addr Len |       txBuffer      |  txBufferLen   | rxBuffer  | rxBufferLen */
        { HIP_Cmd_RdMsg, PTX_HIP_FRAME_CMD_IS_READ,  NULL,         0,   (uint8_t*) &tmp_len, sizeof(tmp_len),        msg,       msgLen };

        status = ptx_HIP_Frame_TRx(&hip->Frame, &frame_data);
    }

    return status;
}

ptx_Status_t ptx_HIP_CfgFrame (ptx_HIP_t *hip, ptx_HIP_Frame_Cfg_t *var, uint8_t *val, uint32_t cnt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(hip, Comp_HIP) && (NULL != var) && (NULL != val))
    {
        PTX_LOG_V(hip->Log, "ptx_HIP_CfgFrame");

        status = ptx_HIP_Frame_Cfg(&hip->Frame, var, val, cnt);
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
