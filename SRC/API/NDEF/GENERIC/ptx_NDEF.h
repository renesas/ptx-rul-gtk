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
    Module      : NFC Data Exchange Format (NDEF)
    File        : ptx_NDEF.h

    Description : NFC Data Exchange Format (NDEF) controls the by NFC Forum defined message format to exchange
                  information between two NFC Forum devices.
*/

/**
 * \addtogroup NDEF
 *
 * @{
 */

#if !defined(PTX_NDEF_H)
#define PTX_NDEF_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include <stddef.h>
#include "ptx_Status.h"
#include "ptx_Components.h"
#include "ptx_RUL.h"

#if !defined(PTX_DISABLE_NDEF_T2T)
    #include "ptx_NDEF_T2T.h"
#endif

#if !defined(PTX_DISABLE_NDEF_T3T)
    #include "ptx_NDEF_T3T.h"
#endif

#if !defined(PTX_DISABLE_NDEF_T4T)
    #include "ptx_NDEF_T4T.h"
#endif

#if !defined(PTX_DISABLE_NDEF_T5T)
    #include "ptx_NDEF_T5T.h"
#endif

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
 * \brief T5T communication mode.
 */
typedef enum ptx_NDEF_T5T_Modes
{
    ptx_NDEF_T5T_Mode_NonAddressed,     /**< No Mode specified. */
    ptx_NDEF_T5T_Mode_Addressed,        /**< Addressed communication.*/
    ptx_NDEF_T5T_Mode_Selected,         /**< Selected communication. */
} ptx_NDEF_T5T_Modes_t;

/**
 * \brief Tag Specific Component Union
*/
typedef union ptx_NDEF_Tag_Ctx
{
#if !defined(PTX_DISABLE_NDEF_T2T)
    ptx_NDEF_T2T_t t2t;                 /**< Tag Type 2 Context */
#endif
#if !defined(PTX_DISABLE_NDEF_T3T)
    ptx_NDEF_T3T_t t3t;                 /**< Tag Type 3 Context */
#endif
#if !defined(PTX_DISABLE_NDEF_T4T)
    ptx_NDEF_T4T_t t4t;                 /**< Tag Type 4 Context */
#endif
#if !defined(PTX_DISABLE_NDEF_T5T)
    ptx_NDEF_T5T_t t5t;                 /**< Tag Type 5 Context */
#endif
} ptx_NDEF_Tag_Ctx_t;



/**
 * \brief Main Component Struture
 */
typedef struct ptx_NDEF
{
    ptx_Comps_t                 CompId;                             /**< Component Id */

    ptx_RUL_CardProtocolType_t  type;                               /**< Active Tag type. */
    ptx_NDEF_Tag_Ctx_t          tagCtx;                             /**< Active Tag context. */

    uint8_t                     *rxBuffer;                          /**< Internal rx buffer. */
    uint32_t                    rxLen;                              /**< Internal rx buffer length. */
    uint8_t                     *txBuffer;                          /**< Internal tx buffer. */
    uint32_t                    txLen;                              /**< Internal tx buffer length. */
    uint8_t                     *workBuffer;                        /**< Internal work buffer. */
    uint32_t                    workBufferLen;                      /**< Internal work buffer length. */
    uint32_t                    rxSize;                             /**< Internal rx buffer maximum length. */
    uint32_t                    txSize;                             /**< Internal tx buffer maximum length. */

    ptx_RUL_t                   *rul;                               /**< Pointer to initialized RUL component. */
} ptx_NDEF_t;

/**
 * \brief Main Component Initialization Parameters
 */
typedef struct ptx_NDEF_InitParams
{
    uint8_t                     *rx;                                /**< Internal rx buffer. Used for component internal exchanges (Recommended size: 256 bytes) */
    uint32_t                    rxLen;                              /**< Internal rx buffer length. */
    uint8_t                     *tx;                                /**< Internal tx buffer. Used for component internal exchanges (Recommended size: 256 bytes) */
    uint32_t                    txLen;                              /**< Internal tx buffer length. */
    uint8_t                     *workBuffer;                        /**< Internal work buffer. Used to store intermediate information (Recommended size: 256 bytes) */
    uint32_t                    workBufferLen;                      /**< Internal work buffer length. */
    ptx_RUL_t                   *rul;                               /**< Pointer to initialized RUL component. */

    ptx_NDEF_T5T_Modes_t        t5tMode;                            /**< T5T communication mode to use. */
} ptx_NDEF_InitParams_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Component Initialization.
 *
 * \param[in]       ndef            Pointer to component.
 * \param[in]       initParams      Pointer to initialization parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_Open (ptx_NDEF_t *ndef, ptx_NDEF_InitParams_t *initParams);

/**
 * \brief Component De-Initialization.
 *
 * \param[in]       ndef            Pointer to component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_Close (ptx_NDEF_t *ndef);

/**
 * \brief Check for NDEF Messages.
 *
 * \param[in]       ndef            Pointer to component
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_CheckMessage (ptx_NDEF_t *ndef);

/**
 * \brief Read NDEF Message. (Check message has to be performed before attempting to read a message)
 *
 * \param[in]       ndef            Pointer to component
 * \param[out]      msgBuffer       Pointer to buffer holding the read NDEF-message.
 * \param[in,out]   msgLen          Size of the buffer (in), Length of the read NDEF-message (out).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_ReadMessage (ptx_NDEF_t *ndef, uint8_t *msgBuffer, uint32_t *msgLen);

/**
 * \brief Write NDEF Message. (Check message has to be performed before attempting to write a message)
 *
 * \param[in]       ndef            Pointer to component
 * \param[in]       msgBuffer       Pointer to buffer holding the NDEF-message to write (NULL -> empty NDEF-message is written).
 * \param[in]       msgLen          Size of NDEF-message (0 -> empty NDEF-message is written).
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_WriteMessage (ptx_NDEF_t *ndef, uint8_t *msgBuffer, uint32_t msgLen);

/**
 * \brief Lock Tag.
 * \param[in]       ndef            Pointer to component
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_LockTag (ptx_NDEF_t *ndef);

/**
 * \brief Format Tag.
 * \param[in]       ndef            Pointer to component
 *
 * \return Status, indicating whether the operation was successful. See \ref ptx_Status_t.
 */
ptx_Status_t ptx_NDEF_FormatTag (ptx_NDEF_t *ndef);


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
