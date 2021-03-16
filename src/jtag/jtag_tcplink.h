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

#ifndef INC_JTAG_TCPLINK_H
#define INC_JTAG_TCPLINK_H

//#define ENABLE_LOG

//# INCLUDE FILES //////////////////////////////////////////////////////////
#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_JTAG_TCPSOCK_H
#include "jtag_tcpsock.h"
#endif

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class TXFIFO;
class RXFIFO;

#define JTAG_PORT 1309 // Allocated by IANA to "jtag-server"

class TCPLINK : public TCPSOCK
{
public:
    bool has_memory(void) { return m_rxbuffer != NULL; }

    enum { DISCONNECT = 0, OK = -1, WOULDBLOCK = -2 };

    void close(void) { TCPSOCK::close(); delete_unsent(); }

    bool send(int mux, const BYTE * data, DWORD len, bool copy);
    bool send_fifos(void);
    bool send_flush(void);

    bool push_data(void);

    int receive_cmnd(DWORD timeout);

    void add_txmux(TXFIFO * fifo);
    void release_txmux(TXFIFO * fifo);
    bool send_fifo(TXFIFO * fifo);

    void add_rxmux(RXFIFO * fifo, DWORD mux);
    void release_rxmux(RXFIFO * fifo, DWORD mux);
    bool poll_fifo(DWORD mux);

    virtual void inform_data_used(DWORD bits) { }

protected:
    TCPLINK(int sock = -1);
    virtual ~TCPLINK(void);

private:
    enum { NODATA = -3, FIFODATA = -4 };

    static void delete_queue(TXFIFO * * queue);
    void delete_unsent(void);

    void add_packet(int mux, const BYTE * data, DWORD len, bool copy);
    void add_fifo_packet(TXFIFO * fifo);
    int  send_queued(bool block);
    bool wait_for_space(void);

    bool space_for_packet(DWORD threshold);
    TXFIFO * next_tx_fifo(bool expect_ready);

    void deactivate_tx_fifo(TXFIFO * fifo);

    int receive(DWORD * mux, bool mux0_ok);
    int wait_for_data(DWORD timeout);

    virtual BYTE * get_mux0_receive_buffer(void) = 0;

    virtual void process_notify(const BYTE * data, DWORD len) { }

    enum { BUFFER_SIZE  = 4 + 0x1000 };
    enum { INDIRECT_MAX = 8 };
    enum { FIFO_MIN = 4, FIFO_MAX = 15 };
    enum { MEMCPY_THRESHOLD = 512 };

    BYTE              m_rxhdrbuffer[2]; // Buffer for header receives
    BYTE              m_rxhdrvalid;
    bool              m_rx_mux0_avail;  // Mux0 data is available in the receive queue

    BYTE            * m_rxbuffer; // Allocated buffer for data receives
    DWORD             m_rxvalid;
    BYTE            * m_rxpacket; // Buffer for this packet

    BYTE            * m_txbuffer;
    BYTE            * m_txstart;
    BYTE            * m_txptr;

#ifdef WIN32
    WSABUF                      m_tx_indirect[INDIRECT_MAX];
    WSABUF          * m_tx_iptr;
    #define iov_base buf
    #define iov_len  len
#else
    struct iovec      m_tx_indirect[INDIRECT_MAX];
    struct iovec    * m_tx_iptr;
#endif

    TXFIFO          * m_txfifo;
    TXFIFO          * m_next_txfifo;

    RXFIFO          * m_rxmux[FIFO_MAX+1 - FIFO_MIN];

    TXFIFO          * m_txdone;

#ifdef ENABLE_LOG
    DWORD             m_logindex;
    DWORD             m_log[256];
#endif
};

#ifdef ENABLE_LOG
#define LOG(n) \
    do \
    { \
        m_logindex = (m_logindex + 1) % (sizeof(m_log)/sizeof(m_log[0])); \
        m_log[m_logindex] = n; \
    } while (0)
#else
#define LOG(n)
#endif

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class TXFIFO
{
public:
    void activate(DWORD mux, bool interleave);

    virtual DWORD get_length(void) = 0;

protected:
    TXFIFO(TCPLINK * link);
    virtual ~TXFIFO(void);

    bool send_fifo(void) { return (m_mux != INACTIVE) ? m_link->send_fifo(this) : true; }

    virtual DWORD get_data_avail(DWORD max) = 0;
    virtual const BYTE * get_data(DWORD count) = 0;

private:
    TCPLINK * m_link;
    DWORD     m_mux;
    bool      m_interleave; // Should we interleave transmission of this FIFO with the next one
    TXFIFO  * m_next;

    enum { INACTIVE = 0xFFFF };

    friend class TCPLINK;
};

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class RXFIFO
{
public:
    void activate(DWORD mux);

protected:
    RXFIFO(TCPLINK * link);
    virtual ~RXFIFO(void);

    bool is_activated(void) const { return m_mux != INACTIVE; }

    void inform_data_used(DWORD bits) { m_link->inform_data_used(bits); }

    bool wait_for_data(void) { return m_link->poll_fifo(m_mux); }

    // Get the address where receive_data would store the data on receive.
    virtual BYTE * get_receive_buffer(DWORD len) = 0;

    // Return true if we have got enough data now.
    virtual bool receive_data(DWORD len) = 0;

    inline void release_full_fifo()
    {
        m_link->release_rxmux(this, m_mux);
        deactivate();
    }

    virtual void deactivate(void);

private:
    TCPLINK * m_link;
    DWORD     m_mux;
    RXFIFO  * m_next;

    enum { INACTIVE = 0xFFFF };

    friend class TCPLINK;
};

#endif
