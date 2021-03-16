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
//# Filename:    jtag_raw_fifo.h
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

#ifndef INC_JTAG_RAW_FIFO_H
#define INC_JTAG_RAW_FIFO_H

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class TXRAWFIFO : public TXFIFO
{
public:
    TXRAWFIFO(TCPLINK * link, DWORD length, const BYTE * data);
    ~TXRAWFIFO(void);

    void invert_data(void);
    bool copy_data(void);
    DWORD get_length(void) { return static_cast<DWORD> (m_end - m_data); }

private:
    virtual DWORD get_data_avail(DWORD max);
    virtual const BYTE * get_data(DWORD max);

    BYTE * m_data;
    BYTE * m_ptr;
    BYTE * m_end;
    bool   m_autofree;

    static const BYTE s_zeros[256];
};

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class RXRAWFIFO : public RXFIFO
{
public:
    RXRAWFIFO(TCPLINK * link, DWORD length = 0, BYTE * buffer = NULL);
    ~RXRAWFIFO(void);

    bool wait_for_data(DWORD length = 0);
    BYTE * get_data(void) { return m_data; }

private:
    virtual BYTE * get_receive_buffer(DWORD len);
    virtual bool receive_data(DWORD len);

    BYTE * m_data;
    BYTE * m_ptr;
    BYTE * m_end;
    bool   m_ok;
    bool   m_autofree;
    bool   m_length_known;
};

#endif
