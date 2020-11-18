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

//# START_MODULE_HEADER/////////////////////////////////////////////////////
//#
//# Filename:    jtag_client_chain.h
//#
//# Description: 
//#
//# Authors:     Andrew Draper
//#
//#              Copyright (c) Altera Corporation 2000
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

#ifndef INC_JTAG_CLIENT_CHAIN_H
#define INC_JTAG_CLIENT_CHAIN_H

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_JTAG_CLIENT_AJI_H
#include "jtag_client_aji.h"
#endif

class AJI_CLIENT;
class AJI_CHAIN;

typedef std::set< AJI_CHAIN *, std::less< AJI_CHAIN * > >  AJI_CHAIN_SET;

static const int MAX_DUMMY_BITS = 16;  // max number of dummy bits tested for

#if PORT == WINDOWS
extern HINSTANCE jtag_client_module;
#endif

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class AJI_CHAIN_JS : public AJI_CHAIN
{
public:
    static AJI_ERROR get_hardware     (DWORD              * hardware_count,
                                       AJI_HARDWARE       * hardware_list,
                                       char             * * server_version_info_list,
                                       DWORD                timeout);

    static AJI_ERROR find_hardware    (DWORD                persistent_id,
                                       AJI_HARDWARE       * hardware,
                                       DWORD                timeout);

    static AJI_ERROR find_hardware    (const char         * hw_name,
                                       AJI_HARDWARE       * hardware,
                                       DWORD                timeout);

    AJI_ERROR print_hardware_name     (char               * hw_name,
                                       DWORD                hw_name_len,
                                       bool                 explicit_localhost,
                                       DWORD              * needed_hw_name_len);

    static AJI_ERROR add_hardware     (const AJI_HARDWARE * hardware);

    AJI_ERROR remove_hardware         (void);

    AJI_ERROR cancel_operation        (void);

    AJI_ERROR lock                    (DWORD timeout);

    AJI_ERROR unlock                  (void);

    AJI_ERROR scan_device_chain       (void);

    AJI_ERROR define_device           (DWORD                tap_position, 
                                       const AJI_DEVICE   * device);

    AJI_ERROR read_device_chain       (DWORD              * device_count,
                                       AJI_DEVICE         * device_list,
                                       bool                 auto_scan);

    DWORD get_device_features         (DWORD tap_position);

    AJI_ERROR set_parameter           (const char         * name,
                                       DWORD                value);

    AJI_ERROR set_parameter           (const char         * name,
                                       const BYTE         * value,
                                       DWORD                valuelen);

    AJI_ERROR get_parameter           (const char         * name,
                                       DWORD              * value);

    AJI_ERROR get_parameter           (const char         * name,
                                       BYTE               * value,
                                       DWORD              * valuemax,
                                       DWORD                valuetx);

    AJI_ERROR open_device             (DWORD                tap_position,
                                       AJI_OPEN_ID        * open_id,
                                       const AJI_CLAIM2   * claims,
                                       DWORD                claim_n,
                                       const char         * application_name);

    AJI_ERROR open_entire_chain       (AJI_OPEN_ID        * open_id,
                                       AJI_CHAIN_TYPE       style,
                                       const char         * application_name);

    AJI_ERROR watch_data(WATCH_FN * watch_fn, void * handle, bool intrusive);


     AJI_ERROR calculate_user_mode_dummy_bits(AJI_OPEN_JS   * open );

     void memset64                    (BYTE               * dest,
                                       QWORD                value);

     AJI_ERROR byte_array_shift_left  (BYTE               * dest_arr,
                                       const BYTE         * src_arr,
                                       int                  arr_size,
                                       int                  shift);

public:
    // For use by AJI_CLIENT only

    AJI_CHAIN_JS(AJI_CLIENT * m_client, unsigned int m_chain_id, AJI_CHAIN_TYPE chain_type, DWORD features);
    ~AJI_CHAIN_JS(void);
    unsigned int get_id(void) { return m_chain_id; }

    static bool strings_match(const char * one, size_t size, const char * two);
    void set_strings(const char * hw_name, const char * port, const char * device, const char * server);
    void set_features(DWORD features) { m_features = features; }
    bool is_dummy() { return (m_features & AJI_FEATURE_DUMMY) != 0; }

    void invalidate(AJI_CLIENT * client);

    inline bool valid(void);
    inline const AJI_CLIENT * get_client(void) const { return m_client; }
    inline unsigned int get_chain_id(void) { return m_chain_id; }

    inline bool get_locked(void) const { return m_locked; }
    inline void set_locked(bool locked) { m_locked = locked; }

    bool read_blocker_info(RXMESSAGE * rx);
    static void clear_blocker_info();

private:
    static AJI_CHAIN_SET m_valid_set;

    static bool match_hardware(const char * one, const char * two);
    static bool strings_match_nocase(const char * one, const char * two);

    operator AJI_HARDWARE(void);
    AJI_ERROR refresh_chain(bool forcescan, bool autoscan);

    AJI_ERROR send_receive(RXMESSAGE * * rx_ptr);

    AJI_CLIENT * m_client;
    unsigned int m_chain_id;
    bool         m_valid;

    char       * m_hw_strings;
    char       * m_hw_name;
    char       * m_port_name;
    char       * m_device_name;
    char       * m_server_name;
    AJI_CHAIN_TYPE m_chain_type;
    DWORD        m_features;
    DWORD        m_dummy_bits;
    bool         m_dummy_bits_calculated;
    enum CLAIM_STYLE { NOCLAIM, SAFECLAIM, IFAVAIL };
    CLAIM_STYLE claimed_class(const AJI_CLAIM2 * claims, DWORD claim_n, AJI_CLAIM_TYPE type, QWORD value, DWORD length = 0);

    // This mutex protects the device list m_taps and its associated data from
    // reading and writing at the same time.  The link mutex is also required for
    // writing it.
    JTAG_MUTEX   m_taps_mutex;

    class DEVICE_INFO
    {
    public:
        AJI_DEVICE   m_device;
    };

    typedef std::vector< DEVICE_INFO > TAPS;
    TAPS         m_taps;
    unsigned int m_chain_tag;
    char       * m_strings;

    bool         m_locked;

    WATCH_FN   * m_watch_fn;
    void       * m_watch_handle;
};

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline bool AJI_CHAIN_JS::valid(void)
//
// Description: Check that the chain pointer (passed in as 'this') is valid.
//              If the pointer is to a valid chain which has returned a fatal
//              error then invalidate it and return false.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
#if PORT == WINDOWS
    // Nothing is valid after Windows has started to unload the DLL.
    if (jtag_client_module == NULL)
        return false;
#endif
    if (m_valid_set.find(this) == m_valid_set.end())
        return false;

    return m_valid;
}

#endif
