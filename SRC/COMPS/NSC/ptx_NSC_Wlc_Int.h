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
    Module      : NFC Soft Controller (NSC) Internal Interface Header for Wireless Charging (WLC)
    File        : ptx_NSC_Wlc_Int.h

    Description : Definition of NFC Soft Controller Internal Interface for Wireless Charging.
                  Contains definition of types specific for wireless charging.
*/

/**
 * \addtogroup NSC_WLC
 *
 * @{
 */

#if !defined(PTX_NSC_WLC_INT_H)
#define PTX_NSC_WLC_INT_H

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

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */
/**
 * \brief Wireless Power Transfer options.
 */
typedef struct ptx_NSC_WLC_WptOptions
{
    uint8_t  WptAmp;    /**< Amplitude level used for WPT cycle. (Range: 0 - 63) */
    uint8_t  NfcAmp;    /**< Amplitude level used for communication. (Range: 0 - 63) */
    uint32_t WptDur;    /**< WPT Duration in milliseconds. */
    uint8_t  IcdEn;     /**< Enable Impedance Change Detection. (Enabled if not 0) */
    uint8_t  IcdGt;     /**< Impedance Change Detection guard time. */
} ptx_NSC_WLC_WptOptions_t;

/**
 * \brief WPT Stop Reason
 */
typedef enum ptx_NSC_WLC_WptStopReason
{
    WptStopReason_Expired   = 1u,   /**< WLC WPT duration expired. */
    WptStopReason_ICD       = 2u,   /**< Detected an impedance change. */
    WptStopReason_RFU,              /**< RFU. */
} ptx_NSC_WLC_WptStopReason_t;

/**
 * \brief WPT Stop Notification
 */
typedef struct ptx_NSC_WLC_WptStop_Ntf
{
    ptx_NSC_WLC_WptStopReason_t Reason;        /**< WPT stop Reason. */
} ptx_NSC_WLC_WptStop_Ntf_t;

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
