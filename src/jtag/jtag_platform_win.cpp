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
// Filename:    ajicom_platform_win.cpp
//
// Description: 
//
// Authors:     Andrew Draper
//
//              Copyright (c) Altera Corporation 2000
//              All rights reserved.
//
//END_MODULE_HEADER//////////////////////////////////////////////////////////

//START_ALGORITHM_HEADER/////////////////////////////////////////////////////
//
// Platform specific functions for Windows NT/98
//
//END_ALGORITHM_HEADER///////////////////////////////////////////////////////

// INCLUDE FILES ////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif

#ifndef INC_WINSOCK2_H
#include <winsock2.h>
#define INC_WINSOCK2_H
#endif

#ifndef INC_WINDOWS_H
#include <Windows.h>
#define INC_WINDOWS_H
#endif

#ifndef INC_GEN_STRING_SYS_H
#include "gen_string_sys.h"
#endif

#ifndef INC_JTAG_PLATFORM_H
#include "jtag_platform.h"
#endif


HINSTANCE jtag_client_module = NULL;

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
int get_time(void)
//
// Return a monotonically increasing time measured in milliseconds (eg. from
// system reset or similar).  This time will overflow after 47 days or so so
// make sure all comparisons are of the form "(t1 - t2) > constant"
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    return GetTickCount();
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

#if !defined(_WIN64)
bool is_windows_nt(void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(os);
    if (GetVersionEx(&os) == 0)
        return false;

    return (os.dwPlatformId == VER_PLATFORM_WIN32_NT);
}
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////

#if !defined(_WIN64)
bool is_windows_2000(void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(os);
    if (GetVersionEx(&os) == 0)
        return false;

    return (os.dwPlatformId == VER_PLATFORM_WIN32_NT && os.dwMajorVersion >= 5);
}
#endif

#if PORT == WINDOWS && !defined(_WIN64) && defined(_WIN32)
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool is_32bit_app_on_64bit_kernel()
//
// Description: Check whether this is a 32 bit process running under a 64 bit
//              windows kernel.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if (NULL != fnIsWow64Process)
        if (fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
            return bIsWow64 != 0;

    return false;
}
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool get_server_path(char * buffer, int bufflen)
//
// Description: Find the path to the running executable
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    if (GetModuleFileName(NULL, buffer, bufflen) == 0)
        return false;
    buffer[bufflen-1] = 0;

    return true;
}

//START_FUNCTION_HEADER/////START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
bool start_quartus_process(const char * cmnd, int argc, const char * * args, bool wait_for_success)
//
// Description: Start a process with the arguments specified.
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    char name[512];
    char command[1024], *leaf;

    GetModuleFileName(jtag_client_module, name, sizeof(name)-2);
    GetFullPathName(name, sizeof(command)-1, command+1, &leaf);

    if (leaf > command + sizeof(command) - 512)
        return false;

#if !defined(_WIN64) && defined(_WIN32)
    if (is_32bit_app_on_64bit_kernel() && leaf - command > 7 && strncmp(leaf-5, "\\bin\\", 5) == 0)
    {
        // If this is a 32 bit DLL running on a 64 bit kernel then we need
        // to launch the 64 bit version of jtagserver so it can access the
        // hardware. 
        strcpy_s(leaf-5, sizeof(command), "\\bin64\\");
        leaf += 2;
    }
#endif

    command[0] = '"';

    strcpy_s(leaf, sizeof(command), cmnd);
    for (DWORD i = 0 ; i < strnlen_s(cmnd, sizeof(command)) ; i++, leaf++)
        if (*leaf == '/')
            *leaf = '\\';
    //strcpy_s(leaf, sizeof(command), ".exe");
    //leaf += 4;

    strcpy_s(name, sizeof(name), command+1);

    *leaf++ = '"';

    for (int i = 0 ; i < argc ; i++)
    {
        int l = (int)strnlen_s(args[i], 1024);
        if (leaf + l - command >= ((int) sizeof(command))-2)
            return false;
        *leaf++ = ' ';
        strcpy_s(leaf, sizeof(command), args[i]);
        leaf += l;
    }

    PROCESS_INFORMATION pi;
    STARTUPINFO si;

    // Set up the start up info struct.
    ZeroMemory(&si,sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = 0;
    si.hStdInput  = 0;
    si.hStdError  = 0;
    si.wShowWindow = SW_HIDE;

    bool ok = false;
    if (CreateProcess(name, // full path to local jtagserver
                      command,  // command line
                      NULL,  // process attributes
                      NULL,  // thread attributes
                      FALSE, // inherit handles
                      0,//CREATE_NO_WINDOW, // flags
                      NULL,  // environment
                      NULL,  // current directory
                      &si,   // startup info
                      &pi))  // process info
    {
        if (wait_for_success)
        {
            // If trying to start the service then check the return code to see
            // whether any errors were reported (the usual one being "not installed").
            if (WaitForSingleObject(pi.hProcess, 1000) == WAIT_OBJECT_0)
            {
                DWORD exit_code;
                if (GetExitCodeProcess(pi.hProcess, &exit_code) == 0 && exit_code == 0)
                    ok = true;
            }
        }
        else
            ok = true;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    return ok;
}

/////////////////////////////////////////////////////
