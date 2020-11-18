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
// Filename:    jtag_configuration.h
//
// Description: 
//
// Authors:     Colin Cross
//
//              Copyright (c) Altera Corporation 2000 - 2001
//              All rights reserved.
//
//END_MODULE_HEADER//////////////////////////////////////////////////////////

// INCLUDE FILES ////////////////////////////////////////////////////////////

#ifndef _JTAG_CONFIGURATION_H_
#define _JTAG_CONFIGURATION_H_

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_MAP
#include <map>
#define MAP

#endif

#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif

#ifndef INC_GEN_STRING_SYS_H
#include "gen_string_sys.h"
#endif

#ifndef INC_STRNLEN_S_H
#include "strnlen_s.h"
#endif

const size_t STRING_BUFFER_SIZE = 512;

void config_set_filename(const char * filename);
const char* config_set_default_filename();
bool config_get_string(bool server, const char * key, char * value, DWORD valuemax);  //server is dummy variable for compatibility, set to false
bool config_set_string(bool server, const char * key, const char * value, bool createonly = false);  //server is dummy variable for compatibility, set to false

bool config_enumerate(bool server, DWORD * instance, char * value, DWORD valuemax); //server is dummy variable for compatibility, set to false
bool config_delete(bool server, const char * key); //server is dummy variable for compatibility, set to false
void config_set_memory(bool server); //server is dummy variable for compatibility, set to false

class AJI_CONFIGURATION
{
public:
    AJI_CONFIGURATION(bool server) {}  //server is dummy variable for compatibility, set to false
    virtual ~AJI_CONFIGURATION(void)  {}

    virtual const char * get_value(const char * key) = 0;
    virtual bool set_value(const char * key, const char * value, bool createonly) = 0;

    virtual bool enumerate(DWORD * instance, char * value, DWORD valuemax) = 0;
    virtual bool deletekey(const char * key) = 0;

protected:
    class STRING
    {
    public:
        STRING(void) { m_data = NULL; }
        STRING(const char * data) { set(data); }
        STRING(const STRING & other) { set(other.m_data); }
        ~STRING(void) { delete[] m_data; }

        void operator = (const STRING & other);
        void operator = (const char * data) { delete[] m_data; set(data); }

        bool operator < (const STRING & other) const;

        const char * c_str(void) const { return m_data; }

    private:
        char * m_data;

        void set(const char * data)
            { if (data != NULL) { m_data = new char[strnlen_s(data, STRING_BUFFER_SIZE)+1]; strcpy_s(m_data, STRING_BUFFER_SIZE, data); } else m_data = NULL; }
    };

    typedef std::map< STRING, STRING, std::less< STRING > > STRING_MAP;

    STRING_MAP m_data;
    bool m_server;  //server is dummy variable for compatibility, set to false
};

#endif
