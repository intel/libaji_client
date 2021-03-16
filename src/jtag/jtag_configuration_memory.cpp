/****************************************************************************
 *   Copyright (c) 2001 by Intel Corporation                                *
 *   author: Cross, Colin                                                   *
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
// Filename:    jtag_configuration_memory.cpp
//
// Description: 
//
// Authors:     Colin Cross
//
//              Copyright (c) Altera Corporation 2000 - 2001
//              All rights reserved.
//
//END_MODULE_HEADER//////////////////////////////////////////////////////////

//START_ALGORITHM_HEADER/////////////////////////////////////////////////////
//
// Configuration data stored only in memory
//
//END_ALGORITHM_HEADER///////////////////////////////////////////////////////

// INCLUDE FILES ////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif

#ifndef INC_JTAG_PLATFORM_H
#include "jtag_platform.h"
#endif

#ifndef INC_JTAG_CONFIGURATION_MEMORY_H
#include "jtag_configuration_memory.h"
#endif

#ifndef INC_JTAG_COMMON_H
#include "jtag_common.h"
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
const char * AJI_CONFIGURATION_MEMORY::get_value(const char * key)
{
    STRING_MAP::const_iterator i;

    i = m_data.find(key);
    if (i == m_data.end())
        return NULL;
    else
        return (*i).second.c_str();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CONFIGURATION_MEMORY::set_value(const char * key, const char * value, bool createonly)
{
    bool ok = true;

    if (value != NULL)
    {
        if (createonly)
        {
            // Request to fail if key exists already
            STRING_MAP::iterator i = m_data.find(key);
            if (i != m_data.end())
                ok = false;
        }

        if (ok)
            m_data[key] = value;
    }
    else
    {
        STRING_MAP::iterator i = m_data.find(key);

        if (i != m_data.end())
            m_data.erase(i);
    }

    return ok;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CONFIGURATION_MEMORY::enumerate(DWORD * instance, char * value, DWORD valuemax)
{
    STRING_MAP::const_iterator i = m_data.begin();

    const int n_buffer_bytes = 64;
    char last[n_buffer_bytes + 1] = { 0 };
    char next[n_buffer_bytes + 1] = { 0 };
    DWORD j = 1;

    if (*instance == 0)
        last[0] = 0;
    else
    {
        DWORD k = *instance;
        for ( ; j < k && i != m_data.end() ; i++, j++)
            ;
        if (i == m_data.end())
        {
            *instance = 0;
            return false;
        }
        strncpy_s(last, n_buffer_bytes+1, (*i).first.c_str(), n_buffer_bytes);
        char * slash = strchr(last, '\\');
        if (slash != NULL)
            *slash = 0;
        i++;
        j++;
    }

    while (i != m_data.end())
    {
        const char * key = (*i).first.c_str();
        const char * slash = strchr(key, '\\');

        if (slash != NULL)
        {
            int n_bytes = static_cast<int> (slash-key) > n_buffer_bytes ? n_buffer_bytes : static_cast<int> (slash-key);
            strncpy_s(next, n_buffer_bytes+1, key, n_bytes);
            next[n_bytes] = 0;

            if (strcmp_(next, sizeof(next), last) != 0)
                break;
        }

        i++;
        j++;
    }

    if (i != m_data.end())
    {
        *instance = j;
        if (strnlen_s(next, n_buffer_bytes+1) >= valuemax)
            return false;
        strcpy_s(value, n_buffer_bytes+1, next);
        return true;
    }
    else
    {
        *instance = 0;
        return false;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CONFIGURATION_MEMORY::deletekey(const char * delkey)
{
    STRING_MAP::iterator i = m_data.begin();
    size_t len = strnlen_s(delkey, STRING_BUFFER_SIZE);

    while (i != m_data.end())
    {
        const char * key = (*i).first.c_str();

        if (memcmp_(key, STRING_BUFFER_SIZE, delkey, len) == 0 && (key[len] == 0 || key[len] == '\\'))
        {
            m_data.erase(i);
            i = m_data.begin();
        }
        else
            i++;
    }

    return true;
}

