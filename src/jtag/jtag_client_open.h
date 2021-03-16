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

//# START_MODULE_HEADER/////////////////////////////////////////////////////
//#
//#
//# Description: 
//#
//# Authors:     Andrew Draper
//#
//#              Copyright (c) Altera Corporation 2000 - 2003
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

#ifndef INC_JTAG_CLIENT_OPEN_H
#define INC_JTAG_CLIENT_OPEN_H

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

class AJI_CHAIN;
enum  TAP_STATE;
class RXRAWFIFO;

#if PORT == WINDOWS
extern HINSTANCE jtag_client_module;
#endif

static const QWORD JTAG_PROGRAM = 0x2ll;
static const QWORD JTAG_USR0    = 0xCll;
static const QWORD JTAG_USR1    = 0xEll;

static const QWORD HUB_INFO             = 0x0ll;
static const QWORD HUB_FORCE_IR_CAPTURE = 0x3ll;

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class AJI_OPEN_JS : public AJI_OPEN
{
public:
    AJI_ERROR close_device            (void);

    AJI_ERROR cancel_operation        (void);

    AJI_ERROR lock                    (DWORD timeout, AJI_PACK_STYLE pack_style);

    AJI_ERROR unlock                  (void);

    AJI_ERROR unlock_lock             (AJI_OPEN * lock_id);

    AJI_ERROR unlock_chain_lock       (AJI_CHAIN * unlock_id, AJI_PACK_STYLE pack_style);

    AJI_ERROR unlock_lock_chain       (AJI_CHAIN * lock_id);

    AJI_ERROR access_ir               (DWORD                instruction,
                                       DWORD              * old_instruction,
                                       DWORD                flags);

    AJI_ERROR access_ir               (DWORD                length_dr,
                                       const BYTE         * write_bits,
                                       BYTE               * read_bits,
                                       DWORD                flags);

    AJI_ERROR access_dr               (DWORD                length_dr,
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
                                       DWORD              * timestamp = NULL);
    AJI_ERROR run_test_idle           (DWORD                num_clocks,
                                       DWORD                flags);

    AJI_ERROR test_logic_reset        (void);

    AJI_ERROR delay                   (DWORD                microseconds);

    AJI_ERROR flush                   (DWORD                timeout);

    AJI_CHAIN_ID get_chain_id         (void);

    AJI_ERROR open_node               (const AJI_HIER_ID  * node_info,
                                       QWORD                overlay_select,
                                       DWORD                overlay_total_length,
                                       DWORD                overlay_node_length,
                                       QWORD                force_capture,
                                       QWORD                prevent_program,
                                       AJI_OPEN_ID        * node_id,
                                       const AJI_CLAIM2   * claims,
                                       DWORD                claim_n,
                                       const char         * application_name);

    AJI_ERROR set_dummy_bits           (DWORD                dummy_bits);

    AJI_ERROR access_overlay          (DWORD                update,
                                       DWORD              * captured);

    void register_progress_callback    (void ( * progress_fn)(void * handle, DWORD progress),
                                       void               * handle);

    virtual SUBTYPE get_subtype() const { return SUBTYPE_JS; }

    virtual bool is_valid(bool for_close);

public: // but for use by AJI_CHAIN only

    enum DEVICE_STYLE { INDIVIDUAL = 1, WHOLE_CHAIN = 2, /* PASSIVE_SERIAL = 4, ACTIVE_SERIAL = 8, */ NODE = 16 };

    AJI_OPEN_JS(AJI_CHAIN_JS * chain, AJI_CLIENT * client, DEVICE_STYLE style, DWORD position,
                const AJI_DEVICE & dev,const AJI_CLAIM2 * claims, DWORD claim_n);
    ~AJI_OPEN_JS(void);
    void set_id(unsigned int id);
    void send_deferred(void) { m_defer_error = send_receive(NULL); }

    void notify_data_used(DWORD data_used);
    void notify_watch_triggered(void) { m_watch_triggered = true; }

    static AJI_OPEN_JS * find_open(AJI_CLIENT * client, DWORD open_id);
    static void invalidate(AJI_CLIENT * client);

public:
    struct DEFERRED
    {
        DEFERRED_RESPONSE_FN_PTR m_fn;
        void                 * m_handle;
        DWORD                  m_checkpoint;
    };

    struct ACTIONS
    {
        enum { DEFERRED_MAX = 64, OUTPUT_MAX = 32 };

        struct ACTIONS * m_next;

        DEFERRED     m_deferred[DEFERRED_MAX];
        DWORD        m_defer_n;
        DWORD        m_defer_write;

        RXRAWFIFO  * m_output[OUTPUT_MAX];
        DWORD        m_output_n;
    };

    struct SORT_INFO
    {
        DWORD index;
        DWORD position;
    };

    struct OVERLAY_CAPTURE
    {
        BYTE         m_data[4];
        DWORD        m_length;
        DWORD      * m_capture_ptr;
    };

private:

    static AJI_ERROR sort_devices(SORT_INFO * * info_ptr, DWORD num_devices, AJI_OPEN_JS * const * open_id);

    void      locked_ok(void);
    AJI_ERROR add_unlock(void);
    bool      instruction_ok(DWORD instruction);

    static AJI_ERROR deferred_response_lock(void * handle, AJI_ERROR error, RXMESSAGE * rx);
    static AJI_ERROR deferred_response_access_ir(void * handle, AJI_ERROR error, RXMESSAGE * rx);
    static AJI_ERROR deferred_response_access_dr(void * handle, AJI_ERROR error, RXMESSAGE * rx);
    static AJI_ERROR deferred_response_access_dr_legacy(void * handle, AJI_ERROR error, RXMESSAGE * rx);
    static AJI_ERROR deferred_response_overlay(void * handle, AJI_ERROR error, RXMESSAGE * rx);

    AJI_ERROR any_sequence_internal(DWORD num_tcks, const BYTE * tdi_bits, const BYTE * tms_bits, BYTE * tdo_bits, bool allow);

    TXMESSAGE * get_txmessage(bool & defer, AJI_ERROR & error, DWORD write, DWORD output_n);
    bool      start_command(TXMESSAGE * & tx, TXMESSAGE::COMMAND cmnd, DWORD len);
    void      assume_success(DEFERRED_RESPONSE_FN_PTR fn, void * handle, DWORD write);
    void      expect_output(RXRAWFIFO * output);
    AJI_ERROR send_receive(RXMESSAGE * * rx_ptr, DWORD timeout = ~0u);
    bool      send_continue(void);

    bool      send_command(void);
    AJI_ERROR receive_response(RXMESSAGE * * rx_ptr, DWORD timeout = ~0u);

    AJI_ERROR pending_error(void);

    bool      claimed_exact(AJI_CLAIM_TYPE type, QWORD value);
    enum CLAIM_STYLE { NOCLAIM, SAFECLAIM, IFAVAIL };
    CLAIM_STYLE claimed_class(AJI_CLAIM_TYPE type, QWORD value, DWORD length = 0);

    AJI_CHAIN_JS * m_chain;
    AJI_CLIENT * m_client;
    DWORD        m_position; // Within chain (0 = near TDO)

    DEVICE_STYLE m_style; // What sort of device is this?
    AJI_DEVICE   m_device;

    AJI_CLAIM2 * m_claims;
    DWORD        m_claim_n;

    // Typically JTAG_USR1
    DWORD        m_overlay_ir;
    // Typically JTAG_USR0
    DWORD        m_overlaid_ir;

    unsigned int m_open_id;

    // The m_locked variable indicates whether we hope we are locked.  For pack
    // modes other than PACK_NEVER this doesn't always agree with whether we are
    // actually locked - when we queue a LOCK command we assume that it will succeed
    // which is sometimes a false assumption.  The m_lock_fail variable indicates
    // when this assumption is no longer true.
    bool         m_locked;
    bool         m_lock_fail;

    bool         m_ir_multiple;
    bool         m_ir_overlay;
    TAP_STATE    m_state;
    QWORD        m_last_overlay;

    AJI_PACK_STYLE m_pack_style;
    bool         m_pack_stream;

    bool         check_transitions(const BYTE * tms_bits, DWORD bitlength, TAP_STATE & state);
    static void  update_state(const BYTE * tms_bits, DWORD bitlength, TAP_STATE & state);

    ACTIONS      m_actions[4];
    ACTIONS    * m_free_actions;
    ACTIONS    * m_fill_actions;
    ACTIONS    * m_next_actions;
    ACTIONS    * m_last_actions;

    AJI_ERROR    m_defer_error;

    DWORD        m_checkpoint;
    DWORD        m_next_checkpoint;
    DWORD        m_next_checkpoint_prev;

    bool         m_watch_triggered;

    void      ( * m_progress_fn)(void * handle, DWORD bits);
    void       * m_progress_handle;

    // For use by nodes
    AJI_HIER_ID  m_node_info;
    QWORD        m_node_overlay_select;
    DWORD        m_node_overlay_total_length;
    DWORD        m_node_overlay_node_length;
    QWORD        m_node_force_capture;
    DWORD        m_dummy_bits;
    DWORD        m_current_instruction;

    friend class AJI_OPEN;
};

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline bool AJI_OPEN_JS::is_valid(bool for_close)
//
// Description: Check that the device pointer (passed in as 'this') is valid.
//              If the pointer is to a valid device which has returned a fatal
//              error (and if we are not closing the device) then invalidate
//              the pointer and return false.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (m_client == NULL && !for_close)
    {
        delete this;
        return false;
    }

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline TXMESSAGE * AJI_OPEN_JS::get_txmessage(bool & defer, AJI_ERROR & error, DWORD write, DWORD output_n)
//
// Description: Work out whether other commands can be packed after this one.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (m_fill_actions == NULL)
    {
        if (m_free_actions == NULL)
        {
            // If we have no more action buffers then we need to wait for a response so
            // that we can fill one up.
            AJI_ERROR err = receive_response(NULL);
            if (err != AJI_NO_ERROR)
            {
                error = err;
                return NULL;
            }
        }

        m_fill_actions = m_free_actions;
        m_free_actions = m_fill_actions->m_next;
        m_fill_actions->m_next = NULL;

        m_fill_actions->m_defer_n = 0;
        m_fill_actions->m_defer_write = 0;
        m_fill_actions->m_output_n = 0;
    }

    // Don't pack if the AJI user isn't expecting it.
    if ((m_pack_style == AJI_PACK_NEVER) ||
        (m_pack_style == AJI_PACK_AUTO && output_n > 0))
        defer = false;

    // This limit is imposed for performance reasons to prevent large memcpys
    // while sending commands.
    else if (write >= 1024)
        defer = false;

#if 0
    // This limit is imposed for performance reasons to prevent us from copying
    // too much write data when packing the command.
    else if (m_fill_actions->m_defer_write + write >= 1024) // MAGIC
        defer = false;

    // TODO: impose a read performance limit.
#endif

    // Don't overflow any fixed size buffers.
    else if (m_fill_actions->m_defer_n >= ACTIONS::DEFERRED_MAX ||
             m_fill_actions->m_output_n + output_n > ACTIONS::OUTPUT_MAX)
    {
        defer = send_continue();
    }

    else
        defer = true;

    return m_client->get_txmessage(this);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline bool AJI_OPEN_JS::start_command(TXMESSAGE * & tx, TXMESSAGE::COMMAND cmnd, DWORD len)
{
    if (tx->add_command(cmnd, len))
        return true;
        
    // No space to add the command.  Send what we have and start the next packet
    if (!send_continue())
        return false;

    tx = m_client->get_txmessage(this);

    return tx->add_command(cmnd, len);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline void AJI_OPEN_JS::assume_success(DEFERRED_RESPONSE_FN_PTR fn, void * handle, DWORD write)
{
    DWORD n = m_fill_actions->m_defer_n++;

    m_fill_actions->m_deferred[n].m_fn     = fn;
    m_fill_actions->m_deferred[n].m_handle = handle;
    m_fill_actions->m_deferred[n].m_checkpoint = m_next_checkpoint;
    m_fill_actions->m_defer_write += write;

    m_next_checkpoint_prev = m_next_checkpoint;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline void AJI_OPEN_JS::expect_output(RXRAWFIFO * output)
{
    AJI_DEBUG_ASSERT(m_fill_actions->m_output_n < ACTIONS::OUTPUT_MAX);
    m_fill_actions->m_output[m_fill_actions->m_output_n++] = output;
}

#endif
