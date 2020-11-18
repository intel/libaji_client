/****************************************************************************
 *   Copyright (c) 2018 by Intel Corporation                                *
 *   author: Loh, Hong Lee                                                  *
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
//# Description: Common header file for codes in pgm/jtag subsystem
//#
//# Authors:     Hong Lee Loh
//#
//#              Copyright (c) Altera Corporation 2000 - 2018
//#              All rights reserved.
//#
//# END_MODULE_HEADER///////////////////////////////////////////////////////

#ifndef INC_JTAG_COMMON_H
#define INC_JTAG_COMMON_H

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_GEN_STRING_SYS_H
#include "gen_string_sys.h"
#endif

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline errno_t memset_(void *dest, unsigned char value, size_t len)
//
// Description: This functions maps to memset_s() in Linux and memset() in
//              Windows because memset_s() is only available for Linux in
//              Intel's Safe String Library
//
// Returns:
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
#if PORT==UNIX
    return memset_s(dest, len, value);
#endif
#if PORT==WINDOWS
    memset(dest, value, len);
    return 0;
#endif
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline int memcmp_(const void *dest, size_t dmax, const void *src, size_t smax)
//
// Description: This functions maps to memcmp_s() and asserts that no error
//              should occur
//
// Returns:
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
#if PORT==UNIX
    int result;
    AJI_ASSERT(memcmp_s(dest, dmax, src, smax, &result) == EOK);
    return result;
#endif
#if PORT==WINDOWS
    return memcmp(dest, src, smax);
#endif
}

//START_FUNCTION_HEADER////////////////////////////////////////////////////////
//
inline int strcmp_(const char *dest, size_t dmax, const char *src)
//
// Description: This functions maps to strcmp_s() and asserts that no error
//              should occur
//
// Returns:
//
//END_FUNCTION_HEADER////////////////////////////////////////////////////////
{
#if PORT==UNIX
    int result;
    AJI_ASSERT(strcmp_s(dest, dmax, src, &result) == EOK);
    return result;
#endif
#if PORT==WINDOWS
    return strcmp(dest, src);
#endif
}

#endif
