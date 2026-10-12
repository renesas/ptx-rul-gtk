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
    Module      : NFC Data Exchange Format (NDEF) Wireless Charging Foreign Object Detection (WLCFOD) Record
    File        : ptx_NDEFRecord_WLCFOD.c

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Wireless Charging Foreign Object Detection (WLCFOD) NDEF records.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NDEFRecord_WLCFOD.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NDEFRecord_WLCFOD_Init (ptx_NDEFRecord_WLCFOD_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        memset(container, 0, sizeof(ptx_NDEFRecord_WLCFOD_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCFOD_Create(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCFOD_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != record) && (NULL != container) )
    {
        status = ptx_NDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) TYPE_RTD_WLCFOD, TYPE_RTD_WLCFOD_LEN, NULL, 0u, (uint8_t*) container, sizeof(ptx_NDEFRecord_WLCFOD_t));
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCFOD_Parse(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCFOD_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != record) && (NULL != container) )
    {
        if ( (record->Payload.Length == sizeof(ptx_NDEFRecord_WLCFOD_t)) &&
             (0 == strncmp(record->Type.sData, TYPE_RTD_WLCFOD, TYPE_RTD_WLCFOD_LEN)) )
        {
            memcpy(container, record->Payload.Data, sizeof(ptx_NDEFRecord_WLCFOD_t));

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCFOD_SetCompanyId(ptx_NDEFRecord_WLCFOD_t *container, uint8_t companyId)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->COMPANY_ID = companyId;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCFOD_GetCompanyId(ptx_NDEFRecord_WLCFOD_t *container, uint8_t *companyId)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != companyId) )
    {
        *companyId = container->COMPANY_ID;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCFOD_SetMethodId(ptx_NDEFRecord_WLCFOD_t *container, uint8_t methodId)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->METHOD_ID = methodId;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCFOD_GetMethodId(ptx_NDEFRecord_WLCFOD_t *container, uint8_t *methodId)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != container) && (NULL != methodId))
    {
        *methodId = container->METHOD_ID;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}
