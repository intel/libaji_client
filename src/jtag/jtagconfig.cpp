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

//# START_MODULE_HEADER/////////////////////////////////////////////////////////
//#
//# Description:
//#
//# Authors:     Andrew Draper
//#
//#              Copyright (c) Altera Corporation 2000 - 2003
//#              All rights reserved.
//#
//# END_MODULE_HEADER///////////////////////////////////////////////////////////

//# START_ALGORITHM_HEADER//////////////////////////////////////////////////
//#
//#
//# END_ALGORITHM_HEADER////////////////////////////////////////////////////
//#

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif 

#include "aji_sys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <set>

#ifndef INC_AJI_H
#include "aji.h"
#endif
#include "ver_sys.h"
#include "gen_string_sys.h"

#include "jtag_common.h"
#include "jtag_platform.h"

void help(FILE * file);
void extrahelp(FILE * file);

int extended_atoi(const char * text);
int enumerate(const char * cable, bool noscan, bool listnodes, bool extrainfo, bool debug);
int load_devices(const char * filename);
int define(const char * id, const char * name, int irlen);
int undefine(const char * id, const char * name);
int enum_defined(void);
int set_param(const char * cable, const char * name, int value);
int get_param(const char * cable, const char * name);
int add(const char * hardware, const char * port, const char * name);
int remove_chain(const char * cable);
int add_server(const char * server, const char * password);
int set_password(const char * password);
#if PORT == WINDOWS
int serverinfo(void);
 #endif

AJI_ERROR get_hardware(DWORD * count, AJI_HARDWARE * * hw, char *** server_version_info, const char * cable);
const char * describe_node(char * buffer, size_t buffer_size, AJI_HIER_ID * hier_id);
AJI_ERROR print_design_hash(AJI_CHAIN_ID chain, int pos, int hub_level, const AJI_HIER_ID * hier_id);
void print_debug_info(AJI_CHAIN_ID chain, const char * param_name, const char * name);
const char * decode_error(AJI_ERROR error);
void enumerate_output(void * handle, DWORD level, const char * line);

//START_GLOBALS////////////////////////////////////////////////////////////////

static const QWORD JTAG_USERCODE = 0x7ll;
static const QWORD JTAG_USR0     = 0xCll;
static const QWORD JTAG_USR1     = 0xEll;

static const QWORD HUB_MINORVERSION = 0x4ll;
static const QWORD HUB_IDENT        = 0x5ll;

// Claim shared access to the (read only) usercode register and the ident registers
static const AJI_CLAIM2 IDENT_CLAIMS[] =
{
    { AJI_CLAIM_IR_SHARED,          0, JTAG_USERCODE },
    { AJI_CLAIM_IR_SHARED_OVERLAY,  0, JTAG_USR1 },
    { AJI_CLAIM_IR_SHARED_OVERLAID, 0, JTAG_USR0 },
    { AJI_CLAIM_OVERLAY_SHARED,     0, HUB_MINORVERSION },
    { AJI_CLAIM_OVERLAY_SHARED,     0, HUB_IDENT }
};

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
/**
 * \note Command line Options
 *       that are commented out are not supported officially.
 *       Their corresponding functions and enums are also commented out.
 *       You should still be able to invoke them. We do not provide 
 *       any assurance that they work
 */

extern "C" int main(int argc, char* argv[])
{

    enum {    ENUM,  LISTHARDWARE, HELP, SYNTAXERROR, EXTRAHELP, ADD, REMOVE,
        DEFINE, UNDEFINE, ENUMDEFINED, 
        ADDSERVER,
        SET_PARAM, GET_PARAM,
        ENABLEREMOTE, DISABLEREMOTE,  SERVERINFO
    } action = ENUM;
    bool debug = false;
    bool tcl = false; // Format output for TCL
    bool extrainfo = false;
    bool listnodes = false;
    const char * cable = NULL;
    
    for ( ; ; )
        if (argc >= 3 && (stricmp(argv[1], "--cable") == 0 || stricmp(argv[1], "-c") == 0))
        {
            cable = argv[2];
            argc -= 2;
            argv += 2;
        }
        else if (argc >= 2 && (stricmp(argv[1], "--debug") == 0 || stricmp(argv[1], "-d") == 0))
        {
            debug = true;
            argc--;
            argv++;
        }
        else if (argc >= 2 && (stricmp(argv[1], "--extra-info") == 0 || stricmp(argv[1], "-x") == 0))
        {
            extrainfo = true;
            argc--;
            argv++;
        }
        else if (argc >= 2 && (stricmp(argv[1], "--nodes") == 0 || stricmp(argv[1], "-n") == 0))
        {
            listnodes = true;
            argc--;
            argv++;
        }
        else if (argc >= 2 && (stricmp(argv[1], "--tcl") == 0 || stricmp(argv[1], "-t") == 0))
        {
            tcl = true;
            argc--;
            argv++;
        }
        else
            break;

    if (argc == 2 && stricmp(argv[1], "--listhardware") == 0) // --listhardware is the same as --enum
        action = LISTHARDWARE;                                
    else if (argc >= 4 && stricmp(argv[1], "--add") == 0)
        action = ADD;
    else if (argc >= 3 && stricmp(argv[1], "--remove") == 0)
        action = REMOVE;
    else if (argc >= 2 && (stricmp(argv[1], "--help") == 0 || stricmp(argv[1], "-h") == 0 || stricmp(argv[1], "-?") == 0))
        action = HELP;
    else if (argc >= 2 && stricmp(argv[1], "--extrahelp") == 0)
        action = EXTRAHELP;
    else if (argc >= 2 && (stricmp(argv[1], "--version") == 0 || stricmp(argv[1], "-v") == 0))
    {
        printf("%s\n", ver_get_full_version().c_str());
        printf("%s\n", ver_get_copyright_string().c_str());
        return 0;
    }
    else if (argc >= 5 && stricmp(argv[1], "--define") == 0)
        action = DEFINE;
    else if (argc >= 4 && stricmp(argv[1], "--undefine") == 0)
        action = UNDEFINE;
    else if (argc >= 2 && stricmp(argv[1], "--defined") == 0)
        action = ENUMDEFINED;
    else if (argc >= 5 && stricmp(argv[1], "--setparam") == 0)
        action = SET_PARAM;
    else if (argc >= 4 && stricmp(argv[1], "--getparam") == 0)
        action = GET_PARAM;
    else if (argc >= 4 && stricmp(argv[1], "--addserver") == 0)
        action = ADDSERVER;
    else if (argc >= 3 && stricmp(argv[1], "--enableremote") == 0)
        action = ENABLEREMOTE;
    else if (argc >= 2 && stricmp(argv[1], "--disableremote") == 0)
        action = DISABLEREMOTE;
#if PORT == WINDOWS
    else if (argc >= 2 && stricmp(argv[1], "--serverinfo") == 0)
        action = SERVERINFO;
#endif
    else if (argc == 1 || stricmp(argv[1], "--enum") == 0)
        action = ENUM;
    else
        action = SYNTAXERROR;

    switch (action)
    {
    case ENUM:
    case LISTHARDWARE:
        return enumerate(cable, action == LISTHARDWARE, listnodes, extrainfo, debug);
    case ADD:
        return add(argv[2], argv[3], (argc >= 5) ? argv[4] : NULL);
    case REMOVE:
        return remove_chain(argv[2]);
    case DEFINE:
        return define(argv[2], argv[3], atoi(argv[4]));
    case UNDEFINE:
        return undefine(argv[2], argv[3]);
    case ENUMDEFINED:
        return enum_defined();
    case SET_PARAM:
        return set_param(argv[2], argv[3], extended_atoi(argv[4]));
    case GET_PARAM:
        return get_param(argv[2], argv[3]);
    case ADDSERVER:
        return add_server(argv[2], argv[3]);
    case ENABLEREMOTE:
        return set_password(argv[2]);
    case DISABLEREMOTE:
        return set_password(NULL);
#if PORT == WINDOWS
    case SERVERINFO:
        return serverinfo();
#endif
    case HELP:
        help(stdout);
        return 0;
    case EXTRAHELP:
        extrahelp(stdout);
        return 0;
    case SYNTAXERROR:
    default:
        help(stderr);
        return 16;
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void help(FILE * file)
{
    fprintf(file, "JTAG Server Configuration\n\n");

    fprintf(file, "Usage: jtagconfig [--enum]\n");
    fprintf(file, "       jtagconfig --add <type> <port> [<name>]\n");
    fprintf(file, "       jtagconfig --remove <cable>\n");
    fprintf(file, "       jtagconfig --getparam <cable> <param>\n");
    fprintf(file, "       jtagconfig --setparam <cable> <param> <value>\n");
    fprintf(file, "       jtagconfig --define <jtagid> <name> <irlength>\n");
    fprintf(file, "       jtagconfig --undefine <jtagid> <name> <irlength>\n");
    fprintf(file, "       jtagconfig --defined\n");
    fprintf(file, "       jtagconfig --addserver <server> <password>\n");
    fprintf(file, "       jtagconfig --enableremote <password>\n");
    fprintf(file, "       jtagconfig --disableremote\n");
    fprintf(file, "       jtagconfig --version\n");
#if PORT == WINDOWS
    fprintf(file, "       jtagconfig --serverinfo\n");
#endif
    fprintf(file, "       jtagconfig --help\n");
    fprintf(file, "       jtagconfig --extrahelp\n\n");
    fprintf(file, "For more details use jtagconfig --extrahelp\n\n");
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void extrahelp(FILE * file)
{
    fprintf(file, "Usage:\n");
    fprintf(file, "------\n\n");

    fprintf(file, "jtagconfig [--enum]\n");
    fprintf(file, "jtagconfig --add <hardware> <port> [<name>]\n");
    fprintf(file, "jtagconfig --remove <cable>\n");
    fprintf(file, "jtagconfig --define <jtagid> <name> <irlength>\n");
    fprintf(file, "jtagconfig --undefine <jtagid> <name> <irlength>\n");
    fprintf(file, "jtagconfig --defined\n");
    fprintf(file, "jtagconfig --addserver <server> <password>\n");
    fprintf(file, "jtagconfig --enableremote <password>\n");
    fprintf(file, "jtagconfig --disableremote\n");
    fprintf(file, "jtagconfig --version | -v\n");
#if PORT == WINDOWS
    fprintf(file, "jtagconfig --serverinfo\n");
#endif
    fprintf(file, "jtagconfig --help | -h\n");
    fprintf(file, "jtagconfig --extrahelp\n\n\n");


    fprintf(file, "Option:           Action :\n");
    fprintf(file, "-------           --------\n\n");

    fprintf(file, "--version | -v    Displays the version of the jtagconfig utility you are using.\n\n");

    fprintf(file, "--help | -h       Displays the options for the jtagconfig utility.\n\n");

    fprintf(file, "--extrahelp       Displays this help information.\n\n");

    fprintf(file, "--enum            Displays the JTAG ID code for the devices in each JTAG chain\n");
    fprintf(file, "                  attached to programming hardware. If the JTAG Server\n");
    fprintf(file, "                  recognizes a device, it displays the device name. If the\n");
    fprintf(file, "                  JTAG Server cannot use a device, it marks the device with an\n");
    fprintf(file, "                  exclamation mark (!).\n\n");
                  
    fprintf(file, "                  The following code is an example of output from the\n");
    fprintf(file, "                  'jtagconfig --enum' command:\n\n");

    fprintf(file, "                  1) ByteBlasterMV on LPT1\n");
    fprintf(file, "                  090010DD   EPXA10\n");
    fprintf(file, "                  049220DD   EPXA_ARM922\n");
    fprintf(file, "                  04000101 !\n");
    fprintf(file, "                  090000DD ! EP20K1000E\n");
    fprintf(file, "                  04056101 !\n");
    fprintf(file, "                  010020DD   EPC2\n");
    fprintf(file, "                  010020DD   EPC2\n");
    fprintf(file, "                  010020DD   EPC2\n\n");

    fprintf(file, "                  In this example, the JTAG Server does not recognize two of\n");
    fprintf(file, "                  the eight devices, and cannot use three devices.  If you need\n");
    fprintf(file, "                  to use one of the unused devices then use the --define option\n");
    fprintf(file, "                  to define unused devices with JTAG IDs.\n\n");

    fprintf(file, "                  You can also use this option by typing the command\n");
    fprintf(file, "                  'jtagconfig' without an argument.\n\n");

    fprintf(file, "--add <hardware>  Specifies the port to which you attached new programming\n");
    fprintf(file, "  <port> [<name>] hardware. For example, 'jtagconfig --add byteblastermv lpt1'\n");
    fprintf(file, "                  specifies that you attached a ByteBlasterMV cable to port\n");
    fprintf(file, "                  LPT1. You can also use '[<name>]' to add a string that\n");
    fprintf(file, "                  describes the hardware to the output from the\n");
    fprintf(file, "                  'jtagconfig --enum' command.\n\n");

    fprintf(file, "                  The JTAG Server automatically detects cables that you attach\n");
    fprintf(file, "                  to a USB port, therefore you do not need to use this command\n");
    fprintf(file, "                  for these cables.\n\n");

    fprintf(file, "--remove <cable>  Removes the hardware setup that is indicated with the number\n");
    fprintf(file, "                  listed for the hardware in the output of the '--enum'\n");
    fprintf(file, "                  command. In the example for the '--enum' command, above, the\n");
    fprintf(file, "                  number '1)' is listed for the ByteBlasterMV cable.\n");
    fprintf(file, "                  Therefore, the command-line 'jtagconfig --remove 1' removes\n");
    fprintf(file, "                  the hardware setup for the ByteBlasterMV cable.\n\n");

    fprintf(file, "                  If the hardware specified is attached to a remote JTAG server\n");
    fprintf(file, "                  then the connection to the remote JTAG server will be removed\n");
    fprintf(file, "                  instead.\n\n");

    fprintf(file, "--getparam        Get a hardware specific parameter from the cable specified.\n");
    fprintf(file, "<cable> <param>   The parameters supported vary by cable type, see the\n");
    fprintf(file, "                  documentation for your cable for details.\n\n");

    fprintf(file, "--setparam        Set a hardware specific parameter on the cable specified.\n");
    fprintf(file, "<cable> <param>   The parameters supported vary by cable type, see the\n");
    fprintf(file, "<value>           documentation for your cable for details.\n");
    fprintf(file, "                  This command can only set numeric parameters (the suffixes\n");
    fprintf(file, "                  k and M are recognised when parsing the value).\n\n");

    fprintf(file, "--define <jtagid> Tells the JTAG server about the name and IR length of a new\n");
    fprintf(file, "<name> <irlength> device.  This will be stored and used in future enumerations\n");
    fprintf(file, "                  to reduce the number of devices which cannot be used\n\n");

    fprintf(file, "--undefine        Tells the JTAG server to remove information about the name\n");
    fprintf(file, "<jtagid> <name>   and IR length of a device.\n\n");

    fprintf(file, "--defined         Displays the JTAG IDs, names and IR lengths of all non-Altera\n");
    fprintf(file, "                  devices known about by the JTAG server.\n\n");

    fprintf(file, "--addserver       Tells Quartus to connect to the remote JTAG server and make\n");
    fprintf(file, "<servername>      all cables on that server available to local applications.\n");
    fprintf(file, "<password>\n\n");

    fprintf(file, "                  An IP address or DNS name may be used to specify the server\n");
    fprintf(file, "                  to connect to.  The password given here must match the\n");
    fprintf(file, "                  password used on the remote server.\n\n");

    fprintf(file, "--enableremote    Tells the JTAG server to allow connections from remote\n");
    fprintf(file, "<password>        machines.  These machines must specify the same password\n");
    fprintf(file, "                  when connecting.\n\n");

    fprintf(file, "--disableremote   Tells the JTAG server not to accept any more connections\n");
    fprintf(file, "                  from remote machines.  Remote connections currently in use\n");
    fprintf(file, "                  are not terminated.\n\n");

#if PORT == WINDOWS
    fprintf(file, "--serverinfo      Displays information about whether the JTAG server is\n");
    fprintf(file, "                  installed, whether it is running etc.\n\n");
#endif
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
// 
int extended_atoi(const char * text)
{
    int value = 0;

    while (*text != 0 && isdigit(*text))
        value = (value * 10) + *text++ - '0';

    if (toupper(*text) == 'K')
        value *= 1000;
    else if (toupper(*text) == 'M')
        value *= 1000000;

    return value;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//

int enumerate(const char * cable, bool noscan, bool listnodes, bool extrainfo, bool debug)
{
    aji_register_output_callback(&enumerate_output, NULL);

    // Enumerate all the chains
    DWORD hw_count = 0;
    AJI_HARDWARE * hw = NULL;
    char * * server_version_info = NULL;
    char buffer[512];
    const char *jtag_client_config = getenv("QUARTUS_JTAG_CLIENT_CONFIG");

    if (jtag_client_config != 0)
    {
        printf("[ Environment variable QUARTUS_JTAG_CLIENT_CONFIG=%s is in effect ]\n", jtag_client_config);
    }

    AJI_ERROR error = get_hardware(&hw_count, &hw, &server_version_info, cable);
    if (error != AJI_NO_ERROR && error != AJI_TIMEOUT)
    {
        fprintf(stderr, "Error when scanning hardware - %s\n", decode_error(error));
        delete[] hw;
        if (server_version_info != NULL)
            for (DWORD i = 0; i < hw_count; i++)
                delete[] server_version_info[i];
        delete[] server_version_info;
        return 0;
    }

    if (hw_count == 0)
    {
        fprintf(stderr, "No JTAG hardware available\n");
        delete[] hw;
        if (server_version_info != NULL)
            for (DWORD i = 0; i < hw_count; i++)
                delete[] server_version_info[i];
        delete[] server_version_info;
        return 4;
    }

    for (DWORD i = 0 ; i < hw_count ; i++)
    {
        if (hw[i].hw_name == NULL)
        {
            // This is a dummy entry which exists to allow users to remove the
            // remote server.
            printf("%lu) Remote server %s", (unsigned long) i+1, hw[i].server);
            if (hw[i].port != NULL)
                printf(": %s", hw[i].port); // Reason for lack of connection
            printf("\n%s", noscan ? "" : "\n");
            if (debug)
                printf("   (JTAG Server %s)\n", server_version_info[i]);
            continue;
        }

        buffer[0] = 0;
        aji_print_hardware_name(hw[i].chain_id, buffer, sizeof(buffer));

        printf("%lu) %s\n", (unsigned long)i+1, buffer);
        if (debug)
            printf("   (JTAG Server %s)\n", server_version_info[i]);

        if (noscan)
            continue;

        // Lock and scan chain
        AJI_CHAIN_ID chain = hw[i].chain_id;

        AJI_ERROR error = aji_lock_chain(chain, 10000);
        if (error != AJI_NO_ERROR)
        {
            printf("  Unable to lock chain - %s\n", decode_error(error));
            continue;
        }

        DWORD needupdate;
        if (aji_get_parameter(chain, "KernelUpdateRequired", &needupdate) == AJI_NO_ERROR &&
            needupdate != 0)
        {
            printf("  (Update kernel driver for higher performance)\n");
        }

        // Read the default JTAG clock before the aji_read_device_chain
        // Calling aji_read_device_chain may trigger the JTAG clock auto-adjustment
        // We need the default JTAG clock to know whether JTAG clock has changed
        DWORD default_jtag_clock = 0;
        aji_get_parameter(chain, "JtagClock", &default_jtag_clock);

        DWORD device_count = 0;
        AJI_DEVICE * device = NULL;

        error = aji_read_device_chain(chain, &device_count, NULL, true);
        if (error == AJI_TOO_MANY_DEVICES)
        {
            device = new AJI_DEVICE[device_count];
            if (device == NULL)
            {
                printf("  Out of memory - %s\n", decode_error(error));
                aji_unlock_chain(chain);
                continue;
            }
            error = aji_read_device_chain(chain, &device_count, device, true);
        }

        if (error != AJI_NO_ERROR)
        {
            printf("  Unable to read device chain - %s\n", decode_error(error));
            device_count = 0;
        }
        else if (device_count == 0)
            printf("  No devices on chain\n");

        bool all_usable = true;

        // Print all the devices on the chain.
        DWORD pos;
        for (pos = 0 ; pos < device_count ; pos++)
        {
            // Start with 16 nodes
            DWORD hier_id_n = 16;
            AJI_HIER_ID * hier_ids = new AJI_HIER_ID[hier_id_n];
            AJI_HUB_INFO * hub_infos = new AJI_HUB_INFO[hier_id_n];
            bool tooManyHierarchies = false;

            if (listnodes || debug || extrainfo)
            {
                error = aji_get_nodes(chain, pos, hier_ids, &hier_id_n, hub_infos);
                
                if (error == AJI_TOO_MANY_DEVICES)
                {
                    // Reallocate if there are more nodes
                    delete[] hier_ids;
                    delete[] hub_infos;
                    hier_ids = new AJI_HIER_ID[hier_id_n];
                    hub_infos = new AJI_HUB_INFO[hier_id_n];
                    error = aji_get_nodes(chain, pos, hier_ids, &hier_id_n, hub_infos);
                }

                if (error != AJI_NO_ERROR && error != AJI_TOO_MANY_HIERARCHIES)
                    hier_id_n = 0;

                if (error == AJI_TOO_MANY_HIERARCHIES)
                    tooManyHierarchies = true;
            }
            else
                hier_id_n = 0;

            printf("  %08lX %c %s", 
                   (unsigned long) device[pos].device_id,
                   device[pos].instruction_length > 0 ? ' ' : '!',
                   device[pos].device_name);

            if (device[pos].instruction_length == 0)
                all_usable = false;

            if (debug)
                printf(" (IR=%d)", device[pos].instruction_length);

            printf("\n");

            if (extrainfo)
            {
                for (DWORD i = 0 ; i < hier_id_n ; i++)
                {
                    printf("    {hub %08lX", (unsigned long) hub_infos[i].hub_idcode[0]);
                    for (int j = 1 ; j <= hier_ids[i].position_n ; j++)
                        printf(" bridge %08lX hub %08lX", 
                               (unsigned long) hub_infos[i].bridge_idcode[j], 
                               (unsigned long) hub_infos[i].hub_idcode[j]
                        );
                    printf(" node %08lX}\n", (unsigned long) hier_ids[i].idcode);
                }
            }
            else
            {
                if (hier_id_n > 0)
                {
                    bool hierHubNotSupported = false;

                    // Accessing design hash in top hub (level 0) will not return
                    // AJI_HIERARCHICAL_HUB_NOT_SUPPORTED so return value can be ignored
                    print_design_hash(chain, pos, 0, (&hier_ids[0]));

                    std::set<QWORD> bridge_id_set;
                    for (DWORD j = 0 ; j < hier_id_n ; j++)
                    {
                        if (hier_ids[j].position_n > 0)
                        {
                        	// Use all ones instead of all zeros because 0x0 is
                        	// a valid position while 0xFF is not
                            QWORD id = 0xFFFFFFFFFFFFFFFFull;
                            for (DWORD k = 0; k < hier_ids[j].position_n; k++)
                            {
                            	id ^= 0xFFLL << k*8;
                                id |= (QWORD)(hier_ids[j].positions[k]) << k*8;

                                if (bridge_id_set.find(id) == bridge_id_set.end())
                                {
                                    bridge_id_set.emplace(id);
                                    error = print_design_hash(chain, pos, k+1, &(hier_ids[j]));
                                    // Older version of jtagserver does not support hierarchical hub
                                    if (error == AJI_HIERARCHICAL_HUB_NOT_SUPPORTED)
                                        hierHubNotSupported = true;
                                }
                            }
                        }
                        // Up to 3 spaces * 8 hierarchies + 1 terminating null
                        // Initialize all elements with spaces
                        char spaces [] = "                        ";
                        spaces[3 * (hier_ids[j].position_n)] = '\0';

                        printf("   %s + Node %08lX  %s\n", spaces, (unsigned long) hier_ids[j].idcode, describe_node(buffer, sizeof(buffer), &(hier_ids[j])));
                    }

                    if (hierHubNotSupported)
                        printf("    Design contains hierarchical hub, please use latest version of JTAG Server\n");
                }
            }

            if (tooManyHierarchies)
                printf("    SLD nodes in hierarchy lower than 8th-level cannot be accessed\n");

            delete[] hier_ids;
            delete[] hub_infos;
        }

        if (debug || !all_usable)
        {
            // If the JTAG server wasn't able to work out all the IR lengths then
            // print the captured IR value to aid in debugging.
            printf("\n");

            print_debug_info(chain, "CapturedDR", "DR after reset");
            print_debug_info(chain, "CapturedIR", "IR after reset");
            print_debug_info(chain, "CapturedBypass", "Bypass after reset");
            print_debug_info(chain, "NormalBypass", "Bypass chain");
            DWORD jtag_clock_adjust = 0;
            error = aji_get_parameter(chain, "JtagClockAutoAdjust", &jtag_clock_adjust);
            if (error == AJI_NO_ERROR && jtag_clock_adjust != 0)
            {
                printf("  JTAG clock speed auto-adjustment is enabled. To disable, set JtagClockAutoAdjust parameter to 0\n");
            }

            DWORD jtag_clock;
            error = aji_get_parameter(chain, "JtagClock", &jtag_clock);
            if (error == AJI_NO_ERROR)
            {
                bool auto_adjusted = (jtag_clock_adjust && default_jtag_clock != 0 && default_jtag_clock != jtag_clock);
                const char * prefix = "";
                if (jtag_clock != 0)
                {
                    if ((jtag_clock % 1000000) == 0)
                    {
                        jtag_clock /= 1000000;
                        prefix = "M";
                    }
                    else if ((jtag_clock % 1000) == 0)
                    {
                        jtag_clock /= 1000;
                        prefix = "k";
                    }
                }
                printf("  JTAG clock speed %s%lu %sHz%s\n", 
                       auto_adjusted ? "auto-adjusted to " : "",  
                       (unsigned long) jtag_clock, 
                       prefix, 
                       auto_adjusted ? " due to failure in BYPASS test" : ""
                );
            }
        }

        delete[] device;
        aji_unlock_chain(chain);

        printf("\n");
    }

    delete[] hw;
    if (server_version_info != NULL)
        for (DWORD i = 0; i < hw_count; i++)
            delete[] server_version_info[i];
    delete[] server_version_info;
    return 0;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR get_hardware(DWORD * count, AJI_HARDWARE * * hw, char *** server_version_info, const char * cable)
{
    DWORD i;
    AJI_ERROR error = AJI_NO_ERROR;

    for (i = 0 ; i < 4*20 ; i++)
    {
        if (cable == NULL)
        {
            error = aji_get_hardware(count, *hw, 250);
            if (error == AJI_TOO_MANY_DEVICES)
            {
                *hw = new AJI_HARDWARE[*count];
                if (hw == NULL)
                    return AJI_NO_MEMORY;

                if (server_version_info == NULL)
                    error = aji_get_hardware(count, *hw, 0);
                else
                {
                    *server_version_info = new char * [*count];
                    if (server_version_info == NULL)
                        return AJI_NO_MEMORY;
                    error = aji_get_hardware2(count, *hw, *server_version_info, 0);
                }
            }
        }
        else
        {
            if (*hw == NULL)
                *hw = new AJI_HARDWARE[1];
            error = aji_find_hardware(cable, *hw, 250);

            if (error == AJI_NO_ERROR)
                *count = 1;
        }

        if (error != AJI_TIMEOUT)
            break;
        if (i == 2)
            fprintf(stderr, "Connecting to server(s) [.                   ]"
                                                     "\b\b\b\b\b\b\b\b\b\b"
                                                     "\b\b\b\b\b\b\b\b\b\b");
        else if ((i%4) == 2)
            fprintf(stderr, ".");
        fflush(stderr);
    }

    if (i >= 2)
        fprintf(stderr, "\r                                              \r");

    return error;
}

//START_STATIC_ARRAY///////////////////////////////////////////////////////////

const char * node_type_110_lo[] =
{
    /*   0 */ "Signal Tap",
    /*   1 */ NULL, // OCP
    /*   2 */ NULL, // "SignalTap HDL"
    /*   3 */ "ROM/RAM/Constant",
//    /*   4 */ "Serial Flash Loader",
    /*   5 */ "Parallel Flash Loader",
    /*   6 */ "DSP Builder HIL",
    /*   7 */ "Logic Analyzer Interface",
    /*   8 */ "Virtual JTAG",
    /*   9 */ "Source/Probe",
};

const char * node_type_110_hi[] =
{
    /* 128 */ "JTAG UART",
    /* 129 */ NULL, // Switch/LED
    /* 130 */ "JTAG Avalon",
    /* 131 */ "JTAG Avalon (debug)",
    /* 132 */ "JTAG PHY",
    /* 133 */ NULL, // Altmemphy debug
    /* 134 */ "Fast downloader",
    /* 135 */ "Nios II (internal)",
};

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
const char * describe_node(char * buffer, size_t buffer_size, AJI_HIER_ID * hier_id)
{
    const char * type_str = NULL;

    DWORD nodeid = hier_id->idcode;
    DWORD inst =  nodeid       &  0xFF;
    DWORD manu = (nodeid >> 8) & 0x7FF;
    DWORD type = (nodeid >> 19) & 0xFF;

    if (manu == 110)
    {
        if (type < sizeof(node_type_110_lo)/sizeof(node_type_110_lo[0]))
            type_str = node_type_110_lo[type];
        else if (type >= 128 && type < 128 + sizeof(node_type_110_hi)/sizeof(node_type_110_hi[0]))
            type_str = node_type_110_hi[type - 128];
        else if (type == 0xFF)
            type_str = "Bridge";
    }
    else if (manu == 70 && type == 34)
        type_str = "Nios II";

    if (type_str != NULL)
        snprintf(buffer, buffer_size, "%s #%lu", type_str, (unsigned long) inst);
    else
        snprintf(buffer, buffer_size, 
                 "(%lu:%lu) #%lu", 
                 (unsigned long) manu, 
                 (unsigned long) type, 
                 (unsigned long) inst
        );

    return buffer;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
AJI_ERROR print_design_hash(AJI_CHAIN_ID chain_id, int tap_pos, int hub_level, const AJI_HIER_ID * hier_id)
{
    // Up to 3 spaces * 8 hierarchies + 1 terminating null
    // Initialize all elements with spaces
    char spaces [] = "                        ";
    spaces[3 * hub_level] = '\0';

    DWORD claim_n = sizeof(IDENT_CLAIMS)/sizeof(IDENT_CLAIMS[0]);
    AJI_OPEN_ID hub_id;

    AJI_ERROR err = aji_open_hub(chain_id, tap_pos, hub_level, hier_id, &hub_id, IDENT_CLAIMS, claim_n, "jtagconfig");

    if (err == AJI_NO_ERROR)
    {
        // Pass lock from chain to node atomically
        err = aji_unlock_chain_lock(chain_id, hub_id, AJI_PACK_MANUAL);

        unsigned char minor_buff[1];
        unsigned char ident_buff[32];

        if (err == AJI_NO_ERROR)
            err = aji_access_overlay(hub_id, HUB_MINORVERSION, NULL);
        if (err == AJI_NO_ERROR)
            err = aji_access_dr(hub_id, 4, AJI_DR_UNUSED_X, 0,0,NULL, 0,4,minor_buff, 1);

        if (err == AJI_NO_ERROR)
            err = aji_access_overlay(hub_id, HUB_IDENT, NULL);
        if (err == AJI_NO_ERROR)
            err = aji_access_dr(hub_id, 4, AJI_DR_UNUSED_X, 0,0,NULL, 0,4,ident_buff, 32);

        // Pass lock from device to chain atomically, flush buffer
        aji_unlock_lock_chain(hub_id, chain_id);

        aji_close_device(hub_id);

        if (err != AJI_NO_ERROR)
            return err;

        if ((minor_buff[0] & 1) == 0)
            return err; // Old hardware, no ident register

        // ident_buff[0] is mixer
        // ident_buff[1..3] is instance (zero for jtag)
        // ident_buff[4..11] is overlay (not used yet)
        // ident_buff[12..31] is design hash

        // Print the design hash
        char buffer[32], *ptr = buffer;
        for (int i = 0 ; i < 20 ; i++)
            ptr += snprintf(ptr, sizeof(buffer), "%X", ident_buff[12+i] & 0xF);

        printf("   %s Design hash    %20s\n", spaces, buffer);
    }
    else
    {
        printf("   %s Design hash not accessible\n", spaces);
    }

    return err;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void print_debug_info(AJI_CHAIN_ID chain, const char * param_name, const char * name)
{
    char buffer[256];
    DWORD bufflen = sizeof(buffer);
    if (aji_get_parameter(chain, param_name, (BYTE *)buffer, &bufflen) == AJI_NO_ERROR)
    {
        buffer[bufflen] = 0;
        printf("  Captured %s = %s\n", name, buffer);
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int define(const char * id, const char * name, int irlen)
{
    DWORD jtagid = strtoul(id, NULL, 16);

    if (((jtagid & 1) == 0 && jtagid != 0) || (jtagid & 0xFFF) == 0x0FF)
    {
        fprintf(stderr, "Invalid JTAG ID (%08lX)\n", (unsigned long) jtagid);
        return 8;
    }

    if (irlen < 2 || irlen > 255)
    {
        fprintf(stderr, "Invalid instruction register length (must be >= 2 and <= 255)\n");
        return 8;
    }

    AJI_DEVICE device;
    memset_(&device, 0, sizeof(device));
    device.device_id = jtagid;
    device.device_name = name;
    device.instruction_length = static_cast<BYTE>(irlen);

    return aji_define_device(&device);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int undefine(const char * id, const char * name)
{
    DWORD jtagid = strtoul(id, NULL, 16);

    if (((jtagid & 1) == 0 && jtagid != 0) || (jtagid & 0xFFF) == 0x0FF)
    {
        fprintf(stderr, "Invalid JTAG ID (%08lX)\n", (unsigned long) jtagid);
        return 8;
    }

    AJI_DEVICE device;
    memset_(&device, 0, sizeof(device));
    device.device_id = jtagid;
    device.device_name = name;

    return aji_undefine_device(&device);
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int enum_defined(void)
{
    aji_register_output_callback(&enumerate_output, NULL);

    AJI_ERROR error;
    DWORD device_count = 0;
    AJI_DEVICE * device = NULL;

    error = aji_get_defined_devices(&device_count, NULL);
    if (error == AJI_TOO_MANY_DEVICES)
    {
        device = new AJI_DEVICE[device_count];
        if (device == NULL)
        {
            printf("  Out of memory - %s\n", decode_error(error));
            return 8;
        }
        error = aji_get_defined_devices(&device_count, device);
    }

    if (error != AJI_NO_ERROR)
    {
        printf("  Unable to read user defined devices - %s\n", decode_error(error));
        delete[] device;
        return 8;
    }

    if (device_count == 0)
        printf("  No user defined devices\n");

    // Print all the user defined devices
    DWORD pos;
    for (pos = 0 ; pos < device_count ; pos++)
    {
        printf("  jtagconfig --define \"%s\" %08lX %d\n",
                               device[pos].device_name,
               (unsigned long) device[pos].device_id,
                               device[pos].instruction_length);
    }

    delete[] device;

    return 0;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int set_param(const char * cable, const char * name, int value)
{
    AJI_HARDWARE hw;
    AJI_ERROR error = aji_find_hardware(cable, &hw, 6000);

    if (error != AJI_NO_ERROR)
    {
        if (error == AJI_FAILURE)
            fprintf(stderr, "Unable to find jtag cable \"%s\"\n", cable);
        else
            fprintf(stderr, "Error when scanning hardware - %s\n", decode_error(error));
        return 8;
    }

    AJI_CHAIN_ID chain = hw.chain_id;

    error = aji_lock_chain(chain, 10000);
    if (error != AJI_NO_ERROR)
    {
        fprintf(stderr, "Error when locking chain - %s\n", decode_error(error));
        return 4;
    }

    error = aji_set_parameter(hw.chain_id, name, value);
    if (error != AJI_NO_ERROR)
    {
        if (error == AJI_UNIMPLEMENTED)
            fprintf(stderr, "No parameter named %s\n", name);
        else
            fprintf(stderr, "Error when setting parameter - %s\n", decode_error(error));
    }
    else
    {
        if(stricmp(name, "JtagClockAutoAdjust") == 0 && value == 0)
        {
            printf("JTAG clock speed auto-adjustment is disabled but will be enabled by default when the cable is power-cycled or re-plugged in\n");
        }
    }

    aji_unlock_chain(chain);

    return (error == AJI_NO_ERROR) ? 0 : 4;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int get_param(const char * cable, const char * name)
{
    AJI_HARDWARE hw;
    AJI_ERROR error = aji_find_hardware(cable, &hw, 6000);

    if (error != AJI_NO_ERROR)
    {
        if (error == AJI_FAILURE)
            fprintf(stderr, "Unable to find jtag cable \"%s\"\n", cable);
        else
            fprintf(stderr, "Error when scanning hardware - %s\n", decode_error(error));
        return 8;
    }

    AJI_CHAIN_ID chain = hw.chain_id;

    error = aji_lock_chain(chain, 10000);
    if (error != AJI_NO_ERROR)
    {
        fprintf(stderr, "Error when locking chain - %s\n", decode_error(error));
        return 4;
    }

    DWORD value;
    error = aji_get_parameter(chain, name, &value);
    if (error == AJI_NO_ERROR)
    {
        if ((value % 1000000) == 0 && value != 0)
            printf("%luM\n", (unsigned long) value / 1000000);
        else if ((value % 1000) == 0 && value != 0)
            printf("%luk\n", (unsigned long) value / 1000);
        else
            printf("%lu\n", (unsigned long) value);
    }
    else if (error == AJI_UNIMPLEMENTED)
    {
        BYTE buffer[AJI_PARAMETER_MAX+1];
        DWORD len = sizeof(buffer)-1;
        error = aji_get_parameter(chain, name, buffer, &len);
        if (error == AJI_NO_ERROR)
        {
            buffer[len] = 0;
            printf("%s\n", buffer);
        }
        else if (error == AJI_UNIMPLEMENTED)
        {
            fprintf(stderr, "No parameter named %s\n", name);
        }
    }

    if (error != AJI_NO_ERROR && error != AJI_UNIMPLEMENTED)
    {
        fprintf(stderr, "Error when getting parameter - %s\n", decode_error(error));
    }

    aji_unlock_chain(chain);

    return (error == AJI_NO_ERROR) ? 0 : 4;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
// 
int add(const char * type, const char * port, const char * name)
{
    // Add a new chain
    AJI_ERROR error;
    AJI_HARDWARE hw;

    hw.hw_name = type;
    hw.port = port;
    hw.device_name = name;

    error = aji_add_hardware(&hw);

    if (error != AJI_NO_ERROR)
    {
        fprintf(stderr, "Unable to add hardware - %s\n", decode_error(error));
        return 4;
    }

    return 0;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//

int remove_chain(const char * cable)
{
#if 0
    // Let users remove hardware by name or position
    AJI_HARDWARE hw;
    AJI_ERROR error = aji_find_hardware(cable, &hw, 6000);

    if (error != AJI_NO_ERROR)
    {
        if (error == AJI_FAILURE)
            fprintf(stderr, "Unable to find jtag cable \"%s\"\n", cable);
        else
            fprintf(stderr, "Error (%d) when scanning hardware\n", error);
        return 4;
    }

    error = aji_remove_hardware(hw.chain_id);

#else

    DWORD id = atoi(cable);

    // TODO: use aji_find_hardware to convert cable name into PID

    // Remove the specified chain
    DWORD hw_count = 0;
    AJI_HARDWARE * hw = NULL;

    AJI_ERROR error = get_hardware(&hw_count, &hw, NULL, NULL);

    if (error != AJI_NO_ERROR && error != AJI_TIMEOUT)
    {
        fprintf(stderr, "Unable to scan hardware - %s\n", decode_error(error));
        return 0;
    }

    if (hw_count == 0)
    {
        fprintf(stderr, "No JTAG hardware found\n");
        return 4;
    }

    if (id == 0 || id > hw_count)
    {
        delete[] hw;
        fprintf(stderr, "No JTAG hardware with id number %lu\n", (unsigned long) id);
        return 4;
    }

    error = aji_remove_hardware(hw[id-1].chain_id);

    delete[] hw;
#endif

    if (error != AJI_NO_ERROR)
    {
        if (error == AJI_INVALID_PARAMETER)
            fprintf(stderr, "Unable to remove automatically added hardware\n");
        else
            fprintf(stderr, "Unable to remove hardware - %s\n", decode_error(error));
        return 4;
    }

    return 0;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int add_server(const char * server, const char * password)
{
    AJI_ERROR error = aji_add_remote_server(server, password);

    if (error != AJI_NO_ERROR)
        fprintf(stderr, "Error when adding remote server - %s\n", decode_error(error));

    return 0;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int set_password(const char * password)
{
    AJI_ERROR error = aji_enable_remote_clients(password != NULL, password);

    if (error != AJI_NO_ERROR)
        fprintf(stderr, "Error when setting password - %s\n", decode_error(error));

    return 0;
}

static const char * error_info(const char * base)
{
    static char buffer[256];

    const char * ei = aji_get_error_info();
    if (ei == NULL && ei[0] != 0)
        return base;

    snprintf(buffer, sizeof(buffer), "%s - check %s", base, ei);
    return buffer;
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
const char * decode_error(AJI_ERROR error)
{
    switch (error)
    {
    case AJI_NO_ERROR:              return "";
    case AJI_TIMEOUT:               return "Timeout";

    case AJI_UNKNOWN_HARDWARE:      return "Unknown hardware";
    case AJI_INVALID_CHAIN_ID:      return "Invalid CHAIN_ID";
    case AJI_LOCKED:                return "Locked already";
    case AJI_NOT_LOCKED:            return "Not locked";

    case AJI_CHAIN_IN_USE:          return error_info("Chain in use");
    case AJI_NO_DEVICES:            return "No devices";
    case AJI_CHAIN_NOT_CONFIGURED:  return "Chain not configured";
    case AJI_BAD_TAP_POSITION:      return "Bad TAP position";
    case AJI_DEVICE_DOESNT_MATCH:   return "Device doesn't match";
    case AJI_IR_LENGTH_ERROR:       return "IR length error";
    case AJI_DEVICE_NOT_CONFIGURED: return "Device not configured";
    case AJI_CHAINS_CLAIMED:        return error_info("Conflict with another device");

    case AJI_INVALID_OPEN_ID:       return "Invalid OPEN_ID";
    case AJI_INVALID_PARAMETER:     return "Invalid parameter";
    case AJI_BAD_TAP_STATE:         return "Bad TAP state";
    case AJI_TOO_MANY_DEVICES:      return "Too many devices";
    case AJI_IR_MULTIPLE:           return "Different TAPS selected";
    case AJI_BAD_SEQUENCE:          return "Bad sequence";
    case AJI_INSTRUCTION_CLAIMED:   return "Instruction claimed";

    case AJI_FILE_ERROR:            return "File not found";
    case AJI_NET_DOWN:              return "Communications error";
    case AJI_SERVER_ERROR:          return "Server error";
    case AJI_NO_MEMORY:             return "Out of memory";
    case AJI_PORT_IN_USE:           return "Hardware in use";
    case AJI_BAD_PORT:              return "Bad port name";
    case AJI_BAD_HARDWARE:          return "Hardware not attached";
    case AJI_BAD_JTAG_CHAIN:        return "JTAG chain broken";
    case AJI_NOT_PERMITTED:         return "Insufficient port permissions";
    case AJI_HARDWARE_DISABLED:     return "Hardware disabled";

    case AJI_UNIMPLEMENTED:         return "Feature not implemented or unavailable under current execution privilege level";
    case AJI_INTERNAL_ERROR:        return "Internal error";

    default:                        return "Unknown error";
    }
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
void enumerate_output(void * handle, DWORD level, const char * line)
{
    const char * prefix;

    (void)handle;

    switch (level)
    {
    case 0:
        prefix = "Error: ";
        break;
    case 1:
        prefix = "Warning: ";
        break;
    case 2:
        prefix = "";
        break;
    default:
        return;
    }

    printf("%s%s\n", prefix, line);
}
