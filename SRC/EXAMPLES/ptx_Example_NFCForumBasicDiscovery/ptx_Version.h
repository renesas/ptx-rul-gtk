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
    Module      : SDK Example (NFC Forum Basic Discovery)
    File        : EXAMPLES/ptx_Example_NFCForumBasicDiscovery/ptx_Version.h

    Description : Version definition for the PTX205R Discovery application.
                  Uses semantic versioning (MAJOR.MINOR.PATCH format).
*/

#if !defined(_PTX_VERSION_H_)
#define _PTX_VERSION_H_

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * Semantic versioning: MAJOR.MINOR.PATCH
 * Update these values to change the application version.
 */
#define PTX_VERSION_MAJOR 0
#define PTX_VERSION_MINOR 0
#define PTX_VERSION_PATCH 0

/* Helper macros to stringify version components */
#define PTX_STRINGIFY(x) #x
#define PTX_XSTRINGIFY(x) PTX_STRINGIFY(x)

/* Full version string (e.g., "0.0.0") */
#define PTX_VERSION_STRING PTX_XSTRINGIFY(PTX_VERSION_MAJOR) "." PTX_XSTRINGIFY(PTX_VERSION_MINOR) "." PTX_XSTRINGIFY(PTX_VERSION_PATCH)

#if defined(__cplusplus)
}
#endif

#endif /* Guard */
