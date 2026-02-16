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
    Module      : NFC Soft Controller (NSC) Interface Header for State Machine
    File        : ptx_NSC_StateMachine.h

    Description : Definition of NFC Soft Controller Interface for State Machine.
                  Contains the function(s) to check if requested state transition is allowed or not.
*/

/**
 * \addtogroup NSC_STATE_MACHINE
 *
 * @{
 */

#if !defined(PTX_NSC_STATEMACHINE_H)
#define PTX_NSC_STATEMACHINE_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
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
 * \brief Forward declaration of ptx_NSC structure.
 */
struct ptx_NSC;

/**
 * \brief Transfer Nsc state to a bit-mask value.
 */
#define PTX_NSC_SM_BITMASK(x)       ((uint16_t)(0x0001 << (uint16_t)x))

/**
 * \name SM States.
 * @{
 */
#define PTX_NSC_SM_RFST_OFF             (PTX_NSC_SM_BITMASK(ptx_NSC_State_Off))             /**< Status OFF */
#define PTX_NSC_SM_RFST_INIT            (PTX_NSC_SM_BITMASK(ptx_NSC_State_Init))            /**< Status INIT */
#define PTX_NSC_SM_RFST_IDLE            (PTX_NSC_SM_BITMASK(ptx_NSC_State_Idle))            /**< Status IDLE */
#define PTX_NSC_SM_RFST_DISCOVERY       (PTX_NSC_SM_BITMASK(ptx_NSC_State_Discovery))       /**< Status DISCOVERY */
#define PTX_NSC_SM_RFST_POLL            (PTX_NSC_SM_BITMASK(ptx_NSC_State_Poll))            /**< Status POLL */
#define PTX_NSC_SM_RFST_POLL_DEP        (PTX_NSC_SM_BITMASK(ptx_NSC_State_Poll_Dep))        /**< Status POLL DEP */
#define PTX_NSC_SM_RFST_POLL_TEST       (PTX_NSC_SM_BITMASK(ptx_NSC_State_Poll_Test))       /**< Status POLL TEST */
#define PTX_NSC_SM_RFST_LISTEN          (PTX_NSC_SM_BITMASK(ptx_NSC_State_Listen))          /**< Status LISTEN */
#define PTX_NSC_SM_RFST_LISTEN_SLEEP    (PTX_NSC_SM_BITMASK(ptx_NSC_State_Listen_Sleep))    /**< Status LISTEN SLEEP */
#define PTX_NSC_SM_RFST_LISTEN_IDLE     (PTX_NSC_SM_BITMASK(ptx_NSC_State_Listen_Idle))     /**< Status LISTEN IDLE */
#define PTX_NSC_SM_RFST_LISTEN_DEP      (PTX_NSC_SM_BITMASK(ptx_NSC_State_Listen_Dep))      /**< Status LISTEN DEP */
#define PTX_NSC_SM_RFST_WPT             (PTX_NSC_SM_BITMASK(ptx_NSC_State_Wpt))             /**< Status WPT Cycle */
#define PTX_NSC_SM_RFST_WPT_DEP         (PTX_NSC_SM_BITMASK(ptx_NSC_State_Wpt_Dep))         /**< Status WPT DEP */
#define PTX_NSC_SM_RFST_STANDBY         (PTX_NSC_SM_BITMASK(ptx_NSC_State_Standby))         /**< Status STANDBY */
#define PTX_NSC_SM_RFST_ERROR           (PTX_NSC_SM_BITMASK(ptx_NSC_State_Error))           /**< Status ERROR */
/** @} */

/**
 * \brief Macro for all states except ERROR, OFF, INIT and STANDBY
 */
#define PTX_NSC_SM_RFST_ALL_IDLE    (PTX_NSC_SM_RFST_IDLE + PTX_NSC_SM_RFST_DISCOVERY + PTX_NSC_SM_RFST_POLL + \
                                     PTX_NSC_SM_RFST_POLL_DEP + PTX_NSC_SM_RFST_POLL_TEST + PTX_NSC_SM_RFST_LISTEN + PTX_NSC_SM_RFST_LISTEN_DEP + \
                                     PTX_NSC_SM_RFST_LISTEN_SLEEP + PTX_NSC_SM_RFST_LISTEN_IDLE + PTX_NSC_SM_RFST_WPT + PTX_NSC_SM_RFST_WPT_DEP)

/**
 * \brief RfDeactivate command SM parameter structure, 3By: LEN TYPE STATE. LEN byte included in total length.
 */
#define PTX_NSC_SM_DEACT_PARAMLEN       3U      /**< RfDeactivate command total parameter length. */
#define PTX_NSC_SM_DEACT_LEN_IDX        0U      /**< Offset of LEN parameter of RfDeactivate command. */
#define PTX_NSC_SM_DEACT_TYPE_IDX       1U      /**< Offset of DEACTIVATION_TYPE parameter of RfDeactivate command. */
#define PTX_NSC_SM_DEACT_STATE_IDX      2U      /**< Offset of DEACTIVATION_STATE_AFTER parameter of RfDeactivate command. */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief NSC (RF) State Machine Check Item for Commands.
 */
typedef enum ptx_NSC_SM_CmdId
{
    ptx_NSC_SM_CmdId_Init,          /**< Init command. */
    ptx_NSC_SM_CmdId_Read,          /**< Read command. */
    ptx_NSC_SM_CmdId_Write,         /**< Write command. */
    ptx_NSC_SM_CmdId_RfConfig,      /**< Rf-Config command. */
    ptx_NSC_SM_CmdId_RfDiscover,    /**< Rf-Discvover command. */
    ptx_NSC_SM_CmdId_RfDeactivate,  /**< Rf-Deactivate command. */
    ptx_NSC_SM_CmdId_RfActivate,    /**< Rf-activate command. */
    ptx_NSC_SM_CmdId_RfSetParam,    /**< Rf-SetParams command. */
    ptx_NSC_SM_CmdId_RfTest,        /**< Rf-Test command. */
    ptx_NSC_SM_CmdId_WptStart,      /**< WPT Start command. */
    ptx_NSC_SM_CmdId_WptStop,       /**< WPT Start command. */
    ptx_NSC_SM_CmdId_Data,          /**< Data message. */
    ptx_NSC_SM_CmdId_Standby,       /**< Standby command. */
    ptx_NSC_SM_CmdId_Wakeup,        /**< Wakeup command. */
    ptx_NSC_SM_CmdId_Loopback,      /**< Loopback command. */
    ptx_NSC_SM_CmdId_GetStatus,     /**< GetStatus command. */
    ptx_NSC_SM_CmdId_RfSetRoutingTable, /**< RfSetRoutingTable command. */
    ptx_NSC_SM_CmdId_Undefined,     /**< Undefined command. */
} ptx_NSC_SM_CmdId_t;


/**
 * \brief NSC (RF) State Machine Check Item for Commands.
 */
typedef struct ptx_NSC_SM_CmdCheckItem
{
    uint8_t                     CmdId;          /**< Command ID. */
    uint16_t                    StateBitMask;   /**< State Bit Mask. */
} ptx_NSC_SM_CmdCheckItem_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Checks if the command execution is allowed in the current (RF) state.
 * If not allowed, INVALID_STATE status is returned.
 *
 * \param[in]   nsc                 Pointer to the component structure.
 * \param[in]   cmdId               Command ID of the NSC command.
 * \param[in]   cmdParams           Command specific parameters that need to be checked. If NULL, no parameters to check.
 *                                  cmdParams[0] := the number of parameters to follow
 *
 * \return Status of the operation. See definition of \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_CheckRfStateCmdExecution(struct ptx_NSC *nsc, ptx_NSC_SM_CmdId_t cmdId, uint32_t *cmdParams);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
