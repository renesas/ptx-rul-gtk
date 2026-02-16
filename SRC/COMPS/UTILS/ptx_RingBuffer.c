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
    File        : ptx_RingBuffer.c

    Description : Implementation of the ring buffer
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptx_RingBuffer.h"

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

void ptx_RingBuffer_Init(ptx_RingBuffer_t *ringBuffer, uint8_t *buffer, uint16_t size)
{
    ringBuffer->buffer = buffer;
    ringBuffer->size = size;
    ptx_RingBuffer_Reset(ringBuffer);
}

void ptx_RingBuffer_Write(ptx_RingBuffer_t *ringBuffer, uint8_t *data, uint16_t len)
{
    uint16_t len_cpy = len;
    uint8_t *data_cpy = data;
    while (0 < len_cpy)
    {
        ringBuffer->buffer[ringBuffer->write_ptr] = *data_cpy;
        data_cpy++;
        if (ptx_RingBuffer_IsFull(ringBuffer))
        {
            ++ringBuffer->read_ptr;
            ringBuffer->read_ptr %= ringBuffer->size;
        }

        ++ringBuffer->write_ptr;
        ringBuffer->write_ptr %= ringBuffer->size;

        --len_cpy;
    }
}

void ptx_RingBuffer_Read(ptx_RingBuffer_t *ringBuffer, uint8_t *data, uint16_t *len)
{

    uint16_t to_read;
    uint16_t available = ptx_RingBuffer_Size(ringBuffer);
    if (*len > available)
    {
        to_read = available;
    }
    else
    {
        to_read = *len;
    }

    *len = to_read;

    uint8_t *data_cpy = data;
    while (to_read > 0)
    {
        *data_cpy++ = ringBuffer->buffer[ringBuffer->read_ptr];
        ++ringBuffer->read_ptr;
        ringBuffer->read_ptr %= ringBuffer->size;
        --to_read;
    }
}

uint8_t ptx_RingBuffer_IsEmpty(ptx_RingBuffer_t *ringBuffer)
{
    return ((ringBuffer->write_ptr == ringBuffer->read_ptr) && !ptx_RingBuffer_IsFull(ringBuffer));
}

uint8_t ptx_RingBuffer_IsFull(ptx_RingBuffer_t *ringBuffer)
{
    return (((ringBuffer->write_ptr + 1) % ringBuffer->size) == ringBuffer->read_ptr);
}

void ptx_RingBuffer_Reset(ptx_RingBuffer_t *ringBuffer)
{
    ringBuffer->write_ptr = 0;
    ringBuffer->read_ptr = 0;
}

uint16_t ptx_RingBuffer_Size(ptx_RingBuffer_t *ringBuffer)
{
    int32_t size;

    if (!ptx_RingBuffer_IsFull(ringBuffer))
    {
        size = ringBuffer->write_ptr - ringBuffer->read_ptr;
        if (ringBuffer->write_ptr < ringBuffer->read_ptr)
        {
            size += ringBuffer->size;
        }
    }
    else
    {
        size = ringBuffer->size;
    }

    return (uint16_t) size;
}
