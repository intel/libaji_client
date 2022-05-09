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
//# Filename:    jtag_message.h
//#
//# Description: 
//#
//# Authors:     Andrew Draper
//#
//#              Copyright (c) Altera Corporation 2000 - 2013
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
//# This file describes the protocol used between AJI client and AJI server.
//# You MUST maintain backwards compatibility when editing this file.

#ifndef INC_JTAG_MESSAGE_H
#define INC_JTAG_MESSAGE_H

//# INCLUDE FILES //////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_CSTRING
#include <cstring>
#define INC_CSTRING
#endif

#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif

#ifndef INC_AJI_H
#include "aji.h"
#endif

#ifndef INC_GEN_STRING_SYS_H
#include "gen_string_sys.h"
#endif

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

#define AJI_SIGNATURE "JTAG Server\r\n"

// If a client is happy with version (x) then it will operate correctly with
// versions between (x) and (x | 0xFFFF).

// The current protocol version.
// TODO: increment this for each release where the protocol changes.
#define AJI_CURRENT_VERSION (13)

// JTAG Server with version 0 shipped with Quartus II 1.1
//                          1              Quartus II 2.0
//                          2              Quartus II 2.1
//                          3              Quartus II 2.2
//                          4              Quartus II 3.0
//                          5              Quartus II 4.0
//                          6              Quartus II 4.1
//                          7              Quartus II 4.2
//                          8              Quartus II 8.0
//                          9              Quartus II 11.0
//                         10              Quartus II 12.1
//                         11              Quartus II 13.1
//                         12              Quartus II 14.0
//                         13              Quartus II 17.0

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class MESSAGE
{
public:
    /* Do not reuse enum value, i.e. extend the list
     * by extend the enumeration value, do not
     * attempt to use values in the gap of enum value
     * This is to prevent confusion if this
     * client is connected to older server that
     * interprets the enum differently
     */
    enum COMMAND                       // Implemented in server with version:
    {                                  // [Deprecated in these versions]
        GET_HARDWARE             = 0x80, // 0+
        ADD_HARDWARE             = 0x81, // 0+

        GET_VERSION_INFO         = 0x83, // 2+
        GET_DEFINED_DEVICES      = 0x84, // 2+

        GET_QUARTUS_DEVICES      = 0x89, // 9+
        PROGRESS                 = 0x9E, // 0+
        OUTPUT                   = 0x9F, // 0+

        REMOVE_HARDWARE          = 0xA1, // 0+
        LOCK_CHAIN               = 0xA2, // 0+
        UNLOCK_CHAIN             = 0xA3, // 0+
        SCAN_CHAIN               = 0xA4, // 0+
        READ_CHAIN               = 0xA5, // 0+

        DEFINE_DEVICE            = 0xA7, // 0+
        OPEN_DEVICE              = 0xA8, // 0+
        OPEN_ENTIRE_CHAIN        = 0xA9, // 1+
        SET_PARAMETER            = 0xAA, // 2+
        GET_PARAMETER            = 0xAB, // 2+

        GET_PARAMETER_BLOCK      = 0xAD, // 2+
        WATCH_DATA               = 0xBE, // 0+

        CLOSE_DEVICE             = 0xC0, // 0+
        LOCK_DEVICE              = 0xC1, // 0+
        UNLOCK_DEVICE            = 0xC2, // 0+

        ACCESS_IR                = 0xC5, // 0+
        ACCESS_DR                = 0xC7, // 0+
        ANY_SEQUENCE             = 0xC9, // [1-2] 3+
        RUN_TEST_IDLE            = 0xCA, // 0+
        TEST_LOGIC_RESET         = 0xCB, // 0+
        DELAY_MICROSECONDS       = 0xCE, // 0+
        ACCESS_IR_FIFO           = 0xCF, // 1+
        UNLOCK_LOCK_DEVICE       = 0xD3, // 2+

        AUTHENTICATE_MD5         = 0xF0, // 0+
        CHECK_PASSWORD           = 0xF1, // 10+
        PING                     = 0xFD, // 12+ for command, notify ignored by all clients
        USE_PROTOCOL_VERSION     = 0xFE, // 2+
        CONTINUE_COMMANDS        = 0xFF  // 0+
    };
};

enum AJI_HW_TYPE // These are transmitted from client to server so must not change.
{
    AJI_HW_OTHER         = 0,
    AJI_HW_BYTEBLASTER   = 1,
    AJI_HW_MASTERBLASTER = 2 // Deprecated
};

enum ALLOW_EVENTS
{
    ALLOW_TEST_LOGIC_RESET     =    0
};

enum SERVER_FLAGS
{
    SERVER_ALLOW_REMOTE = 1
};

const size_t APP_NAME_BUFFER_SIZE = 32;
const size_t HOSTNAME_BUFFER_SIZE = 256;
const size_t PASSWORD_BUFFER_SIZE = 64;
const size_t PARAM_NAME_BUFFER_SIZE = 256; // PARAM_NAME_BUFFER_SIZE
const size_t RECORD_NAME_BUFFER_SIZE = 256;
const size_t SERVER_VERSION_INFO_BUFFER_SIZE = 1024;

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class TXMESSAGE : public MESSAGE
{
public:
    TXMESSAGE(void)
        : m_start(NULL), m_blockstart(NULL), m_ptr(NULL), m_end(NULL) { }

    TXMESSAGE(BYTE * start, DWORD len)
        : m_start(start), m_blockstart(NULL), m_ptr(start), m_end(start + len) { }

    void set_buffer(BYTE * start, DWORD len)
        { m_start = start; m_blockstart = NULL; m_ptr = start; m_end = start + len; }

    bool add_command(COMMAND cmnd, DWORD len = 0);
    void start_response(void);
    void end_response(AJI_ERROR resp);

    void add_int(DWORD value);
    void add_long(QWORD value);
    void add_string(const char * string);
    void add_raw(const void * raw, DWORD len);

    BYTE * add_int_later(void);
    void fill_int(BYTE * where, DWORD value);

    const BYTE * get_data(void) const   { return m_start; }
    DWORD get_length(void) { end_block(); return static_cast<DWORD> (m_ptr - m_start); }

    struct STATE
    {
        BYTE * m_ptr;
    };

    void save_state(STATE * state) const;
    void restore_state(const STATE * state);

protected:
    BYTE * m_start;
    BYTE * m_blockstart;
    BYTE * m_ptr;
    BYTE * m_end;

    void end_block(void);
};

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class RXMESSAGE : public MESSAGE
{
public:
    RXMESSAGE(void)
        : m_ptr(NULL), m_end(NULL), m_blockend(NULL),
          m_string(NULL), m_string_end(NULL) { }

    RXMESSAGE(const BYTE * buffer, DWORD len)
        : m_ptr(buffer), m_end(m_ptr + len), m_blockend(m_ptr),
          m_string(NULL), m_string_end(NULL) { }

    void set_buffer(const BYTE * buffer, DWORD len)
        { m_ptr = m_blockend = buffer; m_end = m_ptr + len; }

    void set_string(char * string, DWORD len);
    DWORD get_string_overflow(void) { return static_cast<DWORD> (m_string - m_string_end); }

    void single_block_buffer(void) { m_blockend = m_end; }

    bool remove_command(COMMAND * cmnd);
    bool remove_response(AJI_ERROR * resp);
    bool remove_int(DWORD * value);
    bool remove_long(QWORD * value);
    bool remove_string(const char * * string);
    bool remove_raw(void * raw, DWORD len);

    void skip_remaining(void) { m_ptr = m_blockend; }

    bool empty(void) { return m_ptr == m_end; };

    struct STATE
    {
        const BYTE * m_ptr;
        const BYTE * m_blockend;
        char * m_string;
    };

    void save_state(STATE * state) const;
    void restore_state(const STATE * state);

protected:
    const BYTE * m_ptr;
    const BYTE * m_end;
    const BYTE * m_blockend;

    char * m_string;
    char * m_string_end;
};

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void TXMESSAGE::save_state(STATE * state) const
{
    AJI_DEBUG_ASSERT(m_blockstart == NULL);

    state->m_ptr = m_ptr;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void TXMESSAGE::restore_state(const STATE * state)
{
    m_ptr    = state->m_ptr;
    m_blockstart = NULL;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline void TXMESSAGE::end_block(void)
{
    if (m_blockstart != NULL)
    {
        DWORD blen = static_cast<DWORD> (m_ptr - m_blockstart);
        AJI_DEBUG_ASSERT(blen >= 4);

        DWORD len = (m_blockstart[2] << 8) | m_blockstart[3];
        AJI_DEBUG_ASSERT(len == 0 || len == blen);

        if (len == 0)
        {
            // If length wasn't filled in at the start of the block then do it now.
            m_blockstart[2] = static_cast<BYTE>(blen >> 8);
            m_blockstart[3] = static_cast<BYTE>(blen);
        }

        m_blockstart = NULL;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline bool TXMESSAGE::add_command(COMMAND cmnd, DWORD len)
{
    AJI_DEBUG_ASSERT(len == 0 || len >= 4);

    if (m_ptr + len > m_end)
        return false;
    AJI_DEBUG_ASSERT(m_ptr + 4 <= m_end);

    end_block();
    m_blockstart = m_ptr;

    *m_ptr++ = static_cast<BYTE>(cmnd);
    *m_ptr++ = 0;

    *m_ptr++ = static_cast<BYTE>(len >> 8);
    *m_ptr++ = static_cast<BYTE>(len);

    // If len == 0 then end_block() will replace the size provided here with the
    // real size.  If len > 0 then end_block() will check there is no overflow.

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void TXMESSAGE::start_response(void)
{
    AJI_DEBUG_ASSERT(m_ptr + 4 <= m_end);
    AJI_DEBUG_ASSERT(m_blockstart == NULL);

    m_blockstart = m_ptr;

    *m_ptr++ = static_cast<BYTE>(AJI_INTERNAL_ERROR); // end_response will fill in
    *m_ptr++ = 0;

    *m_ptr++ = 0; // end_block() will put the command length in these bytes
    *m_ptr++ = 0;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void TXMESSAGE::end_response(AJI_ERROR resp)
{
    AJI_DEBUG_ASSERT(m_blockstart != NULL);

    m_blockstart[0] = static_cast<BYTE>(resp);
    end_block();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void TXMESSAGE::add_int(DWORD value)
{
    AJI_DEBUG_ASSERT(m_ptr + 4 <= m_end);

    for (int i = 0 ; i < 4 ; i++, value <<= 8)
        *m_ptr++ = static_cast<BYTE>(value >> 24);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void TXMESSAGE::add_long(QWORD value)
{
    AJI_DEBUG_ASSERT(m_ptr + 8 <= m_end);

    for (int i = 0 ; i < 8 ; i++, value <<= 8)
        *m_ptr++ = static_cast<BYTE>(value >> 56);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void TXMESSAGE::add_string(const char * string)
{
    size_t len = strnlen_s(string, 512);

    AJI_DEBUG_ASSERT(m_ptr + 1 + len <= m_end && len < 512);

    *m_ptr++ = static_cast<BYTE>(len);

    memcpy_s(m_ptr, (size_t)(m_end-m_ptr), string, len);
    m_ptr += len;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void TXMESSAGE::add_raw(const void * raw, DWORD len)
{
    AJI_DEBUG_ASSERT(m_ptr + len <= m_end);

    memcpy_s(m_ptr, (size_t)(m_end-m_ptr), raw, len);
    m_ptr += len;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline BYTE * TXMESSAGE::add_int_later(void)
{
    AJI_DEBUG_ASSERT(m_ptr + 4 <= m_end);

    BYTE * ptr = m_ptr;
    m_ptr += 4;
    return ptr;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void TXMESSAGE::fill_int(BYTE * ptr, DWORD value)
{
    for (int i = 0 ; i < 4 ; i++, value <<= 8)
        *ptr++ = static_cast<BYTE>(value >> 24);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void RXMESSAGE::set_string(char * string, DWORD len)
{
    m_string     = string;
    m_string_end = string + len;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void RXMESSAGE::save_state(STATE * state) const
{
    state->m_ptr    = m_ptr;
    state->m_blockend = m_blockend;
    state->m_string = m_string;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline void RXMESSAGE::restore_state(const STATE * state)
{
    m_ptr    = state->m_ptr;
    m_blockend = state->m_blockend;
    m_string = state->m_string;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline bool RXMESSAGE::remove_command(COMMAND * cmnd)
{
    AJI_DEBUG_ASSERT(cmnd != NULL);

    // Skip over unused bytes at end of last command
    m_ptr = m_blockend;

    m_blockend = m_ptr + (m_ptr[2] << 8) + m_ptr[3];
    if (m_blockend > m_end || m_ptr + 4 > m_blockend)
        return false;

    *cmnd = static_cast<COMMAND>(m_ptr[0]);

    m_ptr += 4;

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline bool RXMESSAGE::remove_response(AJI_ERROR * resp)
{
    AJI_DEBUG_ASSERT(resp != NULL);

    // Skip over unused bytes at end of last response
    m_ptr = m_blockend;

    m_blockend = m_ptr + (m_ptr[2] << 8) + m_ptr[3];
    if (m_blockend > m_end)
        return false;

    * resp = static_cast<AJI_ERROR>(m_ptr[0]);

    m_ptr += 4;

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline bool RXMESSAGE::remove_int(DWORD * value)
{
    AJI_DEBUG_ASSERT(value != NULL);

    if (m_ptr + 4 > m_blockend)
        return false;

    DWORD v = 0;

    for (int i = 0 ; i < 4 ; i++)
        v = (v << 8) | *m_ptr++;

    * value = v;
    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline bool RXMESSAGE::remove_long(QWORD * value)
{
    AJI_DEBUG_ASSERT(value != NULL);

    if (m_ptr + 8 > m_blockend)
        return false;

    QWORD v = 0;

    for (int i = 0 ; i < 8 ; i++)
        v = (v << 8) | *m_ptr++;

    * value = v;
    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

inline bool RXMESSAGE::remove_string(const char * * string)
{
    AJI_DEBUG_ASSERT(string != NULL);

    if (m_ptr + 1 > m_blockend)
        return false;

    DWORD n = *m_ptr++;
    if (m_ptr + n > m_blockend)
        return false;

    if (m_string + n < m_string_end)
    {
        *string = m_string;
        memcpy_s(m_string, (size_t)(m_string_end-m_string), m_ptr, n);
        m_string[n] = 0;
    }
    else
        *string = NULL;

    m_ptr += n;
    m_string += n + 1;

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline bool RXMESSAGE::remove_raw(void * raw, DWORD len)
//
// Description: remove raw bytes from the message.  If raw==NULL then the bytes
//              removed are discarded.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (m_ptr + len > m_blockend)
        return false;

    if (raw != NULL)
        memcpy_s(raw, len, m_ptr, len);
    m_ptr += len;

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

#endif
