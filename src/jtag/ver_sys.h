/****************************************************************************
 *   Copyright (c) 2012 by Intel Corporation                                *
 *   author: Sundaram, Devin                                                *
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
 
 // START_MODULE_HEADER/////////////////////////////////////////////////////////
//
//
// Description: Interface of the Version APIs
//
// Authors:     Devin Sundaram
//
//              Copyright (c) Altera Corporation 2012-
//              All rights reserved.
//
//
// END_MODULE_HEADER///////////////////////////////////////////////////////////

#ifndef INC_VER_SYS_H
#define INC_VER_SYS_H

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef INC_AJI_SYS_H
#include "aji_sys.h"
#endif

#ifndef INC_STRING
#include <string>
#endif

// START_FUNCTION_HEADER///////////////////////////////////////////////////////
//
std::string ver_get_full_version(bool with_sp = true);
//
// Description: Full version string of Quartus
//
// Returns:     Example: "Version 2.1 Internal Build 65 02/05/2002 SJ Web Edition",
//              "Version 2.1 Internal Build 65 02/05/2002 Service Pack 1 SJ Full Version"
//
// END_FUNCTION_HEADER/////////////////////////////////////////////////////////


// START_FUNCTION_HEADER///////////////////////////////////////////////////////
//
std::string ver_get_copyright_string();
//
// Description:    Copyright string with current year
//
// Returns:     Example: "Copyright (C) 1991-2013 Altera Corporation"
//
// END_FUNCTION_HEADER/////////////////////////////////////////////////////////

                                     
#endif // INC_VER_SYS_H
