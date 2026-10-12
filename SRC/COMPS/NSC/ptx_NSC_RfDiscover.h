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
    Module      : NFC Soft Controller (NSC) Interface Header for RF Discovery
    File        : ptx_NSC_RfDiscover.h

    Description : Definition of NFC Soft Controller Interface for RF Discovery.
                  Contains functions to start RF Discovery process and to parse the information gathered
                  during discovery.
*/

/**
 * \addtogroup NSC_RF_DISCOVER
 *
 * @{
 */

#if !defined(PTX_NSC_RFDISCOVER_H)
#define PTX_NSC_RFDISCOVER_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Int.h"


#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \addtogroup NSC_RF_DISCOVER_CFG
 *
 * @{
 */

/**
 * \name NSC Discovery - poll mode and technology parameters enabler(s).
 * @{
 */
#define PTX_NSC_POLLING_DISABLED        0       /**< Polling disabled. */
#define PTX_NSC_POLLING_ENABLED         1U      /**< Polling enabled. */
#define PTX_NSC_POLLTECH_DISABLED       0       /**< Polling technology disabled. */
#define PTX_NSC_POLLTECH_ENABLED        1U      /**< Polling technology enabled. */
/** @} */

/**
 * \name NSC Discovery poll mode definitions.
 * @{
 */
#define PTX_NSC_POLLMODE_DISABLED       0       /**< Polling disabled. */
#define PTX_NSC_POLLMODE_NFC_FORUM      1U      /**< NFC Forum Polling mode. */
#define PTX_NSC_POLLMODE_EMVCO          2U      /**< EmvCo Polling mode. */
#define PTX_NSC_POLLMODE_ISO            3U      /**< ISO Polling mode. */
#define PTX_NSC_POLLMODE_ACT_RF         4U      /**< Continuous wave. */
#define PTX_NSC_POLLMODE_LPCD           5U      /**< LPCD Polling mode. */
/** @} */

/**
 * \name NSC Discovery - ISO poll mode specific.
 * @{
 */
#define PTX_NSC_ISODEP_POLL_ENABLED     0       /**< Enable ISODEP. */
#define PTX_NSC_ISODEP_POLL_DISABLED    1U      /**< Disable ISODEP. */
/** @} */

/**
 * \name  NSC Discovery - NFC-DEP definitions.
 * @{
 */
#define PTX_NSC_RF_DISCOVER_NFCDEP_ATR_REQ_LEN      15U     /**< Max. length of the NFC-DEP Attribute Request Command (ATR_REQ). */
#define PTX_NSC_RF_DISCOVER_NFCDEP_ATR_RES_LEN      19U     /**< Max. length of the NFC-DEP Attribute Request Response (ATR_RES). */
/** @} */

/**
 * \name Standby and wake-up parameter masks.
 * @{
 */
#define PTX_NSC_POLL_STDBYWU_MASK_WU_GPIO_ENABLE    0x0400  /**< Enable Wake-Up by GPIO. */
#define PTX_NSC_POLL_STDBYWU_MASK_WU_HIF_ENABLE     0x0100  /**< Enable Wake-Up by Host Interface. */
#define PTX_NSC_POLL_STDBYWU_MASK_WU_LPFD_ENABLE    0x0002  /**< Enable Wake-Up by Low Power Field Detection. */
#define PTX_NSC_POLL_STDBYWU_MASK_WU_TIMER_ENABLE   0x0001  /**< Enable Wake-Up by Timer. */
/** @} */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */


/**
 * \brief NSC Discovery parameter: NFC-A/B Polling Frequency.
 */
typedef enum ptx_NSC_PollFreq
{
    NSC_PollFreq_Disabled =     0,      /**< Do not poll for selected technology. */
    NSC_PollFreq_EveryCycle =   1,      /**< Poll for selected technology on every cycle. */
    NSC_PollFreq_CycleMin =     2,      /**< Poll for selected technology every nth cycle, minimum 2. */
    NSC_PollFreq_CycleMax =     10,     /**< Poll for selected technology every nth cycle, maximum 10. */
    NSC_PollFreq_Max                    /**< Poll Frequency Max. */
} ptx_NSC_PollFreq_t;

/**
 * \brief NSC Discovery parameter: NFC-A/B/F Bail out.
 */
typedef enum ptx_NSC_PollBailOut
{
    NSC_PollBailOut_Disabled =      0,       /**< No Bail out when NFC-A/B or F technology is found. */
    NSC_PollBailOut_Enabled =       1,       /**< Bail out when NFC-A/B or F technology is found. */
    NSC_PollBailOut_Max                      /**< NFC-A/B/F bail out max. */
} ptx_NSC_PollBailOut_t;

/**
 * \brief NSC Discovery parameter: NFC-A/B/V Stop on collision.
 */
typedef enum ptx_NSC_PollStopOnCol
{
    NSC_PollStopOnCol_Disabled =    0,      /**< Continue the resolution process in case of collision. */
    NSC_PollStopOnCol_Enabled =     1,      /**< Stop the resolution process in case of a collision. */
    NSC_PollStopOnCol_Max                   /**< Safeguard. */
} ptx_NSC_PollStopOnCol_t;

/**
 * \brief NSC Discovery NFC-A parameter: Anti-collision process enabler.
 */
typedef enum ptx_NSC_PollA_AntiCol
{
    NSC_PollA_AntiCol_Disabled =    0,      /**< Do not run the resolution process. */
    NSC_PollA_AntiCol_Enabled =     1,      /**< Run the resolution process. */
    NSC_PollA_AntiCol_Max                   /**< Safeguard. */
} ptx_NSC_PollA_AntiCol_t;

/**
 * \brief NSC Discovery POLL parameter: Enable/Disable DDPC.
 * DDPC (Digital Dynamic Power Control).
 */
typedef enum ptx_NSC_PollDDPC
{
    NSC_PollDDPC_Disabled =         0U,     /**< DDPC Disabled. */
    NSC_PollDDPC_Enabled =          1U,     /**< DDPC Enabled. */
    NSC_PollDDPC_Max                        /**< DDPC Max. */
} ptx_NSC_PollDDPC_t;

/**
 * \brief NSC Discovery POLL parameter: LPCD (Low Power Card Detection).
 */
typedef enum ptx_NSC_PollLPCD
{
    NSC_PollLPCD_Disabled =         0U,     /**< LPCD Disabled. */
    NSC_PollLPCD_Enabled =          1U,     /**< LPCD Enables. */
    NSC_PollLPCD_HybridMin =        2U,     /**< LPCD Hybrid Mode, regular RF Disc used every nth cycle, LPCD used in the others, minimum Value. */
    NSC_PollLPCD_HybridMax =        255U,   /**< LPCD Hybrid Mode, regular RF Disc used every nth cycle, LPCD used in the others, maximum Value. */
    NSC_PollLPCD_Max                        /**< Safeguard. */
} ptx_NSC_PollLPCD_t;

/**
 * \brief NSC Enable/Disable ISO-DEP automatic activation.
 */
typedef enum ptx_NSC_PollIsoDep
{
    NSC_PollIsoDep_Disabled     =   0x00,   /**< ISO-DEP Disabled. */
    NSC_PollIsoDep_NfcA_Enabled =   0x01,   /**< NFC-A ISO-DEP Enabled. */
    NSC_PollIsoDep_NfcB_Enabled =   0x02,   /**< NFC-B ISO-DEP Enabled. */
    NSC_PollIsoDep_Max                      /**< Safeguard. */
} ptx_NSC_PollIsoDep_t;

/**
 * \brief NSC Enable/Disable ISO-DEP HBR.
 * Enable/Disable Higher bit-rates.
 */
typedef enum ptx_NSC_PollIsoDepHBR
{
    NSC_PollIsoDepHBR_Disabled  =   0,      /**< Higher bit-rates disabled. Standard bit-rate 106kbps shall be used. */
    NSC_PollIsoDepHBR_212       =   1,      /**< Higher bit-rates, up to 212kbps may be used */
    NSC_PollIsoDepHBR_424       =   2,      /**< Higher bit-rates, up to 424kbps may be used */
    NSC_PollIsoDepHBR_848       =   3,      /**< Higher bit-rates, up to 848kbps may be used */
    NSC_PollIsoDepHBR_Max                   /**< Safeguard */
} ptx_NSC_PollIsoDepHBR_t;

/**
 * \brief NSC Enable/Disable NFC-DEP automatic activation.
 */
typedef enum ptx_NSC_PollNfcDep
{
    NSC_PollNfcDep_Disabled =       0x00,   /**< NFC-DEP Disabled. */
    NSC_PollNfcDep_NfcA_Enabled =   0x01,   /**< NFC-A NFC-DEP Enabled. */
    NSC_PollNfcDep_NfcF_Enabled =   0x02,   /**< NFC-F NFC-DEP Enabled. */
    NSC_PollNfcDep_Max                      /**< NFC-DEP Max. */
} ptx_NSC_PollNfcDep_t;

/**
 * \brief NSC Enable/Disable NFC-DEP HBR.
 * Enable/Disable Higher bit-rates.
 */
typedef enum ptx_NSC_PollNfcDepHBR
{
    NSC_PollNfcDepHBR_Disabled  =   0,      /**< Higher bit-rates disabled. Activation bit-rate shall be used. */
    NSC_PollNfcDepHBR_212       =   1,      /**< Higher bit-rates, up to 212kbps may be used */
    NSC_PollNfcDepHBR_424       =   2,      /**< Higher bit-rates, up to 424kbps may be used */
    NSC_PollNfcDepHBR_Max                   /**< Safeguard */
} ptx_NSC_PollNfcDepHBR_t;

/**
 * \brief NSC Enable/Disable Listen mode.
 */
typedef enum ptx_NSC_Listen
{
    NSC_Listen_Disabled =           0,      /**< Listen mode disabled. */
    NSC_Listen_Enabled =            1,      /**< Listen mode enabled. */
    NSC_Listen_Enabled_RfNtf =      3,      /**< Listen Mode enabled, with Rf-Field NTFs. */
    NSC_Listen_Max                          /**< Listen mode Max. */
} ptx_NSC_Listen_t;

/**
 * \brief NSC Enable/Disable NFC-A Listen mode.
 */
typedef enum ptx_NSC_ListenA
{
    NSC_ListenA_Disabled =          0,      /**< NFC-A Listen mode disabled. */
    NSC_ListenA_Enabled =           1,      /**< NFC-A Listen mode enabled. */
    NSC_ListenA_Max                         /**< NFC-A Listen mode Max. */
} ptx_NSC_ListenA_t;

/**
 * \brief NSC Enable/Disable NFC-B Listen mode.
 */
typedef enum ptx_NSC_ListenB
{
    NSC_ListenB_Disabled =          0,      /**< NFC-B Listen mode disabled. */
    NSC_ListenB_Enabled =           1,      /**< NFC-B Listen mode enabled. */
    NSC_ListenB_Enabled_RandomId =  3,      /**< NFC-B Listen mode Max. */
    NSC_ListenB_Max                         /**< NFC-B Listen mode Max. */
} ptx_NSC_ListenB_t;

/**
 * \brief NSC Enable/Disable NFC-F (212 kbps) Listen mode.
 */
typedef enum ptx_NSC_ListenF212
{
    NSC_ListenF212_Disabled =       0,      /**< NFC-F 212 kbps Listen mode disabled. */
    NSC_ListenF212_Enabled =        1,      /**< NFC-F 212 kbps Listen mode enabled. */
    NSC_ListenF212_Max                      /**< NFC-F 212 kbps Listen mode Max. */
} ptx_NSC_ListenF212_t;

/**
 * \brief NSC Enable/Disable NFC-F (424 kbps) Listen mode.
 */
typedef enum ptx_NSC_ListenF424
{
    NSC_ListenF424_Disabled =       0,      /**< NFC-F 424 kbps Listen mode disabled. */
    NSC_ListenF424_Enabled =        1,      /**< NFC-F 424 kbps Listen mode enabled. */
    NSC_ListenF424_Max                      /**< NFC-F 424 kbps Listen mode Max. */
} ptx_NSC_ListenF424_t;

/**
 * \brief NSC NFC-F Listen mode: enable NSC device to handle SENSF_REQ command.
 */
typedef enum ptx_NSC_ListenFHandleSenRq
{
    NSC_ListenF_Handler_Host =      0,      /**< HOST is responsible to handle the command. */
    NSC_ListenF_Handler_NscDevice = 1,      /**< NSC Device is responsible to handle the command. */
} ptx_NSC_ListenFHandleSenRq_t;

/**
 * \brief NSC NFC-F Listen mode: inclusion of RD bytes in SENSF_RES.
 */
typedef enum ptx_NSC_ListenFIncludeRD
{
    NSC_ListenF_DoNotIncludeRD =    0,      /**< NSC Device shall not include RD bytes in SENSF_RES. */
    NSC_ListenF_IncludeRD =         1,      /**< NSC Device shall include RD bytes in SENSF_RES. */
} ptx_NSC_ListenFIncludeRD_t;

/**
 * \brief NSC Enable/Disable ISO-DEP Listen mode.
 */
typedef enum ptx_NSC_ListenIsoDep
{
    NSC_ListenIsoDep_Disabled =     0,      /**< ISO-DEP Listen mode disabled. */
    NSC_ListenIsoDep_T4AT_Enabled = 1,      /**< ISO-DEP T4AT Listen mode enabled. */
    NSC_ListenIsoDep_T4BT_Enabled = 2,      /**< ISO-DEP T4BT Listen mode enabled. */
    NSC_ListenIsoDep_Both_Enabled = 3,      /**< ISO-DEP Listen mode both enabled. */
    NSC_ListenIsoDep_Max                    /**< ISO-DEP Listen mode Max. */
} ptx_NSC_ListenIsoDep_t;

/**
 * \brief NSC Enable/Disable NFC-DEP Listen mode.
 */
typedef enum ptx_NSC_ListenNfcDep
{
    NSC_ListenNfcDep_Disabled =     0,      /**< NFC-DEP Listen mode disabled. */
    NSC_ListenNfcDep_NFCA_Enabled = 1,      /**< NFC-DEP NFC-A Listen mode enabled. */
    NSC_ListenNfcDep_NFCF_Enabled = 2,      /**< NFC-DEP NFC-F Listen mode enabled. */
    NSC_ListenNfcDep_Both_Enabled = 3,      /**< NFC-DEP Listen mode both enabled. */
    NSC_ListenNfcDep_Max                    /**< NFC-DEP Listen mode Max. */
} ptx_NSC_ListenNfcDep_t;

/**
 * \brief Rf-Discovery Parameter Parser Function type definition.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the polling parameter.
 * \param[in,out]   nextState           Pointer to next state, if setting a new state is applicable.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_RfDiscover_Param_Parser_t)(uint8_t** bufferPos, uint32_t* availableLen, ptx_NSC_RfDiscover_Param_Value_t* param, ptx_NSC_State_t *nextState);

/**
 * \brief Structure used to map parameter type to
 * parameter parser.
 */
typedef struct ptx_NSC_CmdDiscover_ParserApi
{
    ptx_NSC_RfDiscover_Param_Type_t   Type;     /**< RF discover param type id */
    ptx_NSC_RfDiscover_Param_Parser_t Parser;   /**< RF discover parser function */
} ptx_NSC_RfDiscover_ParserApi_t;

/**
 * \brief Custom Type-F Polling Command Parameters.
 */
typedef struct ptx_NSC_CmdDiscover_TypeFPollCmdParams
{
    uint8_t     OverwriteDefaultParams;			                    /**< Indicator to overwrite default command parameters. */
    uint8_t     SystemCode[PTX_NSC_TYPES_SENSFREQ_SYS_CODE_LEN];	/**< 16-Bit System Code according to FeliCa/T3T. */
    uint8_t     RequestCode;					                    /**< 8-Bit Requested Code according to FeliCa/T3T. */
    uint8_t     InitialTimeSlotNr;				                    /**< 8-Bit Timeslot-Number according to FeliCa/T3T - used only during initial Polling. */
} ptx_NSC_CmdDiscover_TypeFPollCmdParams_t;

/**
 * \brief Generic configuration parameters for the RF Discovery process.
 */
typedef struct ptx_NSC_RfDiscovery_Cfg
{
    /**
     * RF-Discovery Poll Mode.
     * 0 ... Poll Mode Disabled.
     * 1 ... NFC Forum poll mode.
     * 2 ... EMVCo poll mode.
     * 3 ... ISO poll mode.
     * 4 ... Activate RF field and move to RF_POLL state.
     * 5 ... LPCD poll mode.
     * 6 ... Proprietary Type-F poll mode (RFU).
     */
    uint8_t     PollMode;

    /**
      * Guard Time.
      * Additional guard time in milliseconds after the RF field has been
      * activated and before the first RF command is sent.
      * Range: 0 - 255
      */
    uint8_t     GuardTime;

    /**
     * Enable DDPC (Digital Dynamic Power Control).
     * 0 ... DDPC disabled
     * 1 ... DDPC enabled
     */
    uint8_t     EnableDDPC;

    /**
     * Enable LPCD (Low Power Card Detection).
     * 0 ... LPCD is disabled (regular RF discovery cycle).
     * 1 ... LPCD is used in every discovery cycle.
     * 2-255 ...Hybrid mode where the regular RF discovery
     *          cycle is used in every (2-255)th discovery cycle, and LPCD
     *          is used in other cycles.
    */
    uint8_t     EnableLPCD;

    /**
     * Enable/disable LPCD notifications.
     * 0 ... LPCD notifications disabled
     * 1 ... LPCD notifications enabled
     */
    uint8_t     EnableLPCDNtf;

    /**
     * Enable/disable Error notifications during discovery process.
     * 0 ... Error notifications disabled
     * 1 ... Error notifications enabled
     */
    uint8_t     EnableErrorNtf;

    /**
     * Idle time.
     * Idle time between two consecutive discovery cycles in 32
     * microsecond units. (3 Bytes)
     */
    uint32_t    IdleTime;

    /**
     * Maximum number of the devices to be resolved during NFC poll mode discovery procedure.
     * The parameter has precedence over the technology specific devices limit parameters.
     * 0 ... No limit is set
     */
    uint8_t     PollDevicesLimit;

    /**
     * Enable/disable internal RF-Amplitude Limiter (RFU).
     * 0 ... Internal RF-Limiter disabled
     * 1 ... Internal RF-Limiter enabled
     */
    uint8_t     PollRFAmplitudeLimiterEnable;

    /**
     * Initial poll technology for the NFC Forum poll mode.
     * 1 ... NFC-A
     * 2 ... NFC-B
     * 3 ... NFC-F
     * 4 ... NFC-V
     */
    uint8_t     InitPollTech;

    /**
     * POLL-ACM Polling Frequency.
     * 0 ... Do not poll for NFC-ACM technology.
     * 1 ... Poll for  NFC-ACM technology in every discovery cycle.
     * 2-10 ... Poll for  NFC-ACM technology in every (2-10)th discovery cycle.
     */
    uint8_t     PollTypeAcmPollFrequency;

    /**
     * POLL-ACM Bit-rate.
     * 0 ... 106kbps (NFC-A).
     * 1 ... 212kbps (NFC-F).
     * 2 ... 424kbps (NFC-F).
     */
    uint8_t     PollTypeAcmBitrate;

    /**
     * Type A Polling Frequency.
     * 0 ... Do not poll for Type A technology.
     * 1 ... Poll for Type A technology in every discovery cycle.
     * 2-10 ... Poll for Type A technology in every (2-10)th
     *          discovery cycle.
     */
    uint8_t     PollTypeAPollFrequency;

    /**
     * Max. Type A devices to detect.
     *
     * If set to 0 the pre-configured number of devices will be taken. The pre-configured number of devices
     * is defined by the poll mode \see PollMode.
     */
    uint8_t     PollTypeADeviceLimit;

    /**
     *  Bail out after NFC-A technology is found
     *
     *  0 = No Bail out when NFC-A technology is found
     *  1 = Bail out when NFC-A technology is found
     *  other values are interpreted as 0, no bail out
     */
    uint8_t     PollTypeABailOut;

    /**
     * NFC-B Polling Frequency.
     * 0 ... Do not poll for NFC-B technology.
     * 1 ... Poll for NFC-B technology in every discovery cycle.
     * 2-10 ... Poll for NFC-B technology in every (2-10)th
     *          discovery cycle.
     */
    uint8_t     PollTypeBPollFrequency;

    /**
     * Max. Type B devices to detect.
     *
     * If set to 0 the pre-configured number of devices will be taken. The pre-configured number of devices
     * is defined by the poll mode \see PollMode.
     */
    uint8_t     PollTypeBDeviceLimit;

    /**
     *  Bail out after NFC-A, or NFC-B technology is found
     *
     *  0 = No Bail out when NFC-A, or NFC-B technology is found
     *  1 = Bail out when NFC-A, or NFC-B technology is found
     *  other values are interpreted as 0, no bail out
     */
    uint8_t     PollTypeBBailOut;

    /** RF-Discovery Loop. Enable Type F technology. 212kbps */
    uint8_t     PollTypeF212;

    /** RF-Discovery Loop. Enable Type F technology. 424kbps */
    uint8_t     PollTypeF424;

    /** RF-Discovery Loop. Type F technology. Custom Command Parameters. */
    ptx_NSC_CmdDiscover_TypeFPollCmdParams_t PollTypeFCmdParams;

    /**
     * NFC-F Polling Frequency.
     * 0 ... Do not poll for NFC-F technology.
     * 1 ... Poll for NFC-F technology in every discovery cycle.
     * 2-10 ... Poll for NFC-F technology in every (2-10)th
     *          discovery cycle.
     */
    uint8_t     PollTypeFPollFrequency;

    /**
     * Max. Type F devices to detect.
     *
     * If set to 0 the pre-configured number of devices will be taken. The pre-configured number of devices
     * is defined by the poll mode \see PollMode.
     */
    uint8_t     PollTypeFDeviceLimit;

    /**
     * RF-Field Reset before transmission of SENSF_REQ.
     * 
     * Note: This Feature is currently RFU.
     *
     *  0 = No RF-Reset is performed before next SENSF_REQ (default / standard behavior )
     *  1 = RF-Reset is performed before next SENSF_REQ
     *  other values are interpreted as 1
     */
    uint8_t     PollTypeFFieldReset;

    /**
     *  Bail out after NFC-A, or NFC-B, or NFC-F technology is found
     *
     *  0 = No Bail out when NFC-A, NFC-B, or NFC-F technology is found
     *  1 = Bail out when NFC-A, NFC-B, or NFC-F technology is found
     *  other values are interpreted as 0, no bail out
     */
    uint8_t     PollTypeFBailOut;

    /**
     * Max. Type V devices to detect.
     *
     * If set to 0 the pre-configured number of devices will be taken. The pre-configured number of devices
     * is defined by the poll mode \see PollMode.
     */
    uint8_t     PollTypeVDeviceLimit;

    /**
     * NFC-V Polling Frequency.
     * 0 ... Do not poll for NFC-V technology.
     * 1 ... Poll for NFC-V technology in every discovery cycle.
     * 2-10 ... Poll for NFC-V technology in every (2-10)th
     *          discovery cycle.
     */
    uint8_t     PollTypeVPollFrequency;

    /**
     * Proprietary NFC-A Technology Polling Frequency.
     * 0 ... Do not poll for NFC-PA technology.
     * 1 ... Poll for NFC-PA technology in every discovery cycle.
     * 2-10 ... Poll for NFC-PA technology in every (2-10)th discovery cycle.
     */
    uint8_t     PollTypePAPollFrequency;

    /**
     * Enable/disable Poll ISO-DEP protocol automatic activation.
     * 0 ... Disable
     * 1 ... NFC-A ISO-DEP enable (T4AT)
     * 2 ... NFC-B ISO-DEP enable (T4BT)
     */
    uint8_t     PollIsoDepEnable;

    /**
     * Enable ISO-DEP HBR.
     * 0 ... Standard bit-rates (106kbps) shall be used
     * 1 ... Higher bit-rates, up to 212kbps may be used
     * 2 ... Higher bit-rates, up to 424kbps may be used
     * 3 ... Higher bit-rates, up to 828kbps may be used
     */
    uint8_t     PollIsoDepHbr;

    /**
     * Enable/disable Poll NFC-DEP protocol automatic activation.
     * 0 ... Disable
     * 1 ... NFC-A NFC-DEP enable
     * 2 ... NFC-F NFC-DEP enable
     */
    uint8_t     PollNfcDepEnable;

    /**
     * Enable NFC-DEP HBR.
     * 0 ... Activation bit-rates shall be used
     * 1 ... Higher bit-rates, up to 212kbps may be used
     * 2 ... Higher bit-rates, up to 424kbps may be used
     */
    uint8_t     PollNfcDepHbr;

    /**
     * NFC-DEP Attribute Request Command (ATR_REQ) used by the Initiator to activate a Target.
     */
    uint8_t     PollNfcDepAtrReq[PTX_NSC_RF_DISCOVER_NFCDEP_ATR_REQ_LEN];
    /**
     * Length of the Attribute Request Command (ATR_REQ).
     */
    uint8_t     PollNfcDepAtrReqLen;

    /**
     * Standby and wake-up configuration.
     * Bit(s) description:
     * [15] STANDBY mode enable.
     * [13:12] - WU by AUX-IF enable.
     * [10] - WU by GPIO enable.
     * [9] - WU by I3C CCC enable.
     * [8] - WU by host interface enable.
     * [1] - WU by LPFD (Low Power Field Detect) enable.
     * [0] - WU by timer enable.
     * others - RFU
     */
    uint16_t    StdbyWuCfg;

    /**
     * Activate Listen Mode.
     * 0 ... Deactivated.
     * 1 ... Activated.
     * 3 ... Activated, Rf-Field NTFs enabled.
     */
    uint8_t     ListenMode;

#if !defined(PTX_DISABLE_LISTEN_MODE)
    /**
     * Activate Listen NFC-A.
     * 0 ... Deactivated.
     * 1 ... Activated.
     */
    uint8_t     ListenTypeA;

    /**
     * NFCID Length for Listen NFC-A.
     *  0 ... Use a default NFCID
     *  4 ... NFCID is 4 bytes long
     *  7 ... NFCID is 7 bytes long
     * 10 ... NFCID is 10 bytes long
     *
     * The variable describes how many bytes in #ListenTypeANfcId describe the NFCID.
     */
    uint8_t     ListenTypeANfcIdLen;

    /**
     * Custom NFCID for Listen NFC-A.
     *
     * The NFCID must be either 4/7/10 bytes long.
     * The length of the bytes describing the NFCID is given in #ListenTypeANfcIdLen.
     */
    uint8_t     ListenTypeANfcId[PTX_NSC_RF_DISCOVER_NFCID1_MAX_LEN];

    /**
     * SENS_RES Length.
     *
     * 0 ... No custom SENS_RES is provided
     * 2 ... A custom SENS_RES is provided in #ListenTypeASensRes
     */
    uint8_t     ListenTypeASensResLen;

    /**
     * Provide SENS_RES parameters.
     */
    uint8_t     ListenTypeASensRes[PTX_NSC_RF_DISCOVER_SENSRES_LEN];

    /**
     * Activate Listen NFC-B.
     * 0 ... Deactivated.
     * 1 ... Activated.
     * 3 ... Activated, random NFCID0 enabled.
     */
    uint8_t     ListenTypeB;

    /**
     * NFCID Length for Listen NFC-B.
     *
     * 0 ... No NFCID is provided.
     * 4 ... NFCID is provided with a length of 4 bytes.
     */
    uint8_t     ListenTypeBNfcIdLen;

    /**
     * NFCID for Listen NFC-B.
     *
     * Must contain 4 bytes!
     */
    uint8_t     ListenTypeBNfcId[PTX_NSC_RF_DISCOVER_NFCID0_LEN];


    /**
     * AppData Length for Listen NFC-B.
     *
     * 0 ... No AppData is provided.
     * 4 ... AppData is provided with a length of 4 bytes.
     */
    uint8_t     ListenTypeBAppDataLen;

    /**
     * AppData for Listen NFC-B.
     */
    uint8_t     ListenTypeBAppData[PTX_NSC_RF_DISCOVER_APPDATA_LEN];

    /**
     * Protocol Information Length for Listen NFC-B.
     *
     * 0 ... No Protocol Information is provided.
     * 3 ... Protocol Information is provided with a length of 3 bytes.
     */
    uint8_t     ListenTypeBProtInfoLen;

    /**
     * Protocol Information for Listen NFC-B.
     */
    uint8_t     ListenTypeBProtInfo[PTX_NSC_RF_DISCOVER_PROTOCOLINFO_LEN];


    /**
     * Activate Listen NFC-F, 212 kbps.
     * 0 ... Deactivated.
     * 1 ... Activate.
     */
    uint8_t     ListenTypeF212;

    /**
     * Activate Listen NFC-F, 424 kbps.
     * 0 ... Deactivated.
     * 1 ... Activate.
     */
    uint8_t     ListenTypeF424;

    /**
     * Set NFC-F SENSF_REQ handler:
     * 0 ... Host.
     * 1 ... NSC Device.
     */
    uint8_t     ListenTypeFSenRqHandler;

    /**
     * NFC-F SENSF_RES RD bytes inclusion:
     * 0 ... NSC Device shall not include RD bytes in SENSF_RES.
     * 1 ... NSC Device shall include RD bytes in SENSF_RES.
     */
    uint8_t     ListenTypeFIncludeRD;

    /**
     * NFCID Length for Listen NFC-F.
     *
     * 0 ... No NfcId is provided.
     * 8 ... NFCID for Listen NFC-F is provided.
     */
    uint8_t     ListenTypeFNfcIdLen;

    /**
     * NFCID for Listen NFC-F.
     */
    uint8_t     ListenTypeFNfcId[PTX_NSC_RF_DISCOVER_NFCID2_LEN];

    /**
     * Request Data Length for Listen NFC-F.
     *
     * 0 ... No Request Data provided.
     * 2 ... Request Data for Listen NFC-F is provided.
     */
    uint8_t     ListenTypeFRequestDataLen;

    /**
     * Request Data for Listen NFC-F.
     */
    uint8_t     ListenTypeFRequestData[PTX_NSC_RF_DISCOVER_RD_LEN];

    /**
     * Activate Listen ISO-DEP.
     * 0 ... Deactivated.
     * 1 ... Activated - T4AT.
     * 2 ... Activated - T4BT.
     * 3 ... Activated - both T4AT and T4BT.
     */
    uint8_t     ListenIsoDep;

    /**
     * ATS Length for ISO-DEP
     *
     *    0 ... No ATS provided
     * < 21 ... Number of bytes in #ListenIsoDepATS to set
     */
    uint8_t     ListenIsoDepATSLen;

    /**
     * ATS bytes for ISO-DEP
     */
    uint8_t     ListenIsoDepATS[PTX_NSC_RF_DISCOVER_ATS_LEN];

    /**
     * ATTRIB RES Length for ISO-DEP.
     *
     *    0 ... No ATTRIB_RES provided.
     * < 14 ... Number of bytes in #ListenIsoDepAttribRes
     */
    uint8_t     ListenIsoDepAttribResLen;

    /**
     * ATTRIB RES for ISO-DEP
     */
    uint8_t     ListenIsoDepAttribRes[PTX_NSC_RF_DISCOVER_ATTRIBRES_LEN];

    /**
     * Activate Listen NFC-DEP.
     * 0 ... Deactivated.
     * 1 ... Activated - NFC-A.
     * 2 ... Activated - NFC-F.
     * 3 ... Activated - both NFC-A and NFC-F.
     */
    uint8_t     ListenNfcDep;

    /**
     * Length of the Attribute Request Response (ATR_RES).
     */
    uint8_t     ListenNfcDepAtrResLen;

    /**
     * NFC-DEP Attribute Request Response (ATR_RES) sent by the Target.
     */
    uint8_t     ListenNfcDepAtrRes[PTX_NSC_RF_DISCOVER_ATR_RES_LEN];
#endif /* PTX_DISABLE_LISTEN_MODE */

} ptx_NSC_RfDiscovery_Cfg_t;

/** @} */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief This function builds the NSC_RF_DISCOVER_CMD according to the provided parameters and transmits it to the PTX2xxR/W.
 *
 * \param[in]   nsc                 Pointer to the component structure.
 * \param[in]   discoverParams      Pointer to the parameter list. Can be NULL - in such scenario, the last given parameter
 *                                  configuration shall be used to build the command.
 * \param[in]   paramCnt            The number of parameters in the list. Can be 0.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_RfDiscover(ptx_NSC_t *nsc, ptx_NSC_RfDiscover_Param_t *discoverParams, uint32_t paramCnt);

/**
 * \brief This function parses the NSC_DISCOVER_NTF and stores it in the corresponding structure.
 *
 * \param[in]   buffer              Pointer to buffer, containing the data to be interpreted.
 * \param[in]   bufferLen           Length of the data to be interpreted.
 * \param[in]   ntf                 Pointer to structure for storing the interpreted notification data.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_ProcessRfDiscoverNtf(uint8_t* buffer, uint32_t bufferLen, ptx_NSC_RfDiscover_Ntf_t* ntf);

/**
 * \brief This function gets an RF Discovery parameter value depending on the requested configuration, and fills
 * the output parameter structure correspondingly.
 *
 * \param[in]   nsc                 Pointer to the component structure.
 * \param[in]   type                Requested parameter type.
 * \param[in]   cfg                 Requested RF Discovery configuration.
 * \param[out]  params              Pointer to the structure holding returned parameter data.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_GetRfDiscoveryParam(ptx_NSC_t *nsc, ptx_NSC_RfDiscover_Param_Type_t type, ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);

/**
 * \brief This function gets an RF Discovery parameter value for Poll Mode.
 *
 * \param[in]   nsc                 Pointer to the component structure.
 * \param[in]   type                Requested parameter type.
 * \param[in]   cfg                 Requested RF Discovery configuration.
 * \param[out]  params              Pointer to the structure holding returned parameter data.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_GetRfDiscoveryParam_Poll(ptx_NSC_t *nsc, ptx_NSC_RfDiscover_Param_Type_t type, ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);

/**
 * \brief This function gets an RF Discovery parameter value for Listen Mode.
 *
 * \param[in]   nsc                 Pointer to the component structure.
 * \param[in]   type                Requested parameter type.
 * \param[in]   cfg                 Requested RF Discovery configuration.
 * \param[out]  params              Pointer to the structure holding returned parameter data.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_GetRfDiscoveryParam_Listen(ptx_NSC_t *nsc, ptx_NSC_RfDiscover_Param_Type_t type, ptx_NSC_RfDiscovery_Cfg_t *cfg, ptx_NSC_RfDiscover_Param_t *params);


/*
 * ####################################################################################################################
 * INTERNAL NSC API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Builds the corresponding transmit buffer from the given polling parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the polling parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_Poll(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given POLL-ACM polling parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the polling parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_PollAcm(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given NFC-A polling parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the polling parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_PollA(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given NFC-B polling parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the polling parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_PollB(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given NFC-F polling parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the polling parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_PollF(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given NFC-V polling parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the polling parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_PollV(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given ISO-DEP polling parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the polling parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_PollIsoDep(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given NFC-DEP polling parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the polling parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_PollNfcDep(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given listening parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the listening parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_Listen(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given NFC-DEP polling parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the polling parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_ListenA(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given NFC-A listening parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the listening parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_ListenB(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given NFC-B listening parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the listening parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_ListenF(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given ISO-DEP listening parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the polling parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was listening. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_ListenIsoDep(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);

/**
 * \brief Builds the corresponding transmit buffer from the given NFC-DEP listening parameter.
 *
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 * \param[in]       param               Pointer to the listening parameter.
 * \param[in]       nextState           Pointer to the next state.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_ParamParser_ListenNfcDep(uint8_t **bufferPos, uint32_t *availableLen, ptx_NSC_RfDiscover_Param_Value_t *param, ptx_NSC_State_t *nextState);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
