/****************************************************************************
 *   Copyright (c) 2003 by Intel Corporation                                *
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
//              Copyright (c) Altera Corporation 1997 - 2003
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

#ifndef INC_GEN_STRING_SYS_H
#include "gen_string_sys.h"
#endif

#ifndef INC_CCTYPE
#include <cctype>
#define INC_CCTYPE
#endif

#ifndef INC_AJI_H
#include "aji.h"
#endif

#ifndef INC_JTAG_COMMON_H
#include "jtag_common.h"
#endif

#ifndef INC_JTAG_PROTOCOL_H
#include "jtag_protocol.h"
#endif

#ifndef INC_CLIENT_LINK_H
#include "jtag_client_link.h"
#endif

#ifndef INC_CLIENT_CHAIN_H
#include "jtag_client_chain.h"
#endif

#ifndef INC_CLIENT_OPEN_H
#include "jtag_client_open.h"
#endif

#ifndef INC_JTAG_MESSAGE_H
#include "jtag_message.h"
#endif

#ifndef INC_JTAG_RAW_FIFO_H
#include "jtag_raw_fifo.h"
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_CHAIN_SET AJI_CHAIN_JS::m_valid_set;

char * AJI_CHAIN::s_error_info = NULL;

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
const char * AJI_CHAIN::get_error_info(void)
{
    if (s_error_info != NULL)
        return s_error_info;
    else
        return "";
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::get_hardware(DWORD * hardware_count,
                                  AJI_HARDWARE * hardware_list,
                                  char ** server_version_info_list,
                                  DWORD timeout)
{
    AJI_DEBUG_ASSERT(hardware_count != NULL);
    AJI_DEBUG_ASSERT(*hardware_count == 0 || hardware_list  != NULL);

    DWORD space = * hardware_count;
    DWORD total = 0;
    bool connecting = false;

    if (timeout > 0)
        AJI_CLIENT::update_hardware(timeout);

    for (DWORD i = 0 ; i < AJI_CLIENT::m_link.size() ; i++)
    {
        const AJI_CLIENT::CHAINS * chain = AJI_CLIENT::m_link[i]->get_chains();

        for (AJI_CLIENT::CHAINS::const_iterator j = chain->begin() ; j != chain->end() ; j++)
            if ((*j).m_chain->m_valid)
                if (++total <= space)
                {
                    *hardware_list++ = *(*j).m_chain;
                    if (server_version_info_list != NULL)
                    {
                        const char * server_version_info = AJI_CLIENT::m_link[i]->get_host_server_version_info();
                        if (server_version_info != NULL)
                        {
                        	const size_t buffer_size = strnlen_s(server_version_info, SERVER_VERSION_INFO_BUFFER_SIZE) + 1;
                            *server_version_info_list = new char[buffer_size];
                            strcpy_s(*server_version_info_list++, buffer_size, server_version_info);
                        }
                        else
                        {
                            *server_version_info_list = new char[50];
                            strcpy_s(*server_version_info_list++, 50, "version information not available");
                        }
                    }
                }

        if (AJI_CLIENT::m_link[i]->connecting())
            connecting = true;
    }

    * hardware_count = total;

    if (total > space)
        return AJI_TOO_MANY_DEVICES;

    if (connecting)
        return AJI_TIMEOUT;

    if (total == 0 && !AJI_CLIENT::local_server()->connected())
        return AJI_SERVER_ERROR;

    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::find_hardware(DWORD persistent_id,
                                   AJI_HARDWARE * hardware,
                                   DWORD timeout)
//
// Description: Find a piece of hardware with the persistent ID specified.
//              Don't block for longer than the timeout specified.
//
// Returns:     AJI_NO_ERROR if hardware is found
//              AJI_FAILURE  if hardware not found
//              AJI_TIMEOUT  if timeout before we know an answer
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(hardware != NULL);
    bool connecting;

    if (persistent_id == 0)
        return AJI_FAILURE;

    do
    {
        if (timeout > 0)
        {
            DWORD part_timeout = (timeout < 500) ? timeout : 500;
            AJI_CLIENT::update_hardware(part_timeout);
            timeout -= part_timeout;
        }

        connecting = false;
        for (DWORD i = 0 ; i < AJI_CLIENT::m_link.size() ; i++)
        {
            const AJI_CLIENT * link = AJI_CLIENT::m_link[i];

            // If the persistent ID can't be on this server then don't bother checking
            // it (this saves time if the link to a different server is broken).
            if ((persistent_id >> 24) != link->get_id())
                continue;

            const AJI_CLIENT::CHAINS * chain = link->get_chains();

            for (AJI_CLIENT::CHAINS::const_iterator j = chain->begin() ; j != chain->end() ; j++)
                if ((*j).m_chain->m_valid)
                {
                    AJI_HARDWARE hw = *(*j).m_chain;

                    if (hw.persistent_id == persistent_id)
                    {
                        *hardware = hw;
                        return AJI_NO_ERROR;
                    }
                }

            if (AJI_CLIENT::m_link[i]->connecting())
                connecting = true;
        }

    }
    while (connecting && timeout > 0);

    if (connecting)
        return AJI_TIMEOUT;
    else
        return AJI_FAILURE;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
static char * strip_spaces(char * string, int string_length)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (string == NULL)
        return NULL;

    while (isspace(*string))
        string++;

    char * ptr = string + string_length;

    while (ptr > string && isspace(ptr[-1]))
        ptr--;
    *ptr = 0;

    return string;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline bool AJI_CHAIN_JS::match_hardware(const char * one, const char * two)
//
// Description: Return true if both strings describe the same type of hardware
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (stricmp(one, two) == 0)
        return true;

    if (strnicmp(one, "ByteBlaster", 11) == 0 && strnicmp(two, "ByteBlaster", 11) == 0)
        return true;

    return false;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline bool AJI_CHAIN_JS::strings_match_nocase(const char * one, const char * two)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (one == NULL && two == NULL)
        return true;
    else if (one == NULL || two == NULL)
        return false;
    else
        return stricmp(one, two) == 0;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::find_hardware(const char * hw_name,
                                   AJI_HARDWARE * hardware,
                                   DWORD timeout)
//
// Description: Find a piece of hardware with the name specified.  If a partial
//              name (or a NULL name) is specified then the match will succeed
//              if there is exactly one match.  The formats matched are:
//
//                  <cabletype>               - Any matching cable on any server
//                  <cabletype> on <server>   - Any matching cable on server specified
//                  <cabletype> [<port>]      - The specified cable on the local server
//                  <cabletype> on <server> [<port>] - The specified cable on the server specified
//                  <number>                  - The chain with the index specified
//
//                  <server>::<cabletype> is accepted as equivalent to <cabletype> on <server>
//
//              Don't block for longer than the timeout specified.
//
// Returns:     AJI_NO_ERROR if hardware is found
//              AJI_FAILURE  if hardware not found
//              AJI_TIMEOUT  if timeout before we know an answer
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    const size_t buffer_size = 256;
    char buffer[buffer_size];
    char * server = NULL;
    char * port = NULL;
    int index = -1;

    AJI_DEBUG_ASSERT(hardware != NULL);
    bool connecting;
    int partial_matches;
    int index_togo;

    if (hw_name != NULL && hw_name[0] == 0)
        hw_name = NULL;

    if (hw_name != NULL)
    {
        if (strnlen_s(hw_name, buffer_size) == buffer_size) //TODO Check what happen if hw_name > buffer size?
            return AJI_INVALID_PARAMETER;

        strncpy_s(buffer, buffer_size, hw_name, sizeof(buffer)-1);
        buffer[sizeof(buffer)-1] = 0;
        char * type = buffer;

        char * nptr = buffer;

        char * spaceonspace = strstr(nptr, " on ");
        char * coloncolon = strstr(nptr, "::");
        if (spaceonspace != NULL)
        {
            // The string "<type> on <server>" describes a server
            server = spaceonspace + 4;
            *spaceonspace = 0;
            nptr = server;
        }
        else if (coloncolon != NULL)
        {
            // An old version of quartus_pgm accepted "<server>::<type>" so we cope if a
            // user has embedded that in a script somewhere.
            server = nptr;
            type = coloncolon + 2;
            *coloncolon = 0;
            nptr = type;
        }

        char * openbrace = strchr(nptr, '[');
        if (openbrace != NULL)
        {
            char * closebrace = strchr(openbrace, ']');

            if (closebrace != NULL && closebrace > openbrace + 1)
            {
                port = openbrace + 1;
                *openbrace = 0;
                *closebrace = 0;
            }
        }

        server = strip_spaces(server, (int)strnlen_s(server, buffer_size));
        hw_name = strip_spaces(type, (int)strnlen_s(type, buffer_size));
        port = strip_spaces(port, (int)strnlen_s(port, buffer_size));

        if (server == NULL && port == NULL)
        {
            // Is the string passed in an index?
            bool isindex = true;
            const char * ptr;
            for (ptr = type ; isindex && *ptr != 0 ; ptr++)
                if (!isdigit(*ptr))
                    isindex = false;
            if (isindex)
                index = atoi(type) - 1;
        }
    }

    // If port was specified but server wasn't then this means the requested
    // hardware is on the local server.  If neither was specified then we will
    // accept any server name as part of a partial match.
    bool any_server = (server == NULL && port == NULL);

    if (server != NULL && stricmp(server, "localhost") == 0)
        server = NULL;

    // If port was not specified then we will accept a partial match.
    bool any_port   = (port == NULL);

    do
    {
        if (timeout > 0)
        {
            DWORD part_timeout = (timeout < 500) ? timeout : 500;
            AJI_CLIENT::update_hardware(part_timeout);
            timeout -= part_timeout;
        }

        connecting = false;
        partial_matches = 0;
        index_togo = index;     
        for (DWORD i = 0 ; i < AJI_CLIENT::m_link.size() ; i++)
        {
            const AJI_CLIENT * link = AJI_CLIENT::m_link[i];

            // Can we ignore this server when looking for the chain?  If we need
            // a wildcard match on server then we must check them all.
            bool not_this_server = !any_server && !strings_match_nocase(server, link->server_name());

            // If it's not this server and the user isn't searching by index then
            // don't even try to talk to this server (if they are searching by
            // index then we need to get the number of boards on earlier servers
            // to get the best answer we can).
            if (not_this_server && index < 0)
                continue;

            // Connect to the server if necessary and start to get the chain list
            // from it.
            const AJI_CLIENT::CHAINS * chain = link->get_chains();

            if (link->connecting())
                connecting = true;

            // If the chain is being described by index then look for the 
            if (index >= 0)
            {
                if (!connecting)
                    for (AJI_CLIENT::CHAINS::const_iterator j = chain->begin() ; j != chain->end() ; j++)
                        if ((*j).m_chain->m_valid && index_togo-- == 0)
                        {
                            *hardware = *(*j).m_chain;
                            return AJI_NO_ERROR;
                        }
                continue;
            }

            // We can always ignore incorrect servers from here on.
            if (not_this_server)
                continue;

            for (AJI_CLIENT::CHAINS::const_iterator j = chain->begin() ; j != chain->end() ; j++)
                if ((*j).m_chain->m_valid)
                {
                    AJI_HARDWARE hw = *(*j).m_chain;
                    bool partial = true, full = true;

                    if (hw.features & AJI_FEATURE_DUMMY)
                        continue;

                    if (hw_name != NULL && !match_hardware(hw_name, hw.hw_name))
                        continue;

                    if (any_server)
                        full = false;
                    else if (!strings_match_nocase(server, hw.server))
                        full = partial = false;

                    if (any_port)
                        full = false;
                    else if (!strings_match_nocase(port, hw.port))
                        full = partial = false;

                    if (full)
                    {
                        *hardware = hw;
                        return AJI_NO_ERROR;
                    }
                    else if (partial)
                    {
                        *hardware = hw;
                        partial_matches++;
                    }
                }
        }

        if (!connecting && partial_matches == 1)
            return AJI_NO_ERROR;
    }
    while (connecting && timeout > 0);

    if (connecting)
        return AJI_TIMEOUT;
    else if (partial_matches > 0)
        return AJI_FAILURE;
    else
        return AJI_NO_DEVICES;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::print_hardware_name(char               * hw_name,
                                         DWORD                hw_name_len,
                                         bool                 explicit_localhost ,
                                         DWORD              * need_hw_name_len)
//
// Description: 
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (need_hw_name_len != NULL)
        (*need_hw_name_len) = 0;

    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    DWORD need = static_cast<DWORD> (strnlen_s(m_hw_name, 256)) + 1;
    if (m_server_name != NULL)
        need += 4 + static_cast<DWORD> (strnlen_s(m_server_name, 256));
    else if (explicit_localhost)
        need += 13; // " on localhost"
    if (m_port_name != NULL)
        need += 3 + static_cast<DWORD> (strnlen_s(m_port_name, 256));

    if (need_hw_name_len != NULL)
        (*need_hw_name_len) = need;

    if (hw_name_len < need)
        return AJI_TOO_MANY_DEVICES; // Not enough space

    char * ptr = hw_name;
    char * const end = ptr + hw_name_len;
    ptr += snprintf(ptr, hw_name_len, "%s", m_hw_name);

    if (m_server_name != NULL)
        ptr += snprintf(ptr, (size_t)(end-ptr), " on %s", m_server_name);
    else if (explicit_localhost)
        ptr += snprintf(ptr, (size_t)(end-ptr), "%s", " on localhost");
    if (m_port_name != NULL)
        ptr += snprintf(ptr, (size_t)(end-ptr), " [%s]", m_port_name);

    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_CHAIN_JS::operator AJI_HARDWARE(void)
//
// Description:
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(m_valid);

    AJI_HARDWARE hw;

    hw.chain_id      = this;
    if (m_chain_id != 0)
        hw.persistent_id = ((m_chain_id & 0xFFFF) | (m_client->get_id() << 24)) + 1; // 0 is never a valid ID
    else
        hw.persistent_id = 0; // A dummy chain
    hw.hw_name       = m_hw_name;
    hw.port          = m_port_name;
    hw.device_name   = m_device_name;
    hw.chain_type    = m_chain_type;
    hw.server        = m_server_name;
    hw.features      = m_features;

    return hw;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_CHAIN_JS::AJI_CHAIN_JS(AJI_CLIENT * client, unsigned int chain_id, AJI_CHAIN_TYPE chain_type, DWORD features)
    : m_client(client), m_chain_id(chain_id), m_valid(true), m_hw_strings(NULL),
      m_hw_name(NULL), m_port_name(NULL), m_device_name(NULL), m_server_name(NULL),
      m_chain_type(chain_type), m_features(features),
      m_dummy_bits(0), m_dummy_bits_calculated(false),
      m_chain_tag(0), m_strings(NULL), m_locked(0), m_watch_fn(NULL)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // Store the this pointer so that we can validity check pointers from the
    // user later.
    m_valid_set.insert(this);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_CHAIN_JS::~AJI_CHAIN_JS(void)
{
    // Remove ourselves from the validity checking collection.
    m_valid_set.erase(this);

    delete[] m_hw_strings;
    delete[] m_strings;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CHAIN_JS::invalidate(AJI_CLIENT * client)
//
// Description: The link to the server specified has been disconnected.
//              If this chain uses it then make it invalid.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (m_client == client)
    {
        m_valid = false;

        if (m_locked)
        {
            m_locked = false;
            m_client->release_link();
        }
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline bool AJI_CHAIN_JS::strings_match(const char * one, size_t size, const char * two)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (one == NULL && two == NULL)
        return true;
    else if (one == NULL || two == NULL)
        return false;
    else
        return strcmp_(one, size, two) == 0;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CHAIN_JS::set_strings(const char * hw_name, const char * port, const char * device_name, const char * server)
{
    // This is called becuause we've just got the list of names from the server
    // (or because the server is not connectable somehow).  The connection to the
    // server must be valid again.
    m_valid = true;

    if (strings_match(m_hw_name, 256, hw_name) &&
        strings_match(m_port_name, 256, port) &&
        strings_match(m_device_name, 256, device_name) &&
            strings_match(m_server_name, 256, server))
        return;

    unsigned int hlen = (hw_name != NULL) ? static_cast<unsigned int> (strnlen_s(hw_name, 256)) + 1 : 0;
    unsigned int plen = (port != NULL)    ? static_cast<unsigned int> (strnlen_s(port, 256)) + 1 : 0;
    unsigned int dlen = (device_name != NULL) ? static_cast<unsigned int> (strnlen_s(device_name, 256)) + 1 : 0;
    unsigned int slen = (server != NULL)  ? static_cast<unsigned int> (strnlen_s(server, 256)) + 1 : 0;

    delete[] m_hw_strings;
    m_hw_strings = new char[hlen + plen + dlen + slen];

    m_hw_name     = NULL;
    m_port_name   = NULL;
    m_device_name = NULL;
    m_server_name = NULL;

    if (m_hw_strings != NULL)
    {
        char * ptr = m_hw_strings;
        if (hw_name != NULL)
        {
            m_hw_name = ptr;
            memcpy_s(m_hw_name, hlen, hw_name, hlen);
            ptr += hlen;
        }

        if (port != NULL)
        {
            m_port_name = ptr;
            memcpy_s(m_port_name, plen, port, plen);
            ptr += plen;
        }

        if (device_name != NULL)
        {
            m_device_name = ptr;
            memcpy_s(m_device_name, dlen, device_name, dlen);
            ptr += dlen;
        }

        if (server != NULL)
        {
            m_server_name = ptr;
            memcpy_s(m_server_name, slen, server, slen);
        }
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::add_hardware(const AJI_HARDWARE * hardware)
{
    AJI_DEBUG_ASSERT(hardware != NULL);
    AJI_DEBUG_ASSERT(hardware->hw_name != NULL);

    AJI_CLIENT * client = AJI_CLIENT::local_server();

    if (client == NULL)
        return AJI_SERVER_ACTIVE;

    if (!client->try_claim_link(100))
        return AJI_SERVER_ACTIVE;

    if (!client->connect())
    {
        client->release_link();
        return AJI_NET_DOWN;
    }

    AJI_HW_TYPE hw_type(AJI_HW_OTHER);

    // For the benefit of version 0 servers we fill in the type field.
    if (strnicmp(hardware->hw_name, "ByteBlaster", 11) == 0)
        hw_type = AJI_HW_BYTEBLASTER;

    TXMESSAGE * tx = client->get_txmessage(NULL);

    tx->add_command(MESSAGE::ADD_HARDWARE);
    tx->add_int(hw_type); // For version 0 servers only.  Ignored by v1 servers if hw_name present
    tx->add_string(hardware->port);
    if (client->version_ok(1, 0xFFFF))
    {
        // Version 1 servers use the hw_name field to select the hardware type
        tx->add_string(hardware->hw_name);
        tx->add_string(hardware->device_name);
    }

    AJI_ERROR error = client->send_receive();

    if (error == AJI_NO_ERROR)
    {
        RXMESSAGE * rx = client->get_rxmessage();

        if (!rx->remove_response(&error))
        {
            client->disconnect();
            error = AJI_SERVER_ERROR;
        }
    }

    client->release_link();

    if (error == AJI_NO_ERROR)
        client->refresh_hardware();

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::remove_hardware(void)
{
    if (!valid())
        return AJI_INVALID_CHAIN_ID;

    if (m_client == NULL)
        return AJI_SERVER_ACTIVE;

    // If this is a connection to a remote server then remove the link to the server
    if (m_client->is_remote_server())
        return m_client->remove_remote_server();

    if (!m_client->try_claim_link(100))
        return AJI_SERVER_ACTIVE;

    // NOTE: We could, in some circumstances, trap the 'chain in use' error here
    // without sending the command to the server.  Since the server has to handle
    // the case anyway we don't bother for this version.

    TXMESSAGE * tx = m_client->get_txmessage(NULL);

    tx->add_command(MESSAGE::REMOVE_HARDWARE);
    tx->add_int(m_chain_id);

    RXMESSAGE * rx;
    AJI_ERROR error = send_receive(&rx);

    AJI_CLIENT * client = m_client;

    if (client != NULL)
        client->release_link();

    if (error == AJI_NO_ERROR)
        m_client->refresh_hardware(); // Beware: Deletes 'this'

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::cancel_operation(void)
//
// Description: Cancel the operation which is in progress on this chain (make
//              it return with an error).
//
//              At present this only cancels a call to watch_data, but it might
//              in future cancel some other operations.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    m_watch_fn = NULL;

    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::lock(DWORD timeout)
{
    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    if (m_locked)
        return AJI_LOCKED;

    if (m_client == NULL)
        return AJI_SERVER_ACTIVE;

    if (!m_client->try_claim_link(timeout))
        return AJI_SERVER_ACTIVE;

    // Work around a bug in older JTAG servers (which sometimes start to ignore
    // this client if timeout == 0)
    if (timeout == 0 && !m_client->version_ok(4, 0xFFFF))
        timeout = 1;

    TXMESSAGE * tx = m_client->get_txmessage(NULL);

    tx->add_command(MESSAGE::LOCK_CHAIN);
    tx->add_int(m_chain_id);
    tx->add_int(timeout);

    RXMESSAGE * rx;
    AJI_ERROR error = send_receive(&rx);

    if (error == AJI_NO_ERROR)
        m_locked = true;
    else if (m_client != NULL)
        m_client->release_link();

    if (error == AJI_CHAIN_IN_USE)
        read_blocker_info(rx);

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::unlock(void)
{
    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    if (!m_locked)
        return AJI_NOT_LOCKED;

    TXMESSAGE * tx = m_client->get_txmessage(NULL);

    tx->add_command(MESSAGE::UNLOCK_CHAIN);
    tx->add_int(m_chain_id);

    RXMESSAGE * rx;
    AJI_ERROR error = send_receive(&rx);

    // Assume we've unlocked regardless of errors.
    m_locked = false;
    if (m_client != NULL)
        m_client->release_link();

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::define_device(DWORD tap_position, const AJI_DEVICE * device)
{
    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    if (device == NULL ||
        device->mask & 1 ||
        device->instruction_length < 2 || // device->instruction_length >= 256 ||    // instruction_length is a BYTE
        device->device_name == NULL || device->device_name[0] == 0)
        return AJI_INVALID_PARAMETER;

    if (!m_locked)
        return AJI_NOT_LOCKED;

    if (m_chain_tag == 0)
        return AJI_CHAIN_NOT_CONFIGURED;

    // We have locked the link so noone can change the m_taps array under us
    if (tap_position >= m_taps.size())
        return AJI_BAD_TAP_POSITION;

    AJI_ERROR error = m_client->define_device(m_chain_id, tap_position, device);

    if (error == AJI_NO_ERROR)
    {
        m_taps[tap_position].m_device = *device;
        m_taps[tap_position].m_device.device_name = ""; // TODO: allocate memory (perhaps)
    }

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::scan_device_chain(void)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    if (!m_locked)
        return AJI_NOT_LOCKED;

    // We could fail if any devices are open.  This catches one of the possible
    // failure codes, but since the server checks anyway we don't bother.

    AJI_ERROR error = refresh_chain(true, false);

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::read_device_chain       (DWORD              * device_count,
                                              AJI_DEVICE         * device_list,
                                              bool                 auto_scan)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(device_count != NULL);
    AJI_DEBUG_ASSERT(*device_count == 0 || device_list  != NULL);

    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    if (m_locked)
    {
        // If we have locked the chain then we can check it is up to date
        AJI_ERROR error = refresh_chain(false, auto_scan);

        // Old servers don't auto scan so we must ask them to scan again.
        if (error == AJI_CHAIN_NOT_CONFIGURED && auto_scan)
            error = refresh_chain(true, false);

        if (error != AJI_NO_ERROR)
            return error;
    }

    if (m_chain_tag == 0)
        return AJI_CHAIN_NOT_CONFIGURED;

    // Claim the taps mutex to prevent another thread from changing the m_taps
    // array while we read it.
    if (!m_taps_mutex.try_claim(1000))
        return AJI_SERVER_ACTIVE;

    DWORD space = * device_count;

    * device_count = static_cast<DWORD> (m_taps.size());

    bool toomany = (m_taps.size() > space);

    for (TAPS::iterator j = m_taps.begin() ; j != m_taps.end() && space-- > 0 ; j++)
        *device_list++ = (*j).m_device;

    m_taps_mutex.release();

    if (toomany)
        return AJI_TOO_MANY_DEVICES;
    else
        return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
DWORD AJI_CHAIN_JS::get_device_features(DWORD tap_position)
{
    if (!valid() || m_chain_id == 0 || m_chain_tag == 0)
        return 0;

    if (!m_taps_mutex.try_claim(1000))
        return 0;

    DWORD features;
    if (tap_position < m_taps.size())
        features = m_taps[tap_position].m_device.features;
    else
        features = 0;

    m_taps_mutex.release();

    return features;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::refresh_chain(bool forcescan, bool autoscan)
{
    TXMESSAGE * tx = m_client->get_txmessage(NULL);

    tx->add_command(forcescan ? MESSAGE::SCAN_CHAIN : MESSAGE::READ_CHAIN);
    tx->add_int(m_chain_id);
    tx->add_int(m_chain_tag);

    if (m_client->version_ok(2, 0xFFFF))
        tx->add_int(autoscan ? 1 : 0);

    RXRAWFIFO devicelist(m_client);
    devicelist.activate(0);

    RXMESSAGE * rx;
    AJI_ERROR error = send_receive(&rx);

    if (error != AJI_NO_ERROR)
    {
        m_chain_tag = 0;
        return error;
    }

    DWORD chain_tag(0);
    DWORD device_count(0);
    DWORD fifo_len(0);

    if (!rx->remove_int(&chain_tag) ||
            !rx->remove_int(&device_count) ||
            !rx->remove_int(&fifo_len))
    {
        m_client->disconnect();
        return AJI_SERVER_ERROR;
    }

    if (device_count > 0 && fifo_len > 0)
    {
        if (!devicelist.wait_for_data(fifo_len))
        {
            m_client->disconnect();
            return AJI_SERVER_ERROR;
        }
    }

    // Claim the taps mutex to lock out any clients who want to read the array.
    if (!m_taps_mutex.try_claim(1000))
        return AJI_SERVER_ACTIVE;

    if (device_count == 0)
    {
        m_taps.erase(m_taps.begin(), m_taps.end());
    }
    else if (fifo_len > 0)
    {
        bool version5 = m_client->version_ok(5, 0xFFFF);

        // Download the new TAP list via the first FIFO.  If no data was sent by
        // the server then this means the TAP list hasn't changed.
        m_taps.erase(m_taps.begin(), m_taps.end());

        m_taps.reserve(device_count);

        int stringlen = 0;

        for (int i = 0 ; i < 2 ; i++)
        {
            RXMESSAGE message(devicelist.get_data(), fifo_len);
            message.set_string(m_strings, stringlen);               
            message.single_block_buffer();

            for (DWORD j = 0 ; j < device_count ; j++)
            {
                DEVICE_INFO dev;
                DWORD instruction_length(0);
                DWORD dummy(0);

                if (!message.remove_int(&dev.m_device.device_id) ||
                      !message.remove_int(&instruction_length) ||
                      !message.remove_int(&dev.m_device.features) ||
                      !message.remove_int(&dummy) ||
                      !message.remove_int(&dummy) ||
                      !message.remove_string(&dev.m_device.device_name))
                {
                    i = 2;
                    break;
                }
    
                dev.m_device.instruction_length = static_cast<BYTE>(instruction_length);

                // Old servers never set POSSIBLE_HUB, so we must pretend they set
                // it on all devices so the hub clients work with them.
                if (!version5)
                    dev.m_device.features |= AJI_DEVFEAT_POSSIBLE_HUB;

                if (i == 1)
                    m_taps.push_back(dev);
            }

            if (i == 0)
            {
                stringlen = message.get_string_overflow();
                delete[] m_strings;
                m_strings = new char[stringlen];
            }
        }
    }

    m_chain_tag = chain_tag;

    m_taps_mutex.release();

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//

AJI_ERROR AJI_CHAIN_JS::watch_data(WATCH_FN * watch_fn, void * handle, bool intrusive)
{
    // Check that we're being called from a new version of jtagwatcher or
    // jtagcapture
    if (watch_fn(handle, (BYTE *)1, 0) != 0)
        return AJI_FAILURE;

    // Get a key from jtagwatcher/jtagcapture to validate its use of the interface
    DWORD key = watch_fn(handle, (BYTE *)1, INT_MAX);

    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    if (m_client == NULL)
        return AJI_SERVER_ACTIVE;

    if (!m_client->try_claim_link(1000))
        return AJI_SERVER_ACTIVE;

    TXMESSAGE * tx = m_client->get_txmessage(NULL);

    tx->add_command(MESSAGE::WATCH_DATA);
    tx->add_int(m_chain_id);
    tx->add_int(1);
    tx->add_int(intrusive);
    tx->add_int(key);

    RXMESSAGE * rx;
    AJI_ERROR error = send_receive(&rx);

    if (error == AJI_NO_ERROR)
    {
        m_watch_fn     = watch_fn;
        m_watch_handle = handle;

        (*watch_fn)(handle, NULL, 0);

        // This receive function will not return until the operation is cancelled or
        // the connection is disconnected by the JTAG server since there was no
        // corresponding send to acknoweledge.  We call it to get the TCP socket to
        // start polling for data coming down the notification channel
        do
        {
            error = m_client->receive_timeout(1000);

            if (error != AJI_NO_ERROR && error != AJI_TIMEOUT)
                break;
        }
        while (m_watch_fn != NULL);

        // Tell server to stop sending us stuff
        if (error == AJI_TIMEOUT)
        {
            tx = m_client->get_txmessage(NULL);
            tx->add_command(MESSAGE::WATCH_DATA);
            tx->add_int(m_chain_id);
            tx->add_int(0);
            error = send_receive(&rx);
        }
    }

    if (m_client != NULL)
        m_client->release_link();

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::set_parameter(const char * name, DWORD value)
{
    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    if (name == NULL || name[0] == 0 || strnlen_s(name, PARAM_NAME_BUFFER_SIZE) == PARAM_NAME_BUFFER_SIZE)
        return AJI_INVALID_PARAMETER;

    if (!m_locked)
        return AJI_NOT_LOCKED;

    TXMESSAGE * tx = m_client->get_txmessage(NULL);

    tx->add_command(MESSAGE::SET_PARAMETER);
    tx->add_int(m_chain_id);
    tx->add_string(name);
    tx->add_int(value);

    RXMESSAGE * rx;
    AJI_ERROR error = send_receive(&rx);

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::get_parameter(const char * name, DWORD * value)
{
    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    if (name == NULL || name[0] == 0 || value == NULL)
        return AJI_INVALID_PARAMETER;

    if (strnlen_s(name, PARAM_NAME_BUFFER_SIZE) == PARAM_NAME_BUFFER_SIZE)
        return AJI_INVALID_PARAMETER;

    if (!m_locked)
        return AJI_NOT_LOCKED;

    TXMESSAGE * tx = m_client->get_txmessage(NULL);

    tx->add_command(MESSAGE::GET_PARAMETER);
    tx->add_int(m_chain_id);
    tx->add_string(name);

    RXMESSAGE * rx;
    AJI_ERROR error = send_receive(&rx);

    DWORD val(0);
    if (error == AJI_NO_ERROR && !rx->remove_int(&val))
    {
        m_client->disconnect();
        error = AJI_SERVER_ERROR;
    }

    if (error == AJI_NO_ERROR)
        *value = val;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::get_parameter(const char * name, BYTE * value, DWORD * valuemax, DWORD valuetx)
{
    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    if (name == NULL || name[0] == 0 || value == NULL || valuemax == NULL || *valuemax == 0)
        return AJI_INVALID_PARAMETER;

    if (strnlen_s(name, PARAM_NAME_BUFFER_SIZE) == PARAM_NAME_BUFFER_SIZE || *valuemax > AJI_PARAMETER_MAX || valuetx > AJI_PARAMETER_MAX)
        return AJI_INVALID_PARAMETER;

    if (!m_locked)
        return AJI_NOT_LOCKED;

    TXMESSAGE * tx = m_client->get_txmessage(NULL);

    tx->add_command(MESSAGE::GET_PARAMETER_BLOCK);
    tx->add_int(m_chain_id);
    tx->add_string(name);
    tx->add_int(*valuemax);
    tx->add_int(valuetx);
    if (valuetx > 0)
        tx->add_raw(value, valuetx);

    RXMESSAGE * rx;
    AJI_ERROR error = send_receive(&rx);

    DWORD len(0);
    if (error == AJI_NO_ERROR && !(rx->remove_int(&len) && rx->remove_raw(value, len)))
    {
        m_client->disconnect();
        error = AJI_SERVER_ERROR;
    }

    if (error == AJI_NO_ERROR)
        *valuemax = len;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::open_device (DWORD                tap_position,
                                     AJI_OPEN_ID        * open_id,
                                     const AJI_CLAIM2   * claims,
                                     DWORD                claim_n,
                                     const char         * application_name)
//
// Description: Create a connection to the device, claiming exclusive or
//              shared access to device resources as shown in the claims array.
//              there are three needed claims so check that they are there and
//              if not add them
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    DWORD i;

    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    if (open_id == NULL ||
        (claim_n > 0 && claims == NULL))
        return AJI_INVALID_PARAMETER;

    // Are all the claim length valid?
    for (i = 0 ; i < claim_n ; i++)
    {
        if (claims[i].length > 64)
            return AJI_INVALID_PARAMETER;

        if (claims[i].length > 0 &&
            claims[i].type != AJI_CLAIM_OVERLAY &&
            claims[i].type != AJI_CLAIM_OVERLAY_SHARED &&
            claims[i].type != AJI_CLAIM_OVERLAY_WEAK) {
            return AJI_INVALID_PARAMETER;
        }
    }

    bool version10 = m_client->version_ok(10, 0xFFFF);

    // Are all the claim types valid?  Is the user using overlay chains?
    bool need_version2 = false;
    DWORD claim_count = 0;
    for (i = 0 ; i < claim_n ; i++)
        switch (claims[i].type)
        {
        case AJI_CLAIM_IR:
        case AJI_CLAIM_IR_SHARED:
            claim_count++;
            break;

        case AJI_CLAIM_IR_SHARED_OVERLAY:
        case AJI_CLAIM_IR_OVERLAID:
        case AJI_CLAIM_IR_SHARED_OVERLAID:
        case AJI_CLAIM_OVERLAY:
        case AJI_CLAIM_OVERLAY_SHARED:
            claim_count++;
            need_version2 = true;
            break;

        case AJI_CLAIM_IR_WEAK:
        case AJI_CLAIM_OVERLAY_WEAK:
            if (version10)
                claim_count++;
            need_version2 = true;
            break;

        default:
            return AJI_INVALID_PARAMETER;
        }

    if (!m_locked)
        return AJI_NOT_LOCKED;

    if (m_chain_tag == 0)
        return AJI_CHAIN_NOT_CONFIGURED;

    if (tap_position >= m_taps.size())
        return AJI_BAD_TAP_POSITION;

    AJI_DEVICE & device = m_taps[tap_position].m_device;

    if (device.instruction_length < 2)
        return AJI_DEVICE_NOT_CONFIGURED;

    // If the instruction length is more than 31 then we won't be able to store
    // information about claims correctly, so don't let the user open the device
    if (device.instruction_length > 31) {
        return AJI_INVALID_PARAMETER;
    }
    // Overlays work differently with version 0 and 1 servers.  Don't try and
    // be compatible with older servers.
    bool version2 = m_client->version_ok(2, 0xFFFF);
    if (need_version2 && !version2)
        return AJI_UNIMPLEMENTED;

    AJI_CLAIM2 *new_claims = NULL;
    bool needs_dummy_bit_calc = false;

    // Need certain claims for dummy bits to work.  Add them if needed.
    // dummy_bit_calc needs HUB_INFO but it is not always included so add it here.
    // Also raised a flag to calculate dummy bits.
    if (version2)
    {
        DWORD new_claim_n = 0;
        if (   ((SAFECLAIM == claimed_class(claims, claim_n, AJI_CLAIM_IR, JTAG_USR0)) ||
                (SAFECLAIM == claimed_class(claims, claim_n, AJI_CLAIM_IR_SHARED_OVERLAID, JTAG_USR0)) ||
                (SAFECLAIM == claimed_class(claims, claim_n, AJI_CLAIM_IR_WEAK, JTAG_USR0))) &&
               ((SAFECLAIM == claimed_class(claims, claim_n, AJI_CLAIM_IR, JTAG_USR1)) ||
                (SAFECLAIM == claimed_class(claims, claim_n, AJI_CLAIM_IR_SHARED_OVERLAY, JTAG_USR1)) ||
                (SAFECLAIM == claimed_class(claims, claim_n, AJI_CLAIM_IR_WEAK, JTAG_USR1))))
        {
            needs_dummy_bit_calc = true;

            if (! ((SAFECLAIM == claimed_class(claims, claim_n, AJI_CLAIM_OVERLAY, HUB_INFO)) ||
                   (SAFECLAIM == claimed_class(claims, claim_n, AJI_CLAIM_OVERLAY_SHARED, HUB_INFO)) ||
                   (SAFECLAIM == claimed_class(claims, claim_n, AJI_CLAIM_OVERLAY_WEAK, HUB_INFO))))
            {
                new_claims = new AJI_CLAIM2[claim_n + 1];
                new_claims[new_claim_n].type = AJI_CLAIM_OVERLAY_SHARED;
                new_claims[new_claim_n].length = 0;
                new_claims[new_claim_n++].value = HUB_INFO;
            }
        }
        if( new_claim_n != 0)
        {
            // copy the original claims
            memcpy_s(&new_claims[new_claim_n], sizeof(AJI_CLAIM2) * claim_n, claims, sizeof(AJI_CLAIM2) * claim_n);
            claims = new_claims;
            claim_n += new_claim_n;
            claim_count += new_claim_n;
        }
    }

    AJI_OPEN_JS * open = new AJI_OPEN_JS(this, m_client, AJI_OPEN_JS::INDIVIDUAL, tap_position, device, claims, claim_n);
    if (open == NULL)
    {
        delete[] new_claims;
        return AJI_NO_MEMORY;
    }

    TXMESSAGE * tx = m_client->get_txmessage(NULL);

    tx->add_command(MESSAGE::OPEN_DEVICE);
    tx->add_int(m_chain_id);
    tx->add_int(tap_position);

    bool version13 = m_client->version_ok(13, 0xFFFF);
    if (version13)
    {
        tx->add_int(claim_count);

        for (i = 0 ; i < claim_n ; i++)
        {
            tx->add_int(claims[i].type);
            tx->add_int(claims[i].length);
            tx->add_long(claims[i].value);
        }
    }
    else if (version2)
    {
        // Tell the server about all exclusive and shared claims.  Don't bother
        // telling it about weak claims as there is nothing useful it can do
        // with them.

        tx->add_int(claim_count);

        for (i = 0 ; i < claim_n ; i++)
        {
            // For ACDS 12.1+ servers we pass details of weak claims to the server.
            // The standard server ignores them as it can't do anything useful with
            // them but alternative servers take account of them.
            if ((claims[i].type == AJI_CLAIM_IR_WEAK || claims[i].type == AJI_CLAIM_OVERLAY_WEAK) &&
                !version10)
                continue;

            tx->add_int(claims[i].type);
            tx->add_int(static_cast<DWORD>(claims[i].value));
        }
    }
    else
    {
        tx->add_int(claim_n); // == instruction_n
        tx->add_int(0);

        for (i = 0 ; i < claim_n ; i++)
            tx->add_int(static_cast<DWORD>(claims[i].value));
    }

    tx->add_string(application_name);

    RXMESSAGE * rx;
    AJI_ERROR error = send_receive(&rx);

    bool linkok = true;
    DWORD id(0);
    if (error == AJI_NO_ERROR)
        linkok = rx->remove_int(&id);
    else if (error == AJI_CHAINS_CLAIMED || error == AJI_CHAIN_IN_USE)
        linkok = read_blocker_info(rx);

    if (!linkok)
    {
        m_client->disconnect();
        error = AJI_SERVER_ERROR;
    }

    if (error == AJI_NO_ERROR)
    {
        open->set_id(id);
        *open_id = open;

        // no need to recalculate the dummy bits if they have already been calculated.
        // this also fixes. fogbugz 418241)
        if (needs_dummy_bit_calc && !m_dummy_bits_calculated)
        {
            /* Disable the dummy bit calculations for 17.0 there were too many side effects.  FB 430898
             * This will be re-enabled when jtag frequency scan is enabled. in 17.1
             *

            error = calculate_user_mode_dummy_bits(open);
            */
            if (error == AJI_NO_ERROR)
            {
                m_dummy_bits_calculated = true;
            }
        }
        if (error == AJI_NO_ERROR)
        {
                   // set dummy bits in the aji_open object
                   error = open->set_dummy_bits(m_dummy_bits);
        }
    }
    else
        delete open;

    delete[] new_claims;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::open_entire_chain       (AJI_OPEN_ID        * open_id,
                                              AJI_CHAIN_TYPE       style,
                                              const char         * application_name)
{
    if (!valid() || m_chain_id == 0)
        return AJI_INVALID_CHAIN_ID;

    switch (style)
    {
    case AJI_CHAIN_JTAG:
        if (!m_client->version_ok(2, 0xFFFF))
            return AJI_UNIMPLEMENTED;
        break;

    default:
        return AJI_UNIMPLEMENTED;
    }

    if (open_id == NULL)
        return AJI_INVALID_PARAMETER;

    if (!m_locked)
        return AJI_NOT_LOCKED;

    AJI_DEVICE device;
    memset_(&device, 0, sizeof(device));

    AJI_OPEN_JS::DEVICE_STYLE device_style;
    switch(style)
    {
    default: // Won't happen (see above)
    case AJI_CHAIN_JTAG:
        device_style = AJI_OPEN_JS::WHOLE_CHAIN;
        break;
    }

    // TODO: ensure that the following line means "claim everything"

    AJI_OPEN_JS * open = new AJI_OPEN_JS(this, m_client, device_style, 0, device, NULL, 0);
    if (open == NULL)
        return AJI_NO_MEMORY;

    TXMESSAGE * tx = m_client->get_txmessage(NULL);

    tx->add_command(MESSAGE::OPEN_ENTIRE_CHAIN);
    tx->add_int(m_chain_id);
    tx->add_int(style - 1);
    tx->add_string(application_name);

    RXMESSAGE * rx;
    AJI_ERROR error = send_receive(&rx);

    bool linkok = true;
    DWORD id(0);
    if (error == AJI_NO_ERROR)
        linkok = rx->remove_int(&id);
    else if (error == AJI_CHAINS_CLAIMED || error == AJI_CHAIN_IN_USE)
        linkok = read_blocker_info(rx);

    if (!linkok)
    {
        m_client->disconnect();
        error = AJI_SERVER_ERROR;
    }

    if (error == AJI_NO_ERROR)
    {
        open->set_id(id);
        *open_id = open;
    }
    else
        delete open;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CHAIN_JS::read_blocker_info(RXMESSAGE * rx)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // Older servers don't return information about the application which is
    // preventing us from using the resources we want.
    DWORD other_n(0);
    if (!rx->remove_int(&other_n))
        other_n = 0;

    DWORD i;
    for (i = 0 ; i < other_n ; i++)
    {
        char buffer[256], sbuffer[256];
        BYTE address[16];

        rx->set_string(buffer, sizeof(buffer));

        const char * app_name = NULL;
        if (!rx->remove_string(&app_name))
            return false;

        DWORD ver_size(0);
        if (!rx->remove_int(&ver_size))
            return false;

        DWORD addrsize = (ver_size & 0xFFFF);
        DWORD ip_ver   = ver_size >> 16;

        if (addrsize <= sizeof(address))
        {
            rx->remove_raw(address, addrsize);
        }
        else
        {
            // We don't understand this type of address
            rx->remove_raw(NULL, addrsize);
            ip_ver = 0;
        }

        // At present we only store information about the first conflicting
        // application.
        if (i != 0)
            continue;

        const char * server;
        if (ip_ver == 4 || ip_ver == 6)
        {
            // Other application is remote from the server
            char * ptr = sbuffer;
            char * const end = ptr + sizeof(sbuffer);
            if (ip_ver == 4)
            {
                for (int j = 0 ; j < 4 ; j++)
                    ptr += snprintf(ptr, (size_t)(end-ptr), "%d.", address[j]);
            }
            else
            {
                for (int j = 0 ; j < 16 ; j += 2)
                    ptr += snprintf(ptr, (size_t)(end-ptr), "%04X:", (address[j] << 8) | address[j+1]);
            }

            ptr[-1] = 0;
            server = sbuffer;

            // TODO: convert the IP address to a hostname if possible
        }
        else if (m_client->is_remote_server())
        {
            // Other application is local to a remote server - use the server name
            server = m_client->server_name();
        }
        else
        {
            // Other application is local to the local server
            server = NULL;
        }

        // Buffer size 32 is derived from server side JTAG_SERVER_LINK::open_device()
        int len = static_cast<int> (strnlen_s(app_name, 32));
        if (server != NULL)
            len += 4 + static_cast<int> (strnlen_s(server, 256));

        char * error_info = new char[len + 1];

        if (error_info != NULL)
        {
            strcpy_s(error_info, len + 1, app_name);

            if (server != NULL)
            {
                strcat_s(error_info, len + 1, " on ");
                strcat_s(error_info, len + 1, server);
            }
        }

        delete s_error_info;
        s_error_info = error_info;
    }

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::send_receive(RXMESSAGE * * rx_ptr)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(rx_ptr != NULL);

    AJI_ERROR error = AJI_NO_ERROR;

    if (m_client == NULL)
        error = AJI_NET_DOWN;
    else
        error = m_client->send_receive();

    if (error == AJI_NO_ERROR)
    {
        RXMESSAGE * rx = m_client->get_rxmessage();

        if (!rx->remove_response(&error))
            error = AJI_SERVER_ERROR;

        *rx_ptr = rx;
    }

    if (error == AJI_SERVER_ERROR && m_client != NULL)
    {
        m_client->disconnect();
        AJI_DEBUG_ASSERT(m_client == NULL);
    }

    clear_blocker_info();

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CHAIN_JS::clear_blocker_info()
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    delete s_error_info;
    s_error_info = NULL;
}


//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::calculate_user_mode_dummy_bits(AJI_OPEN_JS * open_id )
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    static const DWORD JTAG_USR1 = 0x000E;
    static const DWORD JTAG_USR0 = 0x000C;
    static const QWORD HUB_INFO  = 0x0ll;
    DWORD second_nibble = 0;
    DWORD third_nibble = 0;
    static const DWORD COUNT =   4  ;// 4 (16 bits)
    static const DWORD BYTES_PER_SCAN = (MAX_DUMMY_BITS + 4 + 7) / 8 ;  //it is 11 (7+4)because we are actually reading a nibble.
    DWORD user_dummy_bits = 0;

    // Pass lock from chain to device atomically
    AJI_ERROR error = open_id->unlock_chain_lock(this, AJI_PACK_MANUAL);

    if (error == AJI_NO_ERROR)
    {
        error = aji_access_ir(open_id, JTAG_USR1, NULL);

        // Maximum number of IR bits supported is 64
        BYTE hubident[8], capture[8];

        QWORD select = HUB_INFO;
        memset64(hubident, select);

        if (error == AJI_NO_ERROR)
            error = aji_access_dr(open_id, 64, 0, 0, 64, hubident, 0, 64, capture);

        if (error == AJI_NO_ERROR)
            error = aji_access_ir(open_id, JTAG_USR0, NULL);

        // dummmy bits needs to be zero before preforming the test
        m_dummy_bits = 0;

        if (error == AJI_NO_ERROR)
           error = open_id->set_dummy_bits(0);

        if (error == AJI_NO_ERROR){
            BYTE * bytes = new BYTE [COUNT*BYTES_PER_SCAN];
            if (bytes == NULL)
                return AJI_NO_MEMORY;
            //add max potential dummy bits to the 4 bits for the nibble we are looking for.
            error = aji_access_dr(open_id, MAX_DUMMY_BITS + 4, AJI_DR_UNUSED_X, 0, 0, NULL, 0, MAX_DUMMY_BITS + 4, bytes, COUNT );

            if (error == AJI_NO_ERROR)
                error = aji_flush(open_id);

            if (error == AJI_NO_ERROR)
            {
                BYTE * nptr2 = &bytes[2*BYTES_PER_SCAN]; // start of second nibble
                BYTE * nptr3 = &bytes[3*BYTES_PER_SCAN]; // start of third nibble
                for (DWORD i = 0 ; i < (BYTES_PER_SCAN) ; i++){
                    second_nibble |= (*nptr2++ & 0xff) << (i * 8);
                    third_nibble |= (*nptr3++ & 0xff) << (i * 8);
                }
             }
             delete[] bytes;

             // now to find the number of dummy bits we look for the mfg code of 0x6e in the HUB_INFO
             // the received data is shifted down one bit at a time until there is a match.
             // then number of shifts is the number of dummy bits.
             if (error == AJI_NO_ERROR)
             {
                 for (int i = 0; i <= MAX_DUMMY_BITS; i++)
                 {
                     if ((((second_nibble >> i) & 0xf) == 0xe) && (((third_nibble >> i) & 0xf) == 0x6))
                     {
                         user_dummy_bits = i;
                         break;
                     }
                 }
              }
              m_dummy_bits = user_dummy_bits;
        }

        if (error == AJI_NO_ERROR)
        {
            // set dummy bits in the aji_open area
            error = open_id->set_dummy_bits(m_dummy_bits);
        }

        // Pass lock from device to chain atomically
        open_id->unlock_lock_chain(this);
    }
    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CHAIN_JS::memset64(BYTE * dest, QWORD value)
//
// Description: Assign a 64-bit value to an array of 8 bytes
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    for (QWORD i = 0; i < sizeof(QWORD); i++, value >>= 8)
        dest[i] = static_cast<BYTE>(value);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CHAIN_JS::byte_array_shift_left(BYTE *dest_arr, const BYTE *src_arr, int arr_size, int shift)
 // dest_arr - pointer to bit array to get a shifted result
 // src_arr - pointer to source bit array
 // arr_size - size of the source array ( ie number of bytes)
 // shift - left bit shift value  ( zero fill value)
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    BYTE mask[8] = {0,1,3,7,0xf,0x1f,0x3f, 0x7f };

    if(dest_arr ==  NULL || src_arr == NULL)
        return AJI_INVALID_PARAMETER; // do nothing for NULL pointers

    BYTE * dest = dest_arr;
    const BYTE * src = src_arr;

    // shift is more than 8 its.  * bits is easy it is just a BYTE
    for(; shift >= 8; shift-=8){
        * dest = 0;
        dest++;
    }
    // now the for real work
    for( int i = 0; i < arr_size; i++)
    {
        dest[i] = (src[i]<<shift) + ((i > 0) ? ((src[i-1] >> (8-shift)) & mask[shift]) : 0);
    }
    return AJI_NO_ERROR;
}


//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_CHAIN_JS::CLAIM_STYLE AJI_CHAIN_JS::claimed_class
(
    const AJI_CLAIM2 * claims,
    DWORD claim_n,
    AJI_CLAIM_TYPE type,
    QWORD value,
    DWORD length /* =0 */
)
//
// Description: Check whether the value has been claimed for this device in the
//              client side. Length is optional (or just 0) for non-overlay claims.
//              Note: The checking for in server side is in
//              JTAG_SERVER_TAP::resource_claimed().
//
// Returns:     NOCLAIM if use not allowed, SAFECLAIM if use allowed, IFAVAIL
//              if we must check with the server.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    const AJI_CLAIM2 * claim;
    const AJI_CLAIM2 * end = claims + claim_n;

    // 0x1 is the mask for overlay claim type
    const int overlay_mask = 0x1;
    // 0x800 is the mask for weak claim type
    const int weak_mask = 0x800;

    // Check for exact match
    for (claim = claims ; claim < end ; claim++)
    {
        if (claim->value == value)
        {
            // For overlay claim, compare length
            if (type & overlay_mask)
            {
                // Claim length is 0 if client still uses old claim or accesses
                // node manually instead of using aji_access_overlay()
                if (claim->length == length || claim->length == 0)
                    // Only server knows if a weak claim is available
                    return (type & weak_mask) ? IFAVAIL : SAFECLAIM;
            }
            // For non-overlay claim, ignore length because all non-overlay claim
            // is assumed to have the same length
            else
                // Only server knows if a weak claim is available
                return (type & weak_mask) ? IFAVAIL : SAFECLAIM;
        }
    }

    // Check for matches with claim-all value (~0ull)
    for (claim = claims ; claim < end ; claim++)
    {
        // A weak claim-all value allows access to all values in that resource
        if (claim->value == ~0ull && claim->type == (type | weak_mask))
        {
            // For overlay type, compare length
            if (type & overlay_mask)
            {
                // Claim length is 0 if client still uses old claim or accesses
                // node manually instead of using aji_access_overlay()
                if(claim->length == length || claim->length == 0)
                    return IFAVAIL;
            }
            // For non-overlay type, ignore length
            else
                return IFAVAIL;
        }
    }

    return NOCLAIM;
}
