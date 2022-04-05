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

AJI_HIER_ID *workspace = (AJI_HIER_ID*) calloc(n, sizeof(AJI_HIER_ID));

    if (error == AJI_NO_ERROR)
    {
        // Check the size allocated by caller
        if (*hier_id_n < n)
            error = AJI_TOO_MANY_DEVICES;

        // Fill up the AJI_HIER_ID data structures
        if (error == AJI_NO_ERROR)
            error = hub->get_hier_ids(0, workspace, *hier_id_n, hub_infos);
    }

    // Hint the caller on how much buffer was used
    *hier_id_n = n;

    hub->put_hub();
printf(">>>> ************************************ aji_get_nodes built: %s %s\n", __DATE__, __TIME__);
for(DWORD i=0; i < *hier_id_n; ++i) {
  printf("workspace[%i]: id_code=0x%08lX position_n=%u, positions=[ ",
		  i, workspace[i].idcode, workspace[i].position_n
  );
  for(DWORD j=0; j<AJI_MAX_HIERARCHICAL_HUB_DEPTH; ++j) {
	printf("%d ", workspace[i].positions[j]);
  };
  printf("]; size=%d, pointers=%p %p [%p %p %p %p %p %p %p %p]\n",
	sizeof(workspace[i]), &(workspace[i].idcode), &(workspace[i].position_n),
    workspace[i].positions+0,workspace[i].positions+1,workspace[i].positions+2,
    workspace[i].positions+3,workspace[i].positions+4,workspace[i].positions+5,
    workspace[i].positions+6,workspace[i].positions+7
  );
}

printf("---\n");
for(DWORD i=0; i < *hier_id_n; ++i) {
   hier_ids[i] = workspace[i];
}
for(DWORD i=0; i < *hier_id_n; ++i) {
  printf("hier_id[%i]: id_code=0x%08lX position_n=%u, positions=[ ",
		  i, hier_ids[i].idcode, hier_ids[i].position_n
  );
  for(DWORD j=0; j<AJI_MAX_HIERARCHICAL_HUB_DEPTH; ++j) {
	printf("%d ", hier_ids[i].positions[j]);
  };
  printf("]; size=%d, pointers=%p %p [%p %p %p %p %p %p %p %p]\n",
	sizeof(hier_ids[i]), &(hier_ids[i].idcode), &(hier_ids[i].position_n),
    hier_ids[i].positions+0,hier_ids[i].positions+1,hier_ids[i].positions+2,
    hier_ids[i].positions+3,hier_ids[i].positions+4,hier_ids[i].positions+5,
    hier_ids[i].positions+6,hier_ids[i].positions+7
  );
}
printf("<<<<************************************ end aji_get_nodes\n");
free(workspace);
    return error;
}

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
