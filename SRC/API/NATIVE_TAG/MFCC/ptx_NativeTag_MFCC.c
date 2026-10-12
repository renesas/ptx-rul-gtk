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
    Module      : API for interfacing with a MFCC card
    File        : ptx_NativeTag_MFCC.c

    Description : This contains an API that can be used to interface with MFCC cards.
                  In order to use this component, a third-party Crypto-1 implementation is
                  required and should be implemented in the ptx_NativeTag_MFCCCrypto1.c file.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NativeTag_MFCC.h"
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * Mifare Classic Command Codes.
 *
 */
#define PTX_MFCC_COMMAND_CODE_AUTHENTICATE_A    (uint8_t)0x60       /* AUTHENTICATE CMD Type A. */
#define PTX_MFCC_COMMAND_CODE_AUTHENTICATE_B    (uint8_t)0x61       /* AUTHENTICATE CMD Type B. */
#define PTX_MFCC_COMMAND_CODE_READ              (uint8_t)0x30       /* READ CMD. */
#define PTX_MFCC_COMMAND_CODE_WRITE             (uint8_t)0xA0       /* WRITE CMD. */
#define PTX_MFCC_COMMAND_CODE_INCREMENT         (uint8_t)0xC1       /* INCREMENT CMD. */
#define PTX_MFCC_COMMAND_CODE_DECREMENT         (uint8_t)0xC0       /* DECREMENT CMD. */
#define PTX_MFCC_COMMAND_CODE_RESTORE           (uint8_t)0xC2       /* RESTORE CMD. */
#define PTX_MFCC_COMMAND_CODE_TRANSFER          (uint8_t)0xB0       /* TRANSFER CMD. */
#define PTX_MFCC_COMMAND_CODE_HALT_A            (uint8_t)0x50       /* HALT CMD Type A. */
#define PTX_MFCC_COMMAND_CODE_ACK               (uint8_t)0x0A       /* ACK Byte. */
#define PTX_MFCC_COMMAND_CODE_NAK_0             (uint8_t)0x00       /* NAK 0 Byte. */
#define PTX_MFCC_COMMAND_CODE_NAK_1             (uint8_t)0x01       /* NAK 1 Byte. */
#define PTX_MFCC_COMMAND_CODE_NAK_4             (uint8_t)0x04       /* NAK 4 Byte. */
#define PTX_MFCC_COMMAND_CODE_NAK_5             (uint8_t)0x05       /* NAK 5 Byte. */
/*  */

/*
 *  Internal Parity Enum.
 */
typedef enum ptx_NativeTag_MFCC_ParityType
{
    ParityType_Odd,             /* Odd parity. */
    ParityType_Even,            /* Even parity. */

} ptx_NativeTag_MFCC_ParityType_t;

/*
 * Internal Defines.
 *
 */
#define TEST_BIT(x, n) ((x) >> (n) & 1)                     /* Bit test macro. */
#define PTX_MFCC_UID_AUTHENT_SIZE           (uint8_t)4u     /* UID size. */
#define PTX_MFCC_KEY_SIZE                   (uint8_t)6u     /* Key size. */
#define PTX_MFCC_APP_TIMEOUT_MS             (uint32_t)300u  /* Default RF timeout. */
#define PTX_MFCC_DATA_BLOCK_SIZE            (uint8_t)16u    /* Size of one data block. */
#define PTX_MFCC_WORK_BUFFER_SIZE           (uint8_t)24u    /**< Internal work buffer size. */
#define PTX_MFCC_BITS_EXCHANGE_BUFFER_SIZE  (uint8_t)24u    /**< Min. buffer size for Bit-Exchange function. */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NativeTag_MFCC_BitsExchange(ptx_NativeTag_MFCC_t *mfcc, uint32_t *rxLength, uint32_t *numTotBits);
static uint64_t ptx_NativeTag_MFCC_BytesToNum(uint8_t *src, uint32_t len);
static void ptx_NativeTag_MFCC_NumToBytes(uint64_t num, uint32_t len, uint8_t *dst);
static uint16_t ptx_NativeTag_MFCC_CalcISO14443ACRC(uint8_t *data, uint32_t offset, uint32_t length);
static uint16_t ptx_NativeTag_MFCC_CalcCRC16 (uint16_t initValue,
                                              uint16_t polynom,
                                              uint8_t shiftLSBFirst,
                                              uint8_t invertCRC,
                                              uint8_t *data,
                                              uint32_t offset,
                                              uint32_t length);
static uint16_t ptx_NativeTag_MFCC_ReverseBitOrder(uint8_t bitWidth, uint16_t x);
static uint8_t ptx_NativeTag_MFCC_GetParity(ptx_NativeTag_MFCC_ParityType_t parityType, const uint8_t x);
static ptx_Status_t ptx_NativeTag_MFCC_TRxCommand(ptx_NativeTag_MFCC_t *mfcc,
                                                  uint8_t isCrypted,
                                                  uint8_t isAuthentCmd,
                                                  uint8_t command,
                                                  uint8_t commandParam,
                                                  uint8_t *response,
                                                  uint32_t *responseLen);
static ptx_Status_t ptx_NativeTag_MFCC_CheckACK(uint8_t value, uint32_t rxLength, ptx_Status_t currentStatus);
static ptx_Status_t ptx_NativeTag_MFCC_EncryptData(ptx_NativeTag_MFCC_t *mfcc, uint8_t *data, uint32_t *length);
static ptx_Status_t ptx_NativeTag_MFCC_Decrypt4BitResponse(ptx_NativeTag_MFCC_t *mfcc, uint8_t enryptedResponse, uint8_t *plainResponse);
static ptx_Status_t ptx_NativeTag_MFCC_HandleValueCommand (ptx_NativeTag_MFCC_t *mfcc, uint8_t valueCommandCode, uint8_t address, uint32_t value);

/*
 *  Odd byte parity array.
 */
const uint8_t ptx_NativeTag_MFCC_OddByteParity[256] =
{
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NativeTag_MFCC_Open (ptx_NativeTag_MFCC_t *mfcc, ptx_NativeTag_MFCC_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != mfcc) && (NULL != initParams))
    {
        if ((NULL != initParams->Rul))
        {
            /* clear component */
            (void)memset(mfcc, 0, sizeof(ptx_NativeTag_MFCC_t));

            /* set members */
            mfcc->Rul            = initParams->Rul;
            mfcc->RxData.Data    = &mfcc->RxByteBuffer[0];
            mfcc->RxData.Parity  = &mfcc->RxBitBuffer[0];
            mfcc->RxData.DataLen = (uint32_t)PTX_MFCC_BITS_EXCHANGE_BUFFER_SIZE;
            mfcc->TxData.Data    = &mfcc->TxByteBuffer[0];
            mfcc->TxData.Parity  = &mfcc->TxBitBuffer[0];
            mfcc->TxData.DataLen = (uint32_t)PTX_MFCC_BITS_EXCHANGE_BUFFER_SIZE;

            status = ptx_NativeTag_MFCCCrypto1_Open(&mfcc->Crypto1Params);

            /* set Component-ID at the end to prevent further calls in case of an error */
            if (PTX_STATUS_SUCCESS == status)
            {
                mfcc->Crypto1Params.CryptoState = initParams->UserContextCrypto1;

                mfcc->CompId = Comp_NativeTag_MFCC;

                status = ptx_NativeTag_MFCC_ResetState(mfcc);
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_MFCC_Enable (ptx_NativeTag_MFCC_t *mfcc, ptx_NativeTag_MFCC_Mode_t enableMode)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC))
    {
        status = ptx_RUL_BitsExchangeMode (mfcc->Rul, (ptx_RUL_BitsExchangeMode_t)enableMode);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* reset internal Crypto-/Authenticated-state */
            mfcc->IsCrypted = 0u;
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_MFCC_ResetState (ptx_NativeTag_MFCC_t *mfcc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC))
    {
        ptx_NativeTag_MFCCCrypto1_Reset(&mfcc->Crypto1Params);

        /* reset internal Crypto-/Authenticated-state */
        mfcc->IsCrypted = 0u;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NativeTag_MFCC_Authenticate (ptx_NativeTag_MFCC_t *mfcc,
                                              ptx_NativeTag_MFCC_KeyType_t keyType,
                                              uint8_t address,
                                              uint8_t *key,
                                              uint8_t keyLen,
                                              uint8_t *uid,
                                              uint8_t uidLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t rx[10] = {0};
    uint32_t rx_len = sizeof(rx);
    uint8_t nr[4];
    uint32_t nt, ntpp;
    uint64_t key_num;
    uint32_t uid_num;
    uint8_t command_code = keyType == MFCC_Key_Type_A ? PTX_MFCC_COMMAND_CODE_AUTHENTICATE_A : PTX_MFCC_COMMAND_CODE_AUTHENTICATE_B;
    uint32_t total_nr_bits = 0;
    int i;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC) &&
        (NULL != key) &&
        (keyLen == PTX_MFCC_KEY_SIZE) &&
        (uid != key) &&
        (uidLen == PTX_MFCC_UID_AUTHENT_SIZE))
    {
        /* prepare key & UID */
        key_num = ptx_NativeTag_MFCC_BytesToNum(&key[0], 6u);
        uid_num = (uint32_t)ptx_NativeTag_MFCC_BytesToNum(&uid[0], 4u);

        /* send Part-1 of AUTHENTICATE-command */
        status = ptx_NativeTag_MFCC_TRxCommand(mfcc, mfcc->IsCrypted, 1u, command_code, address, &rx[0], &rx_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Tag Nonce NT received ? */
            if (4u == rx_len)
            {
                /* reset internal Crypto-1 states if already in encryption mode */
                if (0 != mfcc->IsCrypted)
                {
                    ptx_NativeTag_MFCCCrypto1_Reset(&mfcc->Crypto1Params);
                }

                /* initialize Crypto-1 with given Key */
                ptx_NativeTag_MFCCCrypto1_Set(&mfcc->Crypto1Params, key_num);

                /* store NT */
                nt = (uint32_t)ptx_NativeTag_MFCC_BytesToNum(&rx[0], 4u);

                /* load NT into Crypto-1 (dependent on current crypto-state) */
                if  (0 != mfcc->IsCrypted)
                {
                    nt = ptx_NativeTag_MFCCCrypto1_Word(&mfcc->Crypto1Params, nt ^ uid_num, 1) ^ nt;
                }
                else
                {
                    ptx_NativeTag_MFCCCrypto1_Word(&mfcc->Crypto1Params, nt ^ uid_num, 0);
                }

                /* generate Reader Nonce NR */
                for (i = 0; i < 4; i++)
                {
                    nr[i] = ptx_NativeTag_MFCCCrypto1_GetRandomByte(&mfcc->Crypto1Params);
                }

                /* encrypt NR */
                for (i = 0; i < 4; i++)
                {
                    mfcc->TxData.Data[i]    = (uint8_t) (ptx_NativeTag_MFCCCrypto1_Byte(&mfcc->Crypto1Params, nr[i], 0) ^ nr[i]);
                    mfcc->TxData.Parity[i]  = (uint8_t)((ptx_NativeTag_MFCCCrypto1_Filter(&mfcc->Crypto1Params) ^ ptx_NativeTag_MFCC_GetParity(ParityType_Odd, nr[i])) & 0x01);
                }

                /* shift NT through internal PRNG */
                nt = ptx_NativeTag_MFCCCrypto1_ShiftPRNG(&mfcc->Crypto1Params, nt, 32u);

                /* encrypt NT */
                for (i = 4; i < 8; i++)
                {
                    nt = ptx_NativeTag_MFCCCrypto1_ShiftPRNG(&mfcc->Crypto1Params, nt, 8u);
                    mfcc->TxData.Data[i]    = (uint8_t) (ptx_NativeTag_MFCCCrypto1_Byte(&mfcc->Crypto1Params, 0x00, 0) ^ (nt & 0xFF));
                    mfcc->TxData.Parity[i] = (uint8_t)((ptx_NativeTag_MFCCCrypto1_Filter(&mfcc->Crypto1Params) ^ ptx_NativeTag_MFCC_GetParity(ParityType_Odd, (uint8_t)nt)) & 0x01);
                }

                /* exchange Type-A bitstream (Tx- and Rx-Buffers are contained in MFCC-component) */
                rx_len = sizeof(rx);
                mfcc->TxData.DataLen = 8u;
                status = ptx_NativeTag_MFCC_BitsExchange(mfcc, &rx_len, &total_nr_bits);

                if (PTX_STATUS_SUCCESS == status)
                {
                    if (4u == rx_len)
                    {
                        /* check if received encrypted NT matches previously received plain NT */
                        ntpp = ptx_NativeTag_MFCCCrypto1_ShiftPRNG(&mfcc->Crypto1Params, nt, 32u) ^ ptx_NativeTag_MFCCCrypto1_Word(&mfcc->Crypto1Params, 0x00, 0);

                        if (ntpp == ptx_NativeTag_MFCC_BytesToNum(&mfcc->RxData.Data[0], 4u))
                        {
                            /* Match! - Authentication OK, from now on, every following communication (Tx + Rx) is encrypted */
                            mfcc->IsCrypted = 1u;

                        }
                        else
                        {
                            status = PTX_STATUS_E_MFCC_AUTHENTICATION_FAILED;
                        }
                    }
                    else
                    {
                        status = PTX_STATUS_E_MFCC_AUTHENTICATION_FAILED;
                    }
                }
            }
            else
            {
                status = PTX_STATUS_E_MFCC_AUTHENTICATION_FAILED;
            }
        }
        else
        {
            status = PTX_STATUS_E_MFCC_AUTHENTICATION_FAILED;
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_MFCC_Read (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint8_t *rx, uint32_t *rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC) && (NULL != rx) && (NULL != rxLen))
    {
        /* perform actual MFCC-command */
        status = ptx_NativeTag_MFCC_TRxCommand(mfcc, mfcc->IsCrypted, 0, PTX_MFCC_COMMAND_CODE_READ, address, rx, rxLen);

        /* handle response */
        if (PTX_STATUS_SUCCESS == status)
        {
            if (1u >= *rxLen)
            {
                /* a Mifare-NAK is expected */
                status = ptx_NativeTag_MFCC_CheckACK(rx[0], *rxLen, status);
            }
            else
            {
                /* the response contained in "rx" still contains the CRC at the end - remove it */
                *rxLen = (uint32_t)(*rxLen - 2u);
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_MFCC_Write (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint8_t *blockData, uint8_t blockDataLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t rx[1];
    uint32_t rx_len = sizeof(rx);
    uint32_t tx_len;
    uint32_t total_nr_bits = 0;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC) && (NULL != blockData) && (blockDataLen == PTX_MFCC_DATA_BLOCK_SIZE))
    {
        /* perform actual MFCC-command -> Part 1 */
        status = ptx_NativeTag_MFCC_TRxCommand(mfcc, mfcc->IsCrypted, 0, PTX_MFCC_COMMAND_CODE_WRITE, address, &rx[0], &rx_len);

        /* a Mifare-ACK is expected */
        status = ptx_NativeTag_MFCC_CheckACK(rx[0], rx_len, status);

        /* proceed with Part 2*/
        if (PTX_STATUS_SUCCESS == status)
        {
            /* encrypt data to write */
            tx_len = (uint32_t)blockDataLen;
            status = ptx_NativeTag_MFCC_EncryptData(mfcc, &blockData[0], &tx_len);

            /* send encrypted data */
            if (PTX_STATUS_SUCCESS == status)
            {
                rx_len = sizeof(rx);
                mfcc->TxData.DataLen = tx_len;
                status = ptx_NativeTag_MFCC_BitsExchange(mfcc, &rx_len, &total_nr_bits);

                if (PTX_STATUS_SUCCESS == status)
                {
                    if (1u == rx_len)
                    {
                        /* decrypt ACK / NAK */
                        status = ptx_NativeTag_MFCC_Decrypt4BitResponse(mfcc, mfcc->RxData.Data[0], &rx[0]);
                    }

                    /* a Mifare-ACK is expected */
                    status = ptx_NativeTag_MFCC_CheckACK(rx[0], rx_len, status);
                }
            }
        }

    }

    return status;
}

ptx_Status_t ptx_NativeTag_MFCC_Increment (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint32_t value)
{
    uint32_t value_reversed = (uint32_t)((value & 0x000000FF) << 24u) |
                              ((value & 0x0000FF00) << 8u) |
                              ((value & 0x00FF0000) >> 8u) |
                              ((value & 0xFF000000) >> 24u);
    return ptx_NativeTag_MFCC_HandleValueCommand(mfcc, PTX_MFCC_COMMAND_CODE_INCREMENT, address, value_reversed);
}

ptx_Status_t ptx_NativeTag_MFCC_Decrement (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint32_t value)
{
    uint32_t value_reversed = (uint32_t)((value & 0x000000FF) << 24u) |
                              ((value & 0x0000FF00) << 8u) |
                              ((value & 0x00FF0000) >> 8u) |
                              ((value & 0xFF000000) >> 24u);
    return ptx_NativeTag_MFCC_HandleValueCommand(mfcc, PTX_MFCC_COMMAND_CODE_DECREMENT, address, value_reversed);
}

ptx_Status_t ptx_NativeTag_MFCC_Transfer (ptx_NativeTag_MFCC_t *mfcc, uint8_t address)
{
    return ptx_NativeTag_MFCC_HandleValueCommand(mfcc, PTX_MFCC_COMMAND_CODE_TRANSFER, address, 0);
}

ptx_Status_t ptx_NativeTag_MFCC_Restore (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint32_t value)
{
    return ptx_NativeTag_MFCC_HandleValueCommand(mfcc, PTX_MFCC_COMMAND_CODE_RESTORE, address, value);
}

ptx_Status_t ptx_NativeTag_MFCC_GetFormattedValueBlock (ptx_NativeTag_MFCC_t *mfcc, uint8_t address, uint32_t value, uint8_t *buffer, uint8_t bufferSize)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC) && (buffer != NULL) && (PTX_MFCC_DATA_BLOCK_SIZE <= bufferSize))
    {
        /* store value normal */
        buffer[0]  = (uint8_t)((value >>  0) & (uint8_t)0xFF);
        buffer[1]  = (uint8_t)((value >>  8) & (uint8_t)0xFF);
        buffer[2]  = (uint8_t)((value >> 16) & (uint8_t)0xFF);
        buffer[3]  = (uint8_t)((value >> 24) & (uint8_t)0xFF);

        /* store value inverted */
        buffer[4]  = (uint8_t)((~value >>  0) & (uint8_t)0xFF);
        buffer[5]  = (uint8_t)((~value >>  8) & (uint8_t)0xFF);
        buffer[6]  = (uint8_t)((~value >> 16) & (uint8_t)0xFF);
        buffer[7]  = (uint8_t)((~value >> 24) & (uint8_t)0xFF);

        /* store value normal again */
        buffer[8]  = (uint8_t)((value >>  0) & (uint8_t)0xFF);
        buffer[9]  = (uint8_t)((value >>  8) & (uint8_t)0xFF);
        buffer[10] = (uint8_t)((value >> 16) & (uint8_t)0xFF);
        buffer[11] = (uint8_t)((value >> 24) & (uint8_t)0xFF);

        /* store address normal + inverted */
        buffer[12] = (uint8_t)(address);
        buffer[13] = (uint8_t)(~address);
        buffer[14] = (uint8_t)(address);
        buffer[15] = (uint8_t)(~address);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NativeTag_MFCC_Close (ptx_NativeTag_MFCC_t *mfcc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC))
    {
        status = ptx_NativeTag_MFCCCrypto1_Close(&mfcc->Crypto1Params);

        if (PTX_STATUS_SUCCESS == status)
        {
            memset(mfcc, 0, sizeof(ptx_NativeTag_MFCC_t));
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_MFCC_Halt (ptx_NativeTag_MFCC_t *mfcc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC))
    {
        status = ptx_NativeTag_MFCC_HandleValueCommand(mfcc, PTX_MFCC_COMMAND_CODE_HALT_A, 0, 0);

        /* HALT-A doesn't return a response, so a Timeout-/RF-Error is expected */
        if (PTX_STATUS_E_NSC_RFERROR == status)
        {
            /* revert status */
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK(s)
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NativeTag_MFCC_BitsExchange(ptx_NativeTag_MFCC_t *mfcc, uint32_t *rxLength, uint32_t *numTotBits)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t num_tot_bits = 0;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC) && (NULL != rxLength) && (NULL != numTotBits))
    {
        num_tot_bits = *numTotBits;
        mfcc->RxData.DataLen = *rxLength;

        status = ptx_RUL_BitsExchange (mfcc->Rul,
                                       &mfcc->TxData,
                                       &mfcc->RxData,
                                       &num_tot_bits,
                                       PTX_MFCC_APP_TIMEOUT_MS);

        *numTotBits = (uint32_t)num_tot_bits;

        if (PTX_STATUS_SUCCESS == status)
        {
            *rxLength = mfcc->RxData.DataLen;
        }
        else
        {
            *rxLength = 0;
        }

    }

    return status;
}
static uint64_t ptx_NativeTag_MFCC_BytesToNum(uint8_t *src, uint32_t len)
{
    /* "src"-pointer expected to be valid here */

    uint64_t num = 0;

    while (len--)
    {
        num = (num << 8) | (*src);
        src++;
    }

    return num;
}

static void ptx_NativeTag_MFCC_NumToBytes(uint64_t num, uint32_t len, uint8_t *dst)
{
    /* "dst"-pointer expected to be valid here */
    while (len--)
    {
        dst[len] = (uint8_t) num;
        num >>= 8;
    }
}

static uint16_t ptx_NativeTag_MFCC_CalcISO14443ACRC(uint8_t *data, uint32_t offset, uint32_t length)
{
    return ptx_NativeTag_MFCC_CalcCRC16((uint16_t)0x6363, (uint16_t)0x1021, 1u, 0u, data, offset, length);
}

static uint16_t ptx_NativeTag_MFCC_CalcCRC16 (uint16_t initValue,
                                              uint16_t polynom,
                                              uint8_t shiftLSBFirst,
                                              uint8_t invertCRC,
                                              uint8_t *data,
                                              uint32_t offset,
                                              uint32_t length)
{
    /* "data"-pointer expected to be valid here */

    uint16_t crc = initValue;
    uint16_t p, i;
    uint16_t work_poly;

    work_poly = (shiftLSBFirst != 0) ? ptx_NativeTag_MFCC_ReverseBitOrder((uint8_t)16, polynom) : polynom;

    for (p = 0; p < length; p++)
    {
        if (0 != shiftLSBFirst)
        {
            crc = (uint16_t) (crc ^ (data[offset + p] & 0xFF));

            for (i = 0; i < 8; i++)
            {
                crc = ((crc & 1) != 0) ? ((crc >> 1) ^ work_poly) : (crc >> 1);
            }
        }
        else
        {
            crc ^= (uint16_t) ((data[offset + p] & 0xFF) << 8);

            for (i = 0; i < 8; i++)
            {
                crc = (uint16_t) (((crc & 0x8000) != 0) ? (((crc << 1) & 0xFFFF) ^ work_poly) : ((crc << 1) & 0xFFFF));
            }
        }
    }

    if (0 != invertCRC)
    {
        crc = (uint16_t) ~crc;
    }

    return (crc & 0xFFFF);
}

static uint16_t ptx_NativeTag_MFCC_ReverseBitOrder(uint8_t bitWidth, uint16_t x)
{
    uint16_t b = 0;
    uint8_t i;

    for (i = 0 ; i < bitWidth; i++)
    {
        b = (uint16_t) (b << 1);
        b = (uint16_t) (b | (x & 1));
        x >>= 1;
    }

    return b;
}

static uint8_t ptx_NativeTag_MFCC_GetParity(ptx_NativeTag_MFCC_ParityType_t parityType, const uint8_t x)
{
    uint8_t parity = ptx_NativeTag_MFCC_OddByteParity[x];

    if (parityType == ParityType_Even)
    {
        parity = (uint8_t)(~parity & 1);
    }

    return parity;
}

static ptx_Status_t ptx_NativeTag_MFCC_TRxCommand(ptx_NativeTag_MFCC_t *mfcc,
                                                  uint8_t isCrypted,
                                                  uint8_t isAuthentCmd,
                                                  uint8_t command,
                                                  uint8_t commandParam,
                                                  uint8_t *response,
                                                  uint32_t *responseLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t index = 0;
    uint16_t crc_a;
    uint8_t work_buffer[4] = {0};
    uint32_t rx_len;
    uint32_t total_nr_bits = 0;
    int i;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC) && (NULL != response) && (NULL != responseLen))
    {
        work_buffer[index] = command;
        index++;

        work_buffer[index] = commandParam;
        index++;

        /* calculate CRC according to ISO/IEC 14443-A */
        crc_a = ptx_NativeTag_MFCC_CalcISO14443ACRC(&work_buffer[0], 0, index);

        work_buffer[index] = (uint8_t)((crc_a >> 0) & (uint8_t)0xFF);
        index++;
        work_buffer[index] = (uint8_t)((crc_a >> 8) & (uint8_t)0xFF);
        index++;

        /* encrypt bytes and (parity-)bits depending on Authenticate-state -> assemble Type-A bitstream */
        for (i = 0; i < (int)index; i++)
        {
            if (0 != isCrypted)
            {
                mfcc->TxData.Data[i]    = (uint8_t)(ptx_NativeTag_MFCCCrypto1_Byte(&mfcc->Crypto1Params, 0x00, 0) ^ work_buffer[i]);
                mfcc->TxData.Parity[i]  = (uint8_t)(ptx_NativeTag_MFCCCrypto1_Filter(&mfcc->Crypto1Params) ^ ptx_NativeTag_MFCC_GetParity(ParityType_Odd, work_buffer[i]));
                mfcc->TxData.DataLen = index;
            }
            else
            {
                mfcc->TxData.Data[i]    = work_buffer[i];
                mfcc->TxData.Parity[i]  = ptx_NativeTag_MFCC_GetParity(ParityType_Odd, work_buffer[i]);
                mfcc->TxData.DataLen = index;
            }
        }

        /* exchange Type-A bitstream (Tx- and Rx-Buffers are contained in MFCC-component) */
        rx_len = (uint32_t)PTX_MFCC_WORK_BUFFER_SIZE;
        status = ptx_NativeTag_MFCC_BitsExchange(mfcc, &rx_len, &total_nr_bits);

        if (status == PTX_STATUS_SUCCESS)
        {
            /* does response fit into output buffer ? */
            if (*responseLen >= rx_len)
            {
                if ((0 != isCrypted) && (0 == isAuthentCmd))
                {
                    if (1u == rx_len)
                    {
                        response[0] = 0;

                        for (i = 0; i < (int)4; i++)
                        {
                            response[0] = (uint8_t)(response[0] | (ptx_NativeTag_MFCCCrypto1_Bit(&mfcc->Crypto1Params, 0x00, 0) ^ TEST_BIT(mfcc->RxData.Data[0], i)) << i);
                        }

                    }
                    else
                    {
                        for (i = 0; i < (int)rx_len; i++)
                        {
                            response[i] = (uint8_t)(ptx_NativeTag_MFCCCrypto1_Byte(&mfcc->Crypto1Params, 0x00, 0) ^ mfcc->RxData.Data[i]);
                        }
                    }

                }
                else
                {
                    if (1u == rx_len)
                    {
                        response[0] = 0;

                        for (i = 0; i < (int)total_nr_bits; i++)
                        {
                            response[0] = (uint8_t)(response[0] | TEST_BIT(mfcc->RxData.Data[0], i) << i);
                        }

                    }
                    else
                    {
                        (void)memcpy(&response[0], &mfcc->RxData.Data[0], rx_len);
                    }
                }

                *responseLen = rx_len;

            }
            else
            {
                status = PTX_STATUS_E_MFCC_INSUFFICIENT_RESOURCES;
            }
        }

    }

    return status;
}

static ptx_Status_t ptx_NativeTag_MFCC_CheckACK(uint8_t value, uint32_t rxLength, ptx_Status_t currentStatus)
{
    ptx_Status_t status = currentStatus;

    if (PTX_STATUS_SUCCESS == status)
    {
        if (1u == rxLength)
        {
            switch (value)
            {
                case PTX_MFCC_COMMAND_CODE_ACK:
                    /* do nothing i.e. leave status == SUCCESS */
                    break;

                case PTX_MFCC_COMMAND_CODE_NAK_0:
                /* FALL THROUGH */
                case PTX_MFCC_COMMAND_CODE_NAK_1:
                /* FALL THROUGH */
                case PTX_MFCC_COMMAND_CODE_NAK_4:
                /* FALL THROUGH */
                case PTX_MFCC_COMMAND_CODE_NAK_5:
                    status = PTX_STATUS_E_MFCC_NACK;
                    break;

                default:
                    status = PTX_STATUS_E_MFCC_PROTOCOL_ERROR;
                    break;
            }
        }
        else
        {
            status = PTX_STATUS_E_MFCC_PROTOCOL_ERROR;
        }
    }

    return status;
}

static ptx_Status_t ptx_NativeTag_MFCC_EncryptData(ptx_NativeTag_MFCC_t *mfcc, uint8_t *data, uint32_t *length)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint16_t crc_a;
    uint8_t work_buffer[PTX_MFCC_DATA_BLOCK_SIZE + 2u];
    uint32_t index;
    int i;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC) && (NULL != data) && (NULL != length))
    {
        if ((uint32_t)PTX_MFCC_DATA_BLOCK_SIZE >= *length)
        {
            index = (uint32_t)(*length);

            /* set data */
            (void)memcpy(&work_buffer[0], &data[0], index);

            /* calculate CRC according to ISO/IEC 14443-A */
            crc_a = ptx_NativeTag_MFCC_CalcISO14443ACRC(&work_buffer[0], 0, index);

            work_buffer[index] = (uint8_t)((crc_a >> 0) & (uint8_t)0xFF);
            index++;
            work_buffer[index] = (uint8_t)((crc_a >> 8) & (uint8_t)0xFF);
            index++;

            /* encrypt bytes and (parity-)bits */
            for (i = 0; i < (int)index; i++)
            {
                mfcc->TxData.Data[i]    = (uint8_t)(ptx_NativeTag_MFCCCrypto1_Byte(&mfcc->Crypto1Params, 0x00, 0) ^ work_buffer[i]);
                mfcc->TxData.Parity[i]  = (uint8_t)(ptx_NativeTag_MFCCCrypto1_Filter(&mfcc->Crypto1Params) ^ ptx_NativeTag_MFCC_GetParity(ParityType_Odd, work_buffer[i]));
            }

            /* set output length */
            *length = index;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
    }

    return status;
}

static ptx_Status_t ptx_NativeTag_MFCC_Decrypt4BitResponse(ptx_NativeTag_MFCC_t *mfcc, uint8_t enryptedResponse, uint8_t *plainResponse)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t plain_value = 0;
    int i;

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC) && (NULL != plainResponse))
    {
        for (i = 0; i < (int)4; i++)
        {
            plain_value = (uint8_t)(plain_value | (ptx_NativeTag_MFCCCrypto1_Bit(&mfcc->Crypto1Params, 0x00, 0) ^ TEST_BIT(enryptedResponse, i)) << i);
        }

        /* set plain value */
        *plainResponse = plain_value;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_NativeTag_MFCC_HandleValueCommand (ptx_NativeTag_MFCC_t *mfcc, uint8_t valueCommandCode, uint8_t address, uint32_t value)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t rx[1];
    uint32_t rx_len = sizeof(rx);
    uint32_t tx_len;
    uint32_t total_nr_bits = 0;
    uint8_t value_buffer[4];

    if (PTX_COMP_CHECK(mfcc, Comp_NativeTag_MFCC))
    {
        /* perform actual MFCC-command -> Part 1 */
        status = ptx_NativeTag_MFCC_TRxCommand(mfcc, mfcc->IsCrypted, 0, valueCommandCode, address, &rx[0], &rx_len);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* a Mifare-ACK is expected */
            status = ptx_NativeTag_MFCC_CheckACK(rx[0], rx_len, status);
        }

        /* proceed with Part 2 (except TRANSFER-command) */
        if ((PTX_STATUS_SUCCESS == status) && (PTX_MFCC_COMMAND_CODE_TRANSFER != valueCommandCode))
        {
            /* prepare value buffer */
            ptx_NativeTag_MFCC_NumToBytes((uint64_t)value, 4u, &value_buffer[0]);

            /* encrypt data to write */
            tx_len = (uint32_t)4u;
            status = ptx_NativeTag_MFCC_EncryptData(mfcc, &value_buffer[0], &tx_len);

            /* send encrypted data */
            if (PTX_STATUS_SUCCESS == status)
            {
                rx_len = sizeof(rx);
                mfcc->TxData.DataLen = tx_len;
                status = ptx_NativeTag_MFCC_BitsExchange(mfcc, &rx_len, &total_nr_bits);

                /* Part 2 is expected to be successful in case of a timeout */
                if ((ptx_RUL_StatusInfoRfError_Timeout == mfcc->Rul->RfErrorStatus) && (PTX_STATUS_E_NSC_RFERROR == status))
                {
                    /* everything OK - overwrite status */
                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    status = ptx_NativeTag_MFCC_Decrypt4BitResponse(mfcc, mfcc->RxData.Data[0], &rx[0]);

                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* a Mifare-NAK is expected */
                        status = ptx_NativeTag_MFCC_CheckACK(rx[0], rx_len, status);
                    }
                }
            }
        }
    }

    return status;
}

