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
    Module      : HCE T4T
    File        : ptx_HCET4T.c

    Description : Implementation of Host Card Emulation (HCE) for T4T. This enables the emulation of the
                  PTX2xxR/W device as a T4T Card.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <string.h>
#include <stdint.h>
#include "ptx_HCET4T.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_HCET4T_CC_FILE_SIZE                (uint16_t)15    /* Fixed Capability Container-File size (supporting 1 NDEF-FIle) */
#define PTX_HCET4T_CC_FILE_SIZE_H              (uint8_t)((PTX_HCET4T_CC_FILE_SIZE >> 8) & 0xFF)     /* Upper byte of CC File size. */
#define PTX_HCET4T_CC_FILE_SIZE_L              (uint8_t)((PTX_HCET4T_CC_FILE_SIZE >> 0) & 0xFF)     /* Lower byte of CC File size. */

#define PTX_HCET4T_CC_MAPPING_VERSION          (uint16_t)0x20  /* NDEF-Mapping Version */

#define PTX_HCET4T_CC_MLE                      (uint16_t)510   /* Max. data size that can be read from the T4T using a single ReadBinary command. */
#define PTX_HCET4T_CC_MLE_H                    (uint8_t)((PTX_HCET4T_CC_MLE >> 8) & 0xFF)       /* Upper byte of CC MLe. */
#define PTX_HCET4T_CC_MLE_L                    (uint8_t)((PTX_HCET4T_CC_MLE >> 0) & 0xFF)       /* Lower byte of CC MLe. */

#define PTX_HCET4T_CC_MLC                      (uint16_t)512   /* Max. data size that can be sent to the T4T using a single UpdateBinary command. */
#define PTX_HCET4T_CC_MLC_H                    (uint8_t)((PTX_HCET4T_CC_MLC >> 8) & 0xFF)       /* Upper byte of CC MLc. */
#define PTX_HCET4T_CC_MLC_L                    (uint8_t)((PTX_HCET4T_CC_MLC >> 0) & 0xFF)       /* Lower byte of CC MLc. */

#define PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE      (uint16_t)512   /* 256 Byte(s) NDEF File Size consisting of 2 Byte(s) length + 254 Byte(s) message */
#define PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE_H    (uint8_t)((PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE >> 8) & 0xFF)    /* Upper byte of NDEF File size. */
#define PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE_L    (uint8_t)((PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE >> 0) & 0xFF)    /* Lower byte of NDEF File size. */

#define PTX_HCET4T_NDEF_ACCESS_FULL_READ       (uint8_t)0x00   /* Read-Access from NDEF-File */
#define PTX_HCET4T_NDEF_ACCESS_FULL_WRITE      (uint8_t)0x00   /* Write-Access to NDEF-File */

#define PTX_HCET4T_CLA_ZERO_CHECK_MASK         (uint8_t)0x3F   /**< In practice, upper two bytes might not be zero on the CLA byte */

/*
 * Default File defines.
 *
 */
static uint8_t ndefCCFile[PTX_HCET4T_CC_FILE_SIZE];              /* Default CC File. */
static uint8_t ndefNDEFFile[PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE];  /* Default NDEF File. */


/*
 * NFC Forum Application-ID
 */
static const uint8_t PTX_HCET4T_APPLICATION_IDENTIFIER[] =
{
    0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01,
};

/*
 * NFC Forum Capability Container File-ID
 */
static const uint8_t PTX_HCET4T_CC_FILE_IDENTIFIER[] =
{
    0xE1, 0x03,
};

/*
 * NFC Forum NDEF File-ID
 */
static const uint8_t PTX_HCET4T_NDEF_FILE_IDENTIFIER[] =
{
    0xE1, 0x04,
};

/*
 * Default CC File content.
 */
static const uint8_t defCCData[] =
{
    /* Capability Container-File Part */
    PTX_HCET4T_CC_FILE_SIZE_H, PTX_HCET4T_CC_FILE_SIZE_L,
    PTX_HCET4T_CC_MAPPING_VERSION,
    PTX_HCET4T_CC_MLE_H, PTX_HCET4T_CC_MLE_L,
    PTX_HCET4T_CC_MLC_H, PTX_HCET4T_CC_MLC_L,

    /* NDEF-File Control Part (TLV-Format) */
    0x04,                                               /* T-Field: NDEF-File-Control TLV Tag */
    0x06,                                               /* L-Field: NDEF-File-Control TLV Length */
    0xE1, 0x04,                                         /* V-Field: NDEF File Identifier (see definition above) */
    PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE_H, PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE_L,
    PTX_HCET4T_NDEF_ACCESS_FULL_READ,
    PTX_HCET4T_NDEF_ACCESS_FULL_WRITE,
};
/*  */


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
/*
 *  Compare IDs.
 *
 *  id1       First ID.
 *  id1Len    First ID length.
 *  id2       Second ID.
 *  id2Len    Second ID length.
 *
 * returns Return value of memory compare operation.
 */
static int ptx_HCET4T_CheckID(const uint8_t *id1, uint8_t id1Len, const uint8_t *id2, uint8_t id2Len);

/*
 *  Set Status Word in Tx buffer.
 *
 *  txData            Tx buffer.
 *      txDataOffset      Tx buffer offset.
 *      statusWord        Status word to set.
 */
static void ptx_HCET4T_SetStatusWord(uint8_t *txData, uint32_t txDataOffset, uint16_t statusWord);

static ptx_Status_t ptx_HCET4T_NFCForumLoopback_SendSelectResponse(uint8_t *rx, uint32_t rxLen, uint8_t *tx, uint32_t* txLen);

static ptx_Status_t ptx_HCET4T_NFCForumLoopback_SendLoopbackAPDU(uint8_t *rx, uint32_t rxLen, uint8_t *tx, uint32_t* txLen);

static ptx_Status_t ptx_HCET4T_NFCForumLoopback_SendUnknownCommand(uint8_t *tx, uint32_t* txLen);

static uint32_t ptx_HCET4T_ReadLe(uint8_t *rxData, uint32_t rxLength, uint32_t *offset);

static uint32_t ptx_HCET4T_ReadLc(uint8_t *rxData, uint32_t rxLength, uint32_t *offset);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_HCET4T_Init(ptx_HCET4T_t *t4t, ptx_HCET4T_InitParams_t *t4tInitParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != t4t)
    {
        if (NULL != t4tInitParams)
        {
            if ((NULL != t4tInitParams->CCFile) && (0 != t4tInitParams->CCFileSize))
            {
                t4t->CCFile = t4tInitParams->CCFile;
                t4t->CCFileSize = t4tInitParams->CCFileSize;
            }
            else
            {
                t4t->CCFile = &ndefCCFile[0];
                t4t->CCFileSize = PTX_HCET4T_CC_FILE_SIZE;
                memcpy(t4t->CCFile, defCCData, PTX_HCET4T_CC_FILE_SIZE);
            }

            if ((NULL != t4tInitParams->NDEFFile) && (0 != t4tInitParams->NDEFFileSize))
            {
                t4t->NDEFFile = t4tInitParams->NDEFFile;
                t4t->NDEFFileSize = t4tInitParams->NDEFFileSize;
            }
            else
            {
                t4t->NDEFFile = &ndefNDEFFile[0];
                t4t->NDEFFileSize = PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE;
            }

            t4t->State = HCET4T_State_Reset;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            t4t->CCFile = &ndefCCFile[0];
            t4t->CCFileSize = PTX_HCET4T_CC_FILE_SIZE;
            memcpy(t4t->CCFile, defCCData, PTX_HCET4T_CC_FILE_SIZE);

            t4t->NDEFFile = &ndefNDEFFile[0];
            t4t->NDEFFileSize = PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE;
            status = PTX_STATUS_SUCCESS;
        }

        t4t->CompId = Comp_HCE;
    }

    return status;
}

ptx_Status_t ptx_HCET4T_DeInit(ptx_HCET4T_t *t4t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(t4t, Comp_HCE))
    {
        memset(t4t, 0, sizeof(ptx_HCET4T_t));
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HCET4T_Reset(ptx_HCET4T_t *t4t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(t4t, Comp_HCE))
    {
#if 0   /* Temporarily disable re-setting the CCFile and NDEFFile contents as a restarted discovery from the Poller side would delete the previously stored NDEF Data */
        memcpy(t4t->CCFile, defCCData, PTX_HCET4T_CC_FILE_SIZE);
        memset(t4t->NDEFFile, 0, PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE);
#endif
        t4t->State = HCET4T_State_Reset;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HCET4T_UpdateMemory(ptx_HCET4T_t *t4t, uint16_t startIndex, uint8_t *data, uint32_t dataLength) // Not needed
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(t4t, Comp_HCE) && (NULL != data))
    {
        switch (t4t->State)
        {
            case HCET4T_State_NDEF_File_Selected:
                if (t4t->NDEFFileSize >= (startIndex + dataLength))
                {
                    memcpy(&t4t->NDEFFile[startIndex], &data[0], dataLength);
                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
                }
                break;

            case HCET4T_State_CC_File_Selected:
                if (t4t->CCFileSize >= (startIndex + dataLength))
                {
                    memcpy(&t4t->CCFile[startIndex], &data[0], dataLength);
                    status = PTX_STATUS_SUCCESS;
                }
                else
                {
                    status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
                }
                break;

            case HCET4T_State_Reset:
            /* Fall through. */
            case HCET4T_State_NDEF_Application_Selected:
            /* Fall through. */
            default:
                status = PTX_STATUS_E_GEN_INVALID_STATE;
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_HCET4T_NFCForumLoopback_CommandProcess(ptx_HCET4T_t *t4t, uint8_t *rx, uint32_t rxLen, uint8_t *tx, uint32_t *txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    const uint8_t SELECT_APDU[PTX_HCET4T_LISTEN_SELECT_APDU_LEN] = {0x00, 0xA4, 0x04, 0x00};
    const uint8_t LOOPBACK_CAPDU[PTX_HCET4T_LISTEN_LOOPBACK_CAPDU_LEN] = {0x80, 0xEE, 0x00, 0x00};

    if (PTX_COMP_CHECK(t4t, Comp_HCE) && (NULL != rx) && (0 != rxLen) && (NULL != tx) && (NULL != txLen))
    {
        // Check if received command is a SELECT APDU
        if ((rxLen >= PTX_HCET4T_LISTEN_SELECT_APDU_LEN) && (0 == memcmp(rx, SELECT_APDU, sizeof(uint8_t) * PTX_HCET4T_LISTEN_SELECT_APDU_LEN)))
        {
            // respond with a Select Response
            status = ptx_HCET4T_NFCForumLoopback_SendSelectResponse(rx, rxLen, tx, txLen);
        }
        // Check if received command is a Loopback command
        else if ((rxLen >= PTX_HCET4T_LISTEN_LOOPBACK_CAPDU_LEN) && (0 == memcmp(rx, LOOPBACK_CAPDU, sizeof(uint8_t) * PTX_HCET4T_LISTEN_LOOPBACK_CAPDU_LEN)))
        {
            // respond with loopback APDU
            status  = ptx_HCET4T_NFCForumLoopback_SendLoopbackAPDU(rx, rxLen, tx, txLen);
        }
        // Respond with Status Word command not allowed in case of unknown command
        else
        {
            // respond with unknown command
            status = ptx_HCET4T_NFCForumLoopback_SendUnknownCommand(tx, txLen);
        }
    }

    return status;
}

static ptx_Status_t ptx_HCET4T_NFCForumLoopback_SendSelectResponse(uint8_t *rx, uint32_t rxLen, uint8_t *tx, uint32_t* txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != rx) && (0 != rxLen) && (NULL != tx) && (NULL != txLen))
    {
        uint8_t SEL_RES_DTA[PTX_HCET4T_LISTEN_SELECT_RES_DTA_LEN] = {0x01, 0x00};

        // Check if received select is the proprietary select dta APDU
        uint8_t SELECT_DTA_APDU[PTX_HCET4T_LISTEN_SELECT_DTA_APDU_LEN] = {0x0E, 0x31, 0x4E, 0x46, 0x43, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};
        if ((rxLen >= PTX_HCET4T_ISO7816_SW_LEN + PTX_HCET4T_LISTEN_SELECT_DTA_APDU_LEN) && (0 == memcmp(&rx[PTX_HCET4T_ISO7816_SW_LEN], SELECT_DTA_APDU, sizeof(uint8_t) * PTX_HCET4T_LISTEN_SELECT_DTA_APDU_LEN)))
        {
            // Send back the expected SEL_RES for the Select DTA command
            memcpy(tx, SEL_RES_DTA, PTX_HCET4T_LISTEN_SELECT_RES_DTA_LEN);
            ptx_HCET4T_SetStatusWord(tx, PTX_HCET4T_LISTEN_SELECT_RES_DTA_LEN, PTX_HCET4T_ISO7816_SW_SUCCESS);
            *txLen = PTX_HCET4T_LISTEN_SELECT_RES_DTA_LEN + PTX_HCET4T_ISO7816_SW_LEN;
            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            // Send back the standard SEL_RES response (SW SUCCESS) for other select commands
            ptx_HCET4T_SetStatusWord(tx, 0, PTX_HCET4T_ISO7816_SW_SUCCESS);
            *txLen = PTX_HCET4T_ISO7816_SW_LEN;
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

static ptx_Status_t ptx_HCET4T_NFCForumLoopback_SendLoopbackAPDU(uint8_t *rx, uint32_t rxLen, uint8_t *tx, uint32_t* txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != rx) && (0 != rxLen) && (NULL != tx) && (NULL != txLen))
    {
        // Create Response APDU from received APDU
        *txLen = rxLen + PTX_HCET4T_ISO7816_SW_LEN;
        memcpy(tx, rx, sizeof(uint8_t) * rxLen);
        ptx_HCET4T_SetStatusWord(tx, (uint16_t)rxLen, PTX_HCET4T_ISO7816_SW_SUCCESS);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

static ptx_Status_t ptx_HCET4T_NFCForumLoopback_SendUnknownCommand(uint8_t *tx, uint32_t* txLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != tx) && (NULL != txLen))
    {
        ptx_HCET4T_SetStatusWord(tx, 0, PTX_HCET4T_ISO7816_SW_COMMAND_NOT_ALLOWED);
        *txLen = PTX_HCET4T_ISO7816_SW_LEN;
        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_HCET4T_CommandProcess(ptx_HCET4T_t *t4t, uint8_t *rx, uint32_t rxLen, uint8_t *tx, uint32_t *txLen)
{
    ptx_Status_t status = PTX_STATUS_SUCCESS;

    /* APDU specific variables */
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint32_t lc;
    uint32_t le;

    uint16_t file_offset;
    uint32_t nr_bytes_to_process;
    int result;
    const uint8_t *active_file;
    uint16_t active_file_size;
    uint16_t tx_status_word;
    uint32_t tx_offset = 0;
    uint32_t tx_length = PTX_HCET4T_ISO7816_SW_LEN;

    if (PTX_COMP_CHECK(t4t, Comp_HCE) &&
        (NULL != rx) &&
        (0 != rxLen) &&
        (NULL != tx) &&
        (NULL != txLen))
    {
        /* at least 2 Bytes are needed to set a Status-Word */
        if (*txLen >= (uint32_t)PTX_HCET4T_ISO7816_SW_LEN)
        {
            if (rxLen >= PTX_HCET4T_ISO7816_HEADER_LEN)
            {
                /* get minimum APDU header variables*/
                cla = rx[PTX_HCET4T_ISO7816_OFFSET_CLA];
                ins = rx[PTX_HCET4T_ISO7816_OFFSET_INS];
                p1 = rx[PTX_HCET4T_ISO7816_OFFSET_P1];
                p2 = rx[PTX_HCET4T_ISO7816_OFFSET_P2];

                /* any supported Command from the NFC Forum T4T specification must use CLA-Byte == 0 */
                if ((cla & PTX_HCET4T_CLA_ZERO_CHECK_MASK) == 0)
                {
                    /* check supported commands */
                    switch (ins)
                    {
                        case PTX_HCET4T_ISO7816_INS_SELECT:
                            /* Select must consist of at least 6 Byte(s) */
                            if (rxLen >= 6U)
                            {
                                uint32_t offset = PTX_HCET4T_ISO7816_OFFSET_LC;
                                lc = ptx_HCET4T_ReadLc(rx, rxLen, &offset);

                                /* Select by Name ? */
                                if (((uint8_t)0x04 == p1) && ((uint8_t)0x00 == p2))
                                {
                                    /* NDEF-Application to be selected -> LC must be exactly 7 Bytes */
                                    if (lc == (uint8_t)0x07)
                                    {
                                        /* Note: NDEF-Application is always possible from any state - no need for any checks here */

                                        /* check Application ID */
#if 0   /* Temporarily disable checking the last byte of AID - allow iOS app "NFC Tools" compatibility which sends a different AID */
                                        result = ptx_HCET4T_CheckID(&rx[PTX_HCET4T_ISO7816_OFFSET_CDATA], lc, &PTX_HCET4T_APPLICATION_IDENTIFIER[0], (uint8_t)sizeof(PTX_HCET4T_APPLICATION_IDENTIFIER));
#else
                                        result = ptx_HCET4T_CheckID(&rx[offset], (uint8_t)(lc - 1u), &PTX_HCET4T_APPLICATION_IDENTIFIER[0], (uint8_t)sizeof(PTX_HCET4T_APPLICATION_IDENTIFIER) -1u);
#endif

                                        /* Application-ID match ? */
                                        if (0 == result)
                                        {
                                            t4t->State = HCET4T_State_NDEF_Application_Selected;
                                            tx_status_word = PTX_HCET4T_ISO7816_SW_SUCCESS;

                                        }
                                        else
                                        {
                                            tx_status_word = PTX_HCET4T_ISO7816_SW_FILE_NOT_FOUND;
                                        }

                                    }
                                    else
                                    {
                                        tx_status_word = PTX_HCET4T_ISO7816_SW_FILE_NOT_FOUND;
                                    }
                                }
                                else if (((uint8_t)0x00 == p1) && ((uint8_t)0x0C == p2))
                                {
                                    /* NDEF- or CC-File to be selected -> LC must be exactly 2 Bytes in both cases */
                                    if (lc == (uint8_t)0x02)
                                    {
                                        if (HCET4T_State_Reset != t4t->State)
                                        {
                                            /* check which File should be selected */
                                            /* check Application ID */
                                            result = ptx_HCET4T_CheckID(&rx[offset], (uint8_t)lc, &PTX_HCET4T_CC_FILE_IDENTIFIER[0], (uint8_t)sizeof(PTX_HCET4T_CC_FILE_IDENTIFIER));

                                            if (0 == result)
                                            {
                                                t4t->State = HCET4T_State_CC_File_Selected;
                                                tx_status_word = PTX_HCET4T_ISO7816_SW_SUCCESS;
                                            }
                                            else
                                            {
                                                result = ptx_HCET4T_CheckID(&rx[offset], (uint8_t)lc, &PTX_HCET4T_NDEF_FILE_IDENTIFIER[0], (uint8_t)sizeof(PTX_HCET4T_NDEF_FILE_IDENTIFIER));

                                                if (0 == result)
                                                {
                                                    t4t->State = HCET4T_State_NDEF_File_Selected;
                                                    tx_status_word = PTX_HCET4T_ISO7816_SW_SUCCESS;
                                                }
                                                else
                                                {
                                                    tx_status_word = PTX_HCET4T_ISO7816_SW_FILE_NOT_FOUND;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            tx_status_word = PTX_HCET4T_ISO7816_SW_FILE_NOT_FOUND;
                                        }
                                    }
                                    else
                                    {
                                        tx_status_word = PTX_HCET4T_ISO7816_SW_CONDITIONS_NOT_SATISFIED;
                                    }
                                }
                                else
                                {
                                    tx_status_word = PTX_HCET4T_ISO7816_SW_FILE_NOT_FOUND;
                                }
                            }
                            else
                            {
                                tx_status_word = PTX_HCET4T_ISO7816_SW_WRONG_LENGTH;
                            }
                            break;

                        case PTX_HCET4T_ISO7816_INS_READ_BINARY:
                            /* ReadBinary must consist of exactly 5 Byte(s) */
                            if (rxLen >= PTX_HCET4T_ISO7816_HEADER_LEN)
                            {
                                uint32_t offset = PTX_HCET4T_ISO7816_OFFSET_LE;
                                le = ptx_HCET4T_ReadLe(rx, rxLen, &offset);

                                /* ReadBinary-operations are only allowed if the NDEF- or CC-file is selected */
                                if ((HCET4T_State_NDEF_File_Selected == t4t->State) || (HCET4T_State_CC_File_Selected == t4t->State))
                                {
                                    file_offset = (uint16_t)((p1 << 8) | p2);

                                    active_file = (HCET4T_State_NDEF_File_Selected == t4t->State) ? t4t->NDEFFile : t4t->CCFile;
                                    active_file_size = (HCET4T_State_NDEF_File_Selected == t4t->State) ? PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE : PTX_HCET4T_CC_FILE_SIZE;

                                    /* check out-of-bounds / correct length */
                                    if ((file_offset + le) <= active_file_size)
                                    {
                                        if (le <= PTX_HCET4T_CC_MLE)
                                        {
                                            /* Note: Byte LE == 0x00 means that the Card should send everything it has (limited by MAX_LE) */
                                            nr_bytes_to_process = le == 0x00 ? PTX_HCET4T_CC_MLE : le;

                                            /* Tx-Buffer large enough */
                                            if (nr_bytes_to_process > active_file_size)
                                            {
                                                // Short coding size error -> set SW1=0x6C SW2=max size
                                                if ((offset == PTX_HCET4T_ISO7816_OFFSET_LE + 1))
                                                {
                                                    tx_status_word = (0x6C << 8) | active_file_size;
                                                }
                                                else
                                                {
                                                    tx_status_word = PTX_HCET4T_ISO7816_SW_WRONG_LENGTH;
                                                }
                                            }
                                            else
                                            {
                                                /* Tx-Buffer large enough */
                                                if (nr_bytes_to_process <= *txLen)
                                                {
                                                    /* "Read" actual data */
                                                    (void)memcpy(&tx[0], &active_file[file_offset], nr_bytes_to_process);
                                                    tx_offset = nr_bytes_to_process;
                                                    tx_status_word = PTX_HCET4T_ISO7816_SW_SUCCESS;
                                                    tx_length = (tx_length + nr_bytes_to_process);

                                                }
                                                else
                                                {
                                                    status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            tx_status_word = PTX_HCET4T_ISO7816_SW_WRONG_LENGTH;
                                        }

                                    }
                                    else
                                    {
                                        tx_status_word = PTX_HCET4T_ISO7816_SW_INCORRECT_P1P2;
                                    }

                                }
                                else
                                {
                                    tx_status_word = PTX_HCET4T_ISO7816_SW_CONDITIONS_NOT_SATISFIED;
                                }
                            }
                            else
                            {
                                tx_status_word = PTX_HCET4T_ISO7816_SW_WRONG_LENGTH;
                            }
                            break;

                        case PTX_HCET4T_ISO7816_INS_UPDATE_BINARY:
                            /* ReadBinary must consist of at least 6 Byte(s) */
                            if (rxLen >= 6U)
                            {
                                lc = rx[PTX_HCET4T_ISO7816_OFFSET_LC];

                                /* UpdateBinary-operations are only allowed if the NDEF-file is selected */
                                if ( (HCET4T_State_CC_File_Selected == t4t->State)
                                     || (HCET4T_State_NDEF_File_Selected == t4t->State))
                                {
                                    file_offset = (uint16_t)((p1 << 8) | p2);

                                    /* check out-of-bounds / correct length */
                                    if ((uint16_t)(file_offset + lc) <= PTX_HCET4T_NDEF_MESSAGE_FILE_SIZE)
                                    {
                                        if ((uint16_t)lc <= PTX_HCET4T_CC_MLC)
                                        {
                                            nr_bytes_to_process = (uint16_t)lc;

                                            /* "Write" actual data */
                                            (void)memcpy(&t4t->NDEFFile[file_offset], &rx[PTX_HCET4T_ISO7816_OFFSET_CDATA], nr_bytes_to_process);
                                            tx_status_word = PTX_HCET4T_ISO7816_SW_SUCCESS;

                                        }
                                        else
                                        {
                                            tx_status_word = PTX_HCET4T_ISO7816_SW_WRONG_LENGTH;
                                        }

                                    }
                                    else
                                    {
                                        tx_status_word = PTX_HCET4T_ISO7816_SW_INCORRECT_P1P2;
                                    }

                                }
                                else
                                {
                                    tx_status_word = PTX_HCET4T_ISO7816_SW_CONDITIONS_NOT_SATISFIED;
                                }

                            }
                            else
                            {
                                tx_status_word = PTX_HCET4T_ISO7816_SW_WRONG_LENGTH;
                            }
                            break;

                        default:
                            tx_status_word = PTX_HCET4T_ISO7816_SW_FUNC_NOT_SUPPORTED;
                            break;
                    }

                }
                else
                {
                    tx_status_word = PTX_HCET4T_ISO7816_SW_COMMAND_NOT_ALLOWED;
                }

            }
            else
            {
                tx_status_word = PTX_HCET4T_ISO7816_SW_WRONG_LENGTH;
            }

            /* set Tx-Data (Status-Word) */
            if (PTX_STATUS_SUCCESS == status)
            {
                ptx_HCET4T_SetStatusWord(tx, tx_offset, tx_status_word);
                *txLen = tx_length;

            }
            else
            {
                *txLen = 0;
            }

        }
        else
        {
            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
        }

    }
    else
    {
        status = PTX_STATUS_E_GEN_INVALID_STATE;
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
static void ptx_HCET4T_SetStatusWord(uint8_t *txData, uint32_t txDataOffset, uint16_t statusWord)
{
    /* Note: Pointers / Offsets are assumed to be valid / checked by the calling function */

    txData[txDataOffset + 0] = (uint8_t)((statusWord >> 8) & (uint8_t)0xFF);
    txData[txDataOffset + 1] = (uint8_t)((statusWord >> 0) & (uint8_t)0xFF);
}

static int ptx_HCET4T_CheckID(const uint8_t *id1, uint8_t id1Len, const uint8_t *id2, uint8_t id2Len)
{
    int ret_value = -1;

    /* Note: Pointers / Offsets are assumed to be valid / checked by the calling function */

    if (id1Len == id2Len)
    {
        ret_value = memcmp(id1, id2, id1Len);
    }

    return ret_value;
}

static uint32_t ptx_HCET4T_ReadLe(uint8_t *rxData, uint32_t rxLength, uint32_t *offset)
{
    /* Note: Pointers / Offsets are assumed to be valid / checked by the calling function */
    uint32_t ret = 0;

    if ((*offset + 2 < rxLength) && (0 == rxData[*offset]))         // Extended Coding with absent LC field
    {
        ret = (uint32_t)((rxData[*offset + 1] << 8) | (rxData[*offset + 2]));
        *offset += 3;
    }
    else if (*offset + 1 < rxLength)                                // Extended Coding with present LC field
    {
        ret = (uint32_t)((rxData[*offset] << 8) | (rxData[*offset + 1]));
        *offset += 2;
    }
    else if (*offset < rxLength)                                    // Short Coding
    {
        ret = rxData[*offset];
        *offset += 1;
    }

    return ret;
}


static uint32_t ptx_HCET4T_ReadLc(uint8_t *rxData, uint32_t rxLength, uint32_t *offset)
{
    /* Note: Pointers / Offsets are assumed to be valid / checked by the calling function */
    uint32_t ret = 0;
    if (*offset < rxLength)
    {
        uint8_t coding_field = rxData[*offset];
        *offset += 1;

        if ((0x00 == coding_field) && (*offset + 1 < rxLength))
        {
            ret = (uint32_t)((rxData[*offset] << 8) | (rxData[*offset + 1]));
            *offset += 2;
        }
        else
        {
            ret = coding_field;
        }
    }

    return ret;
}