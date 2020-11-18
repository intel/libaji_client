/****************************************************************************
 *   Copyright (c) 2012 by Intel Corporation                                *
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
//# Filename:    jtag_client_aji.h
//#
//# Description: 
//#
//# Authors:     Andrew Draper
//#
//#              Copyright (c) Altera Corporation 2012
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

#ifndef INC_JTAG_CLIENT_AJI_H
#define INC_JTAG_CLIENT_AJI_H

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if PORT == WINDOWS
#ifndef INC_WINSOCK2_H
#include <winsock2.h>
#define INC_WINSOCK2_H
#endif

#ifndef INC_WINDOWS_H
#include <Windows.h>
#define INC_WINDOWS_H
#endif

#endif // PORT == WINDOWS

#ifndef INC_SET
#include <set>
#endif

class RXMESSAGE;

//START_CLASS_DEFINITION//////////////////////////////////////////////////////
//
class AJI_CHAIN
{
public:
    static const char * get_error_info(void);

    virtual AJI_ERROR print_hardware_name(char            * hw_name,
                                       DWORD                hw_name_len,
                                       bool                 explicit_localhost = false,
                                       DWORD              * needed_hw_name_len = NULL) = 0;

    virtual AJI_ERROR remove_hardware (void)
    {
        return AJI_UNIMPLEMENTED;
    }

    virtual AJI_ERROR cancel_operation(void)
    {
        return AJI_UNIMPLEMENTED;
    }

    virtual AJI_ERROR lock            (DWORD timeout) = 0;

    virtual AJI_ERROR unlock          (void) = 0;

    virtual AJI_ERROR scan_device_chain(void) = 0;

    virtual AJI_ERROR define_device   (DWORD                tap_position, 
                                       const AJI_DEVICE   * device)
    {
        return AJI_UNIMPLEMENTED;
    }

    virtual AJI_ERROR read_device_chain(DWORD             * device_count,
                                       AJI_DEVICE         * device_list,
                                       bool                 auto_scan) = 0;

    virtual DWORD get_device_features (DWORD tap_position) = 0;

    virtual AJI_ERROR set_parameter   (const char         * name,
                                       DWORD                value)
    {
        return AJI_UNIMPLEMENTED;
    }

    virtual AJI_ERROR get_parameter   (const char         * name,
                                       DWORD              * value)
    {
        return AJI_UNIMPLEMENTED;
    }

    virtual AJI_ERROR get_parameter   (const char         * name,
                                       BYTE               * value,
                                       DWORD              * valuemax,
                                       DWORD                valuetx)
    {
        return AJI_UNIMPLEMENTED;
    }

    virtual AJI_ERROR open_device     (DWORD                tap_position,
                                       AJI_OPEN_ID        * open_id,
                                       const AJI_CLAIM2   * claims,
                                       DWORD                claim_n,
                                       const char         * application_name) = 0;

    virtual AJI_ERROR open_entire_chain(AJI_OPEN_ID     * open_id,
                                       AJI_CHAIN_TYPE       style,
                                       const char         * application_name)
    {
        return AJI_UNIMPLEMENTED;
    }

    typedef DWORD WATCH_FN(void * handle, const BYTE * data, DWORD clocks);

protected:
    virtual ~AJI_CHAIN(void) {}
    static char * s_error_info;

};

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class AJI_OPEN
{
public:
    static inline bool valid(AJI_OPEN * open_id, bool for_close = false);

    static AJI_CLAIM2 * create_claims(const AJI_CLAIM * claims, DWORD claim_n);

    typedef AJI_ERROR ( * DEFERRED_RESPONSE_FN_PTR) (void * handle, AJI_ERROR error, RXMESSAGE * rx);

    virtual AJI_ERROR close_device    (void) = 0;

    virtual AJI_ERROR lock            (DWORD timeout, AJI_PACK_STYLE pack_style) = 0;

    virtual AJI_ERROR unlock          (void) = 0;

    virtual AJI_ERROR unlock_lock     (AJI_OPEN * lock_id) = 0;

    virtual AJI_ERROR unlock_chain_lock(AJI_CHAIN * unlock_id, AJI_PACK_STYLE pack_style) = 0;

    virtual AJI_ERROR unlock_lock_chain(AJI_CHAIN * lock_id) = 0;

    virtual AJI_ERROR access_ir       (DWORD                instruction,
                                       DWORD              * old_instruction,
                                       DWORD                flags) = 0;

    virtual AJI_ERROR access_ir       (DWORD                length_dr,
                                       const BYTE         * write_bits,
                                       BYTE               * read_bits,
                                       DWORD                flags)
    {
        return AJI_UNIMPLEMENTED;
    }

    virtual AJI_ERROR access_dr       (DWORD                length_dr,
                                       DWORD                flags,
                                       DWORD                write_offset,
                                       DWORD                write_length,
                                       const BYTE         * write_bits,
                                       DWORD                read_offset,
                                       DWORD                read_length,
                                       BYTE               * read_bits,
                                       DWORD                batch,
                                       DEFERRED_RESPONSE_FN_PTR completion_fn = NULL,
                                       void               * handle = NULL,
                                       DWORD              * timestamp = NULL) = 0;

    virtual AJI_ERROR run_test_idle   (DWORD                num_clocks,
                                       DWORD                flags) = 0;

    virtual AJI_ERROR test_logic_reset(void) = 0;

    virtual AJI_ERROR delay           (DWORD                microseconds) = 0;

    virtual AJI_ERROR flush           (DWORD                timeout) = 0;

    virtual AJI_ERROR open_node       (const AJI_HIER_ID  * node_info,
                                       QWORD                overlay_select,
                                       DWORD                overlay_total_length,
                                       DWORD                overlay_node_length,
                                       QWORD                force_capture,
                                       QWORD                prevent_program,
                                       AJI_OPEN_ID        * node_id,
                                       const AJI_CLAIM2   * claims,
                                       DWORD                claim_n,
                                       const char         * application_name) = 0;

      virtual AJI_ERROR access_overlay  (DWORD                update,
                                       DWORD              * captured) = 0;

    virtual void register_progress_callback(void ( * progress_fn)(void * handle, DWORD progress),
                                            void               * handle) = 0;

    enum SUBTYPE { SUBTYPE_JS };
    virtual SUBTYPE get_subtype() const = 0;

protected:
    virtual ~AJI_OPEN(void) {}

    typedef std::set< AJI_OPEN *, std::less< AJI_OPEN * > > OPENS;
    static OPENS m_valid_set;

    virtual bool is_valid(bool for_close) = 0;

    virtual AJI_ERROR set_dummy_bits  (DWORD                dummy_bits) = 0;
};

#if PORT == WINDOWS
extern HINSTANCE jtag_client_module;
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline bool AJI_OPEN::valid(AJI_OPEN * open_id, bool for_close)
//
// Description: Check that the device pointer passed in is valid.
//              If the pointer is to a valid device which has returned a fatal
//              error (and if we are not closing the device) then invalidate
//              the pointer and return false.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
#if PORT == WINDOWS
    // Nothing is valid after Windows has started to unload the DLL.
    if (jtag_client_module == NULL)
        return false;
#endif

    if (m_valid_set.find(open_id) == m_valid_set.end())
        return false;

    return open_id->is_valid(for_close);
}

#endif
