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
    Module      : NFC Soft Controller (NSC) Interface Header for RF Configuration
    File        : ptx_NSC_RfConfig.h

    Description : Definition of NFC Soft Controller Interface for RF Configuration.
                  Contains function(s) to perform the system RF configuration before discovery process
                  and to gather information about the currently used RF configuration parameters.
*/

/**
 * \addtogroup NSC_RF_CONFIG
 *
 * @{
 */

#if !defined(PTX_NSC_RFCONFIG_H)
#define PTX_NSC_RFCONFIG_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Status.h"
#include <stddef.h>


#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \name Length definitions.
 * @{
 */
#define PTX_NSC_RFCONFIG_PARAMID_MAX_PARAMID                   (22u)         /**< NSC RF Config sets of parameters currently implemented */
/** @} */

/**
 * \brief Forward declaration of ptx_NSC structure.
 */
struct ptx_NSC;

/**
 * \brief Callback definition to load RfConfig data into NSC internal RfConfig structure.
 *
 * \param[in]       nsc     Pointer to context.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_RfConfig_LoadParamCb_t)(struct ptx_NSC *nsc);

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Known Parameter + Tags.
 */
typedef enum ptx_NSC_RfConfig_ParamListId
{
    /* Poll Mode. */
    NSC_RfConfig_ParamId_EoC          = 0x00, /**< End of command. */
    NSC_RfConfig_ParamId_Poll         = 0x02, /**< Poll. */
    NSC_RfConfig_ParamId_PollA106     = 0x03, /**< PollA106. */
    NSC_RfConfig_ParamId_PollA212     = 0x04, /**< PollA212. */
    NSC_RfConfig_ParamId_PollA424     = 0x05, /**< PollA424. */
    NSC_RfConfig_ParamId_PollA848     = 0x06, /**< PollA848. */
    NSC_RfConfig_ParamId_PollB106     = 0x07, /**< PollB106. */
    NSC_RfConfig_ParamId_PollB212     = 0x08, /**< PollB212. */
    NSC_RfConfig_ParamId_PollB424     = 0x09, /**< PollB424. */
    NSC_RfConfig_ParamId_PollB848     = 0x0A, /**< PollB848. */
    NSC_RfConfig_ParamId_PollF212     = 0x0B, /**< PollF212. */
    NSC_RfConfig_ParamId_PollF424     = 0x0C, /**< PollF424. */
    NSC_RfConfig_ParamId_PollV_26K5   = 0x0D, /**< PollV_26K5. */
    NSC_RfConfig_ParamId_PollV53      = 0x0E, /**< PollV53. */
    NSC_RfConfig_ParamId_PollV106     = 0x0F, /**< PollV106. */

#if !defined(PTX_DISABLE_LISTEN_MODE)
    /* Listen Mode. */
    NSC_RfConfig_ParamId_ListenA212   = 0x12, /**< ListenA212. */
    NSC_RfConfig_ParamId_ListenA424   = 0x13, /**< ListenA424. */
    NSC_RfConfig_ParamId_ListenA848   = 0x14, /**< ListenA848. */
    NSC_RfConfig_ParamId_ListenB212   = 0x16, /**< ListenB212. */
    NSC_RfConfig_ParamId_ListenB424   = 0x17, /**< ListenB424. */
    NSC_RfConfig_ParamId_ListenB848   = 0x18, /**< ListenB848. */
    NSC_RfConfig_ParamId_Listen       = 0x1D, /**< Listen. */
#endif /* PTX_DISABLE_LISTEN_MODE */

    NSC_RfConfig_ParamId_WaveGenPat   = 0x20, /**< Wavegen Pattern. */

    NSC_RfConfig_ParamId_MAX,                 /**< Safeguard. */

} ptx_NSC_RfConfig_ParamListId_t;

/**
 * \brief Configuration Parameter TLV.
 */
typedef struct ptx_NSC_RfConfig_Param
{
    ptx_NSC_RfConfig_ParamListId_t  Id;                         /**< Config parameter id. */
    uint32_t                        Length;                     /**< Config parameter length. */
    uint8_t                         *Value;                     /**< Config parameter value. */
} ptx_NSC_RfConfig_Param_t;

/**
 * \brief RF Configuration Structure.
 */
typedef struct ptx_NSC_RfConfig
{
    ptx_NSC_RfConfig_Param_t        RfCfgParam[PTX_NSC_RFCONFIG_PARAMID_MAX_PARAMID];    /**< Array of configuration parameters. */
    uint32_t                        RfCfgParamLen;  /**< Config parameter length. */
    ptx_NSC_RfConfig_LoadParamCb_t  LoadCfgParam;   /**< Callback to load parameter. */
} ptx_NSC_RfConfig_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Function used to build and send NSC_RF_CONFIG_CMD.
 *
 * \note This function just sends RF Parameters in RF Config (Not routing Table).
 *
 * \param[in]    nsc                Pointer to the component structure.
 * \param[in]    rfConfigParams     Pointer to the RF Config Parameters.
 * \param[in]    rfConfigParamCnt   Number of config parameters to be sent.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_RfConfig (struct ptx_NSC *nsc, ptx_NSC_RfConfig_Param_t *rfConfigParams, uint32_t rfConfigParamCnt);

/**
 * \brief Get various RF configuration file information.
 *
 * \param[in]    nsc                Pointer to the component structure.
 * \param[in]    infoData           Buffer to store the retrieved information data.
 * \param[in]    infoDataLen        The length of returned data, in bytes.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_GetRfConfigInfo(struct ptx_NSC *nsc, uint8_t *infoData, uint8_t *infoDataLen);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
