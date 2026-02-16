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
    Module      : NFC Data Exchange Format (NDEF) Device Information (DI) Record
    File        : ptx_NDEFRecord_DI.c

    Description : NFC Data Exchange Format (NDEF) implementation to create and parse Device Information (DI) NDEF records.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NDEFRecord_DI.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptx_Status_t ptx_NDEFRecord_Di_Init (ptx_NDEFRecord_DI_t *container)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (NULL != container)
    {
        memset(container, 0, sizeof(ptx_NDEFRecord_DI_t));

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_Di_Create (ptx_NDEFRecord_t *record, ptx_NDEFRecord_DI_t *container, uint8_t *payloadBuffer)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t length;
    uint8_t *ptr = payloadBuffer;

    if ( (record != NULL) && (container != NULL) && (payloadBuffer != NULL) ) /* check if source and destination are valid */
    {
        if ( (container->ManufacturerName.Length > 0) && (container->ModelName.Length > 0) ) /* check if mandatory parameters are specified */
        {
            /* Prepare NDEF Record Header */
            ptx_NDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) PTX_NDEFRECORD_TYPE_RTD_DEVICE_INFO, PTX_NDEFRECORD_TYPE_RTD_DEVICE_INFO_LEN, NULL, 0, NULL, 0);

            *ptr = DI_Type_ManufacturerName;                    /* specify type */
            ptr++;
            length = container->ManufacturerName.Length;
            *ptr = length;                                      /* specify length */
            ptr++;
            strncpy((char *) ptr, container->ManufacturerName.Value, length); /* copy value into new memory region */
            ptr += length;

            *ptr = DI_Type_ModelName;                           /* specify type */
            ptr++;
            length = container->ModelName.Length;               /* specify length */
            *ptr = length;
            ptr++;
            strncpy((char *) ptr, container->ModelName.Value, length); /* copy value into new memory region */
            ptr += length;


            if (0 < container->DeviceUniqueName.Length)        /* check if optional device unique name is specified */
            {
                /* Create datastructure for device unique name within available memory area (TLV) */
                *ptr = DI_Type_DeviceUniqueName;                /* specify type */
                ptr++;
                length = container->DeviceUniqueName.Length;
                *ptr = length;                                  /* specify length */
                ptr++;
                strncpy((char *) ptr, container->DeviceUniqueName.Value, length); /* copy value into new memory region */
                ptr += length;
            }

            if (0 < container->Uuid.Length)                    /* check if optional device unique name is specified */
            {
                /* Create datastructure for UUID within available memory area (TLV) */
                *ptr = DI_Type_UUID;                            /* specify type */
                ptr++;
                length = container->Uuid.Length;               /* UUID has fixed length of 16 byte */
                *ptr = length;                                  /* specify length */
                ptr++;
                memcpy(ptr, container->Uuid.Value, length);    /* copy value into new memory region */
                ptr += length;
            }

            if (0 < container->FirmwareVersion.Length)        /* check if optional device unique name is specified */
            {
                /* Create datastructure for UUID version */
                *ptr = DI_Type_FirmwareVersion;                 /* specify type */
                ptr++;
                length = container->FirmwareVersion.Length;
                *ptr = length;                                  /* specify length */
                ptr++;
                strncpy((char *) ptr, container->FirmwareVersion.Value, length); /* copy value into new memory region */
                ptr += length;
            }
            record->Payload.Data = payloadBuffer;
            record->Payload.Length = (uint32_t) (ptr - payloadBuffer);

            status = PTX_STATUS_SUCCESS;
        }
        else
        {
            status = PTX_STATUS_E_GEN_INTERNAL;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_Di_Parse (ptx_NDEFRecord_t *srcRecord, ptx_NDEFRecord_DI_t *dstRecord)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    uint8_t manufacturer_name_set = 0;
    uint8_t model_name_set = 0;

    if ( (NULL != srcRecord) && (NULL != srcRecord->Payload.Data) && (NULL != dstRecord) )
    {
        uint8_t *ptr = srcRecord->Payload.Data;
        while ( ((uint32_t)(ptr - srcRecord->Payload.Data)) < srcRecord->Payload.Length )
        {
            ptx_NDEFRecord_DI_type_t type = *ptr;
            ptr++;
            switch (type)
            {
                case DI_Type_ManufacturerName:
                    dstRecord->ManufacturerName.Length = *ptr;
                    ptr++;
                    dstRecord->ManufacturerName.Value = (char*) ptr;
                    ptr += dstRecord->ManufacturerName.Length;
                    manufacturer_name_set = 1;
                    break;

                case DI_Type_ModelName:
                    dstRecord->ModelName.Length = *ptr;
                    ptr++;
                    dstRecord->ModelName.Value = (char*) ptr;
                    ptr += dstRecord->ModelName.Length;
                    model_name_set = 1;
                    break;

                case DI_Type_DeviceUniqueName:
                    dstRecord->DeviceUniqueName.Length = *ptr;
                    ptr++;
                    dstRecord->DeviceUniqueName.Value = (char*) ptr;
                    ptr += dstRecord->DeviceUniqueName.Length;
                    break;

                case DI_Type_UUID:
                    dstRecord->Uuid.Length = *ptr;
                    ptr++;
                    dstRecord->Uuid.Value = ptr;
                    ptr += dstRecord->Uuid.Length;
                    break;

                case DI_Type_FirmwareVersion:
                    dstRecord->FirmwareVersion.Length = *ptr;
                    ptr++;
                    dstRecord->FirmwareVersion.Value = (char*) ptr;
                    ptr += dstRecord->FirmwareVersion.Length;
                    break;

                default: /* Ignore TLV elements that have type codes we don't support. */
                {
                    uint8_t length = *ptr;
                    ptr += (length + 1u);
                    break;
                }
            }
        }

        status = PTX_STATUS_SUCCESS;
    }

    if ( (0 == manufacturer_name_set) || (0 == model_name_set) ) /* check if mandatory parameters are specified */
    {
        status = PTX_STATUS_E_GEN_INVALID_PARAMETER;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_Di_SetManufacturerName (ptx_NDEFRecord_DI_t *container, char *manufacturerName)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != manufacturerName) )
    {
        uint32_t len = (uint32_t)strlen(manufacturerName);
        if (len < PTX_NDEFRECORD_DI_MAX_STRLEN)
        {
            container->ManufacturerName.Value = manufacturerName;
            container->ManufacturerName.Length = (uint8_t) len;

            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_Di_SetModelName (ptx_NDEFRecord_DI_t *container, char *modelName)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != modelName) )
    {
        uint32_t len = (uint32_t)strlen(modelName);
        if (len < PTX_NDEFRECORD_DI_MAX_STRLEN)
        {
            container->ModelName.Value = modelName;
            container->ModelName.Length = (uint8_t) len;

            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_Di_SetDeviceUniqueName (ptx_NDEFRecord_DI_t *container, char *deviceUniqueName)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != deviceUniqueName) )
    {
        uint32_t len = (uint32_t)strlen(deviceUniqueName);
        if (len < PTX_NDEFRECORD_DI_MAX_STRLEN)
        {
            container->DeviceUniqueName.Value = deviceUniqueName;
            container->DeviceUniqueName.Length = (uint8_t) len;

            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_Di_SetUUID (ptx_NDEFRecord_DI_t *container, uint8_t uuid[16])
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != uuid) )
    {
        container->Uuid.Value = uuid;
        container->Uuid.Length = (uint8_t) 16;

        status = PTX_STATUS_SUCCESS;
    }

    return status;
}

ptx_Status_t ptx_NDEFRecord_Di_SetFirmwareVersion (ptx_NDEFRecord_DI_t *container, char *firmwareVersion)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ( (NULL != container) && (NULL != firmwareVersion) )
    {
        uint32_t len = (uint32_t)strlen(firmwareVersion);
        if (len < PTX_NDEFRECORD_DI_MAX_STRLEN)
        {
            container->FirmwareVersion.Value = firmwareVersion;
            container->FirmwareVersion.Length = (uint8_t) len;

            status = PTX_STATUS_SUCCESS;
        }
    }

    return status;
}
