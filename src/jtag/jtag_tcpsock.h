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

#ifndef INC_JTAG_TCPSOCK_H
#define INC_JTAG_TCPSOCK_H

//# INCLUDE FILES //////////////////////////////////////////////////////////

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif 

#ifdef WIN32

#ifndef INC_WINSOCK2_H
#include <winsock2.h>
#define INC_WINSOCK2_H
#endif

#ifndef INC_WINDOWS_H
#include <Windows.h>
#define INC_WINDOWS_H
#endif

#else

#ifndef INC_SYS_TIME_H
#include <sys/time.h>
#define INC_SYS_TIME_H
#endif

#ifndef INC_SYS_TYPES_H
#include <sys/types.h>
#define INC_SYS_TYPES_H
#endif

#ifndef INC_SYS_SOCKET_H
#include <sys/socket.h>
#define INC_SYS_SOCKET_H
#endif

#ifndef INC_SYS_IOCTL_H
#include <sys/ioctl.h>
#define INC_SYS_IOCTL_H
#endif

#ifndef INC_SYS_UIO_H
#include <sys/uio.h>
#define INC_SYS_UIO_H
#endif

#ifndef INC_UNISTD_H
#include <unistd.h>
#define INC_UNISTD_H
#endif

#ifndef INC_NETINET_IN_H
#include <netinet/in.h>
#define INC_NETINET_IN_H
#endif

#ifndef INC_ARPA_INET_H
#include <arpa/inet.h>
#define INC_ARPA_INET_H
#endif

#ifndef INC_NETDB_H
#include <netdb.h>
#define INC_NETDB_H
#endif

#ifndef INC_CERRNO
#include <cerrno>
#endif

// TODO: fix gross hack on next line
#define FIONBIO 0x5421 // HACK, this only applies to linux

#endif

//#define TCP_BUFFER_SIZE 64 // HACK: test with different sized buffers

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class TCPSOCK
{
public:
    bool socket_valid(void) { return m_sock >= 0; }

protected:
    TCPSOCK(int sock = -1);
    ~TCPSOCK(void);

    bool create(void);
    void close(void);

    bool set_nonblock(bool nonblock);

#ifdef TCP_BUFFER_SIZE
    bool set_buffers(void);
#endif

    int        m_sock;

private:
#ifdef WIN32
    bool       m_started;
    static int s_sessions;
#endif
};

#endif
