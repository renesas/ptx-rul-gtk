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
    Module      : NFC Data Exchange Format (NDEF) Wireless Charging Capability (WLCCAP) Record
    File        : ptx_NDEFRecord_WLCCAP.c

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Wireless Charging Capability (WLCCAP) NDEF records.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NDEFRecord_WLCCAP.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NDEFRecord_WLCCAP_Init (ptx_NDEFRecord_WLCCAP_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        memset(container, 0, sizeof(ptx_NDEFRecord_WLCCAP_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_Create (ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCCAP_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != record) && (NULL != container) )
    {
        status = ptx_NDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) PTX_NDEFRECORD_TYPE_RTD_WLCCAP, PTX_NDEFRECORD_TYPE_RTD_WLCCAP_LEN, NULL, 0u, (uint8_t*) container, sizeof(ptx_NDEFRecord_WLCCAP_t));
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_Parse (ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCCAP_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != record) && (NULL != container) )
    {
        if ( (record->Payload.Length >= sizeof(ptx_NDEFRecord_WLCCAP_t)) &&
             (0 == strncmp(record->Type.sData, PTX_NDEFRECORD_TYPE_RTD_WLCCAP, PTX_NDEFRECORD_TYPE_RTD_WLCCAP_LEN)) )
        {
            memcpy(container, record->Payload.Data, sizeof(ptx_NDEFRecord_WLCCAP_t));

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_SetProtVersion (ptx_NDEFRecord_WLCCAP_t *container, uint8_t majorVersion, uint8_t minorVersion)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->WLC_PROTOCOL_VER  = (uint8_t) (((majorVersion << PTX_NDEFRECORD_WLCCAP_PROTOCOLVERSION_MAJOR_POS) & PTX_NDEFRECORD_WLCCAP_PROTOCOLVERSION_MAJOR)
                                                  |  ((minorVersion << PTX_NDEFRECORD_WLCCAP_PROTOCOLVERSION_MINOR_POS) & PTX_NDEFRECORD_WLCCAP_PROTOCOLVERSION_MINOR));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_GetProtVersion (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *majorVersion, uint8_t *minorVersion)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != majorVersion) && (NULL != minorVersion) )
    {
        *majorVersion = (uint8_t) ((container->WLC_PROTOCOL_VER >> PTX_NDEFRECORD_WLCCAP_PROTOCOLVERSION_MAJOR_POS) & 0x0F);
        *minorVersion = (uint8_t) ((container->WLC_PROTOCOL_VER >> PTX_NDEFRECORD_WLCCAP_PROTOCOLVERSION_MINOR_POS) & 0x0F);

        status = PTX_STATUS_SUCCESS;
    }

    return status;

}

ptx_Status_t ptx_NDEFRecord_WLCCAP_SetMode (ptx_NDEFRecord_WLCCAP_t *container, ptx_NDEFRecord_WLCCAP_Mode_t mode)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        /* Clear current setting */
        container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG & ~PTX_NDEFRECORD_WLCCAP_CONFIG_MODEREQ);
        /* Set new value */
        container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG | ((mode << PTX_NDEFRECORD_WLCCAP_CONFIG_MODEREQ_POS) & PTX_NDEFRECORD_WLCCAP_CONFIG_MODEREQ));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_GetMode (ptx_NDEFRecord_WLCCAP_t *container, ptx_NDEFRecord_WLCCAP_Mode_t *mode)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != mode) )
    {
        *mode = ((container->WLC_CONFIG & PTX_NDEFRECORD_WLCCAP_CONFIG_MODEREQ) >> PTX_NDEFRECORD_WLCCAP_CONFIG_MODEREQ_POS);
        if (Mode_RFU == *mode)
        {
            status = PTX_STATUS_E_NDEF_PROTOCOL_ERROR;
        }
        else
        {
            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_SetNWtRetries (ptx_NDEFRecord_WLCCAP_t *container, uint8_t retries)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        /* Clear current setting */
        container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG & ~PTX_NDEFRECORD_WLCCAP_CONFIG_NWTRET);
        /* Set new value */
        container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG | ((retries << PTX_NDEFRECORD_WLCCAP_CONFIG_NWTRET_POS) & PTX_NDEFRECORD_WLCCAP_CONFIG_NWTRET));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_GetNWtRetries (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *retries)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != retries) )
    {
        *retries = ((container->WLC_CONFIG & PTX_NDEFRECORD_WLCCAP_CONFIG_NWTRET) >> PTX_NDEFRECORD_WLCCAP_CONFIG_NWTRET_POS);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_SetNegWait (ptx_NDEFRecord_WLCCAP_t *container, uint8_t negoWait)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        if (negoWait)
        {
            container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG | PTX_NDEFRECORD_WLCCAP_CONFIG_NEGWAIT);
        }
        else
        {
            container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG & ~PTX_NDEFRECORD_WLCCAP_CONFIG_NEGWAIT);
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_GetNegWait (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *negoWait)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != negoWait) )
    {
        *negoWait = (0 != (container->WLC_CONFIG & PTX_NDEFRECORD_WLCCAP_CONFIG_NEGWAIT) ? 1 : 0);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_SetRdConf (ptx_NDEFRecord_WLCCAP_t *container, uint8_t rdConf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        if (rdConf)
        {
            container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG | PTX_NDEFRECORD_WLCCAP_CONFIG_RDCONF);
        }
        else
        {
            container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG & ~PTX_NDEFRECORD_WLCCAP_CONFIG_RDCONF);
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_GetRdConf (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *rdConf)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != rdConf) )
    {
        *rdConf = container->WLC_CONFIG & PTX_NDEFRECORD_WLCCAP_CONFIG_RDCONF;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_SetCapWt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t capWt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        /* Clear current setting */
        container->CAP_WT_INT = (uint8_t)(container->CAP_WT_INT & ~PTX_NDEFRECORD_WLCCAP_CAPWT_INT);
        /* Set new value */
        container->CAP_WT_INT = (uint8_t)(container->CAP_WT_INT | ((capWt << PTX_NDEFRECORD_WLCCAP_CAPWT_INT_POS) & PTX_NDEFRECORD_WLCCAP_CAPWT_INT));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_GetCapWt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *capWt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != capWt) )
    {
        *capWt = ((container->CAP_WT_INT & PTX_NDEFRECORD_WLCCAP_CAPWT_INT) >> PTX_NDEFRECORD_WLCCAP_CAPWT_INT_POS);

        if (*capWt > PTX_NDEFRECORD_WLCCAP_CAPWT_INT_MAX)
        {
            *capWt = PTX_NDEFRECORD_WLCCAP_CAPWT_INT_MAX;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_GetCapWtMillis (ptx_NDEFRecord_WLCCAP_t *container, uint32_t *capWtMillis)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != capWtMillis)
    {
        uint8_t cap_wt;
        status = ptx_NDEFRecord_WLCCAP_GetCapWt(container, &cap_wt);

        if (PTX_STATUS_SUCCESS == status)
        {
            *capWtMillis = (1u << (cap_wt + 3u));
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_SetRdWtInt (ptx_NDEFRecord_WLCCAP_t *container, uint16_t rdWT)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        uint8_t RD_WT = (uint8_t)(rdWT / PTX_NDEFRECORD_WLCCAP_RDWT_INT_FACTOR);

        /* Clear current setting */
        container->NDEF_RD_WT = (uint8_t)(container->NDEF_RD_WT & ~PTX_NDEFRECORD_WLCCAP_RDWT_INT);
        /* Set new value */
        container->NDEF_RD_WT = (uint8_t)(container->NDEF_RD_WT | ((RD_WT << PTX_NDEFRECORD_WLCCAP_RDWT_INT_POS) & PTX_NDEFRECORD_WLCCAP_RDWT_INT));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_GetRdWtInt (ptx_NDEFRecord_WLCCAP_t *container, uint16_t *rdWT)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != rdWT) )
    {
        uint8_t RD_WT = ((container->NDEF_RD_WT & PTX_NDEFRECORD_WLCCAP_RDWT_INT) >> PTX_NDEFRECORD_WLCCAP_RDWT_INT_POS);
        if (RD_WT > PTX_NDEFRECORD_WLCCAP_RDWT_INT_MAX)
        {
            RD_WT = PTX_NDEFRECORD_WLCCAP_RDWT_INT_MAX;
        }

        *rdWT = (uint16_t)(RD_WT * PTX_NDEFRECORD_WLCCAP_RDWT_INT_FACTOR);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_SetWrToInt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t timeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        /* Clear current setting */
        container->NDEF_WR_TO_INT = (uint8_t)(container->NDEF_WR_TO_INT & ~PTX_NDEFRECORD_WLCCAP_WRTOINT_INT);
        /* Set new value */
        container->NDEF_WR_TO_INT = (uint8_t)(container->NDEF_WR_TO_INT | ((timeout << PTX_NDEFRECORD_WLCCAP_WRTOINT_INT_POS) & PTX_NDEFRECORD_WLCCAP_WRTOINT_INT));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_GetWrToInt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *timeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != timeout) )
    {
        *timeout = ((container->NDEF_WR_TO_INT & PTX_NDEFRECORD_WLCCAP_WRTOINT_INT) >> PTX_NDEFRECORD_WLCCAP_WRTOINT_INT_POS);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_SetWrWtInt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t waitingTime)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        /* Clear current setting */
        container->NDEF_WR_WT_INT = (uint8_t)(container->NDEF_WR_WT_INT & ~PTX_NDEFRECORD_WLCCAP_WRWTINT_INT);
        /* Set new value */
        container->NDEF_WR_WT_INT = (uint8_t)(container->NDEF_WR_WT_INT | ((waitingTime << PTX_NDEFRECORD_WLCCAP_WRWTINT_INT_POS) & PTX_NDEFRECORD_WLCCAP_WRWTINT_INT));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCAP_GetWrWtInt (ptx_NDEFRecord_WLCCAP_t *container, uint8_t *waitingTime)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != waitingTime) )
    {
        *waitingTime = ((container->NDEF_WR_WT_INT & PTX_NDEFRECORD_WLCCAP_WRWTINT_INT) >> PTX_NDEFRECORD_WLCCAP_WRWTINT_INT_POS);
        if (*waitingTime > PTX_NDEFRECORD_WLCCAP_WRWTINT_INT_MAX)
        {
            *waitingTime = PTX_NDEFRECORD_WLCCAP_WRWTINT_INT_MAX;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}
