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
    File        : ptx_HCET4T.h

    Description : Implementation of Host Card Emulation (HCE) for T4T. This enables the emulation of the
                  PTX2xxR/W device as a T4T Card.
*/

/**
 * \addtogroup HCE_T4T
 * @{
 */

#if !defined(PTX_HCET4T_H)
#define PTX_HCET4T_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stddef.h>

#include "ptx_Components.h"
#include "ptx_Status.h"


#if defined(__cplusplus)
extern "C" {
#endif


/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
/**
 * \name ISO-7816 INS Standard Definitions.
 * @{
 */
#define PTX_HCET4T_ISO7816_INS_SELECT                                     (uint8_t)0xA4
#define PTX_HCET4T_ISO7816_INS_READ_BINARY                                (uint8_t)0xB0
#define PTX_HCET4T_ISO7816_INS_UPDATE_BINARY                              (uint8_t)0xD6
#define PTX_HCET4T_ISO7816_SELECT_OPTION_P1_BY_NAME                       (uint8_t)0x04
#define PTX_HCET4T_ISO7816_SELECT_OPTION_P2_NAME_FIRST_ONLY_OCC           (uint8_t)0x00
#define PTX_HCET4T_ISO7816_SELECT_OPTION_P1_BY_FILEID                     (uint8_t)0x00
#define PTX_HCET4T_ISO7816_SELECT_OPTION_P2_FILEID_FIRST_ONLY_OCC         (uint8_t)0x0C
/** @} */

/**
 * \name ISO-7816 CLA Definitions.
 * @{
 */
#define PTX_HCET4T_ISO7816_CLA_ISO                            (uint8_t)0x00
/** @} */

/**
 * \name ISO-7816 Status Word Definitions.
 * @{
 */
#define PTX_HCET4T_ISO7816_SW_LEN                             (uint8_t)0x02
#define PTX_HCET4T_ISO7816_SW_SUCCESS                         (uint16_t)0x9000
#define PTX_HCET4T_ISO7816_SW_BYTES_REMAINING_00              (uint16_t)0x6100
#define PTX_HCET4T_ISO7816_SW_WRONG_LENGTH                    (uint16_t)0x6700
#define PTX_HCET4T_ISO7816_SW_SECURITY_STATUS_NOT_SATISFIED   (uint16_t)0x6982
#define PTX_HCET4T_ISO7816_SW_FILE_INVALID                    (uint16_t)0x6983
#define PTX_HCET4T_ISO7816_SW_DATA_INVALID                    (uint16_t)0x6984
#define PTX_HCET4T_ISO7816_SW_CONDITIONS_NOT_SATISFIED        (uint16_t)0x6985
#define PTX_HCET4T_ISO7816_SW_COMMAND_NOT_ALLOWED             (uint16_t)0x6986
#define PTX_HCET4T_ISO7816_SW_APPLET_SELECT_FAILED            (uint16_t)0x6999
#define PTX_HCET4T_ISO7816_SW_FUNC_NOT_SUPPORTED              (uint16_t)0x6A81
#define PTX_HCET4T_ISO7816_SW_FILE_NOT_FOUND                  (uint16_t)0x6A82
#define PTX_HCET4T_ISO7816_SW_RECORD_NOT_FOUND                (uint16_t)0x6A83
#define PTX_HCET4T_ISO7816_SW_INCORRECT_P1P2                  (uint16_t)0x6A86
#define PTX_HCET4T_ISO7816_SW_WRONG_P1P2                      (uint16_t)0x6B00
#define PTX_HCET4T_ISO7816_SW_CORRECT_LENGTH_00               (uint16_t)0x6C00
#define PTX_HCET4T_ISO7816_SW_INS_NOT_SUPPORTED               (uint16_t)0x6D00
#define PTX_HCET4T_ISO7816_SW_CLA_NOT_SUPPORTED               (uint16_t)0x6E00
#define PTX_HCET4T_ISO7816_SW_UNKNOWN                         (uint16_t)0x6F00
#define PTX_HCET4T_ISO7816_SW_FILE_FULL                       (uint16_t)0x6A84
#define PTX_HCET4T_ISO7816_SW_LOGICAL_CHANNEL_NOT_SUPPORTED   (uint16_t)0x6881
#define PTX_HCET4T_ISO7816_SW_SECURE_MESSAGING_NOT_SUPPORTED  (uint16_t)0x6882
#define PTX_HCET4T_ISO7816_SW_WARNING_STATE_UNCHANGED         (uint16_t)0x6200
#define PTX_HCET4T_ISO7816_SW_SELECTED_FILE_DEACTIVATED       (uint16_t)0x6283
#define PTX_HCET4T_ISO7816_SW_NO_INPUT_DATA_AVAILABLE         (uint16_t)0x6286
/** @} */

/**
 * \name ISO-7816 Offsets for Processing.
 * @{
 */
#define PTX_HCET4T_ISO7816_HEADER_LEN                         (uint8_t)0x04
#define PTX_HCET4T_ISO7816_OFFSET_CLA                         (uint8_t)0x00
#define PTX_HCET4T_ISO7816_OFFSET_INS                         (uint8_t)0x01
#define PTX_HCET4T_ISO7816_OFFSET_P1                          (uint8_t)0x02
#define PTX_HCET4T_ISO7816_OFFSET_P2                          (uint8_t)0x03
#define PTX_HCET4T_ISO7816_OFFSET_LC                          (uint8_t)0x04
#define PTX_HCET4T_ISO7816_OFFSET_CDATA                       (uint8_t)0x05
#define PTX_HCET4T_ISO7816_OFFSET_EXT_CDATA                   (uint8_t)0x07
#define PTX_HCET4T_ISO7816_OFFSET_LE                          (uint8_t)0x04   /* only valid for C-APDUs without data (!) */
/** @} */

/**
 * \name NFC Forum Listener Compliance data lengths
 * @{
 */
#define PTX_HCET4T_LISTEN_SELECT_RES_DTA_LEN                    (uint8_t)2
#define PTX_HCET4T_LISTEN_SELECT_DTA_APDU_LEN                   (uint8_t)16
#define PTX_HCET4T_LISTEN_SELECT_APDU_LEN                       (uint8_t)4
#define PTX_HCET4T_LISTEN_LOOPBACK_CAPDU_LEN                    (uint8_t)4
/** @} */
/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * T4T Internal Application States
 */
typedef enum ptx_HCET4T_State
{
    HCET4T_State_Reset,
    HCET4T_State_NDEF_Application_Selected,
    HCET4T_State_CC_File_Selected,
    HCET4T_State_NDEF_File_Selected,

} ptx_HCET4T_State_t;

/**
 * \brief T4T Card Initialisation Parameters.
 */
typedef struct ptx_HCET4T_InitParams
{
    uint8_t                 *NDEFFile;
    uint32_t                NDEFFileSize;

    uint8_t                 *CCFile;
    uint32_t                CCFileSize;
} ptx_HCET4T_InitParams_t;

/**
 * \brief T4T Card Parameters.
 */
typedef struct ptx_HCET4T
{
    ptx_Comps_t             CompId;

    ptx_HCET4T_State_t      State;

    uint8_t                 *NDEFFile;
    uint32_t                NDEFFileSize;

    uint8_t                 *CCFile;
    uint32_t                CCFileSize;
} ptx_HCET4T_t;
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief HCE T4T Initialisation.
 *
 * \param[in] t4t               T4T Card Context.
 * \param[in] initParams        Initialisation Parameters.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                An error occurred in lower layers. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HCET4T_Init(ptx_HCET4T_t *t4t, ptx_HCET4T_InitParams_t *t4tInitParams);

/**
 * \brief HCE T4T Deinitialization.
 *
 * \param[in] t4t               T4T Card Context.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                An error occurred in lower layers. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HCET4T_DeInit(ptx_HCET4T_t *t4t);

/**
 * \brief Reset the emulated T4T.
 *
 * \param[in] t4t               T4T Card Context.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                An error occurred in lower layers. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HCET4T_Reset(ptx_HCET4T_t *t4t);

/**
 * \brief Update the emulated T4T memory.
 *
 * \param[in] t4t               T4T Card Context.
 * \param[in] startIndex        First memory position to update.
 * \param[in] data              Data to be written.
 * \param[in] dataLength        Length of Data to be written.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                An error occurred in lower layers. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HCET4T_UpdateMemory(ptx_HCET4T_t *t4t, uint16_t startIndex, uint8_t *data, uint32_t dataLength);

/**
 * \brief Process a received T4T command.
 *
 * \param[in]     t4t           T4T Card Context.
 * \param[in]     rx            Received Data.
 * \param[in]     rxLen         Length of received Data.
 * \param[out]    tx            Data to be Sent.
 * \param[in,out] txLen         Length of Tx buffer as input, Length of Data to be sent as output.
 *
 * \return PTX_STATUS_SUCCESS                    Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER    Invalid input parameter.
 * \return Others                                An error occurred in lower layers. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_HCET4T_CommandProcess(ptx_HCET4T_t *t4t, uint8_t *rx, uint32_t rxLen, uint8_t *tx, uint32_t *txLen);

ptx_Status_t ptx_HCET4T_NFCForumLoopback_CommandProcess(ptx_HCET4T_t *t4t, uint8_t *rx, uint32_t rxLen, uint8_t *tx, uint32_t *txLen);
#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

