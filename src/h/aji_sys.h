/****************************************************************************
 *   Copyright (c) 1996 by Intel Corporation                                *
 *   author: Redman, Scott                                                  *
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
 
//START_MODULE_HEADER//////////////////////////////////////////////////////////
//
//
// Description: Altera standard header definitions (Common Code)
//
// Authors:     Scott Redman
//
//              Copyright (c) Altera Corporation 1996, 1997.
//              All rights reserved.
//
//
//END_MODULE_HEADER////////////////////////////////////////////////////////////


/* / COMPILER FLAGS ////////////////////////////////////////////////// */
/*
 * libaji_client relies on a collection of compiler flags to be compile
 * Windows/Linux platform. Some of the compiler flags are defined when
 * you invoke the compiler.
 * 
 * This is a non-exhausive collection of compiler flags and their meanings
 * It is collected from various files. The purpose is to make it easier to
 * understand the compiler options.
 * 
 * COMPILER FLAGS controlling libaji_client compilation:
 * - ALL platform:
 *  - LITTLE=1 BIG=2 : options for ENDIAN
 *  - WINDOWS=1 UNIX=2 LINUX=3  # options for PORT and SYS
 *  - AJI_NO_MEM_MANAGER  # define it for now, will be removed later
 *                         # It controls reporting via
 *                         # AJI_INTERNAL_ERROR_WRAPPER
 *                         # Not yet sure the effect of removing it
 * - Example
 *   - For Windows
 *     - ENDIAN=LITTLE #assuming x86
 *     - PORT=WINDOWS #OS variant
 *     - SYS=WINDOWS  #Operating System
 *   - For Linux
 *     - ENDIAN=LITTLE #assuming x86
 *     - PORT=UNIX  #OS variant
 *     - SYS=LINUX  #Operating System 
 * - Optional:
 *   - MOD_64_BIT 
 *         Define if you are compiling to 64 bit binary. This is to set
 *        # the variable size, e.g. ULONG etc, correctly.
 *   - GLIBC_SYMVER_DIRECTIVES 
 *        #Define it To workaround/cure a memcpy issue, \see aji_glibc_sys.h
 *   - JTAG_DLLEXPORT=DLLIMPORT|AJI_DLLEXPORT 
 *        # This controls the behaviour of macro AJI_API. 
 *        # If you are using the function decorated by AJI_API from a 
 *        # dynamically-linked library, set to DLLIMPORT. If instead,
 *        # you are building a library containing the decorated function
 *        # you should define to AJI_DLLEXPORT.
 *   - STRIP_ASSERTION_TEXT
 *        # Replace all internal error messages with "Internal Error"
 *        # Don't use it to hide messages from developer, but
 *        # can use it to make message better for users, e.g.
 *        # to not cause panic if source code is shown.
 
 * -  COMPILER OPTIONS:
 *   - -fvisibility=hidden (compulsory)
 *     # Part of optimization scheme to control visibility of a function. Particularly useful
 *     # when creating library. https://gcc.gnu.org/wiki/Visibility
 *   - -D_GLIBCXX_USE_CXX11_ABI=0 (compulsory, g++)
 *     Not sure why, but we are using old ABI. 
 *     https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_dual_abi.html
 */

/* / INTERFACE DESCRIPTION ////////////////////////////////////////////////// */

#ifndef INC_AJI_SYS_H
#define INC_AJI_SYS_H

/* / INCLUDE FILES /////////////////////////////////////////////////////////  */

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// This header now defines all macros and types needed by Quartus generically
// Also includes standard header files
#ifndef INC_AJI_MACROS_SYS_H
#include "aji_macros_sys.h"
#endif

#define AJI_INTERNAL_ERROR_WRAPPER(string, subsys, file, line) { fprintf(stderr, "%s\n", string); ::exit(-1); }

#ifdef STRIP_ASSERTION_TEXT
#define AJI_INTERNAL_ERROR(string) AJI_INTERNAL_ERROR_WRAPPER("Internal Error", CODE_PREFIX, __FILE__, __LINE__)
#else
#define AJI_INTERNAL_ERROR(string) AJI_INTERNAL_ERROR_WRAPPER(string, CODE_PREFIX, __FILE__, __LINE__)
#endif

#define AJI_ASSERT(condition) \
    if (AJI_UNLIKELY(!(condition))) { AJI_INTERNAL_ERROR(#condition); }
#define AJI_INFO_ASSERT(condition,text) \
    if (AJI_UNLIKELY(!(condition))) { AJI_INTERNAL_ERROR(text); }


#ifdef DEBUG_ASSERT
#define AJI_DEBUG_ASSERT(condition) AJI_ASSERT(condition)
#define AJI_DEBUG_INFO_ASSERT(condition,text) AJI_INFO_ASSERT(condition,text)
#define AJI_DEBUG_INFO_ASSERT_VARG(condition,format_text,...) AJI_INFO_ASSERT_VARG(condition,format_text,__VA_ARGS__)
#define AJI_DEBUG_INTERNAL_ERROR(text) AJI_INTERNAL_ERROR(text)
#define AJI_DEBUG_INTERNAL_ERROR_VARG(format_text,...) AJI_INTERNAL_ERROR_VARG(format_text,__VA_ARGS__)
#else
#define AJI_DEBUG_ASSERT(condition) 
#define AJI_DEBUG_INFO_ASSERT(condition,text) 
#define AJI_DEBUG_INFO_ASSERT_VARG(condition,format_text,...)
#define AJI_DEBUG_INTERNAL_ERROR(text)
#define AJI_DEBUG_INTERNAL_ERROR_VARG(format_text,...)
#endif


#endif /*  INC_AJI_SYS_H */
