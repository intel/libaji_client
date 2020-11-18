


/****************************************************************************
 *   Copyright (c) 2016 by Intel Corporation                                *
 *   author: Ferrucci,Aaron and Sundaram, Devin                             *
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

#ifndef SECURE_GETENV_H
#define SECURE_GETENV_H

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_CSTDLIB
#include <cstdlib>
#endif

// The __secure_getenv()/secure_getenv() function is intended for use
// in general-purpose libraries to avoid vulnerabilities that could occur
// if set-user-ID or set-group-ID programs accidentally trusted the
// environment.

#if PORT==WINDOWS
#define getenv __secure_getenv
#else

#if __GLIBC_PREREQ(2, 17)
// In GLIBC 2.17 (Suse 12), a new function 'secure_getenv' replaces '__secure_getenv'.
// The old function can not be called directly in our code
#define getenv secure_getenv
// But to maintain compatibility with RHEL6, we will direct linkage back to the old implmentation
__asm__(".symver secure_getenv, __secure_getenv@GLIBC_2.2.5");
#else
#define getenv __secure_getenv
#endif

#endif // PORT

#endif // SECURE_GETENV_H
