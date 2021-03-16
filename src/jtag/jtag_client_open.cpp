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
//              Copyright (c) Altera Corporation 2000 - 2003
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

#ifndef INC_JTAG_COMMON_H
#include "jtag_common.h"
#endif

#ifndef INC_JTAG_PROTOCOL_H
#include "jtag_protocol.h"
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

#ifndef INC_JTAG_RAW_FIFO_H
#include "jtag_raw_fifo.h"
#endif

// Solaris requires these funcs to be declared extern "C" since they are
// callbacks passed to qsort and bsearch
extern "C"
{
int aji_compare_claims(const void * elem1, const void * elem2);
int aji_compare_positions(const void * elem1, const void * elem2);
}



#define VALID_STATES ((1 << TEST_LOGIC_RESET) | (1 << RUN_TEST_IDLE) | (1 << SELECT_DR_SCAN) | (1 << UPDATE_DR) | (1 << UPDATE_IR))

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_OPEN_JS::AJI_OPEN_JS(AJI_CHAIN_JS * chain, AJI_CLIENT * client, DEVICE_STYLE style, DWORD position, 
                   const AJI_DEVICE & device, const AJI_CLAIM2 * claims, DWORD claim_n)
//
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
    : m_chain(chain), m_client(client), m_position(position), m_style(style), m_device(device), 
      m_claims(NULL), m_claim_n(0), m_overlay_ir(~0u), m_overlaid_ir(~0u), m_open_id(0),
      m_locked(false), m_lock_fail(false), m_ir_multiple(false),
      m_ir_overlay(false), m_state(TEST_LOGIC_RESET), m_last_overlay(~0ull), 
      m_pack_style(AJI_PACK_NEVER), m_pack_stream(false),
      m_fill_actions(NULL), m_next_actions(NULL), m_last_actions(NULL),
      m_defer_error(AJI_NO_ERROR),
      m_checkpoint(0), m_next_checkpoint(0), m_next_checkpoint_prev(0),
      m_watch_triggered(false),
      m_progress_fn(NULL),
      m_progress_handle(NULL),
      m_node_overlay_select(0),
      m_node_overlay_total_length(0),
      m_node_overlay_node_length(0),
      m_node_force_capture(0),
      m_dummy_bits(0),
      m_current_instruction(0)
{
    DWORD i;

    m_free_actions = &m_actions[0];
    for (i = 0 ; i < sizeof(m_actions)/sizeof(m_actions[0]) - 1 ; i++)
        m_actions[i].m_next = &m_actions[i+1];
    m_actions[i].m_next = NULL;

    if (claim_n > 0)
    {
        m_claims = new AJI_CLAIM2[claim_n];

        if (m_claims != NULL)
        {
            memcpy_s(m_claims, claim_n * sizeof(AJI_CLAIM2), claims, claim_n * sizeof(AJI_CLAIM2));
            m_claim_n = claim_n;

            // Sort claims into order by type, class, then by style
            qsort(m_claims, claim_n, sizeof(AJI_CLAIM2), &aji_compare_claims);

            // TODO: check for duplicate claims
        }

        // Find the overlay and overlaid chain values
        for (i = 0 ; i < claim_n ; i++)
        {
            if (m_claims[i].type == AJI_CLAIM_IR_SHARED_OVERLAY)
                m_overlay_ir = static_cast<DWORD>(m_claims[i].value);

            else if (m_claims[i].type == AJI_CLAIM_IR_OVERLAID ||
                    m_claims[i].type == AJI_CLAIM_IR_SHARED_OVERLAID)
                m_overlaid_ir = static_cast<DWORD>(m_claims[i].value);
        }
    }

    // Store the this pointer so that we can validity check pointers from the
    // user later.
    m_valid_set.insert(this);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
extern "C" int aji_compare_claims(const void * elem1, const void * elem2)
//
// Description: Compares the elements and returns which is larger.
//              Solaris requires it to be declared extern "C" since its a
//              callback passed to qsort and bsearch
//
// Returns:     1 if elem1 should go after elem2, 0 if same, -1 otherwise
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    const AJI_CLAIM2 * claim1 = static_cast<const AJI_CLAIM2 *>(elem1);
    const AJI_CLAIM2 * claim2 = static_cast<const AJI_CLAIM2 *>(elem2);

    // hlloh: The following comment doesn't seem to be correct:
    // Sort by type of claim, then by value, so that claims are in order and we
    // can check for duplicates.

    // TODO: Compare against claim length if we are to check for duplicates
    // hlloh: Here's my understanding of the code:

    // This differentiate the weak claim from the rest
    if ((claim1->type ^ claim2->type) & 0xFF)
        // Weak claim comes after normal claim
        return (claim1->type & 0xFF) - (claim2->type & 0xFF);
    // Larger value comes after smaller value
    else if (claim1->value > claim2->value)
        return 1;
    else if (claim1->value < claim2->value)
        return -1;
    // If values are the same, claim type with larger type comes after smaller type
    else
        return claim1->type - claim2->type;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_OPEN_JS::~AJI_OPEN_JS(void)
{
    // Remove ourselves from the validity checking collection.
    m_valid_set.erase(this);

    delete[] m_claims;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

void AJI_OPEN_JS::set_id(unsigned int id)
{
    m_open_id = id;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_OPEN_JS * AJI_OPEN_JS::find_open(AJI_CLIENT * client, DWORD open_id)
//
// Description: The link to the server specified has been disconnected.
//              Invalidate all devices pointing to it.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    OPENS::iterator i;

    for (i = m_valid_set.begin() ; i != m_valid_set.end() ; i++)
    {
        AJI_OPEN * open = *i;
        if (open->get_subtype() == SUBTYPE_JS)
        {
            AJI_OPEN_JS * open_js = static_cast<AJI_OPEN_JS *>(open);
            if (open_js->m_client == client && open_js->m_open_id == open_id)
                return open_js;
        }
    }
    return NULL;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_OPEN_JS::invalidate(AJI_CLIENT * client)
//
// Description: The link to the server specified has been disconnected.
//              Invalidate all devices pointing to it.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    OPENS::iterator i;

    for (i = m_valid_set.begin() ; i != m_valid_set.end() ; i++)
    {
        AJI_OPEN * open = *i;
        if (open->get_subtype() == SUBTYPE_JS)
        {
            AJI_OPEN_JS * open_js = static_cast<AJI_OPEN_JS *>(open);
            if (open_js->m_client == client)
            {
                open_js->m_client = NULL;
                open_js->m_chain  = NULL;
            }
        }
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline void AJI_OPEN_JS::locked_ok(void)
//
// Description: Note that this open-id has been locked sucessfully.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    m_locked = true;
    m_lock_fail = false;

    // For the purpose of any_sequence we assume that we start in the UPDATE_DR/IR
    // state immediately after being locked.
    m_state = UPDATE_IR;
    m_watch_triggered = false;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::deferred_response_lock(void * handle, AJI_ERROR error, RXMESSAGE * rx)
//
// Description: Process the response from the server when responding to a lock.
//              This is important in the case where we assumed that we were
//              locked but another client locked the server before our lock
//              was processed.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_OPEN_JS * open = reinterpret_cast<AJI_OPEN_JS *>(handle);

    if (error != AJI_NO_ERROR)
        open->m_lock_fail = true;

    if (error == AJI_CHAIN_IN_USE)
        open->m_chain->read_blocker_info(rx);

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::lock(DWORD timeout, AJI_PACK_STYLE pack_style)
//
//
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (m_locked)
        return AJI_LOCKED;

    if (m_defer_error != AJI_NO_ERROR)
        return pending_error();

    if (m_client == NULL)
         return AJI_SERVER_ACTIVE;

    // Lock the TCP client to this thread until we've finished with it.
    if (!m_client->try_claim_link(timeout))
        return AJI_SERVER_ACTIVE;

    // Work around a bug in older JTAG servers (which sometimes start to ignore
    // this client if timeout == 0)
    if (timeout == 0 && !m_client->version_ok(4, 0xFFFF))
        timeout = 1;

    if (pack_style < AJI_PACK_NEVER || pack_style > AJI_PACK_STREAM)
        pack_style = AJI_PACK_NEVER;

    m_pack_stream = (pack_style == AJI_PACK_STREAM);
    m_pack_style = m_pack_stream ? AJI_PACK_MANUAL : pack_style;

    bool defer(false);
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, 0, 0);
    if (tx == NULL)
    {
        if (m_client != NULL)
            m_client->release_link();
        return error;
    }

    start_command(tx, MESSAGE::LOCK_DEVICE, 12);
    tx->add_int(m_open_id);
    tx->add_int(timeout);

    m_chain->clear_blocker_info();

    // Defer execution if we're allowed to
    if (defer)
    {
        assume_success(deferred_response_lock, this, 0);
        error = AJI_NO_ERROR;
    }
    else
    {
        RXMESSAGE * rx;
        error = send_receive(&rx);

        if (error == AJI_CHAIN_IN_USE)
            m_chain->read_blocker_info(rx);
    }

    if (error == AJI_NO_ERROR)
        locked_ok();
    else
        m_client->release_link();

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::unlock(void)
//
// Description: Unlock the chain this device was on.  On return from this
//              function the device will have been closed even if an error was
//              signalled.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (!m_locked)
        return AJI_NOT_LOCKED;

    AJI_ERROR error;

    if (m_defer_error != AJI_NO_ERROR)
    {
        error = pending_error();
    }
    else
    {
        // Unlock is never deferred to ensure that it does a flush first.

        error = add_unlock();

        RXMESSAGE * rx;
        if (error == AJI_NO_ERROR)
            error = send_receive(&rx);
    }

    // send_receive() sometimes releases the link for us.  Make sure we don't do
    // it twice.
    if (m_locked)
    {
        m_locked = false;
        if (m_client != NULL)
            m_client->release_link();
    }

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

AJI_ERROR AJI_OPEN_JS::add_unlock(void)
{
    AJI_DEBUG_ASSERT(m_locked);

    bool defer(false);
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, 0, 0);
    if (tx == NULL)
        return error;

    start_command(tx, MESSAGE::UNLOCK_DEVICE, 8);
    tx->add_int(m_open_id);

    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::unlock_lock(AJI_OPEN * lock_id_u)
//
// Description: Atomically a) remove the lock owned by this device and b) lock
//              the chain on behalf of the device specified.  Since this is
//              atomic the lock operation cannot fail so execution of this
//              command can be deferred.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_OPEN_JS * lock_id = reinterpret_cast<AJI_OPEN_JS *>(lock_id_u);

    if (m_chain != lock_id->m_chain)
        return AJI_INVALID_PARAMETER;
    AJI_DEBUG_ASSERT(m_client == lock_id->m_client);

    if (!m_locked)
        return AJI_NOT_LOCKED;
    AJI_DEBUG_ASSERT(!lock_id->m_locked);

    if (!m_client->version_ok(2, 0xFFFF))
        return AJI_UNIMPLEMENTED;

    if (m_defer_error != AJI_NO_ERROR)
        return pending_error();

    bool defer(false);
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, 0, 0);
    if (tx == NULL)
        return error;

    start_command(tx, MESSAGE::UNLOCK_LOCK_DEVICE, 12);
    tx->add_int(m_open_id);
    tx->add_int(lock_id->m_open_id);

    RXMESSAGE * rx;
    error = send_receive(&rx);

    if (m_locked)
    {
        m_locked = false;

        if (error == AJI_NO_ERROR)
        {
            lock_id->m_pack_style = m_pack_style;
            lock_id->locked_ok();
        }
        else if (m_client != NULL)
            m_client->release_link();
    }

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::unlock_chain_lock(AJI_CHAIN * unlock_chain_u, AJI_PACK_STYLE pack_style)
//
// Description: Atomically a) remove the lock owned by this device and b) lock
//              the chain on behalf of the device specified.  Since this is
//              atomic the lock operation cannot fail so execution of this
//              command can be deferred.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_CHAIN_JS * unlock_chain = reinterpret_cast<AJI_CHAIN_JS *>(unlock_chain_u);

    if (!unlock_chain->valid())
        return AJI_INVALID_OPEN_ID;

    if (unlock_chain != m_chain)
        return AJI_INVALID_PARAMETER;
    AJI_DEBUG_ASSERT(unlock_chain->get_client() == m_client);

    if (!unlock_chain->get_locked())
        return AJI_NOT_LOCKED;
    AJI_DEBUG_ASSERT(!m_locked);

    if (!m_client->version_ok(7, 0xFFFF))
    {
        // Do it non-atomically and hope...
        AJI_ERROR error = unlock_chain->unlock();
        if (error == AJI_NO_ERROR)
            error = lock(10000, pack_style);
        return error;
    }

    bool defer(false);
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, 0, 0);
    if (tx == NULL)
        return error;

    start_command(tx, MESSAGE::UNLOCK_LOCK_DEVICE, 12);
    tx->add_int(unlock_chain->get_chain_id());
    tx->add_int(m_open_id);

    // TODO: should defer this if possible as it will usually succeed so can be
    // packed.

    RXMESSAGE * rx;
    error = send_receive(&rx);

    unlock_chain->set_locked(false);

    if (error == AJI_NO_ERROR)
    {
        m_pack_style = pack_style;
        locked_ok();
    }
    else if (m_client != NULL)
        m_client->release_link();

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::unlock_lock_chain(AJI_CHAIN * lock_chain_u)
//
// Description: Atomically a) remove the lock owned by this device and b) lock
//              the chain on behalf of the device specified.  Since this is
//              atomic the lock operation cannot fail so execution of this
//              command can be deferred.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_CHAIN_JS * lock_chain = reinterpret_cast<AJI_CHAIN_JS *>(lock_chain_u);

    if (!lock_chain->valid())
        return AJI_INVALID_OPEN_ID;

    if (m_chain != lock_chain)
        return AJI_INVALID_PARAMETER;
    AJI_DEBUG_ASSERT(m_client == lock_chain->get_client());

    if (!m_locked)
        return AJI_NOT_LOCKED;
    AJI_DEBUG_ASSERT(!lock_chain->get_locked());

    if (!m_client->version_ok(7, 0xFFFF))
    {
        // Do it non-atomically and hope...
        AJI_ERROR error = unlock();
        if (error == AJI_NO_ERROR)
            error = lock_chain->lock(10000);
        return error;
    }

    if (m_defer_error != AJI_NO_ERROR)
        return pending_error();

    bool defer(false);
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, 0, 0);
    if (tx == NULL && m_client != NULL)
    {
        m_client->release_link();
        return error;
    }

    start_command(tx, MESSAGE::UNLOCK_LOCK_DEVICE, 12);
    tx->add_int(m_open_id);
    tx->add_int(lock_chain->get_chain_id());

    RXMESSAGE * rx;
    error = send_receive(&rx);

    m_locked = false;

    if (error == AJI_NO_ERROR)
        lock_chain->set_locked(true);
    else
        m_client->release_link();

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::close_device(void)
//
// Description: Close the device and free up any resources used by it.  On
//              return from this function the device will have been closed
//              even if an error was signalled.  This is the only function
//              which can be called on a device which has returned a fatal
//              error.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_ERROR error = AJI_NO_ERROR;

    if (m_defer_error != AJI_NO_ERROR)
        error = pending_error();

    if (error == AJI_NO_ERROR && m_locked)
        error = AJI_LOCKED;

    if (m_client != NULL)
    {
        // We might have a lock on the link at this point.  If so then we must
        // not attempt to re-acquire it.
        bool have_link = m_client->link_is_claimed();

        if (have_link || m_client->try_claim_link(20000))
        {
            AJI_ERROR new_error = AJI_NO_ERROR;

            bool defer(false);
            TXMESSAGE * tx = get_txmessage(defer, new_error, 0, 0);

            if (tx != NULL)
            {
                // If there are commands packed before this one then flush them
                if (tx->get_length() > 0)
                {
                    new_error = send_receive(NULL, ~0u);
                    tx = get_txmessage(defer, new_error, 0, 0);
                }

                if (tx != NULL)
                {
                    start_command(tx, MESSAGE::CLOSE_DEVICE, 8);
                    tx->add_int(m_open_id);

                    RXMESSAGE * rx;
                    new_error = send_receive(&rx);
                }
            }

            if (error == AJI_NO_ERROR)
                error = new_error;

            if (!have_link)
                m_client->release_link();
        }
        else
            error = AJI_SERVER_ACTIVE;
    }

    delete this;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

AJI_ERROR AJI_OPEN_JS::deferred_response_access_ir(void * handle, AJI_ERROR error, RXMESSAGE * rx)
{
    DWORD * captured_ir = static_cast<DWORD *>(handle);

    if (error == AJI_NO_ERROR)
    {
        DWORD captured(0);
        if (!rx->remove_int(&captured))
            error = AJI_SERVER_ERROR;

        if (captured_ir != NULL)
            *captured_ir = captured;
    }

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::access_ir(DWORD instruction, DWORD * captured_ir, DWORD flags)
{
    if (!m_locked)
        return AJI_NOT_LOCKED;

    // Can only call this for individual devices and nodes within them
    if ((m_style & (INDIVIDUAL | NODE)) == 0)
        return AJI_INVALID_PARAMETER;

    if (((1 << m_state) & VALID_STATES) == 0)
        return AJI_BAD_TAP_STATE;

    if (!instruction_ok(instruction))
        return AJI_INVALID_PARAMETER;

    // Client must have claimed this instruction, or the right to send any unclaimed
    // instruction
    CLAIM_STYLE style = claimed_class(AJI_CLAIM_IR, instruction);
    if (style == NOCLAIM)
        // TODO: AJI_INSTRUCTION_NOT_CLAIMED perhaps?
        return AJI_INSTRUCTION_CLAIMED;
    bool claimed = (style == SAFECLAIM);

    // Remove obsolete flag
    flags &= ~AJI_IR_COULD_BREAK;

    // Fail if unknown flag specified
    if (flags & ~0)
        return AJI_INVALID_PARAMETER;

    if (m_defer_error != AJI_NO_ERROR)
        return pending_error();

    if (captured_ir != NULL)
        flags |= 1;

    bool defer(false);
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, 0, 0);
    if (tx == NULL)
        return error;

    // We must not defer in AUTO mode.  get_txmessage doesn't know about non-fifo
    // return values so is over-optimistic about that.
    if (m_pack_style == AJI_PACK_AUTO)
        defer = false;

    start_command(tx, MESSAGE::ACCESS_IR, 16);
    tx->add_int(m_open_id);
    tx->add_int(instruction);
    tx->add_int(flags);

    // We can defer execution only if we have claimed the instruction (either
    // exclusively or shared).  If we have not claimed the instruction then
    // the server might return AJI_INSTRUCTION_CLAIMED so we must not defer.
    defer &= claimed;

    if (defer)
    {
        assume_success(deferred_response_access_ir, captured_ir, 0);

        error = AJI_NO_ERROR;
    }
    else
    {
        RXMESSAGE * rx;
        error = send_receive(&rx);

        if (error == AJI_NO_ERROR)
            error = deferred_response_access_ir(captured_ir, AJI_NO_ERROR, rx);
    }

    if (error == AJI_NO_ERROR)
    {
        m_state = UPDATE_IR;
        m_ir_multiple = false;
        m_ir_overlay = (instruction == m_overlay_ir);
        m_current_instruction = instruction;
    }

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::access_ir(DWORD length_ir, const BYTE * write_bits,
                              BYTE * read_bits, DWORD flags)
{
    if (!m_locked)
        return AJI_NOT_LOCKED;

    if (write_bits == NULL)
        return AJI_INVALID_PARAMETER;

    if (m_style & WHOLE_CHAIN)
    {
        // For whole chain devices the length must be sensible
        if (length_ir < 2)
            return AJI_INVALID_PARAMETER;
    }
    else if (m_style & (INDIVIDUAL | NODE))
    {
        // For single devices the length must be correct
        if (length_ir != m_device.instruction_length)
            return AJI_INVALID_PARAMETER;

        DWORD instruction = 0;
        DWORD capture = 0;
        DWORD i;

        for (i = 0 ; i < (length_ir+7)/8 ; i++)
            instruction |= write_bits[i] << (i * 8);

        AJI_ERROR error = access_ir(instruction, (read_bits != NULL) ? &capture : NULL, flags);

        if (error == AJI_NO_ERROR && read_bits != NULL)
        {
            error = flush(~0u);
            for (i = 0 ; i < (length_ir+7)/8 ; i++)
                read_bits[i] = static_cast<BYTE>(capture >> (i * 8));
        }

        return error;
    }
    else
        return AJI_INVALID_PARAMETER;

    if (!m_client->version_ok(1, 0xFFFF))
        return AJI_UNIMPLEMENTED;

    if (((1 << m_state) & VALID_STATES) == 0)
        return AJI_BAD_TAP_STATE;

    // Remove obsolete flag
    flags &= ~AJI_IR_COULD_BREAK;

    // Fail if unknown flag specified
    if (flags & ~0)
        return AJI_INVALID_PARAMETER;

    if (m_defer_error != AJI_NO_ERROR)
        return pending_error();

    DWORD bytes = (length_ir + 7) / 8;

    TXRAWFIFO * txbits = new TXRAWFIFO(m_client, bytes, write_bits);
    if (txbits == NULL)
        return AJI_NO_MEMORY;

    RXRAWFIFO * rxbits = NULL;
    if (read_bits != NULL)
    {
        rxbits = new RXRAWFIFO(m_client, bytes, read_bits);
        if (rxbits == NULL)
        {
            delete txbits;
            return AJI_NO_MEMORY;
        }
    }

    if (read_bits != NULL)
        flags |= 1;

    // Are we going to try and defer execution of this call?
    bool defer(false);
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, bytes, (read_bits != NULL) ? 1 : 0);
    if (tx == NULL)
    {
        delete txbits;
        delete rxbits;
        return error;
    }

    // If we are deferring execution then need to ensure that we have
    // copied the input data since the input buffer will be invalid once
    // we have returned.
    if (defer)
        defer = txbits->copy_data();

    start_command(tx, MESSAGE::ACCESS_IR_FIFO, 16);
    tx->add_int(m_open_id);
    tx->add_int(flags);
    tx->add_int(length_ir);

    txbits->activate(0, false);

    if (read_bits != NULL)
        rxbits->activate(0);

    // Calling activate() on a class derived from TXFIFO passes the ownership
    // to the TCP client socket so we don't need to free it here.

    // Try to defer execution until later
    if (defer)
    {
        assume_success(NULL, NULL, bytes);

        // rxbits has been stored and will be deleted once the command has been
        // executed.
        if (rxbits != NULL)
            expect_output(rxbits);

        m_state = UPDATE_IR;
        m_ir_multiple = false;
        return AJI_NO_ERROR;
    }

    RXMESSAGE * rx;
    error = send_receive(&rx);

    if (error == AJI_NO_ERROR && rxbits != NULL && !rxbits->wait_for_data())
    {
        m_client->disconnect();
        error = AJI_SERVER_ERROR;
    }

    delete rxbits;

    m_state = UPDATE_IR;
    m_ir_multiple = false;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

bool AJI_OPEN_JS::instruction_ok(DWORD instruction)
{
    return (instruction < (1u << m_device.instruction_length));
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::deferred_response_access_dr(void * handle, AJI_ERROR error, RXMESSAGE * rx)
//
// Description: Process the response from the server when responding to an
//              access_dr which requested a timestamp
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    DWORD * timestamp = reinterpret_cast<DWORD *>(handle);

    if (!rx->remove_int(timestamp))
        *timestamp = 0;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::deferred_response_access_dr_legacy(void * handle, AJI_ERROR error, RXMESSAGE * rx)
//
// Description: If access_dr requested a timestamp but the server is too old
//              to provide one then use the client time
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    DWORD * timestamp = reinterpret_cast<DWORD *>(handle);

    *timestamp = get_time();

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
// Description: Updates the dummy bits value
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
AJI_ERROR AJI_OPEN_JS::set_dummy_bits(DWORD dummy_bits)
{
    if((dummy_bits > MAX_DUMMY_BITS) || (dummy_bits < 0))
        return AJI_INVALID_DUMMY_BITS;
    m_dummy_bits = dummy_bits;
    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::access_dr(DWORD length_dr, DWORD flags,
                              DWORD write_offset, DWORD write_length, const BYTE * write_bits,
                              DWORD read_offset, DWORD read_length, BYTE * read_bits,
                              DWORD batch,
                              DEFERRED_RESPONSE_FN_PTR completion_fn, void * handle,
                              DWORD * timestamp)
{
    if (!m_locked)
        return AJI_NOT_LOCKED;

    // Can call this for whole chain or individual devices
    if ((m_style & (INDIVIDUAL | NODE | WHOLE_CHAIN)) == 0)
        return AJI_INVALID_PARAMETER;

    if (((1 << m_state) & VALID_STATES) == 0)
        return AJI_BAD_TAP_STATE;

    if (m_ir_multiple)
        return AJI_IR_MULTIPLE;

    if ((write_length > 0 && write_bits == NULL) ||
        (read_length > 0  && read_bits  == NULL) ||
        (write_offset + write_length > length_dr) ||
        (read_offset  + read_length  > length_dr) ||
        batch == 0 ||
        completion_fn != NULL && timestamp != NULL)
        return AJI_INVALID_PARAMETER;

    QWORD overlay = 0;
    bool claimed;
    if (m_ir_overlay)
    {
        if (write_offset != 0 || write_length != length_dr || batch > 1)
            return AJI_INVALID_PARAMETER;

        if (write_length <= 64)
        {
            DWORD i;
            for (i = 0 ; i < write_length ; i += 8) {
                overlay |= ((QWORD) write_bits[i/8]) << i;
            }
        }
        else
        {
            // We need to know the overlay register length to tell what instruction
            // has been loaded.  We assume the overlay register is less than 64
            // bits and reject attempts to load non-zero values into it.
            DWORD i;
            for (i = write_length - 64 ; i < write_length ; i += 8)
                if (write_bits[i/8] != 0)
                    overlay = ~0ull;
        }

        CLAIM_STYLE style = claimed_class(AJI_CLAIM_OVERLAY, overlay, write_length);
        if (style == NOCLAIM)
            // TODO: AJI_INSTRUCTION_NOT_CLAIMED perhaps?
            return AJI_INSTRUCTION_CLAIMED;

        claimed = (style == SAFECLAIM);
    }
    else
        claimed = true;
    if(((m_current_instruction == JTAG_USR1) || (m_current_instruction == JTAG_USR0))) {
        if( write_length > 0)
            write_offset += m_dummy_bits;
        if( read_length > 0)
            read_offset += m_dummy_bits;
        length_dr += m_dummy_bits;
    }
    if (m_defer_error != AJI_NO_ERROR)
        return pending_error();

    DWORD tx_bytes = (write_length + 7) / 8;
    DWORD rx_bytes = (read_length + 7) / 8;

    DWORD server_version = m_client->get_server_version();
    if (batch > 1 && server_version < 5)
    {
        // Server does not understand batched access_dr - send 'batch' single scans
        // which have the same effect instead
        AJI_ERROR error = AJI_NO_ERROR;

        // The checkpoint must be updated after the *last* scan, otherwise the
        // client will get confused and assume that data is valid when it is not.
        // Fortunately we have kept a copy of the checkpoint we used last time
        // and can go back to it.
        DWORD next_checkpoint = m_next_checkpoint;
        m_next_checkpoint = m_next_checkpoint_prev;

        DWORD i;
        for (i = batch ; i > 0 && error == AJI_NO_ERROR ; i--)
        {
            if (i == 1)
                m_next_checkpoint = next_checkpoint;

            error = access_dr(length_dr, flags, write_offset, write_length, write_bits, read_offset, read_length, read_bits, 1);

            if (write_bits != NULL)
                write_bits += tx_bytes;
            if (read_bits != NULL)
                read_bits += rx_bytes;
        }

        return error;
    }

    TXRAWFIFO * txbits = NULL;
    if (write_length > 0)
    {
        txbits = new TXRAWFIFO(m_client, tx_bytes * batch, write_bits);
        if (txbits == NULL)
            return AJI_NO_MEMORY;
    }

    RXRAWFIFO * rxbits = NULL;
    if (read_length > 0)
    {
        rxbits = new RXRAWFIFO(m_client, rx_bytes * batch, read_bits);
        if (rxbits == NULL)
        {
            delete txbits;
            return AJI_NO_MEMORY;
        }
    }

    // Are we going to try and defer execution of this call?
    bool defer(false);
    DWORD total_tx_bytes = tx_bytes * batch;
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, m_pack_stream ? 0 : total_tx_bytes, (rxbits != NULL) ? 1 : 0);
    if (tx == NULL)
    {
        delete txbits;
        delete rxbits;
        return error;
    }

    // Be safe, don't defer in AUTO mode if there is a completion function
    // because we don't know what it will do (completion functions aren't
    // very useful in AUTO mode anyway).
    if (completion_fn != NULL && m_pack_style == AJI_PACK_AUTO)
        defer = false;

    // Don't defer if the overlay register is selected and if this scan is not
    // to an overlay chain we have claimed.
    defer &= claimed;

    // If the caller has allowed it we'll send the data now rather than
    // copying it.  This lets us defer large writes
    bool stream = m_pack_stream && (total_tx_bytes >= 1024);
    if (txbits != NULL && defer && !stream)
    {
        // If we are deferring execution then need to ensure that we have
        // copied the input data since the input buffer will be invalid once
        // we have returned.
        defer = txbits->copy_data();
    }

    if (timestamp != NULL)
    {
        if (server_version >= 11)
            completion_fn = deferred_response_access_dr;
        else
            completion_fn = deferred_response_access_dr_legacy;
        handle = timestamp;
    }

    start_command(tx, MESSAGE::ACCESS_DR, (server_version >= 5) ? 36 : 32);
    tx->add_int(m_open_id);
    tx->add_int(length_dr);
    tx->add_int(flags);
    tx->add_int(write_offset);
    tx->add_int(write_length);
    tx->add_int(read_offset);
    tx->add_int(read_length);
    if (server_version >= 5)
        tx->add_int(batch);

    if (txbits != NULL)
        txbits->activate(0, false);

    if (read_length > 0)
        rxbits->activate(0);

    // Calling activate() on a class derived from TXFIFO passes the ownership
    // to the TCP client socket so we don't need to free it here.

    // Try to defer execution until later
    if (defer)
    {
        assume_success(completion_fn, handle, total_tx_bytes);

        // rxbits has been stored and will be deleted once the command has been
        // executed.
        if (rxbits != NULL)
            expect_output(rxbits);

        if (m_ir_overlay)
            m_last_overlay = overlay;

        m_state = UPDATE_DR;

        if (stream)
            send_continue();

        return AJI_NO_ERROR;
    }

    RXMESSAGE * rx;
    error = send_receive(&rx);

    if (error == AJI_NO_ERROR && rxbits != NULL && !rxbits->wait_for_data())
    {
        m_client->disconnect();
        error = AJI_SERVER_ERROR;
    }

    if (completion_fn != NULL)
        error = (*completion_fn)(handle, error, rx);

    delete rxbits;

    if (m_ir_overlay)
        m_last_overlay = overlay;

    m_state = UPDATE_DR;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::sort_devices(SORT_INFO * * info_ptr, DWORD num_devices, AJI_OPEN_JS * const * open_id)
{
    SORT_INFO * info;
    DWORD i;
    
    info = new SORT_INFO[num_devices];
    if (info == NULL)
        return AJI_NO_MEMORY;

    for (i = 0; i < num_devices; i++)
    {
        info[i].index = i;
        info[i].position = open_id[i]->m_position;
    }

    qsort(info, num_devices, sizeof(SORT_INFO), &aji_compare_positions);

    // Check that all the devices are different
    for (i = 1 ; i < num_devices ; i++)
        if (info[i-1].position <= info[i].position)
        {
            delete[] info;
            return AJI_INVALID_PARAMETER;
        }

    *info_ptr = info;
    return AJI_NO_ERROR;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
extern "C" int aji_compare_positions(const void * elem1, const void * elem2)
//
// Description: Compares the elements and returns which is larger.
//              Solaris requires it to be declared extern "C" since its a
//              callback passed to qsort and bsearch
//
// Returns:     1 if elem1 is larger than elem2, 0 if same, -1 otherwise
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    const AJI_OPEN_JS::SORT_INFO * info1 = static_cast<const AJI_OPEN_JS::SORT_INFO *>(elem1);
    const AJI_OPEN_JS::SORT_INFO * info2 = static_cast<const AJI_OPEN_JS::SORT_INFO *>(elem2);

    return info2->position - info1->position;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_OPEN_JS::CLAIM_STYLE AJI_OPEN_JS::claimed_class
(
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
//
// Returns:     NOCLAIM if use not allowed, SAFECLAIM if use allowed, IFAVAIL
//              if we must check with the server.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    const AJI_CLAIM2 * claim;
    const AJI_CLAIM2 * end = m_claims + m_claim_n;

    // 0x1 is the mask for overlay claim type
    const int overlay_mask = 0x1;
    // 0x800 is the mask for weak claim type
    const int weak_mask = 0x800;

    // Check for exact match
    for (claim = m_claims ; claim < end ; claim++)
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
    for (claim = m_claims ; claim < end ; claim++)
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
//START_FUNCTION_HEADER////////////////////////////////////////////////////////

AJI_ERROR AJI_OPEN_JS::any_sequence_internal  (DWORD                num_tcks,
                                               const BYTE         * tdi_bits,
                                               const BYTE         * tms_bits,
                                               BYTE               * tdo_bits,
                                               bool                 allow)
{
    if (!m_client->version_ok(4, 0xFFFF))
        return AJI_UNIMPLEMENTED;

    if (!m_locked)
        return AJI_NOT_LOCKED;

    if (!allow)
        return AJI_INVALID_PARAMETER;

    if (m_ir_multiple)
        return AJI_IR_MULTIPLE;

    if (m_defer_error != AJI_NO_ERROR)
        return pending_error();

    // For single device opens check that the sequence is OK.
    // TODO: write back the updated state value at the end of this function,
    // not now as errors later might stop us from updating it.
    TAP_STATE new_state = m_state;
    if (m_style & INDIVIDUAL)
    {
        if (!check_transitions(tms_bits, num_tcks, new_state))
            return AJI_BAD_SEQUENCE;
    }
    else
    {
        update_state(tms_bits, num_tcks, new_state);
    }

    DWORD len = (num_tcks + 7) / 8;

    TXRAWFIFO * tdi = new TXRAWFIFO(m_client, len, tdi_bits);
    TXRAWFIFO * tms = new TXRAWFIFO(m_client, len, tms_bits);

    if (tdi == NULL || tms == NULL)
    {
        delete tdi;
        delete tms;
        return AJI_NO_MEMORY;
    }

    RXRAWFIFO tdo(m_client, (tdo_bits != NULL) ? len : 0, tdo_bits);

    // Are we going to try and defer execution of this call?
    bool defer(false);
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, len * 2, (tdo_bits != NULL) ? 1 : 0);
    if (tx == NULL)
    {
        delete tdi;
        delete tms;
        return error;
    }

    // If we are deferring execution then need to ensure that we have
    // copied the input data since the input buffer will be invalid once
    // we have returned.
    if (defer)
        defer = tms->copy_data();
    if (defer)
        defer = tdi->copy_data();

    start_command(tx, MESSAGE::ANY_SEQUENCE, 16);
    tx->add_int(m_open_id);
    tx->add_int(num_tcks);
    tx->add_int((tdo_bits != NULL) ? 1 : 0);

    tdi->activate(0, true);
    tms->activate(1, false);

    if (tdo_bits != NULL)
        tdo.activate(0);

    // Calling activate() on a class derived from TXFIFO passes the ownership
    // to the TCP client socket so we don't need to free it here.

    // Try to defer execution until later
    if (defer)
    {
        assume_success(NULL, NULL, len * 2);

        // rxbits has been stored and will be deleted once the command has been
        // executed.
        if (tdo_bits != NULL)
            expect_output(&tdo);

        error = AJI_NO_ERROR;
    }
    else
    {
        RXMESSAGE * rx;
        error = send_receive(&rx);

        if (error == AJI_NO_ERROR && tdo_bits != NULL && !tdo.wait_for_data())
        {
            m_client->disconnect();
            error = AJI_SERVER_ERROR;
        }
    }

    m_state = new_state;
    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_OPEN_JS::check_transitions(const BYTE * tms_bits, DWORD bitlength, TAP_STATE & state)
//
// Description: Check that the sequence is legal and calculate the exit state
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    DWORD     bits   = ((tms_bits != NULL) ? (*tms_bits++) : 0) | 0x100;
    DWORD     ir_len = 0;

    for (DWORD i = 0 ; i < bitlength ; i++)
    {
        TAP_STATE next = next_tap_state[state][bits & 1];

        if (next == TEST_LOGIC_RESET ||
            (state == EXIT2_DR && next == SHIFT_DR) ||
            (state == EXIT2_IR && next == SHIFT_IR))
            return false;

        if (state == CAPTURE_IR)
            ir_len = 0;
        else if (state == SHIFT_IR)
        {
            ir_len++;
            if (next != SHIFT_IR && ir_len != m_device.instruction_length)
                return false;
        }

        state = next;

        bits >>= 1;
        if (bits < 2)
            bits = ((tms_bits != NULL) ? (*tms_bits++) : 0) | 0x100;
    }

    // Not allowed to leave the TAP controller within an IR scan since that
    // would defeat IR length checking.
    if (state == CAPTURE_IR ||
        state == SHIFT_IR   || state == EXIT1_IR ||
        state == PAUSE_IR   || state == EXIT2_IR)
        return false;

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_OPEN_JS::update_state(const BYTE * tms_bits, DWORD bitlength, TAP_STATE & state)
//
// Description: Calculate the exit state.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (tms_bits == NULL)
        return;

    DWORD bits = (*tms_bits++) | 0x100;

    for (DWORD i = 0 ; i < bitlength ; i++)
    {
        if (bits < 2)
            bits = (*tms_bits++) | 0x100;

        state = next_tap_state[state][bits & 1];

        bits >>= 1;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

AJI_ERROR AJI_OPEN_JS::run_test_idle(DWORD num_clocks, DWORD flags)
{
    if (num_clocks == 0)
        return AJI_INVALID_PARAMETER;

    // Can call this for individual or whole chain devices
    if ((m_style & (INDIVIDUAL | NODE | WHOLE_CHAIN)) == 0)
        return AJI_INVALID_PARAMETER;

    if (!m_locked)
        return AJI_NOT_LOCKED;

    if (((1 << m_state) & VALID_STATES) == 0)
        return AJI_BAD_TAP_STATE;

    if (m_defer_error != AJI_NO_ERROR)
        return pending_error();

    bool server_supports_flags = m_client->version_ok(12, 0xFFFF);
    if (!server_supports_flags && (flags & AJI_EXIT_RTI_STATE) != 0)
    {
        AJI_ERROR error = run_test_idle(num_clocks, 0);
        if (error == AJI_NO_ERROR)
        {
            BYTE one = 1;
            error = any_sequence_internal(1, &one, &one, NULL, true);
        }
        return error;
    }

    bool defer(false);
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, 0, 0);
    if (tx == NULL)
        return error;

    start_command(tx, MESSAGE::RUN_TEST_IDLE, server_supports_flags ? 16 : 12);
    tx->add_int(m_open_id);
    tx->add_int(num_clocks);
    if (server_supports_flags)
        tx->add_int(flags);

    // Attempt to defer execution until later
    if (defer)
    {
        assume_success(NULL, NULL, 0);

        m_state = RUN_TEST_IDLE;
        return AJI_NO_ERROR;
    }

    RXMESSAGE * rx;
    error = send_receive(&rx);

    if (error == AJI_NO_ERROR)
        m_state = RUN_TEST_IDLE;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::test_logic_reset(void)
//
// Description: clock the TAP controller this device is on through the
//              test-logic-reset state.  This doesn't work on version 0 servers
//              because they require an exclusive lock and that is broken.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (!m_client->version_ok(1, 0xFFFF))
        return AJI_UNIMPLEMENTED;

    if (!m_locked)
        return AJI_NOT_LOCKED;

    // Can call this for individual or whole chain devices
    if ((m_style & (INDIVIDUAL | WHOLE_CHAIN)) == 0)
        return AJI_INVALID_PARAMETER;

    if (m_defer_error != AJI_NO_ERROR)
        return pending_error();

    bool defer(false);
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, 0, 0);
    if (tx == NULL)
        return error;

    start_command(tx, MESSAGE::TEST_LOGIC_RESET, 8);
    tx->add_int(m_open_id);

    // Attempt to defer execution until later if we have an exclusive lock to the
    // chain.  If we don't have an exclusive lock then success depends on which
    // other taps on the chain are open so we must send the message
    if (defer)
    {
        assume_success(NULL, NULL, 0);

        m_state = TEST_LOGIC_RESET;
        return AJI_NO_ERROR;
    }

    RXMESSAGE * rx;
    error = send_receive(&rx);

    m_state = TEST_LOGIC_RESET;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::delay(DWORD microseconds)
{
    if (!m_locked)
        return AJI_NOT_LOCKED;

    if (m_defer_error != AJI_NO_ERROR)
        return pending_error();

    if (microseconds == 0)
        return AJI_NO_ERROR;

    bool defer(false);
    AJI_ERROR error = AJI_NO_ERROR;
    TXMESSAGE * tx = get_txmessage(defer, error, 0, 0);
    if (tx == NULL)
        return error;

    start_command(tx, MESSAGE::DELAY_MICROSECONDS, 12);
    tx->add_int(m_open_id);
    tx->add_int(microseconds);

    // Attempt to defer execution until later
    if (defer)
    {
        assume_success(NULL, NULL, 0);
        return AJI_NO_ERROR;
    }

    RXMESSAGE * rx;
    return send_receive(&rx);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::open_node(const AJI_HIER_ID * node_info,
                                 QWORD              overlay_select,
                                 DWORD              overlay_total_length,
                                 DWORD              overlay_node_length,
                                 QWORD              force_capture,
                                 QWORD              prevent_program,
                                 AJI_OPEN_ID      * node_id,
                                 const AJI_CLAIM2 * claims,
                                 DWORD              claim_n,
                                 const char       * application_name)
//
// Description: Modify overlay claims with select bits and call open_device()
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    bool version13 = m_client->version_ok(13, 0xFFFF);
    bool need_hier_hub_support = node_info->position_n > 0;
    if (need_hier_hub_support && !version13)
        return AJI_HIERARCHICAL_HUB_NOT_SUPPORTED;

    // TODO Use two's complement instead
    QWORD overlay_mask = (1ull << overlay_node_length) - 1;

    if ( (overlay_node_length == 0) || (overlay_node_length > 32) || (overlay_total_length > (sizeof(QWORD) * CHAR_BIT)) )
        return AJI_IR_LENGTH_ERROR;

    if ((overlay_mask & overlay_select) != 0 ||
        ((overlay_mask | overlay_select) >= (1ull << overlay_total_length) && (overlay_total_length < sizeof(QWORD) * CHAR_BIT)) ||
        node_id == NULL || claims == NULL || application_name == NULL)
        return AJI_INVALID_PARAMETER;

    AJI_CLAIM2 * mod_claims = new AJI_CLAIM2[claim_n+4];
    if (mod_claims == NULL)
        return AJI_NO_MEMORY;

    // Overlay and overlaid IR values can be copied from the device
    mod_claims[0].type  = AJI_CLAIM_IR_SHARED_OVERLAY;
    mod_claims[0].length = 0; // Non-overlay claim always have length of 0
    mod_claims[0].value = m_overlay_ir;
    mod_claims[1].type  = AJI_CLAIM_IR_SHARED_OVERLAID;
    mod_claims[1].length = 0;
    mod_claims[1].value = m_overlaid_ir;
    DWORD n = 2;

    // If one has been provided then claim an instruction to prevent the
    // Quartus programmer from running at the same time as we do.
    if (prevent_program != ~0ull)
    {
        mod_claims[n].type  = AJI_CLAIM_IR_SHARED;
        mod_claims[n].length = 0;
        mod_claims[n].value = prevent_program;
        n++;
    }

    // Claim the FORCE_IR_CAPTURE value for this node if appropriate
    if (force_capture != ~0ull)
    {
        mod_claims[n].type  = AJI_CLAIM_OVERLAY_SHARED;
        mod_claims[n].length = overlay_total_length;
        mod_claims[n].value = force_capture;
        n++;
    }

    // Modify the claims based on the overlay stuff
    bool ok = true;
    bool claimed_overlays = false;
    for (DWORD i = 0 ; i < claim_n && ok ; i++)
    {
        mod_claims[n].type = claims[i].type;

        switch(claims[i].type)
        {
        case AJI_CLAIM_IR:
        case AJI_CLAIM_IR_SHARED:
        case AJI_CLAIM_IR_WEAK:
            mod_claims[n].length = 0;
            mod_claims[n++].value = claims[i].value;
            break;

        case AJI_CLAIM_IR_SHARED_OVERLAY:
            mod_claims[n].length = 0;
            ok = (claims[i].value == m_overlay_ir);
            break;
        case AJI_CLAIM_IR_OVERLAID:
        case AJI_CLAIM_IR_SHARED_OVERLAID:
            mod_claims[n].length = 0;
            ok = (claims[i].value == m_overlaid_ir);
            break;

        case AJI_CLAIM_OVERLAY:
        case AJI_CLAIM_OVERLAY_SHARED:
        case AJI_CLAIM_OVERLAY_WEAK:
            mod_claims[n].length = overlay_total_length;
            if (claims[i].value != ~0ull)
            {
                ok = (claims[i].value & ~overlay_mask) == 0;
                mod_claims[n++].value = claims[i].value | overlay_select;
            }
            else
            {
                mod_claims[n++].value = ~0ull;
            }
            claimed_overlays = true;
            break;
        }
    }

    // If no overlay values claimed then there is no point to opening this node
    // as it will be useless.
    if (!claimed_overlays)
        ok = false;

    if (!ok)
    {
        delete[] mod_claims;
        return AJI_INVALID_PARAMETER;
    }

    AJI_OPEN_JS * node = NULL;
    AJI_ERROR error = m_chain->open_device(m_position, reinterpret_cast<AJI_OPEN_ID *>(&node), mod_claims, n, application_name);

    delete[] mod_claims;

    if (error == AJI_NO_ERROR)
    {
        node->m_style                     = AJI_OPEN_JS::NODE;
        node->m_node_overlay_select       = overlay_select;
        node->m_node_overlay_total_length = overlay_total_length;
        node->m_node_overlay_node_length  = overlay_node_length;
        node->m_node_force_capture        = force_capture;
        node->m_node_info.idcode          = node_info->idcode;
        node->m_node_info.position_n      = node_info->position_n;
        for (DWORD i = 0; i < 8; i++)
            node->m_node_info.positions[i] = node_info->positions[i];

        *node_id = node;
    }

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::deferred_response_overlay(void * handle, AJI_ERROR error, RXMESSAGE * )
//
// Description: Process the response from the server after it has handled an
//              access_dr message which captures data from an overlay register.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    OVERLAY_CAPTURE * cap = reinterpret_cast<OVERLAY_CAPTURE *>(handle);

    if (error == AJI_NO_ERROR)
    {
        DWORD capture = 0;
        DWORD i;
        for (i = 0 ; i < cap->m_length ; i += 8)
            capture |= cap->m_data[i/8] << i;

        *(cap->m_capture_ptr) = capture;
    }

    delete cap;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::access_overlay(DWORD update, DWORD * captured)
//
// Description: 
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    DWORD i;
    BYTE update_data[4];

    if (!m_locked)
        return AJI_NOT_LOCKED;

    // Can only call this for nodes
    if ((m_style & NODE) == 0)
        return AJI_INVALID_PARAMETER;

    DWORD overlay_mask = (1u << m_node_overlay_node_length) - 1;

    if (update & ~overlay_mask)
        return AJI_INVALID_PARAMETER;

    if (((1 << m_state) & VALID_STATES) == 0)
        return AJI_BAD_TAP_STATE;

    if (m_defer_error != AJI_NO_ERROR)
        return pending_error();

    OVERLAY_CAPTURE * cap = NULL;
    if (captured != NULL)
    {
        cap = new OVERLAY_CAPTURE;
        if (cap == NULL)
            return AJI_NO_MEMORY;
    }

    // Load up the overlay instruction
    AJI_ERROR error = access_ir(m_overlay_ir, NULL, 0);

    if (captured != NULL && error == AJI_NO_ERROR)
    {
        // If we are capturing the value from the overlay register then we must
        // ensure the value captured comes from the correct node.  Issue the
        // force IR capture instruction to make sure of this.

        // #========================================================#
        // |                       update_data                      |
        // #========================================================#
        // |                   (overlay_total_length)               |
        // |                                                        |
        // +----------------+---------------------------------------+
        // |        0       |             overlay_force             |
        // +----------------+---------------------------------------+
        //     (sel_bits)   |               (ir_bits)               |
        //                  |                                       |
        //                  +----------------+----------------------+
        //                  | overlay_select | HUB_FORCE_IR_CAPTURE |
        //                  +----------------+----------------------+
        //                      (sel_bits)      (N_HUB_INST_BITS)

        for (i = 0 ; i < m_node_overlay_total_length ; i += 8)
            update_data[i/8] = static_cast<BYTE>(m_node_force_capture >> i);

        error = access_dr(m_node_overlay_total_length, 0, 0,m_node_overlay_total_length,update_data,
                          0,0,NULL, 1);
    }

    // Modify instruction to hold the node select bits
    QWORD mod_update = update | m_node_overlay_select;

    // #========================================================#
    // |                       update_data                      |
    // #========================================================#
    // |                   (overlay_total_length)               |
    // |                                                        |
    // +----------------+---------------------------------------+
    // | overlay_select |               update                  |
    // +----------------+---------------------------------------+
    //     (sel_bits)                  (ir_bits)

    for (i = 0 ; i < m_node_overlay_total_length ; i += 8)
        update_data[i/8] = static_cast<BYTE>(mod_update >> i);

    if (error == AJI_NO_ERROR)
    {
        if (captured == NULL)
            error = access_dr(m_node_overlay_total_length, 0, 0,m_node_overlay_total_length,update_data, 
                              0,0,NULL, 1);
        else
        {
            cap->m_capture_ptr = captured;
            cap->m_length = m_node_overlay_node_length;

            error = access_dr(m_node_overlay_total_length, 0, 0,m_node_overlay_total_length,update_data, 
                              0,m_node_overlay_node_length,cap->m_data, 1, 
                              deferred_response_overlay, cap);
        }
    }
    else
        delete cap;

    if (error == AJI_NO_ERROR)
        error = access_ir(m_overlaid_ir, NULL, 0);

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::flush(DWORD timeout)
{
    // All commands which can be deferred require locking.  Check this because
    // we won't have claimed the link mutex unless we are locked.
    if (!m_locked)
        return AJI_NO_ERROR;

    AJI_ERROR error = AJI_NO_ERROR;

    if (m_fill_actions != NULL || m_next_actions != NULL)
        error = send_receive(NULL, timeout);

    if (error == AJI_NO_ERROR && timeout == ~0u)
        m_checkpoint = m_next_checkpoint;

    return error;
}
//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_OPEN_JS::register_progress_callback(void (* progress_fn)(void * handle, DWORD bits),
                                          void * handle)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    m_progress_fn     = progress_fn;
    m_progress_handle = handle;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_OPEN_JS::notify_data_used(DWORD data_used)
{
    if (m_progress_fn != NULL)
        (*m_progress_fn)(m_progress_handle, data_used);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::send_receive(RXMESSAGE * * rx_ptr, DWORD timeout)
//
// Description: Send all the pending messages, wait for a response and then
//              call the appropriate functions.
//
// Preconditions: link mutex must be held
//
// Postconditions: link mutex will be held
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_ERROR error = AJI_NO_ERROR;

    if (m_fill_actions != NULL)
    {
        bool ok;
        if (timeout == ~0u)
            ok = send_command();
        else if (m_client->txmessage_is_continue())
            ok = true;
        else
            ok = send_continue();
        if (!ok)
            error = AJI_NET_DOWN;
    }

    // The JTAG server always sends a response packet for each request packet.
    // If there is a non-fatal error in an earlier packet then the CONTINUE_COMMAND
    // command will ensure that the same error is in all subsequent packets.
    // If there is a fatal error then it will still be there.

    if (error == AJI_NO_ERROR)
        while   (m_next_actions != NULL)
        {
            AJI_ERROR new_error;
            new_error = receive_response((m_next_actions->m_next == NULL) ? rx_ptr : NULL, timeout);
            if (new_error == AJI_NET_DOWN || new_error == AJI_SERVER_ERROR)
            {
                error = new_error;
                break;
            }
            if (error == AJI_NO_ERROR)
                error = new_error;
            if (timeout != ~0u)
                break;
        }

    // TODO: cope with AJI_INVALID_DEVICE from server properly (by deleting *this)

    if ((error == AJI_NET_DOWN || error == AJI_SERVER_ERROR) && m_client != NULL)
    {
        m_client->disconnect();
        AJI_DEBUG_ASSERT(m_client == NULL);
        AJI_DEBUG_ASSERT(m_chain  == NULL);
    }

    // If a speculative lock has failed then tidy up here.
    if (m_locked && m_lock_fail && m_client != NULL)
    {
        m_locked = false;
        m_client->txmessage_cancel(this);
        m_client->release_link();
    }

    m_next_checkpoint_prev = m_next_checkpoint;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
// Description: Send packets to server without waiting for response to free up
//              local tx buffer.
//
bool AJI_OPEN_JS::send_continue(void)
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(m_locked);

    if (!send_command())
        return false;

    AJI_ERROR error = AJI_NO_ERROR;

    bool defer(false);
    TXMESSAGE * tx = get_txmessage(defer, error, 0, 0);
    if (tx == NULL)
        return false;

    AJI_DEBUG_ASSERT(defer);

    start_command(tx, MESSAGE::CONTINUE_COMMANDS, 4);

    assume_success(NULL, (void *)1, 0);

    // TODO: think about how to cancel the command sequence more efficiently if
    // a previous command has failed.  ie. if error has just been set there is no
    // point in sending the next command packet since CONTINUE_COMMANDS will just
    // return the same error.  For the moment we just refuse to defer commands,
    // and send a packet containing CONTINUE and the new command, which will 
    // fail again.

    return (error == AJI_NO_ERROR) && !m_lock_fail;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool AJI_OPEN_JS::send_command(void)
{
    AJI_DEBUG_ASSERT(m_fill_actions != NULL);

    // We shouldn't send a command if it is empty or if it only contains
    // CONTINUE_COMMAND.
    //AJI_DEBUG_ASSERT(m_fill_actions->m_defer_n > 1 || m_fill_actions->m_deferred[0].m_handle != (void *)1);

    if (m_client == NULL)
        return false;

    else if (!m_client->send())
        return false;

    if (m_next_actions == NULL)
        m_next_actions = m_last_actions = m_fill_actions;
    else
    {
        m_last_actions->m_next = m_fill_actions;
        m_last_actions = m_fill_actions;
    }

    m_fill_actions = NULL;

    return true;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::receive_response(RXMESSAGE * * rx_ptr, DWORD timeout)
{
    AJI_DEBUG_ASSERT(m_next_actions != NULL);

    AJI_ERROR error = m_client->receive_timeout(timeout);
    if (error == AJI_TIMEOUT && timeout != ~0u)
        return AJI_NO_ERROR;
    if (error != AJI_NO_ERROR)
        return error;

    ACTIONS * actions = m_next_actions;
    DWORD i;

    RXMESSAGE * rx = m_client->get_rxmessage();

    // If there were many requests in the packet then call the deferred response
    // functions to remove the initial responses from the response packet.
    for (i = 0 ; i < actions->m_defer_n ; i++)
    {
        if (error == AJI_NO_ERROR)
            if (!rx->remove_response(&error))
                error = AJI_SERVER_ERROR;

        DEFERRED * deferred = &actions->m_deferred[i];
        DEFERRED_RESPONSE_FN_PTR fn = deferred->m_fn;

        if (fn != NULL)
            error = (*fn)(deferred->m_handle, error, rx);

        if (error == AJI_NO_ERROR)
            m_checkpoint = deferred->m_checkpoint;
    }

    if (error == AJI_NO_ERROR)
    { 
        // If there were output FIFOs then we need to wait for all of them to fill up.
        // Waiting for just the last one is sufficient.
        if (actions->m_output_n > 0)
            actions->m_output[actions->m_output_n-1]->wait_for_data();
    }

    // Delete any output FIFOs which are collecting responses from this packet.
    for (i = 0 ; i < actions->m_output_n ; i++)
        delete actions->m_output[i];

    if (error == AJI_NO_ERROR && rx_ptr != NULL)
    {
        RXMESSAGE * rx = m_client->get_rxmessage();

        if (!rx->remove_response(&error))
            error = AJI_SERVER_ERROR;

        if (error == AJI_NO_ERROR)
            m_checkpoint = m_next_checkpoint;

        *rx_ptr = rx;
    }

    m_next_actions = actions->m_next;

    actions->m_next = m_free_actions;
    m_free_actions = actions;

    if (error != AJI_NO_ERROR && !m_locked)
    {
        // If we're not locked now but the other end thinks we are then send an extra
        // unlock message here to get us back in sync.

        // TODO: only do this if we might have lost an unlock.
        m_locked = true;
        error = add_unlock();
        m_client->send_receive();

        m_locked = false;
    }

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_OPEN_JS::pending_error(void)
{
    AJI_ERROR error = m_defer_error;
    m_defer_error = AJI_NO_ERROR;

    return error;
}

