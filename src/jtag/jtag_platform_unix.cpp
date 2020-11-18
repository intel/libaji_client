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

#ifndef INC_CERRNO
#include <cerrno>
#define INC_CERRNO
#endif

#ifndef INC_SYS_TIME_H
#include <sys/time.h>
#define INC_SYS_TIME_H
#endif

#ifndef INC_SYS_TYPES_H
#include <sys/types.h>
#define INC_SYS_TYPES_H
#endif

#ifndef INC_SYS_STAT_H
#include <sys/stat.h>
#define INC_SYS_STAT_H
#endif

#ifndef INC_UNISTD_H
#include <unistd.h>
#define INC_UNISTD_H
#endif

#ifndef INC_FCNTL_H
#include <fcntl.h>
#define INC_FCNTL_H
#endif


#if PORT == UNIX && SYS == LINUX
#ifndef __USE_GNU
#define __USE_GNU 1
#endif
#ifndef INC_DLFCN_H
#include <dlfcn.h>
#define INC_DLFCN_H
#endif
#endif // PORT == UNIX && SYS == LINUX  

#ifndef INC_JTAG_PLATFORM_H
#include "jtag_platform.h"
#endif

#ifndef INC_JTAG_CONFIGURATION_H
#include "jtag_configuration.h"
#endif

#ifndef INC_CSTDLIB
#include <cstdlib>
#define INC_CSTDLIB
#endif

#ifndef INC_SECURE_GETENV_H
#include "secure_getenv.h"
#endif

#ifndef INC_GEN_STRING_SYS_H
#include "gen_string_sys.h"
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int get_time(void)
//
// Return a monotonically increasing time measured in milliseconds (eg. from
// system reset or similar).  This time will overflow after 47 days or so so
// make sure all comparisons are of the form "(t1 - t2) > constant"
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void JTAG_MUTEX::claim(void)
{
    if (pthread_mutex_lock(&m_mutex) != 0)
        return;

    AJI_DEBUG_ASSERT(m_threadid == 0);
    m_threadid = pthread_self();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool JTAG_MUTEX::try_claim(DWORD timeout)
{
    // timeout variable is in milliseconds
    int sec = timeout/1000;
    int usec = (timeout % 1000) * 1000;

    // Get the current absolute time
    struct timeval abs_time;
    gettimeofday(&abs_time, NULL);

    // Get the timeout absolute time
    sec += abs_time.tv_sec;
    usec += abs_time.tv_usec;

    // usec should be less than 1 million
    if (usec >= 1000000)
    {
        sec += 1;
        usec -= 1000000;
    }

    struct timespec abs_timeout;
    abs_timeout.tv_sec = sec;
    abs_timeout.tv_nsec = usec * 1000;

    if (pthread_mutex_timedlock(&m_mutex, &abs_timeout) != 0)
        return false;

    AJI_DEBUG_ASSERT(m_threadid == 0);
    m_threadid = pthread_self();
    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool get_server_path(char * buffer, int bufflen)
//
// Description: Find the path to the running executable
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    Dl_info info;
    if (!dladdr((void *)&get_server_path, &info))
        return false;

    strncpy_s(buffer, bufflen, info.dli_fname, (bufflen-1));
    buffer[bufflen-1] = 0;

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool find_quartus_rootdir(char * buffer, int bufflen)
//
// Description: Find the ACDS install directory
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    Dl_info info;
    if (!dladdr((void *)&find_quartus_rootdir, &info))
        return false;

    strncpy_s(buffer, bufflen, info.dli_fname, (bufflen-1));
    buffer[bufflen-1] = 0;

    // Now we have a path something like /foo/bar/quartus/linux64/libjtag_client.so (I hope)
    // Strip off the last two slashes to give us the quartus rootdir
    for (int i = 0 ; i < 2 ; i++)
    {
        char * slash = strrchr(buffer, '/');
        if (slash == NULL)
            return false;

        *slash = 0;
    }

    return true;
}

//START_FUNCTION_HEADER/////START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool start_quartus_process(const char * cmnd, int argc, const char * * args, bool wait_for_success)
//
// Description: Start a process with the arguments specified.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    char command[512], config[512];

    const char * quartus_rootdir = getenv("QUARTUS_ROOTDIR");
    const char * home = getenv("HOME");
    // Use QUARTUS_ROOTDIR if we are running within an overlay due to the symbolic
    // links used. This is safe because the overlays always have QUARTUS_ROOTDIR.
    const char * acds_overlay = getenv("ACDS_OVERLAY");

    if (find_quartus_rootdir(command, sizeof(command)) && strnlen_s(command, sizeof(command)) < sizeof(command) - 13 && !acds_overlay)
    {
        const size_t commandlen = strnlen_s(command, sizeof(command));
        snprintf(command + commandlen, sizeof(command)-commandlen, "/bin/%s", cmnd);
    }
    // Emergency backup uses deprecated environment variable
    else if (quartus_rootdir != NULL && quartus_rootdir[0] != 0)
    {
        const char * slash = quartus_rootdir[strnlen_s(quartus_rootdir, _MAX_PATH)-1] != '/' ? "/" : "";
        snprintf(command, sizeof(command), "%s%sbin/%s", quartus_rootdir, slash, cmnd);
    }
    else
    {
        // Don't know where we are installed so we can't start any jtag servers
        return false;
    }

    if (home != NULL && home[0] != 0)
    {
        // When running as a user store configuration in their home directory.
        const char * slash = home[strnlen_s(home, _MAX_PATH)-1] != '/' ? "/" : "";
        snprintf(config, sizeof(config), "%s%s.jtagd.conf", home, slash);
    }
    else if (quartus_rootdir != NULL && quartus_rootdir[0] != 0)
    {
        // $HOME isn't set - try using the install directory (it might work)
        const char * slash = quartus_rootdir[strnlen_s(quartus_rootdir, _MAX_PATH)-1] != '/' ? "/" : "";
        snprintf(config, sizeof(config), "%s%sbin/jtagd.conf", quartus_rootdir, slash);
    }
    else
        snprintf(config, sizeof(config), "jtagd.conf");

    char * argv[10];
    argv[0] = command;
    memcpy_s(&argv[1], 9 * sizeof(argv[1]), args, argc * sizeof(argv[1]));
    argv[argc+1] = NULL;

    if (fork() == 0)
    {
        // Child process.

        // Start the JTAG daemon running as the current user (for use from the
        // local machine only, stops when finished).
        execv(command, argv);

        // Exec failed if we get here.  Kill the child process.
        exit(4);
    }

    return true;
}

/////////////////////////////////////////////////////
