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
    Module      : NFC Soft Controller (NSC) Interface Header for Reader Universal Library (RUL) RF Configuration
    File        : ptx_NSC_RUL_RfConfig.h

    Description : Definition of NFC Soft Controller Interface for Reader Universal Library RF Configuration.
                  Contains function(s) for RUL-specific handling of RF configuration parameters.
*/

/**
 * \addtogroup NSC_RF_CONFIG
 *
 * @{
 */

#if !defined(PTX_NSC_RUL_RFCONFIG_H)
#define PTX_NSC_RUL_RFCONFIG_H

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
 * \name Supported NSC RF Configuration file version.
 * @{
 */
#define PTX_NSC_RFCONFIG_VERSION        (43u)   /**< This version is supported */
/** @} */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */
/**
 * \brief Parameter IDs for RF_POLL section
 */
typedef enum ptx_NSC_RfConfig_ParamId
{
    /* Poll Mode. */
    ParamId_PollPwrLvl0         = 0x00,
    ParamId_PollPwrLvl1         = 0x01,
    ParamId_PollDdpcAvg         = 0x02,
    ParamId_PollPwrLvl0_Thr1    = 0x03,
    ParamId_PollPwrLvl0_Thr0    = 0x04,
    ParamId_PollPwrLvl1_Thr1    = 0x05,
    ParamId_PollPwrLvl1_Thr0    = 0x06,
    ParamId_Poll_Max            = 0x07,

    ParamId_AnaRxFeConfig_A106  = 0x08,
    ParamId_AnaRxFeConfig_A212  = 0x09,
    ParamId_AnaRxFeConfig_A424  = 0x0A,
    ParamId_AnaRxFeConfig_A848  = 0x0B,
    ParamId_AnaRxFeConfig_B106  = 0x0C,
    ParamId_AnaRxFeConfig_B212  = 0x0D,
    ParamId_AnaRxFeConfig_B424  = 0x0E,
    ParamId_AnaRxFeConfig_B848  = 0x0F,
    ParamId_AnaRxFeConfig_F212  = 0x10,
    ParamId_AnaRxFeConfig_F424  = 0x11,
    ParamId_AnaRxFeConfig_V26   = 0x12,
    ParamId_AnaRxFeConfig_Max   = 0x13,

    ParamId_AnaRxBbConfig_A106  = 0x14,
    ParamId_AnaRxBbConfig_A212  = 0x15,
    ParamId_AnaRxBbConfig_A424  = 0x16,
    ParamId_AnaRxBbConfig_A848  = 0x17,
    ParamId_AnaRxBbConfig_B106  = 0x18,
    ParamId_AnaRxBbConfig_B212  = 0x19,
    ParamId_AnaRxBbConfig_B424  = 0x1A,
    ParamId_AnaRxBbConfig_B848  = 0x1B,
    ParamId_AnaRxBbConfig_F212  = 0x1C,
    ParamId_AnaRxBbConfig_F424  = 0x1D,
    ParamId_AnaRxBbConfig_V26   = 0x1E,
    ParamId_AnaRxBbConfig_Max   = 0x1F,

    ParamId_ClDrxDf_A106        = 0x20,
    ParamId_ClDrxDf_A212        = 0x21,
    ParamId_ClDrxDf_A424        = 0x22,
    ParamId_ClDrxDf_A848        = 0x23,
    ParamId_ClDrxDf_B106        = 0x24,
    ParamId_ClDrxDf_B212        = 0x25,
    ParamId_ClDrxDf_B424        = 0x26,
    ParamId_ClDrxDf_B848        = 0x27,
    ParamId_ClDrxDf_F212        = 0x28,
    ParamId_ClDrxDf_F424        = 0x29,
    ParamId_ClDrxDf_V26         = 0x2A,
    ParamId_ClDrxDf_Max         = 0x2B,
    ParamId_Max
} ptx_NSC_RfConfig_ParamId_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Load configuration parameters into NSC internal RfConfig holder.
 *
 * \param[in]       nsc         Pointer to component.
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER     Invalid input parameter.
 * \return PTX_STATUS_E_GEN_NOT_FOUND             The parameter ID does not exist (RF-config ambiguous).
 * \return PTX_STATUS_E_NSC_INVALID_RFCONFIG      The specified RF-config is outdated.
 * \return Others                                 See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RUL_RfConfig_LoadParam(ptx_NSC_t *nsc);

/**
 * \brief Set/Update the internal RF configuration data structures.
 *
 * \param[in]       nsc         Pointer to component.
 * \param[in]       config      Pointer to the settings data structure for updating the RF configuration accordingly.
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER     Invalid input parameter.
 * \return PTX_STATUS_E_GEN_NOT_FOUND             The parameter ID does not exist (RF-config ambiguous).
 * \return Others                                 See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfConfig_SetSection(ptx_NSC_t *nsc, ptx_NSC_RfConfig_Param_t *config);

/**
 * \brief Retrieves the internal RF configuration data structures.
 *
 * \param[in]       nsc         Pointer to component.
 * \param[in]       config      Pointer to store the RF configuration.
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER     Invalid input parameter.
 * \return PTX_STATUS_E_GEN_NOT_FOUND             The parameter ID does not exist (RF-config ambiguous).
 * \return Others                                 See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfConfig_GetSection(ptx_NSC_t *nsc, ptx_NSC_RfConfig_Param_t *config);

/**
 * \brief Function used to set parameters of the RF-config during runtime.
 *
 * \note This function sets the parameter value within the internal RF-config structure.
 *
 * \param[in]    nsc    Pointer to the component structure.
 * \param[in]    id     Id of the parameter to be set.
 * \param[in]    value  Parameter value to be written to the internal RF-config structure.
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER     Invalid input parameter.
 * \return Others                                 See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfConfig_SetParam(ptx_NSC_t *nsc, ptx_NSC_RfConfig_ParamId_t id, uint8_t value);

/**
 * \brief Function used to retrieve parameters of the RF-config during runtime.
 *
 * \note This function retrieves the specified parameter value from the internal RF-config structure.
 *
 * \param[in]    nsc    Pointer to the component structure.
 * \param[in]    id     Id of the parameter to be retrieved.
 * \param[in]    value  Parameter value retrieved from the internal RF-config structure.
 *
 * \return PTX_STATUS_SUCCESS                     Function executed successfully.
 * \return PTX_STATUS_E_GEN_INVALID_PARAMETER     Invalid input parameter.
 * \return Others                                 See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfConfig_GetParam(ptx_NSC_t *nsc, ptx_NSC_RfConfig_ParamId_t id, uint8_t *value);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
