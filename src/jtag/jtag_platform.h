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

//# START_MODULE_HEADER/////////////////////////////////////////////////////
//#
//# Description: 
//#
//# Authors:     Andrew Draper
//#
//#              Copyright (c) Altera Corporation 2000 - 2001
//#              All rights reserved.
//#
//# END_MODULE_HEADER///////////////////////////////////////////////////////

//# START_ALGORITHM_HEADER//////////////////////////////////////////////////
//#
//#
//# END_ALGORITHM_HEADER////////////////////////////////////////////////////
//#

//# INTERFACE DESCRIPTION //////////////////////////////////////////////////
//#
//# Use this for explanatory text
//# describing the interface contained within this file.
//

#ifndef INC_JTAG_PLATFORM_H
#define INC_JTAG_PLATFORM_H

//# INCLUDE FILES //////////////////////////////////////////////////////////

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_JTAG_CONFIGURATION_H
#include "jtag_configuration.h"
#endif

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

int get_time(void);
//bool is_32bit_app_on_64bit_kernel();
//const char * get_pgm_parts_path(bool daemon);
bool get_server_path(char * buffer, int bufflen);
bool start_quartus_process(const char * cmnd, int argc, const char * * args, bool wait_for_success);

#if PORT == WINDOWS

#ifndef INC_WINSOCK2_H
#include <winsock2.h>
#define INC_WINSOCK2_H
#endif

#ifndef INC_WINDOWS_H
#include <windows.h>
#define INC_WINDOWS_H
#endif

#ifndef INC_PROCESS_H
#include <process.h>
#define INC_PROCESS_H
#endif

#define getpid() _getpid()

class JTAG_MUTEX
{
public:
    JTAG_MUTEX(void);
    ~JTAG_MUTEX(void);

    void claim(void);
    bool try_claim(DWORD timeout = 0);

    void release(void);

    bool is_claimed(void) const { return m_threadid == GetCurrentThreadId(); }

private:
    HANDLE m_mutex;
    DWORD  m_threadid;
};

inline JTAG_MUTEX::JTAG_MUTEX(void)
{
    m_mutex = CreateMutex(NULL, FALSE, NULL);
    m_threadid = 0;
}

inline JTAG_MUTEX::~JTAG_MUTEX(void)
{
    if (m_mutex != NULL)
        CloseHandle(m_mutex);
}

inline void JTAG_MUTEX::claim(void)
{
    try_claim(INFINITE);
}

inline bool JTAG_MUTEX::try_claim(DWORD timeout)
{
    if (WaitForSingleObject(m_mutex, timeout) != WAIT_OBJECT_0)
        return false;

    DWORD threadid = GetCurrentThreadId();

    if (m_threadid == threadid)
    {
        ReleaseMutex(m_mutex);
        return false;
    }

    AJI_DEBUG_ASSERT(m_threadid == 0);
    m_threadid = threadid;

    return true;
}

inline void JTAG_MUTEX::release(void)
{
    AJI_DEBUG_ASSERT(m_threadid == GetCurrentThreadId());
    m_threadid = 0;

    ReleaseMutex(m_mutex);
}

#if !defined(_WIN64)
bool is_windows_nt(void); // true for NT/Win2000, false for 95/98
bool is_windows_2000(void); // true for Win2000, false for NT/95/98
#else
inline bool is_windows_nt(void) { return true; } // All 64 bit windows versions are derived from NT
inline bool is_windows_2000(void) { return false; }
#endif

#define SOCKERROR(err) WSA ## err
 
typedef LPWSAPOLLFD pllfd;
typedef ULONG		nfds_t;
namespace INTEL_JTAG
{
	const int			POLL_INDEFINITE_TIMEOUT = -1;
	const int			POLL_IMMEDIATE_TIMEOUT = 0;
    const SHORT			POLL_EVENT_READ = POLLIN | POLLRDNORM;   // Possibly, POLLIN is suffcient as the POLLRDNORM flag is equivalent to POLLIN on Linux.  Nonetheless they are two different flags.
    const SHORT			POLL_EVENT_WRITE = POLLOUT | POLLWRNORM; // Technically, POLLOUT is suffcient as the POLLOUT flag is defined as the same as the POLLWRNORM flag value.
    const SHORT			POLL_EVENT_ERR = POLLERR | POLLHUP;
    // NOTE: POLL_EVENT_ERR is added as part of POLL_EVENT_READ purely to emulate IS_SET() behavior of the old select() function.
    //       When socket is closed on the client side, the server code was written to detect such state by calling select(), IS_SET(), and recv(), which returns 0
    //       in such situation.  With poll(), POLLERR | POLLHUP events are set instead.  In order to replicate the same server algorithm, POLLERR | POLLHUP events
    //       are added as part of POLL_EVENT_READ to emulate the IS_SET() condition such that recv() is called to detect the client-side socket closure.
    // WARNING: POLL_EVENT_READ_IS_SET_EMU cannot be used for WSAPoll() as input event flags because it will result in error return.
    const SHORT			POLL_EVENT_READ_IS_SET_EMU = POLLIN | POLLRDNORM | POLL_EVENT_ERR;

	// This maps to a POSIX sematics compatible poll function
	inline int poll(pollfd * fds, nfds_t nfds, int timeout)
	{
		int ret = ::WSAPoll(fds, nfds, timeout);

		return ret;
	}
}
#elif PORT == UNIX

#ifndef INC_UNISTD_H
#include <unistd.h>
#define INC_UNISTD_H
#endif

#ifndef INC_POOL_H
#include <poll.h>
#define INC_POOL_H
#endif


// HACKS for unix version.  Should be members of tcpsock really

#define SOCKERROR(err) err
#define WSAGetLastError() errno
#define GetLastError() errno
#define SD_SEND 1

#ifdef stricmp
#undef stricmp
#endif
#define stricmp strcasecmp

#ifdef strnicmp
#undef strnicmp
#endif
#define strnicmp strncasecmp

// Sleep for specified number of milli seconds.
#define Sleep(x) usleep((x) * 1000)

#ifndef INC_PTHREAD_H
#include <pthread.h>
#define INC_PTHREAD_H
#endif

// We must implement the mutex code here using unix pthreads - MainWin can
// not be used as in the rest of Quartus because the JTAG system must be able
// to be compiled independently (it's also faster to do it natively)
class JTAG_MUTEX
{
public:
    JTAG_MUTEX(void);
    ~JTAG_MUTEX(void);

    void claim(void);
    bool try_claim(DWORD timeout = 0);

    void release(void);

    bool is_claimed(void) const { return m_threadid == pthread_self(); }

private:
    pthread_mutex_t m_mutex;
    pthread_t m_threadid;
};

inline JTAG_MUTEX::JTAG_MUTEX(void)
{
    pthread_mutex_init(&m_mutex, NULL);
    m_threadid = 0;
}

inline JTAG_MUTEX::~JTAG_MUTEX(void)
{
    pthread_mutex_destroy(&m_mutex);
}

inline void JTAG_MUTEX::release(void)
{
    AJI_DEBUG_ASSERT(m_threadid == pthread_self());
    m_threadid = 0;

    pthread_mutex_unlock(&m_mutex);
}

namespace INTEL_JTAG
{
	const int			POLL_INDEFINITE_TIMEOUT = -1;
	const int			POLL_IMMEDIATE_TIMEOUT = 0;
    const short			POLL_EVENT_READ = POLLIN | POLLRDNORM;   // Possibly, POLLIN is suffcient as the POLLRDNORM flag is equivalent to POLLIN on Linux.  Nonetheless they are two different flags.
    const short			POLL_EVENT_WRITE = POLLOUT | POLLWRNORM; // Technically, POLLOUT is suffcient as the POLLOUT flag is defined as the same as the POLLWRNORM flag value.
    const short			POLL_EVENT_ERR = POLLERR | POLLHUP;
    // NOTE: POLL_EVENT_ERR is added as part of POLL_EVENT_READ purely to emulate IS_SET() behavior of the old select() function.
    //       When socket is closed on the client side, the server code was written to detect such state by calling select(), IS_SET(), and recv(), which returns 0
    //       in such situation.  With poll(), POLLERR | POLLHUP events are set instead.  In order to replicate the same server algorithm, POLLERR | POLLHUP events
    //       are added as part of POLL_EVENT_READ to emulate the IS_SET() condition such that recv() is called to detect the client-side socket closure.
    const short			POLL_EVENT_READ_IS_SET_EMU = POLLIN | POLLRDNORM | POLL_EVENT_ERR;

	inline int poll(pollfd * fds, nfds_t nfds, int timeout)
	{
		int ret = ::poll(fds, nfds, timeout);

		return ret;
	}
}

#endif

enum { DLOG_WARNING = 0, DLOG_INFO = 1, DLOG_DEBUG = 2 };
// WARNING is always logged, INFO with --foreground, DEBUG with --debug
void logprintf(int level, const char * format, ...);

#endif
