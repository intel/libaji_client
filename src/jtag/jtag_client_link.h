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

#ifndef INC_JTAG_CLIENT_H
#define INC_JTAG_CLIENT_H

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_VECTOR
#include <vector>
#define INC_VECTOR
#endif

#ifndef INC_JTAG_PLTAFORM_H
#include "jtag_platform.h"
#endif

#ifndef INC_JTAG_TCPCLIENT_H
#include "jtag_tcpclient.h"
#endif

#ifndef INC_JTAG_MESSAGE_H
#include "jtag_message.h"
#endif

#ifndef INC_JTAG_PROTOCOL_H
#include "jtag_protocol.h"
#endif

#ifndef INC_JTAG_COMMON_H
#include "jtag_common.h"
#endif

class TXMESSAGE;
class RXMESSAGE;
class AJI_CLIENT;
class AJI_CHAIN_JS;
class AJI_OPEN_JS;

struct const_char_less_hostname
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp_(s1, HOSTNAME_BUFFER_SIZE, s2) < 0;
  }
};

typedef std::vector < const char * > AJI_STRING_VEC;
typedef std::vector < AJI_CLIENT * > AJI_CLIENT_VEC;
typedef std::vector < AJI_DEVICE   > AJI_DEVICE_VEC;

struct AJI_SERVER_INFO
{
    const char * m_server;
    const char * m_password;
    const AJI_CLIENT * m_primary;
};

typedef std::map< const char *, AJI_SERVER_INFO *, const_char_less_hostname >  STRING_SERVER_MAP;

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class AJI_CLIENT : public TCPCLIENT
{
public:
    static AJI_ERROR check_password(DWORD authtype, const BYTE * challenge, const BYTE * response);
//    static AJI_ERROR get_potential_hardware(DWORD         * hardware_count,
//                                       AJI_HARDWARE       * hardware_list);

    static AJI_ERROR add_remote_server(const char * server, const char * password, bool temporary);
//    static AJI_ERROR get_servers(DWORD * count, const char * * servers, bool temporary);

    static AJI_ERROR enable_remote_clients(bool enable, const char * password);
    static AJI_ERROR get_remote_clients_enabled(bool * enable);

//    static AJI_ERROR set_record(const AJI_RECORD * record, bool add);
//    static AJI_ERROR get_records(const char * server, DWORD * record_n, AJI_RECORD * records, DWORD autostart_type_n, const char * * autostart_types);

    static AJI_ERROR replace_local_jtagserver(const char * replace);
//    static AJI_ERROR enable_local_jtagserver(bool enable);
//    static AJI_ERROR configuration_in_memory(bool enable);
//    static AJI_ERROR load_all_quartus_devices(const char * filename);

    static const char * get_server_version_info(void);
    static const char * get_server_path(void);

    class CHAINPTR
    {
    public:
        AJI_CHAIN_JS * m_chain;
        bool        m_destroy;
    };

    typedef std::vector < CHAINPTR > CHAINS;

    AJI_CLIENT(DWORD id, const char * hostname, bool remote, const AJI_CLIENT * primary);
    ~AJI_CLIENT(void);

    static AJI_CLIENT * local_server(void);

    AJI_ERROR remove_remote_server(void);

    DWORD get_id(void) const { return m_id; }
    bool is_remote_server(void) const { return m_remote_server; }
    const char * server_name(void) const { return m_hostname; }

    bool connecting(void) const { return m_state == CONNECTING; }
    bool connected(void) const { return m_state == CONNECTED; }

    bool version_ok(DWORD min, DWORD max) const { return m_server_version >= min && m_server_version <= max; }
    DWORD get_server_version(void) const { return m_server_version; }
    const char * get_host_server_version_info(void) { return m_server_version_info; }

    inline bool connect(void);
    inline void disconnect(void) { disconnect(false); }

    void refresh_hardware(void) { refresh_hardware(false); }
    const CHAINS * get_chains(void) const { return &m_chains; }

    static AJI_ERROR define_device(const AJI_DEVICE * device, bool define);
    static AJI_ERROR get_defined_devices(DWORD * device_count, AJI_DEVICE * device_list);
//    static AJI_ERROR get_local_quartus_devices(DWORD * device_count, AJI_DEVICE * device_list);

    AJI_ERROR define_device(DWORD chain_id, DWORD tap_position, const AJI_DEVICE * device);

    TXMESSAGE * get_txmessage(AJI_OPEN_JS * open);
    void txmessage_cancel(AJI_OPEN_JS * open);
    inline void txmessage_transfer(AJI_OPEN_JS * new_open) { m_partial = new_open; }
    inline bool txmessage_is_continue();

    RXMESSAGE * get_rxmessage(void) { return &m_rx; }

    inline bool try_claim_link(DWORD ms_timeout);
    inline void release_link(void);
    inline bool link_is_claimed(void) {
printf("%s:%d m_link_mutex=%p\n", __FILE__, __LINE__, &m_link_mutex);
        return m_link_mutex.is_claimed();
    }

    inline AJI_ERROR send_receive(void);

    bool send(void);
    AJI_ERROR receive_timeout(DWORD timeout);

    // Vector of all the links which are active or possible at the moment
    static AJI_CLIENT_VEC m_link;

    static void update_hardware(DWORD timeout);
    static DWORD update_servers(void);
    static void connect_all(DWORD timeout);
    static void disconnect_all(bool process_terminate);

    static void register_output_callback(void ( * output_fn)(void * handle, DWORD level, const char * line), void * handle);

private:
    static DWORD find_server(const char * server);
    static void add_temporary_server_internal(const char * server, const char * password, AJI_CLIENT * primary);
    static AJI_ERROR check_remote_clients_possible(void);
//    AJI_ERROR refresh_records(DWORD * record_n, AJI_RECORD * records, DWORD autostart_type_n, const char * * autostart_types);

    bool internal_connect(void);
    bool start_connect(void);
    int  continue_connect(void);
    bool start_local_server(void);
    bool prepare_connection(void);
    bool refresh_hardware(bool check_secondaries);
    bool get_password(char * password, int passlen) const;
    bool update_secondaries(void);
    void clear_tx_buffer(void);

    void disconnect(bool process_terminate);

//    AJI_ERROR refresh_potential(DWORD * hardware_count, AJI_HARDWARE * hardware_list);
    DWORD refresh_defined_devices(void);
    DWORD refresh_quartus_devices(DWORD * expected_count, AJI_DEVICE * device_list);

    AJI_ERROR load_quartus_devices(const char * data, DWORD length, DWORD version);
    AJI_ERROR load_quartus_devices(const char * filename);

    bool get_hardware_from_server(void);
    void refresh_chain(unsigned int chain_id, const char * hw_name,
                       const char * port, const char * device_name,
                       AJI_CHAIN_TYPE chain_type, DWORD features);

    virtual BYTE * get_mux0_receive_buffer(void);

    virtual void process_notify(const BYTE * data, DWORD len);

    static STRING_SERVER_MAP s_temporary_servers;

    const DWORD m_id;

    char     * m_hostname;
    bool       m_remote_server;
    const AJI_CLIENT * m_primary;

    enum CSTATE { FAIL, LOOKUP, FIRSTTRY, RETRY, WAITSERVER, SUCCESS };
    CSTATE     m_connect_state;
    int        m_connect_timeout;

    enum STATE { IDLE, CONNECTING, CONNECTED, NOSERVER, BADVERSION, AUTHFAIL, BROKEN };
    STATE      m_state;
    int        m_retry_connect;

    DWORD      m_server_version;
    DWORD      m_server_flags;
    char     * m_server_version_info;
    char     * m_server_path;
    DWORD      m_pgmparts_version; //Not in use, set to zero
    
    char     * m_potential_strings;

    char     * m_record_strings;
    DWORD    * m_record_numbers;

    AJI_DEVICE_VEC m_defined;
    char     * m_defined_strings;
    char     * m_quartus_strings;
    DWORD      m_defined_tag;

    CHAINS     m_chains;

    BYTE       m_txbuffer[4096];
    BYTE       m_rxbuffer[4096];
    TXMESSAGE  m_tx;
    RXMESSAGE  m_rx;

#ifdef _DEBUG
    DWORD      m_sequence;
#endif

    AJI_OPEN_JS * m_partial;

    // This Mutex protects the TCP connection to the client and the buffers
    // associated with it.
    JTAG_MUTEX m_link_mutex;

    static void ( * m_output_fn)(void * handle, DWORD level, const char * line);
    static void * m_output_handle;
};

inline bool AJI_CLIENT::try_claim_link(DWORD ms_timeout)
{
    return m_link_mutex.try_claim(ms_timeout);
}

inline void AJI_CLIENT::release_link(void)
{
    m_link_mutex.release();
}

inline AJI_ERROR AJI_CLIENT::send_receive(void)
{
    if (!send())
        return AJI_NET_DOWN;

    return receive_timeout(~0u);
}

inline bool AJI_CLIENT::connect(void)
{
    if (m_state == CONNECTED)
        return true;

    return internal_connect();
}

inline bool AJI_CLIENT::txmessage_is_continue()
{
    DWORD length = m_tx.get_length();
    const BYTE * data = m_tx.get_data();
    if (length == 4 && data[0] == MESSAGE::CONTINUE_COMMANDS)
        return true;
#if _DEBUG
    if (length == 12 && data[0] == MESSAGE::PING && data[3] == 8 && data[8] == MESSAGE::CONTINUE_COMMANDS)
        return true;
#endif
    return false; 
}

inline void AJI_CLIENT::clear_tx_buffer()
{
    m_tx.set_buffer(m_txbuffer, sizeof(m_txbuffer));

#ifdef _DEBUG
    if (m_server_version >= 12)
    {
        m_tx.add_command(MESSAGE::PING);
        m_tx.add_int(m_sequence++);
    }
#endif

}

#endif
