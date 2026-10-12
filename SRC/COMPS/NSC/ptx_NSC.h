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
    Module      : NFC Soft Controller (NSC) Common Interface Header
    File        : ptx_NSC.h

    Description : Contains the collection of all NFC Soft Controller Interface header files.
                  Since NSC interface is spread into several modules, by including this single header
                  complete NSC interface get exposed to the user.
*/

#if !defined(SRC_COMMON_SRC_COMPS_NSC_PTX_NSC_H_)
#define SRC_COMMON_SRC_COMPS_NSC_PTX_NSC_H_


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */


#if defined(__cplusplus)
extern "C" {
#endif

#include "ptx_NSC_Base.h"
#include "ptx_NSC_Int.h"
#include "ptx_NSC_Registers.h"
#include "ptx_NSC_CmdProcessor.h"
#include "ptx_NSC_Data.h"
#include "ptx_NSC_Firmware.h"
#include "ptx_NSC_InitHw.h"
#include "ptx_NSC_Interface.h"
#include "ptx_NSC_RfActivate.h"
#include "ptx_NSC_RfConfig.h"
#include "ptx_NSC_RfDeactivate.h"
#include "ptx_NSC_RfDiscover.h"
#include "ptx_NSC_RfSetParameter.h"
#include "ptx_NSC_SystemConfig.h"
#include "ptx_NSC_Error.h"
#include "ptx_NSC_RfSetParameter.h"
#include "ptx_NSC_StateMachine.h"
#include "ptx_NSC_RfTest.h"
#include "ptx_NSC_RevisionInfo.h"
#include "ptx_NSC_RfPresenceCheck.h"
#include "ptx_NSC_Standby.h"
#include "ptx_NSC_Misc.h"
#include "ptx_NSC_RfConfigVal.h"
#include "ptx_NSC_Wakeup.h"

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

