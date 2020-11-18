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
// Filename:    jtag_configuration.cpp
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
// Configuration file interface
//
//END_ALGORITHM_HEADER///////////////////////////////////////////////////////

// INCLUDE FILES ////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif

#ifndef INC_SET
#include <set>
#define INC_SET
#endif

#ifndef INC_JTAG_CONFIGURATION_H
#include "jtag_configuration.h"
#endif

#ifndef INC_CONFIGURATION_FILE_H
#include "jtag_configuration_file.h"
#endif

#ifndef INC_CONFIGURATION_MEMORY_H
#include "jtag_configuration_memory.h"
#endif

#if PORT == WINDOWS
#ifndef INC_JTAG_CONFIGURATION_WIN_H
#include "jtag_configuration_win.h"
#endif
#endif

#ifndef INC_JTAG_COMMON_H
#include "jtag_common.h"
#endif


static AJI_CONFIGURATION *client_config = NULL;
static AJI_CONFIGURATION *server_config = NULL;

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void config_set_memory(bool server)
{
    if (server)
    {
        if (server_config == NULL)
            delete server_config;
        server_config = (AJI_CONFIGURATION*) new AJI_CONFIGURATION_MEMORY(true);
    }
    else
    {
        if (client_config == NULL)
            delete client_config;
        client_config = (AJI_CONFIGURATION*) new AJI_CONFIGURATION_MEMORY(true);
    }   
}

#if PORT==WINDOWS
//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void initialize_config()
{
    if (client_config == NULL)
    {
        const char * jtag_client_config = getenv("QUARTUS_JTAG_CLIENT_CONFIG");
        if (jtag_client_config)
        {
            client_config = (AJI_CONFIGURATION*) new AJI_CONFIGURATION_FILE(false);
            ((AJI_CONFIGURATION_FILE*)client_config)->set_filename(jtag_client_config);
        }
        else
        {
            client_config = (AJI_CONFIGURATION*) new AJI_CONFIGURATION_WIN(false);
        }
    }
    if (server_config == NULL)
    {
        const char * jtag_server_config = getenv("QUARTUS_JTAG_SERVER_CONFIG");
        if (jtag_server_config)
        {
            server_config = (AJI_CONFIGURATION*) new AJI_CONFIGURATION_FILE(true);
            ((AJI_CONFIGURATION_FILE*)server_config)->set_filename(jtag_server_config);
        }
        else
        {
            server_config = (AJI_CONFIGURATION*) new AJI_CONFIGURATION_WIN(true);
        }
    }
}
#else
//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void initialize_config()
{
    if (client_config == NULL)
    {
        client_config = (AJI_CONFIGURATION*) new AJI_CONFIGURATION_FILE(false);
        const char * jtag_client_config = getenv("QUARTUS_JTAG_CLIENT_CONFIG");
        if (jtag_client_config)
        {
            ((AJI_CONFIGURATION_FILE*)client_config)->set_filename(jtag_client_config);
        }
    }
    if (server_config == NULL)
    {
        server_config = (AJI_CONFIGURATION*) new AJI_CONFIGURATION_FILE(true);
        const char * jtag_server_config = getenv("QUARTUS_JTAG_SERVER_CONFIG");
        if (jtag_server_config)
        {
            ((AJI_CONFIGURATION_FILE*)server_config)->set_filename(jtag_server_config);
        }
    }
}
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void config_set_filename(const char * filename)
{
    if (server_config)
        delete server_config;
    server_config = (AJI_CONFIGURATION*) new AJI_CONFIGURATION_FILE(true);
    ((AJI_CONFIGURATION_FILE*)server_config)->set_filename(filename);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
const char* config_set_default_filename(void)
{
    if (server_config)
        delete server_config;
    server_config = (AJI_CONFIGURATION*) new AJI_CONFIGURATION_FILE(true);
    if (((AJI_CONFIGURATION_FILE*)server_config)->set_default_filename()){
        return ((AJI_CONFIGURATION_FILE*)server_config)->get_filename();
    }
    else{
        return NULL;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool config_get_string(bool server, const char * key, char * value, DWORD valuemax)
{
    initialize_config();
    AJI_CONFIGURATION *config = server ? server_config : client_config;

    const char * retrieved = config->get_value(key);
    if (retrieved == NULL)
        return false;

    if (strnlen_s(retrieved, STRING_BUFFER_SIZE) >= valuemax)
        return false;

    strcpy_s(value, valuemax, retrieved);
    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool config_set_string(bool server, const char * key, const char * value, bool createonly)
{
    initialize_config();
    AJI_CONFIGURATION *config = server ? server_config : client_config;

    return config->set_value(key, value, createonly);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool config_enumerate(bool server, DWORD * instance, char * value, DWORD valuemax)
{
    initialize_config();
    AJI_CONFIGURATION *config = server ? server_config : client_config;

    return config->enumerate(instance, value, valuemax);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool config_delete(bool server, const char * key)
{
    initialize_config();
    AJI_CONFIGURATION *config = server ? server_config : client_config;

    return config->deletekey(key);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CONFIGURATION::STRING::operator < (const STRING & other) const
//
// Description: return true if this string sorts before the other key.  Order
//              strings by section then by subsection.  Strings without a
//              section go first.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // Empty strings go first
    if (other.m_data == NULL)
        return false;
    if (m_data == NULL)
        return true;

    // Strings without a subsection go first
    bool no_subsection = (strchr(m_data, '\\') == NULL);
    bool other_no_subsection = (strchr(other.m_data, '\\') == NULL);

    if (no_subsection != other_no_subsection)
        return no_subsection;

    return strcmp_(m_data, STRING_BUFFER_SIZE, other.m_data) < 0;
}
