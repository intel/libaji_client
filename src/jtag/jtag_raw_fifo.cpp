/****************************************************************************
 *   Copyright (c) 1997 by Intel Corporation                                *
 *   author: Draper, Andrew                                                 *
 *   SPDX-License-Identifier: MIT                                           *
 *                                                                          *
 *   Permission is hereby granted, free of charge, to any person obtaining  *
 *   a copy of this software and associated documentation files (the        *
 *   "Software"), to deal in the Software without restriction, including    *
 *   without limitation the rights to use, copy, modify, merge, publish,    *
 *   distribute, sublicense, and/or sell copies of the Software, and to     *
 *   permit persons to whom the Software is furnished to do so, subject to  *
 *   the following conditions:                                              *
 *                                                                          *
 *   The above copyright notice and this permission notice (including the   *
 *   next paragraph) shall be included in all copies or substantial         *
 *   portions of the Software.                                              *
 *                                                                          *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. *
 *   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   *
 *   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   *
 *   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      *
 *   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 *
 ****************************************************************************/

//START_MODULE_HEADER////////////////////////////////////////////////////////
//
// Filename:    ajicli_raw_fifo.cpp
//
// Description: 
//
// Authors:     Andrew Draper
//
//              Copyright (c) Altera Corporation 1997 - 1999
//              All rights reserved.
//
//END_MODULE_HEADER//////////////////////////////////////////////////////////

//START_ALGORITHM_HEADER/////////////////////////////////////////////////////
//
//
//END_ALGORITHM_HEADER///////////////////////////////////////////////////////
//

// INCLUDE FILES ////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif


#ifndef INC_CSTRING
#include <cstring>
#define INC_CSTRING
#endif

#ifndef INC_CSTDLIB
#include <cstdlib>
#define INC_CSTDLIB
#endif

#ifndef INC_GEN_STRING_SYS_H
#include "gen_string_sys.h"
#endif

#ifndef INC_JTAG_TCPLINK_H
#include "jtag_tcplink.h"
#endif

#ifndef INC_JTAG_RAW_FIFO_H
#include "jtag_raw_fifo.h"
#endif

const BYTE TXRAWFIFO::s_zeros[256] = { 0 };

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

TXRAWFIFO::TXRAWFIFO(TCPLINK * link, DWORD length, const BYTE * data)
    : TXFIFO(link), m_data(const_cast<BYTE *>(data)), m_autofree(false)
{
    m_ptr  = m_data;
    m_end  = m_data + length;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

TXRAWFIFO::~TXRAWFIFO(void)
{
    if (m_autofree)
        delete[] m_data;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
DWORD TXRAWFIFO::get_data_avail(DWORD max)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    max &= ~3;

    if (m_data == NULL && max > sizeof(s_zeros))
        max = sizeof(s_zeros);

    if (m_ptr + max > m_end)
        max = static_cast<DWORD> (m_end - m_ptr);

    return max;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
const BYTE * TXRAWFIFO::get_data(DWORD count)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(count > 0);
    AJI_DEBUG_ASSERT(m_ptr + count <= m_end);

    const BYTE * ptr = (m_data == NULL) ? s_zeros : m_ptr;
    m_ptr += count;

    return ptr;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool TXRAWFIFO::copy_data(void)
{
    if (m_autofree || m_data == NULL)
        return true;

    DWORD length = static_cast<DWORD> (m_end - m_data);

    BYTE * newdata = new BYTE[length];
    if (newdata == NULL)
        return false;

    memcpy_s(newdata, length, m_data, length);
    m_data = newdata;
    m_ptr  = newdata;
    m_end  = newdata + length;
    m_autofree = true;

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void TXRAWFIFO::invert_data(void)
{
    AJI_DEBUG_ASSERT(m_autofree);
    AJI_DEBUG_ASSERT(m_data != NULL);
    AJI_DEBUG_ASSERT(m_ptr == m_data);

    BYTE * ptr = m_data;
    for ( ; ptr < m_end ; ptr++)
        *ptr = static_cast<BYTE>(~*ptr);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
RXRAWFIFO::RXRAWFIFO(TCPLINK * link, DWORD length, BYTE * buffer)
//
// Description: Construct an object which can be used to receive raw data into
//              a memory buffer.  If 'buffer' is NULL then the FIFO allocates
//              the memory buffer and frees it on destruction, otherwise a 
//              buffer supplied by the caller is used.
//              If 'length' is non-zero then this is the amount of data which
//              is expected to arrive.  If it is zero then the amount of data
//              is unknown, the correct length will be specified in 
//              wait_for_data. 
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
    : RXFIFO(link), m_data(buffer), m_ok(true), m_autofree(m_data == NULL),
      m_length_known(length > 0)
{
    if (m_data == NULL && length > 0)
        m_data = static_cast<BYTE *>(malloc(length));

    m_ptr  = m_data;
    m_end  = m_data + length;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

RXRAWFIFO::~RXRAWFIFO(void)
{
    if (m_autofree && m_data != NULL)
        free(m_data);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool RXRAWFIFO::wait_for_data(DWORD length)
//
// Description: Wait until all the data has been received sucessfully.  If
//              length was not specified when the FIFO was created then it
//              must be specified here.
//
// Return:      true if the data has been received succesfully.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (!m_length_known)
    {
        m_length_known = true;
        if (length == 0)
            m_ok = false;
        else
        {
            if (m_data == NULL)
                m_data = m_ptr = static_cast<BYTE *>(malloc(length));
        }
        m_end = m_data + length;

        // If we already have this much data then deactivate
        if (m_ptr >= m_end)
            release_full_fifo();
    }

    while (m_ptr < m_end && m_ok)
        if (!RXFIFO::wait_for_data())
            return false;

    return m_ok;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
BYTE * RXRAWFIFO::get_receive_buffer(DWORD len)
//
// Description: Provide a location into which we can receive a buffer worth of
//              data.  Data will be received directly into our buffer.
//
// Return:      The location to copy into, or NULL if an error has occurred.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (m_ptr + len > m_end)
    {
        if (m_length_known)
            return NULL;
        else
        {
            DWORD used = static_cast<DWORD> (m_ptr - m_data);
            DWORD alloc = used + len;
            BYTE * newdata = static_cast<BYTE *>(realloc(m_data, alloc));
            if (newdata == NULL)
                return NULL;

            m_data = newdata;
            m_ptr  = newdata + used;
            m_end  = newdata + alloc;
        }
    }

    return m_ptr;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool RXRAWFIFO::receive_data(DWORD len)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (m_ptr + len > m_end && m_length_known)
        m_ok = false;
    else
    {
        m_ptr += len;
    }

    return m_ptr >= m_end && m_length_known;
}

