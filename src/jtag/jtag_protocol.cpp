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
// Description: 
//
// Authors:     Andrew Draper
//
//              Copyright (c) Altera Corporation 2000 - 2001
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

#ifndef INC_JTAG_PROTOCOL_H
#include "jtag_protocol.h"
#endif


//GLOBALS////////////////////////////////////////////////////////////////////
//
const TAP_STATE next_tap_state[16][2] =
{
    // tms 0              1
    { RUN_TEST_IDLE, TEST_LOGIC_RESET },  // TEST_LOGIC_RESET
    { RUN_TEST_IDLE, SELECT_DR_SCAN },    // RUN_TEST_IDLE,
    { CAPTURE_DR,    SELECT_IR_SCAN },    // SELECT_DR_SCAN
    { SHIFT_DR,      EXIT1_DR },          // CAPTURE_DR
    { SHIFT_DR,      EXIT1_DR },          // SHIFT_DR
    { PAUSE_DR,      UPDATE_DR },         // EXIT1_DR
    { PAUSE_DR,      EXIT2_DR },          // PAUSE_DR
    { SHIFT_DR,      UPDATE_DR },         // EXIT2_DR
    { RUN_TEST_IDLE, SELECT_DR_SCAN },    // UPDATE_DR,
    { CAPTURE_IR,    TEST_LOGIC_RESET },  // SELECT_IR_SCAN
    { SHIFT_IR,      EXIT1_IR },          // CAPTURE_IR
    { SHIFT_IR,      EXIT1_IR },          // SHIFT_IR
    { PAUSE_IR,      UPDATE_IR },         // EXIT1_IR
    { PAUSE_IR,      EXIT2_IR },          // PAUSE_IR
    { SHIFT_IR,      UPDATE_IR },         // EXIT2_IR
    { RUN_TEST_IDLE, SELECT_DR_SCAN },    // UPDATE_IR
};

//START_FUNCTION_HEADER//////////////////////////////////////////////////////


