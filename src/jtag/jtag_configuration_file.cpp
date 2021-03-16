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
// Filename:    jtag_configuration_file.cpp
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
// Platform specific functions for Unix
//
//END_ALGORITHM_HEADER///////////////////////////////////////////////////////

// INCLUDE FILES ////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif

#ifndef INC_CSTDIO
#include <cstdio>
#define INC_CSTDIO
#endif

#ifndef INC_CSTDLIB
#include <cstdlib>
#define INC_CSTDLIB
#endif

#ifndef INC_CCTYPE
#include <cctype>
#define INC_CCTYPE
#endif

#ifndef INC_CERRORNO
#include <cerrno>
#define INC_CERRORNO
#endif

#if PORT == WINDOWS
#ifndef INC_IO_H
#include <io.h>
#define INC_IO_H
#endif
#endif // PORT == WINDOWS

#ifndef INC_SYS_TYPES_H
#include <sys/types.h>
#define INC_SYS_TYPES_H
#endif

#ifndef INC_SYS_STAT_H
#include <sys/stat.h>
#define INC_SYS_STAT_H
#endif

#ifndef INC_FCNTL_H
#include <fcntl.h>
#define INC_FCNTL_H
#endif

#ifndef INC_JTAG_PLATFORM_H
#include "jtag_platform.h"
#endif

#ifndef INC_JTAG_CONFIGURATION_FILE_H
#include "jtag_configuration_file.h"
#endif

#ifndef INC_JTAG_COMMON_H
#include "jtag_common.h"
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CONFIGURATION_FILE::set_default_filename(void)
{
    const char * jtag_client_config = getenv("QUARTUS_JTAG_CLIENT_CONFIG");
    if (jtag_client_config)
    {
        set_filename(jtag_client_config);
    }
    else
    {
        const char * home = getenv("HOME");
        char filename[256];
    
        if (home != NULL && home[0] != 0)
        {
            // Client config should be in the home directory...
            const char * slash = home[strnlen_s(home, _MAX_PATH)-1] != '/' ? "/" : "";
            snprintf(filename, sizeof(filename), "%s%s.jtag.conf", home, slash);
            set_filename(filename);
        }
        else
        {
            // ... so if the user hasn't set $HOME then we fill in an invalid
            // value to stop it crashing
            set_filename(NULL);
        }
    }

    return (m_config_filename != NULL);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CONFIGURATION_FILE::set_filename(const char * filename)
{
    delete[] m_config_filename;

    if (filename == NULL)
    {
        m_config_filename = NULL;
    }
    else
    {
        char * config_filename = new char[strnlen_s(filename, _MAX_PATH)+1];
        m_config_filename = config_filename;
        if (config_filename != NULL)
            strcpy_s(config_filename, _MAX_PATH, filename);
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
const char * AJI_CONFIGURATION_FILE::get_filename(void)
{
    return m_config_filename;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int AJI_CONFIGURATION_FILE::load(void)
{
    if (m_config_filename == NULL && !set_default_filename())
        return -ENOENT;

    // Read the file's statistics.  If we can't then say we need to read it just
    // in case.
  struct stat statbuf;
  if (stat(m_config_filename, &statbuf) == 0)
    {
        // mtime changes when the file is written.  ctime changes when it is recreated
        // If they're both the same then no need to reload the file.
        if (statbuf.st_mtime == m_file_mtime && statbuf.st_ctime == m_file_ctime)
            return 0;
    }

    bool already_open = (m_file != NULL);

    if (!already_open)
    {
        int rc = open_file(false);

        if (rc < 0)
        {
            if (rc == -ENOENT)
            {
                m_file_mtime = 0;
                m_file_ctime = 0;
            }
            return rc;
        }
    }

    m_file_mtime = 0;
    m_file_ctime = 0;
    m_data.clear();

    rewind(m_file);

    int rc = 0;

    READER reader(m_file);
    char key[64], value[256], *kptr;

    // Should do a proper YACC based parser really.
    kptr = key;
    for ( ; ; )
    {
        int t;
        t = reader.token(kptr, static_cast<unsigned int> (sizeof(key)) - static_cast<unsigned int> (kptr - key));
        if (t == '}' && kptr != key)
        {
            kptr = key;
            continue;
        }
        else if (t == 'A')
        {
            t = reader.token(NULL, 0);
            if (t == '{' && kptr == key)
            {
                int len = static_cast<int> (strnlen_s(key, sizeof(key)));
                if (len > 32)
                    break;
                kptr = key + strnlen_s(key, sizeof(key));
                *kptr++ = '\\';
                continue;
            }
            else if (t == '=')
            {
                t = reader.token(value, sizeof(value));
                if (t == 'A' || t == 'Q')
                {
                    t = reader.token(NULL, 0);
                    if (t == ';')
                    {
                        m_data[key] = value;
                        continue;
                    }
                }
            }
        }

        break;
    }

  if (fstat(fileno(m_file), &statbuf) < 0)
        rc = -errno;
    else
    {
        m_file_mtime = statbuf.st_mtime;
        m_file_ctime = statbuf.st_ctime;
    }

    // Close the file only if we opened it before
    if (!already_open)
        close_file();

    return rc;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int AJI_CONFIGURATION_FILE::READER::token(char * buffer, unsigned int max)
{
    char * ptr = buffer;
    char * end = buffer + max - 1;
    int c;
    
    do
    {
        c = read_char();
    }
    while (isspace(c));

    if (isalnum(c))
    {
        if (ptr == NULL)
            return 0;

        do
        {
            if (ptr == end)
                return 0;
            *ptr++ = static_cast<char> (c);
            c = read_char();
        }
        while (isalnum(c) || c == '.' || c == '-' || c == '_' || c == '#');
        *ptr = 0;

        ungetc(c, m_file);
        return 'A';
    }
    else if (c == '"')
    {
        if (ptr == NULL)
            return 0;

        while ((c = read_char(true)) != '"')
        {
            if (ptr == end || c == '\n')
                return 0;
            *ptr++ = static_cast<char> (c);
        }
        *ptr = 0;

        return 'Q';
    }
    else
        return c;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int AJI_CONFIGURATION_FILE::READER::read_char(bool in_string)
{
    int c = fgetc(m_file);

    if (c == '#' && !in_string) // Discard to end of line
    {
        while ((c = fgetc(m_file)) != EOF)
            if (c == '\n')
                break;
    }

    return c;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int AJI_CONFIGURATION_FILE::open_file(bool for_write)
{
    AJI_DEBUG_ASSERT(m_file == NULL);

    if (m_config_filename == NULL && !set_default_filename())
        return -ENOENT;

    // Open the file, if this is for write then it should be created if it
    // doesn't exist.
    int file_handle = open(m_config_filename, for_write ? O_CREAT | O_RDWR : O_RDONLY, S_IREAD | S_IWRITE);
    if (file_handle < 0)
    {
        int rc = -errno;
        //fprintf(stderr, "Can't open file '%s', errno = %d\n", m_config_filename, errno);
        return rc;
    }

    // Lock the file for the appropriate type of access if the filesystem
    // supports it.
#if PORT == UNIX
    struct flock fl;

    fl.l_type   = for_write ? F_WRLCK : F_RDLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;
    fl.l_pid    = getpid();

    int rc = fcntl(file_handle, F_SETLKW, &fl);

    // If we can't lock the file then give up.  If the filesystem doesn't
    // support POSIX record locks then don't bother with locks.
    if (rc < 0 && errno != ENOLCK)
    {
        rc = -errno;
        close(file_handle);
        return rc;
    }
#endif

    // If we can't open the config file then we probably don't have write access
    // to it.
    m_file = fdopen(file_handle, for_write ? "w+" : "r");
    if (m_file == NULL)
    {
        int rc = -errno;
        close(file_handle);
        return rc;
    }

    if (for_write)
    {
        // If the file on disk has changed then load it up again so our memory
        // copy is consistent.
        int rc = load();
        if (rc < 0)
        {
            close_file();
            return rc;
        }
    }

    return 0;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CONFIGURATION_FILE::store(void)
{
    AJI_DEBUG_ASSERT(m_file != NULL);

    rewind(m_file);

    fprintf(m_file, "# %s\n", m_config_filename);
    fprintf(m_file, "#\n");
    fprintf(m_file, "# This file is written by the JTAG %s when its configuration is changed.\n", m_server ? "daemon" : "client");
    fprintf(m_file, "# If you edit this file directly then your changes will probably be lost.\n");
    fprintf(m_file, "\n");

    STRING_MAP::const_iterator i;
    char section[64];

    section[0] = 0;

    for (i = m_data.begin() ; i != m_data.end() ; i++)
    {
        const char * key = (*i).first.c_str();
        const char * subkey = strchr(key, '\\');
        const char * indent = "";

        if (subkey != NULL)
        {
            if (memcmp_(section, sizeof(section), key, subkey - key) != 0 || section[subkey-key] != 0)
            {
                if (section[0] != 0)
                    fprintf(m_file, "}\n");

                memcpy_s(section, sizeof(section), key, subkey - key);
                section[subkey - key] = 0;

                fprintf(m_file, "\n%s {\n", section);
            }

            key = subkey + 1;
            indent = "\t";
        }

        fprintf(m_file, "%s%s = \"%s\";\n", indent, key, (*i).second.c_str());
    }

    if (section[0] != 0)
        fprintf(m_file, "}\n");

    fflush(m_file);

    bool ok(false);
    // protect against calling the file resize functions if ftell() encounters an error (i.e. it returns -1L)
    long stream_position(ftell(m_file));

    if (stream_position >= 0)
    {
        // Truncate the file if it's shorter now than it was before.
#if PORT == WINDOWS
        ok = (_chsize(fileno(m_file), stream_position) == 0);
#else
        ok = (ftruncate(fileno(m_file), stream_position) == 0);
#endif
    }

    if (ferror(m_file))
    {
        ok = false;
    }

    if (!close_file())
        ok = false;

    return ok;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
const char * AJI_CONFIGURATION_FILE::get_value(const char * key)
{
    if (load() < 0)
        return NULL;

    STRING_MAP::const_iterator i;

    i = m_data.find(key);
    if (i == m_data.end())
        return NULL;
    else
        return (*i).second.c_str();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CONFIGURATION_FILE::set_value(const char * key, const char * value, bool createonly)
{
    if (m_config_filename == NULL && !set_default_filename())
        return false;

    // Open the file and lock it so other processes can't change its value
    if (open_file(true) < 0)
        return false;

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

    // Write the file
    if (ok)
        store();
    // TODO: think about what to do if store() returns false (probably a
    // permissions error)

    return ok;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CONFIGURATION_FILE::enumerate(DWORD * instance, char * value, DWORD valuemax)
{
    if (*instance == 0 && load() < 0)
        return false;

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
            int n_bytes = (static_cast<int> (slash-key)) > n_buffer_bytes ? n_buffer_bytes : static_cast<int> (slash-key);
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
        strcpy_s(value, valuemax, next);
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
bool AJI_CONFIGURATION_FILE::deletekey(const char * delkey)
{
    // Open the file and check our local database is up to date
    if (open_file(true) < 0)
        return false;

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

    store();
    // TODO: think about what to do if store() returns false (probably a
    // permissions error)

    return true;
}

