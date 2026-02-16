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
    Module      : NFC Soft Controller (NSC) Main Interface Header
    File        : ptx_NSC_Int.h

    Description : Definition of NFC Soft Controller Main Interface.
                  Contains type definition of the main NSC component structure and other
                  data structures specific for NSC.
*/

/**
 * \addtogroup NSC_COMPONENT
 *
 * @{
 */

#if !defined(PTX_NSC_INT_H)
#define PTX_NSC_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NSC_RfConfig.h"
#include "ptx_NSC_SystemConfig.h"
#include "ptx_NSC_CmdProcessor.h"
#include "ptx_Components.h"
#include <string.h>

#if !defined(PTX_DISABLE_LISTEN_MODE)
    #include "ptx_NSC_RfSetRoutingTable_Int.h"
#endif


#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
/**
 * \name Length and timing definitions.
 * @{
 */
#define PTX_NSC_TIMEOUT_DEFAULT                     (5000u)     /**< Default timeout in milliseconds for NSC responses. */

#define PTX_NSC_TYPES_ATTRIBINF_LEN                 (15U)       /**< NSC Type ATTRIBINF length */
#define PTX_NSC_TYPES_ATRREQGEN_LEN                 (15U)       /**< NSC Type ATR REQ GEN length */

#define PTX_NSC_TYPES_LISASENSRES_LEN               (2U)        /**< NSC Type listen A SENSRES length */
#define PTX_NSC_TYPES_LISANFCID1_LEN                (10U)       /**< NSC Type listen A NFCID1 max. length */
#define PTX_NSC_TYPES_SENSBRES_LEN                  (13U)       /**< NSC Type B SENSRES length */
#define PTX_NSC_TYPES_POLLFNFCID2_LEN               (8U)        /**< NSC Type F NFCID2 length */
#define PTX_NSC_TYPES_SENSFRES_LEN                  (20U)       /**< NSC Type F SENSRRES length */
#define PTX_NSC_TYPES_SENSFRES_LEN_OFFSET           (0U)        /**< NSC Type F SENSRRES length offset */
#define PTX_NSC_TYPES_SENSFREQ_SYS_CODE_LEN         (2U)        /**< NSC Type F SENSRREQ System Code length */
#define PTX_NSC_TYPES_INVENTORYRES_LEN              (10U)       /**< NSC Type V INVENTORYRES length */

#define PTX_NSC_FWI_UNSET                           (0xFF)      /**< Default value for FWI in NSC component in case it is not set by card activation */
#define PTX_NSC_FWI_MICROSECOND_STEP_SIZE           (302u)      /**< FWI step size in microseconds */
#define PTX_NSC_FWI_T4T_DEFAULT                     (49000u)    /**< Default FWI value to be added on to calculated FWI value */
#define PTX_NSC_MAX_TRANSFER_UNIT_SIZE              255u        /**< Max. Message transfer Unit size */
#define PTX_NSC_DEFAULT_NUMOF_RESIDUAL_BITS         0u          /**< Residual bits for T2T, etc. */
/** @} */



/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */
/**
 * \brief HAL Forward declaration.
 */
typedef struct ptx_HAL ptx_HAL_t;

/**
 * \brief HIP Forward declaration.
 */
typedef struct ptx_HIP ptx_HIP_t;


/**
 * \brief Internal NSC States.
 */
typedef enum ptx_NSC_State
{
    ptx_NSC_State_Off,                  /**< State OFF, System is turned off. */
    ptx_NSC_State_Init,                 /**< State INIT, Entered after Reset. */
    ptx_NSC_State_Idle,                 /**< State RF_IDLE, System is on, no field. */
    ptx_NSC_State_Discovery,            /**< State RF_DISCOVERY, Field is on and discovery is running. */
    ptx_NSC_State_Poll,                 /**< State RF_POLL, Poll Mode is activated. */
    ptx_NSC_State_Poll_Dep,             /**< State RF_POLL_DEP, Poll Mode is activated (ISO-DEP, NFC-DEP). */
    ptx_NSC_State_Poll_Test,            /**< State RF_POLL_TEST, Only special test sequences are allowed. */
    ptx_NSC_State_Listen,               /**< State RF_LISTEN, Listen Mode state entered after T2T or T3T activation. */
    ptx_NSC_State_Listen_Sleep,         /**< State RF_LISTEN_SLEEP, Listen Mode state entered after de-select or de-activation to sleep. */
    ptx_NSC_State_Listen_Idle,          /**< State RF_LISTEN_IDLE, Listen Mode state entered after field-ON or adequate de-activate actions. */
    ptx_NSC_State_Listen_Dep,           /**< State RF_LISTEN_DEP, Listen Mode state entered after Iso-Dep or Nfc-Dep activation. */
    ptx_NSC_State_Wpt,                  /**< State WLC_WPT, entered from POLL state with WPT_START command. */
    ptx_NSC_State_Wpt_Dep,              /**< State WLC_WPT_DEP, entered from POLL_DEP state with WPT_START command. */
    ptx_NSC_State_Standby,              /**< State STANDBY, entered by NSC Standby command. */
    ptx_NSC_State_Error,                /**< State ERROR, entered after a critical error NTF came. */
    ptx_NSC_State_Undefined,            /**< Undefined. The number of elements. */
} ptx_NSC_State_t;

/**
 * \brief  Frame Type of a NSC Message.
 */
typedef enum ptx_NSC_FrameType
{
    ptx_NSC_FrameType_Rsp,              /**< Response Frame. */
    ptx_NSC_FrameType_Ntf,              /**< Notification Frame. */
} ptx_NSC_FrameType_t;

/**
 * \brief NSC Information Types.
 */
typedef enum ptx_NSC_InfoType
{
    ptx_NSC_Info_HW_Version,        /**< Hardware version */
    ptx_NSC_Info_C_Stack,           /**< C-Stack revision. */
    ptx_NSC_Info_Local_Changes,     /**< Local changes. */
    ptx_NSC_Info_FW_Version,        /**< DFY-Code version. */
    ptx_NSC_Info_DFY_Toolchain,     /**< Tool-chain revision. */
    ptx_NSC_Info_Rf_Config,         /**< RF-Config mapping version */
    ptx_NSC_Info_Derivate,          /**< Commercial derivate. */
    ptx_NSC_Info_MtuSize,           /**< NSC MTU (max. transfer unit) size. */
    ptx_NSC_Info_Temperature,       /**< Current chip temperature. */
    ptx_NSC_Info_Platform_Revision, /**< Git hash of current version. */
    ptx_NSC_Info_MAX                /**< Max, Undefined. */
} ptx_NSC_InfoType_t;

/**
 * \brief NSC System Reset Options
 */
typedef enum ptx_NSC_Reset_Types
{
    ptx_NSC_Reset_Soft,             /**< System Reset, Invoke HIP Reset command */
    ptx_NSC_Reset_Hard,             /**< System Reset, Toggle PTX reset pin */
    ptx_NSC_Reset_MAX               /**< CMD MAX, used for comparison only */
} ptx_NSC_Reset_Types_t;

/**
 * \brief NSC PLL configuration settings
 */
typedef struct ptx_NSC_PllConfig
{
    uint8_t             XtalDiv2En;     /**< XTAL output divider enable. */
    uint16_t            GatePw;         /**< Number of reference clock periods the sys_clk counter should be enabled for during PLL calibration. */
    uint16_t            SysCntTg;       /**< Expected number of clock pulses during PLL calibration. */
    uint32_t            NHost;          /**< PLL division factor for the selected RF field frequency. */
    uint32_t            NHostAlm;       /**< PLL division factor for the ALM. */
} ptx_NSC_PllConfig_t;

/**
 * \brief Callback definition to setup the discovery datastructure.
 *
 * \param[in]       ctx                 Pointer to component.
 * \param[in,out]   bufferPos           Pointer to the buffer to write to. (Pointer is modified/incremented by the function.)
 * \param[in,out]   availableLen        Pointer to a counter storing the space still left within the buffer.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_ExtendDiscoveryParametersCb_t) (void *ctx, uint8_t **bufferPos, uint32_t *availableLen);

/**
 * \brief Main Component Structure
 */
typedef struct ptx_NSC
{
    ptx_Comps_t                             CompId;         /**< Component ID. */

    ptx_HIP_t                               *HIP;           /**< HIP Layer. */
    ptx_HAL_t                               *HAL;           /**< HW Layer. */

    ptx_NSC_State_t                         State;          /**< Internal State. */
    ptx_NSC_State_t                         LastDiscState;  /**< Internal State after the last successful NSC_RF_DISCOVER_CMD. */
    ptx_NSC_State_t                         PendingState;   /**< Internal State, RfDeactivate tells us where to go. */

    ptx_NSC_CmdProcessor_t                  CmdProcessor;  /**< Command Processor. */
    ptx_NSC_SystemConfig_t                  SysCfg;        /**< System Configuration. */
    ptx_NSC_RfConfig_t                      RfCfg;         /**< RF Configuration. */

    uint8_t                                 ResidualRxBits; /**< Residual Rx bits for T2T, etc. */
    uint8_t                                 ResidualTxBits; /**< Residual Tx bits for T2T, etc. */
    uint8_t                                 MtU;            /**< Max. Message transfer Unit size */

    /*
     * RFU
     */
    #if 0
    uint8_t                                 FWI;            /**< Current Frame Waiting time Integer */
    #endif

    void                                    *Extension;     /**< Specific parameters, used and defined by any of the variants (AIS or RUL). */

    uint32_t                                NscSpeed;       /**< NSC Application Speed, i.e. target NFC chip interface speed. */

#if !defined(PTX_DISABLE_LISTEN_MODE)
    ptx_NSC_RoutingTable_t                  RoutingTable;   /**< Routing Table Structure. */
#endif

    uint8_t                                 HwVersion;      /**< Device HW version. */
    uint16_t                                FwVersion;      /**< Device FW version. */
    uint16_t                                Derivate;       /**< Device commercial derivate. */

    uint8_t                                 PollMode;       /**< Keep track of current poll mode. */
    ptx_NSC_ExtendDiscoveryParametersCb_t   ExtendDiscoveryParametersCb;    /**< Discovery command extension callback. */
    void                                    *ExtendDiscoveryParametersCtx;  /**< Discovery command extension context. */

} ptx_NSC_t;

/**
 * \brief Main Component Initialization Parameters.
 */
typedef struct ptx_NSC_InitParams
{
    ptx_HIP_t                           *Hip;           /**< Pointer to HIP component. */
    ptx_HAL_t                           *Hal;           /**< Pointer to HAL component. */
    void                                *ExtParams;     /**< Reference to Extension specific initialization parameters. Used and defined by any of the variants (AIS or RUL). */
} ptx_NSC_InitParams_t;


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
