/****************************************************************************
 *   Copyright (c) 2002 by Intel Corporation                                *
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
// Filename:    jtag_config_status_win.cpp
//
// Description: 
//
// Authors:     Andrew Draper
//
//              Copyright (c) Altera Corporation 2000 - 2002
//              All rights reserved.
//
//END_MODULE_HEADER//////////////////////////////////////////////////////////

//START_ALGORITHM_HEADER/////////////////////////////////////////////////////
//
// This file contains the functions which check the status of the Windows NT
// or Windows 98 service.
//
//END_ALGORITHM_HEADER///////////////////////////////////////////////////////

// INCLUDE FILES ////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif

#ifndef INC_VER_SYS_H
#include "ver_sys.h"
#endif

#ifndef INC_AJI_H
#include "aji.h"
#endif

#ifndef INC_JTAG_PLATFORM_H
#include "jtag_platform.h"
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

#ifndef INC_TLHELP32_H
#include <tlhelp32.h>
#define INC_TLHELP32_H
#endif

#endif // PORT == WINDOWS

AJI_API const char * aji_get_server_version_info(void);
AJI_API const char * aji_get_server_path(void);

/////////////////////////////////////////////////////////////////////////////

int serverinfo_winnt(char * fullpathname, DWORD namelen, bool & running);
int serverinfo_win98(char * fullpathname, DWORD namelen, bool & running);

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int serverinfo(void)
//
// Description: Function to print the current JTAG server status.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
#if PORT == WINDOWS
    int rc;
    char fullpathname[256];
    bool running = false;

    bool winnt = is_windows_nt();

    if (winnt)
        rc = serverinfo_winnt(fullpathname, sizeof(fullpathname), running);
    else
        rc = serverinfo_win98(fullpathname, sizeof(fullpathname), running);

    if (rc != 0)
        return rc;

    if (fullpathname[0] == 0)
    {
        printf("JTAG Server not installed\n");
    }
    else
    {
        printf("Installed JTAG server is '%s'\n", fullpathname);
        if (winnt)
            printf("  Service manager reports server is %s\n", running ? "running" : "stopped");
    }

    const char * server_path = aji_get_server_path();
    if (server_path != NULL)
        printf("Server reports path: %s\n", server_path);

    const char * version_info = aji_get_server_version_info();
    if (version_info != NULL)
        printf("Server reports version: %s\n", version_info);

    bool remoteok;
    AJI_ERROR error = aji_get_remote_clients_enabled(&remoteok);
    if (error == AJI_NO_ERROR)
        printf("Remote clients are %s\n", remoteok ? "enabled" : "disabled (no password)");
    else if (error == AJI_UNIMPLEMENTED)
        printf("Remote clients are disabled (server policy)\n");

    return 0;
#else
    return 0;
#endif
}

//START_FUNCTION_HEADER//////////////////////////////////////////////////////
//
int serverinfo_winnt(char * fullpathname, DWORD namelen, bool & running)
//
// Description: Check whether the service is installed, and if so get its
//              path and whether or not it is running.
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    SC_HANDLE service, scm;
    int rc;

    scm = OpenSCManager(0, 0, SC_MANAGER_CONNECT);
    if (scm == NULL)
    {
        fprintf(stderr, "Unable to open service control manager\n");
        return 16;
    }

    // Get the service's handle
    service = OpenService(scm, "JTAGServer", SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);

    if (service == NULL)
    {
        fullpathname[0] = 0;
        rc = 0;
    }
    else
    {
        DWORD size;
        if (QueryServiceConfig(service, NULL, 0, &size) || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            fprintf(stderr, "Unable to get service configuration\n");
            rc = 16;
        }
        else
        {
            void * buffer = malloc(size);
            QUERY_SERVICE_CONFIG * config = static_cast<QUERY_SERVICE_CONFIG *>(buffer);
            SERVICE_STATUS status;

            if (buffer == NULL)
            {
                fprintf(stderr, "Out of memory getting service configuration\n");
                rc = 16;
            }
            else if (!QueryServiceConfig(service, config, size, &size))
            {
                fprintf(stderr, "Unable to get service configuration\n");
                rc = 16;
            }
            else
            {
                strncpy_s(fullpathname, namelen, config->lpBinaryPathName, namelen);
                fullpathname[namelen-1] = 0;

                if (QueryServiceStatus(service, &status))
                    running = (status.dwCurrentState == SERVICE_RUNNING);
                
                rc = 0;
            }

            free(buffer);
        }

        CloseServiceHandle(service);
    }

    CloseServiceHandle(scm);

    return rc;
}

//START_FUNCTION_HEADER//////////////////////////////////////////////////////
//
int serverinfo_win98(char * fullpathname, DWORD namelen, bool & running)
//
// Description: Check whether the service is installed, and if so get its
//              path and whether or not it is running.
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
    int rc = 0;

    HKEY key;
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices",
                       0, NULL, REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE, NULL, &key, NULL) != ERROR_SUCCESS)
    {
        fprintf(stderr, "Error opening auto start registry key\n");
        rc = 16;
    }
    else
    {
        DWORD type;
        DWORD size = namelen;
        if (RegQueryValueEx(key, "AlteraJTAGServer", 0, &type, (BYTE *)fullpathname, &size) != ERROR_SUCCESS ||
            type != REG_SZ)
        {
            fullpathname[0] = 0;
            rc = 0;
        }
        else
        {
            // TODO: check whether server is running
            running = true;

            rc = 0;
        }

        RegCloseKey(key);
    }

    return rc;
}

////////////////////////////////////////////////////////////////////////////////

