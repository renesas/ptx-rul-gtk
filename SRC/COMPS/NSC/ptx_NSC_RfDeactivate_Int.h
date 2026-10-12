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
    Module      : NFC Soft Controller (NSC) Internal Interface Header for RF Deactivation
    File        : ptx_NSC_RfDeactivate_Int.h

    Description : Definition of NFC Soft Controller Internal Interface for RF Deactivation.
                  Contains definition of types specific for RF Deactivation interface.
*/

/**
 * \addtogroup NSC_RF_DEACTIVATE
 *
 * @{
 */

#if !defined(PTX_NSC_RFDEACTIVATE_INT_H)
#define PTX_NSC_RFDEACTIVATE_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_Error_Int.h"


#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief RF State enumeration.
 */
typedef enum ptx_NSC_RfDeact_State
{
    NSC_RfDeact_State_Idle                  = 0,    /**< Idle state. */
    NSC_RfDeact_State_Poll                  = 1,    /**< Poll state. */
    NSC_RfDeact_State_Discovery             = 3,    /**< Discovery state. */
    NSC_RfDeact_State_DiscoveryWithRfReset  = 4,    /**< Discovery state with RF field reset. */
    NSC_RfDeact_State_ListenSleep           = 5,    /**< Listen Sleep. */
    NSC_RfDeact_State_ListenIdle            = 6     /**< Listen Idle. */
} ptx_NSC_RfDeact_State_t;

/**
 * \brief Deactivation Type.
 */
typedef enum ptx_NSC_RfDeact_Type
{
    NSC_RfDeact_Type_Generic                = 0,    /**< No protocol specific deactivation. */
    NSC_RfDeact_Type_ProtSpecific           = 1     /**< Protocol specific deactivation.    */
} ptx_NSC_RfDeact_Type_t;

/**
 * \brief Deactivation Parameters.
 */
typedef struct ptx_NSC_RfDeact_Param
{
    ptx_NSC_RfDeact_Type_t          Type;       /**< Deactivation type.                         */
    ptx_NSC_RfDeact_State_t         State;      /**< RF state of PTX2xxR/W after the deactivation.  */
} ptx_NSC_RfDeact_Param_t;

/**
 * \brief Deactivate Reason.
 */
typedef enum ptx_NSC_RfDeactivate_Reason
{
    ptx_NSC_RfDeactivate_Reason_Finished = 1u,  /**< Deactivate command finished. */
    ptx_NSC_RfDeactivate_Reason_Deselect = 2u,  /**< Deselect or sleep request command received. */
    ptx_NSC_RfDeactivate_Reason_Release  = 3u,  /**< Release command received. */
    ptx_NSC_RfDeactivate_Reason_RF_Field = 4u,  /**< Remote RF field deactivated. */
    ptx_NSC_RfDeactivate_Reason_RFU      = 5u   /**< RFU. */
} ptx_NSC_RfDeactivate_Reason_t;

/**
 * \brief Deactivation Notification.
 */
typedef struct ptx_NSC_RfDeact_Ntf
{
    ptx_NSC_RfDeactivate_Reason_t   Reason;     /**< Deactivation reason. */
    ptx_NSC_ErrorCodes_t            Error;      /**< Status of the deactivation. */
} ptx_NSC_RfDeactivate_Ntf_t;

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
