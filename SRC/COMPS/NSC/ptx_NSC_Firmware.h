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
    Module      : NFC Soft Controller (NSC) Interface Header for Firmware Activation
    File        : ptx_NSC_Firmware.h

    Description : Definition of NFC Soft Controller Interface for Firmware Activation.
                  Contains function(s) to download the firmware after the power-up and to
                  activate the firmware after successful download.
*/

/**
 * \addtogroup NSC_FIRMWARE
 *
 * @{
 */

#if !defined(PTX_NSC_FIRMWARE_H)
#define PTX_NSC_FIRMWARE_H

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
#define PTX_NSC_RST_NTF_TIMEOUT             (50u)       /**< Allowed timeout between execution of RST and arrival of EXT_IRQ */
#define PTX_NSC_FIRMWARE_FORCE_DOWNLOAD_ON  (1u)        /**< Force the download of the uCode, reset PTX2xxR/W if needed. */
#define PTX_NSC_FIRMWARE_FORCE_DOWNLOAD_OFF (0u)        /**< Deny uCode download if the PTX2xxR/W is running. */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Callback to download firmware.
 *
 * \param[in]   ctx         Callback execution context.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_FwDownloadCb_t) (ptx_NSC_t *ctx);

/**
 * \brief Callback to verify downloaded firmware.
 *
 * \param[in]   ctx         Callback execution context.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
typedef ptx_Status_t (*ptx_NSC_FwVerifyCb_t) (ptx_NSC_t *ctx);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Downloads the firmware
 *
 * \note This API shall not be called before PTX2xxR/W reset.
 *
 * \param[in] nsc               Pointer to component.
 * \param[in] forceDownload     Forces the stack to download the uCode. If not set and valid FW is already running, the API fails.
 * \param[in] fwDownloadCb      Callback to download firmware. If NULL, use default routine. Otherwise, execute the callback.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_Firmware_Download(ptx_NSC_t *nsc, uint8_t forceDownload, ptx_NSC_FwDownloadCb_t fwDownloadCb);

/**
 * \brief Activates the firmware
 *
 * \note This API is supposed to be called only if NSC has been previously reset, i.e. while
 * NSC is in Off state (NSC_INIT_CMD has not taken place).
 * Using the API in any other state might result in undefined system behaviour.
 *
 * \param[in] nsc               Pointer to component.
 * \param[in] fwVerifyCb        Callback to verify downloaded firmware. If NULL, use default routine. Otherwise, execute the callback.
 *
 * \return Status, indicating whether the operation was successful. See ptx_Status_t.
 */
ptx_Status_t ptx_NSC_Firmware_Activate(ptx_NSC_t *nsc, ptx_NSC_FwVerifyCb_t fwVerifyCb);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
