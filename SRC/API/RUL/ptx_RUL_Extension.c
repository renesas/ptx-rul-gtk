/* \file
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
    Module      : RUL
    File        : ptx_RUL_Extension.c

    Description : Reader Universal Library (RUL) extension API implementation.
                  Contains functions to register and remove extension endpoints for asynchronous communication.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_RUL_Cfg.h"
#include "ptx_RUL.h"

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */



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

ptx_Status_t ptx_RUL_RegisterExtensionEndpoints(ptx_RUL_t *rul, ptx_RUL_Extension_Endpoints_t* endpoint)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (NULL != endpoint) && (NULL != endpoint->Extension) && (Comp_MAX > endpoint->CompId))
    {
        status = PTX_STATUS_SUCCESS;
        uint8_t index = 0;
        switch (endpoint->CompId)
        {
            case Comp_LLCP:
                index = PTX_RUL_EXTENSION_INDEX_LLCP;
                break;
            case Comp_WLC:
                index = PTX_RUL_EXTENSION_INDEX_WLC;
                break;
            case Comp_HCE:
                index = PTX_RUL_EXTENSION_INDEX_HCE;
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_STATE;
                break;
        }

        if (PTX_STATUS_SUCCESS == status)
        {
            if (Comp_None == rul->ExtensionEndpoints[index].CompId)
            {
                rul->ExtensionEndpoints[index] = *endpoint;
            }
            else
            {
                status = PTX_STATUS_E_GEN_INVALID_STATE;
            }

        }
    }

    return status;
}

ptx_Status_t ptx_RUL_RemoveExtensionEndpoints(ptx_RUL_t *rul, ptx_Comps_t compId)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(rul, Comp_RUL) && (Comp_MAX > compId))
    {
        for (uint8_t i = 0; i < PTX_RUL_MAX_EXTENSIONS; i++)
        {
            if (rul->ExtensionEndpoints[i].CompId == compId)
            {
                memset(&rul->ExtensionEndpoints[i], 0, sizeof(ptx_RUL_Extension_Endpoints_t));
                status = PTX_STATUS_SUCCESS;
            }

            if ((PTX_STATUS_SUCCESS == status) && (rul->ActiveExtensionEndpoint == &rul->ExtensionEndpoints[i]))
            {
                rul->ActiveExtensionEndpoint = NULL;
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
