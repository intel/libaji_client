/****************************************************************************
 *   Copyright (c) 2000 by Intel Corporation                                *
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
// Description: 
//
// Authors:     Andrew Draper
//
//              Copyright (c) Altera Corporation 2000
//              All rights reserved.
//
//END_MODULE_HEADER//////////////////////////////////////////////////////////

//START_ALGORITHM_HEADER/////////////////////////////////////////////////////
//
//
//END_ALGORITHM_HEADER///////////////////////////////////////////////////////
//

// INCLUDE FILES ////////////////////////////////////////////////////////////
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif

#ifndef INC_CLIMITS
#include <climits>
#define INC_CLIMITS
#endif

#if PORT == UNIX
#ifndef INC_NETINET_TCP_H
#include <netinet/tcp.h>
#define INC_NETINET_TCP_H
#endif
#endif // PORT == UNIX

#ifndef INC_JTAG_PLTAFORM_H
#include "jtag_platform.h"
#endif

#ifndef INC_JTAG_TCPSOCK_H
#include "jtag_tcpsock.h"
#endif

#ifdef WIN32
int TCPSOCK::s_sessions = 0;
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

TCPSOCK::TCPSOCK(int sock)
  : m_sock(sock)
{
#ifdef WIN32
    m_started = false;

    WSADATA winsock_data;
  if (s_sessions == 0 && WSAStartup(MAKEWORD(1,0), &winsock_data) != 0)
        return;

    m_started = true;
    s_sessions++;
#endif
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

TCPSOCK::~TCPSOCK(void)
{
    close();

#ifdef WIN32
    if (m_started && --s_sessions == 0)
        WSACleanup();
#endif
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

bool TCPSOCK::create(void)
{
    AJI_DEBUG_ASSERT(m_sock < 0);
#ifdef WIN32
    AJI_DEBUG_ASSERT(m_started);
#endif

    m_sock = static_cast<int> (socket(PF_INET, SOCK_STREAM, 0));
  if (m_sock < 0)
        return false;

#if 1 //def NO_NAGLE_TEST
    int on = 1;
    setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
#endif

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

bool TCPSOCK::set_nonblock(bool nonblock)
{
#ifdef WIN32
    u_long value = nonblock ? 1 : 0;
    ioctlsocket(m_sock, FIONBIO, &value);
#else
    int value = nonblock ? 1 : 0;
    ioctl(m_sock, FIONBIO, &value);
#endif

    return true;
}

#ifdef TCP_BUFFER_SIZE

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool TCPSOCK::set_buffers(void)
//
// Description: Debug function to check speed with different sized buffers.
//
{
#ifdef WIN32
    int sndbuf = TCP_BUFFER_SIZE * 1024;
    setsockopt(m_sock, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, sizeof(sndbuf));

    int rcvbuf = TCP_BUFFER_SIZE * 1024;
    setsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, sizeof(sndbuf));
#else
#endif

    return true;
}

#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void TCPSOCK::close(void)
{
    if (m_sock >= 0)
    {
#ifdef WIN32
        closesocket(m_sock);
#else
        ::close(m_sock);
#endif
        m_sock = -1;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////


