/****************************************************************************
 *   Copyright (c) 2004 by Intel Corporation                                *
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
//              Copyright (c) Altera Corporation 2004
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

#ifndef INC_CCTYPE
#include <cctype>
#define INC_CCTYPE
#endif

#ifndef INC_STRING
#include <string>
#define INC_STRING
#endif

#ifndef INC_JTAG_UTILS_H
#include "jtag_utils.h"
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
DWORD jtag_decode_version(const char * version)
//
// Description: Decodes a Quartus II version string, in the format:
//              <major>.<minor> [ SP<pack> ] [ build <build> ]
//
// Returns:     The version number
//
//END_FUNCTION_HEADER//////////////////////////////////////////////////////////
{
    DWORD major = 0, minor = 0, sp = 0, build = 0;

    while (isspace(*version))
        version++;

    if (!isdigit(*version))
        return 0;
    while (isdigit(*version))
        major = (major * 10) + (*version++ - '0');

    if (*version != '.')
        return 0;
    version++;

    if (!isdigit(*version))
        return 0;
    while (isdigit(*version))
        minor = (minor * 10) + (*version++ - '0');

    while (isspace(*version))
        version++;

    if (strncmp(version, "SP", 2) == 0)
    {
        version += 2;

        if (!isdigit(*version))
            return 0;
        while (isdigit(*version))
            sp = (sp * 10) + (*version++ - '0');
    }

    while (isspace(*version))
        version++;

    if (strnicmp(version, "build ", 6) == 0)
    {
        version += 6;

        if (!isdigit(*version))
            return 0;
        while (isdigit(*version))
            build = (build * 10) + (*version++ - '0');
    }

    // Return a monotonic version number which is:
    //  31:28  0 (for future expansion)
    //  27:22  major
    //  21:16  minor
    //  15:12  SP
    //  11: 0  build
    //

    if (major >= 32 || minor >= 32 || sp >= 16 || build >= 4096)
        return 0;

    return (major << 22) | (minor << 16) | (sp << 12) | build;
}

//START_FUNCTION_HEADER//////////////////////////////////////////////////////


