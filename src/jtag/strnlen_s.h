#ifndef _TOREMOVE_STRNLEN_S_H_
#define _TOREMOVE_STRNLEN_S_H_

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif 

// temporary hack to remove the issue with strnlen_s not available
// in mingw

/*------------------------------------------------------------------
 * strnlen_s.c
 *
 * October 2008, Bo Berry
 *
 * Copyright (c) 2008-2011 by Cisco Systems, Inc
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *------------------------------------------------------------------
 */

/**
 * NAME
 *    strnlen_s
 *
 * SYNOPSIS
 *    #include "safe_str_lib.h"
 *    rsize_t
 *    strnlen_s(const char *dest, rsize_t dmax)
 *
 * DESCRIPTION
 *    The strnlen_s function computes the length of the string pointed
 *    to by dest.
 *
 * SPECIFIED IN
 *    ISO/IEC TR 24731-1, Programming languages, environments
 *    and system software interfaces, Extensions to the C Library,
 *    Part I: Bounds-checking interfaces
 *
 * INPUT PARAMETERS
 *    dest      pointer to string
 *
 *    dmax      restricted maximum length.
 *
 * OUTPUT PARAMETERS
 *    none
 *
 * RUNTIME CONSTRAINTS
 *    dest shall not be a null pointer
 *    dmax shall not be greater than RSIZE_MAX_STR
 *    dmax shall not equal zero
 *
 * RETURN VALUE
 *    The function returns the string length, excluding  the terminating
 *    null character.  If dest is NULL, then strnlen_s returns 0.
 *
 *    Otherwise, the strnlen_s function returns the number of characters
 *    that precede the terminating null character. If there is no null
 *    character in the first dmax characters of dest then strnlen_s returns
 *    dmax. At most the first dmax characters of dest are accessed
 *    by strnlen_s.
 *
 * ALSO SEE
 *    strnterminate_s()
 *
 */

#ifndef INC_CSTDDEF
#include <cstddef>
#define INC_CSTDDEF
#endif

size_t
strnlen_s(const char* dest, size_t dmax);

#endif