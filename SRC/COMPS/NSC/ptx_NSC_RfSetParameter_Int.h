
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
    Module      : NFC Soft Controller (NSC) Internal Interface Header for Set Parameter
    File        : ptx_NSC_RfSetParameter_Int.h

    Description : Definition of NFC Soft Controller Internal Interface for Set Parameter.
                  Contains definition of types specific for Set Parameter interface.
*/

/**
 * \addtogroup NSC_RF_SET_PARAM
 *
 * @{
 */

#if !defined(PTX_NSC_RFSETPARAMETER_INT_H)
#define PTX_NSC_RFSETPARAMETER_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name Length definitions for NSC RfSetParmeter command.
 *
 * @{
 */
#define PTX_NSC_RF_SET_PARAM_RF_TECH_LEN                    (2u)        /**< NSC RF Set Param command parameter RF_TECH length */
#define PTX_NSC_RF_SET_PARAM_TX_BIT_RATE_LEN                (2u)        /**< NSC RF Set Param command parameter TX_BIT_RATE length */
#define PTX_NSC_RF_SET_PARAM_RX_BIT_RATE_LEN                (2u)        /**< NSC RF Set Param command parameter RX_BIT_RATE length */
#define PTX_NSC_RF_SET_PARAM_PARITY_LEN                     (2u)        /**< NSC RF Set Param command parameter PARITY length */
#define PTX_NSC_RF_SET_PARAM_CRC_LEN                        (2u)        /**< NSC RF Set Param command parameter CRC length */
#define PTX_NSC_RF_SET_PARAM_RX_BIT_OFFSET_LEN              (2u)        /**< NSC RF Set Param command parameter RX_BIT_OFFSET length */
#define PTX_NSC_RF_SET_PARAM_BIT_MAXRSPLIM_LEN              (2u)        /**< NSC RF Set Param command parameter MAX_RSP_LIM length */
#define PTX_NSC_RF_SET_PARAM_BIT_FWT_LEN                    (4u)        /**< NSC RF Set Param command parameter FWT length */
/** @} */

/**
 * \name Bit-rate settings for NSC RfSetParameter command.
 *
 * @{
 */
#define PTX_NSC_RF_SET_BITRATE_106                        (0x00u)     /**< NSC RF Type bit-rate 106kbps */
#define PTX_NSC_RF_SET_BITRATE_212                        (0x01u)     /**< NSC RF Type bit-rate 212kbps */
#define PTX_NSC_RF_SET_BITRATE_424                        (0x02u)     /**< NSC RF Type bit-rate 424kbps */
#define PTX_NSC_RF_SET_BITRATE_848                        (0x03u)     /**< NSC RF Type bit-rate 848kbps */
#define PTX_NSC_RF_SET_BITRATE_26_5                       (0x06u)     /**< NSC RF Type bit-rate 26.5kbps */
#define PTX_NSC_RF_SET_BITRATE_53                         (0x07u)     /**< NSC RF Type bit-rate 53kbps */
/** @} */

/**
 * \name Internal length definitions.
 * @{
 */
#define PTX_NSC_RF_FWT_PARAM_LEN                    (3u)       /**< NSC FWT parameter length */
#define PTX_NSC_RF_SET_PARAMS_MAX                   (8u)       /**< NSC RF set Parameters maximum count */
#define PTX_NSC_RF_SET_PARAMS_MAX_BYTE_LEN          (PTX_NSC_RF_SET_PARAMS_MAX + PTX_NSC_RF_FWT_PARAM_LEN)       /**< NSC RF set Parameters maximum raw bytes count */
/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief The parameters of the set parameter command are grouped into logical subsets (POLL, POLL_A, POLL_B, …).
 * ptx_NSC_RfSetParameter_Param_Type_t describes the IDs for the individual subset.
 */
typedef enum ptx_NSC_RfSetParameter_Param_Type
{
    NSC_RfSetParameter_Param_Type_EoC          = 0x00, /**< End of command. */
    NSC_RfSetParameter_Param_Type_RfTech       = 0x01, /**< RF Technology. */
    NSC_RfSetParameter_Param_Type_TxBitRate    = 0x02, /**< Tx Bit Rate. */
    NSC_RfSetParameter_Param_Type_RxBitRate    = 0x03, /**< Rx Bit Rate. */
    NSC_RfSetParameter_Param_Type_Parity       = 0x04, /**< Parity. */
    NSC_RfSetParameter_Param_Type_Crc          = 0x05, /**< CRC. */
    NSC_RfSetParameter_Param_Type_RxBitOffset  = 0x06, /**< Rx Bit Offset. */
    NSC_RfSetParameter_Param_Type_MaxRspLim    = 0x07, /**< Maximum response limit. */
    NSC_RfSetParameter_Param_Type_Fwt          = 0x08  /**< FWT. */
} ptx_NSC_RfSetParameter_Param_Type_t;

/**
 * \brief Describes the parameters of the RF technology configuration.
 */
typedef enum ptx_NSC_RfSetParameter_Param_Rf_Tech
{
    NSC_RfSetParameter_RfTech_NFC_A = 0x01, /**< Type A. */
    NSC_RfSetParameter_RfTech_NFC_B = 0x02, /**< Type B. */
    NSC_RfSetParameter_RfTech_NFC_F = 0x03, /**< Type F. */
    NSC_RfSetParameter_RfTech_NFC_V = 0x04  /**< Type V. */
} ptx_NSC_RfSetParameter_Param_Rf_Tech_t;

/**
 * \brief Describes the Rx or Tx bit-rate.
 */
typedef struct ptx_NSC_RfSetParameter_Param_BitRate
{
    uint8_t BitrateValue; /**< Tx Bit Rate. */
} ptx_NSC_RfSetParameter_Param_BitRate_t;

/**
 * \brief Tx and Rx parity bit values.
 */
typedef enum ptx_NSC_RfSetParameter_Parity_Cfg
{
    NSC_RfSetParameter_Parity_Disable   = 0,    /**< Parity disable. */
    NSC_RfSetParameter_Parity_Enable    = 1U    /**< Parity enable. */
} ptx_NSC_RfSetParameter_Parity_Cfg_t;

/**
 * \brief Describes Tx and Rx parity bits.
 */
typedef struct ptx_NSC_RfSetParameter_Param_Parity
{
    ptx_NSC_RfSetParameter_Parity_Cfg_t TxParity;   /**< Tx Parity. */
    ptx_NSC_RfSetParameter_Parity_Cfg_t RxParity;   /**< Rx Parity. */
} ptx_NSC_RfSetParameter_Param_Parity_t;

/**
 * \brief Describes Tx and Rx CRC bit values.
 */
typedef enum ptx_NSC_RfSetParameter_Crc_Cfg
{
    NSC_RfSetParameter_Crc_Disable  = 0,    /**< CRC disable. */
    NSC_RfSetParameter_Crc_Enable   = 1U,   /**< CRC enable. */
} ptx_NSC_RfSetParameter_Crc_Cfg_t;

/**
 * \brief Describes CRC enabler parameter.
 */
typedef struct ptx_NSC_RfSetParameter_Param_Crc
{
    ptx_NSC_RfSetParameter_Crc_Cfg_t    TxCrc;  /**< Tx CRC. */
    ptx_NSC_RfSetParameter_Crc_Cfg_t    RxCrc;  /**< Rx CRC. */
} ptx_NSC_RfSetParameter_Param_Crc_t;

/**
 * \brief Describes Rx Bit Offset parameter.
 */
typedef struct ptx_NSC_RfSetParameter_Param_RxBitOffset
{
    uint8_t RxBitOffsetValue;   /**< Bit offset of the 1st byte of the received data. */
} ptx_NSC_RfSetParameter_Param_RxBitOffset_t;

/**
 * \brief Describes max response parameter.
 */
typedef struct ptx_NSC_RfSetParameter_Param_Max_Rsp_Lim
{
    uint8_t MaxRspLimValue;    /**< Defines max number of responses. If set 0 wait until FWT expires. */
} ptx_NSC_RfSetParameter_Param_Max_Rsp_Lim_t;

/**
 * \brief Describes FWT frame waiting time parameter.
 */
typedef struct ptx_NSC_RfSetParameter_Param_Fwt
{
    uint8_t FwtValue[PTX_NSC_RF_FWT_PARAM_LEN];     /**< Frame Waiting Time in 128/fc time units. */
} ptx_NSC_RfSetParameter_Param_Fwt_t;

/**
 * \brief Union of parameter values
 */
typedef union ptx_NSC_RfSetParameter_Param_Value
{
    ptx_NSC_RfSetParameter_Param_Rf_Tech_t      RfTech;         /**< RF Technology. */
    ptx_NSC_RfSetParameter_Param_BitRate_t      TxBitRate;      /**< Tx Bit Rate. */
    ptx_NSC_RfSetParameter_Param_BitRate_t      RxBitRate;      /**< Rx Bit Rate. */
    ptx_NSC_RfSetParameter_Param_Parity_t       Parity;         /**< Parity. */
    ptx_NSC_RfSetParameter_Param_Crc_t          Crc;            /**< CRC. */
    ptx_NSC_RfSetParameter_Param_RxBitOffset_t  RxBitOffset;    /**< Rx Bit Offset. */
    ptx_NSC_RfSetParameter_Param_Max_Rsp_Lim_t  MaxRspLim;      /**< Max Response Limit. */
    ptx_NSC_RfSetParameter_Param_Fwt_t          Fwt;            /**< Frame Waiting Time. */
} ptx_NSC_RfSetParameter_Param_Value_t;

/**
 * \brief Type-Value-Pair storing individual poll mode parameters.
 */
typedef struct ptx_NSC_RfSetParameter_Param
{
    ptx_NSC_RfSetParameter_Param_Type_t   Type;     /**< Parameter Type. */
    ptx_NSC_RfSetParameter_Param_Value_t  Value;    /**< Parameter Value. */
} ptx_NSC_RfSetParameter_Param_t;

/**
 * \brief Callback definition to parse a parameter
 *
 * \param[in,out]       bufferPos       Position where the buffer is able to write to
 * \param[in,out]       availableLen    Pointer to available length in \see bufferPos
 * \param[in]           param           Pointer to parameter to parse
 */
typedef ptx_Status_t (*ptx_NSC_RfSetParameter_ParamParserCb_t)(uint8_t** bufferPos, uint32_t* availableLen, ptx_NSC_RfSetParameter_Param_Value_t* param);

/**
 * \brief Structure used to map parameter type to parameter parser.
 */
typedef struct ptx_NSC_CmdSetParameter_ParserApi
{
    ptx_NSC_RfSetParameter_Param_Type_t    Type;    /**< Type of parameter. */
    ptx_NSC_RfSetParameter_ParamParserCb_t Parser;  /**< Parser for parameter. */
} ptx_NSC_RfSetParameter_ParserApi_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
