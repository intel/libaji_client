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

#ifndef INC_JTAG_CLIENT_HUB_H
#include "jtag_client_hub.h"
#endif

#define AJI_API JTAG_DLLEXPORT

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

struct POSINDEX
{
    DWORD       m_pos;
    DWORD       m_index;
    AJI_OPEN_ID m_id;
};

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_get_nodes       (AJI_CHAIN_ID         chain_id,
                                       DWORD                tap_position,
                                       DWORD              * idcodes,
                                       DWORD              * idcode_n)
//
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    return aji_get_nodes(chain_id, tap_position, idcodes, idcode_n, NULL);
};

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_get_nodes       (AJI_CHAIN_ID         chain_id,
                                       DWORD                tap_position,
                                       DWORD              * idcodes,
                                       DWORD              * idcode_n,
                                       DWORD              * hub_info)
//
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    AJI_HUB * hub = AJI_HUB::get_hub(chain_id, tap_position);

    AJI_ERROR error = hub->read_hub_information(false);
    DWORD n = 0;

    if (error == AJI_NO_ERROR)
    {
        n = hub->get_idcode_n();

        if (n <= *idcode_n)
            memcpy_s(idcodes, *idcode_n * sizeof(DWORD), hub->get_idcodes(), n * sizeof(DWORD));
        else
            error = AJI_TOO_MANY_DEVICES;
    }

    *idcode_n = n;

    if (hub_info != NULL)
        *hub_info = hub->get_hub_idcode();

    hub->put_hub();

    return error;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_get_nodes       (AJI_CHAIN_ID         chain_id,
                                       DWORD                tap_position,
                                       AJI_HIER_ID        * hier_ids,
                                       DWORD              * hier_id_n,
                                       AJI_HUB_INFO       * hub_infos)
//
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    if (hier_ids == NULL || hier_id_n == NULL)
        return AJI_INVALID_PARAMETER;

    AJI_HUB * hub = AJI_HUB::get_hub(chain_id, tap_position);

    AJI_ERROR error = hub->read_hub_information(true);

    // Retrieve the total number of nodes discovered
    DWORD n = hub->get_hier_id_n();

    if (error == AJI_NO_ERROR)
    {
        // Check the size allocated by caller
        if (*hier_id_n < n)
            error = AJI_TOO_MANY_DEVICES;

        // Fill up the AJI_HIER_ID data structures
        if (error == AJI_NO_ERROR)
            error = hub->get_hier_ids(0, hier_ids, *hier_id_n, hub_infos);
    }

    // Hint the caller on how much buffer was used
    *hier_id_n = n;

    hub->put_hub();

    return error;
}
/*
//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_open_node       (AJI_CHAIN_ID         chain_id,
                                       DWORD                tap_position,
                                       DWORD                idcode,
                                       AJI_OPEN_ID        * node_id,
                                       const AJI_CLAIM    * claims,
                                       DWORD                claim_n,
                                       const char         * application_name)
//
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    return aji_open_node(chain_id, tap_position, ~0u, idcode, node_id, claims, claim_n, application_name);
}
*/ /*
//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_open_node       (AJI_CHAIN_ID         chain_id,
                                       DWORD                tap_position,
                                       DWORD                node_position,
                                       DWORD                idcode,
                                       AJI_OPEN_ID        * node_id,
                                       const AJI_CLAIM    * claims,
                                       DWORD                claim_n,
                                       const char         * application_name)
//
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    AJI_HUB * hub = AJI_HUB::get_hub(chain_id, tap_position);
    AJI_ERROR error = hub->read_hub_information(false);

    if (error != AJI_NO_ERROR)
        return error;

    AJI_CLAIM2 * claims2  = AJI_OPEN::create_claims(claims, claim_n);

    error = AJI_FAILURE;

    for (DWORD i = 0 ; i < hub->get_idcode_n() ; i++)
        if (hub->get_idcodes()[i] == idcode && (node_position == ~0u || node_position == i))
        {
            error = hub->open_node(node_id, i, claims2, claim_n, application_name);
            break;
        }

    hub->put_hub();

    delete[] claims2;

    return error;
}
*/ /*
//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_open_node       (AJI_CHAIN_ID         chain_id,
                                       DWORD                tap_position,
                                       const AJI_HIER_ID  * hier_id,
                                       AJI_OPEN_ID        * node_id,
                                       const AJI_CLAIM2   * claims,
                                       DWORD                claim_n,
                                       const char         * application_name)
//
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    if (chain_id == NULL ||
        hier_id == NULL ||
        (claim_n > 0 && claims == NULL))
        return AJI_INVALID_PARAMETER;

    AJI_HUB * top_hub = AJI_HUB::get_hub(chain_id, tap_position);

    AJI_ERROR error = top_hub->read_hub_information(true);

    if (error != AJI_NO_ERROR)
        return error;

    error = AJI_NO_MATCHING_NODES;

    bool matched = true;
    AJI_HUB * hub = top_hub;

    // Iterate every hierarchy level
    for (BYTE level = 0 ; matched && (level <= hier_id->position_n) ; level++)
    {
        matched = false;
        // If this is the final level, find the node
        if (level == hier_id->position_n)
        {
            // Target the matching node in this hierarchy level
            for (DWORD i = 0 ; i < hub->get_node_count() ; i++)
            {
                if (hub->get_idcodes()[i] == hier_id->idcode &&
                        (hier_id->positions[level] == ((BYTE)0xFF) || hier_id->positions[level] == i))
                {
                    error = hub->open_node(node_id, hier_id, claims, claim_n, application_name);
                    break;
                }
            }
        }
        // Else find the matching bridge and go down one level
        else
        {
            for (DWORD i = 0 ; i < hub->get_node_count() ; i++)
            {
                if (hier_id->positions[level] == ~0 || hier_id->positions[level] == i)
                {
                    matched = true;
                    hub = hub->get_child_hub(i);
                    if (hub == NULL)
                        return AJI_INTERNAL_ERROR;
                    break;
                }
            }
        }
    }

    top_hub->put_hub();

    return error;
}
*/
//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_open_hub (AJI_CHAIN_ID         chain_id,
                                DWORD                tap_position,
                                DWORD                hub_level,
                                const AJI_HIER_ID  * hier_id,
                                AJI_OPEN_ID        * hub_id,
                                const AJI_CLAIM2   * claims,
                                DWORD                claim_n,
                                const char         * application_name)
//
// Description: Similar to aji_open_node(), but targeting the node's hub instead
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    if (chain_id == NULL ||
        hier_id == NULL ||
        hub_level > hier_id->position_n ||
        (claim_n > 0 && claims == NULL))
        return AJI_INVALID_PARAMETER;

    AJI_HUB * top_hub = AJI_HUB::get_hub(chain_id, tap_position);

    AJI_ERROR error = top_hub->read_hub_information(true);

    if (error != AJI_NO_ERROR)
        return error;

    error = AJI_NO_MATCHING_NODES;

    bool matched = true;
    AJI_HUB * hub = top_hub;

    // Iterate every hierarchy level
    for (BYTE level = 0 ; matched && (level <= hub_level) ; level++)
    {
        matched = false;
        // If this is the final level, open the hub
        if (level == hub_level)
        {
            AJI_HIER_ID hub_hier_id = { 0, level, 0};
            for (int i = 0; i < level; i++)
                hub_hier_id.positions[i] = hier_id->positions[i];
            hub_hier_id.positions[level] = (BYTE) 0xff;
            error = hub->open_node(hub_id, &hub_hier_id, claims, claim_n, application_name);
            break;
        }
        // Else find the matching bridge and go down one level
        else
        {
            for (DWORD i = 0 ; i < hub->get_node_count() ; i++)
            {
                if (hier_id->positions[level] == i)
                {
                    matched = true;
                    hub = hub->get_child_hub(i);
                    if (hub == NULL)
                        return AJI_INTERNAL_ERROR;
                    break;
                }
            }
        }
    }

    // Release top hub to prevent memory leak, this will also release its child hubs
    top_hub->put_hub();

    return error;

}

/*
//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_find_node(AJI_CHAIN_ID chain_id, int device_index, int instance, DWORD type, 
                        const AJI_CLAIM * claims, DWORD claim_n, const char * application_name,
                        AJI_OPEN_ID * node_id, DWORD * node_n)
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    AJI_CLAIM2 * claims2  = AJI_OPEN::create_claims(claims, claim_n);
    AJI_ERROR result = aji_find_node(chain_id, device_index, instance, type, claims2, claim_n, application_name, node_id, node_n);
    delete[] claims2;

    return result;
}
*/ /*
//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR AJI_API aji_find_node(AJI_CHAIN_ID chain_id, int device_index, int instance, DWORD type,
                        const AJI_CLAIM2 * claims, DWORD claim_n, const char * application_name,
                        AJI_OPEN_ID * node_id, DWORD * node_n)
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    if (claims == NULL || claim_n == 0 || application_name == NULL ||
        node_id == NULL || node_n == NULL || *node_n == 0)
        return AJI_INVALID_PARAMETER;

    DWORD i, j;

    // Read the device list
    DWORD device_count = 0;
    AJI_DEVICE * device = NULL;

    AJI_ERROR error = aji_read_device_chain(chain_id, &device_count, NULL, true);
    if (error == AJI_TOO_MANY_DEVICES)
    {
        device = new AJI_DEVICE[device_count];
        if (device == NULL)
            error = AJI_NO_MEMORY;
        else
            error = aji_read_device_chain(chain_id, &device_count, device, true);
    }

    AJI_HUB * * hub = NULL;
    if (error == AJI_NO_ERROR)
    {
        hub = new AJI_HUB * [device_count];
        if (hub == NULL)
            error = AJI_NO_MEMORY;
    }

    // Create a hub on all appropriate devices and enumerate it
    DWORD hub_n = 0;
    for (i = 0 ; error == AJI_NO_ERROR && i < device_count ; i++)
    {
        hub[i] = NULL;

        if (device_index >= 0 && static_cast<DWORD>(device_index) != i)
            continue;

        // Create a hub (if appropriate) and read the idcodes
        hub[i] = AJI_HUB::get_hub(chain_id, i);
        if (hub[i] == NULL)
            continue; // with error == AJI_NO_MEMORY

        if (hub[i]->read_hub_information(false) != AJI_NO_ERROR)
        {
            hub[i]->put_hub();
            hub[i] = NULL;
            continue;
        }

        hub_n++;
    }

    if (error == AJI_NO_ERROR)
    {
        if (hub_n == 0)
            error = AJI_NO_HUBS;

        //// Can't specify instance if there is more than one hub
        //if (hub_n > 1 && instance >= 0)
        //  error = AJI_TOO_MANY_HUBS;
    }

    // Ignore version and instance passed in using the `type` parameter.  We
    // open all versions of the device.  The instances we open are controlled
    // by the instance parameter passed in.
    type <<= 8;
    type &= 0x07FFFFF00;

    DWORD space = *node_n;

    // Count the number of suitable nodes which are present.  On the first pass
    // we count them so we can allocate memory, on the second pass record where
    // they are.
    POSINDEX * match = NULL;
    DWORD match_n = 0;
    for (DWORD k = 0 ; error == AJI_NO_ERROR && k < 2 ; k++)
    {
        match_n = 0;
        for (i = 0 ; i < device_count ; i++)
            if (hub[i] != NULL)
            {
                DWORD idcode_n = hub[i]->get_idcode_n();
                const DWORD * idcodes = hub[i]->get_idcodes();

                for (j = 0 ; j < idcode_n ; j++)
                    if (instance < 0 || static_cast<DWORD>(instance) == (idcodes[j] & 0xFF))
                        if ((idcodes[j] & 0x07FFFF00) == type)
                        {
                            if (match != NULL)
                            {
                                match[match_n].m_pos = i;
                                match[match_n].m_index = j;
                            }
                            match_n++;
                        }
            }

        *node_n = match_n;

        if (k == 0)
        {
            // If there is no space for matches then the user must choose
            if (match_n == 0)
                error = AJI_NO_MATCHING_NODES;
            else if (match_n > space)
                error = AJI_TOO_MANY_MATCHING_NODES;
            else
            {
                match = new POSINDEX[match_n];
                if (match == NULL)
                    error = AJI_NO_MEMORY;
            }
        }
    }

    // Create all the nodes.
    if (error == AJI_NO_ERROR)
    {
        for (i = 0 ; error == AJI_NO_ERROR && i < match_n ; i++)
            error = hub[match[i].m_pos]->open_node(&match[i].m_id, match[i].m_index, claims, claim_n, application_name);

        // If any open failed then cancel the ones which succeeded
        if (error != AJI_NO_ERROR)
            for (j = 0 ; j < i-1 ; j++)
                aji_close_device(match[i].m_id);

        if (error == AJI_NO_ERROR)
            for (i = 0 ; i < match_n ; i++)
                node_id[i] = match[i].m_id;
    }

    // Free our connections to the hubs
    if (hub != NULL)
        for (i = 0 ; i < device_count ; i++)
            if (hub[i] != NULL)
                hub[i]->put_hub();

    delete[] match;
    delete[] hub;
    delete[] device;

    return error;
}
*/