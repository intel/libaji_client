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
// Filename:    jtag_configuration_win.h
//
// Description: 
//
// Authors:     Andrew Draper
//
//              Copyright (c) Altera Corporation 2000 - 2001
//              All rights reserved.
//
//END_MODULE_HEADER//////////////////////////////////////////////////////////

// INCLUDE FILES ////////////////////////////////////////////////////////////

#ifndef _JTAG_CONFIGURATION_WIN_H_
#define _JTAG_CONFIGURATION_WIN_H_

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

#if PORT == WINDOWS
#ifndef INC_WINSOCK2_H
#include <winsock2.h>
#define INC_WINSOCK2_H
#endif

#ifndef INC_WINDOWS_H
#include <Windows.h>
#endif
#endif // PORT == WINDOWS

class AJI_CONFIGURATION_WIN : AJI_CONFIGURATION
{
public:
    AJI_CONFIGURATION_WIN(bool server);  //server is dummy variable for compatibility, set to false
    ~AJI_CONFIGURATION_WIN(void)  {}

    const char * get_value(const char * key);
    bool set_value(const char * key, const char * value, bool createonly);

    bool enumerate(DWORD * instance, char * value, DWORD valuemax);
    bool deletekey(const char * key);

    inline HKEY config_open_key(bool server);  //server is dummy variable for compatibility, set to false
private:
    const char * get_registry_value(const char * key);
    inline HKEY open_key(bool server);  //server is dummy variable for compatibility, set to false
    inline REGSAM get_regsam(REGSAM regsam_in);
    REGSAM m_regsam_modifier;
};

#endif
