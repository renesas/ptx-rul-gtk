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
    Module      : Common Module Interface for File Logging
    File        : ptx_Log_File.h

    Description : Logging-implementation for platforms supporting writing into Text-files.
                  Contains the type definition and configuration used in File logging mechanism.
*/

/**
 * \addtogroup LOG_FILE
 *
 * @{
 */

#if !defined(PTX_LOG_FILE_H)
#define PTX_LOG_FILE_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx_Status.h"
#include "ptx_Components.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

/**
 * PTX_LOG_FILE_DIRECT_WRITE_MODE is a compile option which continously writes Log-entries directly into a log-file in write-through mode i.e. no caching used.
 * If it's not set, the log-entries will be writen into an internal cache which gets written at the end when the Log-component gets closed/deinitialized.
 */

/**
 * \brief Number of log-entries (can be relatively large since file-loger is preferably used on Desktop-systems)
 */
#define  PTX_LOG_FILE_NR_LOG_CACHE_ENTRIES         (uint32_t)100000

/**
 * \brief Max. line length of a single entry (append 2 Bytes to print also Log-Level)
 */
#define  PTX_LOG_FILE_ENTRY_MAX_LENGTH             (uint32_t)1000

/**
 * \brief Platform Separator.
 */
#if defined(WIN32) || defined(_WIN32)
#define PTX_LOG_PLAT_SEPERATOR "\\"
#else
#define PTX_LOG_PLAT_SEPERATOR "/"
#endif

/**
 * \brief Max. Log-File Path String-length
 */
#define  PTX_LOG_FILE_MAX_PATH_LENGTH  256

/**
 * \brief Log-File Path (default = use current directory)
 */
#define  PTX_LOG_FILE_PATH             "."

/**
 * \brief Log-File Name
 */
#define  PTX_LOG_FILE_NAME             "LogOutput.txt"


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief Main Component Struture
 */
typedef struct ptx_Log_File
{
    void          *FileHandle;                                                                /**< Pointer to a file stream. */
    char          FilePath[PTX_LOG_FILE_MAX_PATH_LENGTH];                                     /**< file path */
    char          LogCache[PTX_LOG_FILE_NR_LOG_CACHE_ENTRIES][PTX_LOG_FILE_ENTRY_MAX_LENGTH]; /**< Log Cache */
    uint32_t      LogCacheIndex;                                                              /**< Log Cache Index Pointer */
    uint8_t       IsCacheOverflow;                                                            /**< Indicator that Log Cache Index Pointer reached its max. size */
} ptx_Log_File_t;

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
