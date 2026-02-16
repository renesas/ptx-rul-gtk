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
    Module      : NFC Soft Controller (NSC) Interface Header for Command Processing
    File        : ptx_NSC_CmdProcessor.h

    Description : Definition of NFC Soft Controller Interface for Command Processing.
                  Contains function(s) for basic NSC protocol message processing and dispatching.
*/

/**
 * \addtogroup NSC_CMD_PROCESSOR
 *
 * @{
 */

#if !defined(PTX_NSC_CMDPROCESSOR_H)
#define PTX_NSC_CMDPROCESSOR_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NSC_Data_Int.h"
#include "ptx_NSC_RfActivate_Int.h"
#include "ptx_NSC_RfDeactivate_Int.h"
#include "ptx_NSC_RfDiscover_Int.h"
#include "ptx_NSC_Wlc_Int.h"
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
 * \name Frame-type definitions.
 * @{
 */
#define PTX_NSC_CMD_PROCESSOR_FRAMETYPE_OFFSET              (0u)            /**< Offset in message */
#define PTX_NSC_CMD_PROCESSOR_FRAMETYPE_PAYLOAD_OFFSET      (1u)            /**< NSC Frame-type Offset in payload. */
#define PTX_NSC_CMD_PROCESSOR_FRAMETYPE_MASK                (0xC0u)         /**< NSC Frame-type */
#define PTX_NSC_CMD_PROCESSOR_FRAMETYPE_CMD                 (0x00u)         /**< NSC Frame-type command */
#define PTX_NSC_CMD_PROCESSOR_FRAMETYPE_RSP                 (0x40u)         /**< NSC Frame-type response */
#define PTX_NSC_CMD_PROCESSOR_FRAMETYPE_NTF                 (0x80u)         /**< NSC Frame-type NTF */
#define PTX_NSC_CMD_PROCESSOR_FRAMETYPE_DATA                (0xC0u)         /**< NSC Frame-type Data */
/** @} */

/**
 * \name Data Message Definitions
 * @{
 */
#define PTX_NSC_LENGTH_MAX                  (264u)      /**< Maximum NSC message length: OPCODE (1By) + Payload (263) */
#define PTX_NSC_LENGTH_DATAMSG_MIN          (3u)        /**< Defined by minimum set of expected bytes in an NSC_DATA_MSG: OPCODE + FCB + DataLen */

#define PTX_NSC_DATA_MSG_CHAINING_FALSE     (0u)        /**< No Chaining */
#define PTX_NSC_DATA_MSG_CHAINING_TRUE      (1u)        /**< Chaining */
#define PTX_NSC_DATA_MSG_HEADER_LENGTH      (3u)        /**< MSG Header Length */
#define PTX_NSC_DATA_MSG_OPCODE             (0xC2)      /**< Opcode for a data message */

/*
 * NSC Data Message Indices (position in the message)
 *
 * Note: some indices overlap. This depends on the bit values of the FCB and is
 * applied appropriately during message parsing.
 */
#define PTX_NSC_DATA_MSG_IDX_OPCODE         (0x00)      /**< Opcode index */
#define PTX_NSC_DATA_MSG_IDX_FCB            (0x01)      /**< FCB index */
#define PTX_NSC_DATA_MSG_IDX_DATALEN        (0x02)      /**< Data Length index */
#define PTX_NSC_DATA_MSG_IDX_CTRL           (0x02)      /**< Control byte index */
#define PTX_NSC_DATA_MSG_IDX_DATA           (0x03)      /**< Data payload index */

/*
 * Frame Control Byte (FCB) of the NSC Data Message
 */
#define PTX_NSC_DATA_MSG_FCB_BIT_CHAIN      (0x08)      /**< Chaining bit */
#define PTX_NSC_DATA_MSG_FCB_BIT_STATUS     (0x10)      /**< Status Byte Present bit*/
#define PTX_NSC_DATA_MSG_FCB_BIT_CTRL       (0x80)      /**< Control Message bit */

/*
 * Status Byte of the NSC Data Message
 */
#define PTX_NSC_DATA_MSG_STATUSBY_BIT_ERR   (0x80)      /**< Error bit */
#define PTX_NSC_DATA_MSG_STATUSBY_BIT_COLL  (0x40)      /**< Collision bit */
#define PTX_NSC_DATA_MSG_STATUSBY_BIT_RES   (0x07)      /**< Residual bits */

/*
 * NSC Control Data Message Definitions.
 */
#define PTX_NSC_DATA_MSG_ACK                (0x00)      /**< Acknowledge Message */
#define PTX_NSC_DATA_MSG_NACK               (0x01)      /**< Not Acknowledge Message, see code for reason */
#define PTX_NSC_DATA_MSG_EOF                (0x02)      /**< Isolated EoF Message (only T5T protocol) */
#define PTX_NSC_DATA_MSG_ACK_LENGTH         (0x03)      /**< Acknowledge/NotAcknowledge Message Length */

/*
 * Isolated EoF Data Message Definitions.
 */
#define PTX_NSC_DATA_MSG_EOF                (0x02)      /**< Isolated EoF Message (T5T only) */
#define PTX_NSC_DATA_MSG_EOF_LENGTH         (0x03)      /**< Isolated EoF Message Length */
/** @} */

/**
 * \name ID of buffers handled by Hardware
 * @{
 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_0      ((uint16_t)0u)     /**< Write Buffer 0 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_1      ((uint16_t)1u)     /**< Write Buffer 1 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_2      ((uint16_t)2u)     /**< Write Buffer 2*/
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_3      ((uint16_t)3u)     /**< Write Buffer 3 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_4      ((uint16_t)4u)     /**< Write Buffer 4 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_5      ((uint16_t)5u)     /**< Write Buffer 5 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_6      ((uint16_t)6u)     /**< Write Buffer 6 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_7      ((uint16_t)7u)     /**< Write Buffer 7 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_8      ((uint16_t)8u)     /**< Write Buffer 8 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_9      ((uint16_t)9u)     /**< Write Buffer 9 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_10     ((uint16_t)10u)    /**< Write Buffer 10 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_11     ((uint16_t)11u)    /**< Write Buffer 11 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_12     ((uint16_t)12u)    /**< Write Buffer 12 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_13     ((uint16_t)13u)    /**< Write Buffer 13 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_14     ((uint16_t)14u)    /**< Write Buffer 14 */
#define PTX_NSC_CMD_PROCESSOR_WRITE_BUFFER_ID_15     ((uint16_t)15u)    /**< Write Buffer 15 */
/** @} */

/**
 * \name Control Message Types
 * @{
 */

/**
 * \brief Commands
 */
#define PTX_NSC_INIT_HW_CMD_OPCODE                          (0x02u)         /**< NSC Init Command opcode */
#define PTX_NSC_READ_CMD_OPCODE                             (0x03u)         /**< NSC Read Command opcode */
#define PTX_NSC_WRITE_CMD_OPCODE                            (0x04u)         /**< NSC Write Command opcode */
#define PTX_NSC_STANDBY_CMD_OPCODE                          (0x05u)         /**< NSC Standby Command opcode */
#define PTX_NSC_WAKEUP_CMD_OPCODE                           (0x06u)         /**< NSC Wakeup Command opcode */
#define PTX_NSC_LOOPBACK_CMD_OPCODE                         (0x07u)         /**< NSC Loopback Command opcode */
#define PTX_NSC_GET_STATUS_CMD_OPCODE                       (0x08u)         /**< NSC Get Status Command opcode */
#define PTX_NSC_RF_CONFIG_CMD_OPCODE                        (0x11u)         /**< NSC RF Config Command opcode */
#define PTX_NSC_RF_DISCOVER_CMD_OPCODE                      (0x12u)         /**< NSC RF Discover Command opcode */
#define PTX_NSC_RF_ACTIVATE_CMD_OPCODE                      (0x13u)         /**< NSC RF Activate Command opcode */
#define PTX_NSC_RF_DEACTIVATE_CMD_OPCODE                    (0x14u)         /**< NSC RF Deactivate Command opcode */
#define PTX_NSC_RF_SET_ROUTING_TABLE_CMD_OPCODE             (0x15u)         /**< NSC RF Set Routing Table Command opcode */
#define PTX_NSC_RF_SET_PARAM_CMD_OPCODE                     (0x16u)         /**< NSC RF Set Param Command opcode */
#define PTX_NSC_RF_TEST_CMD_OPCODE                          (0x17U)         /**< NSC RF Test Command opcode. */
#define PTX_NSC_WLC_WPT_START_CMD_OPCODE                    (0x32u)         /**< NSC WPT Start Command opcode */
#define PTX_NSC_WLC_WPT_STOP_CMD_OPCODE                     (0x33u)         /**< NSC WPT Stop Command opcode */

#define PTX_NSC_CMD_OPCODE_LEN                              (0x01u)         /**< NSC Command opcode length*/

/**
 * \brief Responses
 */
#define PTX_NSC_CMD_PROCESSOR_INIT_RSP                      (0x42u)         /**< NSC Init Response Code */
#define PTX_NSC_CMD_PROCESSOR_INIT_RSPLEN                   (0x02u)         /**< NSC Init Response Length */

#define PTX_NSC_CMD_PROCESSOR_READ_RSP                      (0x43u)         /**< NSC Read Response Code */
#define PTX_NSC_CMD_PROCESSOR_READ_RSPLEN                   (0x02u)         /**< NSC Read Response Minimum Length */

#define PTX_NSC_CMD_PROCESSOR_WRITE_RSP                     (0x44u)         /**< NSC Write Response Code */
#define PTX_NSC_CMD_PROCESSOR_WRITE_RSPLEN                  (0x02u)         /**< NSC Write Response Length */

#define PTX_NSC_CMD_PROCESSOR_STANDBY_RSP                   (0x45u)         /**< NSC Standby Response Code */
#define PTX_NSC_CMD_PROCESSOR_STANDBY_RSPLEN                (0x02u)         /**< NSC Standby Response Length */

#define PTX_NSC_CMD_PROCESSOR_WAKEUP_RSP                    (0x46u)         /**< NSC Wakeup Response Code */
#define PTX_NSC_CMD_PROCESSOR_WAKEUP_RSPLEN                 (0x02u)         /**< NSC Wakeup Response Length */

#define PTX_NSC_CMD_PROCESSOR_LOOPBACK_RSP                  (0x47u)         /**< NSC Loopback Response Code */
#define PTX_NSC_CMD_PROCESSOR_LOOPBACK_RSPLEN               (0x02u)         /**< NSC Loopback Response Minimum Length */

#define PTX_NSC_CMD_PROCESSOR_GETSTATUS_RSP                 (0x48u)         /**< NSC GetStatus Response Code */
#define PTX_NSC_CMD_PROCESSOR_GETSTATUS_RSPLEN              (0x0Au)         /**< NSC GetStatus Response Length */

#define PTX_NSC_CMD_PROCESSOR_RFCONFIG_RSP                  (0x51u)         /**< NSC RF Config Response Code */
#define PTX_NSC_CMD_PROCESSOR_RFCONFIG_RSPLEN               (0x02u)         /**< NSC RF Config Response Length */

#define PTX_NSC_CMD_PROCESSOR_RFDISCOVER_RSP                (0x52u)         /**< NSC RF Discover Response Code */
#define PTX_NSC_CMD_PROCESSOR_RFDISCOVER_RSPLEN             (0x02u)         /**< NSC RF Discover Response Length */

#define PTX_NSC_CMD_PROCESSOR_RFACTIVATE_RSP                (0x53u)         /**< NSC RF Activate Response Code */
#define PTX_NSC_CMD_PROCESSOR_RFACTIVATE_RSPLEN             (0x02u)         /**< NSC RF Activate Response Minimum Length */

#define PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_RSP              (0x54u)         /**< NSC RF Deactivate Response Code */
#define PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_RSPLEN           (0x03u)         /**< NSC RF Deactivate Response Length */

#define PTX_NSC_CMD_PROCESSOR_RFSETROUTINGTABLE_RSP         (0x55u)         /**< NSC RF Set Routing Table Response Code */
#define PTX_NSC_CMD_PROCESSOR_RFSETROUTINGTABLE_RSPLEN      (0x02u)         /**< NSC RF RF Set Routing Table Response Length */

#define PTX_NSC_CMD_PROCESSOR_RFSETPARAM_RSP                (0x56u)         /**< NSC RF SetParam Response Code */
#define PTX_NSC_CMD_PROCESSOR_RFSETPARAM_RSPLEN             (0x02u)         /**< NSC RF SetParam Response Length */

#define PTX_NSC_CMD_PROCESSOR_RFTEST_RSP                    (0x57u)         /**< NSC RF Test Response Code */
#define PTX_NSC_CMD_PROCESSOR_RFTEST_RSPLEN                 (0x02u)         /**< NSC RF Test Response Minimum Length */

#define PTX_NSC_CMD_PROCESSOR_WPT_START_RSP                 (0x72u)         /**< NSC WPT Start Response Code */
#define PTX_NSC_CMD_PROCESSOR_WPT_START_RSPLEN              (0x02u)         /**< NSC WPT Start Response Length */

#define PTX_NSC_CMD_PROCESSOR_WPT_STOP_RSP                  (0x73u)         /**< NSC WPT Stop Response Code */
#define PTX_NSC_CMD_PROCESSOR_WPT_STOP_RSPLEN               (0x02u)         /**< NSC WPT Stop Response Length */

/**
 * \brief Notifications
 */

/**
 * \brief NSC_RESET_NTF parameters
 */
#define PTX_NSC_CMD_PROCESSOR_RESET_NTF                     (0x90u)         /**< NSC Reset Notification Code */
#define PTX_NSC_CMD_PROCESSOR_RESET_NTFLEN                  (0x06u)         /**< NSC Reset Notification Length */
#define PTX_NSC_RESET_NTF_HWVER_IDX                         (0x01u)         /**< Reset Notification: HW version parameter index */
#define PTX_NSC_RESET_NTF_FWVER_IDX                         (0x02u)         /**< Reset Notification: FW version parameter index */
#define PTX_NSC_RESET_NTF_FWVER_LEN                         (0x02u)         /**< Reset Notification: FW version parameter length */
#define PTX_NSC_RESET_NTF_DERIVATE_IDX                      (0x04u)         /**< Reset Notification: commercial derivate parameter index */
#define PTX_NSC_RESET_NTF_INVALID_FIRMWARE                  (0x0000u)       /**< No Firmware available, or invalid Firmware (e.g. CRC invalid) */


#define PTX_NSC_CMD_PROCESSOR_RFFIELD_NTF                   (0x91u)         /**< NSC RF-Field Notification Code */
#define PTX_NSC_CMD_PROCESSOR_RFFIELD_NTFLEN                (0x02u)         /**< NSC RF-Field Notification Length */

#define PTX_NSC_CMD_PROCESSOR_RFDISCOVER_NTF                (0x92u)         /**< NSC RF Discover Notification Code */
#define PTX_NSC_CMD_PROCESSOR_RFACTIVATE_NTF                (0x93u)         /**< NSC RF Activate Notification Code */

#define PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_NTF              (0x94u)         /**< NSC RF Deactivate Notification Code */
#define PTX_NSC_CMD_PROCESSOR_RFDEACTIVATE_NTFLEN           (0x03u)         /**< NSC RF Deactivate Notification Length */

#define PTX_NSC_CMD_PROCESSOR_ERROR_NTF                     (0x95u)         /**< NSC Error Notification Code */
#define PTX_NSC_CMD_PROCESSOR_ERROR_NTFLEN                  (0x02u)         /**< NSC Error Notification Length */

#define PTX_NSC_CMD_PROCESSOR_WPT_STOP_NTF                  (0x96u)         /**< NSC WLC WPT Stop Notification Code */
#define PTX_NSC_CMD_PROCESSOR_WPT_STOP_NTFLEN               (0x02u)         /**< NSC WLC WPT Stop Notification Length */

#define PTX_NSC_CMD_PROCESSOR_RFLPCD_NTF                    (0x97u)         /**< NSC RF LPCD Notification Code */
#define PTX_NSC_CMD_PROCESSOR_RFLPCD_NTFLEN                 (0x01u)         /**< NSC RF LPCD Notification Length */

#define PTX_NSC_CMD_PROCESSOR_DBG_NTF                       (0x9Fu)         /**< NSC Debug Notification Code */
/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Response message IDs.
 */
typedef enum ptx_NSC_CmdProcessor_RspId
{
    ptx_NSC_CmdProcessor_RspId_None,            /**< None. */
    ptx_NSC_CmdProcessor_RspId_Init,            /**< Init response. */
    ptx_NSC_CmdProcessor_RspId_Read,            /**< Read response. */
    ptx_NSC_CmdProcessor_RspId_Write,           /**< Write response. */
    ptx_NSC_CmdProcessor_RspId_RfActivate,      /**< Rf-Activate response. */
    ptx_NSC_CmdProcessor_RspId_RfDeactivate,    /**< Rf-Deactivate response */
    ptx_NSC_CmdProcessor_RspId_RfDiscover,      /**< Rf-Discover response. */
    ptx_NSC_CmdProcessor_RspId_RfConfig,        /**< Rf-Config response. */
    ptx_NSC_CmdProcessor_RspId_RfSetParam,      /**< Rf-SetParam response. */
    ptx_NSC_CmdProcessor_RspId_WptStart,        /**< WPT Start response. */
    ptx_NSC_CmdProcessor_RspId_WptStop,         /**< WPT Stop response. */
    ptx_NSC_CmdProcessor_RspId_Standby,         /**< Standby response. */
    ptx_NSC_CmdProcessor_RspId_Wakeup,          /**< Wakeup response. */
    ptx_NSC_CmdProcessor_RspId_Loopback,        /**< Loopback response. */
    ptx_NSC_CmdProcessor_RspId_GetStatus,       /**< GetStatus response. */
    ptx_NSC_CmdProcessor_RspId_RfSetRoutingTbl, /**< Rf-SetRoutingTable response. */
    ptx_NSC_CmdProcessor_RspId_Max              /**< Safeguard. */
} ptx_NSC_CmdProcessor_RspId_t;

/**
 * \brief NSC Message structure.
 */
typedef struct ptx_NSC_CmdProcessor_Data
{
    uint8_t IsNewMsg;                                       /**< Set if content is new. */
    uint8_t Type;                                           /**< Type of message, e.g. RSP, NTF, or DATA. */
    uint8_t Payload[PTX_NSC_LENGTH_MAX];                    /**< Payload. */
    uint32_t Length;                                        /**< Length of the payload. */
} ptx_NSC_CmdProcessor_Data_t;

/**
 * \brief Response Data structure.
 */
typedef struct ptx_NSC_CmdProcessor_RspData
{
    uint8_t IsNewRsp;                                       /**< Set if content is new. */
    uint8_t Payload[PTX_NSC_LENGTH_MAX];                    /**< Payload. */
    uint32_t Length;                                        /**< Length of the payload. */
} ptx_NSC_CmdProcessor_RspData_t;

/**
 * \brief Data Message structure.
 */
typedef struct ptx_NSC_CmdProcessor_DataMsg
{
    uint8_t IsNewData;                                      /**< set if content is new. */
    uint8_t Fcb;                                            /**< Frame Control Byte. */
    uint8_t Type;                                           /**< Data Message Type. */
    uint8_t Payload[PTX_NSC_LENGTH_MAX];                    /**< Payload. */
    uint32_t Length;                                        /**< Length of the payload. */
} ptx_NSC_CmdProcessor_DataMsg_t;


/**
 * \brief Forward declaration of ptx_NSC structure.
 */
struct ptx_NSC;

/**
 * \brief CmdProcessor type definition.
 */
typedef void ptx_NSC_CmdProcessor_Ctx_t;

/**
 * \brief Callback definition to fetch a message.
 *
 * \param[in]   nsc       Pointer to component.
 * \param[out]  msg       Pointer to data.
 * \param[out]  msgLen    Length of data.
 * \param[in]   timeout   Timeout in milliseconds to stop the operation.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_CmdProcessor_FetchMsgCb_t) (struct ptx_NSC *nsc, uint8_t *msg, uint32_t *msgLen, uint32_t timeout);

/**
 * \brief Callback definition to process a response message.
 *
 * \param[in]   nsc       Pointer to component.
 * \param[out]  msg       Pointer to data.
 * \param[out]  msgLen    Length of data.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_CmdProcessor_ProcessRspMsgCb_t) (struct ptx_NSC *nsc, uint8_t *msg, uint32_t msgLen);

/**
 * \brief Callback definition to process a reset notification.
 *
 * \param[in]   ctx       Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_CmdProcessor_ProcessResetNtfCb_t) (ptx_NSC_CmdProcessor_Ctx_t *ctx);

/**
 * \brief Callback definition to process a RF field notification.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   field     Flag whether field is ON or OFF.w
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_CmdProcessor_ProcessRfFieldNtfCb_t) (ptx_NSC_CmdProcessor_Ctx_t *ctx, uint8_t field);

/**
 * \brief Callback definition to process a RF discover notification.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   ntf       Pointer to notification.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_CmdProcessor_ProcessRfDiscoverNtfCb_t) (ptx_NSC_CmdProcessor_Ctx_t *ctx, ptx_NSC_RfDiscover_Ntf_t *ntf);

/**
 * \brief Callback definition to process a RF LPCD notification.
 *
 * \param[in]   ctx       Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_CmdProcessor_ProcessRfLpcdNtfCb_t) (ptx_NSC_CmdProcessor_Ctx_t *ctx);

/**
 * \brief Callback definition to process a RF activate notification.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   ntf       Pointer to notification.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_CmdProcessor_ProcessRfActivateNtfCb_t) (ptx_NSC_CmdProcessor_Ctx_t *ctx, ptx_NSC_RfActivate_Ntf_t *ntf);

/**
 * \brief Callback definition to process a RF deactivate notification.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   ntf       Pointer to notification.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_CmdProcessor_ProcessRfDeactivateNtfCb_t) (ptx_NSC_CmdProcessor_Ctx_t *ctx, ptx_NSC_RfDeactivate_Ntf_t *ntf);

/**
 * \brief Callback definition to process a error notification.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   ntf       Pointer to notification.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_CmdProcessor_ProcessErrorNtfCb_t) (ptx_NSC_CmdProcessor_Ctx_t *ctx, ptx_NSC_Error_Ntf_t *ntf);

/**
 * \brief Callback definition to process a WLC WPT stop notification
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   ntf       Pointer to notification.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_CmdProcessor_ProcessWlcWptStopNtfCb_t) (ptx_NSC_CmdProcessor_Ctx_t *ctx, ptx_NSC_WLC_WptStop_Ntf_t *ntf);

/**
 * \brief Callback definition to process a data message.
 *
 * \param[in]   ctx       Pointer to component.
 * \param[in]   msg       Pointer to data message (structure).
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_CmdProcessor_ProcessDataMsgCb_t) (ptx_NSC_CmdProcessor_Ctx_t *ctx, ptx_NSC_RfData_Msg_t *msg);


/**
 * \brief Command Processor Configuration Structure.
 */
typedef struct ptx_NSC_CmdProcessor_Cfg
{
    ptx_NSC_CmdProcessor_ProcessResetNtfCb_t            ProcessResetNtf;        /**< NSC Reset notification handler callback */
    ptx_NSC_CmdProcessor_ProcessRfFieldNtfCb_t          ProcessRfFieldNtf;      /**< NSC RF Field notification handler callback */
    ptx_NSC_CmdProcessor_ProcessRfDiscoverNtfCb_t       ProcessRfDiscoverNtf;   /**< NSC RF Discovery notification handler callback */
    ptx_NSC_CmdProcessor_ProcessRfLpcdNtfCb_t           ProcessRfLpcdNtf;       /**< NSC LPCD (low power card detection) notification handler callback */
    ptx_NSC_CmdProcessor_ProcessRfActivateNtfCb_t       ProcessRfActivateNtf;   /**< NSC RF Activate notification handler callback */
    ptx_NSC_CmdProcessor_ProcessRfDeactivateNtfCb_t     ProcessRfDeactivateNtf; /**< NSC RF Deactivate notification handler callback */
    ptx_NSC_CmdProcessor_ProcessErrorNtfCb_t            ProcessErrorNtf;        /**< NSC Error notification handler callback */
    ptx_NSC_CmdProcessor_ProcessWlcWptStopNtfCb_t       ProcessWptStopNtf;      /**< NSC WLC WPT (wireless power transfer) notification handler callback */
    ptx_NSC_CmdProcessor_ProcessDataMsgCb_t             ProcessDataMsg;         /**< NSC Data message handler callback */

    ptx_NSC_CmdProcessor_Ctx_t                          *Ctx;                   /**< Context for data and notifications callback functions */
} ptx_NSC_CmdProcessor_Cfg_t;


/**
 * \brief Command Processor Main Structure.
 */
typedef struct ptx_NSC_CmdProcessor
{
    ptx_NSC_CmdProcessor_Data_t                         LastMsg;                /**< Last received message */

    ptx_NSC_CmdProcessor_FetchMsgCb_t                   FetchMsg;               /**< Callback to fetch messages */

    ptx_NSC_CmdProcessor_ProcessResetNtfCb_t            ProcessResetNtf;        /**< NSC Reset notification handler callback */
    ptx_NSC_CmdProcessor_ProcessRfFieldNtfCb_t          ProcessRfFieldNtf;      /**< NSC RF Field notification handler callback */
    ptx_NSC_CmdProcessor_ProcessRfDiscoverNtfCb_t       ProcessRfDiscoverNtf;   /**< NSC RF Discovery notification handler callback */
    ptx_NSC_CmdProcessor_ProcessRfLpcdNtfCb_t           ProcessRfLpcdNtf;       /**< NSC LPCD (low power card detection) notification handler callback */
    ptx_NSC_CmdProcessor_ProcessRfActivateNtfCb_t       ProcessRfActivateNtf;   /**< NSC RF Activate notification handler callback */
    ptx_NSC_CmdProcessor_ProcessRfDeactivateNtfCb_t     ProcessRfDeactivateNtf; /**< NSC RF Deactivate notification handler callback */
    ptx_NSC_CmdProcessor_ProcessErrorNtfCb_t            ProcessErrorNtf;        /**< NSC Error notification handler callback */
    ptx_NSC_CmdProcessor_ProcessWlcWptStopNtfCb_t       ProcessWptStopNtf;      /**< NSC WLC WPT (wireless power transfer) notification handler callback */
    ptx_NSC_CmdProcessor_ProcessDataMsgCb_t             ProcessDataMsg;         /**< NSC Data message handler callback */

    ptx_NSC_CmdProcessor_Ctx_t                          *Ctx;                   /**< Context for data and notifications callback functions */
} ptx_NSC_CmdProcessor_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Configure CmdProcessor structure.
 *  Parameters which are not NULL will be set to the provided values. All other are kept unchanged.
 *
 * \param[in] nsc           Pointer to component.
 * \param[in] cfgParams     Configuration parameters for NSC Command Processor.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_CmdProcessor_Config(struct ptx_NSC *nsc, ptx_NSC_CmdProcessor_Cfg_t *cfgParams);

/**
 * \brief Dispatch a message.
 *
 * \param[in] nsc           Pointer to component.
 * \param[in] writeBuffer   Buffer to write data in
 * \param[in] msg           Pointer to message
 * \param[in] msgLen        Length of the message
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_CmdProcessor_DispatchMsg(struct ptx_NSC *nsc, uint16_t writeBuffer, uint8_t *msg, uint32_t msgLen);

/**
 * \brief Fetches a message with the requested op-code.
 * This function reads messages sent from PTX2xxR/W, until the message with the requested op-code is received.
 *
 * \note Maximum number of messages retrieved (if expected message does not come) is defined with PTX_NSC_FETCHMSG_COUNT_MAX.
 * Maximum time to wait for a single message is provided in the timeout parameter.
 * This function blocks the execution until a message is available for a maximum number of milliseconds provided in timeout.
 *
 * \param[in] nsc           Pointer to component.
 * \param[in] msg           Pointer to data.
 * \param[in] msgLen        Length of data.
 * \param[in] timeout       Timeout in milliseconds.
 * \param[in] msgOpCode     Expected message op-code. If set to 0, the first message is taken.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_CmdProcessor_FetchMsg(struct ptx_NSC *nsc, uint8_t *msg, uint32_t *msgLen, uint32_t timeout, uint8_t msgOpCode);

/**
 * \brief Processes a response message.
 *
 * \note Typically, a response message consists of OPCODE and STATUS/ERROR bytes.
 * However, some response messages, e.g. NSC_RF_ACTIVATE_RSP contain also data payload.
 * If there is data payload in the response, it is copied to the provided data buffer.
 * The caller needs to allocate enough space for data storage.
 *
 * \param[in] nsc           Pointer to component.
 * \param[in] rspId         Id (opcode) of the response to be checked.
 * \param[in] msg           Pointer to data.
 * \param[in] msgLen        Length of data.
 * \param[out] data         Data returned in response.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_CmdProcessor_ProcessRsp(struct ptx_NSC *nsc, ptx_NSC_CmdProcessor_RspId_t rspId, uint8_t *msg, uint32_t msgLen, uint8_t *data);

/**
 * \brief Processes a message.
 *
 * \param[in] nsc           Pointer to component.
 * \param[in] msg           Pointer to data.
 * \param[in] msgLen        Length of data.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_CmdProcessor_ProcessMsg(struct ptx_NSC *nsc, uint8_t *msg, uint32_t msgLen);

/**
 * \brief Gets a pending message.
 *
 * \note This function checks if an NSC message is pending (if IRQ line is set or not) and reads the message if it is available.
 * If there is no pending message, the function exits immediately.
 *
 * \param[in] nsc           Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_CmdProcessor_GetPendingMsg(struct ptx_NSC *nsc);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
