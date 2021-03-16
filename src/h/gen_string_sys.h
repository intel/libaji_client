/****************************************************************************
 *   Copyright (c) 1997 by Intel Corporation                                *
 *   author: Bimm, Michael                                                  *
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
//#
//# Description:    General purpose string functions
//#
//# Authors:        Michael Bimm 
//#
//#              Copyright (c) Altera Corporation 1997 - 
//#              All rights reserved.
//#
//#
//# END_MODULE_HEADER///////////////////////////////////////////////////////////

//# START_ALGORITHM_HEADER//////////////////////////////////////////////////////
//#
//#
//# END_ALGORITHM_HEADER////////////////////////////////////////////////////////
//#

//# INTERFACE DESCRIPTION //////////////////////////////////////////////////////
//# 
//# This section is optional. Use this for explanatory text
//# describing the interface contained within this file.

#ifndef INC_GEN_STRING_SYS_H
#define INC_GEN_STRING_SYS_H

//# INCLUDE FILES //////////////////////////////////////////////////////////////
//#
//# Include files in the following order below the
//# corresponding headers.
//#

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//# STANDARD INCLUDE FILES
#ifndef INC_CSTRING
#include <cstring>
#define INC_CSTRING
#endif

#ifndef INC_CSTDINT
#include <cstdint>
#define INC_CSTDINT
#endif

//# INTEL SAFE STRING LIBRARY
#if PORT==UNIX && SYS==LINUX
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __SAFE_LIB_H__
#include "safe_lib.h"
#endif
#ifdef __cplusplus
}
#endif
#endif

#if PORT==WINDOWS
// This is to solve strtok_s difference between linux (needs 4 args) and windows (needs 3 args)
inline char * strtok_s(char *strToken, size_t *strTokenMax, const char *strDelimit, char **context)
{
    if (strTokenMax == NULL) {
        return (NULL);
    }
    
    if (*strTokenMax == 0) {
        return (NULL);
    }
    
    return strtok_s(strToken, strDelimit, context);
}
#endif

#endif //# INC_GEN_STRING_SYS_H
