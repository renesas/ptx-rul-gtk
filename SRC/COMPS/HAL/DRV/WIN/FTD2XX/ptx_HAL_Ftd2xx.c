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
    Module      : HAL Windows FTDI
    File        : ptx_HAL_Ftd2xx.c

    Description : FTDI Driver Implementation for Windows
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_HAL_Ftd2xx.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

static ptx_HAL_FTD2XXCtx_t sFtCtx;

/* Library path */
#define PTX_FTDD2XX_LIB "C:\\repos\\PTX2K\\PTX2K_SHARED_BIN\\FTD2xx\\ftd2xx64.dll\0"

#define PTX_MAX_DEVICE_NAME_LEN             (uint32_t)128
#define PTX_MAX_FTDI_TX_BUFFER_SIZE_MID     (uint32_t)256
#define PTX_MAX_FTDI_TX_BUFFER_SIZE_LARGE   (uint32_t)4096
#define PTX_MAX_FTDI_RX_BUFFER_SIZE_LARGE   (uint32_t)4096
#define PTX_MAX_FTDI_MPSSE_SYNC_RETRIES     (uint8_t)5

/*
 * ####################################################################################################################
 * LOCALS
 * ####################################################################################################################
 */

const char serialAppendI2c[2]       = {'A', 0};
const char InterfaceNameAppendI2c[3]      = {' ', 'A', 0};
const char serialAppendSpi[2]       = {'B', 0};
const char InterfaceNameAppendSpi[3]      = {' ', 'B', 0};
const char serialAppendUart[2]      = {'C', 0};
const char InterfaceNameAppendUart[3]     = {' ', 'C', 0};
const char serialAppendSoftI2c[2]   = {'D', 0};
const char InterfaceNameAppendSoftI2c[3]  = {' ', 'D', 0};

/* MPSSE opcodes */
const uint8_t FTD2XX_MSB_RISING_EDGE_CLOCK_BYTE_OUT     = 0x10;
const uint8_t FTD2XX_MSB_FALLING_EDGE_CLOCK_BYTE_OUT    = 0x11;

const uint8_t FTD2XX_MSB_FALLING_EDGE_CLOCK_BIT_OUT     = 0x13;
const uint8_t FTD2XX_MSB_RISING_EDGE_CLOCK_BYTE_IN      = 0x20;
const uint8_t FTD2XX_MSB_RISING_EDGE_CLOCK_BIT_IN       = 0x22;
const uint8_t FTD2XX_MSB_FALLING_EDGE_CLOCK_BYTE_IN     = 0x24;
const uint8_t FTD2XX_MSB_CLOCK_DATA_BYTES_IN_AND_OUT    = 0x31;
const uint8_t FTD2XX_SET_BITS_LOW_BYTE                  = 0x80;
const uint8_t FTD2XX_READ_DATA_BITS_LOW_BYTE            = 0x81;
const uint8_t FTD2XX_READ_DATA_BITS_HIGH_BYTE           = 0x83;
const uint8_t FTD2XX_SET_BITS_HIGH_BYTE                 = 0x82;
const uint8_t FTD2XX_ENABLE_LOOPBACK                    = 0x84;
const uint8_t FTD2XX_SEND_IMMEDIATE                     = 0x87;
const uint8_t FTD2XX_DISABLE_LOOPBACK                   = 0x85;
const uint8_t FTD2XX_SET_CLK_DIV                        = 0x86;
const uint8_t FTD2XX_DISABLE_CLK_DIV_5                  = 0x8A;
const uint8_t FTD2XX_ENABLE_CLK_DIV_5                   = 0x8B;
const uint8_t FTD2XX_ENABLE_3_PHASE_CLK                 = 0x8C;
const uint8_t FTD2XX_DISABLE_3_PHASE_CLK                = 0x8D;
const uint8_t FTD2XX_DISABLE_ADAPTIVE_CLK               = 0x97;
const uint8_t FTD2XX_OPEN_DRAIN                         = 0x9E;

static ptx_Status_t ptx_HAL_FTD2XX_Read(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataRx, uint32_t *dataRxLen);
static ptx_Status_t ptx_HAL_FTD2XX_Write(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataTx, uint32_t dataTxLen);
static ptx_Status_t ptx_HAL_FTD2XX_MPSSE_Settings_I2C(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
static ptx_Status_t ptx_HAL_FTD2XX_MPSSE_Settings_SPI(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
static ptx_Status_t ptx_HAL_FTD2XX_MPSSE_Settings_SPI_Close(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
static ptx_Status_t ptx_HAL_FTD2XX_CheckIRQLine(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
static ptx_Status_t ptx_HAL_FTD2XX_GPIOBitPos(ptx_HAL_FTD2XX_GPIO_t gpio, int* bitPos);

static ptx_Status_t ptx_HAL_FTD2XX_I2CIdle(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
static ptx_Status_t ptx_HAL_FTD2XX_I2CStart(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
static ptx_Status_t ptx_HAL_FTD2XX_I2CStop(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
static ptx_Status_t ptx_HAL_FTD2XX_I2CRdByte(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataRx, ptx_HAL_FTD2XX_I2C_TypAck_t ackTp);
static ptx_Status_t ptx_HAL_FTD2XX_I2CRdByteAndAck(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataRx);
static ptx_Status_t ptx_HAL_FTD2XX_I2CRdByteAndNack(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataRx);
static ptx_Status_t ptx_HAL_FTD2XX_I2CSendByte(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t dataTx);
static ptx_Status_t ptx_HAL_FTD2XX_I2CSendAdd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t i2cSlave, ptx_HAL_FTD2XX_I2C_TypOp_t i2cType);
static ptx_Status_t ptx_HAL_FTD2XX_I2CTx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t i2cSlave, uint8_t *dataTx, uint32_t dataTxLen);
static ptx_Status_t ptx_HAL_FTD2XX_I2CRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t i2cSlave, uint8_t *dataRx, uint32_t *dataRxLen);

static ptx_Status_t ptx_HAL_FTD2XX_SPITx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataTx, uint32_t dataTxLen);
static ptx_Status_t ptx_HAL_FTD2XX_SPIRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataRx, uint32_t *dataRxLen);

static ptx_Status_t ptx_HAL_FTD2XX_SPITxRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataTx, uint32_t dataTxLen, uint8_t *dataRx, uint32_t *dataRxLen);
static ptx_Status_t ptx_HAL_FTD2XX_SPISendReceiveByte(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t dataTx, uint8_t *dataRx);

static ptx_Status_t ptx_HAL_FTD2XX_SynchronizeMPSSE(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *inputBuffer, uint32_t busSpeed);
static FT_STATUS    ptx_HAL_FTD2XX_SynchronizeMPSSECMDSequence(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *inputBuffer, const uint8_t syncCommand);

/* Open dll and load function pointers */
static ptx_Status_t ptx_HAL_FTD2XX_SetLibCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);

/* Wrappers calling dll functions */
static FT_STATUS ptx_HAL_FTD2XX_GetQueueStatusCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, LPDWORD lpdwAmountInRxQueue);
static FT_STATUS ptx_HAL_FTD2XX_WriteCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx,  LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpBytesWritten);
static FT_STATUS ptx_HAL_FTD2XX_ReadCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx,  LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpBytesRead);
static FT_STATUS ptx_HAL_FTD2XX_PurgeCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, DWORD dwMask);
static FT_STATUS ptx_HAL_FTD2XX_SetBitModeCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, UCHAR ucMask, UCHAR ucEnable);
static FT_STATUS ptx_HAL_FTD2XX_SetLatencyTimerCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, CHAR ucLatency);
static FT_STATUS ptx_HAL_FTD2XX_ResetDeviceCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
static FT_STATUS ptx_HAL_FTD2XX_SetTimeoutsCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, DWORD dwReadTimeout, DWORD dwWriteTimeout);
static FT_STATUS ptx_HAL_FTD2XX_SetCharsCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, UCHAR uEventCh, UCHAR uEventChEn, UCHAR uErrorCh, UCHAR uErrorChEn);
static FT_STATUS ptx_HAL_FTD2XX_SetUSBParametersCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, ULONG ulInTransferSize, ULONG ulOutTransferSize);

/*
 * ####################################################################################################################
 * FUNCTIONS - Wrappers for D2XX Driver
 * ####################################################################################################################
 */

static ptx_Status_t ptx_HAL_FTD2XX_SetLibCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ftd2xxCtx == &sFtCtx)
    {
        ptx_Win_Lib_t *lh = NULL;

        /* Open dynamic library and load the function pointers to the Context.*/
        st = ptx_LibOpen(PTX_FTDD2XX_LIB, &lh);

        if (PTX_STATUS_SUCCESS == st)
        {
            ftd2xxCtx->LH = lh;

            st += ptx_LibFn(lh, "FT_Open", (void **)&ftd2xxCtx->FT_Open);                           // Not used
            st += ptx_LibFn(lh, "FT_OpenEx", (void **)&ftd2xxCtx->FT_OpenEx);                       // Done Room for improvement
            st += ptx_LibFn(lh, "FT_ListDevices", (void **)&ftd2xxCtx->FT_ListDevices);             // Done
            st += ptx_LibFn(lh, "FT_Close", (void **)&ftd2xxCtx->FT_Close);                         // Done
            st += ptx_LibFn(lh, "FT_GetStatus", (void **)&ftd2xxCtx->FT_GetStatus);                 // Done
            st += ptx_LibFn(lh, "FT_GetQueueStatus", (void **)&ftd2xxCtx->FT_GetQueueStatus);       // Done
            st += ptx_LibFn(lh, "FT_Write", (void **)&ftd2xxCtx->FT_Write);                         // Done
            st += ptx_LibFn(lh, "FT_Read", (void **)&ftd2xxCtx->FT_Read);                           // Done
            st += ptx_LibFn(lh, "FT_Purge", (void **)&ftd2xxCtx->FT_Purge);                         // Done
            st += ptx_LibFn(lh, "FT_SetBitMode", (void **)&ftd2xxCtx->FT_SetBitMode);               // Done
            st += ptx_LibFn(lh, "FT_GetBitMode", (void **)&ftd2xxCtx->FT_GetBitMode);               // Not used
            st += ptx_LibFn(lh, "FT_SetFlowControl", (void **)&ftd2xxCtx->FT_SetFlowControl);       // Done
            st += ptx_LibFn(lh, "FT_SetLatencyTimer", (void **)&ftd2xxCtx->FT_SetLatencyTimer);     // Done
            st += ptx_LibFn(lh, "FT_SetUSBParameters", (void **)&ftd2xxCtx->FT_SetUSBParameters);   // Done
            st += ptx_LibFn(lh, "FT_SetChars", (void **)&ftd2xxCtx->FT_SetChars);                   // Done
            st += ptx_LibFn(lh, "FT_SetTimeouts", (void **)&ftd2xxCtx->FT_SetTimeouts);             // Done
            st += ptx_LibFn(lh, "FT_ResetDevice", (void **)&ftd2xxCtx->FT_ResetDevice);             // Done
            st += ptx_LibFn(lh, "FT_SetBaudRate", (void **)&ftd2xxCtx->FT_SetBaudRate);             // Not used
            st += ptx_LibFn(lh, "FT_SetRts", (void **)&ftd2xxCtx->FT_SetRts);                       // Not used
            st += ptx_LibFn(lh, "FT_ClrRts", (void **)&ftd2xxCtx->FT_ClrRts);                       // Not used
            st += ptx_LibFn(lh, "FT_GetModemStatus", (void **)&ftd2xxCtx->FT_GetModemStatus);       // Not used
            st += ptx_LibFn(lh, "FT_SetDataCharacteristics", (void **)&ftd2xxCtx->FT_SetDataCharacteristics);   // Not used
            st += ptx_LibFn(lh, "FT_SetEventNotification", (void **)&ftd2xxCtx->FT_SetEventNotification);       // Not used
            st += ptx_LibFn(lh, "FT_CreateDeviceInfoList", (void **)&ftd2xxCtx->FT_CreateDeviceInfoList);       // Not used
        }
    }

    return st;
}

static FT_STATUS ptx_HAL_FTD2XX_WriteCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx,  LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpBytesWritten)
{
    FT_STATUS  ft_status = FT_INVALID_ARGS;

    if (ftd2xxCtx == &sFtCtx)
    {
        ft_status = ftd2xxCtx->FT_Write(ftd2xxCtx->H, lpBuffer, dwBytesToWrite, lpBytesWritten);
    }

    return ft_status;
}

static FT_STATUS ptx_HAL_FTD2XX_ReadCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx,  LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpBytesRead)
{
    FT_STATUS  ft_status = FT_INVALID_ARGS;

    if (ftd2xxCtx == &sFtCtx)
    {
        ft_status = ftd2xxCtx->FT_Read(ftd2xxCtx->H, lpBuffer, dwBytesToRead, lpBytesRead);
    }

    return ft_status;
}

static FT_STATUS ptx_HAL_FTD2XX_GetQueueStatusCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, LPDWORD lpdwAmountInRxQueue)
{
    FT_STATUS  ft_status = FT_INVALID_ARGS;

    if (ftd2xxCtx == &sFtCtx)
    {
        ft_status =  ftd2xxCtx->FT_GetQueueStatus(ftd2xxCtx->H, lpdwAmountInRxQueue);
    }

    return ft_status;
}

static FT_STATUS ptx_HAL_FTD2XX_PurgeCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, DWORD dwMask)
{
    FT_STATUS  ft_status = FT_INVALID_ARGS;

    if (ftd2xxCtx == &sFtCtx)
    {
        ft_status = ftd2xxCtx->FT_Purge(ftd2xxCtx->H, dwMask);
    }

    return ft_status;
}

static FT_STATUS ptx_HAL_FTD2XX_SetBitModeCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, UCHAR ucMask, UCHAR ucEnable)
{
    FT_STATUS  ft_status = FT_INVALID_ARGS;

    if (ftd2xxCtx == &sFtCtx)
    {
        ft_status = ftd2xxCtx->FT_SetBitMode(ftd2xxCtx->H, ucMask, ucEnable);
    }

    return ft_status;
}

static FT_STATUS ptx_HAL_FTD2XX_SetLatencyTimerCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, CHAR ucLatency)
{
    FT_STATUS  ft_status = FT_INVALID_ARGS;

    if (ftd2xxCtx == &sFtCtx)
    {
        ft_status = ftd2xxCtx->FT_SetLatencyTimer(ftd2xxCtx->H, ucLatency);
    }

    return ft_status;
}

static FT_STATUS ptx_HAL_FTD2XX_ResetDeviceCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    FT_STATUS  ft_status = FT_INVALID_ARGS;

    if (ftd2xxCtx == &sFtCtx)
    {
        ft_status = ftd2xxCtx->FT_ResetDevice(ftd2xxCtx->H);
    }

    return ft_status;
}

static FT_STATUS ptx_HAL_FTD2XX_SetTimeoutsCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, DWORD dwReadTimeout, DWORD dwWriteTimeout)
{
    FT_STATUS  ft_status = FT_INVALID_ARGS;

    if (ftd2xxCtx == &sFtCtx)
    {
        ft_status = ftd2xxCtx->FT_SetTimeouts(ftd2xxCtx->H, dwReadTimeout, dwWriteTimeout);
    }

    return ft_status;
}

static FT_STATUS ptx_HAL_FTD2XX_SetCharsCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, UCHAR uEventCh, UCHAR uEventChEn, UCHAR uErrorCh, UCHAR uErrorChEn)
{
    FT_STATUS  ft_status = FT_INVALID_ARGS;

    if (ftd2xxCtx == &sFtCtx)
    {
        ft_status = ftd2xxCtx->FT_SetChars(ftd2xxCtx->H, uEventCh, uEventChEn, uErrorCh, uErrorChEn);
    }

    return ft_status;
}

static FT_STATUS ptx_HAL_FTD2XX_SetUSBParametersCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, ULONG ulInTransferSize, ULONG ulOutTransferSize)
{
    FT_STATUS ft_status = FT_INVALID_ARGS;

    if (ftd2xxCtx == &sFtCtx)
    {
        ft_status = ftd2xxCtx->FT_SetUSBParameters(ftd2xxCtx->H, ulInTransferSize, ulOutTransferSize);
    }

    return ft_status;
}

static FT_STATUS ptx_HAL_FTD2XX_SingleDeviceOpenCmd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t deviceChannel)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t serial_check = FTDI_STRING_INVALID;
    uint8_t dev_name_check = FTDI_STRING_INVALID;
    uint8_t channel_check = FTDI_DEV_INTF_NONE;

    if (NULL != ftd2xxCtx->DevSerialNumber)
    {
        if (strlen(ftd2xxCtx->DevSerialNumber) <= PTX_MAX_DEVICE_NAME_LEN)
        {
            serial_check = FTDI_SERIAL_VALID;
        }
    }
    else
    {
        serial_check = FTDI_SERIAL_NULL;
    }

    if (NULL != ftd2xxCtx->InterfaceName)
    {
        if (strlen(ftd2xxCtx->InterfaceName) <= PTX_MAX_DEVICE_NAME_LEN)
        {
            dev_name_check = FTDI_DESCRIPTOR_VALID;
        }
    }

    if ((FTDI_DEV_INTF_I2C == deviceChannel)  || (FTDI_DEV_INTF_SPI == deviceChannel) ||
        (FTDI_DEV_INTF_UART == deviceChannel) || (FTDI_DEV_INTF_SW_I2C == deviceChannel))
    {
        channel_check = deviceChannel;
    }

    if ((FTDI_STRING_INVALID != serial_check) && (FTDI_STRING_INVALID != dev_name_check) && (FTDI_DEV_INTF_NONE != channel_check))
    {
        /* Find the FTDI device. */
        FT_STATUS ft_status;
        DWORD num_devs = 0;

        /*
         * It is assumed that Device name will have only the general description of the FTDI device.
         * Which channel is going to be used depends on the chosen interface.
         * I2C:         A
         * SPI:         B
         * UART:        C
         * SOFT_I2C:    D
         * This channel identifier has to be appended in the corresponding Open function i.e. in Uart_Open we append C to serial, or descriptor.
         *
         * Found Device´s name must always match with the provided. If a valid serial is provided, then open a device only if
         * both descriptor and serial match.
         *
         * Name and Serial must not be longer than (63 + NULL) characters !!! Where should this be checked???
         */
        char ftdi_device_serial_no[PTX_MAX_DEVICE_NAME_LEN + 1];
        char ftdi_descriptor_name[PTX_MAX_DEVICE_NAME_LEN + 1];
        char serial_append[2];
        char dev_name_append[3];

        (void)memset(dev_name_append, 0, 3u);
        (void)memset(serial_append, 0, 2u);

        if (FTDI_DEV_INTF_I2C == deviceChannel)
        {
            (void)memcpy(serial_append, serialAppendI2c, 2u);
            (void)memcpy(dev_name_append, InterfaceNameAppendI2c, 3u);
        }
        else if (FTDI_DEV_INTF_SPI == deviceChannel)
        {
            (void)memcpy(serial_append, serialAppendSpi, 2u);
            (void)memcpy(dev_name_append, InterfaceNameAppendSpi, 3u);
        }
        else if (FTDI_DEV_INTF_UART == deviceChannel)
        {
            (void)memcpy(serial_append, serialAppendUart, 2u);
            (void)memcpy(dev_name_append, InterfaceNameAppendUart, 3u);
        }
        else if (FTDI_DEV_INTF_SW_I2C == deviceChannel)
        {
            (void)memcpy(serial_append, serialAppendSoftI2c, 2u);
            (void)memcpy(dev_name_append, InterfaceNameAppendSoftI2c, 3u);
        }

        (void)memset(ftdi_device_serial_no, 0, PTX_MAX_DEVICE_NAME_LEN + 1);
        (void)memset(ftdi_descriptor_name, 0, PTX_MAX_DEVICE_NAME_LEN + 1);

        uint32_t string_size;
        uint32_t append_size;
        if (FTDI_SERIAL_VALID == serial_check)
        {
            /* Serial Number size has already been checked. We don´t do it again. */
            string_size = (uint32_t)strlen(ftd2xxCtx->DevSerialNumber);
            /* SonarQube:: vulnerability fix
             * Use strncpy instead strcpy, use strncat instead of strcat
             * Take care of buffer size: 20By. 19 characters + 1 null-terminate character max.
             *
             * Buffer size limit is: (PTX_MAX_DEVICE_NAME_LEN+1)
             */
            (void)strncpy(ftdi_device_serial_no, ftd2xxCtx->DevSerialNumber, PTX_MAX_DEVICE_NAME_LEN);
            ftdi_device_serial_no[string_size] = 0;

            /* serial_append is size 2, 0-terminated. Only 1By copied */
            if ((string_size + strlen(serial_append)) > PTX_MAX_DEVICE_NAME_LEN)
            {
                append_size = PTX_MAX_DEVICE_NAME_LEN - string_size;
            }
            else
            {
                append_size = (uint32_t)strlen(serial_append);
            }

            (void)strncat(ftdi_device_serial_no, serial_append, append_size);
            ftdi_device_serial_no[string_size + append_size] = 0;
        }

        /* Device name size has already been checked, so we don´t do it again  */
        (void)strncpy(ftdi_descriptor_name, ftd2xxCtx->InterfaceName, PTX_MAX_DEVICE_NAME_LEN);

        uint8_t num_ftdi_devices = 0;
        uint8_t ftdi_device_open_type = FTDI_DEVICE_OPEN_TYPE_UNDEFINED;
        ft_status = ftd2xxCtx->FT_ListDevices(&num_devs, NULL, FT_LIST_NUMBER_ONLY);

        if (FT_OK == ft_status)
        {
            if (num_devs > 0)
            {
                char name_buffer[PTX_MAX_DEVICE_NAME_LEN + 1]; /* Buffer used to get serial number and descriptor name. */

                for (DWORD i = 0; i < num_devs; i++)
                {
                    (void)memset(name_buffer, 0, PTX_MAX_DEVICE_NAME_LEN + 1);
                    /* Check by Descriptor */
                    ft_status = ftd2xxCtx->FT_ListDevices((PVOID)(uintptr_t)i, name_buffer, FT_LIST_BY_INDEX | FT_OPEN_BY_DESCRIPTION);
                    if (FT_OK == ft_status)
                    {
                        /* Compare descriptor Name with default descriptor name for an uart device. */
                        if (0 == strcmp(name_buffer, ftdi_descriptor_name))
                        {
                            ft_status = ftd2xxCtx->FT_ListDevices((PVOID)(uintptr_t)i, name_buffer, FT_LIST_BY_INDEX | FT_OPEN_BY_SERIAL_NUMBER);
                            if (FT_OK == ft_status)
                            {
                                num_ftdi_devices++;

                                if (FTDI_DEVICE_OPEN_TYPE_UNDEFINED == ftdi_device_open_type)
                                {
                                    if (FTDI_SERIAL_VALID == serial_check)
                                    {
                                        /* Compare serial numbers. */
                                        if (0 == strcmp(name_buffer, ftdi_device_serial_no))
                                        {
                                            /* Perfect serial match! */
                                            ftdi_device_open_type = FTDI_DEVICE_OPEN_BY_SERIAL;
                                            /* break; */
                                        }
                                    }
                                    else
                                    {
                                        /* No serial provided, so we can take this device descriptor */
                                        /* FT_ListDevices OK, descriptor match */
                                        ftdi_device_open_type = FTDI_DEVICE_OPEN_BY_DESCRIPTOR;
                                        /* break; */
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (FTDI_DEVICE_OPEN_BY_DESCRIPTOR == ftdi_device_open_type)
        {
            if (1u == num_ftdi_devices)
            {
                /* Open by descriptor. */
                ft_status = ftd2xxCtx->FT_OpenEx(ftdi_descriptor_name, FT_OPEN_BY_DESCRIPTION, (FT_HANDLE *)&ftd2xxCtx->H);
                if (FT_OK == ft_status)
                {
                    st = PTX_STATUS_SUCCESS;
                }
            }
        }
        else if (FTDI_DEVICE_OPEN_BY_SERIAL == ftdi_device_open_type)
        {
            /* Open by serial number. */
            ft_status = ftd2xxCtx->FT_OpenEx(ftdi_device_serial_no, FT_OPEN_BY_SERIAL_NUMBER, (FT_HANDLE *)&ftd2xxCtx->H);
            if (FT_OK == ft_status)
            {
                st = PTX_STATUS_SUCCESS;
            }
        }
        else
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return st;
}

static FT_STATUS ptx_HAL_FTD2XX_CloseCmd(void *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;
    ptx_HAL_FTD2XXCtx_t *ftd2xx_ctx = (ptx_HAL_FTD2XXCtx_t *)ftd2xxCtx;

    FT_STATUS ft_status;

    ft_status = ftd2xx_ctx->FT_Close(ftd2xx_ctx->H);
    ftd2xx_ctx->H = NULL;

    if (ft_status == FT_OK)
    {
        st = PTX_STATUS_SUCCESS;
    }

    return st;
}

/*
* ####################################################################################################################
* FUNCTIONS LIBRARY.
* ####################################################################################################################
*/
ptx_Status_t ptx_HAL_FTD2XX_LibOpen(void **ftd2xxCtx, char *InterfaceName, char *devSerialNumber)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Set init values */
    sFtCtx.InterfaceName = InterfaceName;
    sFtCtx.DevSerialNumber = devSerialNumber;

    /**
     * Let's execute SetLib command.
     */
    st = ptx_HAL_FTD2XX_SetLibCmd(&sFtCtx);

    if (PTX_STATUS_SUCCESS != st)
    {
        st = PTX_STATUS_E_GEN_NOT_FOUND;
    }
    else
    {
        *ftd2xxCtx = &sFtCtx;
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_LibClose(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ftd2xxCtx == &sFtCtx)
    {
        (void) ptx_LibClose(&sFtCtx.LH);

        ftd2xxCtx = NULL;

        st = PTX_STATUS_SUCCESS;
    }
    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_SingleDeviceOpen(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, ptx_HAL_FTD2XX_Device_Interface_t devIntf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INTERNAL;

    if (ftd2xxCtx == &sFtCtx)
    {
        FT_STATUS ft_st = ptx_HAL_FTD2XX_SingleDeviceOpenCmd(ftd2xxCtx, devIntf);
        if (FT_OK == ft_st)
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

/*
* ####################################################################################################################
* FUNCTIONS - COMMON: I2C & SPI.
* ####################################################################################################################
*/
ptx_Status_t ptx_HAL_FTD2XX_CancelWaitIrq(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ftd2xxCtx == &sFtCtx)
    {
        ftd2xxCtx->WaitForIrqFlag = 0;

        st = PTX_STATUS_SUCCESS;
    }
    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_StartWaitIrq(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ftd2xxCtx == &sFtCtx)
    {
        ptx_HAL_FTD2XXCtx_t *ftd_ctx = ftd2xxCtx;
        ftd_ctx->WaitForIrqFlag = 1u;

        st = PTX_STATUS_SUCCESS;
    }
    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_WaitForInterrupt(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * This function checks status of the GPIO line of MPSSE interface.
     * GPIO line used is defined with FT_IRQ_LINE_GPIO_BIT
     */
    if (ftd2xxCtx == &sFtCtx)
    {
        ftd2xxCtx->WaitForIrqFlag = 1u;

        do
        {
            (void)ptx_MutexLock(&ftd2xxCtx->Mutex);
            st = ptx_HAL_FTD2XX_CheckIRQLine(ftd2xxCtx);
            (void)ptx_MutexUnlock(&ftd2xxCtx->Mutex);

            if (PTX_STATUS_SUCCESS == st)
            {
                break;
            }
        }
        while (0 != ftd2xxCtx->WaitForIrqFlag);

        ftd2xxCtx->WaitForIrqFlag = 0;
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_GetGPIOLevel(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, ptx_HAL_FTD2XX_GPIO_t gpio, uint16_t *level)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    (void)ptx_MutexLock(&ftd2xxCtx->Mutex);

    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];
    uint8_t bytes_rx[1u];
    uint32_t bytes_to_read = 1u;

    /* Read current pin state. */
    index = 0;
    bytes_tx[index++] = FTD2XX_READ_DATA_BITS_HIGH_BYTE;
    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    if (PTX_STATUS_SUCCESS == st)
    {
        bytes_rx[0] = 0;
        st = ptx_HAL_FTD2XX_Read(ftd2xxCtx, &bytes_rx[0], &bytes_to_read);
        if (PTX_STATUS_SUCCESS == st)
        {
            int n = 0;
            st = ptx_HAL_FTD2XX_GPIOBitPos(gpio, &n);
            *level = ((uint16_t) bytes_rx[0] & (1 << n)) > 0;
        }
    }

    (void)ptx_MutexUnlock(&ftd2xxCtx->Mutex);

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_SetGPIOLevel(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, ptx_HAL_FTD2XX_GPIO_t gpio, uint16_t level)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;
    (void)ptx_MutexLock(&ftd2xxCtx->Mutex);

    uint8_t current_state = 0;
    st = ptx_HAL_FTD2XX_GetHighPinsState(ftd2xxCtx, &current_state);

    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

    bytes_tx[index++] = FTD2XX_SET_BITS_HIGH_BYTE;
    if (level)
    {
        bytes_tx[index++] = (uint8_t)(current_state | gpio);
    }
    else
    {
        bytes_tx[index++] = (uint8_t)(current_state & ~gpio);
    }
    bytes_tx[index++] = 0xFF;
    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    (void)ptx_MutexUnlock(&ftd2xxCtx->Mutex);

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_GetHighPinsState(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *current_state)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;
    (void)ptx_MutexLock(&ftd2xxCtx->Mutex);

    /* Read current pin state. */
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];
    uint8_t bytes_rx[1u];

    uint32_t index = 0;
    uint32_t bytes_to_read = 1u;

    bytes_tx[index++] = FTD2XX_READ_DATA_BITS_HIGH_BYTE;
    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    if (PTX_STATUS_SUCCESS == st)
    {
        bytes_rx[0] = 0;
        st = ptx_HAL_FTD2XX_Read(ftd2xxCtx, &bytes_rx[0], &bytes_to_read);
        *current_state = bytes_rx[0];
    }

    (void)ptx_MutexUnlock(&ftd2xxCtx->Mutex);

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_SetHighPinsState(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint16_t level, uint16_t mask)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;
    (void)ptx_MutexLock(&ftd2xxCtx->Mutex);

    uint8_t current_state = 0;
    st = ptx_HAL_FTD2XX_GetHighPinsState(ftd2xxCtx, &current_state);

    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

    bytes_tx[index++] = FTD2XX_SET_BITS_HIGH_BYTE;
    if (level)
    {
        bytes_tx[index++] = current_state | (uint8_t) (level & mask);
    }
    else
    {
        bytes_tx[index++] = current_state & (uint8_t) ~mask;
    }
    bytes_tx[index++] = 0xFF;
    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    (void)ptx_MutexUnlock(&ftd2xxCtx->Mutex);

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_GetIRQLine(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint16_t *level)
{
    (void)ptx_MutexLock(&ftd2xxCtx->Mutex);

    /* Only in case of returned SUCCESS, this means that IRQ is set. For all other values: set level 0. */
    *level = (uint16_t)((PTX_STATUS_SUCCESS == ptx_HAL_FTD2XX_CheckIRQLine(ftd2xxCtx)) ? 1U : 0);

    (void)ptx_MutexUnlock(&ftd2xxCtx->Mutex);
    return PTX_STATUS_SUCCESS;
}

/*
 * ####################################################################################################################
 * FUNCTIONS - I2C
 * ####################################################################################################################
 */

ptx_Status_t ptx_HAL_FTD2XX_I2C_Open(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t busSpeed)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    FT_STATUS ft_status = FT_INSUFFICIENT_RESOURCES;

    if (ftd2xxCtx == &sFtCtx && sFtCtx.H == NULL)
    {
        /* If device handle is NULL open device */
        ft_status = ptx_HAL_FTD2XX_SingleDeviceOpenCmd(ftd2xxCtx, FTDI_DEV_INTF_I2C);
    }
    else
    {
        /* Device was already opened */
        ft_status = FT_OK;
    }

    if (FT_SUCCESS(ft_status))
    {
        /* Clear No Stop Flag. */
        ftd2xxCtx->I2C_NoStopFlag = 0;

        /* Initial state of WaitForIrqFlag is 0. */
        ftd2xxCtx->WaitForIrqFlag = 0;

        /* Init mutex object for Trx and IRQ read */
        st = ptx_MutexInit(&ftd2xxCtx->Mutex);
    }

    if (PTX_STATUS_SUCCESS == st)
    {
        st = ptx_HAL_FTD2XX_CheckOpening(ftd2xxCtx, FTDI_DEV_INTF_I2C, busSpeed);
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_I2C_SetSpeed(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t busSpeed)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * We should not allow setting the speed which is out of the (MIN,MAX) range.
     * For 60MHz clock, data rate range is between 457Hz and 30MHz, because Divisor range is 0 - 65535
     */

    /*
     * With the speed values up to 400kHz, it seems there are no problems even without using pull up resistors.
     * With higher speeds, there are problems in communication. Until, it is defined where the problem is, we use 400000kHz msx.
     * const uint32_t bus_speed_max = 400000u;
     *
     * The test of sole send on I2C channel and monitoring with LogicAnalyzer shows that up to 15MHz data bits are created
     * at expected width. For higher speed settings not.
     */
    const uint32_t bus_speed_max = 15000000u; /* Max. speed supported; 15MHz */
    const uint32_t bus_speed_min = 457u;      /* Min. speed supported; 457Hz */

    (void)ptx_MutexLock(&ftd2xxCtx->Mutex);

    if ((busSpeed <= bus_speed_max) && (busSpeed >= bus_speed_min))
    {
        uint32_t bus_speed = busSpeed;
        /*
         *  Configure the dividers to set the SCLK frequency which we will use.
         *  The SCLK clock frequency can be worked out by the algorithm (when divide-by-5 is off):
         *  SCLK frequency  = 60MHz /( (1 + Divisor)*2 )
         */

        uint32_t index = 0;
        uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];
        uint32_t clock_divisor;

        const uint32_t mpsse_clk_src = 60000000u; /* 60 MHz. */

        index = 0;

        /*
         * frequency = 60MHz/((1 + dwClockDivisor) * 2)
         */
        clock_divisor = (mpsse_clk_src / bus_speed);
        clock_divisor /= 2u;
        clock_divisor -= 1u;

        /* Do not divide clock. Use high speed. */
        bytes_tx[index++] = FTD2XX_DISABLE_CLK_DIV_5;

        /* This will set the clock divisor. */
        bytes_tx[index++] = FTD2XX_SET_CLK_DIV;

        /* Set 0xValueL of clock divisor. */
        bytes_tx[index++] = clock_divisor & 0xFF;

        /* Set 0xValueH of clock divisor. */
        bytes_tx[index++] = (clock_divisor >> 8) & 0xFF;

        st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);
    }

    (void)ptx_MutexUnlock(&ftd2xxCtx->Mutex);

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_I2C_TRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t busAddr, uint8_t *wrBuf, uint32_t *wrLen, uint8_t *rdBuf, uint32_t *rdLen, uint8_t restart)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;
    uint32_t num_bytes_w = (NULL != wrLen) ? (*wrLen) : 0;
    uint32_t num_bytes_r = (NULL != rdLen) ? (*rdLen) : 0;

    (void)ptx_MutexLock(&ftd2xxCtx->Mutex);

    if ((wrBuf != NULL) && (num_bytes_w > 0) && (rdBuf != NULL) && (num_bytes_r > 0))
    {
        /* Write and Read Operation. */

        /* For the Write and Read Operation there is re-start condition. */
        if (restart)
        {
            (void)ptx_HAL_FTD2XX_I2C_SetNotStop(ftd2xxCtx);
        }

        st = ptx_HAL_FTD2XX_I2CTx(ftd2xxCtx, busAddr, wrBuf, num_bytes_w);

        if (PTX_STATUS_SUCCESS == st)
        {
            st = ptx_HAL_FTD2XX_I2CRx(ftd2xxCtx, busAddr, rdBuf, &num_bytes_r);
            *rdLen = num_bytes_r;
        }
        (void)ptx_HAL_FTD2XX_I2C_ClearNotStop(ftd2xxCtx);

    }
    else if ((wrBuf != NULL) && (num_bytes_w > 0))
    {
        if (restart)
        {
            (void)ptx_HAL_FTD2XX_I2C_SetNotStop(ftd2xxCtx);
        }

        /* Write Operation. */
        st = ptx_HAL_FTD2XX_I2CTx(ftd2xxCtx, busAddr, wrBuf, num_bytes_w);

        if (restart)
        {
            (void)ptx_HAL_FTD2XX_I2C_ClearNotStop(ftd2xxCtx);
        }
    }
    else if ((rdBuf != NULL) && (num_bytes_r > 0))
    {
        if (restart)
        {
            (void)ptx_HAL_FTD2XX_I2C_SetNotStop(ftd2xxCtx);
        }

        /* Read Operation. */
        st = ptx_HAL_FTD2XX_I2CRx(ftd2xxCtx, busAddr, rdBuf, &num_bytes_r);
        *rdLen = num_bytes_r;

        if (restart)
        {
            (void)ptx_HAL_FTD2XX_I2C_ClearNotStop(ftd2xxCtx);
        }
    }

    (void)ptx_MutexUnlock(&ftd2xxCtx->Mutex);

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_I2C_Close(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ftd2xxCtx == &sFtCtx)
    {
        (void)ptx_MutexLock(&ftd2xxCtx->Mutex);
        (void)ptx_HAL_FTD2XX_CloseCmd(ftd2xxCtx);
        (void)ptx_MutexUnlock(&ftd2xxCtx->Mutex);

        /* Destroy mutex object for Trx and IRQ read */
        (void)ptx_MutexDestroy(&ftd2xxCtx->Mutex);

        st = PTX_STATUS_SUCCESS;
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_I2C_SetNotStop(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_SUCCESS;
    ftd2xxCtx->I2C_NoStopFlag = 1u;

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_I2C_ClearNotStop(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_SUCCESS;
    ftd2xxCtx->I2C_NoStopFlag = 0;

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_I2C_ScanBus(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint16_t *addr_list_buff, uint16_t *addr_list_len)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    if (NULL != ftd2xxCtx)
    {
        ptx_HAL_FTD2XX_I2C_TypOp_t i2c_type = I2C_Wr;
        uint16_t found_count = 0;

        for (uint8_t i = 0; i < 128; ++i)
        {
            uint8_t i2cSlave = i << 1;

            st = ptx_HAL_FTD2XX_I2CStart(ftd2xxCtx);
            if (PTX_STATUS_SUCCESS == st)
            {
                st = ptx_HAL_FTD2XX_I2CSendAdd(ftd2xxCtx, i2cSlave, i2c_type);
            }
            if (PTX_STATUS_SUCCESS == st)
            {
                *(addr_list_buff + found_count) = i << 1;
                found_count ++;
            }
            st = ptx_HAL_FTD2XX_I2CStop(ftd2xxCtx);
        }
        *addr_list_len = found_count;

    }

    return PTX_STATUS_SUCCESS;
}

static ptx_Status_t ptx_HAL_FTD2XX_I2CTx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t i2cSlave, uint8_t *dataTx, uint32_t dataTxLen)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((dataTx != NULL) && (dataTxLen > 0))
    {
        ptx_HAL_FTD2XX_I2C_TypOp_t i2c_type = I2C_Wr;

        st = ptx_HAL_FTD2XX_I2CStart(ftd2xxCtx);

        if (PTX_STATUS_SUCCESS == st)
        {
            st = ptx_HAL_FTD2XX_I2CSendAdd(ftd2xxCtx, i2cSlave, i2c_type);

            if (PTX_STATUS_SUCCESS == st)
            {
                for (uint32_t i = 0; i < dataTxLen; i++)
                {
                    st = ptx_HAL_FTD2XX_I2CSendByte(ftd2xxCtx, dataTx[i]);

                    if (PTX_STATUS_SUCCESS != st)
                    {
                        break;
                    }
                }
            }

            if (PTX_STATUS_SUCCESS == st)
            {
                if (0 == ftd2xxCtx->I2C_NoStopFlag)
                {
                    st = ptx_HAL_FTD2XX_I2CStop(ftd2xxCtx);
                }
            }
        }
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_I2CRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t i2cSlave, uint8_t *dataRx, uint32_t *dataRxLen)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((dataRx != NULL) && (dataRxLen != NULL))
    {
        if (*dataRxLen > 0)
        {
            ptx_HAL_FTD2XX_I2C_TypOp_t i2c_type = I2C_Rd;

            st = ptx_HAL_FTD2XX_I2CStart(ftd2xxCtx);

            if (PTX_STATUS_SUCCESS == st)
            {
                uint32_t i = 0;
                st = ptx_HAL_FTD2XX_I2CSendAdd(ftd2xxCtx, i2cSlave, i2c_type);

                if (PTX_STATUS_SUCCESS == st)
                {
                    for (i = 0; i < ((*dataRxLen) - 1u); i++)
                    {
                        st = ptx_HAL_FTD2XX_I2CRdByteAndAck(ftd2xxCtx, &dataRx[i]);

                        if (PTX_STATUS_SUCCESS != st)
                        {
                            break;
                        }
                    }
                }

                /* Last byte shall be NACK-ed. (in case previous ones are properly transmitted). */
                if (st == PTX_STATUS_SUCCESS)
                {
                    st = ptx_HAL_FTD2XX_I2CRdByteAndNack(ftd2xxCtx, &dataRx[i]);
                }

                /* Stop condition always*/
                if (PTX_STATUS_SUCCESS == st)
                {
                    st = ptx_HAL_FTD2XX_I2CStop(ftd2xxCtx);
                }
                else
                {
                    (void)ptx_HAL_FTD2XX_I2CStop(ftd2xxCtx);
                }
            }
        }
        else
        {
            st = PTX_STATUS_SUCCESS;
        }
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_I2CSendAdd(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t i2cSlave, ptx_HAL_FTD2XX_I2C_TypOp_t i2cType)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    const uint8_t max_add_slave = 0x7F;

    /*
     * This function sends the address of the I2C slave + Rd/Wr bit and checks the ACK.
     */

    if (((I2C_Wr == i2cType) || (I2C_Rd == i2cType)) && (i2cSlave <= max_add_slave))
    {
        uint8_t i2c_sl = ((i2cSlave << 1u) | i2cType);
        st = ptx_HAL_FTD2XX_I2CSendByte(ftd2xxCtx, i2c_sl);
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_I2CRdByte(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataRx, ptx_HAL_FTD2XX_I2C_TypAck_t ackTp)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((dataRx != NULL) && ((I2C_ACK == ackTp) || (I2C_NACK == ackTp)))
    {
        /*
         * This function triggers a read byte operation on the I2C slave.
         */
        uint32_t index = 0;
        uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

        /*
         * Commands for sending the address + wr/rd bit.
         */

        /* Configure pins everything input but CLK. */
        bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;
        bytes_tx[index++] = 0x00;
        bytes_tx[index++] = 0x01;

        /* Command to clock data byte in on the clock rising edge. */
        bytes_tx[index++] = FTD2XX_MSB_RISING_EDGE_CLOCK_BYTE_IN;
        /* Data length of 0x0000 means 1 byte data to clock in. */
        bytes_tx[index++] = 0x00;
        bytes_tx[index++] = 0x00;

        /* Clock out one bit (the ACK/NAK bit). This bit has value '1' to send a NAK to the I2C Slave*/

        /* Configure pins everything input but CLK and DATA_IN as output. */
        bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;
        bytes_tx[index++] = 0x00;
        bytes_tx[index++] = 0x03;

        /* Command to clock data bits out on clock falling edge. */
        bytes_tx[index++] = FTD2XX_MSB_FALLING_EDGE_CLOCK_BIT_OUT;
        /* Length of 0x00 means clock out ONE bit. */
        bytes_tx[index++] = 0x00;

        if (I2C_NACK == ackTp)
        {
            /* Send a '1' as NACK. */
            bytes_tx[index++] = 0xFF;
        }
        else
        {
            /* Send a '0' as ACK. */
            bytes_tx[index++] = 0x00;
        }

        /* Clock line driven low, Data line high (open drain). */
        /* Configure pins everything input but CLK. */
        bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;
        bytes_tx[index++] = 0x00;
        bytes_tx[index++] = 0x01;

        /* Configure the MPSSE to send any results gathered back immediately. */
        bytes_tx[index++] = FTD2XX_SEND_IMMEDIATE;

        st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

        if (PTX_STATUS_SUCCESS == st)
        {
            uint8_t data_rx;
            uint32_t data_rx_len = 1u;
            st = ptx_HAL_FTD2XX_Read(ftd2xxCtx, &data_rx, &data_rx_len);

            if (PTX_STATUS_SUCCESS == st)
            {
                *dataRx = data_rx;
            }
        }
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_I2CRdByteAndAck(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataRx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (dataRx != NULL)
    {
        ptx_HAL_FTD2XX_I2C_TypAck_t ack_tp = I2C_ACK;
        st = ptx_HAL_FTD2XX_I2CRdByte(ftd2xxCtx, dataRx, ack_tp);
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_I2CRdByteAndNack(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataRx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (dataRx != NULL)
    {
        ptx_HAL_FTD2XX_I2C_TypAck_t ack_tp = I2C_NACK;
        st = ptx_HAL_FTD2XX_I2CRdByte(ftd2xxCtx, dataRx, ack_tp);
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_I2CSendByte(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t dataTx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    /*
     * This function sends the address of the I2C slave and checks the ACK.
     */

    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

    /*
     * Commands for sending the address + wr/rd bit.
     */

    /* Configure pins everything input but CLK and DATA_IN as output. */
    bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;
    bytes_tx[index++] = 0x00;
    bytes_tx[index++] = 0x03;

    /* Command to clock data bytes out MSB first on clock falling edge. */
    bytes_tx[index++] = FTD2XX_MSB_FALLING_EDGE_CLOCK_BYTE_OUT;

    /* Data length of 0x0000 means 1 byte data to clock out. */
    bytes_tx[index++] = 0x00;
    bytes_tx[index++] = 0x00;

    /* Actual byte to clock out. */
    bytes_tx[index++] = dataTx;

    /* Configure pins everything input but CLK as output. */
    bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;
    bytes_tx[index++] = 0x00;
    bytes_tx[index++] = 0x01;

    /*
     * Commands for reading the ACK bit.
     */

    /* Command to clock in bits MSB first on clock rising edge. */
    bytes_tx[index++] = FTD2XX_MSB_RISING_EDGE_CLOCK_BIT_IN;

    /* Length of 0x00 means to scan in 1 bit. */
    bytes_tx[index++] = 0x00;

    /* This command then tells the MPSSE to send any results gathered back immediately */
    bytes_tx[index++] = FTD2XX_SEND_IMMEDIATE;

    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    if (PTX_STATUS_SUCCESS == st)
    {
        const uint8_t ack_value = 0;
        uint8_t data_rx;
        uint32_t data_rx_len = 1u;
        st = ptx_HAL_FTD2XX_Read(ftd2xxCtx, &data_rx, &data_rx_len);

        if ((PTX_STATUS_SUCCESS == st) && (ack_value == (data_rx & 0x01)))
        {
            st = PTX_STATUS_SUCCESS;
        }
        else
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_I2CStop(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    /*
     * This function sets the I2C lines to make the stop condition.
     * TODO: It is assumed that the I2C lines are fixed to AD0, AD1 and AD2. Make this configurable!
     */

    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

    /* Initial condition for the I2C Stop - Pull data low (Clock will already be low and is kept low). */
    for (int i = 0; i < 4; i++)
    {
        /* Command to set directions of ADbus and data values for pins set as o/p. */
        bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;

        /* Put data and clock low. */
        bytes_tx[index++] = 0xFC;

        /* Set all pins as output except bit 2 which is the data_in and GPIO used for IRQ. */
        uint8_t irq_bit = (uint8_t)(~FT_IRQ_LINE_GPIO_BIT);
        bytes_tx[index++] = irq_bit & 0xFB;
    }

    /* Clock now goes high (open drain). */
    for (int i = 0; i < 4; i++)
    {
        /* Command to set directions of ADbus and data values for pins set as o/p. */
        bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;

        /* Put data low, clock remains high (open drain, pulled up externally). */
        bytes_tx[index++] = 0xFD;

        /* Set all pins as output except bit 2 which is the data_in and GPIO IRQ. */
        uint8_t irq_bit = (uint8_t)(~FT_IRQ_LINE_GPIO_BIT);
        bytes_tx[index++] = irq_bit & 0xFB;
    }

    /* Data now goes high too (both clock and data now high / open drain). */
    for (int i = 0; i < 4; i++)
    {
        /* Command to set directions of ADbus and data values for pins set as o/p. */
        bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;

        /* Both clock and data now high (open drain, pulled up externally). */
        bytes_tx[index++] = 0xFF;

        /* Set all pins as output except bit 2 which is the data_in and GPIO IRQ. */
        uint8_t irq_bit = (uint8_t)(~FT_IRQ_LINE_GPIO_BIT);
        bytes_tx[index++] = irq_bit & 0xFB;
    }

    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_I2CStart(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    /*
     * This function sets the I2C lines to make the start condition.
     * TODO: Lines have to be set to Idle before the Start condition.
     *
     * TODO: It is assumed that the I2C lines are fixed to AD0, AD1 and AD2. Make this configurable!
     */

    st = ptx_HAL_FTD2XX_I2CIdle(ftd2xxCtx);

    if (PTX_STATUS_SUCCESS == st)
    {
        uint32_t index = 0;
        uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

        for (int i = 0; i < 4; i++)
        {
            /* Command to set directions of ADbus and data values for pins set as o/p. */
            bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;

            /* Bring data out low (bit 1). */
            bytes_tx[index++] = 0xFD;

            /* Set all pins as output except bit 2 which is the data_in and GPIO line for IRQ. */
            uint8_t irq_bit = (uint8_t)(~FT_IRQ_LINE_GPIO_BIT);
            bytes_tx[index++] = irq_bit & 0xFB;
        }

        for (int i = 0; i < 4; i++)
        {
            /* Command to set directions of ADbus and data values for pins set as o/p. */
            bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;

            /* Bring clock line low too to make clock and data low. */
            bytes_tx[index++] = 0xFC;

            /* Set all pins as output except bit 2 which is the data_in and GPIO line for IRQ. */
            uint8_t irq_bit = (uint8_t)(~FT_IRQ_LINE_GPIO_BIT);
            bytes_tx[index++] = irq_bit & 0xFB;
        }

        st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_I2CIdle(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    /*
     * This function sets the lines involved in the I2C communication in
     * idle state.  TODO: It is assumed that the I2C lines are fixed to AD0, AD1 and AD2.
     */

    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

    /* Command to set directions of ADbus and data values for pins set as o/p. */
    bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;

    /* Set all 8 lines to high level (only affects pins which are output). */
    bytes_tx[index++] = 0xFF;

    /* Set all pins as output except bit 2 which is the data_in */

    uint8_t irq_bit = (uint8_t)(~FT_IRQ_LINE_GPIO_BIT);
    bytes_tx[index++] = irq_bit & 0xFB;

    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    return st;
}

/*
 * ####################################################################################################################
 * FUNCTIONS - SPI
 * ####################################################################################################################
 */

ptx_Status_t ptx_HAL_FTD2XX_SPI_Open(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t busSpeed)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    FT_STATUS ft_status = FT_INSUFFICIENT_RESOURCES;
    if (ftd2xxCtx == &sFtCtx && sFtCtx.H == NULL)
    {
        /* If device handle is NULL open device */
        ft_status = ptx_HAL_FTD2XX_SingleDeviceOpenCmd(ftd2xxCtx, FTDI_DEV_INTF_SPI);
    }
    else
    {
        /* Device was already opened */
        ft_status = FT_OK;
    }

    if (FT_SUCCESS(ft_status))
    {

        /* Set Keep Nss Active Flag. */
        ftd2xxCtx->SPI_KeepNssActiveFlag = 1;

        /* Initial state of WaitForIrqFlag is 0. */
        ftd2xxCtx->WaitForIrqFlag = 0;

        /* Init mutex object for Trx and IRQ read */
        st = ptx_MutexInit(&ftd2xxCtx->Mutex);

        if (PTX_STATUS_SUCCESS == st)
        {
            st = ptx_HAL_FTD2XX_CheckOpening(ftd2xxCtx, FTDI_DEV_INTF_SPI, busSpeed);
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_SPI_Close(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ftd2xxCtx == &sFtCtx)
    {
        ptx_HAL_FTD2XXCtx_t *ftd_ctx = (ptx_HAL_FTD2XXCtx_t *)ftd2xxCtx;

        (void)ptx_MutexLock(&ftd_ctx->Mutex);
        (void)ptx_HAL_FTD2XX_MPSSE_Settings_SPI_Close(ftd_ctx);
        (void)ptx_HAL_FTD2XX_CloseCmd(ftd_ctx);
        (void)ptx_MutexUnlock(&ftd_ctx->Mutex);

        /* Destroy mutex object for Trx and IRQ read */
        (void)ptx_MutexDestroy(&ftd_ctx->Mutex);

        st = PTX_STATUS_SUCCESS;
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_SetClock(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t busSpeed)
{
    /* For high speed clock up to 30MHz speed */
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * FTDI supports divisor settings which correspond to speed setting of 457Hz - 30MHz
     * However, PTX supports speed up to 10MHz.
     * So, we limit it to this ? Or not, e.g. if we want to test PTX with higher speed what will happen...
     *
     * */
    const uint32_t bus_speed_max = 15000000u; /* Max. speed supported; 15MHz */
    const uint32_t bus_speed_min = 457u;      /* Min. speed supported; 457Hz */

    if ((busSpeed <= bus_speed_max) && (busSpeed >= bus_speed_min))
    {
        /*
         * We should not allow setting the speed which is out of the (MIN,MAX) range.
         * For 60MHz clock, data rate range is between ~460Hz and 30MHz, because Divisor range is 0 - 65535
         *
         * The test of sole send on SPI channel and monitoring with LogicAnalyzer shows that up to 15MHz data bits are created
         * properly and at expected width. For higher speed settings not.
         */
        uint32_t bus_speed = busSpeed;

        /*
         *  Configure the dividers to set the SCLK frequency which we will use.
         *  The SCLK clock frequency can be worked out by the algorithm (when divide-by-5 is off).
         *  SCLK frequency  = 60MHz /( (1 + Divisor)*2 )
         */

        uint32_t index = 0;
        uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];
        uint32_t clock_divisor;

        const uint32_t mpsse_clk_src_6MHz   = 6000000u;     /* 6 MHz. */
        const uint32_t mpsse_clk_src_30MHz  = 30000000u;    /* 30 MHz. */

        index = 0;

        if (bus_speed <= mpsse_clk_src_6MHz)
        {
            /* Enable clock divider */
            uint8_t enab_cmd = FTD2XX_ENABLE_CLK_DIV_5;
            st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &enab_cmd, 1);
            clock_divisor = (mpsse_clk_src_6MHz / bus_speed) - 1;
        }
        else
        {
            uint8_t disab_cmd = FTD2XX_DISABLE_CLK_DIV_5;
            st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &disab_cmd, 1);
            clock_divisor = (mpsse_clk_src_30MHz / bus_speed) -1;
        }

        /* This will set the clock divisor. */
        bytes_tx[index++] = FTD2XX_SET_CLK_DIV;

        /* Set 0xValueL of clock divisor. */
        bytes_tx[index++] = clock_divisor & 0xFF;

        /* Set 0xValueH of clock divisor. */
        bytes_tx[index++] = (clock_divisor >> 8) & 0xFF;

        st |= ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);
        Sleep(20);

        index = 0;
        /* Ensure internal loopback is off. */
        bytes_tx[index++] = FTD2XX_DISABLE_LOOPBACK;
        st |= ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);
        Sleep(30u);
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_SPI_SetKeepNssActive(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_SUCCESS;

    ftd2xxCtx->SPI_KeepNssActiveFlag = 1u;

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_SPI_ClearKeepNssActive(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_SUCCESS;

    ftd2xxCtx->SPI_KeepNssActiveFlag = 0;

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_SPICSEnable(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    (void)ptx_MutexLock(&ftd2xxCtx->Mutex);

    /*
     * SPI channel used: A, ADBUS
     * This function enables SPI slave by setting CS line low.
     */
    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

    bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;               /* Command to set directions of LOW byte pins (SPI - ADBUS0, ADBUS1, ADBUS2, ADBUS3) */
    bytes_tx[index++] = 0x00 | FT_MOSI_PIN_VAL_HI;                  /* Low byte Value:      Set MOSI/MISO/CLK/NSS low. */
    bytes_tx[index++] = FT_DIRECTION_CONFIG_DEFAULT_SPI_PINS;   /* Low byte direction   Set MOSI/CLK/NSS output and MISO input.*/

    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    (void)ptx_MutexUnlock(&ftd2xxCtx->Mutex);

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_SPICSDisable(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    (void)ptx_MutexLock(&ftd2xxCtx->Mutex);

    /*
     * SPI channel used: A, ADBUS
     * This function disables SPI slave by setting CS line high.
     */
    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

    bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;               /* Command to set directions of LOW byte pins (SPI - ADBUS0, ADBUS1, ADBUS2, ADBUS3) */
    bytes_tx[index++] = FT_NSS_PIN_VAL_DISABLE_PIN | FT_MOSI_PIN_VAL_HI;             /* Low byte Value:      Set MOSI/MISO/CLK/NSS low. */
    bytes_tx[index++] = FT_DIRECTION_CONFIG_DEFAULT_SPI_PINS;   /* Low byte direction   Set MOSI/CLK/NSS output and MISO input.*/

    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    (void)ptx_MutexUnlock(&ftd2xxCtx->Mutex);

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_SPI_TRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *wrBuf, uint32_t *wrLen, uint8_t *rdBuf, uint32_t *rdLen)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;
    uint32_t num_bytes_w = (NULL != wrLen) ? (*wrLen) : 0;
    uint32_t num_bytes_r = (NULL != rdLen) ? (*rdLen) : 0;

    ptx_HAL_FTD2XXCtx_t *ftd2xx_ctx = (ptx_HAL_FTD2XXCtx_t *)ftd2xxCtx;

    (void)ptx_MutexLock(&ftd2xx_ctx->Mutex);

    if ((wrBuf != NULL) && (num_bytes_w > 0) && (rdBuf != NULL) && (num_bytes_r > 0))
    {
        /* Write and Read Operation. */
        /* Send data bytes. SPI is full duplex. */
        st = ptx_HAL_FTD2XX_SPITxRx(ftd2xx_ctx, wrBuf, num_bytes_w, rdBuf, &num_bytes_r);
        if (PTX_STATUS_SUCCESS == st)
        {
            *rdLen = num_bytes_r;
        }
    }
    else if ((wrBuf != NULL) && (num_bytes_w > 0))
    {
        /* Write Operation. */
        st = ptx_HAL_FTD2XX_SPITx(ftd2xx_ctx, wrBuf, num_bytes_w);
    }
    else if ((rdBuf != NULL) && (num_bytes_r > 0))
    {
        /* Read Operation. */
        st = ptx_HAL_FTD2XX_SPIRx(ftd2xx_ctx, rdBuf, &num_bytes_r);
        *rdLen = num_bytes_r;
    }

    (void)ptx_MutexUnlock(&ftd2xx_ctx->Mutex);

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_SPITxRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataTx, uint32_t dataTxLen, uint8_t *dataRx, uint32_t *dataRxLen)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t j = 0;
    uint32_t num_to_read = *dataRxLen;

    /* Parameter validity is checked in calling routine. Should it be again? */
    if ((dataTx != NULL) && (dataTxLen > 0) && (dataRx != NULL) && (num_to_read > 0))
    {
        for (uint32_t i = 0; i < dataTxLen; i++)
        {
            st = ptx_HAL_FTD2XX_SPISendReceiveByte(ftd2xxCtx, dataTx[i], &dataRx[j]);

            j++;

            if ((PTX_STATUS_SUCCESS != st) || (j >= num_to_read))
            {
                break;
            }
        }
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_SPISendReceiveByte(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t dataTx, uint8_t *dataRx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

    /*
     * This might be done e.g. to load the whole packet first: CS + data byte, and then send.
     * Or, send CS config first and then data.
     *
     * uint8_t data_num=0;
     * (void)ptx_HAL_FTD2XX_SPICSEnableLoad(bytes_tx, &data_num);
     * index += data_num;
     *
     */

    /* Command to clock data bytes in and out at the same time most significant bit first. */
    bytes_tx[index++] = FTD2XX_MSB_CLOCK_DATA_BYTES_IN_AND_OUT;

    /* Data length of 0x0000 means 1 byte data to clock out. */
    bytes_tx[index++] = 0x00;
    bytes_tx[index++] = 0x00;

    /* Actual byte to clock out. */
    bytes_tx[index++] = dataTx;

    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    if (PTX_STATUS_SUCCESS == st)
    {
        uint8_t data_rx;
        uint32_t data_rx_len = 1u;
        st = ptx_HAL_FTD2XX_Read(ftd2xxCtx, &data_rx, &data_rx_len);

        if (PTX_STATUS_SUCCESS == st)
        {
            *dataRx = data_rx;
        }
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_SPITx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataTx, uint32_t dataTxLen)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((dataTx != NULL) && (dataTxLen > 0))
    {

        uint32_t index = 0;
        uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_LARGE];

        /* Command to clock data bytes out MSB first on clock falling edge. */
        bytes_tx[index++] = FTD2XX_MSB_FALLING_EDGE_CLOCK_BYTE_OUT;

        /* Data length of 0x0000 means 1 byte data to clock out. */
        bytes_tx[index++] = (uint8_t)((dataTxLen - 1) & 0xFF);               /* lengthL */
        bytes_tx[index++] = (uint8_t)(((dataTxLen - 1) & 0xFF00) >> 8u);     /* lengthH */

        /* Actual byte(s) to clock out. */
        for (uint16_t bufCnt = 0; bufCnt < dataTxLen; bufCnt++)
        {
            bytes_tx[index++] = dataTx[bufCnt];
        }

        st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_SPIRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataRx, uint32_t *dataRxLen)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((dataRx != NULL) && (dataRxLen != NULL) && (*dataRxLen > 0))
    {

        /*
        * This function triggers a read byte operation on the SPI slave.
        */
        uint32_t index = 0;
        uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];
        uint16_t length = (uint16_t) * dataRxLen;

        /* Command to clock data byte in on the clock rising edge. */
        bytes_tx[index++] = FTD2XX_MSB_RISING_EDGE_CLOCK_BYTE_IN;

        /* Data length of 0x0000 means 1 byte data to clock in. */
        bytes_tx[index++] = (uint8_t)((length - 1) & 0xFF);               /* lengthL */
        bytes_tx[index++] = (uint8_t)(((length - 1) & 0xFF00) >> 8u);     /* lengthH */

        st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

        if (PTX_STATUS_SUCCESS == st)
        {
            uint8_t temp[PTX_MAX_FTDI_RX_BUFFER_SIZE_LARGE];
            uint32_t tempsize = length;
            st = ptx_HAL_FTD2XX_Read(ftd2xxCtx, temp, &tempsize);

            memcpy(dataRx, temp, tempsize);
        }
        else
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return st;
}


/*
 * ####################################################################################################################
 * FUNCTIONS - UART
 * ####################################################################################################################
 */
ptx_Status_t ptx_HAL_FTD2XX_UART_Open(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t busSpeed)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    FT_STATUS ft_status = FT_INSUFFICIENT_RESOURCES;
    if (ftd2xxCtx == &sFtCtx && sFtCtx.H == NULL)
    {
        /* If device handle is NULL open device */
        ft_status = ptx_HAL_FTD2XX_SingleDeviceOpenCmd(ftd2xxCtx, FTDI_DEV_INTF_UART);
    }
    else
    {
        /* Device was already opened */
        ft_status = FT_OK;
    }

    if (FT_OK == ft_status)
    {
        st = ptx_HAL_FTD2XX_CheckOpening(ftd2xxCtx, FTDI_DEV_INTF_UART, busSpeed);
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_UART_Close(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ftd2xxCtx == &sFtCtx)
    {
        ptx_HAL_FTD2XXCtx_t *ftd_ctx = (ptx_HAL_FTD2XXCtx_t *)ftd2xxCtx;

        (void)ptx_MutexLock(&ftd_ctx->Mutex);
        (void)ptx_HAL_FTD2XX_CloseCmd(ftd_ctx);
        (void)ptx_MutexUnlock(&ftd_ctx->Mutex);

        /* Destroy mutex object for Trx and IRQ read */
        (void)ptx_MutexDestroy(&ftd_ctx->Mutex);

        st = PTX_STATUS_SUCCESS;
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_UART_TRx_StaticCtx(uint8_t *wrBuf, uint32_t *wrLen, uint8_t *rdBuf, uint32_t *rdLen)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    st = ptx_HAL_FTD2XX_UART_TRx(&sFtCtx, wrBuf, wrLen, rdBuf, rdLen);

    if (PTX_STATUS_SUCCESS != st)
    {
        PTX_LOG_I(sFtCtx.Log, "ERROR ptx_HAL_FTD2XX_UART_TRx_StaticCtx Status: %d, ", st);
    }
    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_UART_TRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *wrBuf, uint32_t *wrLen, uint8_t *rdBuf, uint32_t *rdLen)
{
    ptx_Status_t st = PTX_STATUS_SUCCESS;
    uint32_t num_bytes_w = (NULL != wrLen) ? (*wrLen) : 0;
    uint32_t num_bytes_r = (NULL != rdLen) ? (*rdLen) : 0;
    uint8_t valid_parameters = 0;

    if (ftd2xxCtx == &sFtCtx)
    {
        /* Check if something is to be written */
        if ((wrBuf != NULL) && (0 != num_bytes_w))
        {
            valid_parameters++;
            st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, wrBuf, num_bytes_w);
        }

        /* Check if something is to be read */
        if ((rdBuf != NULL) && (0 != num_bytes_r))
        {
            /* Check status may be changed from previous write */
            if (PTX_STATUS_SUCCESS == st)
            {
                valid_parameters++;
                st = ptx_HAL_FTD2XX_Read (ftd2xxCtx, rdBuf, &num_bytes_r);
                if (PTX_STATUS_SUCCESS == st)
                {
                    *rdLen = num_bytes_r;
                }
            }
        }

        if (0 == valid_parameters)
        {
            /* Neither Tx nor Rx parameters provided -> invalid call */
            st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_UART_SetSpeed(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t busSpeed)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /*
     * FT232H datasheet says:
     * Data Transfer Rate. The FT232H supports a data transfer rate up to 12 Mbit/s when configured as an
     * RS232/RS422/RS485 UART interface.
     * Please note the FT4232H does not support the baud rates of 7 Mbaud 9 Mbaud, 10 Mbaud and 11 Mbaud.
     * Also, trying to set baud rates around those values returns error. There is also a problem of setting br around
     * 5,1MHz,5,48MHz, 5,8MHz...
     *
     * Therefore, to avoid checking and setting (or trying to set) speed to a possibly unsupported value, it is limited for continuous
     * setting speed up to 5100000 bps.
     * Additionally, FTDI creates valid signals for 6, 8 and 12Mbps. This is also allowed.
     *
     */

    const uint32_t busSpeed_max = 5100000u; /* Max. speed supported;*/
    const uint32_t busSpeed_min = 1u; /* Min. speed supported */

    const uint32_t list_supported_speed[3] = {6000000u, 8000000u, 12000000u};

    if (ftd2xxCtx == &sFtCtx)
    {
        if (
            ( ((busSpeed <= busSpeed_max) && (busSpeed >= busSpeed_min)) || (list_supported_speed[0] == busSpeed)
              || (list_supported_speed[1] == busSpeed) || (list_supported_speed[2] == busSpeed)  )
        )
        {
            FT_STATUS ftStatus;

            /* FTDI workaround NSC Default baudrate is 115200 which is already set in UART_Open
                (setting the same speed twice caused instabilities on FTDI side in form of framing errors)
            */
            if (busSpeed != FT_BAUD_115200)
            {
                ftStatus = ftd2xxCtx->FT_SetBaudRate((FT_HANDLE)ftd2xxCtx->H, busSpeed);
            }
            else
            {
                ftStatus = FT_OK;
            }

            if (ftStatus != FT_OK)
            {
                st = PTX_STATUS_E_GEN_INTERNAL;
            }
            else
            {
                st = PTX_STATUS_SUCCESS;
            }
        }
        else
        {
            st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_UART_SetTimeout(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t timeout)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ftd2xxCtx == &sFtCtx)
    {
        FT_STATUS ftStatus;

        DWORD write_to = 50;
        DWORD read_to = timeout;

        ftStatus = ftd2xxCtx->FT_SetTimeouts(ftd2xxCtx->H, read_to, write_to);

        if (ftStatus != FT_OK)
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_UART_GetModemStatus(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t *modemStatus)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t modem_status = 0;

    if (ftd2xxCtx == &sFtCtx)
    {
        FT_STATUS ftStatus;

        ftStatus = ftd2xxCtx->FT_GetModemStatus((FT_HANDLE)ftd2xxCtx->H, (DWORD*)&modem_status);

        if (ftStatus != FT_OK)
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            *modemStatus = modem_status;
            st = PTX_STATUS_SUCCESS;
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_UART_ClrRts(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ftd2xxCtx == &sFtCtx)
    {
        FT_STATUS ftStatus;

        ftStatus = ftd2xxCtx->FT_ClrRts((FT_HANDLE)ftd2xxCtx->H);

        if (ftStatus != FT_OK)
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            st = PTX_STATUS_SUCCESS;
        }
    }
    return st;

}

ptx_Status_t ptx_HAL_FTD2XX_UART_SetRts(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ftd2xxCtx == &sFtCtx)
    {
        FT_STATUS ftStatus;

        ftStatus = ftd2xxCtx->FT_SetRts((FT_HANDLE)ftd2xxCtx->H);

        if (ftStatus != FT_OK)
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
        else
        {
            st = PTX_STATUS_SUCCESS;
        }
    }
    return st;

}

ptx_Status_t ptx_HAL_FTD2XX_UART_SetFlowControl(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t enableFc)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (ftd2xxCtx == &sFtCtx)
    {
        FT_STATUS ftStatus;
        uint16_t flow_control = FT_FLOW_NONE;

        if (0 != enableFc)
        {
            flow_control = FT_FLOW_RTS_CTS;
        }

        ftStatus = ftd2xxCtx->FT_SetFlowControl((FT_HANDLE)ftd2xxCtx->H, flow_control, 0, 0);
        if (ftStatus != FT_OK)
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
    }
    return st;

}


ptx_Status_t ptx_HAL_FTD2XX_UART_SetDataCharacteristics(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t WordLength, uint8_t StopBits, uint8_t Parity)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* Intentionally not checked other data formats except: 7,8 len,  Even or None, 1 or 2 stop bits
     * Will we even allow different char set for chip communication ?
     */
    uint8_t word_length = (FT_BITS_7 == WordLength) ? FT_BITS_7 : FT_BITS_8;
    uint8_t stop_bits = (FT_STOP_BITS_2 == StopBits) ? FT_STOP_BITS_2 : FT_STOP_BITS_1;
    uint8_t parity_bit = (FT_PARITY_EVEN == Parity) ? FT_PARITY_EVEN : FT_PARITY_NONE;

    if (ftd2xxCtx == &sFtCtx)
    {
        FT_STATUS ftStatus;

        ftStatus = ftd2xxCtx->FT_SetDataCharacteristics((FT_HANDLE)ftd2xxCtx->H, word_length, stop_bits, parity_bit);

        if (ftStatus != FT_OK)
        {
            st = PTX_STATUS_E_GEN_INTERNAL;
        }
    }
    return st;

}

/*
 * ####################################################################################################################
 * FUNCTIONS - HELPERS
 * ####################################################################################################################
 */

static ptx_Status_t ptx_HAL_FTD2XX_GPIOBitPos(ptx_HAL_FTD2XX_GPIO_t gpio, int* bitPos)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    switch (gpio)
    {
        case GPIOH0:
            *bitPos = 0;
            st = PTX_STATUS_SUCCESS;
            break;
        case GPIOH1:
            *bitPos = 1;
            st = PTX_STATUS_SUCCESS;
            break;
        case GPIOH2:
            *bitPos = 2;
            st = PTX_STATUS_SUCCESS;
            break;
        case GPIOH3:
            *bitPos = 3;
            st = PTX_STATUS_SUCCESS;
            break;
        case GPIOH4:
            *bitPos = 4;
            st = PTX_STATUS_SUCCESS;
            break;
        case GPIOH5:
            *bitPos = 5;
            st = PTX_STATUS_SUCCESS;
            break;
        case GPIOH6:
            *bitPos = 6;
            st = PTX_STATUS_SUCCESS;
            break;
        case GPIOH7:
            *bitPos = 7;
            st = PTX_STATUS_SUCCESS;
            break;
        default:
            st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            break;
    }
    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_CheckIRQLine(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];
    uint8_t bytes_rx[1u];
    uint32_t bytes_to_read = 1u;

    /* Set all high pins to input.  */
    bytes_tx[index++] = FTD2XX_SET_BITS_HIGH_BYTE;
    bytes_tx[index++] = 0x00;
    bytes_tx[index++] = 0x00;
    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    if (PTX_STATUS_SUCCESS == st)
    {
        /* Read current pin state. */
        index = 0;
        bytes_tx[index++] = FTD2XX_READ_DATA_BITS_HIGH_BYTE;
        st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

        if (PTX_STATUS_SUCCESS == st)
        {
            bytes_rx[0] = 0;
            st = ptx_HAL_FTD2XX_Read(ftd2xxCtx, &bytes_rx[0], &bytes_to_read);

            if (PTX_STATUS_SUCCESS == st)
            {
                uint8_t current_status = (uint8_t)(bytes_rx[0] & ftd2xxCtx->IRQPin_High);

                if (0 == current_status)
                {
                    st = PTX_STATUS_E_GEN_INTERFACE_IRQ_READ;
                }
            }
        }
    }
    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_MPSSE_Settings_I2C(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

    /* Command to set clock divisor. */
    bytes_tx[index++] = FTD2XX_DISABLE_CLK_DIV_5;

    /* Ensure adaptive clocking is off. */
    bytes_tx[index++] = FTD2XX_DISABLE_ADAPTIVE_CLK;

    /* Enable 3 phase data clocking, data valid on both clock edges for I2C. */
    bytes_tx[index++] = FTD2XX_ENABLE_3_PHASE_CLK;

    /* Ensure internal loopback is off. */
    bytes_tx[index++] = FTD2XX_DISABLE_LOOPBACK;

    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    /* Set the initial directions and values to the lines */
    index = 0;
    bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;               /* Command to set directions of LOW byte pins (I2C - ADBUS0, ADBUS1, ADBUS2, ADBUS3) */
    bytes_tx[index++] = 0x07;                                   /* Low byte Value:      Set all high. (Setup for I2C requires lines not to change simultaneously from initial state) */
    bytes_tx[index++] = 0xFF;                                   /* Low byte Direction:  All as output. */

    /* Set all high pins to input.  */
    bytes_tx[index++] = FTD2XX_SET_BITS_HIGH_BYTE;
    bytes_tx[index++] = 0x00;
    bytes_tx[index++] = 0x00;

    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_MPSSE_Settings_SPI(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

    /* Set the initial directions and values to the lines */
    index = 0;
    bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;               /* Command to set directions of LOW byte pins (SPI - ADBUS0, ADBUS1, ADBUS2, ADBUS3) */
    bytes_tx[index++] = 0x00;                                   /* Low byte Value:      Set all low. */
    bytes_tx[index++] = FT_DIRECTION_CONFIG_DEFAULT_SPI_PINS;   /* Low byte direction   Set MOSI/CLK/NSS output and MISO input.*/

    /* Set all high pins to input.  */
    bytes_tx[index++] = FTD2XX_SET_BITS_HIGH_BYTE;
    bytes_tx[index++] = 0x00;
    bytes_tx[index++] = 0x00;

    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_MPSSE_Settings_SPI_Close(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    uint32_t index = 0;
    uint8_t bytes_tx[PTX_MAX_FTDI_TX_BUFFER_SIZE_MID];

    /* Set the initial directions and values to the lines */
    index = 0;
    bytes_tx[index++] = FTD2XX_SET_BITS_LOW_BYTE;               /* Command to set directions of LOW byte pins (SPI - ADBUS0, ADBUS1, ADBUS2, ADBUS3) */
    bytes_tx[index++] = 0x00;                                   /* Low byte Value:      Set all low. */
    bytes_tx[index++] = 0x00;                                   /* Low byte direction   Set MOSI/CLK/NSS/MISO input.*/

    /* Set all high pins to input.  */
    bytes_tx[index++] = FTD2XX_SET_BITS_HIGH_BYTE;
    bytes_tx[index++] = 0x00;
    bytes_tx[index++] = 0x00;

    st = ptx_HAL_FTD2XX_Write(ftd2xxCtx, &bytes_tx[0], index);

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_Write(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataTx, uint32_t dataTxLen)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((dataTx != NULL) && (dataTxLen > 0))
    {
        FT_STATUS ft_status;
        DWORD bytes_sent = 0;

        ft_status = ptx_HAL_FTD2XX_WriteCmd(ftd2xxCtx, (BYTE *)dataTx, (DWORD)dataTxLen, &bytes_sent);

        if ((FT_OK == ft_status) && ((DWORD)dataTxLen == bytes_sent))
        {
            /*
             * Successful write operation.
             */
            st = PTX_STATUS_SUCCESS;
        }
        else
        {
            if (FT_OK == ft_status)
            {
                st = PTX_STATUS_E_GEN_TIMEOUT;
            }
            else
            {
                st = PTX_STATUS_E_GEN_INTERNAL;
            }
        }
    }

    return st;
}

static ptx_Status_t ptx_HAL_FTD2XX_Read(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *dataRx, uint32_t *dataRxLen)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != dataRx) && (NULL != dataRxLen))
    {
        FT_STATUS ft_status;
        DWORD bytes_to_read = (DWORD)(*dataRxLen);
        *dataRxLen = 0;

        ft_status = ptx_HAL_FTD2XX_ReadCmd(ftd2xxCtx, (LPVOID)dataRx, bytes_to_read, (LPDWORD)dataRxLen);

        if ((FT_OK == ft_status) && (bytes_to_read == (DWORD)(*dataRxLen)))
        {
            /*
             * Successful read operation.
             */
            st = PTX_STATUS_SUCCESS;
        }
        else
        {
            if (FT_OK == ft_status)
            {
                st = PTX_STATUS_E_GEN_TIMEOUT;
            }
            else
            {
                st = PTX_STATUS_E_GEN_INTERNAL;
            }
        }
    }

    return st;
}

ptx_Status_t ptx_HAL_FTD2XX_CheckOpening(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, ptx_HAL_FTD2XX_Device_Interface_t intfType, uint32_t busSpeed)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    /*
     * intfType determins which specific interface routines (i.e. specific setup) will be used: i2c or spi.
     */

    const uint32_t read_to = 5000u;
    const uint32_t write_to = 5000u;

    FT_STATUS ft_status;
    DWORD dwNumInputBuffer = 0;
    BYTE InputBuffer[1024];
    DWORD dwNumBytesRead = 0;
    uint8_t input_buffer[1024];

    memset(&input_buffer[0], 0, 1024);

    /* Reset the FT232H. */
    ft_status = ptx_HAL_FTD2XX_ResetDeviceCmd(ftd2xxCtx);

    /* Purge receive buffer. */
    ft_status |= ftd2xxCtx->FT_GetQueueStatus(ftd2xxCtx->H, &dwNumInputBuffer);

    if ((FT_OK == ft_status) && (dwNumInputBuffer > 0))
    {
        ftd2xxCtx->FT_Read(ftd2xxCtx->H, &InputBuffer, dwNumInputBuffer, &dwNumBytesRead);
    }

    /* Purge */
    ft_status |= ptx_HAL_FTD2XX_PurgeCmd(ftd2xxCtx, FT_PURGE_RX | FT_PURGE_TX);

    /* Set USB request transfer sizes */
    ft_status |= ptx_HAL_FTD2XX_SetUSBParametersCmd(ftd2xxCtx, 65535, 65535);

    /* Disable event and error characters */
    ft_status |= ptx_HAL_FTD2XX_SetCharsCmd(ftd2xxCtx, false, false, false, false);

    /* Set the read and write timeouts. */
    ft_status |= ptx_HAL_FTD2XX_SetTimeoutsCmd(ftd2xxCtx, (DWORD)read_to, (DWORD)write_to);

    /* Set latency timer to 2ms if SPI and to 16 otherwise (Uart / I2C). Valid range is 2-255 */
    ft_status |= ptx_HAL_FTD2XX_SetLatencyTimerCmd(ftd2xxCtx, FTDI_DEV_INTF_SPI == intfType ? FT_LATENCY_TIMER_SPI : FT_LATENCY_TIMER_I2C);

    switch (intfType)
    {
        case FTDI_DEV_INTF_SPI:
        /* FALL THROUGH */
        case FTDI_DEV_INTF_I2C:
        /* FALL THROUGH */
        case FTDI_DEV_INTF_MPSSE_ONLY:

            /* Reset the mode to whatever is set in EEPROM */
            ft_status |= ptx_HAL_FTD2XX_SetBitModeCmd(ftd2xxCtx, 0x0, 0x00);

            /* Enable MPSSE mode. */
            ft_status |= ptx_HAL_FTD2XX_SetBitModeCmd(ftd2xxCtx, 0x0, 0x02);
            break;
        case FTDI_DEV_INTF_UART:
            /* Set baud rate */
            ft_status |= ftd2xxCtx->FT_SetBaudRate(ftd2xxCtx->H, busSpeed);

            /* No flow control */
            ft_status |= ftd2xxCtx->FT_SetFlowControl(ftd2xxCtx->H, FT_FLOW_NONE, 0, 0);

            /* Set data characteristic: 8 bit, no parity, 2 stop bit */
            ft_status |= ftd2xxCtx->FT_SetDataCharacteristics(ftd2xxCtx->H, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);

            /* Reset the mode to whatever is set in EEPROM */
            ft_status |= ptx_HAL_FTD2XX_SetBitModeCmd(ftd2xxCtx, 0x0, 0x00);

            break;

        case FTDI_DEV_INTF_NONE:
            /* NOTHING TODO (error already set) */
            break;
        default:
            break;
    }

    /*
    * Recommended pause from ftdi examples. Wait 50ms for all the USB stuff to complete and work.
    */
    Sleep(50u);

    /* If SPI / I2C Synchronize MPSSE and call respective settings */
    if (FT_OK == ft_status && (FTDI_DEV_INTF_I2C == intfType || FTDI_DEV_INTF_SPI == intfType || FTDI_DEV_INTF_MPSSE_ONLY == intfType))
    {

        st = ptx_HAL_FTD2XX_SynchronizeMPSSE(ftd2xxCtx, input_buffer, busSpeed);

        if (PTX_STATUS_SUCCESS == st)
        {
            if (FTDI_DEV_INTF_SPI == intfType)
            {
                st = ptx_HAL_FTD2XX_MPSSE_Settings_SPI(ftd2xxCtx);
            }
            else if (FTDI_DEV_INTF_I2C == intfType)
            {
                st = ptx_HAL_FTD2XX_MPSSE_Settings_I2C(ftd2xxCtx);
            }
            else if (FTDI_DEV_INTF_MPSSE_ONLY == intfType)
            {
                /* NOTHING TODO */
                st = PTX_STATUS_SUCCESS;
            }
            else
            {
                st = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }

    }
    else if (FT_OK == ft_status && FTDI_DEV_INTF_UART == intfType)
    {
        /* Nothing to do for UART */
        st = PTX_STATUS_SUCCESS;
    }

    return st;
}

static FT_STATUS ptx_HAL_FTD2XX_SynchronizeMPSSECMDSequence(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *inputBuffer, const uint8_t syncCommand)
{
    DWORD dw_num_bytes_to_send;
    DWORD dw_num_input_buffer = 0;
    DWORD dw_num_bytes_trxd = 0;
    uint8_t output_buffer[10];
    DWORD index;
    FT_STATUS ft_status = FT_OK;

    dw_num_bytes_to_send = 0;
    output_buffer[dw_num_bytes_to_send++] = syncCommand;

    ft_status |= ptx_HAL_FTD2XX_WriteCmd(ftd2xxCtx, output_buffer, dw_num_bytes_to_send, &dw_num_bytes_trxd);

    dw_num_bytes_to_send = 0;
    dw_num_input_buffer = 0;
    do
    {
        ft_status |= ptx_HAL_FTD2XX_GetQueueStatusCmd(ftd2xxCtx, &dw_num_input_buffer);
    }
    while ((1 > dw_num_input_buffer) && (FT_OK == ft_status));

    uint8_t cmd_echod = 0;
    ft_status |= ptx_HAL_FTD2XX_ReadCmd(ftd2xxCtx, inputBuffer, dw_num_input_buffer, &dw_num_bytes_trxd);

    if (dw_num_bytes_trxd > 0)
    {
        for (index = 0; index < (dw_num_bytes_trxd - 1); index++)
        {
            if ((0xFA == inputBuffer[index]) && (syncCommand == inputBuffer[index + 1]))
            {
                cmd_echod = 1u;
                break;
            }
        }
    }

    if ((FT_OK == ft_status) && (0u == cmd_echod))
    {
        ft_status = FT_IO_ERROR;
    }

    return ft_status;
}

static ptx_Status_t ptx_HAL_FTD2XX_SynchronizeMPSSE(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *inputBuffer, uint32_t busSpeed)
{
    ptx_Status_t st = PTX_STATUS_E_GEN_INTERNAL;

    /*
     * The pauses are added after FT_Write according to the provided FTDI documentation.
     * It turned out that if the pauses are not used, synchronization fails.
     */

    DWORD dw_num_bytes_to_send;
    DWORD dw_num_input_buffer = 0;
    DWORD dw_num_bytes_trxd = 0;
    uint8_t output_buffer[10];
    FT_STATUS ft_status;


    /* start sync loop */
    const uint8_t retries = PTX_MAX_FTDI_MPSSE_SYNC_RETRIES;
    uint8_t cnt = 0;
    do
    {
        ft_status = ptx_HAL_FTD2XX_PurgeCmd(ftd2xxCtx, FT_PURGE_RX | FT_PURGE_TX);

        /* enable loopback */
        dw_num_bytes_to_send = 0;
        output_buffer[dw_num_bytes_to_send++] = FTD2XX_ENABLE_LOOPBACK;
        ft_status |= ptx_HAL_FTD2XX_WriteCmd(ftd2xxCtx, output_buffer, dw_num_bytes_to_send, &dw_num_bytes_trxd);

        /* Empty Device input buffer */
        ft_status |= ptx_HAL_FTD2XX_GetQueueStatusCmd(ftd2xxCtx, &dw_num_input_buffer);
        if ((FT_OK == ft_status) && (dw_num_input_buffer > 0))
        {
            uint8_t readBuffer[PTX_MAX_FTDI_RX_BUFFER_SIZE_LARGE];
            ft_status |= ptx_HAL_FTD2XX_Read(ftd2xxCtx, (uint8_t*)&readBuffer, (uint32_t*)&dw_num_input_buffer);
        }

        /* Sync with bad command 0xAA */
        const uint8_t sync_byte_pattern_AA = 0xAA;
        ft_status |= ptx_HAL_FTD2XX_SynchronizeMPSSECMDSequence(ftd2xxCtx, inputBuffer, sync_byte_pattern_AA);

        /* Sync with bad command 0xAB */
        const uint8_t sync_byte_pattern_AB = 0xAB;
        ft_status |= ptx_HAL_FTD2XX_SynchronizeMPSSECMDSequence(ftd2xxCtx, inputBuffer, sync_byte_pattern_AB);

        cnt++;
    }
    while (FT_OK != ft_status && cnt < retries);

    /* Wait for USB */
    Sleep(50);

    /* Set Clock frequency */
    if (busSpeed != 0)
    {
        (void) ptx_HAL_FTD2XX_SetClock(ftd2xxCtx, busSpeed);
        Sleep(20);
    }

    /* disable loopback */
    dw_num_bytes_to_send = 0;
    output_buffer[dw_num_bytes_to_send++] = FTD2XX_DISABLE_LOOPBACK;

    ft_status |= ptx_HAL_FTD2XX_WriteCmd(ftd2xxCtx, output_buffer, dw_num_bytes_to_send, &dw_num_bytes_trxd);

    /* Empty Device input buffer */
    ft_status |= ptx_HAL_FTD2XX_GetQueueStatusCmd(ftd2xxCtx, &dw_num_input_buffer);
    if ((FT_OK == ft_status) && (dw_num_input_buffer > 0))
    {
        uint8_t readBuffer[PTX_MAX_FTDI_RX_BUFFER_SIZE_LARGE];
        ft_status |= ptx_HAL_FTD2XX_Read(ftd2xxCtx, (uint8_t*)&readBuffer, (uint32_t*)&dw_num_input_buffer);
    }

    if (FT_OK == ft_status)
    {
        /* synchronization successful */
        st = PTX_STATUS_SUCCESS;
    }
    return st;
}