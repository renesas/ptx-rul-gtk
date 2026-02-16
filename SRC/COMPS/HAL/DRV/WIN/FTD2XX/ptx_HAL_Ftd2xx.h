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
    File        : ptx_HAL_Ftd2xx.h

    Description : FTDI Driver Implementation for Windows
*/

#if !defined(IO_ptx_HAL_FTD2XX_H_)
#define IO_ptx_HAL_FTD2XX_H_

/*
 * ####################################################################################################################
 * INCLUDE
 * ####################################################################################################################
 */
#include <stddef.h>
#include <stdint.h>
#include "ftd2xx.h"
#include "ptx_HAL_WinUtils.h"
#include "ptx_Status.h"
#include "ptx_Log.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/*
 * ####################################################################################################################
 * DEFINE
 * ####################################################################################################################
 */
#define MODULE_DESCRIPTOR       "PTX2K\0"

/**
 * FTDI modem status definitions.
 */
#define FT_MODEM_STATUS_CTS 0x00000010
#define FT_MODEM_STATUS_DSR 0x00000020
#define FT_MODEM_STATUS_RI  0x00000040
#define FT_MODEM_STATUS_DCD 0x00000080

#define FT_LINE_STATUS_OE 0x00000200
#define FT_LINE_STATUS_PE 0x00000400
#define FT_LINE_STATUS_FE 0x00000800
#define FT_LINE_STATUS_BI 0x00001000

/** Direction configuration of default spi pins
 * 1011 =
 * bit3: CS     output
 * bit2: MISO   input
 * bit1: MOSI   output
 * bit0: SCK    output
 */
#define FT_DIRECTION_CONFIG_DEFAULT_SPI_PINS 0x0B
/* #define FT_DIRECTION_CONFIG_DEFAULT_SPI_PINS 0xFB */

#define FT_NSS_PIN_VAL_DISABLE_PIN      0x08

#define FT_MOSI_PIN_VAL_HI              0x02

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * FTDI GPIO line used for IRQ polling on MPSSE I2C and SPI.
 */
typedef enum ptx_HAL_FTD2XX_GPIO
{
    /** FT232H */
    GPIOH0 = 0x01, // pin C0
    GPIOH1 = 0x02, // pin C1
    GPIOH2 = 0x04, // pin C2
    GPIOH3 = 0x08, // pin C3
    GPIOH4 = 0x10, // pin C4
    GPIOH5 = 0x20, // pin C5
    GPIOH6 = 0x40, // pin C6
    GPIOH7 = 0x80, // pin C7

    /** If needed in the future */
    //GPIOL0 = 0x10, // pin D4
    //GPIOL1 = 0x20, // pin D5
    //GPIOL2 = 0x40, // pin D6
    //GPIOL3 = 0x80, // pin D7
} ptx_HAL_FTD2XX_GPIO_t;

#define FT_IRQ_LINE_GPIO_BIT    GPIOH0 /* Pin C0 on FTDI board connected to IRQ line on PTX2xxR/W side */
#define FT_IRQ_LINE_GPIO_BIT_LO ~GPIOH0

#define FT_LATENCY_TIMER_SPI 1
#define FT_LATENCY_TIMER_I2C 2

/**
 * Functions pointers for the d2xx library.
 */
typedef FT_STATUS (*pCpD2xx_Write_t)(FT_HANDLE ftHandle, LPVOID LpBuffer, DWORD dwBytesToWrite, LPDWORD LpBytesWritten);
typedef FT_STATUS (*pCpD2xx_Read_t)(FT_HANDLE ftHandle, LPVOID LpBuffer, DWORD DwBytesToRead, LPDWORD LpBytesWritten);
typedef FT_STATUS (*pCpD2xx_GetStatus_t)(FT_HANDLE ftHandle, LPDWORD lpdwAmountInRxQueue, LPDWORD lpdwAmountInTxQueue, LPDWORD lpdwEventStatus);
typedef FT_STATUS (*pCpD2xx_Purge_t)(FT_HANDLE ftHandle, DWORD dwMask);
typedef FT_STATUS (*pCpD2xx_QueueStatus_t)(FT_HANDLE ftHandle, LPDWORD lpdwAmountInRxQueue);
typedef FT_STATUS (*pCpD2xx_Open_t)(int iDevice, FT_HANDLE *ftHandle);
typedef FT_STATUS (*pCpD2xx_OpenEX_t)(PVOID pArg1, DWORD Flags, FT_HANDLE *pHandle);
typedef FT_STATUS (*pCpD2xx_FT_ListDevices_t)(PVOID pvArg1, PVOID pvArg2, DWORD dwFlagsPVOID);
typedef FT_STATUS (*pCpD2xx_Close_t)(FT_HANDLE ftHandle);
typedef FT_STATUS (*pCpD2xx_SetBitMode_t)(FT_HANDLE ftHandle, UCHAR ucMask, UCHAR ucEnable);
typedef FT_STATUS (*pCpD2xx_GetBitMode_t)(FT_HANDLE ftHandle, PUCHAR pucMode);
typedef FT_STATUS (*pCpD2xx_SetFlowControl_t)(FT_HANDLE ftHandle, USHORT FlowControl, UCHAR XonChar, UCHAR XoffChar);
typedef FT_STATUS (*pCpD2xx_SetLatencyTimer_t)(FT_HANDLE ftHandle, CHAR ucLatency);
typedef FT_STATUS (*pCpD2xx_SetUSBParameters_t)(FT_HANDLE ftHandle, ULONG ulInTransferSize, ULONG ulOutTransferSize);
typedef FT_STATUS (*pCpD2xx_ResetDevice_t)(FT_HANDLE ftHandle);
typedef FT_STATUS (*pCpD2xx_SetChars_t)(FT_HANDLE ftHandle, UCHAR uEventCh, UCHAR uEventChEn, UCHAR uErrorCh, UCHAR uErrorChEn);
typedef FT_STATUS (*pCpD2xx_SetTimeouts_t)(FT_HANDLE ftHandle, DWORD dwReadTimeout, DWORD dwWriteTimeout);
typedef FT_STATUS (*pCpD2xx_SetBaudRate_t)(FT_HANDLE ftHandle, ULONG ulBaudRate);
typedef FT_STATUS (*pCpD2xx_SetRts_t)(FT_HANDLE ftHandle);
typedef FT_STATUS (*pCpD2xx_ClrRts_t)(FT_HANDLE ftHandle);
typedef FT_STATUS (*pCpD2xx_GetModemStatus_t)(FT_HANDLE ftHandle, ULONG *pModemStatus);
typedef FT_STATUS (*pCpD2xx_SetDataCharacteristics_t)(FT_HANDLE ftHandle, UCHAR uWordLength, UCHAR uStopBits, UCHAR uParity);
typedef FT_STATUS (*pCpD2xx_SetEventNotification_t)(FT_HANDLE ftHandle, DWORD dwEventMask, PVOID pvArg);
typedef FT_STATUS (*pCpD2xx_CreateDeviceInfoList_t)(LPDWORD lpdwNumDevs);

/**
 * Component FTDI D2XX structure.
 */
typedef struct ptx_HAL_FTD2XXCtx
{
    /* Component administrative. */
    ptx_Log_t       *Log;
    ptx_Win_Lib_t   *LH;                            /**< DLL handle. */
    void *H;
    ptx_Win_Mutex_t                 Mutex;          /** Mutex for TRx and IRQ read methods */
    ptx_Status_t                    StatusCmd;
    FT_STATUS                       FtStatus;
    ptx_HAL_FTD2XX_GPIO_t           IRQPin_Low;     /**< IRQ GPIO Pin low byte*/
    ptx_HAL_FTD2XX_GPIO_t           IRQPin_High;    /**< IRQ GPIO Pin high byte*/

    /* Component parameters. */
    char *InterfaceName;           /**< String used for Opening the FTDI device name based on Descriptor Name.
                                        Shall contain '\0' as last character. */
    char *DevSerialNumber;         /**< String used for Opening the FTDI device name based on Serial Number.
                                        Shall contain '\0' as last character. */
    uint8_t I2C_NoStopFlag;        /**< Flag used for I2C Restart Condition */
    uint8_t SPI_KeepNssActiveFlag; /**< Flag to keep chip select i.e. SPI slave in active state. */
    uint8_t WaitForIrqFlag;        /**< Enter/Exit control of the GPIO IRQ waiting loop. */

    /* Pointer functions to load from external dll. */
    pCpD2xx_Open_t              FT_Open;
    pCpD2xx_OpenEX_t            FT_OpenEx;
    pCpD2xx_FT_ListDevices_t    FT_ListDevices;
    pCpD2xx_Close_t             FT_Close;
    pCpD2xx_Write_t             FT_Write;
    pCpD2xx_Read_t              FT_Read;
    pCpD2xx_GetStatus_t         FT_GetStatus;
    pCpD2xx_Purge_t             FT_Purge;
    pCpD2xx_QueueStatus_t       FT_GetQueueStatus;
    pCpD2xx_SetBitMode_t        FT_SetBitMode;
    pCpD2xx_GetBitMode_t        FT_GetBitMode;
    pCpD2xx_SetFlowControl_t    FT_SetFlowControl;
    pCpD2xx_SetLatencyTimer_t   FT_SetLatencyTimer;
    pCpD2xx_SetUSBParameters_t  FT_SetUSBParameters;
    pCpD2xx_ResetDevice_t       FT_ResetDevice;
    pCpD2xx_SetChars_t          FT_SetChars;
    pCpD2xx_SetTimeouts_t       FT_SetTimeouts;
    pCpD2xx_SetBaudRate_t       FT_SetBaudRate;

    pCpD2xx_SetRts_t            FT_SetRts;
    pCpD2xx_ClrRts_t            FT_ClrRts;
    pCpD2xx_GetModemStatus_t    FT_GetModemStatus;
    pCpD2xx_SetDataCharacteristics_t FT_SetDataCharacteristics;
    pCpD2xx_SetEventNotification_t  FT_SetEventNotification;
    pCpD2xx_CreateDeviceInfoList_t  FT_CreateDeviceInfoList;
} ptx_HAL_FTD2XXCtx_t;

typedef enum ptx_HAL_FTD2XX_I2C_TypOp
{
    I2C_Wr,
    I2C_Rd
} ptx_HAL_FTD2XX_I2C_TypOp_t;

typedef enum ptx_HAL_FTD2XX_I2C_TypAck
{
    I2C_ACK,
    I2C_NACK
} ptx_HAL_FTD2XX_I2C_TypAck_t;

typedef enum ptx_HAL_FTD2XX_Soft_I2C_TypAck
{
    SOFT_I2C_NOACK  = 0,
    SOFT_I2C_ACK    = 1,
    SOFT_I2C_NACK   = 2
} ptx_HAL_FTD2XX_Soft_I2C_TypAck_t;

typedef enum ptx_HAL_FTD2XX_Input_Strings
{
    FTDI_STRING_INVALID     = 0,
    FTDI_SERIAL_VALID       = 1,
    FTDI_SERIAL_NULL        = 2,
    FTDI_DESCRIPTOR_VALID   = 3
} ptx_HAL_FTD2XX_Input_Strings_t;

typedef enum ptx_HAL_FTD2XX_Open_Type
{
    FTDI_DEVICE_OPEN_TYPE_UNDEFINED = 0,
    FTDI_DEVICE_OPEN_BY_SERIAL      = 1,
    FTDI_DEVICE_OPEN_BY_DESCRIPTOR  = 2
} ptx_HAL_FTD2XX_Open_Type_t;

typedef enum ptx_HAL_FTD2XX_Device_Interface
{
    FTDI_DEV_INTF_NONE = 0,
    FTDI_DEV_INTF_I2C,
    FTDI_DEV_INTF_SPI,
    FTDI_DEV_INTF_UART,
    FTDI_DEV_INTF_SW_I2C,
    FTDI_DEV_INTF_MPSSE_ONLY,
} ptx_HAL_FTD2XX_Device_Interface_t;

/*
* ####################################################################################################################
* EXTERN definitions.
* ####################################################################################################################
*/
extern const char serialAppendI2c[2];
extern const char InterfaceNameAppendI2c[3];
extern const char serialAppendSpi[2];
extern const char InterfaceNameAppendSpi[3];
extern const char serialAppendUart[2];
extern const char InterfaceNameAppendUart[3];
extern const char serialAppendSoftI2c[2];
extern const char InterfaceNameAppendSoftI2c[3];

/*
* ####################################################################################################################
* FUNCTIONS LIBRARY.
* ####################################################################################################################
*/
ptx_Status_t ptx_HAL_FTD2XX_LibOpen(void **ftd2xxCtx, char *InterfaceName, char *devSerialNumber);
ptx_Status_t ptx_HAL_FTD2XX_LibClose(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_SingleDeviceOpen(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, ptx_HAL_FTD2XX_Device_Interface_t deviceChannel);
ptx_Status_t ptx_HAL_FTD2XX_CheckOpening(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, ptx_HAL_FTD2XX_Device_Interface_t intfType, uint32_t busSpeed);

/*
* ####################################################################################################################
* FUNCTIONS - COMMON: I2C & SPI.
* ####################################################################################################################
*/
ptx_Status_t ptx_HAL_FTD2XX_CancelWaitIrq(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_StartWaitIrq(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_WaitForInterrupt(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_GetGPIOLevel(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, ptx_HAL_FTD2XX_GPIO_t gpio, uint16_t *level);
ptx_Status_t ptx_HAL_FTD2XX_SetGPIOLevel(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, ptx_HAL_FTD2XX_GPIO_t gpio, uint16_t level);
ptx_Status_t ptx_HAL_FTD2XX_GetHighPinsState(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *current_state);
ptx_Status_t ptx_HAL_FTD2XX_SetHighPinsState(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint16_t level, uint16_t mask);
ptx_Status_t ptx_HAL_FTD2XX_GetIRQLine(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint16_t *level);

/*
* ####################################################################################################################
* FUNCTIONS I2C.
* ####################################################################################################################
*/
ptx_Status_t ptx_HAL_FTD2XX_I2C_Open(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t busSpeed);
ptx_Status_t ptx_HAL_FTD2XX_I2C_SetSpeed(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t busSpeed);
ptx_Status_t ptx_HAL_FTD2XX_I2C_TRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t busAddr, uint8_t *wrBuf, uint32_t *wrLen, uint8_t *rdBuf, uint32_t *rdLen, uint8_t restart);
ptx_Status_t ptx_HAL_FTD2XX_I2C_Close(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_I2C_SetNotStop(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_I2C_ClearNotStop(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_I2C_ScanBus(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint16_t *addr_list, uint16_t *found_count);

/*
* ####################################################################################################################
* FUNCTIONS SW I2C.
* ####################################################################################################################
*/
ptx_Status_t ptx_HAL_FTD2XX_SW_I2C_Open(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_SW_I2C_Close(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_SW_I2C_TRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t busAddr, uint8_t *wrBuf, uint32_t *wrLen, uint8_t *rdBuf, uint32_t *rdLen);
ptx_Status_t ptx_HAL_FTD2XX_SW_I2C_SetStopCondition(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_SW_I2C_SetStartCondition(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);

/*
* ####################################################################################################################
* FUNCTIONS SPI.
* ####################################################################################################################
*/
ptx_Status_t ptx_HAL_FTD2XX_SPI_Open(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t busSpeed);
ptx_Status_t ptx_HAL_FTD2XX_SPI_Close(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_SPI_SetKeepNssActive(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_SPI_ClearKeepNssActive(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_SPICSEnable(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_SPICSDisable(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_SPI_TRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *wrBuf, uint32_t *wrLen, uint8_t *rdBuf, uint32_t *rdLen);

/*
* ####################################################################################################################
* FUNCTIONS UART.
* ####################################################################################################################
*/
ptx_Status_t ptx_HAL_FTD2XX_UART_Open(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t busSpeed);
ptx_Status_t ptx_HAL_FTD2XX_UART_Close(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_UART_TRx(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t *wrBuf, uint32_t *wrLen, uint8_t *rdBuf, uint32_t *rdLen);
ptx_Status_t ptx_HAL_FTD2XX_UART_TRx_StaticCtx(uint8_t *wrBuf, uint32_t *wrLen, uint8_t *rdBuf, uint32_t *rdLen);
ptx_Status_t ptx_HAL_FTD2XX_UART_SetSpeed(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t busSpeed);
ptx_Status_t ptx_HAL_FTD2XX_UART_SetRts(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_UART_ClrRts(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx);
ptx_Status_t ptx_HAL_FTD2XX_UART_SetFlowControl(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t enableFc);
ptx_Status_t ptx_HAL_FTD2XX_UART_SetDataCharacteristics(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint8_t WordLength, uint8_t StopBits, uint8_t Parity);
ptx_Status_t ptx_HAL_FTD2XX_UART_GetModemStatus(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t *modemStatus);
ptx_Status_t ptx_HAL_FTD2XX_UART_SetTimeout(ptx_HAL_FTD2XXCtx_t *ftd2xxCtx, uint32_t timeout);

#if defined(__cplusplus)
}
#endif

#endif /* IO_ptx_HAL_FTD2XX_H_ */
