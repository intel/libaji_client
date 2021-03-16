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
// Description: The entry point for AJI Client DLL
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

#ifndef INC_AJI_H
#include "aji.h"
#endif

#ifndef JTAG_CLIENT_AJI_H
#include "jtag_client_aji.h"
#endif

#ifndef JTAG_CLIENT_LINK_H
#include "jtag_client_link.h"
#endif

#ifndef JTAG_CLIENT_CHAIN_H
#include "jtag_client_chain.h"
#endif

#ifndef JTAG_CLIENT_OPEN_H
#include "jtag_client_open.h"
#endif


#define AJI_API JTAG_DLLEXPORT

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
#ifdef WIN32

extern HINSTANCE jtag_client_module;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        jtag_client_module = static_cast<HINSTANCE>(hModule);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        // According to an article on MSDN, the reserved field is NULL if the
        // detach is due to FreeLibrary, non-NULL if the detach is due to process
        // termination.  In the latter case we can't make any calls to other DLLs
        // because they might have already been terminated.
        AJI_CLIENT::disconnect_all(reserved != NULL);
        jtag_client_module = NULL;
        break;
    }

    return TRUE;
}

#endif

AJI_OPEN::OPENS AJI_OPEN::m_valid_set;

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_API const char * aji_get_error_info(void)
{
    return AJI_CHAIN::get_error_info();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_check_password(DWORD authtype, const BYTE * challenge, const BYTE * response)
{
    return AJI_CLIENT::check_password(authtype, challenge, response);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_get_hardware(DWORD * hardware_count,
                                   AJI_HARDWARE * hardware_list,
                                   DWORD timeout)
{
    return AJI_CHAIN_JS::get_hardware(hardware_count, hardware_list, NULL, timeout);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_get_hardware2(DWORD * hardware_count,
                                    AJI_HARDWARE * hardware_list,
                                    char ** server_version_info_list,
                                    DWORD timeout)
{
    return AJI_CHAIN_JS::get_hardware(hardware_count, hardware_list, server_version_info_list, timeout);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_find_hardware(DWORD persistent_id,
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
    return AJI_CHAIN_JS::find_hardware(persistent_id, hardware, timeout);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_find_hardware(const char * hw_name,
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
    return AJI_CHAIN_JS::find_hardware(hw_name, hardware, timeout);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_print_hardware_name(AJI_CHAIN_ID         chain_id,
                                          char               * hw_name,
                                          DWORD                hw_name_len)
{
    return chain_id->print_hardware_name(hw_name, hw_name_len);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_print_hardware_name(AJI_CHAIN_ID         chain_id,
                                          char               * hw_name,
                                          DWORD                hw_name_len,
                                          bool                 explicit_localhost ,
                                          DWORD              * needed_hw_name_len)
{
    return chain_id->print_hardware_name(hw_name, hw_name_len, explicit_localhost , needed_hw_name_len);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_add_hardware(const AJI_HARDWARE * hardware)
{
    return AJI_CHAIN_JS::add_hardware(hardware);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_remove_hardware(AJI_CHAIN_ID chain_id)
{
    return chain_id->remove_hardware();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//

AJI_ERROR AJI_API aji_add_remote_server       (const char         * server,
                                              const char         * password)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    return AJI_CLIENT::add_remote_server(server, password, false);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_add_remote_server       (const char         * server,
                                               const char         * password,
                                               bool                 temporary)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    return AJI_CLIENT::add_remote_server(server, password, temporary);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
// 
/*
AJI_ERROR AJI_API aji_get_servers             (DWORD              * count,
                                               const char       * * servers,
                                               bool                 temporary)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    return AJI_CLIENT::get_servers(count, servers, temporary);
}
*/

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
// 
AJI_ERROR AJI_API aji_enable_remote_clients   (bool                 enable,
                                               const char         * password)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    return AJI_CLIENT::enable_remote_clients(enable, password);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//

AJI_ERROR AJI_API aji_get_remote_clients_enabled(bool             * enable)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    return AJI_CLIENT::get_remote_clients_enabled(enable);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_API const char * aji_get_server_version_info(void)
{
    return AJI_CLIENT::get_server_version_info();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_API const char * aji_get_server_path(void)
{
    return AJI_CLIENT::get_server_path();
}

AJI_ERROR AJI_API aji_scan_device_chain       (AJI_CHAIN_ID         chain_id)
{
    return chain_id->scan_device_chain();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_define_device           (const AJI_DEVICE   * device)
{
    return AJI_CLIENT::define_device(device, true);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_undefine_device         (const AJI_DEVICE   * device)
{
    return AJI_CLIENT::define_device(device, false);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
// 
AJI_ERROR AJI_API aji_get_defined_devices     (DWORD              * device_count,
                                               AJI_DEVICE         * device_list)
{
    return AJI_CLIENT::get_defined_devices(device_count, device_list);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_read_device_chain       (AJI_CHAIN_ID         chain_id,
                                               DWORD              * device_count,
                                               AJI_DEVICE         * device_list,
                                               bool                 auto_scan)
{
    return chain_id->read_device_chain(device_count, device_list, auto_scan);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_set_parameter           (AJI_CHAIN_ID         chain_id,
                                               const char         * name,
                                               DWORD                value)
{
    return chain_id->set_parameter(name, value);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_get_parameter           (AJI_CHAIN_ID         chain_id,
                                               const char         * name,
                                               DWORD              * value)
{
    return chain_id->get_parameter(name, value);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_get_parameter           (AJI_CHAIN_ID         chain_id,
                                               const char         * name,
                                               BYTE               * value,
                                               DWORD              * valuemax)
//
// This version is deprecated (use the one below), but is kept in case any
// external (to Quartus) libraries have linked against older versions of the
// DLL.  We want to keep them working.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    return chain_id->get_parameter(name, value, valuemax, 0);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_get_parameter           (AJI_CHAIN_ID         chain_id,
                                               const char         * name,
                                               BYTE               * value,
                                               DWORD              * valuemax,
                                               DWORD                valuetx)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    return chain_id->get_parameter(name, value, valuemax, valuetx);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_open_device             (AJI_CHAIN_ID         chain_id,
                                               DWORD                tap_position, 
                                               AJI_OPEN_ID        * open_id,
                                               const AJI_CLAIM    * claims,
                                               DWORD                claim_n,
                                               const char         * application_name)
{
    AJI_CLAIM2 * claims2 = AJI_OPEN::create_claims(claims, claim_n);
    AJI_ERROR result = chain_id->open_device(tap_position, open_id, claims2, claim_n, application_name);
    delete[] claims2;

    return result;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_open_device             (AJI_CHAIN_ID         chain_id,
                                               DWORD                tap_position,
                                               AJI_OPEN_ID        * open_id,
                                               const AJI_CLAIM2   * claims,
                                               DWORD                claim_n,
                                               const char         * application_name)
{
    return chain_id->open_device(tap_position, open_id, claims, claim_n, application_name);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_open_entire_device_chain(AJI_CHAIN_ID         chain_id,
                                               AJI_OPEN_ID        * open_id,
                                               AJI_CHAIN_TYPE       style,
                                               const char         * application_name)
{
    return chain_id->open_entire_chain(open_id, style, application_name);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_close_device            (AJI_OPEN_ID          open_id)
{
    if (!AJI_OPEN::valid(open_id, true))
        return AJI_INVALID_OPEN_ID;

    return open_id->close_device();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_lock                    (AJI_OPEN_ID          open_id,
                                               DWORD                timeout,
                                               AJI_PACK_STYLE       pack_style)
{
    if (!AJI_OPEN::valid(open_id))
        return AJI_INVALID_OPEN_ID;

    return open_id->lock(timeout, pack_style);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_unlock                  (AJI_OPEN_ID          open_id)
{
    if (!AJI_OPEN::valid(open_id))
        return AJI_INVALID_OPEN_ID;

    return open_id->unlock();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_unlock_lock             (AJI_OPEN_ID          unlock_id,
                                               AJI_OPEN_ID          lock_id)
{
    if (!AJI_OPEN::valid(unlock_id) || !AJI_OPEN::valid(lock_id))
        return AJI_INVALID_OPEN_ID;

    return unlock_id->unlock_lock(lock_id);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_unlock_chain_lock       (AJI_CHAIN_ID         unlock_id,
                                               AJI_OPEN_ID          lock_id,
                                               AJI_PACK_STYLE       pack_style)
{
    if (!AJI_OPEN::valid(lock_id))
        return AJI_INVALID_OPEN_ID;

    return lock_id->unlock_chain_lock(unlock_id, pack_style);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_unlock_lock_chain       (AJI_OPEN_ID          unlock_id,
                                               AJI_CHAIN_ID         lock_id)
{
    if (!AJI_OPEN::valid(unlock_id))
        return AJI_INVALID_OPEN_ID;

    return unlock_id->unlock_lock_chain(lock_id);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_flush                   (AJI_OPEN_ID          open_id)
{
    if (!AJI_OPEN::valid(open_id))
        return AJI_INVALID_OPEN_ID;

    return open_id->flush(~0u);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_lock_chain              (AJI_CHAIN_ID         chain_id,
                                               DWORD                timeout)
{
    return chain_id->lock(timeout);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_unlock_chain            (AJI_CHAIN_ID         chain_id)
{
    return chain_id->unlock();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_API aji_cancel_operation             (AJI_CHAIN_ID         chain_id)
{
    chain_id->cancel_operation();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_access_ir               (AJI_OPEN_ID          open_id,
                                               DWORD                instruction,
                                               DWORD              * captured_ir,
                                               DWORD                flags)
{
    if (!AJI_OPEN::valid(open_id))
        return AJI_INVALID_OPEN_ID;

    return open_id->access_ir(instruction, captured_ir, flags);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

AJI_ERROR AJI_API aji_access_ir               (AJI_OPEN_ID          open_id,
                                               DWORD                length_ir,
                                               const BYTE         * write_bits,
                                               BYTE               * read_bits,
                                               DWORD                flags)
{
    if (!AJI_OPEN::valid(open_id))
        return AJI_INVALID_OPEN_ID;

    return open_id->access_ir(length_ir, write_bits, read_bits, flags);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//

AJI_ERROR AJI_API aji_access_dr               (AJI_OPEN_ID          open_id,
                                               DWORD                length_dr,
                                               DWORD                flags,
                                               DWORD                write_offset,
                                               DWORD                write_length,
                                               const BYTE         * write_bits,
                                               DWORD                read_offset,
                                               DWORD                read_length,
                                               BYTE               * read_bits)
{
    if (!AJI_OPEN::valid(open_id))
        return AJI_INVALID_OPEN_ID;

    return open_id->access_dr(length_dr, flags,
                              write_offset, write_length, write_bits,
                              read_offset, read_length, read_bits, 1);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

AJI_ERROR AJI_API aji_access_dr               (AJI_OPEN_ID          open_id,
                                               DWORD                length_dr,
                                               DWORD                flags,
                                               DWORD                write_offset,
                                               DWORD                write_length,
                                               const BYTE         * write_bits,
                                               DWORD                read_offset,
                                               DWORD                read_length,
                                               BYTE               * read_bits,
                                               DWORD                batch)
{
    if (!AJI_OPEN::valid(open_id))
        return AJI_INVALID_OPEN_ID;

    return open_id->access_dr(length_dr, flags,
                              write_offset, write_length, write_bits,
                              read_offset, read_length, read_bits, batch);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_run_test_idle           (AJI_OPEN_ID          open_id,
                                               DWORD                num_clocks)
{
    return aji_run_test_idle(open_id, num_clocks, 0);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_run_test_idle           (AJI_OPEN_ID          open_id,
                                               DWORD                num_clocks,
                                               DWORD                flags)
{
    if (!AJI_OPEN::valid(open_id))
        return AJI_INVALID_OPEN_ID;

    return open_id->run_test_idle(num_clocks, flags);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_delay                   (AJI_OPEN_ID          open_id,
                                               DWORD                microseconds)
{
    if (!AJI_OPEN::valid(open_id))
        return AJI_INVALID_OPEN_ID;

    return open_id->delay(microseconds);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_test_logic_reset        (AJI_OPEN_ID          open_id)
{
    if (!AJI_OPEN::valid(open_id))
        return AJI_INVALID_OPEN_ID;

    return open_id->test_logic_reset();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_access_overlay(AJI_OPEN_ID open_id, DWORD overlay, DWORD * captured)
//
// Description: 
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (!AJI_OPEN::valid(open_id))
        return AJI_INVALID_OPEN_ID;

    return open_id->access_overlay(overlay, captured);
}

//START_FUNCTION_HEADER//////////////////////////////////////////////////////
//
void AJI_API aji_register_output_callback     (void   (* output_fn)(void * handle, DWORD level, const char * line),
                                               void                * handle)
{
    AJI_CLIENT::register_output_callback(output_fn, handle);
}

//START_FUNCTION_HEADER//////////////////////////////////////////////////////
//
void AJI_API aji_register_progress_callback   (AJI_OPEN_ID          open_id,
                                               void  (* progress_fn)(void * handle, DWORD bits),
                                               void                * handle)
{
    if (!AJI_OPEN::valid(open_id))
        return;

    open_id->register_progress_callback(progress_fn, handle);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_CLAIM2 * AJI_OPEN::create_claims(const AJI_CLAIM * claims, DWORD claim_n)
//
// Description: Convert and create an array of AJI_CLAIM2 from AJI_CLAIM.
//              The caller is responsible to deallocate the memory.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_CLAIM2 * claims2 = new AJI_CLAIM2[claim_n];
    for (DWORD i = 0 ; i < claim_n; i++)
    {
        claims2[i].type = claims[i].type;
        // AJI_CLAIM is the legacy claim that only works for non-hierarchical
        // hub and top-level hub only. AJI_CLAIM2 with length 0 is an
        // equivalent of this legacy claim.
        claims2[i].length = 0;
        // 0x800 is the mask for weak claim type while a weak claim value of
        // ~0u allows access to all values in that resource
        if ( (claims[i].type & 0x800) == 0x800 && claims[i].value == ~0u )
            claims2[i].value = ~0ull;
        else
            claims2[i].value = claims[i].value;
    }
    return claims2;
}
