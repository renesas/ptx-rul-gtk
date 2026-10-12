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
    Module      : NFC Soft Controller (NSC) Interface for Revision Information
    File        : ptx_NSC_RevisionInfo.c

    Description : Implementation of NFC Soft Controller Interface for Revision Information.
                  Contains function(s) to gather various system information.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NSC_RevisionInfo.h"
#include "ptx_NSC_uCODE.h"
#include "ptx_NSC_Misc.h"


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/*
 * NSC information data sizes (in number of bytes).
 */
#define PTX_GETINFO_DATA_SIZE_1     1U
#define PTX_GETINFO_DATA_SIZE_2     2U
#define PTX_GETINFO_DATA_SIZE_3     3U
#define PTX_GETINFO_DATA_SIZE_8     8U

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NSC_GetInfo(ptx_NSC_t *nscCtx, ptx_NSC_InfoType_t infoType, uint8_t *infoData, uint8_t *infoDataLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nscCtx, Comp_NSC) && (NULL != infoData) && (NULL != infoDataLen))
    {
        PTX_LOG_I(nscCtx->Log, "ptx_NSC_GetInfo");

        status = PTX_STATUS_SUCCESS;

        switch (infoType)
        {
            case ptx_NSC_Info_HW_Version:
            {
                /* HW version is stored in NSC after the reset.
                 * 1By length
                 */
                *infoData = nscCtx->HwVersion;
                *infoDataLen = PTX_GETINFO_DATA_SIZE_1;
                break;
            }

            case ptx_NSC_Info_Local_Changes:
            {
                /* 1By length */
                *infoData = PTX_PLATFORM_LOCAL_CHANGES;
                *infoDataLen = PTX_GETINFO_DATA_SIZE_1;
                break;
            }

            case ptx_NSC_Info_FW_Version:
            {
                /* FW version is stored in NSC after the reset.
                 * 2By length
                 */
                infoData[0] = (uint8_t)(nscCtx->FwVersion & 0x00FF);
                infoData[1] = (uint8_t)((nscCtx->FwVersion >> 8) & 0x00FF);
                *infoDataLen = PTX_GETINFO_DATA_SIZE_2;
                break;
            }

            case ptx_NSC_Info_DFY_Toolchain:
            {
                /* 2By length */
                infoData[0] = (uint8_t)((uint16_t)ptx_NSC_uCODE_ASM_REV & 0x00FF);
                infoData[1] = (uint8_t)((uint16_t)(ptx_NSC_uCODE_ASM_REV >> 8) & 0x00FF);
                *infoDataLen = PTX_GETINFO_DATA_SIZE_2;
                break;
            }

            case ptx_NSC_Info_Rf_Config:
            {
                /* Implementation specific RF config information. */
                status = ptx_NSC_GetRfConfigInfo(nscCtx, infoData, infoDataLen);
                break;
            }

            case ptx_NSC_Info_C_Stack:
            {
                /* 3By length */
                infoData[0] = PTX_STACK_REVISION_MAJOR;
                infoData[1] = PTX_STACK_REVISION_MINOR;
                infoData[2] = PTX_STACK_REVISION_PATCH;
                *infoDataLen = PTX_GETINFO_DATA_SIZE_3;
                break;
            }

            case ptx_NSC_Info_Derivate:
            {
                /* 2By length */
                infoData[0] = (uint8_t)(nscCtx->Derivate & 0x00FF);
                infoData[1] = (uint8_t)((nscCtx->Derivate >> 8) & 0x00FF);
                *infoDataLen = PTX_GETINFO_DATA_SIZE_2;
                break;
            }

            case ptx_NSC_Info_MtuSize:
            {
                /* 1By length */
                *infoData = nscCtx->MtU;
                *infoDataLen = PTX_GETINFO_DATA_SIZE_1;
                break;
            }

            case ptx_NSC_Info_Platform_Revision:
            {
                for (uint8_t index = 0; index < PTX_GETINFO_DATA_SIZE_8; index++)
                {
                    infoData[index] = (uint8_t)((PTX_PLATFORM_REVISION >> (8 * index)) & 0xFF);
                }
                *infoDataLen = PTX_GETINFO_DATA_SIZE_8;
                break;
            }

            default:
            {
                status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
                break;
            }
        }
    }

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
