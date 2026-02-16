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
    Module      : NFC Soft Controller (NSC) Interface for Reader Universal Library (RUL) Message Handling
    File        : ptx_NSC_RUL_MsgProcessing.c

    Description : Implementation of NFC Soft Controller Interface for Reader Universal Library Message Handling.
                  Contains function(s) for RUL-specific message handling.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_NSC_RUL_MsgProcessing.h"
#include "ptx_NSC_CmdProcessor.h"

#include "ptx_HAL.h"
/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define     PTX_NSC_RUL_NEW_MSG_RECEIVED        (1u)
#define     PTX_NSC_RUL_NEW_MSG_NOT_RECEIVED    (0)

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS DECLARATIONS / HELPERS
 * ####################################################################################################################
 */

/*
 * Routine to check if new message is pending and to retrieve this message if available.
 */
static uint8_t ptx_NSC_RUL_NewMsgReceived (ptx_NSC_t *nsc, uint8_t *msg, uint32_t *msgLen, ptx_Status_t *errStatus);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptx_Status_t ptx_NSC_RUL_FetchMsg (ptx_NSC_t *nsc, uint8_t *msg, uint32_t *msgLen, uint32_t timeout)
{
    ptx_Status_t status = PTX_STATUS_E_GEN_INVALID_PARAMETER;

    if (PTX_COMP_CHECK(nsc, Comp_NSC) && (NULL != msg) && (0 != *msgLen))
    {
        if (PTX_NSC_RUL_NEW_MSG_RECEIVED != ptx_NSC_RUL_NewMsgReceived (nsc, msg, msgLen, &status))
        {
            uint8_t timer_elapsed;
            uint8_t rx_irq_pending;

            status = ptx_HAL_Tmr_Start(nsc->HAL, HAL_Tmr_Id_0, HAL_DRV_Tmr_Unit_mSec, timeout);

            /* Wait for any kind of interrupt event only if the timer has been successfully started. */
            if (PTX_STATUS_SUCCESS == status)
            {
                timer_elapsed = PTX_HAL_DRV_TMR_IS_ELAPSED_FALSE;
                rx_irq_pending = 0;

                /*
                 * Wait for a period of time defined by timeout. Expect to get IRQ-line event or a timer-elapsed event.
                 * Any other events are disregarded.
                 */
                do
                {
                    (void)ptx_HAL_DisableIrq(nsc->HAL);

                    (void)ptx_HAL_Tmr_IsElapsed(nsc->HAL, HAL_Tmr_Id_0, &timer_elapsed);

                    (void)ptx_HAL_Interface_RxPending(nsc->HAL, &rx_irq_pending);

                    if ( (PTX_HAL_DRV_TMR_IS_ELAPSED_TRUE != timer_elapsed) && (0 == rx_irq_pending) )
                    {
                        (void)ptx_HAL_WaitForIrq(nsc->HAL);
                    }

                    (void)ptx_HAL_EnableIrq(nsc->HAL);

                }
                while ((PTX_HAL_DRV_TMR_IS_ELAPSED_TRUE != timer_elapsed) && (0 == rx_irq_pending));


                if (0 != rx_irq_pending)
                {
                    /* Get the message. Status is overwritten inside. */
                    if (PTX_NSC_RUL_NEW_MSG_RECEIVED != ptx_NSC_RUL_NewMsgReceived (nsc, msg, msgLen, &status))
                    {
                        /* */
                    }
                }
                else
                {
                    nsc->CmdProcessor.LastMsg.IsNewMsg = 0;
                    status = PTX_STATUS_E_GEN_TIMEOUT;
                }

                ptx_Status_t st = ptx_HAL_Tmr_Stop(nsc->HAL, HAL_Tmr_Id_0);

                /* Do not overwrite status if an error has already been set. */
                status = (PTX_STATUS_SUCCESS != status) ? status : st;
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

static uint8_t ptx_NSC_RUL_NewMsgReceived (ptx_NSC_t *nsc, uint8_t *msg, uint32_t *msgLen, ptx_Status_t *errStatus)
{
    /* Parameter checking done within the caller. All must be valid. */

    uint8_t ret_val = PTX_NSC_RUL_NEW_MSG_NOT_RECEIVED;

    nsc->CmdProcessor.LastMsg.IsNewMsg = 0;

    ptx_Status_t status = ptx_NSC_CmdProcessor_GetPendingMsg(nsc);
    if (PTX_STATUS_SUCCESS == status)
    {
        /* Check if we have a new message in the internal buffer. */
        if (1 == nsc->CmdProcessor.LastMsg.IsNewMsg)
        {
            uint32_t len = nsc->CmdProcessor.LastMsg.Length;

            /* Is the destination buffer large enough? */
            if (*msgLen >= len)
            {
                /* Copy the data from source to destination buffer. */
                memcpy(&msg[0], nsc->CmdProcessor.LastMsg.Payload, len);
                *msgLen = len;

                /* Reset "NewMsg"-flag so we can receive the next one. */
                nsc->CmdProcessor.LastMsg.IsNewMsg = 0;
            }
            else
            {
                /* The destination buffer is not large enough. */
                status = PTX_STATUS_E_NSC_RX_ERROR_LENGTH;
                *msgLen = 0;
            }

            ret_val = PTX_NSC_RUL_NEW_MSG_RECEIVED;
        }
    }

    *errStatus = status;

    return ret_val;
}
