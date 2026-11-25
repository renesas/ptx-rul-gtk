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
    Module      : NFC Data Exchange Format (NDEF)
    File        : ptx_NDEF.c

    Description : NFC Data Exchange Format (NDEF) controls the by NFC Forum defined message format to exchange
                  information between two NFC Forum devices.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_NDEF.h"

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
ptx_Status_t ptx_NDEF_Open (ptx_NDEF_t *ndef, ptx_NDEF_InitParams_t *initParams)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if ((NULL != ndef) && (NULL != initParams))
    {
        if ((NULL != initParams->Tx) && (0 != initParams->TxLen) &&
            (NULL != initParams->Rx) && (0 != initParams->RxLen) &&
            (NULL != initParams->WorkBuffer) && (0 != initParams->WorkBufferLen))
        {

            ndef->TxLen = initParams->TxLen;
            ndef->TxBuffer = initParams->Tx;
            ndef->RxLen = initParams->RxLen;
            ndef->RxBuffer = initParams->Rx;
            ndef->RxSize = initParams->RxLen;
            ndef->TxSize = initParams->TxLen;
            ndef->WorkBufferLen = initParams->WorkBufferLen;
            ndef->WorkBuffer = initParams->WorkBuffer;
            ndef->Rul = initParams->Rul;

            ndef->Type = ndef->Rul->CardRegistry.ActiveProtocol;

            switch (ndef->Type)
            {
                case ptx_RUL_CardProtocolType_T2T:
                {
#if !defined(PTX_DISABLE_NDEF_T2T)
                    (void)memset(&ndef->TagCtx.T2t, 0, sizeof(ptx_NDEF_T2T_t));

                    ptx_NDEF_T2T_InitParams_t tagInitParams;
                    (void)memset(&tagInitParams, 0x00, sizeof(ptx_NDEF_T2T_InitParams_t));

                    tagInitParams.RxBufferSize = ndef->RxLen;
                    tagInitParams.RxBuffer = ndef->RxBuffer;
                    tagInitParams.TxBufferSize = ndef->TxLen;
                    tagInitParams.TxBuffer = ndef->TxBuffer;
                    tagInitParams.Rul = ndef->Rul;
                    tagInitParams.ReadCCVariant = ReadCCVariant_Block_3_Default;

                    status = ptx_NDEF_T2T_Open(&ndef->TagCtx.T2t, &tagInitParams);
#else
                    status = PTX_STATUS_E_GEN_FEATURE_DISABLED;
#endif
                    break;
                }
                case ptx_RUL_CardProtocolType_T3T:
                {
#if !defined(PTX_DISABLE_NDEF_T3T)
                    (void)memset(&ndef->TagCtx.T3t, 0, sizeof(ptx_NDEF_T3T_t));

                    ptx_NDEF_T3T_InitParams_t tagInitParams;
                    (void)memset(&tagInitParams, 0x00, sizeof(ptx_NDEF_T3T_InitParams_t));

                    tagInitParams.RxBufferSize = ndef->RxLen;
                    tagInitParams.RxBuffer = ndef->RxBuffer;
                    tagInitParams.TxBufferSize = ndef->TxLen;
                    tagInitParams.TxBuffer = ndef->TxBuffer;
                    tagInitParams.Rul = ndef->Rul;

                    status = ptx_NDEF_T3T_Open(&ndef->TagCtx.T3t, &tagInitParams);
#else
                    status = PTX_STATUS_E_GEN_FEATURE_DISABLED;
#endif
                    break;
                }
                case ptx_RUL_CardProtocolType_T4T:
                {
#if !defined(PTX_DISABLE_NDEF_T4T)
                    (void)memset(&ndef->TagCtx.T4t, 0, sizeof(ptx_NDEF_T4T_t));

                    ptx_NDEF_T4T_InitParams_t tagInitParams;
                    (void)memset(&tagInitParams, 0x00, sizeof(ptx_NDEF_T4T_InitParams_t));

                    tagInitParams.RxBufferSize = ndef->RxLen;
                    tagInitParams.RxBuffer = ndef->RxBuffer;
                    tagInitParams.TxBufferSize = (uint32_t)(ndef->TxLen);
                    tagInitParams.TxBuffer = ndef->TxBuffer;
                    tagInitParams.Rul = ndef->Rul;

                    status = ptx_NDEF_T4T_Open(&ndef->TagCtx.T4t, &tagInitParams);
#else
                    status = PTX_STATUS_E_GEN_FEATURE_DISABLED;
#endif
                    break;
                }
                case ptx_RUL_CardProtocolType_T5T:
                {
#if !defined(PTX_DISABLE_NDEF_T5T)
                    (void)memset(&ndef->TagCtx.T5t, 0, sizeof(ptx_NDEF_T5T_t));

                    ptx_NDEF_T5T_InitParams_t tagInitParams;
                    (void)memset(&tagInitParams, 0x00, sizeof(ptx_NDEF_T5T_InitParams_t));

                    tagInitParams.RxBufferSize = (uint32_t)(ndef->RxLen);
                    tagInitParams.RxBuffer = ndef->RxBuffer;
                    tagInitParams.WorkBufferSize = (uint32_t)(ndef->WorkBufferLen);
                    tagInitParams.WorkBuffer = ndef->WorkBuffer;
                    tagInitParams.TxBufferSize = (uint32_t)(ndef->TxLen);
                    tagInitParams.TxBuffer = ndef->TxBuffer;
                    tagInitParams.Rul = ndef->Rul;
                    tagInitParams.isSelected = (ptx_NDEF_T5T_Mode_Selected == initParams->T5tMode) ? (1) : (0);
                    tagInitParams.isAddressed = (ptx_NDEF_T5T_Mode_NonAddressed == initParams->T5tMode) ? (0) : (1);

                    status = ptx_NDEF_T5T_Open(&ndef->TagCtx.T5t, &tagInitParams);
#else
                    status = PTX_STATUS_E_GEN_FEATURE_DISABLED;
#endif
                    break;
                }
                default:
                {
                    status = PTX_STATUS_E_NDEF_INVALID_TAG_TYPE;
                    break;
                }
            }

            if (PTX_STATUS_SUCCESS == status)
            {
                ndef->CompId = Comp_NDEF;
            }
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_Close (ptx_NDEF_t *ndef)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndef, Comp_NDEF))
    {
        status = PTX_STATUS_E_GEN_FEATURE_DISABLED;

        switch (ndef->Type)
        {
            case ptx_RUL_CardProtocolType_T2T:
#if !defined(PTX_DISABLE_NDEF_T2T)
                status = ptx_NDEF_T2T_Close(&ndef->TagCtx.T2t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T3T:
#if !defined(PTX_DISABLE_NDEF_T3T)
                status = ptx_NDEF_T3T_Close(&ndef->TagCtx.T3t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T4T:
#if !defined(PTX_DISABLE_NDEF_T4T)
                status = ptx_NDEF_T4T_Close(&ndef->TagCtx.T4t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T5T:
#if !defined(PTX_DISABLE_NDEF_T5T)
                status = ptx_NDEF_T5T_Close(&ndef->TagCtx.T5t);
#endif
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_STATE;
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_CheckMessage (ptx_NDEF_t *ndef)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndef, Comp_NDEF))
    {
        status = PTX_STATUS_E_GEN_FEATURE_DISABLED;

        switch (ndef->Type)
        {
            case ptx_RUL_CardProtocolType_T2T:
#if !defined(PTX_DISABLE_NDEF_T2T)
                status = ptx_NDEF_T2T_CheckMessage(&ndef->TagCtx.T2t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T3T:
#if !defined(PTX_DISABLE_NDEF_T3T)
                status = ptx_NDEF_T3T_CheckMessage(&ndef->TagCtx.T3t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T4T:
#if !defined(PTX_DISABLE_NDEF_T4T)
                status = ptx_NDEF_T4T_CheckMessage(&ndef->TagCtx.T4t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T5T:
#if !defined(PTX_DISABLE_NDEF_T5T)
                status = ptx_NDEF_T5T_CheckMessage(&ndef->TagCtx.T5t);
#endif
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_STATE;
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_ReadMessage (ptx_NDEF_t *ndef, uint8_t *msgBuffer, uint32_t *msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndef, Comp_NDEF))
    {
        status = PTX_STATUS_E_GEN_FEATURE_DISABLED;

        switch (ndef->Type)
        {
            case ptx_RUL_CardProtocolType_T2T:
#if !defined(PTX_DISABLE_NDEF_T2T)
                status = ptx_NDEF_T2T_ReadMessage(&ndef->TagCtx.T2t, msgBuffer, msgLen);
#endif
                break;
            case ptx_RUL_CardProtocolType_T3T:
#if !defined(PTX_DISABLE_NDEF_T3T)
                status = ptx_NDEF_T3T_ReadMessage(&ndef->TagCtx.T3t, msgBuffer, msgLen);
#endif
                break;
            case ptx_RUL_CardProtocolType_T4T:
#if !defined(PTX_DISABLE_NDEF_T4T)
                status = ptx_NDEF_T4T_ReadMessage(&ndef->TagCtx.T4t, msgBuffer, msgLen);
#endif
                break;
            case ptx_RUL_CardProtocolType_T5T:
#if !defined(PTX_DISABLE_NDEF_T5T)
                status = ptx_NDEF_T5T_ReadMessage(&ndef->TagCtx.T5t, msgBuffer, msgLen);
#endif
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_STATE;
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_WriteMessage (ptx_NDEF_t *ndef, uint8_t *msgBuffer, uint32_t msgLen)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndef, Comp_NDEF))
    {
        status = PTX_STATUS_E_GEN_FEATURE_DISABLED;

        switch (ndef->Type)
        {
            case ptx_RUL_CardProtocolType_T2T:
#if !defined(PTX_DISABLE_NDEF_T2T)
                status = ptx_NDEF_T2T_WriteMessage(&ndef->TagCtx.T2t, msgBuffer, msgLen);
#endif
                break;
            case ptx_RUL_CardProtocolType_T3T:
#if !defined(PTX_DISABLE_NDEF_T3T)
                status = ptx_NDEF_T3T_WriteMessage(&ndef->TagCtx.T3t, msgBuffer, msgLen);
#endif
                break;
            case ptx_RUL_CardProtocolType_T4T:
#if !defined(PTX_DISABLE_NDEF_T4T)
                status = ptx_NDEF_T4T_WriteMessage(&ndef->TagCtx.T4t, msgBuffer, msgLen);
#endif
                break;
            case ptx_RUL_CardProtocolType_T5T:
#if !defined(PTX_DISABLE_NDEF_T5T)
                status = ptx_NDEF_T5T_WriteMessage(&ndef->TagCtx.T5t, msgBuffer, msgLen);
#endif
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_STATE;
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_FormatTag (ptx_NDEF_t *ndef)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndef, Comp_NDEF))
    {
        status = PTX_STATUS_E_GEN_FEATURE_DISABLED;

        switch (ndef->Type)
        {
            case ptx_RUL_CardProtocolType_T2T:
#if !defined(PTX_DISABLE_NDEF_T2T)
                status = ptx_NDEF_T2T_FormatTag(&ndef->TagCtx.T2t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T3T:
#if !defined(PTX_DISABLE_NDEF_T3T)
                status = ptx_NDEF_T3T_FormatTag(&ndef->TagCtx.T3t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T4T:
#if !defined(PTX_DISABLE_NDEF_T4T)
                status = ptx_NDEF_T4T_FormatTag(&ndef->TagCtx.T4t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T5T:
#if !defined(PTX_DISABLE_NDEF_T5T)
                status = ptx_NDEF_T5T_FormatTag(&ndef->TagCtx.T5t);
#endif
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_STATE;
                break;
        }
    }

    return status;
}

ptx_Status_t ptx_NDEF_LockTag (ptx_NDEF_t *ndef)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(ndef, Comp_NDEF))
    {
        status = PTX_STATUS_E_GEN_FEATURE_DISABLED;

        switch (ndef->Type)
        {
            case ptx_RUL_CardProtocolType_T2T:
#if !defined(PTX_DISABLE_NDEF_T2T)
                status = ptx_NDEF_T2T_LockTag(&ndef->TagCtx.T2t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T3T:
#if !defined(PTX_DISABLE_NDEF_T3T)
                status = ptx_NDEF_T3T_LockTag(&ndef->TagCtx.T3t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T4T:
#if !defined(PTX_DISABLE_NDEF_T4T)
                status = ptx_NDEF_T4T_LockTag(&ndef->TagCtx.T4t);
#endif
                break;
            case ptx_RUL_CardProtocolType_T5T:
#if !defined(PTX_DISABLE_NDEF_T5T)
                status = ptx_NDEF_T5T_LockTag(&ndef->TagCtx.T5t);
#endif
                break;
            default:
                status = PTX_STATUS_E_GEN_INVALID_STATE;
                break;
        }
    }

    return status;
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS / CALLBACK
 * ####################################################################################################################
 */
