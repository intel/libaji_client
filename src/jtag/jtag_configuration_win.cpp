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
// Filename:    jtag_configuration_win.cpp
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

#ifndef INCLUDE_AJI_H
#include "aji_sys.h"
#endif

#ifndef INC_JTAG_PLATFORM_H
#include "jtag_platform.h"
#endif

#ifndef INC_JTAG_CONFIGURATION_H
#include "jtag_configuration_win.h"
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_CONFIGURATION_WIN::AJI_CONFIGURATION_WIN
(
    bool server
) :
    AJI_CONFIGURATION(server),
    m_regsam_modifier(0)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
#if !defined(_WIN64) && defined(_WIN32)
    m_regsam_modifier |= (is_32bit_app_on_64bit_kernel() ? KEY_WOW64_64KEY : 0);
#endif
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
const char * AJI_CONFIGURATION_WIN::get_value(const char * key)
{
    STRING_MAP::const_iterator i;

    return get_registry_value(key);
}

const char * AJI_CONFIGURATION_WIN::get_registry_value(const char * key)
{
    HKEY aji_key = open_key(false);
    if (aji_key == 0)
        return NULL;

    HKEY subkey = aji_key;
    const char * leaf = strrchr(key, '\\');
    const char * k = key;

    if (leaf != NULL)
    {
        char buffer[64];
        memcpy_s(buffer, sizeof(buffer), k, leaf - k);
        buffer[leaf - k] = 0;

        if (RegOpenKeyEx(aji_key, buffer, 0, this->get_regsam(KEY_READ), &subkey) != ERROR_SUCCESS)
        {
            return NULL;
        }

        k = leaf + 1;
    }

    DWORD expect = REG_SZ;
    DWORD type;
    BYTE buf[1024];
    DWORD valuemax = 1024;
    if (RegQueryValueEx(subkey, k, NULL, &type, &buf[0], &valuemax) != ERROR_SUCCESS)
        return NULL;
    
    if (subkey != aji_key)
        RegCloseKey(subkey);
    if (type != expect)
        return NULL;
        
    m_data[key] = (char*)(&buf[0]);
    return m_data[key].c_str();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CONFIGURATION_WIN::set_value(const char * key, const char * value, bool createonly)
{
    DWORD type = REG_SZ;
    DWORD len = (value != NULL) ? static_cast<DWORD> (strnlen_s(value, STRING_BUFFER_SIZE)) + 1 : 0;

    HKEY aji_key = open_key(false);
    if (aji_key == 0)
        return false;

    HKEY subkey = aji_key;
    const char * leaf = strrchr(key, '\\');
    const char * ptr = key;

    if (leaf != NULL)
    {
        char buffer[64];
        memcpy_s(buffer, sizeof(buffer), ptr, leaf - ptr);
        buffer[leaf - ptr] = 0;

        DWORD disposition;
        if (RegCreateKeyEx(aji_key, buffer, 0, NULL, REG_OPTION_NON_VOLATILE, this->get_regsam(KEY_WRITE), NULL, &subkey, &disposition) != ERROR_SUCCESS)
            return false;

        // If createonly was set then this function must fail if the key existed beforehand.
        if (createonly && disposition != REG_CREATED_NEW_KEY)
        {
            RegCloseKey(subkey);
            return false;
        }

        ptr = leaf + 1;
    }

    bool ok = true;

    if (value == NULL)
        ok = (RegDeleteValue(subkey, ptr) == ERROR_SUCCESS);
    else
        ok = (RegSetValueEx(subkey, ptr, 0, type, reinterpret_cast<const BYTE *>(value), len) == ERROR_SUCCESS);
    
    if (subkey != aji_key)
        RegCloseKey(subkey);

    m_data[key] = value;
    return ok;
    
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CONFIGURATION_WIN::enumerate(DWORD * instance, char * value, DWORD valuemax)
{
    HKEY aji_key = open_key(false);
    if (aji_key == 0)
        return false;
    
    if (RegEnumKeyEx(aji_key, *instance, value, &valuemax, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
        return false;
    
    (*instance)++;
    
    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CONFIGURATION_WIN::deletekey(const char * key)
{
    HKEY aji_key = open_key(false);
    if (aji_key == 0)
        return false;

    if (RegDeleteKey(aji_key, key) != ERROR_SUCCESS)
        return false;
    
    m_data.erase(key);
    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline HKEY AJI_CONFIGURATION_WIN::open_key(bool server)  //server is dummy variable for compatibility, set to false
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    static HKEY aji_key[2];

    DWORD index   = 0;
    HKEY base_key = HKEY_CURRENT_USER;

    if (aji_key[index] != 0)
        return aji_key[index];

    // Try to open an existing key, or to create one if it doesn't yet exist.
    if (RegCreateKeyEx(base_key, "Software\\Altera Corporation\\JTAGServer", 0, NULL,
                       REG_OPTION_NON_VOLATILE, this->get_regsam(KEY_ALL_ACCESS), NULL, 
                       &aji_key[index], NULL) == ERROR_SUCCESS)
        return aji_key[index];

    return 0;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline REGSAM AJI_CONFIGURATION_WIN::get_regsam(REGSAM regsam_in)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    REGSAM regsam_out(m_regsam_modifier | regsam_in);

    return regsam_out;
}
