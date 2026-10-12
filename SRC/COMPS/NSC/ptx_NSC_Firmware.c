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
    Module      : NFC Soft Controller (NSC) Interface for Firmware Activation
    File        : ptx_NSC_Firmware.c

    Description : Implementation of NFC Soft Controller Interface for Firmware Activation.
                  Contains function(s) to download the firmware after the power-up and to
                  activate the firmware after successful download.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_HIP.h"
#include "ptx_NSC_Firmware.h"
#include "ptx_NSC_Base.h"
#include "ptx_NSC_uCODE.h"
#include "ptx_HAL.h"


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NSC_FIRMWARE_CRC_ADDRESS   0x5FFF      /* Address of the CRC in code memory */
#define PTX_NSC_HARDWARE_VERSION       0xAA        /* Hardware/Silicon version */

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */
static ptx_Status_t ptx_NSC_WriteSection(ptx_NSC_t *nsc, const uint32_t address, const uint16_t *data, const uint32_t dataLength);
static ptx_Status_t ptx_NSC_PrepareForcedDownload(ptx_NSC_t *nsc);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NSC_Firmware_Download(ptx_NSC_t *nsc, uint8_t forceDownload, ptx_NSC_FwDownloadCb_t fwDownloadCb)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_I(nsc->Log, "ptx_NSC_Firmware_Download");

        /* NSC structure holds HW and FW version information retrieved from NSC_RESET_NTF. */

        /* Check FW version. */
        if (0 == nsc->FwVersion)
        {
            /* No firmware inside. Validate HW version next. */
            if (PTX_NSC_HARDWARE_VERSION != nsc->HwVersion)
            {
                /* Detected a wrong HW version. */
                status = PTX_STATUS_E_GEN_WRONG_HARDWARE;
            }
            else
            {
                /* Download new firmware. */
                status = PTX_STATUS_SUCCESS;
            }
        }
        else
        {
            /* There is valid firmware inside the chip. Check if we need to download anyway. */
            if (PTX_NSC_FIRMWARE_FORCE_DOWNLOAD_ON == forceDownload)
            {
                /* Special routine is required to stop DFY and prepare it for FW download. */
                status = ptx_NSC_PrepareForcedDownload(nsc);
            }
            else
            {
                /* There is valid FW inside, but we called this API and do not want to download. */
                status = PTX_STATUS_E_NSC_FW_DL_DENIED;
            }
        }

        if ((PTX_STATUS_SUCCESS == status) && (NULL == fwDownloadCb))
        {
            /* Download Firmware from the source file. */

            /* Write one code section after the other until finished or an error occurred. */
            uint32_t i = 0;
            while ( (PTX_STATUS_SUCCESS == status) && (i < NUM_OF_SECTIONS) )
            {
                status = ptx_NSC_WriteSection(nsc, (uint32_t)ptx_NSC_uCODE_init_adds[i],  ptx_NSC_uCODE_sections[i], (uint32_t)ptx_NSC_uCODE_sections_size[i]);
                i++;
            }
        }
        else if ((PTX_STATUS_SUCCESS == status) && (NULL != fwDownloadCb))
        {
            /* Download Firmware through the user provided callback. */
            status = fwDownloadCb(nsc);
        }
        else
        {
            /* Keep the error */
        }
    }

    return status;
}

ptx_Status_t ptx_NSC_Firmware_Activate(ptx_NSC_t *nsc, ptx_NSC_FwVerifyCb_t fwVerifyCb)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_Firmware_Activate");

        /* System reset and enable DFY. */
        status = ptx_HIP_SysRst(nsc->HIP, HIP_Cmd_SysRst_RstEnDfy);

#if defined(PTX_ENABLE_I3C) 
        if (PTX_STATUS_SUCCESS == status)
        {
            /* 
             * Add a delay for the system to wait after a reset until we can communicate again with the PTX.
             * This delay is required because the firmware download is done faster using I3C
             */
            status = ptx_HAL_Sleep(nsc->HAL, 1U);
            if (PTX_STATUS_SUCCESS == status)
            {
                ptx_HAL_DRV_Interface_Parameter_t hal_param;
                hal_param.type = HAL_DRV_Interface_Parameter_Type_I3C_DynamicAddressAssignment;
                hal_param.value.I3C_DynamicAddressAssignment = PTX_HAL_DRV_INTERFACE_I3C_DAA;
                status = ptx_HAL_Interface_SetParameter(nsc->HAL, &hal_param);
            }
        }
#endif
        if (PTX_STATUS_SUCCESS == status)
        {
            /* Receive NSC_RST_NTF */
            status = ptx_NSC_GetResetNtf(nsc);
        }

        if ((PTX_STATUS_SUCCESS == status) && (NULL == fwVerifyCb))
        {
            /* Verify firmware. */

            /* Get the expected FW version from a source file. */
            uint16_t expected_fw_version = (uint16_t) ptx_NSC_uCODE_SRC_REV;

            /* FW version shall match the one that was downloaded. Expected FW version shall never be 0. */
            if ((0 != nsc->FwVersion) && (expected_fw_version == nsc->FwVersion))
            {
                /* Valid FW is inside. */
                status = PTX_STATUS_SUCCESS;
            }
            else
            {
                /* Invalid FW. */
                status = PTX_STATUS_E_GEN_FW_INVALID;
            }
        }
        else if ((PTX_STATUS_SUCCESS == status) && (NULL != fwVerifyCb))
        {
            /* Verify Firmware through the user provided callback. */
            status = fwVerifyCb(nsc);
        }
        else
        {
            /* Keep the error */
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Wait for 10ms  until the PTX2xxR/W is fully operational. */
            ptx_HAL_Sleep(nsc->HAL, 10);
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */

static ptx_Status_t ptx_NSC_WriteSection(ptx_NSC_t *nsc, const uint32_t address, const uint16_t *data, const uint32_t dataLength)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_WriteSection");

        /* System is ready to be flashed. */
        uint16_t write_address = (uint16_t) address;
        uint16_t *write_content = (uint16_t*) data;

        /*
         *  Use maximum possible payload length.
         *  Subtract frame overhead (SOF, LEN(2), FCB, CRC(2)).
         */
        uint32_t frame_len = PTX_HIP_MAX_PAYLOAD_LENGTH - PTX_HIP_MAX_ADDRESS_LENGTH;

        /* We want to know the frame_len in units of uint16_t. */
        frame_len = (frame_len / sizeof(uint16_t));

        /* Calculate the amount of frames to send. */
        uint32_t frames = dataLength / frame_len;

        /* Set status to SUCCESS and send frames until finished or error occurred. */
        status = PTX_STATUS_SUCCESS;
        while ( (PTX_STATUS_SUCCESS == status) && (frames > 0) )
        {
            status = ptx_HIP_WrCMem(nsc->HIP, write_address, write_content, frame_len);

            write_address = (uint16_t) (write_address + frame_len);  /* advance write address */
            write_content = (uint16_t*) (write_content + frame_len); /* advance write pointer */
            frames--;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Check if there is remaining data to send. */
            uint32_t final_frame_len = dataLength % frame_len;
            if (0 < final_frame_len)
            {
                /* Send final frame. */
                status = ptx_HIP_WrCMem(nsc->HIP, write_address, write_content, final_frame_len);
            }
        }
    }

    return status;
}

static ptx_Status_t ptx_NSC_PrepareForcedDownload(ptx_NSC_t *nsc)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    /* This routine is called when a valid firmware is already running and it is requested to download new firmware.
     * 1. Stop DFY
     * 2. Invalidate the CRC
     * 3. Restart DFY
     * 4. Wait for the reset NTF
     * (5. Download new FW) => done outside this function, within the caller
     */

    if (PTX_COMP_CHECK(nsc, Comp_NSC))
    {
        PTX_LOG_V(nsc->Log, "ptx_NSC_PrepareForcedDownload");

        /* Stop DFY. */
        status = ptx_HIP_SysRst(nsc->HIP, HIP_Cmd_SysRst_NoRstDisDfy);

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Invalidate CRC: write 0 to 0x5FFF. */
            uint16_t crc_address = PTX_NSC_FIRMWARE_CRC_ADDRESS;
            uint16_t new_value = 0;
            uint32_t data_len = 1;
            status = ptx_HIP_WrCMem(nsc->HIP, crc_address, &new_value, data_len);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Restart DFY. */
            status = ptx_HIP_SysRst(nsc->HIP, HIP_Cmd_SysRst_RstEnDfy);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* Receive NSC_RST_NTF */
            status = ptx_NSC_GetResetNtf(nsc);
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            /* NSC_RESET_NTF should hold FW_Version == 0 (invalid FW version). */
            if (0 != nsc->FwVersion)
            {
                /* CRC invalidation failed. */
                status = PTX_STATUS_E_GEN_INTERNAL;
            }

            if (PTX_NSC_HARDWARE_VERSION != nsc->HwVersion)
            {
                /* CRC invalidation failed. */
                status = PTX_STATUS_E_GEN_WRONG_HARDWARE;
            }
        }
    }

    return status;
}
