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
    Module      : Host Interface Protocol (HIP) Internal Common Types and Definitions.
    File        : ptx_HIP_Int.h

    Description : Common Types and Definitions internally used for all Host Interface Protocol (HIP) modules.
*/

/**
 * \addtogroup HIP
 *
 * @{
 */

#if !defined(PTX_HIP_INT_H)
#define PTX_HIP_INT_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * \brief Swap the two adjacent bytes in an (u)int16_t with each other.
 *
 * The swap is done in place, the variable is overwritten with the new result.
 * The macro uses the __buildin_bswap16 macro from the compiler.
 *
 *    INPUT    ->    OUTPUT
 * +----+----+    +----+----+
 * | B0 | B1 | -> | B1 | B0 |
 * +----+----+    +----+----+
 *
 * \param[in,out]   x    (u)int16_t bytes to be swapped
 */
#define PTX_HIP_BSWAP16(x) x = __builtin_bswap16(x)

/**
 * \brief Swaps two adjacent bytes in an (u)int16_t in array x of size x
 *
 * The macro iterates over the array in x and swaps the two bytes the (u)int16_t.
 * The macro uses the \see PTX_HIP_BSWAP16 macro to swap the actual data.
 *
 * \param[in,out]   x      array of (u)int16_t
 * \param[in]       l      length of the array x
 */
#define PTX_HIP_BSWAP16_ARRAY(x,l)        \
    do {                                  \
        for (uint32_t i=0; i<(l); ++i)    \
        {                                 \
            PTX_HIP_BSWAP16(x[i]);        \
        }                                 \
    } while (1 == 0)

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief HIP Command Codes
 */
typedef enum ptx_HIP_Cmd
{
    HIP_Cmd_Rst      = 0x01, /**< Command Reset */
    HIP_Cmd_RSS      = 0x02, /**< Command Read System Status */
    HIP_Cmd_WrCMem   = 0x05, /**< Command Write Code Memory */
    HIP_Cmd_RdCMem   = 0x06, /**< Command Read Code Memory */
    HIP_Cmd_WrMsg    = 0x07, /**< Command Write Message */
    HIP_Cmd_RdMsg    = 0x08, /**< Command Read Message */
    HIP_Cmd_RdML     = 0x09, /**< Command Read Message Length */
    HIP_Cmd_WrDMem   = 0x0A, /**< Command Write Data Message */
    HIP_Cmd_RdDMem   = 0x0B, /**< Command Read Data Message*/

    HIP_Cmd_MAX      = 0x10, /**< Max Command, used for comparison only */
} ptx_HIP_Cmd_t;

/**
 * \brief HIP DFY Reset Reasons
 */
typedef enum ptx_HIP_Cmd_SysRst
{
    HIP_Cmd_SysRst_RstDisDfy   = 0x00, /**< System Reset, Disable Dragonfly */
    HIP_Cmd_SysRst_RstEnDfy    = 0x01, /**< System Reset, Enable Dragonfly */
    HIP_Cmd_SysRst_NoRstDisDfy = 0x02, /**< No System Reset, Disable Dragonfly */
    HIP_Cmd_SysRst_NoRstEnDfy  = 0x03, /**< No System Reset, Enable Dragonfly */

    HIP_Cmd_SysRst_MAX         = 0x04  /**< CMD MAX, used for comparison only */
} ptx_HIP_Cmd_SysRst_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */


#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
