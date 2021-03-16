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
 
 //# START_MODULE_HEADER/////////////////////////////////////////////////////
//#
//# Filename:    jtag_protocol.h
//#
//# Description: 
//#
//# Authors:     Alan Whitaker
//#
//#              Copyright (c) Altera Corporation 1997 - 1999
//#              All rights reserved.
//#
//# END_MODULE_HEADER///////////////////////////////////////////////////////

//# START_ALGORITHM_HEADER//////////////////////////////////////////////////
//#
//#
//# END_ALGORITHM_HEADER////////////////////////////////////////////////////
//#

//# INTERFACE DESCRIPTION //////////////////////////////////////////////////
//#
//# Use this for explanatory text
//# describing the interface contained within this file.

#ifndef INC_JTAG_PROTOCOL_H
#define INC_JTAG_PROTOCOL_H

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//# INCLUDE FILES //////////////////////////////////////////////////////////

//      Constants and typedefs

enum TAP_STATE
{
    TEST_LOGIC_RESET = 0,
    RUN_TEST_IDLE    = 1,
    SELECT_DR_SCAN   = 2,
    CAPTURE_DR       = 3, 
    SHIFT_DR         = 4,
    EXIT1_DR         = 5,
    PAUSE_DR         = 6,
    EXIT2_DR         = 7, 
    UPDATE_DR        = 8,
    SELECT_IR_SCAN   = 9,
    CAPTURE_IR      = 10,
    SHIFT_IR        = 11,
    EXIT1_IR        = 12,
    PAUSE_IR        = 13,
    EXIT2_IR        = 14,
    UPDATE_IR       = 15
};

extern const TAP_STATE next_tap_state[16][2];

// The watcher data stream consists of one byte for each clock (value 0x00..7F)
// or to indicate special circumstances (0xFF).
enum WATCH_BITS
{
    // Within bytes <= 0x7F the bits have the following meaning
    WATCH_TDI = 1,            // TDI was high on the rising edge of TCK
    WATCH_TMS = 2,            // TMS was high on the rising edge of TCK
    WATCH_TDO = 8,            // TDO was high on the rising edge of TCK

    WATCH_TDI_RECIRC  = 0x20, // This TDI value was recirculated from TDO on the
                              // last clock cycle.  If TDO_UNKNOWN was set on that
                              // clock cycle the TDI is unknown on this.

    WATCH_TDO_UNKNOWN = 0x40, // The hardware did not sample TDO on this clock cycle
                              // so the TDO bit should be ignored.

    WATCH_SPECIAL     = 0x80, // This byte is a special byte, not part of the data
                              // stream of clocks.  See below for values:


    WATCH_LOCK_CHANGE = 0xF0, // The lock state of the chain has changed.  The next
                              // three bytes indicate the new lock state (MSb of each
                              // byte is set to show it is special):
                              // 80 80 80 - not locked
                              // XX 80 80 - chain locked by client XX
                              // XX YY YY - chain locked by device YYYY on client XX

    WATCH_OLD_INFO    = 0xF1, // The server's view of the devices on the chain follows:
                              //   +1  1  Server version
                              //   +2  1  Number of TAPs on chain
                              //   +3  1  Length of data for each TAP
                              // Then a description of each TAP
                              //   +0  4  TAP IDCODE (0 if none)
                              //   +4  1  TAP IR length
                              //   +5  1  Reserved (0)
                              //   +6  2  TAP features (MSB first)

    WATCH_TIMESTAMP   = 0xF2, // Some time has passed since the last timestamp message
                              //   +1  2  Number of ms since last timestamp (MSB first)

    WATCH_CHAIN_INFO  = 0xFC, // The server's view of the devices on the chain follows:
                              //   +1  1  Version, should be 0
                              //   +2  2  Length of chain information, MSB first
                              //   +4  4  Pseudo
                              //   +8  1  Header size (should be 12)
                              //   +9  1  TAP state
                              //   +10 1  Number of TAPs on chain
                              //   +11 1  Length of data for each TAP
                              // Then a description of each TAP
                              //   +0  4  TAP IDCODE (0 if none)
                              //   +4  1  TAP IR length
                              //   +5  1  Reserved (0)
                              //   +6  2  TAP features (MSB first)

    WATCH_SPOOL_ON    = 0xFD, // Not returned by the JTAG server but inserted into
                              // spool files to indicate where spool was turned on.
                              // Information about the chain follows:
                              //   +1  1  Information version, should be 0
                              //   +2  2  Length of SPOOL_ON information, MSB first
                              //   +4  1  Offset to strings (so)
                              //   +5  1  Length of machine name string (lmn)
                              //   +6  1  Length of user name string (lun)
                              //   +7  1  Length of cable name string (lcn)
                              //   +8  4  Time spool was opened, seconds since 1970 GMT
                              //  +so lmn Machine name, no terminating 0
                              // next lun User name, no terminating 0
                              // next lcn Cable name, no terminating 0

    WATCH_SPOOL_OFF   = 0xFE, // Not returned by the JTAG server but inserted into
                              // spool files to indicate where spool was turned off
                              // (rather than just abandoned due to ^C or similar)

    WATCH_FLUSH       = 0xFF  // We flushed data at this point.
};

#endif
