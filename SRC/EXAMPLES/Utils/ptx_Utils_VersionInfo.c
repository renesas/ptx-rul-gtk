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
    Module      : Utils
    File        : ptx_Utils_VersionInfo.c

    Description : Implements a helper-function to read the available revision-infos from the RUL-Stack and from the Chip.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_Utils_VersionInfo.h"
#include "ptx_Utils_Print.h"
#include <stdio.h>
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_RUL_REV_INFO_MAX_BUFFER_SIZE              (uint8_t)8  /* Maximum buffer size to store individual revision/version info. */


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_Utils_VersionInfo_Print_Revision(ptx_RUL_t *rul)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    ptx_RUL_InfoTypes_t info_type;

    uint8_t rev_info[PTX_RUL_REV_INFO_MAX_BUFFER_SIZE];
    uint8_t rev_info_size;

    if (NULL != rul)
    {
        ptx_Utils_Print("***********************************************************\n");
        ptx_Utils_Print("**************** Revision Information *********************\n");
        ptx_Utils_Print("***********************************************************\n");

        /* Get C-Stack Revision */
        info_type = ptx_RUL_InfoType_Stack_Revision;
        rev_info_size = PTX_RUL_REV_INFO_MAX_BUFFER_SIZE;
        status = ptx_RUL_GetInfo(rul, info_type, rev_info, &rev_info_size);
        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_Utils_Print("  C-Stack Revision.......: v%02X.%02X.%02X\n", rev_info[0], rev_info[1], rev_info[2]);
        }

        /* Get FW-Code Revision */
        info_type = ptx_RUL_InfoType_FW_Revision;
        rev_info_size = PTX_RUL_REV_INFO_MAX_BUFFER_SIZE;
        status = ptx_RUL_GetInfo(rul, info_type, rev_info, &rev_info_size);
        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_Utils_Print("  FW-Code Revision.......: 0x%02X%02X\n", rev_info[1], rev_info[0]);
        }

        /* Get RF-Config Version */
        info_type = ptx_RUL_InfoType_RF_Config_Version;
        rev_info_size = PTX_RUL_REV_INFO_MAX_BUFFER_SIZE;
        status = ptx_RUL_GetInfo(rul, info_type, rev_info, &rev_info_size);
        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_Utils_Print("  RF-Config Version......: 0x%02X\n", rev_info[0]);
        }

        /* Get HW-Version */
        info_type = ptx_RUL_InfoType_HW_Version;
        rev_info_size = PTX_RUL_REV_INFO_MAX_BUFFER_SIZE;
        status = ptx_RUL_GetInfo(rul, info_type, rev_info, &rev_info_size);
        if (PTX_STATUS_SUCCESS == status)
        {
            ptx_Utils_Print("  HW Version.............: 0x%02X\n", rev_info[0]);
        }

        /* Get Git Hash */
        info_type = ptx_RUL_InfoType_Platform_Revision;
        rev_info_size = PTX_RUL_REV_INFO_MAX_BUFFER_SIZE;
        status = ptx_RUL_GetInfo(rul, info_type, rev_info, &rev_info_size);
        if (PTX_STATUS_SUCCESS == status)
        {
            uint64_t hash = 0;
            for (int i = 0; i < 8; i++) {
                hash |= ((uint64_t)rev_info[i]) << (8 * i);
            }
            ptx_Utils_Print("  Platform Revision......: %01X%08X\n", (uint32_t)(hash >> 32 & 0xF), (uint32_t)(hash & 0xFFFFFFFF));
        }

        /* Get Local Changes */
        info_type = ptx_RUL_InfoType_Local_Changes;
        rev_info_size = PTX_RUL_REV_INFO_MAX_BUFFER_SIZE;
        status = ptx_RUL_GetInfo(rul, info_type, rev_info, &rev_info_size);
        if (PTX_STATUS_SUCCESS == status)
        {
            const char* value = rev_info[0] ? "yes": "no";
            ptx_Utils_Print("  Local Changes..........: %s\n", value);
        }

        ptx_Utils_Print("***********************************************************\n");

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

