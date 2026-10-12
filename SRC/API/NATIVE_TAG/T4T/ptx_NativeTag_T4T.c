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
    Module      : API for interfacing with a Type 4 Tag (T4T) card
    File        : ptx_NativeTag_T4T.c

    Description : NativeTag T4T implements the interface to communicate with a T4T NFC card.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL.h"
#include "ptx_NativeTag_T4T.h"
#include <string.h>
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
/*
 * Instruction Codes.
 *
 */
#define PTX_T4T_CLASS_BYTE_NON_SECURE_MESSAGING                 (uint8_t)0x00 /* Security Class byte Non-Secure. */
#define PTX_T4T_INSTRUCTION_CODE_SELECT                         (uint8_t)0xA4 /* OpCode Select. */
#define PTX_T4T_INSTRUCTION_CODE_READ_BINARY                    (uint8_t)0xB0 /* OpCode Read Binary. */
#define PTX_T4T_INSTRUCTION_CODE_READ_BINARY_ODO                (uint8_t)0xB1 /* OpCode Read Binary using ODO. */
#define PTX_T4T_INSTRUCTION_CODE_UPDATE_BINARY                  (uint8_t)0xD6 /* OpCode Update Binary. */
#define PTX_T4T_INSTRUCTION_CODE_UPDATE_BINARY_ODO              (uint8_t)0xD7 /* OpCode Update Binary using ODO. */
/*  */

/*
 *  TLV Defines.
 *
 */
#define PTX_T4T_BER_TLV_T_FIELD                                 (uint8_t)0x54 /* BER TLV T-value. */
#define PTX_T4T_DATA_TLV_T_FIELD                                (uint8_t)0x53 /* Data TLV T-value. */
#define PTX_T4T_TLV_BYTES_HEADER_LEN                            (uint8_t)2u   /* Number of TLV header bytes. */
/*  */

/*
 * NDEF APP Select Defines.
 *
 */
#define PTX_T4T_P1_SELECT_NDEF_APP                              (uint8_t)0x04 /* P1 for Select NDEF Application command. */
#define PTX_T4T_P2_SELECT_NDEF_APP                              (uint8_t)0x00 /* P2 for Select NDEF Application command. */
#define LC_FIELD_NDEF_APP_SELECT                                (uint8_t)0x07 /* Lc field for NDEF Application Select command. */
#define LE_FIELD_NDEF_APP_SELECT                                (uint8_t)0x00 /* Le field for NDEF Application Select command. */
/*  */

/*
 * NDEF file select Defines.
 *
 */
#define PTX_T4T_P1_SELECT_FILE                                  (uint8_t)0x00 /* P1 for NDEF File Select command. */
#define PTX_T4T_P2_SELECT_FILE                                  (uint8_t)0x0C /* P2 for NDEF File Select command. */
#define LC_FIELD_NDEF_FILE_SELECT                               (uint8_t)0x02 /* Lc for NDEF File Select command. */
#define LE_FIELD_NDEF_FILE_SELECT                               (uint8_t)0x00 /* Le for NDEF File Select command. */
/*  */

/*
 * UPDATE BINARY Defines.
 *
 */
#define PTX_T4T_UPDATE_BINARY_WORK_BUFFER_SIZE                  (uint8_t)255  /* Internal work-buffer size to write data via UPDATE BINARY. */
/*  */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
/*
 *  Set Command APDU.
 *
 *  t4t               Pointer to component.
 *  classByte         APDU Class Byte.
 *  instructionByte   Instruction Code.
 *  paramByte1        P1.
 *  paramByte2        P2.
 *  bytesWritten  Buffer Offset on input and output.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NativeTag_T4T_SetCommandAPDU (ptx_NativeTag_T4T_t *t4t,
                                                      uint8_t classByte,
                                                      uint8_t instructionByte,
                                                      uint8_t paramByte1,
                                                      uint8_t paramByte2,
                                                      uint8_t *bytesWritten);

/*
 *  Set Command APDU.
 *
 *  t4t               Pointer to component.
 *  instructionByte   Instruction Code.
 *  bytesWritten  Buffer Offset on input and output.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NativeTag_T4T_SetODOCommandAPDU (ptx_NativeTag_T4T_t *t4t,
                                                         uint8_t classByte,
                                                         uint8_t instructionByte,
                                                         uint8_t *bytesWritten);

/*
 *  Parse payload to internal Tx buffer.
 *
 *  t4t               Pointer to component.
 *  idx           Offset within buffer.
 *  instructionByte   Command code.
 *  p1                Command P1.
 *  p2                Command P2.
 *  useODO            Flag whether ODO is in use.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NativeTag_T4T_ParsePayload(ptx_NativeTag_T4T_t *t4t, uint8_t *idx, uint8_t instructionByte, uint8_t p1, uint8_t p2, uint8_t useODO);


/*
 *  Set Maximum Transfer Unit.
 *
 *  useODO                Flag whether ODO is in use.
 *  updateOperation       Flag whether the Operation is a write.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static uint8_t ptx_NativeTag_T4T_SetMTU (uint8_t useODO, uint8_t updateOperation);

/*
 *  Data Exchange with a T4T.
 *
 *  t4t               Pointer to component.
 *  txLen             Length of Data in internal TxBuffer to be sent.
 *  rx            Response Buffer.
 *  rxLen         Length of Response Buffer on input, Length of Response on output.
 *  msTimeout         Timeout in ms.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NativeTag_T4T_Transceive (ptx_NativeTag_T4T_t *t4t, uint32_t txLen, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NativeTag_T4T_Open (ptx_NativeTag_T4T_t *t4t, ptx_NativeTag_T4T_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != t4t) && (NULL != initParams))
    {
        if ((NULL != initParams->rul) && (NULL != initParams->TxBuffer) && (PTX_T4T_MIN_TX_BUFFER_SIZE <= initParams->TxBufferSize))
        {
            /* clear component */
            (void)memset(t4t, 0, sizeof(ptx_NativeTag_T4T_t));

            /* set members */
            t4t->CompId = Comp_NativeTag_T4T;
            t4t->rul = initParams->rul;
            t4t->TxBuffer = initParams->TxBuffer;
            /* Communicate success to upper layers. */
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T4T_Select_File (ptx_NativeTag_T4T_t *t4t,
                                            uint8_t *data,
                                            uint8_t *rx,
                                            uint32_t *rxLen,
                                            uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;


    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T) && (NULL != data) && (NULL != rx) && (NULL != rxLen))
    {
        t4t->Fields.nbr_data_length_bytes = 1u;
        t4t->Fields.nbr_expected_length_bytes = 0u;


        status = ptx_NativeTag_T4T_Select(t4t, PTX_T4T_P1_SELECT_FILE, PTX_T4T_P2_SELECT_FILE,
                                          data, LC_FIELD_NDEF_FILE_SELECT, LE_FIELD_NDEF_FILE_SELECT,
                                          rx, rxLen, msTimeout);
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T4T_Select_NDEF_Application (ptx_NativeTag_T4T_t *t4t,
                                                        uint8_t *rx,
                                                        uint32_t *rxLen,
                                                        uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;


    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T) && (NULL != rx) && (NULL != rxLen))
    {
        t4t->Fields.nbr_data_length_bytes = 1u;
        t4t->Fields.nbr_expected_length_bytes = 1u;

        uint8_t PTX_T4T_DATA_FIELD_NDEF_APP_SELECT[] = {0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01};

        status = ptx_NativeTag_T4T_Select(t4t, PTX_T4T_P1_SELECT_NDEF_APP, PTX_T4T_P2_SELECT_NDEF_APP,
                                          PTX_T4T_DATA_FIELD_NDEF_APP_SELECT, LC_FIELD_NDEF_APP_SELECT,
                                          LE_FIELD_NDEF_APP_SELECT, rx, rxLen, msTimeout);
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T4T_Select (ptx_NativeTag_T4T_t *t4t,
                                       uint8_t paramByte1,
                                       uint8_t paramByte2,
                                       uint8_t *data,
                                       uint8_t nbrDataBytes,
                                       uint8_t expectedResponseLen,
                                       uint8_t *rx,
                                       uint32_t *rxLen,
                                       uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t p1 = paramByte1;
    uint8_t p2 = paramByte2;
    uint8_t bytes_set = 0;
    uint8_t data_field[PTX_T4T_MIN_BUFFER_LEN];

    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T) && (NULL != data) && (NULL != rx) && (NULL != rxLen))
    {

        /* fill the fields */
        t4t->Fields.data_length_field[0] = nbrDataBytes;
        (void)memcpy(&data_field[0], &data[0], (uint32_t)nbrDataBytes);
        t4t->Fields.nbr_data_bytes = nbrDataBytes;

        if (0 != t4t->Fields.nbr_expected_length_bytes)
        {
            t4t->Fields.expected_length_field[0] = expectedResponseLen;
        }

        status = ptx_NativeTag_T4T_SetCommandAPDU(t4t, PTX_T4T_CLASS_BYTE_NON_SECURE_MESSAGING, PTX_T4T_INSTRUCTION_CODE_SELECT, p1, p2, &bytes_set);

        /* push Le byte back, then fill data */
        if (0 != t4t->Fields.nbr_expected_length_bytes)
        {
            if (bytes_set > 0)
            {
                t4t->TxBuffer[bytes_set + nbrDataBytes - 1] = t4t->TxBuffer[bytes_set - 1];
                (void)memcpy(&t4t->TxBuffer[bytes_set - 1], &data_field[0], nbrDataBytes);
                bytes_set = (uint8_t)(bytes_set + nbrDataBytes);
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
        else
        {
            (void)memcpy(&t4t->TxBuffer[bytes_set], &data_field[0], nbrDataBytes);
            bytes_set = (uint8_t)(bytes_set + nbrDataBytes);
        }

        /* everything ok so far? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T4T_Transceive(t4t, bytes_set, rx, rxLen, msTimeout);

            if (PTX_STATUS_SUCCESS == status)
            {
                (void)memset(&t4t->Fields, 0, sizeof(ptx_NativeTag_T4T_CommandAPDUFields_t));
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T4T_ReadBinary (ptx_NativeTag_T4T_t *t4t,
                                           uint16_t offset,
                                           uint8_t nbrExpectedResponseBytes,
                                           uint8_t *rx,
                                           uint32_t *rxLen,
                                           uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t bytes_written;
    uint8_t p1;
    uint8_t p2;

    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T) && (NULL != rx) && (NULL != rxLen))
    {

        /* offset in p1 and p2 */
        p1 = (uint8_t)((uint16_t)offset >> 8u);
        p2 = (uint8_t)((uint16_t)offset & ((1u << 8u) -1u));

        /* fill the fields */
        t4t->Fields.expected_length_field[0] = nbrExpectedResponseBytes;
        t4t->Fields.nbr_data_bytes = 0;
        t4t->Fields.nbr_data_length_bytes = 0;
        t4t->Fields.nbr_expected_length_bytes = 1;


        status = ptx_NativeTag_T4T_SetCommandAPDU(t4t, PTX_T4T_CLASS_BYTE_NON_SECURE_MESSAGING, PTX_T4T_INSTRUCTION_CODE_READ_BINARY, p1, p2, &bytes_written);

        /* everything ok so far? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T4T_Transceive(t4t, bytes_written, rx, rxLen, msTimeout);

            if (PTX_STATUS_SUCCESS == status)
            {
                (void)memset(&t4t->Fields, 0, sizeof(ptx_NativeTag_T4T_CommandAPDUFields_t));
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T4T_ReadBinaryODO (ptx_NativeTag_T4T_t *t4t,
                                              uint32_t offset,
                                              uint8_t nbrExpectedResponseBytes,
                                              uint8_t *rx,
                                              uint32_t *rxLen,
                                              uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t bytes_written;

    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T) && (NULL != rx) && (NULL != rxLen))
    {
        /* offset contained in data field via BER-TLV */
        t4t->Fields.ber_tlv_data_field[0] = PTX_T4T_BER_TLV_T_FIELD;
        t4t->Fields.ber_tlv_data_field[1] = 0x03;

        t4t->Fields.ber_tlv_data_field[2] =  (uint8_t)(offset >> 16u); /* byte index 1 (index 0 unused) */
        t4t->Fields.ber_tlv_data_field[3] = (uint8_t)((uint16_t)offset >> 8u); /* byte index 2 */
        t4t->Fields.ber_tlv_data_field[4] = ((uint16_t)offset & ((1u << 8u) -1u)); /* byte index 3 */

        t4t->Fields.data_length_field[0] = sizeof(t4t->Fields.ber_tlv_data_field); /* first value */
        t4t->Fields.nbr_data_length_bytes = 1;

        /* fill the fields */
        t4t->Fields.expected_length_field[0] = (nbrExpectedResponseBytes + PTX_T4T_TLV_BYTES_HEADER_LEN);
        t4t->Fields.nbr_expected_length_bytes = 1;

        status = ptx_NativeTag_T4T_SetODOCommandAPDU(t4t, PTX_T4T_CLASS_BYTE_NON_SECURE_MESSAGING, PTX_T4T_INSTRUCTION_CODE_READ_BINARY_ODO, &bytes_written);

        /* everything ok so far? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T4T_Transceive(t4t, bytes_written, rx, rxLen, msTimeout);

            if (PTX_STATUS_SUCCESS == status)
            {
                (void)memset(&t4t->Fields, 0, sizeof(ptx_NativeTag_T4T_CommandAPDUFields_t));
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T4T_UpdateBinary (ptx_NativeTag_T4T_t *t4t,
                                             uint16_t offset,
                                             uint8_t *data,
                                             uint8_t nbrDataBytes,
                                             uint8_t *rx,
                                             uint32_t *rxLen,
                                             uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t bytes_set;
    uint8_t p1;
    uint8_t p2;
    uint8_t data_field[PTX_T4T_UPDATE_BINARY_WORK_BUFFER_SIZE];

    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T) && (NULL != data) && (NULL != rx) && (NULL != rxLen))
    {
        /* offset in p1 and p2 */
        p1 = (uint8_t)((uint16_t)offset >> 8u);
        p2 = (uint8_t)((uint16_t)offset & ((1u << 8u) -1u));

        /* fill the fields */
        t4t->Fields.data_length_field[0] = nbrDataBytes;
        t4t->Fields.nbr_data_length_bytes = 1;
        (void)memcpy(&data_field[0], data, (uint32_t)nbrDataBytes);
        t4t->Fields.nbr_data_bytes = nbrDataBytes;

        status = ptx_NativeTag_T4T_SetCommandAPDU(t4t, PTX_T4T_CLASS_BYTE_NON_SECURE_MESSAGING, PTX_T4T_INSTRUCTION_CODE_UPDATE_BINARY, p1, p2, &bytes_set);

        (void)memcpy(&t4t->TxBuffer[bytes_set], &data_field[0], t4t->Fields.nbr_data_bytes);
        bytes_set = (uint8_t)(bytes_set + t4t->Fields.nbr_data_bytes);

        /* everything ok so far? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            status = ptx_NativeTag_T4T_Transceive(t4t, bytes_set, rx, rxLen, msTimeout);

            if (PTX_STATUS_SUCCESS == status)
            {
                (void)memset(&t4t->Fields, 0, sizeof(ptx_NativeTag_T4T_CommandAPDUFields_t));
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T4T_UpdateBinaryODO (ptx_NativeTag_T4T_t *t4t,
                                                uint32_t offset,
                                                uint8_t *data,
                                                uint8_t nbrDataBytes,
                                                uint8_t *rx,
                                                uint32_t *rxLen,
                                                uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t data_field_ODO[PTX_T4T_MIN_BUFFER_LEN + PTX_T4T_TLV_BYTES_HEADER_LEN];
    uint8_t bytes_set = 0;

    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T) && (NULL != data) && (NULL != rx) && (NULL != rxLen))
    {
        /* offset contained in data field via BER-TLV */
        t4t->Fields.ber_tlv_data_field[0] = PTX_T4T_BER_TLV_T_FIELD;
        t4t->Fields.ber_tlv_data_field[1] = 0x03;

        t4t->Fields.ber_tlv_data_field[2] =  (uint8_t)(offset >> 16u); //byte index 1 (index 0 unused)
        t4t->Fields.ber_tlv_data_field[3] = (uint8_t)((uint16_t)offset >> 8u); //byte index 2
        t4t->Fields.ber_tlv_data_field[4] = ((uint16_t)offset & ((1u << 8u) -1u)); //byte index 3

        /* fill the fields */
        data_field_ODO[0] = PTX_T4T_DATA_TLV_T_FIELD;
        data_field_ODO[1] = nbrDataBytes;
        (void)memcpy(&data_field_ODO[2], data, (uint32_t)nbrDataBytes);

        /* change LC field */
        t4t->Fields.nbr_data_length_bytes = 1;
        if (0xFF >= (nbrDataBytes + 7))
        {
            t4t->Fields.data_length_field[0] = (uint8_t)(nbrDataBytes + 7); /* data + ODO + 2 bytes data structure */

            t4t->Fields.nbr_data_bytes = (uint16_t)(nbrDataBytes + 2); /* data + DDO bytes, equal to length of data_field_ODO */

            status = ptx_NativeTag_T4T_SetODOCommandAPDU(t4t, PTX_T4T_CLASS_BYTE_NON_SECURE_MESSAGING, PTX_T4T_INSTRUCTION_CODE_UPDATE_BINARY_ODO, &bytes_set);
        }
        else
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }

        /* everything ok so far? - send data */
        if (PTX_STATUS_SUCCESS == status)
        {
            (void)memcpy(&t4t->TxBuffer[bytes_set], &data_field_ODO[0], t4t->Fields.nbr_data_bytes);
            bytes_set = (uint8_t)(bytes_set + t4t->Fields.nbr_data_bytes);

            status = ptx_NativeTag_T4T_Transceive(t4t, bytes_set, rx, rxLen, msTimeout);

            if (PTX_STATUS_SUCCESS == status)
            {
                (void)memset(&t4t->Fields, 0, sizeof(ptx_NativeTag_T4T_CommandAPDUFields_t));
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NativeTag_T4T_Close (ptx_NativeTag_T4T_t *t4t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T))
    {
        memset(t4t, 0, sizeof(ptx_NativeTag_T4T_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK(s)
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NativeTag_T4T_SetCommandAPDU (ptx_NativeTag_T4T_t *t4t,
                                                      uint8_t classByte,
                                                      uint8_t instructionByte,
                                                      uint8_t paramByte1,
                                                      uint8_t paramByte2,
                                                      uint8_t *bytesWritten)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t index = 0;

    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T) && (NULL != bytesWritten))
    {
        t4t->TxBuffer[index] = classByte;
        index++;
        t4t->TxBuffer[index] = instructionByte;
        index++;
        t4t->TxBuffer[index] = paramByte1;
        index++;
        t4t->TxBuffer[index] = paramByte2;
        index++;

        status = ptx_NativeTag_T4T_ParsePayload(t4t, &index, instructionByte, paramByte1, paramByte2, 0);

        *bytesWritten = index;
    }

    return status;
}

static ptx_Status_t ptx_NativeTag_T4T_SetODOCommandAPDU (ptx_NativeTag_T4T_t *t4t,
                                                         uint8_t classByte,
                                                         uint8_t instructionByte,
                                                         uint8_t *bytesWritten)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t index = 0;
    uint8_t paramByteODO = 0x00;

    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T) && (NULL != bytesWritten))
    {
        t4t->TxBuffer[index] = classByte;
        index++;
        t4t->TxBuffer[index] = instructionByte;
        index++;
        t4t->TxBuffer[index] = 0x00; /* p1 */
        index++;
        t4t->TxBuffer[index] = 0x00; /* p2 */
        index++;

        status = ptx_NativeTag_T4T_ParsePayload(t4t, &index, instructionByte, paramByteODO, paramByteODO, 1);

        *bytesWritten = index;
    }

    return status;
}

static ptx_Status_t ptx_NativeTag_T4T_ParsePayload(ptx_NativeTag_T4T_t *t4t, uint8_t *idx, uint8_t instructionByte, uint8_t p1, uint8_t p2, uint8_t useODO)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t index = 0;
    uint8_t max_transfer = 253u;
    uint8_t ber_tlv_len = 5u;   /* (T, L, V, V, V) */
    uint8_t command_header_len = 4u; /* (class,instruction,p1,p2) */

    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T) && (NULL != idx))
    {
        index = *idx;
        if ((PTX_T4T_INSTRUCTION_CODE_READ_BINARY == instructionByte) ||
            (PTX_T4T_INSTRUCTION_CODE_READ_BINARY_ODO == instructionByte))
        {
            max_transfer = ptx_NativeTag_T4T_SetMTU(useODO, 0);
        }
        if (PTX_T4T_INSTRUCTION_CODE_UPDATE_BINARY_ODO == instructionByte)
        {
            max_transfer = ptx_NativeTag_T4T_SetMTU(useODO, 1);
        }

        switch (t4t->Fields.nbr_data_length_bytes)
        {
            case 0:
                status = PTX_STATUS_SUCCESS;
                break;
            case 1:
                if (0 != t4t->Fields.data_length_field[0])
                {
                    if (max_transfer >= (command_header_len + t4t->Fields.data_length_field[0] + 1)) /* (class,instruction,p1,p2) + data + Le */
                    {
                        t4t->TxBuffer[index] = t4t->Fields.data_length_field[0];
                        index++;
                        status = PTX_STATUS_SUCCESS;
                    }
                    else
                    {
                        status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                    }
                }
                else
                {
                    status = PTX_STATUS_SUCCESS;
                }
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER; /* invalid length */
                break;
        }

        if (0x00 != useODO)
        {
            memcpy(&t4t->TxBuffer[index], &t4t->Fields.ber_tlv_data_field[0], ber_tlv_len);
            index = (uint8_t)(index + ber_tlv_len);
        }

        switch (t4t->Fields.nbr_expected_length_bytes)
        {
            case 0:
                /* the NDEF App Select requires a set LE Byte */
                if ((0x00 == useODO) && (PTX_T4T_P1_SELECT_NDEF_APP == p1) && (PTX_T4T_P2_SELECT_NDEF_APP == p2))
                {
                    t4t->TxBuffer[index] = t4t->Fields.expected_length_field[0];
                    index++;
                    t4t->Fields.nbr_expected_length_bytes = 1;
                }
                break;
            case 1:
                t4t->TxBuffer[index] = t4t->Fields.expected_length_field[0];
                index++;
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER; /* invalid length */
                break;
        }
        if (PTX_STATUS_SUCCESS == status)
        {
            *idx = index;
        }
    }

    return status;
}

static uint8_t ptx_NativeTag_T4T_SetMTU (uint8_t useODO, uint8_t updateOperation)
{
    uint8_t transfer_len;
    if (0x00 == useODO)
    {
        transfer_len = 253u; /* maximal nbr of bytes we can send or receive in one go */
    }
    else
    {
        if (updateOperation)
        {
            transfer_len = 253u - 5u - 2u; /* max bytes - BERTLV - (0x53 Ld(1 byte)) */
        }
        else
        {
            transfer_len = 253u - 5u; /* max bytes - BERTLV */
        }
    }

    return transfer_len;
}

static ptx_Status_t ptx_NativeTag_T4T_Transceive (ptx_NativeTag_T4T_t *t4t, uint32_t txLen, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(t4t, Comp_NativeTag_T4T) && (NULL != rx) && (NULL != rxLen))
    {
        /* send command */
        status = ptx_RUL_Exchange(t4t->rul, &t4t->TxBuffer[0], txLen, rx, rxLen, msTimeout);

        if (PTX_STATUS_SUCCESS != status)
        {
            *rxLen = 0;
        }
    }

    return status;
}

