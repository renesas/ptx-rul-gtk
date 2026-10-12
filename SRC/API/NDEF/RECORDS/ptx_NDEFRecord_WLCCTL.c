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
    Module      : NFC Data Exchange Format (NDEF) Wireless Charging Control (WLCCTL) Record
    File        : ptx_NDEFRecord_WLCCTL.c

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Wireless Charging Control (WLCCTL) NDEF records.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NDEFRecord_WLCCTL.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NDEFRecord_WLCCTL_Init (ptx_NDEFRecord_WLCCTL_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        memset(container, 0, sizeof(ptx_NDEFRecord_WLCCTL_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_Create(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCCTL_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != record) && (NULL != container) )
    {
        uint8_t len;
        (void) ptx_NDEFRecord_WLCCTL_GetLength(container, &len);
        status = ptx_NDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) PTX_NDEFRECORD_TYPE_RTD_WLCCTL, PTX_NDEFRECORD_TYPE_RTD_WLCCTL_LEN, NULL, 0, (uint8_t*) container, len);
    }

    return status;
}



ptx_Status_t ptx_NDEFRecord_WLCCTL_Parse(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCCTL_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != record) && (NULL != container) )
    {
        if (0 == strncmp(record->Type.sData, PTX_NDEFRECORD_TYPE_RTD_WLCCTL, PTX_NDEFRECORD_TYPE_RTD_WLCCTL_LEN))
        {
            size_t recordLength = sizeof(ptx_NDEFRecord_WLCCTL_t) -1;

            /* Only copy the last optional byte of the record if it exists. */
            if (record->Payload.Length == sizeof(ptx_NDEFRecord_WLCCTL_t))
            {
                recordLength = sizeof(ptx_NDEFRecord_WLCCTL_t);
            }

            memset(container, 0, sizeof(ptx_NDEFRecord_WLCCTL_t));
            memcpy(container, record->Payload.Data, recordLength);

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetLength(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *len)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != len)
    {
        uint8_t hasErrorField;
        uint8_t containerLen = sizeof(ptx_NDEFRecord_WLCCTL_t);

        status = ptx_NDEFRecord_WLCCTL_GetErrorFlag(container, &hasErrorField);
        if (PTX_STATUS_SUCCESS == status)
        {
            if (!hasErrorField)
            {
                containerLen = (uint8_t)(containerLen - sizeof(((ptx_NDEFRecord_WLCCTL_t *)0)->ERROR_INFO));
            }
            *len = containerLen;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetErrorFlag(ptx_NDEFRecord_WLCCTL_t *container, uint8_t error)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        if (error)
        {
            container->STATUS_INFO = (uint8_t)(container->STATUS_INFO | PTX_NDEFRECORD_WLCCTL_STATUSINFO_ERRORFLAG);
        }
        else
        {
            container->STATUS_INFO = (uint8_t)(container->STATUS_INFO & ~PTX_NDEFRECORD_WLCCTL_STATUSINFO_ERRORFLAG);
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetErrorFlag(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *error)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != error) )
    {
        *error = (container->STATUS_INFO & PTX_NDEFRECORD_WLCCTL_STATUSINFO_ERRORFLAG);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetBatteryStatus(ptx_NDEFRecord_WLCCTL_t *container, ptx_NDEFRecord_WLCCTL_BatteryStatus_t batteryStatus)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->STATUS_INFO = (uint8_t)(container->STATUS_INFO & ~PTX_NDEFRECORD_WLCCTL_STATUSINFO_BATTERYSTATUS);
        container->STATUS_INFO = (uint8_t)(container->STATUS_INFO | (batteryStatus << PTX_NDEFRECORD_WLCCTL_STATUSINFO_BATTERYSTATUS_POS));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetBatteryStatus(ptx_NDEFRecord_WLCCTL_t *container, ptx_NDEFRecord_WLCCTL_BatteryStatus_t *batteryStatus)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != batteryStatus) )
    {
        *batteryStatus = ((container->STATUS_INFO & PTX_NDEFRECORD_WLCCTL_STATUSINFO_BATTERYSTATUS) >> PTX_NDEFRECORD_WLCCTL_STATUSINFO_BATTERYSTATUS_POS);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetCounter(ptx_NDEFRecord_WLCCTL_t *container, uint8_t counter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->STATUS_INFO = (uint8_t)(container->STATUS_INFO & ~PTX_NDEFRECORD_WLCCTL_STATUSINFO_COUNTER);
        container->STATUS_INFO = (uint8_t)(container->STATUS_INFO | (counter & PTX_NDEFRECORD_WLCCTL_STATUSINFO_COUNTER));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetCounter(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *counter)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != counter) )
    {
        *counter = (container->STATUS_INFO & PTX_NDEFRECORD_WLCCTL_STATUSINFO_COUNTER);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetWptReq(ptx_NDEFRecord_WLCCTL_t *container, uint8_t wptRequest)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        if (wptRequest)
        {
            container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG | (0x01 << PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTREQ_POS));
        }
        else
        {
            container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG & ~PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTREQ);
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetWptReq(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *wptRequest)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != wptRequest) )
    {
        *wptRequest = container->WPT_CONFIG & PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTREQ;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetWptDuration(ptx_NDEFRecord_WLCCTL_t *container, uint8_t duration)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG & ~PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTDURATION);
        container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG | ((duration << PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTDURATION_POS) & PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTDURATION));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetWptDuration(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *duration)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != duration) )
    {
        *duration = ((container->WPT_CONFIG & PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTDURATION) >> PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTDURATION_POS);

        if (*duration > PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTDURATION_MAX)
        {
            *duration = PTX_NDEFRECORD_WLCCTL_WPTCONFIG_WPTDURATION_MAX;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetWptDurationMillis(ptx_NDEFRecord_WLCCTL_t *container, uint32_t *wptDurationMs)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != wptDurationMs)
    {
        uint8_t wpt_duration;
        status = ptx_NDEFRecord_WLCCTL_GetWptDuration(container, &wpt_duration);

        if (PTX_STATUS_SUCCESS == status)
        {
            *wptDurationMs = 1u << (wpt_duration + 3u);
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetWptInfoReq(ptx_NDEFRecord_WLCCTL_t *container, uint8_t request)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        if (request)
        {
            container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG | PTX_NDEFRECORD_WLCCTL_WPTCONFIG_INFOREQ);
        }
        else
        {
            container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG & ~PTX_NDEFRECORD_WLCCTL_WPTCONFIG_INFOREQ);
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetWptInfoReq(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *request)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != request) )
    {
        *request = (container->WPT_CONFIG & PTX_NDEFRECORD_WLCCTL_WPTCONFIG_INFOREQ);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetPowerAdjReq(ptx_NDEFRecord_WLCCTL_t *container, int8_t adjustLevel)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->POWER_ADJ_REQ = adjustLevel;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetPowerAdjReq(ptx_NDEFRecord_WLCCTL_t *container, int8_t *adjustLevel)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != adjustLevel) )
    {
        *adjustLevel = container->POWER_ADJ_REQ;

        if ( (*adjustLevel > PTX_NDEFRECORD_WLCCTL_PWRADJREQ_MAX) || (*adjustLevel < PTX_NDEFRECORD_WLCCTL_PWRADJREQ_MIN) )
        {
            *adjustLevel = PTX_NDEFRECORD_WLCCTL_PWRADJREQ_DEFAULT;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetBatteryLevel(ptx_NDEFRecord_WLCCTL_t *container, uint8_t batteryLevel)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->BATTERY_LEVEL = batteryLevel;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetBatteryLevel(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *batteryLevel)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != batteryLevel) )
    {
        *batteryLevel = container->BATTERY_LEVEL;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetDvrFlag(ptx_NDEFRecord_WLCCTL_t *container, ptx_NDEFRecord_WLCCTL_DvrSupport_t support)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->DVR_INFO = (uint8_t)(container->DVR_INFO & ~PTX_NDEFRECORD_WLCCTL_DVRINFO_FLAG);
        container->DVR_INFO = (uint8_t)(container->DVR_INFO | (support << PTX_NDEFRECORD_WLCCTL_DVRINFO_FLAG_POS));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetDvrFlag(ptx_NDEFRecord_WLCCTL_t *container, ptx_NDEFRecord_WLCCTL_DvrSupport_t *support)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != support) )
    {
        *support = ((container->DVR_INFO & PTX_NDEFRECORD_WLCCTL_DVRINFO_FLAG) >> PTX_NDEFRECORD_WLCCTL_DVRINFO_FLAG_POS);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetDvrValue(ptx_NDEFRecord_WLCCTL_t *container, uint8_t currDvr)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->DVR_INFO = (uint8_t)(container->DVR_INFO & ~PTX_NDEFRECORD_WLCCTL_DVRINFO_VALUE);
        container->DVR_INFO = (uint8_t)(container->DVR_INFO | (currDvr & PTX_NDEFRECORD_WLCCTL_DVRINFO_VALUE));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetDvrValue(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *currDvr)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != currDvr) )
    {
        *currDvr = (container->DVR_INFO & PTX_NDEFRECORD_WLCCTL_DVRINFO_VALUE);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetHoldOffWt(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *holdOffWt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != holdOffWt) )
    {
        *holdOffWt = container->HOLD_OFF_WT_INT;

        if (*holdOffWt > PTX_NDEFRECORD_WLCCTL_HOLDOFFWT_MAX)
        {
            *holdOffWt = PTX_NDEFRECORD_WLCCTL_HOLDOFFWT_MAX;
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetHoldOffWtMillis(ptx_NDEFRecord_WLCCTL_t *container, uint32_t *holdOffWtMillis)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != holdOffWtMillis)
    {
        uint8_t hold_off_wt;
        status = ptx_NDEFRecord_WLCCTL_GetHoldOffWt(container, &hold_off_wt);

        if (PTX_STATUS_SUCCESS == status)
        {
            *holdOffWtMillis = (uint32_t) (hold_off_wt << 1u);
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetHoldOffWt(ptx_NDEFRecord_WLCCTL_t *container, uint8_t holdOffWt)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->HOLD_OFF_WT_INT = holdOffWt;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetErrorInfoOvTemp(ptx_NDEFRecord_WLCCTL_t *container, uint8_t ovTemp)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        if (ovTemp)
        {
            container->ERROR_INFO = (uint8_t)(container->ERROR_INFO | PTX_NDEFRECORD_WLCCTL_ERRINFO_TEMP);
        }
        else
        {
            container->ERROR_INFO = (uint8_t)(container->ERROR_INFO & ~PTX_NDEFRECORD_WLCCTL_ERRINFO_TEMP);
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetErrorInfoOvTemp(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *ovTemp)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != ovTemp) )
    {
        *ovTemp = (container->ERROR_INFO & PTX_NDEFRECORD_WLCCTL_ERRINFO_TEMP);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_SetErrorInfoProtocol(ptx_NDEFRecord_WLCCTL_t *container, uint8_t protError)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        if (protError)
        {
            container->ERROR_INFO = (uint8_t)(container->ERROR_INFO | PTX_NDEFRECORD_WLCCTL_ERRINFO_PROT);
        }
        else
        {
            container->ERROR_INFO = (uint8_t)(container->ERROR_INFO & ~PTX_NDEFRECORD_WLCCTL_ERRINFO_PROT);
        }

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetErrorInfoProtocol(ptx_NDEFRecord_WLCCTL_t *container, uint8_t *protError)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != protError) )
    {
        *protError = (container->ERROR_INFO & PTX_NDEFRECORD_WLCCTL_ERRINFO_PROT);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCCTL_GetErrorInfoProprietary(ptx_NDEFRecord_WLCCTL_t *container, ptx_NDEFRecord_WLCCTL_Proprietary_ErrorInfo_t *err)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != err) )
    {
        *err = (container->ERROR_INFO & PTX_NDEFRECORD_WLCCTL_ERRORINFO_PROP_MASK);

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}
