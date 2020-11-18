/****************************************************************************
 *   Copyright (c) 2002 by Intel Corporation                                *
 *   author: Yiew, Chin Koe                                                 *
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
// Description: This file defines types commonly associated with Windows
//                  and used by Quartus. Some are 
//
// Authors:     Yiew Chin Koe
//
//                  Copyright (c) Altera Corporation 2002.
//                  All rights reserved.
//
//
// END_MODULE_HEADER///////////////////////////////////////////////////////////

// INTERFACE DESCRIPTION //////////////////////////////////////////////////////
//
// This section is optional. Use this for explanatory text
// describing the interface contained within this file.

#ifndef INC_WIN_TYPE_SYS_H
#define INC_WIN_TYPE_SYS_H

// INCLUDE FILES //////////////////////////////////////////////////////////////
//
// Include files in the following order below the
// corresponding headers.
//

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#if PORT == WINDOWS
#ifndef INC_WINSOCK2_H
#include <winsock2.h>
#define INC_WINSOCK2_H
#endif

#ifndef INC_WINDOWS_H
#include <Windows.h>
#define INC_WINDOWS_H
#endif
#endif // PORT == WINDOWS

// STANDARD INCLUDE FILES
#ifndef INC_CLIMITS
#include <climits> // for min & max type sizes
#endif

// HEADER FILES FROM quartus/h
// HEADER FILES FROM quartus/<my_system>/h
// LOCAL INCLUDE FILES FROM WITHIN MY SUBSYSTEM

///////////////////////////////////////////////////////////////////////////////
// Basic Windows Types And Defines ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// We can't duplicate these defs if windef.h is already included, Only a conflict in C code
#if (!defined(_WINDEF_) && !defined(MAINWIN)) || defined(__cplusplus)

// BOOL ///////////////////////////////////////////////////////////////////////

typedef int BOOL;
#ifndef TRUE
#   define TRUE         1
#endif
#ifndef FALSE
#   define FALSE        0
#endif

// Uppercase types ////////////////////////////////////////////////////////////

#include <cstdint>

typedef char          CHAR;
typedef unsigned char UCHAR;
typedef uint8_t       BYTE;

typedef short   	SHORT;
typedef unsigned short  USHORT;
typedef uint16_t  	WORD;

typedef int32_t      INT;
typedef uint32_t    UINT;

#if PORT==UNIX && defined(_WIN64)
// MainWin 5.1.1 64-bit treat LONG and ULONG as 4-bytes
typedef int             LONG;
typedef unsigned int    ULONG;
#else
typedef long            LONG;
typedef unsigned long   ULONG;
#endif

#ifndef INC_CSTDINT
#include <cstdint>
#define INC_CSTDINT
#endif

/* In MinGW, you will get a conflict warning for
 * DWORD because MinGW's minwindef.h includes
 * its own definition of DWORD.
 * The purpose is to handle realign 
 * Cygwin's native datatype to Window's (and
 * our's) expection of DWORD as a 4 byte 
 * unsigned int datatype.
 * 
 * minwindef.h is #include-d in windows.h and 
 * indirectly from winsock2.h.
 *
 * MinGW uses include guard _MINWINDEF_ for
 * minwindef.h so will test for MINGW using the 
 * include guard. This means winsock2.h and
 * windows.h must be #include-d first. As
 * per best practice they are #include-d at
 * the beginning of the file, before all
 * other #includes
 */

#ifndef _MINWINDEF_
typedef uint32_t DWORD;
#endif // _MINWINDEF_


#endif // !_WINDEF_ || __cplusplus

///////////////////////////////////////////////////////////////////////////////
// From here on down is just for UNIX when not using Mainwin
#if PORT==UNIX && !defined(WIN32)
///////////////////////////////////////////////////////////////////////////////

// Size of WPARAM need to be same as size of pointer
#ifdef MODE_64_BIT
typedef unsigned long WPARAM;
#else
typedef UINT WPARAM;
#endif
typedef long LPARAM;

typedef long LRESULT;
typedef char TCHAR;
typedef void* LPVOID;
typedef const char* LPCSTR;

// __int64 ////////////////////////////////////////////////////////////////////

#ifdef NO_INT64_TYPEDEFS

#   ifndef __int64
#       ifdef MODE_64_BIT
#           define __int64 long
#       else
#           define __int64 long long
#       endif // MODE_64_BIT
#   endif // __int64

#else

#   if !defined(__INTEL_COMPILER)
#       ifdef MODE_64_BIT
            typedef long __int64;
#       else
            typedef long long __int64;
#       endif // MODE_64_BIT
#   endif

#endif // NO_INT64_TYPEDEFS

typedef __int64 _int64;

// time_t /////////////////////////////////////////////////////////////////////
// Needed by _finddata_t struct decl below

// _TIME_T is for solaris&hp, __time_t_defined is for linux
#if ! defined(_TIME_T) && ! defined(__time_t_defined)

typedef long time_t;
#if SYS==LINUX
#define __time_t_defined
#else
#define _TIME_T
#endif

#endif // _TIME_T

typedef unsigned long _fsize_t;

// Min/Max values ///////////////////////////////////////////////////////

#define MAXBYTE     UCHAR_MAX
#define MAXWORD     USHRT_MAX
#define MAXDWORD    UINT_MAX

#ifdef MODE_64_BIT 
#define _UI64_MAX    ULONG_MAX
#define _I64_MAX     LONG_MAX
#define _I64_MIN     LONG_MIN
#else
#define _UI64_MAX    ULONG_LONG_MAX
#define _I64_MAX     LONG_LONG_MAX
#define _I64_MIN     LONG_LONG_MIN
#endif // MODE_64_BIT

// __T ////////////////////////////////////////////////////////////////////////

// see tchar.h on windows for __T def
#ifdef _UNICODE
#define __T(x)      L ## x
#else
#define __T(x)      x
#endif
#define _T(x)      __T(x)

// Path lengths ///////////////////////////////////////////////////////////////

#define _MAX_PATH       1024 /* Maximum length of full path */
#define _MAX_FNAME      256  /* Maximum length of file name */
#define _MAX_EXT        256  /* Maximum length of extension */
#define _MAX_DRIVE      3    /* Maximum length of drive */
#define _MAX_DIR        1024 /* Maximum length of directory */
#define MAX_PATH        _MAX_PATH

// Linkage ////////////////////////////////////////////////////////////////////

#define __cdecl
#define __fastcall
#define _fastcall
#define WINAPI
#define PASCAL

///////////////////////////////////////////////////////////////////////////////
// Function mapping from Windows name to UNIX name ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define _stat     stat
#define _fstat    fstat

#ifdef stricmp
#undef stricmp
#endif
#define stricmp strcasecmp

#ifdef strnicmp
#undef strnicmp
#endif
#define strnicmp strncasecmp

///////////////////////////////////////////////////////////////////////////////
// Windows GUI & WIN32 Types And Defines ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// This is for gen_formatmessage function, copied from FormatMessage
#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_IGNORE_INSERTS  0x00000200
#define FORMAT_MESSAGE_FROM_STRING     0x00000400
#define FORMAT_MESSAGE_FROM_HMODULE    0x00000800
#define FORMAT_MESSAGE_FROM_SYSTEM     0x00001000
#define FORMAT_MESSAGE_ARGUMENT_ARRAY  0x00002000
#define FORMAT_MESSAGE_MAX_WIDTH_MASK  0x000000FF

// Dialog Box Command IDs used in msg_sys.h
#ifndef IDOK
#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
#define IDCLOSE             8
#define IDHELP              9
#endif

// Needed by arc_messenger.h since non-gui part posts windows messages
// using virtual interface to ARC_GUI_MESSENGER
#define WM_USER             0x0400

// GUID/CLSID stuff needed by FIO for mapping to FIO_FILE_TYPE
typedef struct _GUID
{
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];
} GUID;
typedef GUID CLSID;
#ifdef __cplusplus

#ifndef INC_CSTRING
#include <cstring>
#define INC_CSTRING
#endif

inline int operator ==(const GUID& lhs, const GUID& rhs)
{
    return !memcmp(&lhs, &rhs, sizeof(GUID));
}

inline int operator !=(const GUID& lhs, const GUID& rhs)
{
    return !(lhs == rhs);
}
#endif

// Needed by GIO since it is used in both GUI & non-GUI
typedef struct tagRECT
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECT;
typedef struct tagPOINT
{
    LONG  x;
    LONG  y;
} POINT;
typedef struct tagSIZE
{
    LONG        cx;
    LONG        cy;
} SIZE;

// Needed by FIO_FILE_SYS's find routines (findfirst, findnext, ...)
struct _finddata_t
{
    unsigned    attrib;
    time_t      time_create;    /* -1 for FAT file systems */
    time_t      time_access;    /* -1 for FAT file systems */
    time_t      time_write;
    _fsize_t    size;
    char        name[260];
};
// File attribute constants for _findfirst()
#define _A_NORMAL   0x00    /* Normal file - No read/write restrictions */
#define _A_RDONLY   0x01    /* Read only file */
#define _A_HIDDEN   0x02    /* Hidden file */
#define _A_SYSTEM   0x04    /* System file */
#define _A_SUBDIR   0x10    /* Subdirectory */
#define _A_ARCH     0x20    /* Archive file */

///////////////////////////////////////////////////////////////////////////////
#endif // UNIX && !WIN32
///////////////////////////////////////////////////////////////////////////////

#endif // INC_WIN_TYPE_SYS_H
