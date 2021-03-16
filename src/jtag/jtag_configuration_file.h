/****************************************************************************
 *   Copyright (c) 2001 by Intel Corporation                                *
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
// Filename:    jtag_platform_unix.cpp
//
// Description: 
//
// Authors:     Andrew Draper
//
//              Copyright (c) Altera Corporation 2000 - 2001
//              All rights reserved.
//
//END_MODULE_HEADER//////////////////////////////////////////////////////////

//START_ALGORITHM_HEADER/////////////////////////////////////////////////////
//
// Platform specific functions for Unix
//
//END_ALGORITHM_HEADER///////////////////////////////////////////////////////

// INCLUDE FILES ////////////////////////////////////////////////////////////

#ifndef _JTAG_CONFIGURATION_FILE_H_
#define _JTAG_CONFIGURATION_FILE_H_

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif

#ifndef INC_JTAG_CONFIGURATION_H
#include "jtag_configuration.h"
#endif

class AJI_CONFIGURATION_FILE : AJI_CONFIGURATION
{
public:
    AJI_CONFIGURATION_FILE(bool server)
      : AJI_CONFIGURATION(server), m_config_filename(NULL), m_file_mtime(0), m_file_ctime(0),
        m_file(NULL) {}
    ~AJI_CONFIGURATION_FILE(void) { delete[] m_config_filename; }

    bool set_default_filename(void);
    void set_filename(const char * filename);
    const char * get_filename(void);

    int load(void);
    bool store(void);

    const char * get_value(const char * key);
    bool set_value(const char * key, const char * value, bool createonly);

    bool enumerate(DWORD * instance, char * value, DWORD valuemax);
    bool deletekey(const char * key);

private:
    int open_file(bool for_write);
    bool close_file(void) { bool ok = fclose(m_file) == 0; m_file = NULL; return ok; }

    class READER
    {
    public:
        READER(FILE * file) { m_file = file; }

        int token(char * buffer, unsigned int max);
        int read_char(bool in_string = false);

    private:
        FILE * m_file;
    };

    const char * m_config_filename;

    time_t m_file_mtime;
    time_t m_file_ctime;

    FILE * m_file;
};

#endif
