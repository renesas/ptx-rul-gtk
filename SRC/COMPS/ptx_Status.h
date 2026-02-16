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
    Module      : Status Definitions
    File        : ptx_Status.h

    Description : Definition of status (error) codes, types and macros used by the stack.
*/

/**
 * \addtogroup STATUS_CODES
 * @{
 */

#if !defined(PTX_STATUS_H_)
#define PTX_STATUS_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>

/**
 * INTERFACE
 */

#if defined(__cplusplus)
extern "C"
{
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/**
 * \brief This macro is used to suppress compiler warnings about a parameter not being used in a function.
 */
#define PTX_UNUSED_PARAMETER(x)     (void)((x))

/**
 * \brief PTX_STATUS Type
 */
typedef uint16_t    ptx_Status_t;

/**
 * \brief GENERIC STATUS CODES
 */
#define PTX_STATUS_SUCCESS                          ((ptx_Status_t) 0x0000)     /**< Successful operation. */

/**
 * \brief GENERIC ERROR CODES
 */
#define PTX_STATUS_E_GEN_INVALID_PARAMETER          ((ptx_Status_t) 0x0001)     /**< Invalid input parameter. */
#define PTX_STATUS_E_GEN_INTERNAL                   ((ptx_Status_t) 0x0002)     /**< Internal error in the processing. */
#define PTX_STATUS_E_GEN_TIMEOUT                    ((ptx_Status_t) 0x0003)     /**< Action aborted due to timeout. */
#define PTX_STATUS_E_GEN_INVALID_STATE              ((ptx_Status_t) 0x0004)     /**< Invalid state: the action is not allowed in that state. */
#define PTX_STATUS_E_GEN_INVALID_BITRATE            ((ptx_Status_t) 0x0005)     /**< Interface invalid bitrate */
#define PTX_STATUS_E_GEN_INTERFACE_OPEN             ((ptx_Status_t) 0x0006)     /**< Interface could not be opened */
#define PTX_STATUS_E_GEN_INTERFACE_CLOSE            ((ptx_Status_t) 0x0007)     /**< Interface could not be closed */
#define PTX_STATUS_E_GEN_INTERFACE_TX               ((ptx_Status_t) 0x0008)     /**< Interface could not tx data */
#define PTX_STATUS_E_GEN_INTERFACE_RX               ((ptx_Status_t) 0x0009)     /**< Interface could not rx data */
#define PTX_STATUS_E_GEN_INTERFACE_NSS              ((ptx_Status_t) 0x000A)     /**< Interface could not assert NSS pin */
#define PTX_STATUS_E_GEN_INTERFACE_RX_IRQ           ((ptx_Status_t) 0x000B)     /**< Interface could not assert external interrupt pin */
#define PTX_STATUS_E_GEN_INTERFACE_IRQ_READ         ((ptx_Status_t) 0x000C)     /**< Interface could read external interrupt pin */
#define PTX_STATUS_E_GEN_INTERFACE_TIMEOUT          ((ptx_Status_t) 0x000D)     /**< Interface response took too long */
#define PTX_STATUS_E_GEN_TRANSFER_ABORTED           ((ptx_Status_t) 0x000E)     /**< Interface aborted transfer */
#define PTX_STATUS_E_GEN_INSUFFICIENT_RESOURCES     ((ptx_Status_t) 0x000F)     /**< Insufficient Resources to provide for required operation. */
#define PTX_STATUS_E_GEN_NOT_FOUND                  ((ptx_Status_t) 0x0010)     /**< System resource such as a file, path or method, not found. */
#define PTX_STATUS_E_GEN_INTERFACE_IRQ_DIRECTION    ((ptx_Status_t) 0x0011)     /**< Interface error, IRQ Pin direction (or other configuration) could not be set */
#define PTX_STATUS_E_GEN_INTERFACE_IRQ_EDGE         ((ptx_Status_t) 0x0012)     /**< Interface error, IRQ Pin edge could not be set */
#define PTX_STATUS_E_GEN_INTERFACE_IRQ_VALUE        ((ptx_Status_t) 0x0013)     /**< Interface error, IRQ Pin could not be read or written */
#define PTX_STATUS_E_GEN_NOT_IMPLEMENTED            ((ptx_Status_t) 0x0014)     /**< Not implemented function */
#define PTX_STATUS_E_GEN_FEATURE_DISABLED           ((ptx_Status_t) 0x0015)     /**< System Feature is disabled. */
#define PTX_STATUS_E_GEN_ACCESS_DENIED              ((ptx_Status_t) 0x0016)     /**< Access Denied to system resource or configuration parameter. */
#define PTX_STATUS_E_GEN_NOT_PERMITTED              ((ptx_Status_t) 0x0017)     /**< Operation not permitted. */
#define PTX_STATUS_E_GEN_ADCCLIPPING                ((ptx_Status_t) 0x0018)     /**< ADC overdriven (value is within clipping region). */
#define PTX_STATUS_E_GEN_NOT_AVAILABLE              ((ptx_Status_t) 0x0019)     /**< Unavailable functionality */
#define PTX_STATUS_E_GEN_WRONG_HARDWARE             ((ptx_Status_t) 0x001A)     /**< Silicon version incompatible with SDK */
#define PTX_STATUS_E_GEN_FW_INVALID                 ((ptx_Status_t) 0x001B)     /**< Invalid Firmware detected */
#define PTX_STATUS_E_GEN_OUT_OF_RANGE               ((ptx_Status_t) 0x001C)     /**< A value/measurement exceeds the valid range. */

/**
 * \brief ERROR CODES RELATED TO NSC
 */
#define PTX_STATUS_E_NSC_PROTOCOL                   ((ptx_Status_t) 0x0100)     /**< Error at NSC protocol. */
#define PTX_STATUS_E_NSC_RFERROR                    ((ptx_Status_t) 0x0101)     /**< Error at NSC. RF ERROR. */
#define PTX_STATUS_E_NSC_NO_TAG_AVAILABLE           ((ptx_Status_t) 0x0102)     /**< No Rf Config Tag available */
#define PTX_STATUS_E_NSC_NO_FIT_TAG                 ((ptx_Status_t) 0x0103)     /**< Tag does not fit into the buffer */
#define PTX_STATUS_E_NSC_TAG_NEVER_FITS             ((ptx_Status_t) 0x0104)     /**< Tag will never fit into our transfer buffer */
#define PTX_STATUS_E_NSC_RX_ERROR_LENGTH            ((ptx_Status_t) 0x0105)     /**< Received message in buffer is too big */
#define PTX_STATUS_E_NSC_INVALID_RFCONFIG           ((ptx_Status_t) 0x0106)     /**< RF Configuration file not supported */
#define PTX_STATUS_E_NSC_FW_DL_DENIED               ((ptx_Status_t) 0x0107)     /**< Firmware download is denied */
#define PTX_STATUS_E_NSC_ERR_STANDBY                ((ptx_Status_t) 0x0108)     /**< Exited Standby mode in DISCOVERY state. */
#define PTX_STATUS_E_NSC_COLLISION                  ((ptx_Status_t) 0x0109)     /**< Error during data exchange due to collision. */
#define PTX_STATUS_E_NSC_SYSTEM                     ((ptx_Status_t) 0x010A)     /**< Critical system error. */
#define PTX_STATUS_E_NSC_INVALID_DATA_LENGTH        ((ptx_Status_t) 0x010B)     /**< Invalid data message length. */
#define PTX_STATUS_E_NSC_RESIDUAL_BITS              ((ptx_Status_t) 0x010C)     /**< Residual bits detected during data exchange. */
#define PTX_STATUS_E_NSC_INSUFFICIENT_RESOURCES     ((ptx_Status_t) 0x010D)     /**< Insufficient Resources to provide for required operation. */
#define PTX_STATUS_E_NSC_INTERNAL                   ((ptx_Status_t) 0x010E)     /**< Internal error in the processing. */

/**
 * \brief ERROR CODES RELATED TO HAL
 */
#define PTX_STATUS_E_HAL_CANCELLED                  ((ptx_Status_t) 0x0200)     /**< HAL Operation Cancelled. */

/**
 * \brief ERROR CODES RELATED TO HAL: TIMERS
 */
#define PTX_STATUS_E_HALTIMER_FUNCTION_NOT_SET      ((ptx_Status_t) 0x0201)     /**< Timer function not set */
#define PTX_STATUS_E_HALTIMER_INTERNAL_OPEN         ((ptx_Status_t) 0x0202)     /**< Timer open failed */
#define PTX_STATUS_E_HALTIMER_INTERNAL_CLOSE        ((ptx_Status_t) 0x0203)     /**< Timer close failed */
#define PTX_STATUS_E_HALTIMER_INTERNAL_START        ((ptx_Status_t) 0x0204)     /**< Timer start failed */
#define PTX_STATUS_E_HALTIMER_INTERNAL_STOP         ((ptx_Status_t) 0x0205)     /**< Timer stop failed */

/**
 * \brief ERROR CODES RELATED TO HIP
 */
#define PTX_STATUS_E_HIP_NACK_INVALID_CMD           ((ptx_Status_t) 0x0300)     /**< Write command error: Invalid Command */
#define PTX_STATUS_E_HIP_NACK_INVALID_LEN           ((ptx_Status_t) 0x0301)     /**< Write command error: Invalid Length */
#define PTX_STATUS_E_HIP_NACK_ERROR_CRC             ((ptx_Status_t) 0x0302)     /**< Write command error: CRC Error */
#define PTX_STATUS_E_HIP_NACK_INVALID_PARAM         ((ptx_Status_t) 0x0303)     /**< Write command error: Invalid Parameter */
#define PTX_STATUS_E_HIP_NACK_WRITE_BUF_FULL        ((ptx_Status_t) 0x0304)     /**< Write command error: Write Buffer Full */
#define PTX_STATUS_E_HIP_RX_FRAME_INVALID_LENGTH    ((ptx_Status_t) 0x0305)     /**< Decode frame error: Invalid Length */
#define PTX_STATUS_E_HIP_RX_FRAME_INVALID_CRC       ((ptx_Status_t) 0x0306)     /**< Decode frame error: CRC Error */
#define PTX_STATUS_E_HIP_RX_BUFFER_TOO_SMALL        ((ptx_Status_t) 0x0307)     /**< Decode frame error: Provided RX Buffer is too small for data */
#define PTX_STATUS_E_HIP_RX_FRAME_INVALID_CMD       ((ptx_Status_t) 0x0308)     /**< Decode frame error: Sent and received CMD do not match */

/**
 * \brief ERROR CODES RELATED TO TFCI
 */
#define PTX_STATUS_E_TFCI_HALMISMATCH               ((ptx_Status_t) 0x0400)     /**< Mismatching previous HAL and newly initialised, must deInit first. */
#define PTX_STATUS_E_TFCI_HIPMISMATCH               ((ptx_Status_t) 0x0401)     /**< Mismatching previous HIP and newly initialised, must deInit first. */
#define PTX_STATUS_E_TFCI_NOTINCLUDED               ((ptx_Status_t) 0x0402)     /**< A call to a wrapper not included in the build was received. */
#define PTX_STATUS_E_TFCI_LOGGER_COMM_NOTAVAILABLE  ((ptx_Status_t) 0x0403)     /**< TFCI Logger is only available when a connected HDLC (communication channel) is available, TFCI application is expected for the TFCI Logger to function properly. */
#define PTX_STATUS_E_TFCI_NDEF_INVALID_SIZE         ((ptx_Status_t) 0x0404)     /**< TFCI NDEF size is invalid, and would cause buffer overflows. */
#define PTX_STATUS_E_TFCI_LOGMISMATCH               ((ptx_Status_t) 0x0405)     /**< Mismatching previous LOG and newly initialised, must deInit first. */

/**
 * \brief ERROR CODES RELATED TO NCI
 */
#define PTX_STATUS_E_NCI_IGNORE_MESSAGE             ((ptx_Status_t) 0x0500)     /**< CMD- or DATA-packet from the DH which shall be ignored by the NFCC */
#define PTX_STATUS_E_NCI_INTERFACE_MAPPING          ((ptx_Status_t) 0x0501)     /**< Wrong RF Interface mapping or Invalid Interface. */
#define PTX_STATUS_E_NCI_INTERFACE_EXT_MAPPING      ((ptx_Status_t) 0x0502)     /**< Wrong RF Interface Extension mapping or Interface Extension is not started. */
#define PTX_STATUS_E_NCI_PARAM_CORRUPTED            ((ptx_Status_t) 0x0503)     /**< Message contains corrupted data. */

/**
 * \brief ERROR CODES RELATED TO SWP
 */
#define PTX_STATUS_E_SWP_ACT_FAILED                 ((ptx_Status_t) 0x0600)     /**< SWP Activation process failed. */
#define PTX_STATUS_E_SWP_IDENTITY_ERROR             ((ptx_Status_t) 0x0601)     /**< Identity check failed during the SWP activation process. */
#define PTX_STATUS_E_SWP_DEACT_ERROR                ((ptx_Status_t) 0x0602)     /**< Error during the SWP de-activation process. */

/**
 * \brief ERROR CODES RELATED TO HCI
 */
#define PTX_STATUS_E_HCI_PIPE_NOTOPENED             ((ptx_Status_t) 0x0700)     /**< Pipe is not open. */
#define PTX_STATUS_E_HCI_RFPARAM_NOTWRITTEN         ((ptx_Status_t) 0x0701)     /**< RF parameters are not set.(e.g. MODE is no in enabled state.) */

/**
 * \brief ERROR CODES RELATED TO NDEF OPERATIONS
 */
#define PTX_STATUS_E_NDEF_UNSUPPORTED               ((ptx_Status_t) 0x0800)     /**< Unsupported Operation. */
#define PTX_STATUS_E_NDEF_NO_MESSAGES               ((ptx_Status_t) 0x0801)     /**< No NDEF Messages present on Tag. */
#define PTX_STATUS_E_NDEF_INVALID_TAG_TYPE          ((ptx_Status_t) 0x0802)     /**< Unknown/Wrong Tag TYpe. */
#define PTX_STATUS_E_NDEF_TAG_ACCESS_DENIED         ((ptx_Status_t) 0x0803)     /**< Tag Operation Access Denied. */
#define PTX_STATUS_E_NDEF_INVALID_TAG_STATE         ((ptx_Status_t) 0x0804)     /**< Invalid Tag LC State for requested operation. */
#define PTX_STATUS_E_NDEF_TLV_OVERWRITE_ERROR       ((ptx_Status_t) 0x0805)     /**< Data to be written would overwrite existing TLVs. */
#define PTX_STATUS_E_NDEF_PROTOCOL_ERROR            ((ptx_Status_t) 0x0806)     /**< Indicate that something is out of order in NDEF Operations. */
#define PTX_STATUS_E_NDEF_INSUFFICIENT_RESOURCES    ((ptx_Status_t) 0x0807)     /**< Not enough memory space. */
#define PTX_STATUS_E_NDEF_ASSERTION_ERROR           ((ptx_Status_t) 0x0808)     /**< An assertion has failed. */
#define PTX_STATUS_E_NDEF_NOT_FOUND                 ((ptx_Status_t) 0x0809)     /**< Something was missing on the Tag. */
#define PTX_STATUS_E_NDEF_T3T_WRITE_IN_PROGRESS     ((ptx_Status_t) 0x080A)     /**< T3T Update Operation has not finished. */
#define PTX_STATUS_E_NDEF_TAG_NACK                  ((ptx_Status_t) 0x080B)     /**< Tag responded with a NACK of some kind. */

/**
 * \brief ERROR CODES RELATED TO MFCC OPERATIONS
 */
#define PTX_STATUS_E_MFCC_AUTHENTICATION_FAILED     ((ptx_Status_t) 0x0900)     /**< Auth. Failed. */
#define PTX_STATUS_E_MFCC_NACK                      ((ptx_Status_t) 0x0901)     /**< NACK received. */
#define PTX_STATUS_E_MFCC_PROTOCOL_ERROR            ((ptx_Status_t) 0x0902)     /**< Indicate that something is out of order in MFCC Operations. */
#define PTX_STATUS_E_MFCC_INSUFFICIENT_RESOURCES    ((ptx_Status_t) 0x0903)     /**< Not enough memory space. */

/**
 * \brief ERROR CODES RELATED TO FELICA
 */
#define PTX_STATUS_E_FELICA_PROTOCOL_ERROR          ((ptx_Status_t) 0x0A00)     /**< Protocol error. */
#define PTX_STATUS_E_FELICA_PERMISSION_DENIED       ((ptx_Status_t) 0x0A01)     /**< Permission denied. */

/**
 * \brief ERROR CODES RELATED TO PSL
 */
#define PTX_STATUS_E_PSL_ENABLESIGNALMISSING        ((ptx_Status_t) 0x0B00)     /**< Missing enable signal. */

/**
 * \brief LLCP STATUS CODES.
 */
#define PTX_STATUS_LLCP_INVALID_PARAMETER                   ((ptx_Status_t) 0xC001)   /**< Invalid parameter. */
#define PTX_STATUS_LLCP_VERSION_AGREEMENT                   ((ptx_Status_t) 0xC002)   /**< Version agreement failure. */
#define PTX_STATUS_LLCP_MAC_DEACTIVATED_BY_REMOTE           ((ptx_Status_t) 0xC003)   /**< The MAC layer has been deactivated. */
#define PTX_STATUS_LLCP_TIMEOUT                             ((ptx_Status_t) 0xC004)   /**< Timeout exceeded. */
#define PTX_STATUS_LLCP_DISCONNECTED_BY_REMOTE              ((ptx_Status_t) 0xC005)   /**< The remote peer terminated the connection. */
#define PTX_STATUS_LLCP_CONNECTION_DISCONNECTED_BY_REMOTE   ((ptx_Status_t) 0xC006)   /**< The link layer has been deactivated. */
#define PTX_STATUS_LLCP_REMOTE_PEER_BUSY                    ((ptx_Status_t) 0xC007)   /**< The remote peer is currently busy. */
#define PTX_STATUS_LLCP_FRMR_EXCHANGED                      ((ptx_Status_t) 0xC008)   /**< The frame was rejected by the remote peer.*/
#define PTX_STATUS_LLCP_REMOTE_RW_FULL                      ((ptx_Status_t) 0xC009)   /**< The buffer of the remote peer is full.*/

/**
 * \brief SNEP STATUS CODES.
 */
#define PTX_STATUS_SNEP_INVALID_PARAMETER                   ((ptx_Status_t) 0xD001)   /**< Invalid parameter. */
#define PTX_STATUS_SNEP_INVALID_STATE                       ((ptx_Status_t) 0xD002)   /**< Invalid state. */
#define PTX_STATUS_SNEP_LLCP_NOT_SUPPORTED                  ((ptx_Status_t) 0xD003)   /**< LLCP not support (due to version mismatch). */
#define PTX_STATUS_SNEP_LLCP_PROTOCOL_ERROR                 ((ptx_Status_t) 0xD004)   /**< Protocol error on LLCP layer. */
#define PTX_STATUS_SNEP_LLCP_UNKNOWN_ERROR                  ((ptx_Status_t) 0xD005)   /**< Unknown error on LLCP layer. */
#define PTX_STATUS_SNEP_TIMEOUT_ERROR                       ((ptx_Status_t) 0xD006)   /**< Timeout error on LLCP layer. */
#define PTX_STATUS_SNEP_CONNECTION_LOSS_ERROR               ((ptx_Status_t) 0xD007)   /**< Connection loss. */
#define PTX_STATUS_SNEP_LINK_LOSS_ERROR                     ((ptx_Status_t) 0xD008)   /**< Link has been closed. */
#define PTX_STATUS_SNEP_OPERATION_FAILED                    ((ptx_Status_t) 0xD009)   /**< Operation failed. */

/**
 * \brief WLC STATUS CODES.
 */
#define PTX_STATUS_WLC_INVALID_STATE                        ((ptx_Status_t) 0xE001)   /**< The WLC state machine is in the wrong state. */
#define PTX_STATUS_WLC_INCOMPATIBLE_TAG_PROTOCOL            ((ptx_Status_t) 0xE002)   /**< The available protocols of the listener are not compatible with WLC. */
#define PTX_STATUS_WLC_INVALID_WLC_PROTOCOL_VERSION         ((ptx_Status_t) 0xE003)   /**< Invalid WLC Protocol Version of the WLC-L device. */
#define PTX_STATUS_WLC_CAP_RECORD_MISSING                   ((ptx_Status_t) 0xE004)   /**< Cannot read the WLC Capability Record. */
#define PTX_STATUS_WLC_NEGO_WAIT_MAX_NUM_RETRIES            ((ptx_Status_t) 0xE005)   /**< Max number of retires during waiting time for WLC-L in Negotiated mode. */
#define PTX_STATUS_WLC_CTL_RECORD_MISSING                   ((ptx_Status_t) 0xE006)   /**< Cannot read the WLC Control Record. */
#define PTX_STATUS_WLC_CTL_MAX_NUM_RETRIES                  ((ptx_Status_t) 0xE007)   /**< Max Number of retries during wait for valid WLC Control Record. */
#define PTX_STATUS_WLC_INVALID_MODE                         ((ptx_Status_t) 0xE008)   /**< The listener is in an invalid WLC Mode. */
#define PTX_STATUS_WLC_PROTOCOL_ERROR                       ((ptx_Status_t) 0xE009)   /**< The listener reported a protocol error. */
#define PTX_STATUS_WLC_INVALID_HOLD_OFF_DURATION            ((ptx_Status_t) 0xE00A)   /**< The hold-off duration contains an invalid value (must be <= 25% of WPT duration). */
#define PTX_STATUS_WLC_WPT_ICD                              ((ptx_Status_t) 0xE00B)   /**< The WPT cycle stopped due to a Impedance Change Detection (ICD). */
#define PTX_STATUS_WLC_WPT_TIMEOUT                          ((ptx_Status_t) 0xE00C)   /**< The WPT cycle stopped due to a timeout without a response of the chip. */

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */

