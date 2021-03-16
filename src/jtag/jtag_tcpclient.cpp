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

#ifndef INC_CSTDLIB
#include <cstdlib>
#define INC_CSTDLIB
#endif

#ifndef INC_CSTRING
#include <cstring>
#define INC_CSTRING
#endif


#ifndef INC_GEN_STRING_SYS_H
#include "gen_string_sys.h"
#endif

#ifndef INC_JTAG_PLATFORM_H
#include "jtag_platform.h"
#endif

#ifndef INC_JTAG_TCPCLIENT_H
#include "jtag_tcpclient.h"
#endif

#ifndef INC_JTAG_MESSAGE_H
#include "jtag_message.h"
#endif

#ifndef INC_JTAG_COMMON_H
#include "jtag_common.h"
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

TCPCLIENT::TCPCLIENT(void) : m_addr(NULL), m_addr_count(0)
{
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

TCPCLIENT::~TCPCLIENT(void)
{
    delete[] m_addr;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

bool TCPCLIENT::lookup(const char * server, DWORD port)
{
    unsigned long addr;
    struct hostent * ent;

    delete[] m_addr;
    m_addr = NULL;
    m_addr_count = 0;

    if (server == NULL || strcmp_(server, HOSTNAME_BUFFER_SIZE, "localhost") == 0)
    {
        m_addr = new BYTE[4];
        if (m_addr == NULL)
            return false;

        m_addr_len = 4;
        m_addr_count = 1;

        m_addr[0] = 127;
        m_addr[1] = 0;
        m_addr[2] = 0;
        m_addr[3] = 1;
    }
    else if ((addr = inet_addr(server)) != INADDR_NONE && addr != 0)
    {
        m_addr = new BYTE[4];
        if (m_addr == NULL)
            return false;

        m_addr_len = 4;
        m_addr_count = 1;

        * reinterpret_cast<unsigned long *>(m_addr) = addr;
    }
    else
    {
        ent = gethostbyname(server);
        if (ent == NULL)
            return false; //WSAGetLastError();
        if (ent->h_addrtype != AF_INET || ent->h_addr_list[0] == NULL)
            return false; //SOCKERROR(EBADF);

        m_addr_len = ent->h_length;

        int n = 0;
        while (ent->h_addr_list[n] != NULL)
            n++;

        m_addr = new BYTE[m_addr_len * n];
        if (m_addr != NULL)
        {
            m_addr_count = n;

            for (int i = 0 ; i < n ; i++)
                memcpy_s(m_addr + i * m_addr_len, (((n-i)*m_addr_len)*sizeof(BYTE)), ent->h_addr_list[i], m_addr_len);
        }
    }

    m_port = port;
    m_addr_index = 0;

    return (m_addr_count > 0);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool TCPCLIENT::connect(void)
//
// Returns: true if connection has succeeded or failed, false if we are still
//          trying.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    while (m_sock < 0)
    {
        if (m_addr_index == m_addr_count)
        {
            m_addr_index = 0;
            return true;
        }

        // Try new connect
        if (!create())
        {
            m_addr_index = 0;
            return true;
        }

        struct linger linger;
        linger.l_onoff = 1;
        linger.l_linger = 10;
        if (setsockopt(m_sock, SOL_SOCKET, SO_LINGER, reinterpret_cast<char *>(&linger), sizeof(linger)) != 0)
        {
            close();
            m_addr_index = 0;
            return true;
        }

        set_nonblock(true);

        sockaddr_in sin = { 0 };
        sin.sin_family = AF_INET;
        memcpy_s(&sin.sin_addr, sizeof(sin.sin_addr), m_addr + m_addr_index * m_addr_len, m_addr_len);
        sin.sin_port = htons(static_cast<unsigned short>(m_port));

        m_addr_index++;

        if (::connect(m_sock, (sockaddr *)&sin, sizeof(sin)) == 0)
            break;

#if PORT==UNIX && SYS==LINUX
        if (WSAGetLastError() == SOCKERROR(EINPROGRESS))
#else
        if (WSAGetLastError() == SOCKERROR(EWOULDBLOCK))
#endif
            return false;

        close();
    }

    // If we get here then we've successfully started to connect to the server.
    // Check whether the connection has succeeded yet.
    fd_set writefds;
    fd_set exceptfds;

    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

#pragma AJI_DISABLE_WARNING_PRAGMA(4127)
    // Socket is marked writable when connect succeeds
    FD_SET((unsigned int)m_sock, &writefds);
    FD_SET((unsigned int)m_sock, &exceptfds);
#pragma AJI_RESTORE_WARNING_PRAGMA(4127)

    // Return immediately if nothing has happened
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int rc = select(m_sock+1, NULL, &writefds, &exceptfds, &tv);
    if (rc == 0) // Timeout
        return false;

    // If there is space to write and no connection error reported then return now.
    int error;
#if PORT != WINDOWS && SYS==LINUX
    socklen_t errlen = sizeof(error);
#else
    int errlen = sizeof(error);
#endif
    if (FD_ISSET(m_sock, &writefds) &&
        getsockopt(m_sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&error), &errlen) == 0 &&
        error == 0)
    {
#ifdef TCP_BUFFER_SIZE
        set_buffers();
#endif
        return true;
    }

    // Must be a failure then
    close();
    m_addr_index = 0;
    return true;
}

