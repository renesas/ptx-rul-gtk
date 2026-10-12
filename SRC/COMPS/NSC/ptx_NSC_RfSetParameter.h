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
    Module      : NFC Soft Controller (NSC) Interface Header for Set Parameter
    File        : ptx_NSC_RfSetParameter.h

    Description : Definition of NFC Soft Controller Interface for Set Parameter.
                  Contains function(s) to customize parameters of the forthcoming RF transactions,
                  as might be required by the currently active RF interface or protocol.
*/

/**
 * \addtogroup NSC_RF_SET_PARAM
 *
 * @{
 */

#if !defined(SRC_COMPS_NSC_PTX_NSC_RFSETPARAMETER_H_)
#define SRC_COMPS_NSC_PTX_NSC_RFSETPARAMETER_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Int.h"
#include "ptx_NSC_RfSetParameter_Int.h"

#if defined(__cplusplus)
extern "C"
{
#endif


/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

#define PTX_NSC_RF_SET_PARAM_MAXPARAMCOUNT              (8u)             /**<  Maximum parameter count: there are 8 different parameter types. */
#define PTX_NSC_RF_SET_PARAM_FWT_THIRD_BYTE_INDEX       (2u)             /**<  Third Byte (MSB) index of the Frame Waiting Time parameter. */
#define PTX_NSC_RF_SET_PARAM_FWT_SECOND_BYTE_INDEX      (1u)             /**<  Second Byte index of the Frame Waiting Time parameter. */
#define PTX_NSC_RF_SET_PARAM_FWT_FIRST_BYTE_INDEX       (0u)             /**<  First Byte (LSB) index of the Frame Waiting Time parameter. */
#define PTX_NSC_RF_SET_PARAM_FWT_FIRST_BYTE_POSITION    (0u)             /**<  First Byte (LSB) starting Bit position of the Frame Waiting Time parameter. */
#define PTX_NSC_RF_SET_PARAM_FWT_SECOND_BYTE_POSITION   (8u)             /**<  Second Byte starting Bit position of the Frame Waiting Time parameter. */
#define PTX_NSC_RF_SET_PARAM_FWT_THIRD_BYTE_POSITION    (16u)            /**<  Third Byte (MSB) starting Bit position of the Frame Waiting Time parameter. */
#define PTX_NSC_RF_SET_PARAM_FWT_BYTE_MASK              ((uint8_t)0xFF)  /**<  Single Byte MAsk for the three FWT bytes. */

#define PTX_NSC_RF_SET_PARAM_RX_START_BIT               (4u)             /**<  Start Bit Rx parameters for various Set Param fields. */
#define PTX_NSC_RF_SET_PARAM_RX_BITOFFSET_MASK          (0x07u)          /**<  Mask for Rx Bit Offset. */
#define PTX_NSC_RF_SET_PARAM_FLAG_MASK                  (0x01u)          /**<  Mask for all one bit Parameter Flags. */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief This function builds the NSC_RF_SET_PARAM_CMD accordingly and transmits it to the PTX2xxR/W.
 *
 * \param[in]   nsc                 Pointer to the component structure.
 * \param[in]   setParams           Initialization parameters to build up the command properly.
 * \param[in]   paramCnt            Length of initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 *
 */
ptx_Status_t ptx_NSC_RfSetParameter(ptx_NSC_t *nsc, ptx_NSC_RfSetParameter_Param_t *setParams, uint32_t paramCnt);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
