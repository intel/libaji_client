/****************************************************************************
 *   Copyright (c) 2016 by Intel Corporation                                *
 *   author: Draper, Andrew and Loh, Hong Lee                               *
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
// START_MODULE_HEADER/////////////////////////////////////////////////////
//
// Filename:    jtag_client_hub.h
//
// Description:
//
// Authors:     Andrew Draper, Hong Lee Loh
//
//              Copyright (c) Altera Corporation 2016
//              All rights reserved.
//
// END_MODULE_HEADER///////////////////////////////////////////////////////

// START_ALGORITHM_HEADER//////////////////////////////////////////////////
//
//
// END_ALGORITHM_HEADER////////////////////////////////////////////////////
//

#ifndef INC_JTAG_CLIENT_HUB_H_
#define INC_JTAG_CLIENT_HUB_H_

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_AJI_H
#include "aji.h"
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

//START_CLASS_DEFINITION//////////////////////////////////////////////////////

class AJI_HUB
{
public:
    static AJI_HUB * get_hub(AJI_CHAIN_ID chain_id, DWORD tap_position);
    static AJI_HUB * get_hub(AJI_CHAIN_ID chain_id, DWORD tap_position, AJI_HUB * parent_hub);

    AJI_ERROR read_hub_information(bool scan_hier_hub, DWORD parent_levels_select_bits = 0, QWORD parent_levels_select = 0);

    void put_hub(void) { printf("****** B m_use_count=%ld\n", m_use_count);  if (--m_use_count == 0) delete this; }
    
    const DWORD * get_idcodes(void) const { return m_idcodes; }
    DWORD get_idcode_n(void) const { return m_idcode_n; }

    AJI_HUB * get_parent_hub() const { return m_parent_hub; }
    AJI_HUB * get_child_hub(DWORD index) const { return (m_child_hubs.find(index) != m_child_hubs.end()) ? m_child_hubs.at(index) : NULL; }
    DWORD get_child_hub_n(void) const { return static_cast<DWORD>(m_child_hubs.size()); }

    AJI_ERROR get_hier_ids(BYTE position, AJI_HIER_ID * hier_ids, DWORD hier_id_n, AJI_HUB_INFO * hub_infos);
    DWORD get_hier_id_n();

    AJI_ERROR open_node(AJI_OPEN_ID * node_id, DWORD node_index, const AJI_CLAIM2 * claims, DWORD claim_n, const char * application_name);
    AJI_ERROR open_node(AJI_OPEN_ID * node_id, const AJI_HIER_ID * hier_id, const AJI_CLAIM2 * claims, DWORD claim_n, const char * application_name);

    DWORD get_hub_idcode(void) const  { return m_hub_idcode; }
    DWORD get_ir_bits(void) const     { return (m_hub_idcode & 0x000000FFul); }
    DWORD get_mfg_id(void) const      { return (m_hub_idcode & 0x0007FF00ul) >> 8; }
    DWORD get_node_count(void) const  { return (m_hub_idcode & 0x07F80000ul) >> 19; }
    DWORD get_hub_version(void) const { return (m_hub_idcode & 0xF8000000ul) >> 27; }
    DWORD get_sel_bits(void) const    { return m_sel_bits; }

private:

    AJI_HUB(AJI_CHAIN_ID chain_id, DWORD tap_position, AJI_HUB * parent_hub);
    ~AJI_HUB(void);

    AJI_ERROR read_idcode(DWORD * idcode, DWORD count);

    AJI_ERROR get_hier_ids(BYTE position, AJI_HIER_ID * hier_ids, DWORD hier_id_n, DWORD * hier_id_used_n, AJI_HUB_INFO * hub_infos);

    static DWORD get_node_instance(DWORD idcode) { return (idcode & 0x000000FFul); }
    static DWORD get_node_mfg_id(DWORD idcode)   { return (idcode & 0x0007FF00ul) >> 8; }
    static DWORD get_node_id(DWORD idcode)       { return (idcode & 0x07F80000ul) >> 19; }
    static DWORD get_node_version(DWORD idcode)  { return (idcode & 0xF8000000ul) >> 27; }

    static DWORD ceil_log2(DWORD data);
    static void memset64(BYTE * dest, QWORD value);

    AJI_OPEN_ID m_hub_id;

    AJI_CLAIM2 m_claims[4];

    AJI_CHAIN_ID m_chain;

    DWORD m_tap_position;

    DWORD     m_use_count;

    DWORD * m_idcodes;
    DWORD   m_idcode_n;

    AJI_HUB * m_parent_hub;
    std::map <DWORD, AJI_HUB *> m_child_hubs;

    DWORD m_hub_idcode;

    DWORD m_sel_bits;
};

#endif
