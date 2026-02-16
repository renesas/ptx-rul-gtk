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
    Module      : ptx_RingBuffer
    File        : ptx_RingBuffer.h

    Description : RingBuffer utility
*/

/**
 * \addtogroup ptx_RingBuffer
 * @{
 */


#if !defined(PTX_RINGBUFFER_H)
#define PTX_RINGBUFFER_H

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif


/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */


/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */

/**
 * \brief  Main Component Structure
 *
 */
typedef struct ptx_RingBuffer
{
    uint8_t *buffer;        /**< Ring buffer buffer */
    uint16_t write_ptr;     /**< Ring buffer write pointer */
    uint16_t read_ptr;      /**< Ring buffer read pointer */
    uint16_t size;          /**< Ring buffer size */
} ptx_RingBuffer_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Ring buffer Init
 *
 * Initialize a Ring buffer on an existing regular buffer
 *
 * \param[in]       ringBuffer      Ring buffer component structure
 * \param[in]       buffer          Buffer to be used as a Ring Buffer
 * \param[in]       size            Size of the buffer that is to be used
 */
void ptx_RingBuffer_Init(ptx_RingBuffer_t *ringBuffer, uint8_t *buffer, uint16_t size);

/**
 * \brief Ring buffer Write
 *
 * Write data to the ring buffer
 *
 * \param[in]       ringBuffer      Ring buffer component structure
 * \param[in]       data            Buffer of data to be written to the Ring Buffer
 * \param[in]       len             Size of the buffer that is to be written
 */
void ptx_RingBuffer_Write(ptx_RingBuffer_t *ringBuffer, uint8_t *data, uint16_t len);

/**
 * \brief Ring buffer Read
 *
 * Read data from the ring buffer to the given array
 *
 * \param[in]       ringBuffer      Ring buffer component structure
 * \param[in]       data            Array where the data from the buffer is to be stored
 * \param[in]       len             Number of bytes to read
 */
void ptx_RingBuffer_Read(ptx_RingBuffer_t *ringBuffer, uint8_t *data, uint16_t *len);

/**
 * \brief Ring buffer IsEmpty
 *
 * Checks if the ring buffer is empty
 *
 * \param[in]       ringBuffer      Ring buffer component structure
 *
 * \return 1 if the ring buffer is empty, else 0
 */
uint8_t ptx_RingBuffer_IsEmpty(ptx_RingBuffer_t *ringBuffer);

/**
 * \brief Ring buffer IsFull
 *
 * Checks if the ring buffer is full
 *
 * \param[in]       ringBuffer      Ring buffer component structure
 *
 * \return 1 if the ring buffer is full, else 0
 */
uint8_t ptx_RingBuffer_IsFull(ptx_RingBuffer_t *ringBuffer);

/**
 * \brief Ring buffer Reset
 *
 * Clears the ring buffer
 *
 * \param[in]       ringBuffer      Ring buffer component structure
 */
void ptx_RingBuffer_Reset(ptx_RingBuffer_t *ringBuffer);

/**
 * \brief Ring buffer Size
 *
 * Calculates the number of bytes stored in the ring buffer
 *
 * \param[in]       ringBuffer      Pointer to the ring buffer component structure
 *
 * \return Number of bytes in the ring bufer
 */
uint16_t ptx_RingBuffer_Size(ptx_RingBuffer_t *ringBuffer);

#if defined(__cplusplus)
}
#endif

#endif /* Guard */

/** @} */
