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
    Module      : NFC Data Exchange Format (NDEF) T4T
    File        : ptx_NDEF_T4T.c

    Description : NFC Data Exchange Format (NDEF) implementation to control NDEF Messages on T4T Devices.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_RUL.h"
#include "ptx_NDEF_T4T.h"
#include "ptx_NativeTag_T4T.h"
#include <string.h>
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * Status Flags.
 *
 */
#define PTX_T4T_SUCCESS_FLAG_SW1                                (uint8_t)0x90           /* Status Flag OK 1. */
#define PTX_T4T_SUCCESS_FLAG_SW2                                (uint8_t)0x00           /* Status Flag OK 2. */
#define PTX_T4T_STATUS_WORD_LENGTH                              (uint8_t)0x02           /* Status Flag length. */
#define PTX_T4T_SW1_ERROR_WRONG_P1P2                            (uint8_t)0x6A           /* Status flag Wrong P1P2. */
#define PTX_T4T_SW2_ERROR_FILE_OR_APP_NOT_FOUND                 (uint8_t)0x82           /* Status flag Application or File not found. */
#define PTX_T4T_SELECT_FAIL_FLAG_SW1                            (uint8_t)0x6A           /* Status Flag Select failed 1. */
#define PTX_T4T_SELECT_FAIL_FLAG_SW2                            (uint8_t)0x82           /* Status Flag Select failed 2. */
#define PTX_T4T_READ_FAIL_FLAG_SW1_SIMPLE                       (uint8_t)0x67           /* Status Flag Read failed 1. */
#define PTX_T4T_READ_FAIL_FLAG_SW2_SIMPLE                       (uint8_t)0x00           /* Status Flag Read failed 2. */
#define PTX_T4T_READ_FAIL_FLAG_SW1_EXT                          (uint8_t)0x6C           /* Status Flag Extended Read failed 1. */
/*  */

/*
 * CC and TLV Defines.
 *
 */
#define PTX_T4T_NDEF_LENGTH                                     (uint8_t)0x08           /* NDEF Information length. */
#define PTX_T4T_ENDEF_LENGTH                                    (uint8_t)0x0A           /* Extended NDEF Information length. */
#define PTX_T4T_MAPPING_VERSION_2_0                             (uint8_t)0x20           /* Mapping version 2.0. */
#define PTX_T4T_MAPPING_VERSION_MAJOR_2                         (uint8_t)2u             /* Mapping major version 2. */
#define PTX_T4T_MAPPING_VERSION_MAJOR_3                         (uint8_t)3u             /* Mapping major version 3. */
#define PTX_T4T_MAPPING_VERSION_3_0                             (uint8_t)0x30           /* Mapping version 3.0. */
#define PTX_T4T_CC_LEN_MIN                                      (uint8_t)15u            /* Minimum length of CC file. */
#define PTX_T4T_ACCESS_RW                                       (uint8_t)0x00           /* Read and write access granted. */
/*  */

/*
 * Defines for CC file select
 *
 */
#define DATA_FIELD_CC_SELECT                                    (uint16_t)0xE103        /* CC File Identifier. */
/*  */



/*
 * Capacity Defines.
 *
 */
#define PTX_T4T_MAX_BYTES_SEND                                  (uint8_t)0xFD           /* Internal maximum of bytes sendable. */
#define PTX_T4T_MAX_BYTES_RECEIVE                               (uint8_t)0xFD           /* Internal maximum of bytes receivable. */
#define PTX_T4T_MAX_NLEN                                        (uint16_t)0x7FFD        /* Maximum NLEN. */
#define PTX_T4T_MAX_ENLEN                                       (uint32_t)0xFFFFFFFA    /* Maximum Extended NLEN. */
#define PTX_T4T_MAX_READ_LEN                                    (uint8_t)251u           /* Internal maximum read length. */
#define PTX_T4T_CAPDU_SIZE_ODO                                  (uint8_t)19u            /* CAPDU size when using ODO. */
#define PTX_T4T_ODO_THRESHOLD                                   (uint16_t)0x7FFF        /* ODO usage threshold. */
/*  */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / HELPERS
 * ####################################################################################################################
 */
/*
 *  Check the Response of the Select command
 *
 *  ndefT4t               Pointer to component.
 *  rx                Response Buffer.
 *  rxLen             Length of the Response.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T4T_CheckSelectResponse(ptx_NDEF_T4T_t *ndefT4t, uint8_t *rx, uint32_t *rxLen);


/*
 *  Read from File in App on T4T.
 *  ndefT4t                   Pointer to component.
 *  offset                    Offset at which to start reading.
 *  nbrExpectedResponseBytes  Expected Length of Response.
 *  rx                    Response Buffer.
 *  rxLen                 Length of Response Buffer on input, Length of Response on output.
 *  msTimeout                 Timeout in ms.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T4T_ReadBinary (ptx_NDEF_T4T_t *ndefT4t, uint32_t offset, uint8_t nbrExpectedResponseBytes, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout);

/*
 *  Write to File in App on T4T.
 *
 *  ndefT4t               Pointer to component.
 *  offset                Offset at which to start writing.
 *  dataField             Data to be written.
 *  nbrDataBytes          Length of Data.
 *  rx                Response Buffer.
 *  rxLen             Length of Response Buffer on input, Length of Response on output.
 *  msTimeout             Timeout in ms.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T4T_UpdateBinary (ptx_NDEF_T4T_t *ndefT4t, uint32_t offset, uint8_t *dataField, uint8_t nbrDataBytes, uint8_t *rx, uint32_t *rxLen, uint32_t msTimeout);

/*
 *  Get CC Information.
 *
 *  ndefT4t               Pointer to component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T4T_GetCCInfo (ptx_NDEF_T4T_t *ndefT4t);

/*
 *  Handle CC Information.
 *
 *  ndefT4t               Pointer to component.
 *  ccInfo                Buffer with CC File data.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T4T_HandleCCInfo (ptx_NDEF_T4T_t *ndefT4t, uint8_t *ccInfo);

/*
 *  Get Length Information of NDEF File.
 *
 *  ndefT4t               Pointer to component.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T4T_GetNLEN (ptx_NDEF_T4T_t *ndefT4t);

/*
 *  Read NDEF File content.
 *
 *      ndefT4t               Pointer to component.
 *  msgBuffer             Message Data Buffer.
 *     msgLen                Message Data Length.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T4T_ReadFileContent (ptx_NDEF_T4T_t *ndefT4t, uint8_t *msgBuffer, uint32_t *msgLen);

/*
 *  Update NDEF File Length.
 *
 *  ndefT4t               Pointer to component.
 *  newLen                New NDEF File Length.
 *
 * returns Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
static ptx_Status_t ptx_NDEF_T4T_UpdateLength (ptx_NDEF_T4T_t *ndefT4t, uint32_t newLen);
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NDEF_T4T_Open (ptx_NDEF_T4T_t *ndefT4t, ptx_NDEF_T4T_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ndefT4t) && (NULL != initParams))
    {
        if ((NULL != initParams->RxBuffer) &&
            (0 != initParams->RxBufferSize))
        {
            if(ptx_RUL_CardProtocolType_T4T == initParams->Rul->CardRegistry.ActiveProtocol)
            {
                /* clear component */
                (void)memset(ndefT4t, 0, sizeof(ptx_NDEF_T4T_t));

                ndefT4t->RxBuffer = initParams->RxBuffer;
                ndefT4t->RxBufferSize = initParams->RxBufferSize;
                ndefT4t->LifeCycle = TagLC_NoNDEFTag;

                /* initialize lower layer component */
                ptx_NativeTag_T4T_InitParams_t t4t_init_params;
                (void)memset(&t4t_init_params, 0, sizeof(ptx_NativeTag_T4T_InitParams_t));
                t4t_init_params.Rul = initParams->Rul;
                t4t_init_params.TxBuffer = initParams->TxBuffer;
                t4t_init_params.TxBufferSize = initParams->TxBufferSize;

                status = ptx_NativeTag_T4T_Open(&ndefT4t->T4t, &t4t_init_params);

                /* set Component-ID at the end to prevent further calls in case of an error */
                if (PTX_STATUS_SUCCESS == status)
                {
                    ndefT4t->CompId = Comp_T4TOP;
                }
            }
            else
            {
                status = PTX_STATUS_E_GEN_NOT_PERMITTED;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
        }
       
    }

    return status;
}

ptx_Status_t ptx_NDEF_T4T_FormatTag (ptx_NDEF_T4T_t *ndefT4t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            status = PTX_STATUS_E_NDEF_UNSUPPORTED;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T4T_CheckMessage (ptx_NDEF_T4T_t *ndefT4t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            /* select NDEF application */
            uint32_t rx_len = ndefT4t->RxBufferSize;

            status = ptx_NativeTag_T4T_Select_NDEF_Application(&ndefT4t->T4t,
                                                            &ndefT4t->RxBuffer[0], &rx_len, PTX_T4T_DEFAULT_TIMEOUT_MS);


            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T4T_CheckSelectResponse(ndefT4t, &ndefT4t->RxBuffer[0], &rx_len);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                /* get CC info */
                status = ptx_NDEF_T4T_GetCCInfo(ndefT4t);

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T4T_GetNLEN(ndefT4t);
                }
                else
                {
                    ndefT4t->LifeCycle = TagLC_NoNDEFTag;
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T4T_ReadMessage (ptx_NDEF_T4T_t *ndefT4t, uint8_t *msgBuffer, uint32_t *msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t msg_buffer_size;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP) && (NULL != msgBuffer) && (NULL != msgLen) && (0 != *msgLen))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            msg_buffer_size = (uint32_t)(*msgLen);

            if (0 != msg_buffer_size)
            {
                /* check tag LC */
                switch (ndefT4t->LifeCycle)
                {
                    case TagLC_Initialised:
                    case TagLC_ReadWrite:
                    case TagLC_ReadOnly:
                        /* OK */
                        status = PTX_STATUS_SUCCESS;
                        break;

                    default:
                        status = PTX_STATUS_E_NDEF_INVALID_TAG_STATE;
                        break;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    status = ptx_NDEF_T4T_ReadFileContent(ndefT4t, msgBuffer, &msg_buffer_size);

                    if ((PTX_STATUS_SUCCESS != status) || (ndefT4t->NLEN.DigitNLEN != msg_buffer_size))
                    {
                        msg_buffer_size = 0;
                    }

                    *msgLen = msg_buffer_size;
                }
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T4T_WriteMessage (ptx_NDEF_T4T_t *ndefT4t, uint8_t *msgBuffer, uint32_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint32_t msg_len;
    uint32_t bytes_written = 0;
    uint8_t bytes_to_write;
    uint32_t ndef_file_size;
    uint8_t nbr_nlen_bytes;
    uint8_t *msg_buffer;
    uint8_t EMPTY_NDEF_MESSAGE[3] = {0xD0, 0x00, 0x00};
    uint8_t empty_record_len = 0x03;
    uint8_t c_apdu_size;
    uint32_t rx_len;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP) && (NULL != msgBuffer))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            msg_len = msgLen;
            msg_buffer = msgBuffer;
            ndef_file_size = (uint32_t)ndefT4t->CCParams.NDEFFileSize;
            nbr_nlen_bytes = ndefT4t->NLEN.NbrNLENBytes;
            c_apdu_size = 5u; /* class, inst, p1, p2, Le */

            /* check tag LC */
            switch (ndefT4t->LifeCycle)
            {
                case TagLC_Initialised:
                case TagLC_ReadWrite:
                    /* OK */
                    status = PTX_STATUS_SUCCESS;
                    break;

                default:
                    status = PTX_STATUS_E_NDEF_INVALID_TAG_STATE;
                    break;
            }

            if (0 != msg_len)
            {
                if (PTX_STATUS_SUCCESS == status)
                {
                    if (ndef_file_size >= msg_len)
                    {
                        if ((uint16_t)(ndefT4t->CCParams.MLcDigit - c_apdu_size) >= msg_len) /* MLcDigit is by standard 0xFFFF at max */
                        {
                            /* reset NLEN before write */
                            status = ptx_NDEF_T4T_UpdateLength(ndefT4t, 0);

                            if (PTX_STATUS_SUCCESS == status)
                            {
                                /* write message in one go */
                                status = ptx_NDEF_T4T_UpdateBinary(ndefT4t, (uint32_t)nbr_nlen_bytes, &msg_buffer[0], (uint8_t)msg_len,
                                                                &ndefT4t->RxBuffer[0], &rx_len, PTX_T4T_DEFAULT_TIMEOUT_MS);

                                if (PTX_STATUS_SUCCESS == status)
                                {
                                    bytes_written = (uint32_t)(bytes_written + msg_len);
                                }
                            }
                        }
                        else
                        {
                            /* reset NLEN before write */
                            status = ptx_NDEF_T4T_UpdateLength(ndefT4t, 0);

                            if (PTX_STATUS_SUCCESS == status)
                            {
                                while (msg_len > bytes_written)
                                {
                                    if (bytes_written + nbr_nlen_bytes >= PTX_T4T_ODO_THRESHOLD)
                                    {
                                        c_apdu_size = PTX_T4T_CAPDU_SIZE_ODO; /* Some available bytes are lost due to ODO C-APDU */
                                    }

                                    bytes_to_write = ((uint32_t)(msg_len - bytes_written) > (uint32_t)(ndefT4t->CCParams.MLcDigit - c_apdu_size)) ? ((uint8_t)(ndefT4t->CCParams.MLcDigit - c_apdu_size)) : ((uint8_t)(msg_len - bytes_written));

                                    status = ptx_NDEF_T4T_UpdateBinary(ndefT4t, (uint32_t)(nbr_nlen_bytes + bytes_written), &msg_buffer[bytes_written], bytes_to_write,
                                                                    &ndefT4t->RxBuffer[0], &rx_len, PTX_T4T_DEFAULT_TIMEOUT_MS);
                                    if (PTX_STATUS_SUCCESS == status)
                                    {
                                        bytes_written = (uint32_t)(bytes_written + bytes_to_write);
                                    }
                                }
                            }
                        }

                        /* message written correctly? */
                        if ((PTX_STATUS_SUCCESS == status) && (msgLen == bytes_written))
                        {
                            /* OK, update (E)NLEN */
                            status = ptx_NDEF_T4T_UpdateLength(ndefT4t, (uint32_t)msg_len);
                        }
                    }
                    else
                    {
                        status = PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES;
                    }

                }
            }
            else
            {
                /* reset NLEN before write */
                status = ptx_NDEF_T4T_UpdateLength(ndefT4t, 0);

                if (PTX_STATUS_SUCCESS == status)
                {
                    /* set empty message */
                    msg_buffer = &EMPTY_NDEF_MESSAGE[0];
                    status = ptx_NDEF_T4T_UpdateBinary(ndefT4t, (uint32_t)nbr_nlen_bytes, &msg_buffer[0], empty_record_len,
                                                    &ndefT4t->RxBuffer[0], &rx_len, PTX_T4T_DEFAULT_TIMEOUT_MS);
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        /* OK, update (E)NLEN */
                        status = ptx_NDEF_T4T_UpdateLength(ndefT4t, empty_record_len);
                        if (PTX_STATUS_SUCCESS == status)
                        {
                            ndefT4t->LifeCycle = TagLC_Initialised;
                        }
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    (void)msgBuffer;
    (void)msgLen;

    return status;
}

ptx_Status_t ptx_NDEF_T4T_LockTag (ptx_NDEF_T4T_t *ndefT4t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            status = PTX_STATUS_SUCCESS; /* there is no procedure to lock the tag via the NDEF API */
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_T4T_Close (ptx_NDEF_T4T_t *ndefT4t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            status = ptx_NativeTag_T4T_Close(&ndefT4t->T4t);
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK(s)
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NDEF_T4T_CheckSelectResponse(ptx_NDEF_T4T_t *ndefT4t, uint8_t *rx, uint32_t *rxLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP) && (NULL != rx) && (NULL != rxLen))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            if (0 != *rxLen)
            {
                if (PTX_T4T_STATUS_WORD_LENGTH <= *rxLen)
                {
                    if ((PTX_T4T_SUCCESS_FLAG_SW1 == rx[*rxLen - 2]) && (PTX_T4T_SUCCESS_FLAG_SW2 == rx[*rxLen - 1]))
                    {
                        /* successful select */
                        *rxLen = *rxLen - PTX_T4T_STATUS_WORD_LENGTH;
                        status = PTX_STATUS_SUCCESS;
                    }
                    else
                    {
                        status = PTX_STATUS_E_NSC_RFERROR;
                        if (((PTX_T4T_SW1_ERROR_WRONG_P1P2 == rx[*rxLen - 2]) && (PTX_T4T_SW2_ERROR_FILE_OR_APP_NOT_FOUND == rx[*rxLen - 1])) ||
                            ((PTX_T4T_READ_FAIL_FLAG_SW1_SIMPLE == rx[*rxLen - 2]) && (PTX_T4T_READ_FAIL_FLAG_SW2_SIMPLE == rx[*rxLen - 1])))
                        {
                            status = PTX_STATUS_E_NDEF_NOT_FOUND;
                        }
                    }
                }
                else
                {
                    status = PTX_STATUS_E_NSC_RFERROR;
                }
            }
            else
            {
                status = PTX_STATUS_E_NSC_RFERROR;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T4T_ReadBinary (ptx_NDEF_T4T_t *ndefT4t,
                                             uint32_t offset,
                                             uint8_t nbrExpectedResponseBytes,
                                             uint8_t *rx,
                                             uint32_t *rxLen,
                                             uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t cut_idx = 0;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP) && (NULL != rx) && (NULL != rxLen))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            *rxLen = (uint32_t)ndefT4t->RxBufferSize;

            switch (ndefT4t->CCParams.MappingMajor)
            {
                case 2:
                    status = ptx_NativeTag_T4T_ReadBinary(&ndefT4t->T4t, (uint16_t)offset, nbrExpectedResponseBytes, rx, rxLen, msTimeout);
                    break;
                case 3:
                    status = ptx_NativeTag_T4T_ReadBinaryODO(&ndefT4t->T4t, offset, nbrExpectedResponseBytes, rx, rxLen, msTimeout);
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        cut_idx = (0x81 != rx[1]) ? (2u) : (3u);

                        *rxLen = *rxLen - cut_idx; /* remove BER-TLV T and L field */
                        memmove(&rx[0], &rx[cut_idx], *rxLen);
                    }
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                if (0 != *rxLen)
                {
                    if (PTX_T4T_STATUS_WORD_LENGTH <= *rxLen)
                    {
                        if ((PTX_T4T_SUCCESS_FLAG_SW1 == rx[*rxLen - 2]) && (PTX_T4T_SUCCESS_FLAG_SW2 == rx[*rxLen - 1]))
                        {
                            /* successful read */
                            *rxLen = *rxLen - PTX_T4T_STATUS_WORD_LENGTH;
                        }
                        else
                        {
                            status = PTX_STATUS_E_NSC_RFERROR;
                            if ((PTX_T4T_READ_FAIL_FLAG_SW1_SIMPLE == rx[*rxLen - 2]) && (PTX_T4T_READ_FAIL_FLAG_SW2_SIMPLE == rx[*rxLen - 1]))
                            {
                                status = PTX_STATUS_E_GEN_INVALID_PARAMETER; /* wrong length, no further information */
                            }

                            if (PTX_T4T_READ_FAIL_FLAG_SW1_EXT == rx[*rxLen - 2])
                            {
                                status = PTX_STATUS_E_GEN_INVALID_PARAMETER; /* wrong length, ndefT4t->RxBuffer[rxLen-1] has nbr of available data bytes */
                            }
                        }
                    }
                    else
                    {
                        status = PTX_STATUS_E_NSC_RFERROR;
                    }
                }
            }
            else
            {
                status = PTX_STATUS_E_NSC_RFERROR;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T4T_UpdateBinary (ptx_NDEF_T4T_t *ndefT4t,
                                               uint32_t offset,
                                               uint8_t *dataField,
                                               uint8_t nbrDataBytes,
                                               uint8_t *rx,
                                               uint32_t *rxLen,
                                               uint32_t msTimeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP) && (NULL != dataField) && (NULL != rx) && (NULL != rxLen))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            *rxLen = (uint32_t)ndefT4t->RxBufferSize;

            switch (ndefT4t->CCParams.MappingMajor)
            {
                case 2:
                    status = ptx_NativeTag_T4T_UpdateBinary(&ndefT4t->T4t, (uint16_t)offset, dataField, nbrDataBytes, rx, rxLen, msTimeout);
                    break;
                case 3:
                    if (offset >= 0x7FFF)
                    {
                        status = ptx_NativeTag_T4T_UpdateBinaryODO(&ndefT4t->T4t, offset, dataField, nbrDataBytes, rx, rxLen, msTimeout);
                    }
                    else
                    {
                        status = ptx_NativeTag_T4T_UpdateBinary(&ndefT4t->T4t, (uint16_t)offset, dataField, nbrDataBytes, rx, rxLen, msTimeout);
                    }

                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                if ((0 != *rxLen) && (PTX_T4T_STATUS_WORD_LENGTH <= *rxLen))
                {
                    if ((PTX_T4T_SUCCESS_FLAG_SW1 == rx[*rxLen - 2]) && (PTX_T4T_SUCCESS_FLAG_SW2 == rx[*rxLen - 1]))
                    {
                        /* successful update */
                        *rxLen = *rxLen - PTX_T4T_STATUS_WORD_LENGTH;
                    }
                    else
                    {
                        status = PTX_STATUS_E_NSC_RFERROR;
                    }
                }
                else
                {
                    status = PTX_STATUS_E_NSC_RFERROR;
                }
            }
            else
            {
                status = PTX_STATUS_E_NSC_RFERROR;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T4T_GetCCInfo (ptx_NDEF_T4T_t *ndefT4t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t cc_le_field = 0x02;
    uint8_t cc_index = 0;
    uint8_t data_field[2];

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            data_field[0] = (uint8_t)((DATA_FIELD_CC_SELECT >> 8u) & 0xFF);
            data_field[1] = (uint8_t)(DATA_FIELD_CC_SELECT & 0xFF);

            /* select CC file (function is to be called AFTER NDEF application select) */
            uint32_t rx_len = ndefT4t->RxBufferSize;

            status = ptx_NativeTag_T4T_Select_File(&ndefT4t->T4t, data_field,
                                                &ndefT4t->RxBuffer[0], &rx_len,
                                                PTX_T4T_DEFAULT_TIMEOUT_MS);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T4T_CheckSelectResponse(ndefT4t, &ndefT4t->RxBuffer[0], &rx_len);
            }


            if (PTX_STATUS_SUCCESS == status)
            {
                /* read CCLen bytes of CC file */
                ndefT4t->CCParams.MappingVersion = PTX_T4T_MAPPING_VERSION_2_0; /* preset for CC collection */
                ndefT4t->CCParams.MappingMajor = 2u;
                ndefT4t->CCParams.MappingMinor = 0u;
                status = ptx_NDEF_T4T_ReadBinary(ndefT4t, (uint32_t)cc_index, cc_le_field,
                                                &ndefT4t->RxBuffer[0], &rx_len, PTX_T4T_DEFAULT_TIMEOUT_MS);
                cc_index = (uint8_t)(cc_index + cc_le_field);

                if (PTX_STATUS_SUCCESS == status)
                {
                    (void)memcpy(&ndefT4t->CCParams.CCLen[0], &ndefT4t->RxBuffer[0], (uint32_t)2);

                    /* CCLen <= 0xFF, read in one go (proprietary ignored) */
                    cc_le_field = ndefT4t->CCParams.CCLen[1];

                    /* read rest of the CC file */
                    status = ptx_NDEF_T4T_ReadBinary(ndefT4t, (uint32_t)cc_index, (uint8_t)(cc_le_field - cc_index),
                                                    &ndefT4t->RxBuffer[0], &rx_len, PTX_T4T_DEFAULT_TIMEOUT_MS);

                    if ((PTX_STATUS_SUCCESS == status) && (PTX_T4T_CC_LEN_MIN - 2u <= rx_len))
                    {
                        status = ptx_NDEF_T4T_HandleCCInfo(ndefT4t, &ndefT4t->RxBuffer[0]);
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T4T_HandleCCInfo (ptx_NDEF_T4T_t *ndefT4t, uint8_t *ccInfo)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP) && (NULL != ccInfo))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            uint8_t data_index = 0;
            ndefT4t->CCParams.MappingVersion = ccInfo[data_index];
            ndefT4t->CCParams.MappingMajor = (uint8_t)((ndefT4t->CCParams.MappingVersion >> 4u) & 0x0F);
            ndefT4t->CCParams.MappingMinor = (uint8_t)(ndefT4t->CCParams.MappingVersion & 0x0F);
            data_index++;
            ndefT4t->CCParams.MLeDigit = (uint16_t)(((uint16_t)ccInfo[data_index] << 8) | ccInfo[data_index + 1]);
            data_index = (uint8_t)(data_index + 2);
            if (PTX_T4T_MAX_BYTES_RECEIVE < ndefT4t->CCParams.MLeDigit)
            {
                ndefT4t->CCParams.MLeDigit = PTX_T4T_MAX_BYTES_RECEIVE;
            }
            ndefT4t->CCParams.MLcDigit = (uint16_t)(((uint16_t)ndefT4t->RxBuffer[data_index] << 8) | ccInfo[data_index + 1]);
            data_index = (uint8_t)(data_index + 2);
            if (PTX_T4T_MAX_BYTES_SEND < ndefT4t->CCParams.MLcDigit)
            {
                ndefT4t->CCParams.MLcDigit = PTX_T4T_MAX_BYTES_SEND;
            }
            status = PTX_STATUS_SUCCESS;
            if ((ndefT4t->CCParams.MLeDigit == 0) || (ndefT4t->CCParams.MLcDigit == 0))
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                switch (ndefT4t->CCParams.MappingMajor)
                {
                    case 2:
                        /* NDEF control TLV */
                        (void)memcpy(&ndefT4t->CCParams.NDEFTLV[0], &ccInfo[data_index], (uint8_t)PTX_T4T_NDEF_LENGTH);
                        data_index = (uint8_t)(data_index + PTX_T4T_NDEF_LENGTH);
                        (void)memcpy(&ndefT4t->CCParams.NDEFFileIdentifier[0], &ndefT4t->CCParams.NDEFTLV[2], (uint32_t)2);
                        ndefT4t->CCParams.NDEFFileSize = (uint32_t)(((uint32_t)ndefT4t->CCParams.NDEFTLV[4] << 8u) | ((uint32_t)ndefT4t->CCParams.NDEFTLV[5]));
                        ndefT4t->CCParams.NDEFAccessRead = ndefT4t->CCParams.NDEFTLV[6];
                        ndefT4t->CCParams.NDEFAccessWrite = ndefT4t->CCParams.NDEFTLV[7];
                        break;
                    case 3:
                        /* ENDEF control TLV */
                        (void)memcpy(&ndefT4t->CCParams.NDEFTLV[0], &ccInfo[data_index], (uint8_t)PTX_T4T_ENDEF_LENGTH);
                        data_index = (uint8_t)(data_index + PTX_T4T_ENDEF_LENGTH);
                        (void)memcpy(&ndefT4t->CCParams.NDEFFileIdentifier[0], &ndefT4t->CCParams.NDEFTLV[2], (uint32_t)2);
                        ndefT4t->CCParams.NDEFFileSize = (uint32_t)((ndefT4t->CCParams.NDEFTLV[4] << 24u) | (ndefT4t->CCParams.NDEFTLV[5] << 16u) | (ndefT4t->CCParams.NDEFTLV[6] << 8u) | (ndefT4t->CCParams.NDEFTLV[7]));
                        ndefT4t->CCParams.NDEFAccessRead = ndefT4t->CCParams.NDEFTLV[8];
                        ndefT4t->CCParams.NDEFAccessWrite = ndefT4t->CCParams.NDEFTLV[9];
                        break;
                    default:
                        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                        break;
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T4T_GetNLEN (ptx_NDEF_T4T_t *ndefT4t)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t nbr_nlen_bytes;
    uint32_t nlen_offset = 0x00;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            ndefT4t->LifeCycle = TagLC_Initialised;
            /* select NDEF file */
            uint32_t rx_len = ndefT4t->RxBufferSize;

            status = ptx_NativeTag_T4T_Select_File(&ndefT4t->T4t, &ndefT4t->CCParams.NDEFFileIdentifier[0],
                                                &ndefT4t->RxBuffer[0], &rx_len,
                                                PTX_T4T_DEFAULT_TIMEOUT_MS);

            if (PTX_STATUS_SUCCESS == status)
            {
                status = ptx_NDEF_T4T_CheckSelectResponse(ndefT4t, &ndefT4t->RxBuffer[0], &rx_len);
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                nbr_nlen_bytes = (PTX_T4T_MAPPING_VERSION_MAJOR_2 != ndefT4t->CCParams.MappingMajor) ? (4u) : (2u);
                ndefT4t->NLEN.NbrNLENBytes = nbr_nlen_bytes;
                status = ptx_NDEF_T4T_ReadBinary(ndefT4t, nlen_offset, nbr_nlen_bytes,
                                                &ndefT4t->RxBuffer[0], &rx_len, PTX_T4T_DEFAULT_TIMEOUT_MS);

                if ((PTX_STATUS_SUCCESS == status) && (nbr_nlen_bytes == rx_len))
                {
                    (void)memcpy(&ndefT4t->NLEN.NLEN[0], &ndefT4t->RxBuffer[0], (uint32_t)nbr_nlen_bytes);
                    ndefT4t->NLEN.NbrNLENBytes = nbr_nlen_bytes;
                }

                if (PTX_STATUS_SUCCESS == status)
                {
                    switch (ndefT4t->CCParams.MappingMajor)
                    {
                        case PTX_T4T_MAPPING_VERSION_MAJOR_2:
                            ndefT4t->NLEN.DigitNLEN = (uint16_t)(ndefT4t->NLEN.NLEN[1] | (ndefT4t->NLEN.NLEN[0] << 8u));
                            break;
                        case PTX_T4T_MAPPING_VERSION_MAJOR_3:
                            ndefT4t->NLEN.DigitNLEN = (uint32_t)(ndefT4t->NLEN.NLEN[3] | (ndefT4t->NLEN.NLEN[2] << 8u) | (ndefT4t->NLEN.NLEN[1]) << 16u | (ndefT4t->NLEN.NLEN[0]) << 24u);
                            break;
                        default:
                            ndefT4t->LifeCycle = TagLC_NoNDEFTag;
                            status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                            break;
                    }
                    if ((0 != ndefT4t->NLEN.DigitNLEN) && (PTX_STATUS_SUCCESS == status))
                    {
                        switch (ndefT4t->CCParams.NDEFAccessWrite)
                        {
                            case PTX_T4T_ACCESS_RW:
                                ndefT4t->LifeCycle = TagLC_ReadWrite;
                                break;

                            default:
                                ndefT4t->LifeCycle = TagLC_ReadOnly;
                                break;
                        }
                    }
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T4T_ReadFileContent (ptx_NDEF_T4T_t *ndefT4t, uint8_t *msgBuffer, uint32_t *msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint32_t rx_len = 0;
    uint16_t bytes_to_read = 0;
    uint32_t bytes_read = 0;
    uint32_t msg_len = 0;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP) && (NULL != msgBuffer) && (NULL != msgLen) && (0 != *msgLen))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            msg_len = *msgLen;

            /* read NDEF file after (E)NLEN bytes, SELECT was done in check */
            if (((uint32_t)(ndefT4t->CCParams.MLeDigit - PTX_T4T_STATUS_WORD_LENGTH) > ndefT4t->NLEN.DigitNLEN)
                && (PTX_T4T_MAX_READ_LEN > ndefT4t->NLEN.DigitNLEN))
            {
                /* read message in one go */

                status = ptx_NDEF_T4T_ReadBinary(ndefT4t, (uint32_t)ndefT4t->NLEN.NbrNLENBytes,
                                                (uint8_t)ndefT4t->NLEN.DigitNLEN,
                                                &ndefT4t->RxBuffer[0], &rx_len, PTX_T4T_DEFAULT_TIMEOUT_MS);
                bytes_read = (uint32_t)(bytes_read + rx_len);

                if (PTX_STATUS_SUCCESS == status)
                {
                    if (msg_len >= bytes_read)
                    {
                        (void) memcpy(&msgBuffer[0], &ndefT4t->RxBuffer[0], bytes_read);
                    }
                    else
                    {
                        status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
                    }
                }
            }
            else
            {
                status = PTX_STATUS_SUCCESS;

                while ((ndefT4t->NLEN.DigitNLEN > bytes_read) && (PTX_STATUS_SUCCESS == status))
                {
                    bytes_to_read = ((uint32_t)(ndefT4t->NLEN.DigitNLEN - bytes_read) > (uint32_t)(ndefT4t->CCParams.MLeDigit)) ? ((uint16_t)(ndefT4t->CCParams.MLeDigit)) : ((uint16_t)(ndefT4t->NLEN.DigitNLEN - bytes_read));

                    status = ptx_NDEF_T4T_ReadBinary(ndefT4t, (uint32_t)(ndefT4t->NLEN.NbrNLENBytes + bytes_read),
                                                    (uint8_t)bytes_to_read,
                                                    &ndefT4t->RxBuffer[0], &rx_len, PTX_T4T_DEFAULT_TIMEOUT_MS);
                    if (PTX_STATUS_SUCCESS == status)
                    {
                        if (msg_len >= (bytes_read + bytes_to_read))
                        {
                            (void) memcpy(&msgBuffer[bytes_read], &ndefT4t->RxBuffer[0], bytes_to_read);
                            bytes_read = (uint32_t)(bytes_read + rx_len);
                        }
                        else
                        {
                            status = PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES;
                        }
                    }
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                *msgLen = bytes_read;
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}

static ptx_Status_t ptx_NDEF_T4T_UpdateLength (ptx_NDEF_T4T_t *ndefT4t, uint32_t newLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    uint8_t lc_field;
    uint8_t length_data_field[4];
    uint8_t nlen_offset = 0;
    uint32_t rx_len;

    if (PTX_COMP_CHECK(ndefT4t, Comp_T4TOP))
    {
        if(ptx_RUL_CardProtocolType_T4T == ndefT4t->T4t.Rul->CardRegistry.ActiveProtocol)
        {
            switch (ndefT4t->CCParams.MappingMajor)
            {
                case PTX_T4T_MAPPING_VERSION_MAJOR_2:
                    if (PTX_T4T_MAX_NLEN < newLen)
                    {
                        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    }
                    else
                    {
                        length_data_field[0] = (uint8_t)(((uint16_t)newLen & 0xFF00) >> 8u);
                        length_data_field[1] = (uint8_t)((uint16_t)newLen & 0x00FF);

                        lc_field = 2u;

                        status = ptx_NDEF_T4T_UpdateBinary(ndefT4t, nlen_offset, length_data_field, lc_field, &ndefT4t->RxBuffer[0], &rx_len, PTX_T4T_DEFAULT_TIMEOUT_MS);
                    }
                    break;
                case PTX_T4T_MAPPING_VERSION_MAJOR_3:
                    if (PTX_T4T_MAX_ENLEN < newLen)
                    {
                        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    }
                    else
                    {
                        length_data_field[3] = (uint8_t)(newLen & 0x000000FF);
                        length_data_field[2] = (uint8_t)((newLen & 0x0000FF00) >> 8u);
                        length_data_field[1] = (uint8_t)((newLen & 0x00FF0000) >> 16u);
                        length_data_field[0] = (uint8_t)((newLen & 0xFF000000) >> 24u);

                        lc_field = 4;

                        status = ptx_NDEF_T4T_UpdateBinary(ndefT4t, nlen_offset, length_data_field, lc_field, &ndefT4t->RxBuffer[0], &rx_len, PTX_T4T_DEFAULT_TIMEOUT_MS);
                    }
                    break;

                default:
                    status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                    break;
            }
            if (PTX_STATUS_SUCCESS == status)
            {
                ndefT4t->NLEN.DigitNLEN = newLen;
                ndefT4t->NLEN.NbrNLENBytes = lc_field;
                (void)memcpy(&ndefT4t->NLEN.NLEN[0], &length_data_field[0], (uint32_t)lc_field);

                if (newLen != 0)
                {
                    ndefT4t->LifeCycle = TagLC_ReadWrite;
                }
                else
                {
                    ndefT4t->LifeCycle = TagLC_Initialised;
                }
            }
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_PERMITTED;
        }
    }

    return status;
}
