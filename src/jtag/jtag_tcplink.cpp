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

#ifndef INC_CSTDIO
#include <cstdio>
#define INC_CSTDIO
#endif

#ifndef INC_CSTRING
#include <cstring>
#define INC_CSTRING
#endif

#ifndef INC_CLIMITS
#include <climits>
#define INC_CLIMITS
#endif

#ifndef INC_GEN_STRING_SYS_H
#include "gen_string_sys.h"
#endif

#ifndef INC_JTAG_PLTAFORM_H
#include "jtag_platform.h"
#endif

#ifndef INC_JTAG_TCPLINK_H
#include "jtag_tcplink.h"
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

TCPLINK::TCPLINK(int sock)
  : TCPSOCK(sock)
{
    m_rxbuffer   = new unsigned char[BUFFER_SIZE * 2];
    m_rxhdrvalid = 0;
    m_rx_mux0_avail = 0;
    m_rxvalid    = 0;
    m_rxpacket   = NULL;

    m_txbuffer   = m_rxbuffer + BUFFER_SIZE;
    m_txptr      = m_txbuffer;
    m_txstart    = m_txbuffer;

    m_tx_iptr    = m_tx_indirect;

    m_txfifo     = NULL;
    m_next_txfifo= NULL;

    for (int i = 0 ; i < FIFO_MAX+1-FIFO_MIN ; i++)
        m_rxmux[i] = NULL;

    m_txdone     = NULL;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
TCPLINK::~TCPLINK(void)
{
    if (m_sock >= 0)
    {
        // Send any remaining data.  If we are being disconnected due to an error or
        // a remote disconnection then this will return an error immediately.
        send_flush();

        shutdown(m_sock, SD_SEND);
    }

    delete[] m_rxbuffer;

    // TODO: delete transmit and receive fifos from queue?

    delete_queue(&m_txdone);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void TCPLINK::delete_queue(TXFIFO * * queue)
{
    while (*queue != NULL)
    {
        TXFIFO * fifo = *queue;
        *queue = fifo->m_next;
        delete fifo;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void TCPLINK::delete_unsent(void)
//
// Description: Delete all unsent data as the connection has broken.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    m_txptr = m_txstart = m_txbuffer;
    m_tx_iptr = m_tx_indirect;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline void TCPLINK::add_packet(int mux, const BYTE * data, DWORD len, bool copy)
//
// Description: Add one packet of data to the transmit queue.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(len >= 1 && len <= 0x1000);
    AJI_DEBUG_ASSERT(m_tx_iptr <= m_tx_indirect + INDIRECT_MAX - 2);

    DWORD header = (len - 1) | (mux << 12);

    *m_txptr++ = static_cast<BYTE>(header >> 8);
    *m_txptr++ = static_cast<BYTE>(header);

    if (copy)
    {
        memcpy_s(m_txptr, len, data, len);
        m_txptr += len;
    }

    m_tx_iptr->iov_base = (char *)m_txstart;
    m_tx_iptr->iov_len  = static_cast<DWORD> (m_txptr - m_txstart);
    m_tx_iptr++;

    if (!copy)
    {
        m_tx_iptr->iov_base = (char *)data;
        m_tx_iptr->iov_len  = len;
        m_tx_iptr++;
    }

    m_txstart = m_txptr;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline void TCPLINK::add_fifo_packet(TXFIFO * fifo)
//
// Description: Add a packet holding as much data as possible from the FIFO
//              specified to the transmit queue.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(fifo != NULL);
    AJI_DEBUG_ASSERT(m_txptr < m_txbuffer + BUFFER_SIZE - 2 - MEMCPY_THRESHOLD);
    AJI_DEBUG_ASSERT(m_tx_iptr <= m_tx_indirect + INDIRECT_MAX - 2);

    DWORD flen = fifo->get_data_avail(4096);
    if (flen < MEMCPY_THRESHOLD)
        flen = fifo->get_data_avail(static_cast<DWORD> (m_txbuffer + BUFFER_SIZE - 2 - m_txptr));

    const BYTE * data = fifo->get_data(flen);
    DWORD mux  = fifo->m_mux + FIFO_MIN;

    DWORD header = (flen - 1) | (mux << 12);
    *m_txptr++ = static_cast<BYTE>(header >> 8);
    *m_txptr++ = static_cast<BYTE>(header);

    if (flen >= MEMCPY_THRESHOLD)
    {
        // If the buffer is big enough then it may be more efficient to avoid
        // the copy.
        m_tx_iptr->iov_base = (char *)m_txstart;
        m_tx_iptr->iov_len  = static_cast<DWORD> (m_txptr - m_txstart);
        m_tx_iptr++;
        m_tx_iptr->iov_base = (char *)data;
        m_tx_iptr->iov_len  = flen;
        m_tx_iptr++;

        m_txstart = m_txptr;
    }
    else
    {
        memcpy_s(m_txptr, flen, data, flen);
        m_txptr += flen;
    }

    m_next_txfifo = fifo->m_interleave ? fifo->m_next : NULL;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline bool TCPLINK::space_for_packet(DWORD threshold)
//
// Returns:     true if there is space to queue this packet
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    return (m_txptr < m_txbuffer + BUFFER_SIZE - 2 - threshold) &&
           (m_tx_iptr <= m_tx_indirect + INDIRECT_MAX - 2);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline void TCPLINK::deactivate_tx_fifo(TXFIFO * fifo)
//
// Description: mark the FIFO as ready for deletion.  We can't delete it
//              immediately as the vectors might have transmit pointers into
//              our data so we delete all the old transmit FIFOs once the
//              transmit queue is empty.
{
    release_txmux(fifo);

    // Mark it as inactive
    fifo->m_mux = TXFIFO::INACTIVE;

    // And add it to the ready for deletion queue.
    fifo->m_next = m_txdone;
    m_txdone = fifo;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool TCPLINK::send(int mux, const BYTE * data, DWORD len, bool copy)
//
// Description: Queue the data specified for transmission as a packet via TCP.
//              The data may not be passed into TCPs send queue immediately, if
//              copy is false then it must remain valid until send_flush() is
//              called.  If copy is true then the data is copied and need not
//              remain valid.
//
// Returns:     true if link is still valid
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // Send any unsent data so that we have space to queue the packet.
    if (!space_for_packet(copy ? len : 0) && send_queued(true) != OK)
        return false;

    // Queue the packet, and enough FIFO data to ensure that TCP will be able
    // to send a full IP packet.
    add_packet(mux, data, len, copy);

    // Pass as much FIFO data as possible into the TCP transmit queue.  This is 
    // a performance optimisation when the transmit queue is empty as it lets
    // TCP send a full packet, thus avoiding problems with triggering the Nagle
    // algorithm.  When the queue is not empty queueing several blocks at once
    // has no disadvantages.  [Turning off Nagle is a bad idea if it can be
    // avoided (as it can in this case).]
    while (space_for_packet(MEMCPY_THRESHOLD))
    {
        TXFIFO * fifo = next_tx_fifo(false);

        if (fifo == NULL)
            break;

        add_fifo_packet(fifo);
    }

    // Attempt to send the queued packet
    int result = send_queued(false);

    return (result == OK) || (result == WOULDBLOCK);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool TCPLINK::send_fifo(TXFIFO * fifo)
{
    // Send any unsent data if there will be no space to add this fifo.
    if (!space_for_packet(MEMCPY_THRESHOLD))
    {
        int result = send_queued(false);
        if (result != OK)
            return (result == WOULDBLOCK);
    }

    // Peer expects fifos to be sent in the right order.
    while (m_txfifo != NULL)
    {
        DWORD len = m_txfifo->get_data_avail(4096);

        if (len == 0xFFFFFFFF) // No data yet
            break;

        if (len == 0)
        {
            deactivate_tx_fifo(m_txfifo);
            continue;
        }

        if (!space_for_packet(MEMCPY_THRESHOLD))
            break;

        add_fifo_packet(m_txfifo);
    }

    // No point in sending until we have a packets worth of data ready.
    if (space_for_packet(MEMCPY_THRESHOLD))
        return true;

    int result = send_queued(false);

    return (result == OK) || (result == WOULDBLOCK);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool TCPLINK::send_fifos(void)
//
// Description: Send the contents of any FIFOs which are pending, and wait
//              until all data has been sent from transmit buffers.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    TXFIFO * fifo;

    // Send all pending FIFOs
    while ((fifo = next_tx_fifo(true)) != NULL)
    {
        if (!space_for_packet(MEMCPY_THRESHOLD) && send_queued(true) != OK)
            return false;

        add_fifo_packet(fifo);
    }

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool TCPLINK::send_flush(void)
//
// Description: Wait until all data has been sent from transmit buffers.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // Send any unsent data.
    if (m_tx_iptr != m_tx_indirect || m_txptr != m_txstart)
        if (send_queued(true) != OK)
            return false;

    AJI_DEBUG_ASSERT(m_tx_iptr == m_tx_indirect);
    AJI_DEBUG_ASSERT(m_txptr == m_txstart);

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool TCPLINK::push_data(void)
//
// Description: If there is data waiting in the user-space buffers then move 
//              as much as possible into the kernel buffers.
//
// Returns:     true if there is more data to send
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // If there is data to push then do so.
    if (m_tx_iptr != m_tx_indirect || m_txptr != m_txstart)
        if (send_queued(false) != OK)
            return true;

    AJI_DEBUG_ASSERT(m_tx_iptr == m_tx_indirect);
    AJI_DEBUG_ASSERT(m_txptr == m_txstart);

    return false;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int TCPLINK::send_queued(bool block)
//
// Description: Transfer data from our list of transmit vectors to the TCP
//              transmit queue.  If 'block' is true then this function will
//              block until all the data has been queued.  Otherwise it will
//              queue as much as possible and return.
//
// Returns:     OK if all data has been queued
//              WOULDBLOCK if some data remains unqueued (block == false only)
//              DISCONNECT if an error occurs
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // If we haven't added the last vector then do so now.
    if (m_txptr != m_txstart)
    {
        AJI_DEBUG_ASSERT(m_tx_iptr <= m_tx_indirect + INDIRECT_MAX - 1);

        m_tx_iptr->iov_base = (char *)m_txstart;
        m_tx_iptr->iov_len  = static_cast<DWORD> (m_txptr - m_txstart);
        m_tx_iptr++;

        m_txstart = m_txptr;
    }

    AJI_DEBUG_ASSERT(m_tx_iptr != m_tx_indirect);

    // Send any unsent data
    for ( ; ; )
    {
        DWORD indirect_n = static_cast<DWORD> (m_tx_iptr - m_tx_indirect);
        int n;
#if PORT == WINDOWS
        {
            DWORD  sent;
            n = WSASend(m_sock, m_tx_indirect, indirect_n, &sent, 0, NULL, NULL);
            if (n == 0)
                n = sent;
            else
                n = -WSAGetLastError();
        }
#else
        n = writev(m_sock, m_tx_indirect, indirect_n);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            n = -errno;
        }
#endif

        if (n < 0) // error
        {
            if (n == -SOCKERROR(EWOULDBLOCK))
                n = 0;
            else
            {
                close();
                return DISCONNECT;
            }
        }

        LOG(n);

        DWORD i = 0;

        while (n > 0)
        {
            if ((DWORD)n >= m_tx_indirect[i].iov_len)
            {
                n -= m_tx_indirect[i].iov_len;
                i++;
            }
            else
            {
                m_tx_indirect[i].iov_len  -= n;
                m_tx_indirect[i].iov_base  = static_cast<char *>(m_tx_indirect[i].iov_base) + n;
                break;
            }
        }

        if (i > 0)
        {
            if (i < indirect_n)
                memmove_s(m_tx_indirect, sizeof(m_tx_indirect), m_tx_indirect + i, (indirect_n - i) * sizeof(m_tx_indirect[0]));
            m_tx_iptr -= i;
        }

        if (m_tx_iptr == m_tx_indirect)
            break;

        LOG(0x02000000 | (m_tx_iptr - m_tx_indirect));

        if (!block)
            return WOULDBLOCK;

        if (!wait_for_space())
        {
            close();
            return DISCONNECT;
        }
    }

    AJI_DEBUG_ASSERT(m_tx_iptr == m_tx_indirect);

    // At this point we know there are no indirect blocks waiting to be added to
    // the kernel transmit queue.  Thus we can free up any buffers which these
    // might have been pointing to.

    m_txptr = m_txstart = m_txbuffer;

    delete_queue(&m_txdone);

    return OK;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
TXFIFO * TCPLINK::next_tx_fifo(bool expect_ready)
//
// Description: pick the next FIFO to transmit from.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    while (m_txfifo != NULL)
    {
        TXFIFO * fifo = (m_next_txfifo != NULL) ? m_next_txfifo : m_txfifo;

        // If the fifo is marked as interleave with next then we will try to send
        // some data from the next FIFO after sending it from this one.
        //TXFIFO * next = fifo->m_interleave ? fifo->m_next : NULL;

        DWORD len = fifo->get_data_avail(4096);

        if (len == 0)
            deactivate_tx_fifo(fifo);
        else
        {
            if (len <= 4096)
                return fifo;
            else // a value of 0xFFFFFFFF means "not yet"
            {
                AJI_DEBUG_ASSERT(!expect_ready);
                return NULL;
            }
        }
    }

    return NULL;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int TCPLINK::receive_cmnd(DWORD timeout)
//
// Description: Receive a packet of data on the command/response channel.
//
// Returns:     > 0 Length of received packet
//              WOULDBLOCK No received packed available and non-blocking selected
//              DISCONNECT Network connection lost
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    for ( ; ; )
    {
        int result = receive(NULL, true);

        if (result > 0)
            return result;

        else if (result == FIFODATA)
            continue;

        if (result == WOULDBLOCK && timeout != 0)
        {
            result = wait_for_data(timeout);
            if (result == OK)               
                continue;
        }

        return result;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool TCPLINK::wait_for_space(void)
//
// Description: The send queue on this socket is full.  Wait until there is
//              space to send more data.  During this time we must poll the
//              receive data as the other end might be in the same function.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(m_tx_iptr != m_tx_indirect);

    // We care if there is data to send or receive on this socket
    pollfd  fds[] = {  // Assume that m_sock doesn't change value during the lifespan of this TCPLINK object
#if PORT == WINDOWS
        { (SOCKET)m_sock, (INTEL_JTAG::POLL_EVENT_READ | INTEL_JTAG::POLL_EVENT_WRITE) }
#else
        { m_sock, (INTEL_JTAG::POLL_EVENT_READ | INTEL_JTAG::POLL_EVENT_WRITE) }
#endif
    };

	for ( ; ; )
    {
        // Wait until write space is available.  If there is no mux0 packet at the
        // head of the receive queue then we receive data if we would otherwise be
        // blocked.
		fds[0].events = m_rx_mux0_avail ? INTEL_JTAG::POLL_EVENT_WRITE : (INTEL_JTAG::POLL_EVENT_READ | INTEL_JTAG::POLL_EVENT_WRITE);
        int rc = INTEL_JTAG::poll(fds, (sizeof(fds) / sizeof(pollfd)), INTEL_JTAG::POLL_INDEFINITE_TIMEOUT);

        if (rc <= 0) // Error (or accidental timeout)
        {
#if PORT == UNIX
            if (errno == EINTR)
                continue;
#endif
            return false;
        }

        // If there is space to write then return now.
        if ( fds[0].revents & INTEL_JTAG::POLL_EVENT_WRITE )
            return true;

        if (!m_rx_mux0_avail && (fds[0].revents & INTEL_JTAG::POLL_EVENT_READ_IS_SET_EMU))
        {
            int result = receive(NULL, false);

            if (result == DISCONNECT)
                return false;

            // An OK result would mean we had to do something with the data returned.
            // We ignore it so this is an error.
            AJI_DEBUG_ASSERT(result != OK);
        }
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int TCPLINK::wait_for_data(DWORD timeout)
//
// Description: A receive on this socket had no data available.  Wait until
//              there is more data to receive or until the timeout specified
//              expires.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    bool forever = (timeout == ~0u);
    if (forever)
        timeout = 10000;

    LOG(0x82000000);

    // We care if there is data to send or receive on this socket
    pollfd  fds[] = {  // Assume that m_sock doesn't change value during the lifespan of this TCPLINK object
#if PORT == WINDOWS
        { (SOCKET)m_sock, INTEL_JTAG::POLL_EVENT_READ }
#else
        { m_sock, INTEL_JTAG::POLL_EVENT_READ }
#endif
    };
    static_assert((sizeof(fds) / sizeof(pollfd)) == 1, "unexpected fds array size");

	for ( ; ; )
    {
        int rc = INTEL_JTAG::poll(fds, (sizeof(fds) / sizeof(pollfd)), timeout);
        if (rc < 0)
        {
#if PORT == UNIX
            if (errno == EINTR)
                continue;
#endif
            close();
            return DISCONNECT; // Error
        }
        else if (rc == 0)
        {
            if (forever)
                continue;
            return WOULDBLOCK; // Timeout
        }

        // If there is data to read then return now.
        if (fds[0].revents & INTEL_JTAG::POLL_EVENT_READ_IS_SET_EMU)
            return OK;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool TCPLINK::poll_fifo(DWORD want_mux)
//
// Description: Poll the receive stream for data aimed at the FIFO specified.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    for ( ; ; )
    {
        DWORD mux;
        int result = receive(&mux, false);

        if (result == WOULDBLOCK)
        {
            result = wait_for_data(~0u);
            if (result == OK)
                continue;
        }
        if (result != FIFODATA)
            return false;

        AJI_DEBUG_ASSERT(mux >= FIFO_MIN);

        if (mux == want_mux+FIFO_MIN)
            return true;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int TCPLINK::receive(DWORD * mux_ptr, bool mux0_ok)
//
// Description: Internal receive function.  Receive and demultiplex packets.
//              This function never blocks, call wait_for_data() to do that.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // Receive the header into our local buffer (unless we have received it before)
    while (m_rxhdrvalid < 2)
    {
        int n = recv(m_sock, (char *)m_rxhdrbuffer + m_rxhdrvalid, 2 - m_rxhdrvalid, 0);

        if (n > 0)
            m_rxhdrvalid = static_cast<BYTE>(m_rxhdrvalid + n);
        else if (n == 0)
        {
            close();
            return DISCONNECT;
        }
        else // (n < 0)
        {
            int err = WSAGetLastError();
#if PORT == UNIX
            if (err == EAGAIN)
                err = EWOULDBLOCK;
            else if (err == EINTR)
                continue;
#endif
            if (err == SOCKERROR(EWOULDBLOCK))
                return WOULDBLOCK;
            else
            {
#ifdef _DEBUG
                logprintf(DLOG_DEBUG, "Link lost, error %d\n", err);
#endif
                close();
                return DISCONNECT;
            }
        }

        LOG(0x80000000 | n);
    }

    DWORD header = (m_rxhdrbuffer[0] << 8) | m_rxhdrbuffer[1];
    DWORD expect = 1 + (header & 0x0FFF);
    DWORD mux = (header & 0xF000) >> 12;

    // TODO: if we are allowed to then receive into the response buffer but hide
    // the received response until later.

    if (mux == 0 && !mux0_ok)
    {
        m_rx_mux0_avail = true;
        return NODATA;
    }

    if (m_rxpacket == NULL)
    {
        if (mux >= FIFO_MIN && mux <= FIFO_MAX && m_rxmux[mux-FIFO_MIN] != NULL)
        {
            // If the FIFO can cope with receiving data directly into its buffers then
            // receive directly into those buffers (and save a memcpy later).
            m_rxpacket = m_rxmux[mux-FIFO_MIN]->get_receive_buffer(expect);
        }
        else if (mux == 0)
            m_rxpacket = get_mux0_receive_buffer();
        else
            m_rxpacket = m_rxbuffer;

        // TODO: handle this error case properly rather than asserting.
        AJI_DEBUG_ASSERT(m_rxpacket != NULL);
    }

    while (m_rxvalid < expect)
    {
        int n = recv(m_sock, (char *)m_rxpacket + m_rxvalid, expect - m_rxvalid, 0);

        if (n > 0)
            m_rxvalid += n;
        else if (n == 0)
        {
            close();
            return DISCONNECT;
        }
        else // (n < 0)
        {
            int err = WSAGetLastError();
#if PORT == UNIX
            if (err == EAGAIN)
                err = EWOULDBLOCK;
            else if (err == EINTR)
                continue;
#endif
            if (err == SOCKERROR(EWOULDBLOCK))
                return WOULDBLOCK;
            else
            {
#ifdef _DEBUG
                logprintf(DLOG_DEBUG, "Link lost, error %d\n", err);
#endif
                close();
                return DISCONNECT;
            }
        }

        LOG(0x81000000 | n | (mux << 20));
    }

    if (mux_ptr != NULL)
        *mux_ptr   = mux;

    m_rxhdrvalid = 0;
    m_rxvalid  = 0;
    m_rxpacket = NULL;

    if (mux >= FIFO_MIN && mux <= FIFO_MAX)
    {
        RXFIFO * fifo = m_rxmux[mux-FIFO_MIN];

        // Receive some data.  If this is the last data required by the FIFO then
        // remove it from the queue and let the next FIFO do something with the data.
        if (fifo != NULL && fifo->receive_data(expect))
        {
            release_rxmux(fifo, mux - FIFO_MIN);
            fifo->deactivate();
        }

        return FIFODATA;
    }
    else if (mux == 0)
    {
        m_rx_mux0_avail = false;
        return expect;
    }
    else if (mux == 1)
    {
        process_notify(m_rxbuffer, expect);

        return FIFODATA;
    }
    else // (unused muxes) // Ignore the data
    {
        return FIFODATA;
    }

}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void TCPLINK::add_rxmux(RXFIFO * fifo, DWORD mux)
{
    AJI_DEBUG_ASSERT(mux <= FIFO_MAX - FIFO_MIN);

    if (m_rxmux[mux] == NULL)
        m_rxmux[mux] = fifo;
    else
    {
        RXFIFO * last = m_rxmux[mux];

        while (last->m_next != NULL)
            last = last->m_next;

        last->m_next = fifo;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

void TCPLINK::release_rxmux(RXFIFO * fifo, DWORD mux)
{
    AJI_DEBUG_ASSERT(mux <= FIFO_MAX - FIFO_MIN);

    if (m_rxmux[mux] == fifo)
        m_rxmux[mux] = fifo->m_next;
    else
    {
        // Not at the front,  Find it in the chain and remove it
        RXFIFO * last = m_rxmux[mux];
        while (last != NULL)
            if (last->m_next != fifo)
                last = last->m_next;
            else
            {
                last->m_next = fifo->m_next;
                break;
            }
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void TCPLINK::add_txmux(TXFIFO * fifo)
{
    if (m_txfifo == NULL)
        m_txfifo = fifo;
    else
    {
        TXFIFO * last = m_txfifo;

        while (last->m_next != NULL)
            last = last->m_next;

        last->m_next = fifo;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

void TCPLINK::release_txmux(TXFIFO * fifo)
{
    if (m_txfifo == fifo)
        m_txfifo = fifo->m_next;
    else
    {
        // Not at the front,  Find it in the chain and remove it
        TXFIFO * last = m_txfifo;
        while (last != NULL)
            if (last->m_next != fifo)
                last = last->m_next;
            else
            {
                last->m_next = fifo->m_next;
                break;
            }
    }

    if (m_next_txfifo == fifo)
        m_next_txfifo = fifo->m_next;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

TXFIFO::TXFIFO(TCPLINK * link)
    : m_link(link), m_mux(INACTIVE), m_next(NULL)
{
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

TXFIFO::~TXFIFO(void)
{
    if (m_mux != INACTIVE)
        m_link->release_txmux(this);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void TXFIFO::activate(DWORD mux, bool interleave)
{
    m_mux = mux;
    m_interleave = interleave;

    m_link->add_txmux(this);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

RXFIFO::RXFIFO(TCPLINK * link)
    : m_link(link), m_mux(INACTIVE), m_next(NULL)
{
    AJI_DEBUG_ASSERT(link != NULL);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

RXFIFO::~RXFIFO(void)
{
    if (m_mux != INACTIVE)
        m_link->release_rxmux(this, m_mux);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void RXFIFO::activate(DWORD mux)
{
    m_mux = mux;
    m_link->add_rxmux(this, m_mux);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void RXFIFO::deactivate(void)
{
    m_mux = INACTIVE;
}
