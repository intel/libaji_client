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
#define INC_STDIO
#endif

#ifndef INC_CCTYPE
#include <cctype>
#define INC_CCTYPE
#endif

#ifndef INC_GEN_STRING_SYS_H
#include "gen_string_sys.h"
#endif

#ifndef INC_JTAG_MD5_PD_H
#include "jtag_md5_pd.h"
#endif

#ifndef INC_AJI_H
#include "aji.h"
#endif

#ifndef INC_JTAG_COMMON_H
#include "jtag_common.h"
#endif

#ifndef INC_JTAG_PLATFORM_H
#include "jtag_platform.h"
#endif

#ifndef INC_JTAG_PROTOCOL_H
#include "jtag_protocol.h"
#endif

#ifndef INC_JTAG_UTILS_H
#include "jtag_utils.h"
#endif

#ifndef INC_JTAG_CLIENT_LINK_H
#include "jtag_client_link.h"
#endif

#ifndef INC_JTAG_CLIENT_CHAIN_H
#include "jtag_client_chain.h"
#endif

#ifndef INC_JTAG_CLIENT_OPEN_H
#include "jtag_client_open.h"
#endif

#ifndef INC_JTAG_MESSAGE_H
#include "jtag_message.h"
#endif

#ifndef INC_JTAG_CONFIGIRUATION_H
#include "jtag_configuration.h"
#endif

#ifndef INC_JTAG_RAW_FIFO_H
#include "jtag_raw_fifo.h"
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

static AJI_CLIENT *local_link = 0;

static bool replace_local_jtag_server = false;
//static bool configuration_is_in_memory = false;
static bool enable_local_jtag_server = true;

STRING_SERVER_MAP AJI_CLIENT::s_temporary_servers;

AJI_CLIENT_VEC AJI_CLIENT::m_link;

void ( * AJI_CLIENT::m_output_fn)(void * handle, DWORD level, const char * line) = NULL;
void * AJI_CLIENT::m_output_handle = NULL;

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_CLIENT * AJI_CLIENT::local_server(void)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    update_servers();
    
    return local_link;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CLIENT::update_hardware(DWORD timeout)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    update_servers();

    connect_all(timeout);

    bool changed = false;
    for (DWORD i = 0 ; i < m_link.size() ; i++)
        changed |= m_link[i]->refresh_hardware(true);

    if (changed)
    {
        update_servers();
        connect_all(500);

        for (DWORD i = 0 ; i < m_link.size() ; i++)
            if (m_link[i]->m_primary != NULL)
                m_link[i]->refresh_hardware(false);
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
DWORD AJI_CLIENT::update_servers(void)
//
// Description: Read the config file and create an AJI_CLIENT for each server
//              described there.  Return the next available server number
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    DWORD i, j;
    DWORD nextid(1);
    
    if (enable_local_jtag_server && !getenv("QUARTUS_JTAG_CLIENT_NO_LOCAL_SERVER"))
    {
        if (local_link == NULL)
        {
            char c[128+1];
            replace_local_jtag_server = config_get_string(false, "ReplaceLocalJtagServer", c, 128);
            if (!replace_local_jtag_server)
                local_link = new AJI_CLIENT(0, NULL, false, NULL);
            else
                local_link = new AJI_CLIENT(0, c, false, NULL);
        }

        if (m_link.size() == 0)
            m_link.push_back(local_link);
    }
    
    DWORD id;
    char buffer[32], host[64];

    if (m_link.size() > 0)
    {
        // Delete servers which no longer exist
        AJI_CLIENT_VEC::iterator link_iter;
        for (link_iter = m_link.end()-1; link_iter != m_link.begin(); )
        {
            DWORD id = (*link_iter)->m_id;
            bool keep;
            if (id != 0xFF)
            {
                snprintf(buffer, sizeof(buffer), "Remote%ld\\Host", (long)id);
                keep = config_get_string(false, buffer, host, sizeof(host)) &&
                       strcmp_(host, sizeof(host), (*link_iter)->m_hostname) == 0;
            }
            else
            {
                keep = s_temporary_servers.find((*link_iter)->m_hostname) != s_temporary_servers.end();
            }

            if (keep)
                link_iter--;
            else
            {
                // server config has been changed or deleted
                delete (*link_iter);
                m_link.erase(link_iter);
                link_iter = m_link.end()-1;
            }
        }
    }

    // If there are details of remote servers then connect to them.
    i = 0;
    while (config_enumerate(false, &i, buffer, sizeof(buffer)))
    {
        if (strncmp(buffer, "Remote", 6) != 0 || !isdigit(buffer[6]))
            continue;

        id = atoi(buffer+6);
        if (id == 0)
            continue;

        if (id >= nextid)
            nextid = id + 1;

        snprintf(buffer, sizeof(buffer), "Remote%ld\\Host", (long)id);
        if (!config_get_string(false, buffer, host, sizeof(host)))
            continue;

        for (j = 0 ; j < m_link.size() ; j++)
            if (m_link[j]->m_id == id)
                break;

        if (j == m_link.size())
        {
            AJI_CLIENT * client = new AJI_CLIENT(id, host, true, NULL);
            if (client != NULL)
                m_link.push_back(client);
        }
    }

    // Add any temporary servers the user has specified
    STRING_SERVER_MAP::const_iterator k = s_temporary_servers.begin();

    for (; k != s_temporary_servers.end(); k++)
    {
        if (find_server((*k).first) != ~0u)
            continue;

        const AJI_CLIENT * primary = (*k).second->m_primary;
        AJI_CLIENT * client = new AJI_CLIENT(0xFF, (*k).first, true, primary);
        if (client != NULL)
            m_link.push_back(client);
    }

    return nextid;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CLIENT::connect_all(DWORD timeout)
//
// Description: Try and connect to as many as possible of our servers in
//              parallel - this reduces the startup delays.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    int timeout_end = get_time() + timeout;

    DWORD i;
    DWORD n = 0;

    AJI_CLIENT * * active = new AJI_CLIENT * [(unsigned int) m_link.size()];
    if (active == NULL)
        return;

    for (i = 0 ; i < m_link.size() ; i++)
        if (m_link[i]->m_state == CONNECTED)
            ;
        else if (!m_link[i]->try_claim_link(100))
            ;
        else if (!m_link[i]->start_connect())
            m_link[i]->release_link();
        else
            active[n++] = m_link[i];

    while (n > 0 && get_time() - timeout_end < 0)
    {
        for (i = 0 ; i < n ; i++)
            if (active[i]->continue_connect() == 0)
            {
                active[i]->release_link();
                active[i] = active[--n];
            }

        if (n > 0)
            Sleep(100);
    }

    for (i = 0 ; i < n ; i++)
        active[i]->release_link();

    delete[] active;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CLIENT::add_remote_server(const char * server, const char * password, bool temporary)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (server == NULL || password == NULL)
        return AJI_INVALID_PARAMETER;

    if (strnlen_s(server, HOSTNAME_BUFFER_SIZE) == HOSTNAME_BUFFER_SIZE || strnlen_s(password, PASSWORD_BUFFER_SIZE) == PASSWORD_BUFFER_SIZE)
        return AJI_INVALID_PARAMETER;

    if (stricmp(server, "localhost") == 0)
        return AJI_INVALID_PARAMETER;

    // Check if this remote server is already present, if so then don't do
    // anything
    update_servers();
    DWORD i = find_server(server);
    DWORD retry = 0;

    if (i != ~0u)
    {
        if (!temporary && m_link[i]->m_id == 0xFF)
        {
            // User asked for a persistent server but temporary exists, delete the latter
            m_link[i]->remove_remote_server();
            i = ~0u;
        }
        else if (temporary && m_link[i]->m_id != 0xFF)
        {
            // User asked for temporary but persistent exists, switch to persistent
            temporary = false;
        }
    }

    // Adding or updating a temporary server is just a map entry
    if (temporary)
    {
        add_temporary_server_internal(server, password, NULL);
        update_servers();
        return AJI_NO_ERROR;
    }

    while (i == ~0u)
    {
        // Server doesn't exist in the configuration, allocate a new ID and create it.
        // There is a race where two different clients try to allocate a new ID at the
        // same time.  If the race triggers then do it again.
        i = update_servers();

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Remote%ld\\Host", (long)i);
        if (!config_set_string(false, buffer, server, true))
        {
            if (retry++ > 5)
                return AJI_FILE_ERROR;

            // It's possible that the other client is trying to add the same server.  If
            // they have just done so then we don't need to add it again.
            update_servers();
            i = find_server(server);
        }
    }
    
    AJI_DEBUG_ASSERT(i != ~0u);

    char buffer[32], link_password[64];
    snprintf(buffer, sizeof(buffer), "Remote%ld\\Password", (long)i);

    // If the password is already correct then no need to set it again.
    if (config_get_string(false, buffer, link_password, sizeof(link_password)) &&
        stricmp(password, link_password) == 0)
        return AJI_NO_ERROR;

    // Set the password.  If two clients try to set different passwords at the
    // same time then which one wins is random.
    if (!config_set_string(false, buffer, password))
        return AJI_FILE_ERROR;

    update_servers();

    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
DWORD AJI_CLIENT::find_server(const char * server)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    DWORD i;

    for (i = 0 ; i < m_link.size() ; i++)
        if (m_link[i]->m_hostname != NULL && strcmp_(m_link[i]->m_hostname, HOSTNAME_BUFFER_SIZE, server) == 0)
            return i;

    return ~0u;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CLIENT::add_temporary_server_internal(const char * server, const char * password, AJI_CLIENT * primary)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    STRING_SERVER_MAP::iterator j = s_temporary_servers.find(server);
    if (j != s_temporary_servers.end())
    {
        const char * server_prev = (*j).first;
        s_temporary_servers.erase(j);
        delete (char *) server_prev;
    }

    size_t svlen = strnlen_s(server, HOSTNAME_BUFFER_SIZE) + 1;
    size_t pwlen = strnlen_s(password, PASSWORD_BUFFER_SIZE) + 1;
    size_t len = sizeof(AJI_SERVER_INFO) + pwlen + svlen;

    AJI_SERVER_INFO * info = (AJI_SERVER_INFO *) new char[len];
    char * server_dup = (char *)(info + 1);
    strcpy_s(server_dup, svlen, server);
    char * password_dup = server_dup + svlen;
    strcpy_s(password_dup, pwlen, password);

    info->m_server = server;
    info->m_password = password;
    info->m_primary = primary;

    s_temporary_servers[server_dup] = info;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CLIENT::enable_remote_clients(bool enable, const char * password)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // Connect to the local server.  This serves three purposes: it lets us find
    // out whether the local server could provide remote connections; it starts
    // up the server if it was stopped (needed on NT if we are enabling remote)
    // and it wakes up the server so that it can stop itself (NT if disabling
    // remote)
    AJI_ERROR error = check_remote_clients_possible();
    if (error != AJI_NO_ERROR)
        return error;

    // TODO: check that we are root/administrator equivalent.

    if (enable && password != NULL && password[0] != 0)
    {
        if (strnlen_s(password, PASSWORD_BUFFER_SIZE) == PASSWORD_BUFFER_SIZE)
            return AJI_INVALID_PARAMETER;

        // Set the password and enable remote connections.
        config_set_string(true, "Password", password);
    }
    else
    {
        // Disable remote connections in the future.
        config_set_string(true, "Password", NULL);
    }

    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CLIENT::get_remote_clients_enabled(bool * enable)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(enable != NULL);

    // Connect to the server and check whether remote connections are possible.
    AJI_ERROR error = check_remote_clients_possible();
    if (error != AJI_NO_ERROR)
        return error;

    char password[32];
    if (!config_get_string(true, "Password", password, sizeof(password)) ||
        password[0] == 0)
        *enable = false;
    else
        *enable = true;

    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CLIENT::check_remote_clients_possible(void)
//
// Description: Check whether remote clients can be enabled with the currently
//              installed JTAG server.  This differs per build because on UNIX
//              one can run the JTAG server without installing it - this can't
//              support remote clients.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // Check that the local server is running - start it up if it isn't
    AJI_CLIENT * local = local_server();

    if (local != NULL && local->try_claim_link(100))
    {
        bool ok = local->connect();
        local->release_link();
        if (!ok)
            return AJI_SERVER_ERROR;
    }

    // If the server won't allow remote connections (for example if it has been
    // started by a user and will stop automatically soon) then remote clients
    // aren't implemented.
    if ((local->m_server_flags & SERVER_ALLOW_REMOTE) == 0)
        return AJI_UNIMPLEMENTED;

    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
const char * AJI_CLIENT::get_server_version_info(void)
//
// Description: Find out what version the server reports it is running.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // Check that the local server is running - start it up if it isn't
    AJI_CLIENT * local = local_server();

    if (local == NULL)
         return NULL;

    if (local->try_claim_link(100))
    {
        bool ok = local->connect();
        local->release_link();
        if (!ok)
            return NULL;
    }

    return local->m_server_version_info;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
const char * AJI_CLIENT::get_server_path(void)
//
// Description: Find out what version the server reports it is running.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // Check that the local server is running - start it up if it isn't
    AJI_CLIENT * local = local_server();

    if (local == NULL)
        return NULL;

    if (local->try_claim_link(100))
    {
        bool ok = local->connect();
        local->release_link();
        if (!ok)
            return NULL;
    }

    return local->m_server_path;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CLIENT::disconnect_all(bool process_terminate)
//
// Description: Disconnect everything because we are being unloaded
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    for (DWORD i = 0 ; i < m_link.size() ; i++)
        m_link[i]->disconnect(process_terminate);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_CLIENT::AJI_CLIENT(DWORD id, const char * hostname, bool remote, const AJI_CLIENT * primary)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
    : m_id(id), m_remote_server(remote), m_primary(primary),
      m_connect_state(FAIL), m_connect_timeout(0), m_state(IDLE), m_retry_connect(0),
      m_server_version(0), m_server_flags(0), 
      m_server_version_info(NULL), m_server_path(NULL),
      m_potential_strings(NULL), m_record_strings(NULL), m_record_numbers(NULL),
      m_defined_strings(NULL), m_quartus_strings(NULL), m_defined_tag(0),
      m_tx(m_txbuffer, sizeof(m_txbuffer)), m_rx(m_rxbuffer, sizeof(m_rxbuffer)),
 #ifdef _DEBUG
      m_sequence(0),
#endif
      m_partial(NULL)
{
    if (hostname == NULL)
        m_hostname = NULL;
    else
    {
    	size_t buffer_size = strnlen_s(hostname, HOSTNAME_BUFFER_SIZE) + 1;
        m_hostname = new char[buffer_size];
        if (m_hostname != NULL)
            strcpy_s(m_hostname, buffer_size, hostname);
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

AJI_CLIENT::~AJI_CLIENT(void)
{
    if (m_sock >= 0)
        close();

    // Delete any chains which are attached to this link
    CHAINS::iterator i;
    for (i = m_chains.begin() ; i != m_chains.end() ; i++)
        delete (*i).m_chain;

    delete[] m_hostname;
    delete[] m_server_version_info;
    delete[] m_server_path;
    delete[] m_potential_strings;
    delete[] m_record_strings;
    delete[] m_record_numbers;
    delete[] m_defined_strings;
    delete[] m_quartus_strings;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CLIENT::remove_remote_server(void)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // TODO: check whether this server is in use and fail if it is

    if (m_id != 0xFF)
    {
        // Remove from config database
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Remote%ld", (long)m_id);
        config_delete(false, buffer);
    }
    else
    {
        STRING_SERVER_MAP::iterator j = s_temporary_servers.find(m_hostname);
        if (j != s_temporary_servers.end())
        {
            AJI_SERVER_INFO * info = (*j).second;
            if (info->m_primary != NULL)
                return AJI_NO_ERROR; // Can't remove secondary servers manually
            s_temporary_servers.erase(j);
            delete (char *) info; // Delete memory for info, server & password
        }
    }

    // If the server is in the list of remote servers then remove it
    AJI_CLIENT_VEC::iterator i;
    for (i = m_link.begin() ; i != m_link.end() ; i++)
        if ((*i) == this)
        {
            m_link.erase(i);
            break;
        }

    // Delete any secondary servers which point to this one
    AJI_STRING_VEC remove;
    for (STRING_SERVER_MAP::const_iterator k = s_temporary_servers.begin(); k != s_temporary_servers.end(); k++)
        if ((*k).second->m_primary == this)
            remove.push_back((*k).first);

    for (AJI_STRING_VEC::iterator j = remove.begin(); j != remove.end(); j++)
        s_temporary_servers.erase(*j);

    delete this;

    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CLIENT::internal_connect(void)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (!start_connect())
        return false;

    // TODO: make this whole function asynchronous.  For now make async calls
    // to the function which does the real work.

    int delay;
    while ((delay = continue_connect()) != 0)
        Sleep(delay);

    return (m_state == CONNECTED);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CLIENT::start_connect(void)
//
// Description: Start connecting this client to its server.
//
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(link_is_claimed());
    AJI_DEBUG_ASSERT(m_state != CONNECTED);

    if (m_state == CONNECTING)
        return true;

    int now = get_time();

    if (m_state != IDLE && now - m_retry_connect < 0)
    {
        // Don't try to connect again if we have failed to connect recently (or
        // have been disconnected by an error recently).  This cuts down network
        // traffic because a) the reachability of the server is unlikely to change
        // within a smaller timescale than this and b) callers will typically call
        // get_hardware and similar functions several times in a short space of
        // time.  It can take a while to connect so this would at least double the
        // time taken to check out which servers are present.
        return false;
    }

    // Wait 30s before reconnecting after a failure to connect.
    m_retry_connect = now + 30 * 1000;

    // Start connecting now.
    m_state = CONNECTING;
    m_connect_state = LOOKUP;
    close();

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int AJI_CLIENT::continue_connect(void)
//
// Description: Do as much as possible to connect to this server without
//              blocking.
//
// Returns:     0 if the connection has succeeded or failed or the suggested
//              time before we are called again.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(link_is_claimed());
    AJI_DEBUG_ASSERT(m_state != CONNECTED);

    for (; ; ) {
        switch (m_connect_state)
        {
        case SUCCESS:
            // Shouldn't happen.
            return 0;

        case FAIL:
            // The raw connect has failed - there is nothing listening on the port or
            // a network problem is stopping us from connecting.
            m_state = NOSERVER;
            m_retry_connect = get_time() + 30 * 1000;
            close();
            return 0;

        case LOOKUP:
        {
            // In this state we resolve the name given to one or more IP addresses
            int port = JTAG_PORT;
            char *hostname = 0;
            if (m_hostname != NULL) 
            {
                hostname = strdup(m_hostname);
                char *port_seperator = strchr(hostname,':');
                if (port_seperator != NULL)
                {
                    *port_seperator = 0;
                    port = atoi(port_seperator+1);
                }
            }
            if (!TCPCLIENT::lookup(hostname, port))
                m_connect_state = FAIL;
            else
            {
                m_connect_state = FIRSTTRY;
                m_connect_timeout = get_time() + 30000;
            }

            if (hostname != NULL)
            {
                free(hostname);
            }
            break;
        }

        case FIRSTTRY:
        case RETRY:
            // In these states we are trying to connect to the server.  
            // For local servers we try to start the local server and reconnect -  #no longer true
            // moving to the RETRY state to ensure that this is only done once.
            if (!TCPCLIENT::connect())
            {
                // Connect is still in progress (no success or failure yet)
                if (get_time() - m_connect_timeout > 0)
                    m_connect_state = FAIL;
                else
                    return 100;
            }
            else if (TCPCLIENT::is_connected())
            {
                // Connect has succeeded.
                // Set up pointers to receive the initial response from the server.
                m_rx.set_buffer(m_rxbuffer, sizeof(m_rxbuffer));

                m_connect_timeout = get_time() + 10000;
                m_connect_state = WAITSERVER;
            }
            else if (m_hostname != NULL)
            {
                // Connect to a remote server has failed.  Give up.
                m_connect_state = FAIL;
            } 
            else if (m_connect_state == FIRSTTRY)
            {
                // Connect to the local server has failed for the first time.
                // If we can start the local server (in case it was stopped before)
                // then try to connect to it for upto 5s.
                if (!replace_local_jtag_server && enable_local_jtag_server && !getenv("QUARTUS_JTAG_CLIENT_NO_LOCAL_SERVER") && start_local_server())
                {
                    m_connect_timeout = get_time() + 5000;
                    m_connect_state = RETRY;
                }
                else
                    m_connect_state = FAIL;
            } 
            else
            {
                // Connect to a local server has failed again.  Keep trying until the
                // timeout expires.
                return 100;
            }
            break;

        case WAITSERVER:
            // In this state we're waiting for the server to send us an initial
            // message.  If it hasn't sent us one in ten seconds then it's
            // probably crashed.
            if (receive_timeout(0) == AJI_NO_ERROR)
            {
                // There's something there.  Check we are compatible with it and
                // authenticate ourselves to it if necessary.  prepare_connection will
                // set m_state to the correct value.
                if (prepare_connection())
                    m_connect_state = SUCCESS;
                else
                {
                    m_connect_state = FAIL;
                    close();
                }
                return 0;
            }
            else if (get_time() - m_connect_timeout > 0)
                m_connect_state = FAIL;
            else
                return 100;
            break;

        default:
            m_connect_state = FAIL;
            return 0;
        }
    } //end for(;;)
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CLIENT::start_local_server(void)
//
// Description: Attempt to start the local JTAG server as it does not appear
//              to be running.  If two users attempt to start the JTAG server
//              at the same time then the server which fails to bind to the
//              port will silently exit.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    const char* serverline = getenv("QUARTUS_JTAG_SERVER_CLI");
    if (serverline) {

        char cmdline[512];
        strcpy_s(cmdline, sizeof(cmdline), serverline);

        const int MAX_ARGS_SIZE = 20;
        const char* args[MAX_ARGS_SIZE];
        int  argsize = 0;
        rsize_t remain = strnlen_s(cmdline, sizeof(cmdline));
        char* token, * innard;
        const char delim = ' ';
        token = strtok_s(cmdline, &remain, &delim, &innard);

        for (token = strtok_s(NULL, &remain, &delim, &innard);
            token && argsize < MAX_ARGS_SIZE;
            token = strtok_s(NULL, &remain, &delim, &innard), ++argsize
            ) {
            args[argsize] = token;
        }

        return start_quartus_process(cmdline, argsize, args, true);
    } else {
#if PORT == UNIX
        const int buffer_size = 512;
        char config[buffer_size];

        const char* quartus_rootdir = getenv("QUARTUS_ROOTDIR");
        const char* home = getenv("HOME");

        if (home != NULL && home[0] != 0)
        {
            // When running as a user store configuration in their home directory.
            const char* slash = home[strnlen_s(home, _MAX_PATH) - 1] != '/' ? "/" : "";
            snprintf(config, sizeof(config), "%s%s.jtagd.conf", home, slash);
        }
        else if (quartus_rootdir != NULL && quartus_rootdir[0] != 0)
        {
            // $HOME isn't set - try using the install directory (it might work)
            const char* slash = quartus_rootdir[strnlen_s(quartus_rootdir, _MAX_PATH) - 1] != '/' ? "/" : "";
            snprintf(config, sizeof(config), "%s%sbin/jtagd.conf", quartus_rootdir, slash);
        }
        else {
            strcpy_s(config, buffer_size, "jtagd.conf");
        }

        // Start the JTAG daemon running as the current user (for use from the
        // local machine only, stops when finished).
        const char* args[3];
        args[0] = "--user-start";
        args[1] = "--config";
        args[2] = config;

        return start_quartus_process("jtagd", 3, args, false);
#else
        const char* args[1];

        // For the first try we try and start an installed jtagserver process
        // We issue the command "jtagserver --start" as this is most likely to work
        // in all cases.  On NT this works for users without administrative rights
        // wheras --install doesn't.  On 98 both are equivalent.
        args[0] = "--start";
        if (start_quartus_process("jtagserver.exe", 1, args, true))
            return true;

        // For the second try we run jtagserver in the foreground using the idle-stop
        // flag (so that it terminates after it's been idle for some time) as that
        // will work if it hasn't been installed correctly.  This is a bit hacky as
        // it will fail if another program has randomly claimed the TCP port so only
        // try it if all else fails.
        args[0] = "--idle-stop";
        return start_quartus_process("jtagserver.exe", 1, args, false);
#endif
    } //else-if (serverline)
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CLIENT::prepare_connection(void)
//
// Description: Carry out the initial protocol negotiation with the server,
//              authenticating and setting up the protocol if necessary.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    // There's something there.  Check whether we are compatible with it.
    const char * signature = NULL;
    DWORD authtype(0);
    char strings[16];

    m_rx.single_block_buffer();
    m_rx.set_string(strings, sizeof(strings));

    if (!m_rx.remove_string(&signature) || 
        !m_rx.remove_int(&m_server_version) || m_server_version > 0xFFFF ||
        !m_rx.remove_int(&authtype) ||
        signature == NULL || strcmp_(signature, sizeof(strings), AJI_SIGNATURE) != 0)
    {
        m_state = BADVERSION;
        return false;
    }

    // We are compatible with it.  A failure now will be due to authentication
    // failure.
    m_state = AUTHFAIL;

    if (authtype == MESSAGE::AUTHENTICATE_MD5)
    {
        // Server wants us to authenticate.  If we know the password then respond
        // to their challenge.
        BYTE challenge[16];
        if (!m_rx.remove_raw(challenge, 16))
            return false;

        char password[64];
        if (!get_password(password, sizeof(password)))
            return false;

        MD5_CTX context;
        BYTE hash[16];

        MD5Init(&context);
        MD5Update(&context, challenge, 16);
        MD5Update(&context, password, static_cast<unsigned int> (strnlen_s(password, 64)));
        MD5Final(hash, &context);

        m_tx.set_buffer(m_txbuffer, sizeof(m_txbuffer));
        m_tx.add_command(MESSAGE::AUTHENTICATE_MD5);
        m_tx.add_raw(hash, 16);

        if (send_receive() != AJI_NO_ERROR)
            return false;

        m_rx.set_buffer(m_rxbuffer, sizeof(m_rxbuffer));

        AJI_ERROR error = AJI_NO_ERROR; // Prevent warning
        if (!m_rx.remove_response(&error) || error != AJI_NO_ERROR)
            return false;
    }
    else if (authtype != 0)
        return false;

    // Select the highest protocol version understood by us and by the server.
    // A failure now indicates that the server is broken.
    m_state = BROKEN;

    if (m_server_version >= 2)
    {
        if (m_server_version > AJI_CURRENT_VERSION)
            m_server_version = AJI_CURRENT_VERSION;

        clear_tx_buffer();
        m_tx.add_command(MESSAGE::USE_PROTOCOL_VERSION);
        m_tx.add_int(m_server_version);

        m_tx.add_command(MESSAGE::GET_VERSION_INFO);

        if (send_receive() != AJI_NO_ERROR)
            return false;

        AJI_ERROR resp(AJI_NO_ERROR);
        if (!m_rx.remove_response(&resp) || resp != AJI_NO_ERROR ||
            !m_rx.remove_int(&m_server_flags))
            return false;

        char info[SERVER_VERSION_INFO_BUFFER_SIZE];
        m_rx.set_string(info, sizeof(info));

        const char * version_info;
        if (!m_rx.remove_response(&resp) || resp != AJI_NO_ERROR ||
            !m_rx.remove_string(&version_info))
            return false;

        delete[] m_server_version_info;
        m_server_version_info = NULL;
        size_t version_info_len = strnlen_s(version_info, 256);
        if (version_info_len != 0 && version_info_len != 256)
        {
            m_server_version_info = new char[version_info_len+1];
            strcpy_s(m_server_version_info, version_info_len+1, version_info);
        }

        //if (!m_rx.remove_int(&m_pgmparts_version))
        //    m_pgmparts_version = 0;
        m_rx.remove_int(&m_pgmparts_version); 
        m_pgmparts_version = 0;

        delete[] m_server_path;
        m_server_path = NULL;
        const char * server_path;
        if (m_rx.remove_string(&server_path))
        {
        	// server_path buffer size is 512 bytes in JTAG_SERVER_LINK::get_version_info()
            size_t server_path_len = strnlen_s(server_path, 512);
            if (server_path_len != 0 && server_path_len != 512)
            {
                m_server_path = new char[server_path_len+1];
                strcpy_s(m_server_path, server_path_len+1, server_path);
            }
        }

        m_rx.set_buffer(m_rxbuffer, sizeof(m_rxbuffer));
    }
    else
        m_server_flags = 0;

    //load_quartus_devices(get_pgm_parts_path(false));
    
    // Connected to remote server.  Good.
    clear_tx_buffer();
    m_state = CONNECTED;

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CLIENT::check_password(DWORD authtype, const BYTE * challenge, const BYTE * response)
//
// Description: Check whether the challenge/response matches the password set
//              for the local server.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_CLIENT * client = AJI_CLIENT::local_server();

    if (client == NULL)
        return AJI_SERVER_ACTIVE;

    if (!client->try_claim_link(100))
        return AJI_SERVER_ACTIVE;

    AJI_ERROR error;

    if (!client->connect())
        return AJI_SERVER_ERROR;
    else if (authtype != MESSAGE::AUTHENTICATE_MD5)
        error = AJI_INVALID_PARAMETER;
    else
    {
        TXMESSAGE * tx = client->get_txmessage(NULL);
        RXMESSAGE * rx = client->get_rxmessage();

        tx->add_command(MESSAGE::CHECK_PASSWORD);
        tx->add_int(authtype);
        tx->add_raw(challenge, 16);
        tx->add_raw(response, 16);

        error = client->send_receive();
        if (error == AJI_NO_ERROR && !rx->remove_response(&error))
            error = AJI_SERVER_ERROR;
    }

    if (client != NULL)
        client->release_link();

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CLIENT::get_password(char * password, int passlen) const
//
// Description: Get the password for this server.
//
// Returns:     true if we added or removed secondary servers
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (m_primary != NULL)
        return m_primary->get_password(password, passlen);

    if (m_id != 0xFF)
    {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Remote%ld\\Password", (long)m_id);
        if (!config_get_string(false, buffer, password, passlen) || password[0] == 0)
            return false;
    }
    else
    {
        AJI_SERVER_INFO * info = s_temporary_servers[m_hostname];
        const char * pw = info->m_password;
        if (pw == NULL)
            return false;
        strncpy_s(password, passlen, pw, passlen-1);
        password[passlen-1] = 0;
    }

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CLIENT::update_secondaries(void)
//
// Description: If this is not a secondary server then ask the server whether
//              it knows about any secondaries.
//
// Returns:     true if we added or removed secondary servers
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (m_primary != NULL)
        return false; // No need to check

//    AJI_RECORD * records = NULL;
//    DWORD record_n = 32;
//    AJI_ERROR error;

/*    do {
        delete[] records;
        records = new AJI_RECORD[record_n];
        error = refresh_records(&record_n, records, 0, NULL);
    } while (error == AJI_TOO_MANY_DEVICES);

    if (error != AJI_NO_ERROR)
    {
        delete[] records;
        
        return false;
    }
 */   
    AJI_STRING_VEC remove;
    for (STRING_SERVER_MAP::const_iterator k = s_temporary_servers.begin(); k != s_temporary_servers.end(); k++)
        if ((*k).second->m_primary == this)
            remove.push_back((*k).first);

    bool changed = false;
    for (AJI_STRING_VEC::iterator j = remove.begin(); j != remove.end(); j++)
    {
        s_temporary_servers.erase(*j);
        changed = true;
    }

    // TODO: Delete old secondaries
    return changed;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CLIENT::send(void)
//
// Description: Send a command to the server.  Don't wait for a response.
//
// Returns:     false if the network connection has failed (in which case
//              the disconnect() function may have invalidated lots of stuff).
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(link_is_claimed());

    bool ok = TCPLINK::send(0, m_tx.get_data(), m_tx.get_length(), false);

    if (ok)
        ok = send_fifos();
    if (ok)
        ok = send_flush();

    clear_tx_buffer();
    m_partial = NULL;

    if (!ok)
        disconnect(false);

    return ok;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CLIENT::disconnect(bool process_terminate)
{
    // Be paranoid about opens or chains calling disconnect more than once.
    AJI_DEBUG_ASSERT(this != NULL);

    // Close the TCP socket.  We must not do this if the process is terminating
    // as the winsock DLL might have been terminated already.  If it has then
    // calling functions like this will make it crash.
    if (!process_terminate)
        close();

    // Tidy up internal state.
    m_connect_state = FAIL;
    m_state = BROKEN;
    m_retry_connect = get_time() + 5 * 1000;

    // Invalidate any open devices which are linked to this client
    AJI_OPEN_JS::invalidate(this);

    // Invalidate any chains which are linked to this client
    CHAINS::iterator i;
    for (i = m_chains.begin() ; i != m_chains.end() ; i++)
        (*i).m_chain->invalidate(this);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
TXMESSAGE * AJI_CLIENT::get_txmessage(AJI_OPEN_JS * open)
{
    AJI_DEBUG_ASSERT(link_is_claimed());

    if (m_partial != NULL && m_partial != open)
    {
        // This is a different client/device from the last message.  Send it and
        // store the error.
        m_partial->send_deferred();
    }

    m_partial = open;
    return &m_tx;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void  AJI_CLIENT::txmessage_cancel(AJI_OPEN_JS * open)
{
    if (m_partial == open)
    {
        m_partial = NULL;
        clear_tx_buffer();
        // TODO: Release FIFOs
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
BYTE * AJI_CLIENT::get_mux0_receive_buffer(void)
{
    return m_rxbuffer; 
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CLIENT::receive_timeout(DWORD timeout)
{
    AJI_DEBUG_ASSERT(link_is_claimed());

    int len = TCPLINK::receive_cmnd(timeout);

    if (len > 0)
    {
        m_rx.set_buffer(m_rxbuffer, len);
#ifdef _DEBUG
        if (m_server_version >= 12)
        {
            // Ignore the response to the PING command
            AJI_ERROR resp;
            m_rx.remove_response(&resp);
        }
#endif
        return AJI_NO_ERROR;
    }

    switch (len)
    {
    case WOULDBLOCK:
        return AJI_TIMEOUT;

    case DISCONNECT:
    default:
        return AJI_NET_DOWN;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CLIENT::refresh_hardware(bool check_secondaries)
//
// Returns:     true if secondary servers have been modified
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    bool mutex_claimed = try_claim_link(100);
    bool changed = false;

    CHAINS::iterator i;
    for (i = m_chains.begin() ; i != m_chains.end() ; i++)
        (*i).m_destroy = mutex_claimed;

    // Reconnect if connection lost
    if (mutex_claimed)
    {
        if (m_state != CONNECTED && m_state != CONNECTING)
            connect();

        if (m_state == CONNECTED)
        {
            if (check_secondaries)
                changed |= update_secondaries();
            get_hardware_from_server();
        }
    }

    // Add a dummy entry so that users can remove
    if (m_hostname != NULL && m_primary == NULL)
        switch (m_state)
        {
        case CONNECTING:
            refresh_chain(0, NULL, "Connecting ...", NULL, AJI_CHAIN_UNKNOWN, AJI_FEATURE_DUMMY);
            break;

        case CONNECTED:
            {
                bool has_real_chain = false;
                for (i = m_chains.begin() ; i != m_chains.end() ; i++)
                    if (!(*i).m_chain->is_dummy())
                        has_real_chain = true;
                if (!has_real_chain)
                    refresh_chain(0, NULL, "No hardware available", NULL, AJI_CHAIN_UNKNOWN, AJI_FEATURE_DUMMY);
            }
            break;

        case NOSERVER:
            refresh_chain(0, NULL, "Unable to connect", NULL, AJI_CHAIN_UNKNOWN, AJI_FEATURE_DUMMY);
            break;

        case BADVERSION:
            refresh_chain(0, NULL, "Incompatible server version", NULL, AJI_CHAIN_UNKNOWN, AJI_FEATURE_DUMMY);
            break;

        case AUTHFAIL:
            refresh_chain(0, NULL, "Authentication failure", NULL, AJI_CHAIN_UNKNOWN, AJI_FEATURE_DUMMY);
            break;

        case IDLE:
        case BROKEN:
        default:
            refresh_chain(0, NULL, "Problem with remote server", NULL, AJI_CHAIN_UNKNOWN, AJI_FEATURE_DUMMY);
            break;
        }

    // Remove any chains which the server didn't tell us about (they must have
    // been deleted.
    for (i = m_chains.begin() ; i != m_chains.end() ; )
        if ((*i).m_destroy)
        {
            delete (*i).m_chain;

            CHAINS::size_type offset = i - m_chains.begin();
            m_chains.erase(i);
            i = m_chains.begin() + offset;
        }
        else
            i++;

    if (mutex_claimed)
        release_link();

    return changed;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_CLIENT::get_hardware_from_server(void)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    TXMESSAGE * tx = get_txmessage(NULL);
    RXMESSAGE * rx = get_rxmessage();
    char string[512];

    tx->add_command(MESSAGE::GET_HARDWARE);

    // Create a receive FIFO of unknown length to receive the data (v1+ only)
    RXRAWFIFO hardware_list(this);

    // Version 0 servers don't support sending a version request in the packet.
    // For version 1+ servers we ask for the most recent information we will
    // understand.
    if (m_server_version > 0)
    {
        tx->add_int(m_server_version);
        hardware_list.activate(0);
    }

    if (send_receive() != AJI_NO_ERROR)
        return false;

    AJI_ERROR resp(AJI_NO_ERROR);
    if (!rx->remove_response(&resp) || resp != AJI_NO_ERROR)
        return false;

    DWORD n(0);
    if (!rx->remove_int(&n))
        return false;

    if (m_server_version >= 1)
    {
        DWORD fifo_len(0);
        if (!rx->remove_int(&fifo_len))
            return false;

        if (fifo_len > 0)
        {
            // Download the potential hardware list via the first FIFO.
            if (!hardware_list.wait_for_data(fifo_len))
            {
                disconnect(false);
                return false;
            }

            RXMESSAGE message(hardware_list.get_data(), fifo_len);
            message.single_block_buffer();

            for (DWORD j = 0 ; j < n ; j++)
            {
                message.set_string(string, sizeof(string));             

                DWORD chain_id(0);
                const char * hw_name = NULL;
                const char * port = NULL;
                const char * device_name = NULL;
                DWORD chain_type(0);
                DWORD features(0);

                rx->set_string(string, sizeof(string));

                if (!message.remove_int(&chain_id) || !message.remove_string(&hw_name) ||
                        !message.remove_string(&port)  || !message.remove_int(&chain_type) ||
                        !message.remove_string(&device_name))
                    break;

                // Features word is only provided by versions 2+
                if (m_server_version >= 2 && !message.remove_int(&features))
                    break;

                refresh_chain(chain_id, hw_name, port, device_name,
                              static_cast<AJI_CHAIN_TYPE>(chain_type), features);
            }
        }
    }
    else
    {
        for (DWORD j = 0 ; j < n ; j++)
        {
            DWORD chain_id(0), hw_type(0);
            const char * port = NULL;
            const char * hw_name = NULL;
            const char * device_name = NULL;
            DWORD chain_type(0);

            rx->set_string(string, sizeof(string));

            if (!rx->remove_int(&chain_id) || !rx->remove_int(&hw_type) ||
                    !rx->remove_string(&port)  || !rx->remove_string(&hw_name) ||
                    !rx->remove_int(&chain_type))
                break;

            // Fill in hw_name if not provided by server.  Version 1 servers always provide
            // a name so this switch only needs to contain the hardware supported by
            // version 0 servers.
            if (hw_name == NULL || hw_name[0] == 0)
                switch (hw_type)
                {
                case AJI_HW_BYTEBLASTER:
                    hw_name = "ByteBlaster";
                    break;
                }

            refresh_chain(chain_id, hw_name, port, device_name,
                                        static_cast<AJI_CHAIN_TYPE>(chain_type), 0);
        }
    }

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

void AJI_CLIENT::refresh_chain(unsigned int chain_id, const char * hw_name, 
                               const char * port, const char * name, AJI_CHAIN_TYPE chain_type, DWORD features)
{
    if (port != NULL && port[0] == 0)
        port = NULL;

    if (name != NULL && name[0] == 0)
        name = NULL;

    for (CHAINS::iterator i = m_chains.begin() ; i != m_chains.end() ; i++)
        if ((*i).m_chain->get_id() == chain_id)
        {
            (*i).m_chain->set_strings(hw_name, port, name, m_hostname);
            (*i).m_chain->set_features(features);
            (*i).m_destroy = false;
            return;
        }

    CHAINPTR cp;
    cp.m_destroy = false;

    cp.m_chain = new AJI_CHAIN_JS(this, chain_id, chain_type, features);
    if (cp.m_chain == NULL)
        return;
    cp.m_chain->set_strings(hw_name, port, name, m_hostname);

    m_chains.push_back(cp);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CLIENT::define_device(const AJI_DEVICE * device, bool define)
//
// Description: Define a device (if define == true) or undefine it (if false)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEVICE device_copy;

    if (device == NULL ||
        device->mask & 1 ||
        device->device_name == NULL || device->device_name[0] == 0 ||
        ((device->device_id != 0 && (device->device_id & 1) == 0)))
        return AJI_INVALID_PARAMETER;

    if (define)
    {
        if (device->instruction_length < 2) // || device->instruction_length > 255)   //   instruction_length is a BYTE
            return AJI_INVALID_PARAMETER;
    }
    else
    {
        device_copy.device_id   = device->device_id;
        device_copy.mask        = 0;
        device_copy.device_name = device->device_name;
        device_copy.instruction_length  = 0;
        device_copy.features            = 0;

        device = &device_copy;
    }

    DWORD i;

    update_servers();

    for (i = 0 ; i < m_link.size() ; i++)
        if (m_link[i]->try_claim_link(100))
        {
            m_link[i]->define_device(0, 0, device);
            m_link[i]->release_link();
        }

    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

AJI_ERROR AJI_CLIENT::define_device(DWORD chain_id, DWORD tap_position, const AJI_DEVICE * device)
{
    if (!connect())
    {
        release_link();
        return AJI_NET_DOWN;
    }

    TXMESSAGE * tx = get_txmessage(NULL);

    tx->add_command(MESSAGE::DEFINE_DEVICE);
    tx->add_int(chain_id);
    tx->add_int(tap_position);
    tx->add_int(device->device_id);
    tx->add_int(device->instruction_length);
    tx->add_int(device->features);
    tx->add_int(device->mask);
    tx->add_int(0);
    tx->add_string(device->device_name);

    AJI_ERROR error = send_receive();
    if (error == AJI_NO_ERROR && !m_rx.remove_response(&error))
        error = AJI_NET_DOWN;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_CLIENT::get_defined_devices(DWORD * device_count, AJI_DEVICE * device_list)
//
// Description: Get the list of devices which the JTAG server knows about but
//              which are not in the device file shipped with Quartus.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    update_servers();

    DWORD i, j, k, count;
    for (i = 0, count = 0 ; i < m_link.size() ; i++)
    {
        // Get the list of defined devices from this server
        count += m_link[i]->refresh_defined_devices();
    }

    if (*device_count < count)
    {
        *device_count = count;
        return AJI_TOO_MANY_DEVICES;
    }

    // Copy the details into the users buffer.  Make sure we don't put the same
    // device in twice.
    for (i = 0, count = 0 ; i < m_link.size() ; i++)
    {
        const AJI_DEVICE_VEC & defined = m_link[i]->m_defined;
        DWORD first = count;
    
        for (j = 0 ; j < defined.size() ; j++)
        {
            bool duplicate = false;

            for (k = 0 ; k < first && !duplicate ; k++)
                if (defined[j].device_id == 0)
                    duplicate = stricmp(defined[j].device_name, device_list[k].device_name) == 0;
                else
                    duplicate = defined[j].device_id == device_list[k].device_id;

            if (!duplicate)
                device_list[count++] = defined[j];
        }
    }

    *device_count = count;
    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
DWORD AJI_CLIENT::refresh_defined_devices(void)
{
    if (!try_claim_link(100))
        return static_cast<DWORD> (m_defined.size());

    if (!connect())
    {
        release_link();
        return static_cast<DWORD> (m_defined.size());
    }

    TXMESSAGE * tx = get_txmessage(NULL);

    tx->add_command(MESSAGE::GET_DEFINED_DEVICES);
    tx->add_int(m_defined_tag);

    RXRAWFIFO devicelist(this);
    devicelist.activate(0);

    AJI_ERROR error = send_receive();

    if (error == AJI_NO_ERROR)
        if (!m_rx.remove_response(&error))
            error = AJI_SERVER_ERROR;

    DWORD defined_tag(0);
    DWORD device_count(0);
    DWORD fifo_len(0);

    if (error != AJI_NO_ERROR ||
        !m_rx.remove_int(&defined_tag) ||
            !m_rx.remove_int(&device_count) ||
            !m_rx.remove_int(&fifo_len))
    {
        disconnect(false);
        m_defined.erase(m_defined.begin(), m_defined.end());
        m_defined_tag = 0;
        release_link();
        return 0;
    }

    if (device_count == 0)
    {
        m_defined.erase(m_defined.begin(), m_defined.end());
    }
    else if (fifo_len > 0)
    {
        // Download the new TAP list via the first FIFO.  If no data was sent by
        // the server then this means the TAP list hasn't changed.
        m_defined.erase(m_defined.begin(), m_defined.end());

        m_defined.reserve(device_count);

        if (devicelist.wait_for_data(fifo_len))
        {
            int stringlen = 0;

            for (int i = 0 ; i < 2 ; i++)
            {
                RXMESSAGE message(devicelist.get_data(), fifo_len);
                message.set_string(m_defined_strings, stringlen);               
                message.single_block_buffer();

                for (DWORD j = 0 ; j < device_count ; j++)
                {
                    AJI_DEVICE dev;
                    DWORD instruction_length(0), dummy(0);

                    if (!message.remove_int(&dev.device_id) ||
                        !message.remove_int(&instruction_length) ||
                        !message.remove_int(&dev.features) ||
                        !message.remove_int(&dev.mask) ||
                        !message.remove_int(&dummy) ||
                        !message.remove_string(&dev.device_name))
                    {
                        i = 2;
                        break;
                    }
        
                    dev.instruction_length = static_cast<BYTE>(instruction_length);

                    if (i == 1)
                        m_defined.push_back(dev);
                }

                if (i == 0)
                {
                    stringlen = message.get_string_overflow();
                    delete[] m_defined_strings;
                    m_defined_strings = new char[stringlen];
                }
            }
        }
    }

    m_defined_tag = defined_tag;
    release_link();

    return static_cast<DWORD> (m_defined.size());
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

DWORD AJI_CLIENT::refresh_quartus_devices(DWORD * expected_count, AJI_DEVICE * device_list)
{
    if (!try_claim_link(100))
        return static_cast<DWORD> (0);

    if (!connect())
    {
        release_link();
        return static_cast<DWORD> (0);
    }

    TXMESSAGE * tx = get_txmessage(NULL);

    tx->add_command(MESSAGE::GET_QUARTUS_DEVICES);

    RXRAWFIFO devicelist(this);
    devicelist.activate(0);

    AJI_ERROR error = send_receive();

    if (error == AJI_NO_ERROR)
        if (!m_rx.remove_response(&error))
            error = AJI_SERVER_ERROR;

    DWORD device_count(0);
    DWORD fifo_len(0);

    if (error != AJI_NO_ERROR ||
            !m_rx.remove_int(&device_count) ||
            !m_rx.remove_int(&fifo_len))
    {
        disconnect(false);
        release_link();
        return 0;
    }

    if ((device_count > 0) && 
         (fifo_len > 0) && 
         (device_list != NULL) && 
         (*expected_count == device_count))
    {
        if (devicelist.wait_for_data(fifo_len))
        {
            int stringlen = 0;

            for (int i = 0 ; i < 2 ; i++)
            {
                RXMESSAGE message(devicelist.get_data(), fifo_len);
                message.set_string(m_quartus_strings, stringlen);               
                message.single_block_buffer();

                for (DWORD j = 0 ; j < device_count ; j++)
                {
                    AJI_DEVICE dev;
                    DWORD instruction_length(0), dummy(0);

                    if (!message.remove_int(&dev.device_id) ||
                        !message.remove_int(&instruction_length) ||
                        !message.remove_int(&dev.features) ||
                        !message.remove_int(&dev.mask) ||
                        !message.remove_int(&dummy) ||
                        !message.remove_string(&dev.device_name))
                    {
                        i = 2;
                        break;
                    }
        
                    dev.instruction_length = static_cast<BYTE>(instruction_length);

                    if (i == 1)
                        device_list[j] = dev;

                    if (i == 0)
                    {
                        stringlen = message.get_string_overflow();
                        delete[] m_quartus_strings;
                        m_quartus_strings = new char[stringlen];
                    }
                }

                if (i == 0)
                {
                    stringlen = message.get_string_overflow();
                }
            }
        }
    }

    release_link();

    return device_count;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CLIENT::process_notify(const BYTE * data, DWORD len)
{
    RXMESSAGE rx(data, len);
    MESSAGE::COMMAND cmnd(static_cast<MESSAGE::COMMAND>(0));

    if (!rx.remove_command(&cmnd))
        return;

    switch (cmnd)
    {
    case MESSAGE::PROGRESS:
        {
        DWORD data_used(0);
        if (!rx.remove_int(&data_used))
            return;

        if (m_partial != NULL)
            m_partial->notify_data_used(data_used);
        }
        break;

    case MESSAGE::OUTPUT:
        if (m_output_fn != NULL)
        {
            DWORD level(0);
            if (!rx.remove_int(&level))
                return;

            DWORD skip = 8; // TODO: get offset in block from 'rx'
            (*m_output_fn)(m_output_handle, level, reinterpret_cast<const char *>(data + skip));
        }
        break;

    default:
        break;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_CLIENT::register_output_callback(void ( * output_fn)(void * handle, DWORD level, const char * line), void * handle)
{
    m_output_fn = output_fn;
    m_output_handle = handle;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

