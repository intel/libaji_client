/****************************************************************************
 *   Copyright (c) 2016 by Intel Corporation                                *
 *   author: Draper, Andrew  and Loh, Hong Lee                              *
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
// Description:
//
// Authors:     Andrew Draper, Hong Lee Loh
//
//              Copyright (c) Altera Corporation 2016
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

#ifndef INC_AJI_H
#include "aji.h"
#endif

#ifndef INC_JTAG_CLIENT_HUB_H
#include "jtag_client_hub.h"
#endif

static const int N_HUB_INST_BITS = 3;
static const int MAX_HIERARCHIES = AJI_MAX_HIERARCHICAL_HUB_DEPTH;

static const DWORD NODE_ID_BRIDGE = 0xFF;
static const DWORD MFG_ID_ALTERA  = 0x6E;

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_HUB * AJI_HUB::get_hub
(
    AJI_CHAIN_ID   chain_id,
    DWORD          tap_position
)
//
// Description: Create a hub attached to the position specified.  The chain
//              must be locked.
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    return AJI_HUB::get_hub(chain_id, tap_position, NULL);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_HUB * AJI_HUB::get_hub
(
    AJI_CHAIN_ID   chain_id,
    DWORD          tap_position,
    AJI_HUB      * parent_hub
)
//
// Description:
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    // TODO: if a hub already exists then share it.  This will mean that we can
    // cut out a lot of the re-enumeration stuff.

    AJI_HUB * hub = new AJI_HUB(chain_id, tap_position, parent_hub);
    if (hub == NULL)
        return NULL;

    return hub;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_HUB::AJI_HUB
(
    AJI_CHAIN_ID   chain,
    DWORD          tap_position,
    AJI_HUB      * parent_hub
)
//
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
    : m_hub_id(NULL), m_chain(chain), m_tap_position(tap_position),
      m_use_count(1), m_idcodes(NULL), m_idcode_n(0),
      m_parent_hub(parent_hub), m_hub_idcode(0), m_sel_bits(0)
{
    // Claim the PROGRAM instruction to prevent the Quartus programmer from
    // running at the same time as any SLD client.  The programmer will reload
    // the soft logic so will break anything using the hub if it is able to run.
    // TODO Why not AJI_CLAIM_IR exclusive?
    m_claims[0] = { AJI_CLAIM_IR_SHARED,          0, JTAG_PROGRAM };
    // Claim the JTAG IR for Hub VIR
    m_claims[1] = { AJI_CLAIM_IR_SHARED_OVERLAY,  0, JTAG_USR1    };
    m_claims[2] = { AJI_CLAIM_IR_SHARED_OVERLAID, 0, JTAG_USR0    };
    // Claim the HUB_INFO instruction, to be modified with select bits for
    // non-top-level hub. Must always be the last element in the array
    m_claims[3] = { AJI_CLAIM_OVERLAY_SHARED,     0, HUB_INFO     };

    // TODO: register in set of currently in use hubs
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_HUB::~AJI_HUB(void)
//
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    // TODO: remove from set of currently in use hubs
    // Release child hubs
    for (std::map<DWORD, AJI_HUB *>::iterator iter = m_child_hubs.begin(); iter != m_child_hubs.end(); ++iter)
        delete (*iter).second;
printf("***********A");
    if (m_hub_id != NULL) {
printf("***********A in");
       m_hub_id->close_device();
printf("***********A in kill");
        m_hub_id = nullptr;
    }
printf("***********A end");

    delete[] m_idcodes;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_HUB::read_hub_information(bool  scan_hier_hub,
                                        DWORD parent_levels_select_bits /* =0 */,
                                        QWORD parent_levels_select      /* =0 */)
//
// Description: Scan the hub attached to the device specified in order to find
//              out which nodes are present on it.  The chain must be locked
//              and must have been scanned before calling this function.
//              Typically a caller will just use the default value of 0 for
//              parent_levels_select_bits and parent_levels_select. A non-zero
//              value is only passed in by this function itself when a
//              recursive call is made to discover hierarchical hubs.
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    if (m_hub_id != NULL)
        return AJI_NO_ERROR;

    // Don't create the hub if device does not have Altera SRAM style USR0/1
    // instructions - not only is there no point but the instructions might do
    // something dangerous instead.
    if ((m_chain->get_device_features(m_tap_position) & AJI_DEVFEAT_POSSIBLE_HUB) == 0)
    {
        m_idcode_n = 0;
        return AJI_NO_ERROR;
    }

    // Prepend the select bits for the hierarchical hub, it should
    // be parent_levels_select | (HUB_INFO << (64-parent_levels_select_bits))
    // but there is no point of shifting since HUB_INFO is all zeros
    QWORD hub_info_overlay = parent_levels_select | HUB_INFO;

    // The claim for HUB_INFO overlay is always the last one in the m_claims array
    m_claims[sizeof(m_claims)/sizeof(m_claims[0]) - 1].value = hub_info_overlay;
    AJI_ERROR error = aji_open_device(m_chain, m_tap_position, &m_hub_id, m_claims,
                                      sizeof(m_claims)/sizeof(m_claims[0]), "AJI_HUB");

    if (error != AJI_NO_ERROR)
    {
        m_hub_id = NULL;
        return error;
    }

    delete[] m_idcodes;
    m_idcodes = NULL;
    m_idcode_n = 0;
    m_hub_idcode = 0;
    m_child_hubs.clear();

    // Pass lock from chain to device atomically
    error = aji_unlock_chain_lock(m_chain, m_hub_id, AJI_PACK_MANUAL);

    if (error == AJI_NO_ERROR)
    {
        error = aji_access_ir(m_hub_id, JTAG_USR1, NULL);

        // Maximum number of IR bits supported is 64
        BYTE hub_info_overlay_bytes[8], capture[8];
        memset64(hub_info_overlay_bytes, hub_info_overlay);

        if (error == AJI_NO_ERROR)
            error = aji_access_dr(m_hub_id, 64,0, 0,64,hub_info_overlay_bytes, 0,64,capture);

        if (error == AJI_NO_ERROR)
            error = aji_access_ir(m_hub_id, JTAG_USR0, NULL);

        if (error == AJI_NO_ERROR)
            error = read_idcode(&m_hub_idcode, 1);

        if (error != AJI_NO_ERROR || get_mfg_id() != MFG_ID_ALTERA || get_hub_version() != 1 || get_ir_bits() == 0)
            m_hub_idcode = 0;

        // It isn't an error if there is no hub present, we return OK with
        // m_idcode_n = 0 in this case.
        if (error == AJI_NO_ERROR && get_mfg_id() == MFG_ID_ALTERA)
        {
            DWORD idcode_n = get_node_count();
            m_sel_bits = ceil_log2(idcode_n+1);

            if (get_hub_version() != 1)
                error = AJI_FAILURE;

            // If the overlay value's we'll be writing are more than 64 bits then
            // we won't be able to claim them so refuse to deal with this hub.
            else if (parent_levels_select_bits + m_sel_bits + get_ir_bits() > sizeof(QWORD) * CHAR_BIT)
                error = AJI_FAILURE;

            if (error == AJI_NO_ERROR && idcode_n > 0)
            {
                m_idcodes = new DWORD[idcode_n];
                if (m_idcodes == NULL)
                    error = AJI_NO_MEMORY;
                else
                {
                    m_idcode_n = idcode_n;
                    error = read_idcode(m_idcodes, idcode_n);
                }
            }

            // Discover child hubs recursively if supported
            if (scan_hier_hub && error == AJI_NO_ERROR)
            {
                for (DWORD i = 0 ; i < idcode_n ; i++)
                {
                    if (get_node_id(m_idcodes[i]) == NODE_ID_BRIDGE &&
                            get_node_mfg_id(m_idcodes[i]) == MFG_ID_ALTERA)
                    {
                        AJI_HUB * child_hub = AJI_HUB::get_hub(m_chain, m_tap_position, this);

                        // Number of child level hub_info_overlay bits is the accumulation of
                        // all parent levels and current child level hub_info_overlay bits
                        DWORD child_level_select_bits = parent_levels_select_bits + m_sel_bits;

                        // Maximum number of IR bits supported is 64, left align hub_info_overlay bits
                        QWORD child_level_select = ((QWORD)(i + 1)) << (64 - child_level_select_bits);

                        // Construct the hub_info_overlay value for child hub
                        child_level_select |= parent_levels_select;

                        // Pass lock from device to chain atomically
                        aji_unlock_lock_chain(m_hub_id, m_chain);

                        error = child_hub->read_hub_information(scan_hier_hub, child_level_select_bits, child_level_select);

                        // There could be no child hub, only register a child hub if it exists
                        if (child_hub->get_mfg_id() == MFG_ID_ALTERA)
                        	m_child_hubs[i] = child_hub;
                        // Else discard it
                        else
                        	delete child_hub;
                    }
                }
            }
        }

        // Pass lock from device to chain atomically
        aji_unlock_lock_chain(m_hub_id, m_chain);
    }

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
DWORD AJI_HUB::get_hier_id_n()
//
// Description: Get the total number of nodes (not including hubs) in this
//              hierarchy level and beyond.
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    // Number of nodes in this hierarchy level
    DWORD hier_id_n = m_idcode_n;

    // Number of descendant nodes in lower hierarchy levels behind the bridges
    for (std::map<DWORD, AJI_HUB *>::iterator iter = m_child_hubs.begin(); iter != m_child_hubs.end(); ++iter)
    {
        hier_id_n += (*iter).second->get_hier_id_n();
    }

    return hier_id_n;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_HUB::get_hier_ids(BYTE hierarchy, AJI_HIER_ID * hier_ids, DWORD hier_id_n, AJI_HUB_INFO * hub_infos)
//
// Description:
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    DWORD hier_id_used_n;
    AJI_ERROR error = get_hier_ids(hierarchy, hier_ids, hier_id_n, &hier_id_used_n, hub_infos);

    if (error == AJI_NO_ERROR)
    {
        // Check for corner case where top hub has no nodes,
        // its idcode should still be returned
        if (get_parent_hub() == NULL && m_idcode_n == 0 && hub_infos != NULL)
            hub_infos[0].hub_idcode[hierarchy] = m_hub_idcode;
    }

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_HUB::get_hier_ids(BYTE hierarchy, AJI_HIER_ID * hier_ids, DWORD hier_id_n, DWORD * hier_id_used_n, AJI_HUB_INFO * hub_infos)
//
// Description:
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    // Do not exceed the maximum number of hierarchies
    if (hierarchy >= MAX_HIERARCHIES)
        return AJI_TOO_MANY_HIERARCHIES;

    // Check the size allocated by caller
    if (hier_id_n < get_hier_id_n())
        return AJI_TOO_MANY_DEVICES;

    AJI_ERROR error = AJI_NO_ERROR;

    // Always start from 0
    *hier_id_used_n = 0;

    // Iterate all the sibling nodes of the current hub
    for (DWORD i = 0, index = 0; i < m_idcode_n; i++)
    {
        // Only the first node in this hierarchy level has its corresponding elements in
        // higher level populated, other nodes need to copy the them from the first node
        if (i > 0)
            for (int upper_hierarchy = hierarchy-1; upper_hierarchy >= 0 ; upper_hierarchy--)
            {
                hier_ids[index].positions[upper_hierarchy] = hier_ids[0].positions[upper_hierarchy];
                if (hub_infos != NULL) {
                    hub_infos[index].hub_idcode[upper_hierarchy] = hub_infos[0].hub_idcode[upper_hierarchy];
                    hub_infos[index].bridge_idcode[upper_hierarchy+1] = hub_infos[0].bridge_idcode[upper_hierarchy+1];
                }
            }

        if (hub_infos != NULL)
        {
            // Top level nodes do not have a bridge, just assign 0
            if (get_parent_hub() == NULL)
                hub_infos[index].bridge_idcode[hierarchy] = 0xff;
            hub_infos[index].hub_idcode[hierarchy] = m_hub_idcode;
        }

        // Assign information for each nodes
        hier_ids[index].idcode = m_idcodes[i];
        hier_ids[index].position_n = hierarchy;
        hier_ids[index].positions[hierarchy] = static_cast<BYTE>(i);
        (*hier_id_used_n)++;
        index++;
        hier_id_n--;

        // If bridge, recursively go into lower hierarchy
        if (get_node_id(m_idcodes[i]) == NODE_ID_BRIDGE &&
                get_node_mfg_id(m_idcodes[i]) == MFG_ID_ALTERA)
        {
            AJI_HUB * child_hub = get_child_hub(i);
            // There could be no child hub behind a bridge, e.g. the bridge connects to a PR region with signals grounded
            if (child_hub != NULL)
            {
				// Only the first node in this hierarchy level has its corresponding elements in
				// higher level populated, other nodes need to copy the them from the first node
				if (i > 0)
					for (int upper_hierarchy = hierarchy-1; upper_hierarchy >= 0 ; upper_hierarchy--)
					{
						hier_ids[index].positions[upper_hierarchy] = hier_ids[0].positions[upper_hierarchy];
						if (hub_infos != NULL) {
							hub_infos[index].hub_idcode[upper_hierarchy] = hub_infos[0].hub_idcode[upper_hierarchy];
							hub_infos[index].bridge_idcode[upper_hierarchy+1] = hub_infos[0].bridge_idcode[upper_hierarchy+1];
						}
					}

				if (hub_infos != NULL) {
					// If not last level of hierarchy
					if (hierarchy < MAX_HIERARCHIES-1)
						hub_infos[index].bridge_idcode[hierarchy+1] = m_idcodes[i];
					hub_infos[index].hub_idcode[hierarchy] = m_hub_idcode;
				}

				// Position of bridge in this hierarchy level
				hier_ids[index].positions[hierarchy] = static_cast<BYTE>(i);

				DWORD hier_id_used_for_lower_hier_n = 0;
				// Dive into next hierarchy, so increment hierarchy, but use the element of same index
				error = child_hub->get_hier_ids(hierarchy+1, &(hier_ids[index]), hier_id_n, &hier_id_used_for_lower_hier_n, (hub_infos==NULL)?NULL:&(hub_infos[index]));
				// Don't continue if encountered error except AJI_TOO_MANY_HIERARCHIES
				// to know how many nodes within 8 levels of hierarchies are there
				if (error != AJI_NO_ERROR && error != AJI_TOO_MANY_HIERARCHIES)
					break;
				// Lower hierarchies might have more than one child nodes
				(*hier_id_used_n) += hier_id_used_for_lower_hier_n;
				index += hier_id_used_for_lower_hier_n;
				hier_id_n -= hier_id_used_for_lower_hier_n;
            }
        }
    }

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_HUB::read_idcode(DWORD * idcode, DWORD count)
//
// Description: Read eight nibbles from the hub device list port and combine
//              them together into an ID.
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    AJI_DEBUG_ASSERT(count > 0);

    BYTE * nibble = new BYTE [count * 8];
    if (nibble == NULL)
        return AJI_NO_MEMORY;

    AJI_ERROR error = aji_access_dr(m_hub_id, 4,AJI_DR_UNUSED_X, 0,0,NULL, 0,4,nibble, count * 8);

    if (error == AJI_NO_ERROR)
        error = aji_flush(m_hub_id);

    if (error == AJI_NO_ERROR)
    {
        BYTE * nptr = nibble;
        do
        {
            DWORD code = 0;
            for (DWORD i = 0 ; i < 8 ; i++)
                code |= *nptr++ << (i * 4);

            *idcode++ = code;
        }
        while (--count > 0);
    }

    delete[] nibble;

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
DWORD AJI_HUB::ceil_log2(DWORD data)
//
// Description: Work out how many bits are required to encode "data" states.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    DWORD out = 0;

    if (data == 0)
        return 0;

    data--;
    while (data > 0)
        out++, data >>= 1;

    return out;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void AJI_HUB::memset64(BYTE * dest, QWORD value)
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
AJI_ERROR AJI_HUB::open_node(AJI_OPEN_ID * node_id, DWORD node_index, const AJI_CLAIM2 * claims, DWORD claim_n, const char * application_name)
//
// Description:
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    DWORD ir_bits = get_ir_bits();

    QWORD overlay_select = (QWORD)(node_index + 1) << ir_bits;
    DWORD overlay_total_length = m_sel_bits + ir_bits;

    QWORD overlay_force  = ( (node_index + 1) << N_HUB_INST_BITS ) | HUB_FORCE_IR_CAPTURE;

    // +----------------+---------------------------------------+
    // |       0        |             overlay_force             |
    // +----------------+---------------------------------------+
    //     (sel_bits)   |               (ir_bits)               |
    //                  |                                       |
    //                  +----------------+----------------------+
    //                  | overlay_select | HUB_FORCE_IR_CAPTURE |
    //                  +----------------+----------------------+
    //                      (sel_bits)      (N_HUB_INST_BITS)

    // +----------------+---------------------------------------+
    // | overlay_select |               update                  |
    // +----------------+---------------------------------------+
    //     (sel_bits)                  (ir_bits)

    AJI_HIER_ID hier_id = { m_idcodes[node_index], 0, (BYTE)node_index,0,0,0,0,0,0,0 };
    return m_hub_id->open_node(&hier_id, overlay_select, overlay_total_length, ir_bits,
                               overlay_force, JTAG_PROGRAM, node_id, claims, claim_n, application_name);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_HUB::open_node(AJI_OPEN_ID * node_id, const AJI_HIER_ID * hier_id, const AJI_CLAIM2 * claims, DWORD claim_n, const char * application_name)
//
// Description:
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{

    BYTE level = hier_id->position_n;
    DWORD ir_bits = get_ir_bits();
    DWORD overlay_total_length = ir_bits;

    // Use BYTE to make sure 0xff (hub select) will wrap to 0x0
    BYTE select = hier_id->positions[level] + 1;
    QWORD overlay_select = (QWORD)select << overlay_total_length;
    overlay_total_length += get_sel_bits();
    AJI_HUB * hub = get_parent_hub();
    while (hub != NULL)
    {
        level--;
        // Use BYTE to make sure 0xff (hub select) will wrap to 0x0
        select = hier_id->positions[level] + 1;
        overlay_select |= select << overlay_total_length;
        overlay_total_length += hub->get_sel_bits();
        hub = hub->get_parent_hub();
    }

    QWORD overlay_force  = ( select << N_HUB_INST_BITS ) | HUB_FORCE_IR_CAPTURE;

    return m_hub_id->open_node(hier_id, overlay_select, overlay_total_length, ir_bits,
                               overlay_force, JTAG_PROGRAM, node_id, claims, claim_n, application_name);
}
