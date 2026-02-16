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
   Module      : Operating System Abstraction Layer (OSAL) Library Functions
   File        : ptx_OSAL_Ext_Lib.h

   Description : Contains an interface to open and interact with external libraries
                 regardless of the operating system.
*/

/**
 * \addtogroup OSAL
 *
 * @{
 */

#if !defined(PTX_OSAL_EXT_LIB_H)
#define PTX_OSAL_EXT_LIB_H

#include "ptx_OSAL.h"
#include <stdio.h>
#include <string.h>

#define MAX_LIB_NAME_LEN 256u             /**< Maximum Length of a Library that can be opened */

/**
 * \brief Library Wrapper Structure.
 */
typedef struct ptx_OSAL_Lib
{
    void               *LibH;               /**< Loaded library handle. Assigned in \ref ptx_OSAL_LibOpen. */
    uint8_t             IsLoaded;           /**< Library loaded successfully (1) or not (0). Assigned in \ref ptx_OSAL_LibOpen. */
    uint32_t            LastError;          /**< If library load in \ref ptx_OSAL_LibOpen fails, holds resulting Error code. */
} ptx_OSAL_Lib_t;


/**
 * \brief Open a dynamic load library.
 *
 * \param[in]       osalCtx         Pointer to an initialized OSAL component structure.
 * \param[in,out]   libH            Pointer to an allocated (\ref ptx_OSAL_LibOpen) library structure.
 * \param[in]       fileNameID      File name or ID, depending on the system.
 *
 * \return Status, indicating whether the operation was successful.
 *
 */
ptx_Status_t ptx_OSAL_LibOpen(ptx_OSAL_t *osalCtx, ptx_OSAL_Lib_t *libH, const char *fileNameID);

/**
 * \brief Close the dynamically loaded library.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[out]  libH            Pointer to a an allocated (\ref ptx_OSAL_LibOpen) library structure.
 *
 * \return Status, indicating whether the operation was successful.
 *
 */
ptx_Status_t ptx_OSAL_LibClose(ptx_OSAL_t *osalCtx, ptx_OSAL_Lib_t *libH);

/**
 * \brief Get a pointer to a library function.
 *
 * \param[in]   osalCtx         Pointer to an initialized OSAL component structure.
 * \param[out]  libH            Pointer to an allocated (\ref ptx_OSAL_LibOpen) library structure.
 * \param[out]  fnName          Name of the function to load.
 *
 * \return Pointer to a library function. NULL in case of error.
 *
 */
void *ptx_OSAL_GetLibFn(ptx_OSAL_t *osalCtx, ptx_OSAL_Lib_t *libH, const char *fnName);

#endif /* Guard */

/** @} */
