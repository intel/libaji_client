/****************************************************************************
 *   Copyright (c) 2004 by Intel Corporation                                *
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

 //START_MODULE_HEADER//////////////////////////////////////////////////////////
//
// Description: Altera standard macro definitions
//
// Authors:     Devin Sundaram
//
//              Copyright (c) Altera Corporation 2004.
//              All rights reserved.
//
//
//END_MODULE_HEADER////////////////////////////////////////////////////////////

#ifndef INC_AJI_MACROS_SYS_H
#define INC_AJI_MACROS_SYS_H

///// This checks to make sure "aji_sys.h" or "aji_macros_sys.h" is the first header included, coding std rule 61
#if !defined(AJI_NO_HEADER_FIRST_CHECK) && !defined(PDB_PARSING_PHASE) && !defined(AJI_NO_MEM_MANAGER)
#   if PORT==WINDOWS
#       if defined(_INC_STDIO) || defined(_INC_STDARG)
#           error "aji.h must be included before any other headers"
#       endif
#   elif PORT==UNIX && SYS==LINUX
#       if defined(_STDIO_H) || defined(_STDARG_H)
#           error "aji.h must be included before any other headers"
#       endif
#   endif // PORT
#endif // !AJI_NO_HEADER_FIRST_CHECK


///////////////////////////////////////////////////////////////////////////////
// Standard Includes //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if PORT==UNIX && SYS==LINUX
#pragma GCC visibility push(default)
#endif
#ifndef INC_CSTDIO
#include <cstdio>
#define INC_CSTDIO
#endif

#ifndef INC_CSTDLIB
#include <cstdlib>
#define INC_CSTDLIB
#endif

#if PORT==UNIX && SYS==LINUX
#pragma GCC visibility pop
#endif

#ifndef INC_AJI_GLIBC_SYS_H
#include "aji_glibc_sys.h"
#endif

// DLLIMPORT/DLLEXPORT //////////////////////////////////////////////////

#if PORT==WINDOWS

#   define DLLEXPORT __declspec(dllexport)
#   define DLLIMPORT __declspec(dllimport)


    // These are used in builtins.mak (since DLLEXPORT is defined by some
    // 3rd party header files, and we need to defined them differently
    // during the PDB_PARSING_PHASE below
#   define AJI_DLLPRIVATE
#   define AJI_DLLEXPORT __declspec(dllexport)
#   define AJI_DLLIMPORT __declspec(dllimport)
//  Override class linkage
//  (since you can't take address of a DLLEXPORTed function on Linux)
#   define AJI_DLLEXPORT_CALLBACK __declspec(dllexport)
#   define AJI_DLLIMPORT_CALLBACK __declspec(dllimport)
#   define AJI_CALLBACK
#   define AJI_NORETURN __declspec(noreturn)
#   define AJI_NOINLINE __declspec(noinline)
#   define AJI_PRINTF_ARGS(fmt_pos,args_pos)
//  Faster function calling convention:
#   if defined(PDB_PARSING_PHASE)
#       define AJI_DEPRECATED
#       define AJI_REMOVE_BY(x)
#       define AJI_DECL
#       define AJI_DECL_MFPTR
#       define NO_AJI_DECL  // Placeholder to not use AJI_DECL
#       define AJI_ALIGN(n)
#   else
#       define AJI_DEPRECATED __declspec(deprecated)
#       define AJI_REMOVE_BY(x) __declspec(deprecated("This deprecated function is targeted for removal in Quartus Prime " #x))
#       define AJI_DECL_IS_DEFINED // Used to know if AJI_DECL is actually defined
#       define AJI_DECL   // DISABLED -- will be removed soon (used to be __fastcall)
#       define AJI_DECL_MFPTR
#       define NO_AJI_DECL  // Placeholder to not use AJI_DECL
#       define AJI_ALIGN(n) __declspec(align(n))
#   endif

#else // UNIX
#   if defined(PDB_PARSING_PHASE)
#       define DLLIMPORT __declspec(dllimport)
#       define AJI_PRIVATE
#       define AJI_DLLEXPORT __declspec(dllexport)
#       define AJI_DLLIMPORT __declspec(dllimport)
#       define AJI_DLLEXPORT_CALLBACK __declspec(dllexport)
#       define AJI_DLLIMPORT_CALLBACK __declspec(dllimport)
#       define AJI_CALLBACK
#       define AJI_NORETURN
#       define AJI_NOINLINE
#       define AJI_PRINTF_ARGS(fmt_pos,args_pos)
#       define AJI_DEPRECATED
#       define AJI_REMOVE_BY(x)
#       define AJI_DECL
#       define AJI_DECL_MFPTR // fastcall+member_function_pointers doesn't work in GCC4.2.1
#       define NO_AJI_DECL  // Placeholder to not use AJI_DECL
#       define AJI_ALIGN(n)
#   elif __GNUC__ >= 4
#       if __GNUC__ == 4
#           if __GNUC_MINOR__ >=5 && __GNUC_MINOR__ <=8
#               define DLLEXPORT __attribute__((visibility("default")))
#               define AJI_DLLEXPORT __attribute__((visibility("default")))
#           else
#               define DLLEXPORT __attribute__((visibility("protected")))
#               define AJI_DLLEXPORT __attribute__((visibility("protected")))
#           endif
#       elif __GNUC__ >= 5
#           define DLLEXPORT __attribute__((visibility("default")))
#           define AJI_DLLEXPORT __attribute__((visibility("default")))
#       endif
#       define DLLIMPORT __attribute__((visibility("default")))
#       define AJI_DLLPRIVATE __attribute__((visibility("hidden")))
#       define AJI_DLLIMPORT __attribute__((visibility("default")))
//      A function that has its address taken must have "default" linkage on Linux
#       define AJI_DLLEXPORT_CALLBACK __attribute__((visibility("default")))
#       define AJI_DLLIMPORT_CALLBACK __attribute__((visibility("default")))
#       define AJI_CALLBACK __attribute__((visibility("default")))
#       define AJI_NORETURN __attribute__((noreturn))
#       define AJI_NOINLINE __attribute__((noinline))
#       define AJI_PRINTF_ARGS(fmt_pos,args_pos) __attribute__((format (printf, fmt_pos, args_pos)))
#       define AJI_DEPRECATED __attribute__((deprecated))
#       define AJI_REMOVE_BY(x) __attribute__((deprecated("This deprecated function is targeted for removal in Quartus Prime " #x)))
#       define AJI_DECL // __attribute__((regparm(3)))
#       define AJI_DECL_MFPTR // fastcall+member_function_ pointers doesn't work in GCC4.2.1
#       define NO_AJI_DECL  // Placeholder to not use AJI_DECL
#       define AJI_ALIGN(n) __attribute__((aligned(n)))
#   else
#       define DLLEXPORT
#       define DLLIMPORT
#       define AJI_DLLPRIVATE
#       define AJI_DLLEXPORT
#       define AJI_DLLIMPORT
#       define AJI_DLLEXPORT_CALLBACK
#       define AJI_DLLIMPORT_CALLBACK
#       define AJI_CALLBACK
#       define AJI_NORETURN
#       define AJI_NOINLINE
#       define AJI_PRINTF_ARGS(fmt_pos,args_pos)
#       define AJI_DEPRECATED
#       define AJI_REMOVE_BY(x)
#       define AJI_DECL // __attribute__((regparm(3)))
#       define AJI_DECL_MFPTR
#       define NO_AJI_DECL  // Placeholder to not use AJI_DECL
#       define AJI_ALIGN(n) __attribute__((aligned(n)))
#   endif // PDB_PARSING_PHASE

#endif // PORT

// *****************************************
// Note that these rules arrived at by looking at VC10 and GCC 4.5.3.  They may
// need revision for future compilers.
// *****************************************
//
// Pure abstract classes have special requirements for being optimally
// exported from a DLL.
//
// The class itself should not be decorated with __declspec(dllexport) on
// Windows as this impedes inlining of trivial functions after PGO.  However,
// Linux requires that the class be decorated with
// __attribute__((visibility("default"))) in *all* translation units in order
// to ensure that the vtable and typeinfo don't become hidden after linking
// (due to -fvisibility=hidden).
//
// As always, prefer to define trivial functions inline.
//
// From a build-break-avoidance perspective, this is particularly important for
// PDB-writable pure abstract classes as their typeinfo is required.  For
// example, the following is a correctly decorated pure abstract class:
//
//    class AJI_PURE_ABSTRACT_CLASS TAPI_RE_DELAY_TREE_NODE
//    : PDB_ABSTRACT_BASE(TAPI_RE_DELAY_TREE_NODE)
//    {
//      PDB_ABSTRACT_BASE_CLASS(PDB_DDB_SEG, TAPI_RE_DELAY_TREE_NODE, 1);
//    public:
//       // Destructor
//       virtual ~TAPI_RE_DELAY_TREE_NODE() { };
//
//       // Add an existing child node to my list of children
//       virtual void add_child(TAPI_RE_DELAY_TREE_NODE *child) = 0;
//       .
//       .
//       .
//    };
//
// If the pure abstract class has non-inline members (e.g. some non-trivial
// virtual functions), those must be marked as AJI_DLLEXPORT as usual.  This
// will mark the function as __declspec(dllexport) in the translation unit that
// defines the function and __declspec(dllimport) in all other translation
// units on Windows; and it will mark the function as
// __attribute__((visibility("default"))) in all translation units on Linux.
//
// You might be wondering why we care about inlining of trivial functions in a
// pure abstract class.  Note that wherever the called function can be
// statically determined, the compiler can inline.  For example:
//
//    struct A {
//      virtual void abstract() = 0;
//      vector<int> vi;
//    };
//    struct B : public A {
//      virtual void abstract() {}
//    };
//
// In this case, exporting at the class level will prevent inlining the
// destructor for vi into ~B().  Even if A has no members, ~B() will include a
// call to ~A() -- which is an empty function -- after PGO on Windows if A is
// marked dllimport at the class level.  Similarly, there's an
// implicitly-generated constructor for both A and B; these will both be
// inlined *only* if the class is not imported.  Note that it's specifically
// *import* -- the inlining will occur in instances of the abstract base class
// that appear in the exporting DLL, they just won't be inlined in importing
// DLLs.
//
// This change also turns out to be useful for generating debug symbols on
// Windows for some esoteric (and as-yet-unanalyzed) issue.
//
// Note that PDB parsing can't handle the presence of __attribute__.
//
#if defined(PDB_PARSING_PHASE) || PORT==WINDOWS
#   define AJI_PURE_ABSTRACT_CLASS
#else // UNIX, not during PDB parsing
#   define AJI_PURE_ABSTRACT_CLASS __attribute__((visibility("default")))
#endif

#define AJI_CACHE_ALIGN AJI_ALIGN(128)

// The default linkage for C-runtime library & callback functions to the CRT library
#define AJI_C_DECL
#define AJI_STD_CDECL AJI_C_DECL

#if defined(__cplusplus)
#   define AJI_EXTERN_C extern "C"
#else
#   define AJI_EXTERN_C
#endif

// Helper macros //////////////////////////////////////////////////////////////

// Two macros are needed to get the correct behaviour from the pre-processor; see
// http://gcc.gnu.org/onlinedocs/cpp/Argument-Prescan.html
#define AJI_STRINGIFY_(x) #x
#define AJI_STRINGIFY(x) AJI_STRINGIFY_(x)

#define AJI_PASTE_( x, y ) x ## y
#define AJI_PASTE( x, y ) AJI_PASTE_( x, y )

// Default module prefix //////////////////////////////////////////////////////

// this is a little trick from mainwin's init.C so we can pass the code prefix
// on the cmdline (as CODE_PREFIX_RAW) without any quotes
#ifdef COMMON_CODE_PREFIX_RAW
#   define COMMON_CODE_PREFIX AJI_STRINGIFY(COMMON_CODE_PREFIX_RAW)
#endif
#ifdef CODE_PREFIX_RAW
#   define CODE_PREFIX AJI_STRINGIFY(CODE_PREFIX_RAW)
#endif

// Every DLL or *.so should be built using a different CODE_PREFIX.  If a subsytem
// builds multiple DLLs, each DLL should be built in a sub-makefile using a different
// CODE_PREFIX.  This is because, on Linux, functions defined with DLLEXPORT *must*
// be defined in the translation unit (unlike on Windows)
#ifndef CODE_PREFIX
#   define CODE_PREFIX ""
#endif
// If a subsystem builds multiple DLLs that share OBJ files, the makefile should
// define COMMON_CODE_PREFIX, which will define $(COMMON_CODE_PREFIX)_DLLEXPORT.
// This prefix can then me used for functions and classes in the *.cpp files that
// build those OBJs -- just as $(CODE_PREFIX)_DLLEXPORT is used for *.cpp files
// specific to a single DLL file.
#ifndef COMMON_CODE_PREFIX
#   define COMMON_CODE_PREFIX ""
#endif
///////////////////////////////////////////////////////////////////////////////
// TYPES //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus) && defined(MODE_64_BIT)
#   if PORT==WINDOWS

#       ifdef NULL
#           undef NULL
#       endif
#       define NULL 0LL

#   endif // PORT
#endif // __cplusplus && MODE_64_BIT

// All Windows types that we need
#ifndef INC_WIN_TYPE_SYS_H
#include "win_type_sys.h"
#endif

// need signed versions of Win32 capitalized types ////////////////////////////
typedef signed char         SBYTE;
typedef signed short        SWORD;
#if PORT==UNIX && defined(MODE_64_BIT)
    typedef signed int          SDWORD;
#else
    typedef signed long         SDWORD;
#endif

#ifndef INC_CSTDINT
#include <cstdint>
#define INC_CSTDINT
#endif

typedef uint64_t QWORD;
typedef int64_t  SQWORD;

// Codes for printf ///////////////////////////////////////////////////////////

//// QWORD

#if PORT==WINDOWS
#   define QWORD_SIZE_FORMAT    "I64"
#else
#   define QWORD_SIZE_FORMAT    "ll"
#endif

#define QWORD_uFORMAT       "%" QWORD_SIZE_FORMAT "u"
#define QWORD_dFORMAT       "%" QWORD_SIZE_FORMAT "d"
#define QWORD_oFORMAT       "%" QWORD_SIZE_FORMAT "o"
#define QWORD_xFORMAT       "%" QWORD_SIZE_FORMAT "x"
#define QWORD_XFORMAT       "%" QWORD_SIZE_FORMAT "X"

// Defaults
#define QWORD_FORMAT        QWORD_uFORMAT
#define SQWORD_FORMAT       QWORD_dFORMAT

//// size_t

#ifndef MODE_64_BIT
    // All 32-bit platforms have size_t == unsigned int
#   define SIZET_SIZE_FORMAT ""
#elif PORT==WINDOWS
    // WIN64: size_t == unsigned __int64
#   define SIZET_SIZE_FORMAT QWORD_SIZE_FORMAT
#else
    // UNIX64: size_t == unsigned long && long == 64-bits
#   define SIZET_SIZE_FORMAT "l"
#endif

#define SIZET_uFORMAT       "%" SIZET_SIZE_FORMAT "u"
#define SIZET_oFORMAT       "%" SIZET_SIZE_FORMAT "o"
#define SIZET_xFORMAT       "%" SIZET_SIZE_FORMAT "x"
#define SIZET_XFORMAT       "%" SIZET_SIZE_FORMAT "X"

// The bullseye codecoverage compiler doesn't recognize the "ULL" suffix:
#if PORT==WINDOWS
#define AJI_ULL_CONSTANT( x ) AJI_PASTE(x,ui64)
#define AJI_LL_CONSTANT( x ) AJI_PASTE(x,i64)
#else
#define AJI_ULL_CONSTANT( x ) AJI_PASTE(x,ull)
#define AJI_LL_CONSTANT( x ) AJI_PASTE(x,ll)
#endif

// Defaults
#define SIZET_FORMAT SIZET_uFORMAT

// Pointer types //////////////////////////////////////////////////////////////

#if PORT==WINDOWS

#   ifndef INC_BASETSD_H
#      include <basetsd.h>
#      define INC_BASETSD_H
#   endif
    typedef UINT_PTR    AJI_UINTPTR;
    typedef INT_PTR     AJI_INTPTR;

#else
// Without this define, macros such as PRIx64, PRIu64,... are not included from
// inttypes.h. 

/* The correct solution for C++ code is to include <cinttypes>
// instead, but aji_macros_sys.h is also included by C code, so we can't do
// that, because C code compilation fails with <cinttypes> included.
// See https://stackoverflow.com/a/8132440
*/
#    define __STDC_FORMAT_MACROS
#    ifndef INC_INTTYPES_H
#       include <cinttypes>
#       define INC_INTTYPES_H
#    endif
    typedef uintptr_t   AJI_UINTPTR;
    typedef intptr_t    AJI_INTPTR;

#endif // WINDOWS

// Type sizes /////////////////////////////////////////////////////////////////

enum INT_TYPE_BITS_LEN {BYTE_BITS=8,
                        QWORD_BITS=sizeof(QWORD)*8,
                        DWORD_BITS=QWORD_BITS/2,
                        WORD_BITS=DWORD_BITS/2};

#define MIN_QWORD  ((QWORD) 0)
#define MAX_QWORD  ((QWORD) _UI64_MAX)

#define MIN_SQWORD ((SQWORD) _I64_MIN)
#define MAX_SQWORD ((SQWORD) _I64_MAX)

// Endian /////////////////////////////////////////////////////////////////////

#if PORT==UNIX && SYS==LINUX

    // On Linux, these are #defined in /usr/include/endian.h which is
    // included from ctype.h. Need to #undef these for Quartus
    // Need to include endian.h here to make sure we undef its defs AFTER it
    // is included
#   ifndef INC_ENDIAN_H
#       include <endian.h>
#       define INC_ENDIAN_H
#   endif
#   ifdef BYTE_ORDER
#       undef BYTE_ORDER
#   endif
#   ifdef LITTLE_ENDIAN
#       undef LITTLE_ENDIAN
#   endif
#   ifdef BIG_ENDIAN
#       undef BIG_ENDIAN
#   endif

#endif // UNIX && LINUX

/* __builtin_expect tells the compiler the likely result of
 * given expression. Used to almost completely eliminate the cost
 * of asserts. Only works for GCC.
 * !! in front of expr is used to map the value into either 0 or 1.
 *
 * This macro is also defined in fitter/vpr20k/vpr_common/my_utils.h.
 * Keep the two defn's in synch!
 */
#if PORT==UNIX && SYS==LINUX
#   define AJI_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))
#   define AJI_LIKELY(expr)   (__builtin_expect(!!(expr), 1))
#else
#   define AJI_UNLIKELY
#   define AJI_LIKELY
#endif


// Macro for declaring a static object to be in thread-local storage:
// Example:
//      AJI_TLS int my_count;
#if PORT==WINDOWS
#   define AJI_TLS __declspec( thread )
#else
#   define AJI_TLS __thread
#endif

#endif // INC_AJI_MACROS_SYS_H
