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
    Module      : NFC Data Exchange Format (NDEF) Wireless Charging Status and Information (WLCSTAI) Record
    File        : ptx_NDEFRecord_WLCSTAI.c

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Wireless Charging Status and Information (WLCSTAI) NDEF records.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NDEFRecord_WLCSTAI.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NDEFRecord_WLCSTAI_Init (ptx_NDEFRecord_WLCSTAI_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        memset(container, 0, sizeof(ptx_NDEFRecord_WLCSTAI_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_Create(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCSTAI_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != record) && (NULL != container) )
    {
        status = ptx_NDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) PTX_NDEFRECORD_TYPE_RTD_WLCSTAI, PTX_NDEFRECORD_TYPE_RTD_WLCSTAI_LEN, NULL, 0u, NULL, 0);

        uint8_t *payload_cpy = record->Payload.Data;
        *payload_cpy = container->CONTROL;
        payload_cpy++;

        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_BATLVL)
        {
            *payload_cpy = container->BATTERY_LEVEL;
            payload_cpy++;
        }
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_RECPWR)
        {
            *payload_cpy = container->RECEIVE_POWER;
            payload_cpy++;
        }
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_RECVLT)
        {
            *payload_cpy = container->RECEIVE_VOLTAGE;
            payload_cpy++;
        }
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_RECCUR)
        {
            *payload_cpy = container->RECEIVE_CURRENT;
            payload_cpy++;
        }
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_TEMPBAT)
        {
            *payload_cpy = ((uint8_t)container->TEMPERATURE_BATTERY);
            payload_cpy++;
        }
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_TEMPWLCL)
        {
            *payload_cpy = ((uint8_t)container->TEMPERATURE_WLCL);
            payload_cpy++;
        }
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_CTRL2)
        {
            *payload_cpy = container->CONTROL2;
            payload_cpy++;
        }

        if (0 != container->CONTROL2)
        {
            if (container->CONTROL2 & PTX_NDEFRECORD_WLCSTAI_CTRL2_BATVLT)
            {
                *payload_cpy = container->BATTERY_VOLTAGE;
                payload_cpy++;
            }
            if (container->CONTROL2 & PTX_NDEFRECORD_WLCSTAI_CTRL2_BATCUR)
            {
                *payload_cpy = container->BATTERY_CURRENT;
                payload_cpy++;
            }
        }

        record->Payload.Length = (uint32_t) (payload_cpy - record->Payload.Data);
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_Parse(ptx_NDEFRecord_t *record, ptx_NDEFRecord_WLCSTAI_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != record) && (NULL != container) )
    {
        if ( (record->Payload.Length > 1u) &&
             (record->Payload.Length < sizeof(ptx_NDEFRecord_WLCSTAI_t)) &&
             (0 == strncmp(record->Type.sData, PTX_NDEFRECORD_TYPE_RTD_WLCSTAI, PTX_NDEFRECORD_TYPE_RTD_WLCSTAI_LEN)))
        {
            memset(container, 0, sizeof(ptx_NDEFRecord_WLCSTAI_t));

            uint8_t *payload_cpy = record->Payload.Data;
            container->CONTROL = *payload_cpy;
            payload_cpy++;

            uint8_t *container_cpy = (uint8_t*)(container + 1);

            uint8_t mask = 0x01;
            for (uint8_t i = 0; i < 8; ++i)
            {
                if ((container->CONTROL & mask))
                {
                    *container_cpy = *payload_cpy;
                    payload_cpy++;
                }
                container_cpy++;
                mask = ((uint8_t) (mask << 1));
            }

            if (0 != container->CONTROL2)
            {
                mask = 0x01;
                container_cpy = &(container->BATTERY_VOLTAGE);
                for (uint8_t i = 0; i < 2; ++i)
                {
                    if ((container->CONTROL2 & mask))
                    {
                        *payload_cpy = *container_cpy;
                        payload_cpy++;
                    }
                    container_cpy++;
                    mask = ((uint8_t) (mask << 1));
                }
            }

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetBatteryLevel(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t percent)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | PTX_NDEFRECORD_WLCSTAI_CTRL1_BATLVL);
        container->BATTERY_LEVEL = percent;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetBatteryLevel(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *percent)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != percent) )
    {
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_BATLVL)
        {
            *percent = container->BATTERY_LEVEL;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetReceivePower(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t power)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | PTX_NDEFRECORD_WLCSTAI_CTRL1_RECPWR);
        container->RECEIVE_POWER = power;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetReceivePower(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *power)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != power) )
    {
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_RECPWR)
        {
            *power = container->RECEIVE_POWER;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetReceiveVoltage(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t voltage)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | PTX_NDEFRECORD_WLCSTAI_CTRL1_RECVLT);
        container->RECEIVE_VOLTAGE = voltage;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetReceiveVoltage(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *voltage)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != voltage) )
    {
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_RECVLT)
        {
            *voltage = container->RECEIVE_VOLTAGE;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetReceiveCurrent(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t current)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | PTX_NDEFRECORD_WLCSTAI_CTRL1_RECCUR);
        container->RECEIVE_CURRENT = current;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetReceiveCurrent(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *current)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != current) )
    {
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_RECCUR)
        {
            *current = container->RECEIVE_CURRENT;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetTemperatureBat(ptx_NDEFRecord_WLCSTAI_t *container, int8_t celsius)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | PTX_NDEFRECORD_WLCSTAI_CTRL1_TEMPBAT);
        container->TEMPERATURE_BATTERY = celsius;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetTemperatureBat(ptx_NDEFRecord_WLCSTAI_t *container, int8_t *celsius)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != celsius) )
    {
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_TEMPBAT)
        {
            *celsius = container->TEMPERATURE_BATTERY;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetTemperatureWlcl(ptx_NDEFRecord_WLCSTAI_t *container, int8_t celsius)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | PTX_NDEFRECORD_WLCSTAI_CTRL1_TEMPWLCL);
        container->TEMPERATURE_WLCL = celsius;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetTemperatureWlcl(ptx_NDEFRecord_WLCSTAI_t *container, int8_t *celsius)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != celsius) )
    {
        if (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_TEMPWLCL)
        {
            *celsius = container->TEMPERATURE_WLCL;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetBatteryVoltage(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t voltage)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | PTX_NDEFRECORD_WLCSTAI_CTRL1_CTRL2);
        container->CONTROL2 = (uint8_t)(container->CONTROL2 | PTX_NDEFRECORD_WLCSTAI_CTRL2_BATVLT);
        container->BATTERY_VOLTAGE = voltage;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetBatteryVoltage(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *voltage)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != voltage) )
    {
        if ( (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_CTRL2) &&
             (container->CONTROL2 & PTX_NDEFRECORD_WLCSTAI_CTRL2_BATVLT))
        {
            *voltage = container->BATTERY_VOLTAGE;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_SetBatteryCurrent(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t current)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | PTX_NDEFRECORD_WLCSTAI_CTRL1_CTRL2);
        container->CONTROL2 = (uint8_t)(container->CONTROL2 | PTX_NDEFRECORD_WLCSTAI_CTRL2_BATCUR);
        container->BATTERY_CURRENT = current;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_WLCSTAI_GetBatteryCurrent(ptx_NDEFRecord_WLCSTAI_t *container, uint8_t *current)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != current) )
    {
        if ( (container->CONTROL & PTX_NDEFRECORD_WLCSTAI_CTRL1_CTRL2) &&
             (container->CONTROL2 & PTX_NDEFRECORD_WLCSTAI_CTRL2_BATCUR))
        {
            *current = container->BATTERY_CURRENT;

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_NOT_FOUND;
        }
    }

    return status;
}
