AC_INIT([libaji_client], [0.2.0], [cinlyooi@intel.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_PROG_AR

LT_INIT
AC_SUBST([LIBTOOL_DEPS])

AC_CONFIG_SRCDIR([src/jtag/jtag_client_aji.cpp])

AC_LANG([C++])
AC_PROG_RANLIB
AC_PROG_CC
AC_PROG_CXX

## for each xxx below, we will have the following:
##     - is_xxx (all lowercase) for use in configure.ac and is 
##         the reference variable for the rest
##     - BUILD_XXX (all uppercase for use by Makefile.am.
##         Symbol defined if is_XXX is true.
##     - IS_XXX (all uppercase) defined for use by source files.
##         Value is 0 or 1.

is_windows="no"
is_linux="no"
is_mingw="no"

AC_CANONICAL_HOST

AS_CASE([$host_os],
    [*mingw* | *msys*], [
        is_windows="yes"
        is_mingw="yes"
        AC_SUBST([HOST_CPPFLAGS], [-D__USE_MINGW_ANSI_STDIO])
    ],
    [linux*],[
	    is_linux="yes"
    ],
    [
	    AC_MSG_ERROR([Not yet capable of compiling for $host_os])
    ]
)
AM_CONDITIONAL([BUILD_WINDOWS], [test "x${is_windows}" = "xyes"])
AM_CONDITIONAL([BUILD_LINUX], [test "x${is_linux}" = "xyes"])
AM_CONDITIONAL([BUILD_MINGW], [test "x${is_mingw}" = "xyes"])

AS_IF([test "x${is_windows}" = "xyes"],
  [AC_DEFINE("IS_WINDOWS", [1], [1 to build for Windows platform])],
  [AC_DEFINE("IS_WINDOWS", [0], [1 to not build for Windows platform])]
)
AS_IF([test "x${is_linux}" = "xyes"],
  [AC_DEFINE("IS_LINUX", [1], [1 to build for Linux platform])],
  [AC_DEFINE("IS_LINUX", [0], [1 to not build for Linux platform])]
)

AS_IF([test "x${is_mingw}" = "xyes"],
  [AC_DEFINE("IS_MINGW", [1], [1 to build for Windows platform with MinGW])],
  [AC_DEFINE("IS_MINGW", [0], [1 to not build for Windows platform with MinGW])]
)

if test "x${is_windows}" != "xyes"; then
    AC_MSG_CHECKING([Checking for safestringlib ...]) 
    if test -f "3rdparties/safestringlib/include/safe_lib.h" ;
    then
       AC_MSG_RESULT([yes])
    else
         AC_MSG_RESULT([no])
         AC_MSG_ERROR([Cannot find safestringlib, run git submodule update --init --recursive])
    fi
    AC_CONFIG_FILES([3rdparties/safestringlib/Makefile 3rdparties/safestringlib/safeclib/Makefile 3rdparties/safestringlib/unittests/Makefile])
fi

## Note, I used a standard if statmenet because I cannot check for SafeString library using standard AC*()
## instruction.
## config.log shows that even with
##    LD_FLAGS='-L ../git.openocd-jtagserv/src/jtag/minijtagserv/lib64'  ./configure
## -L were not passed to the c command line testing for SafeString library
## A secondary problem is the program constructed to test for memcmp_s() will not take parameters.
#AC_SEARCH_LIBS([memcmp_s], [SafeString], [],[AC_MSG_ERROR([libSafeString.so/SafeString.dll not found])])
#AC_CHECK_LIB([SafeString], [memcmp_s], [],[AC_MSG_ERROR([libSafeString.so/SafeString.dll not found])])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/jtag/Makefile])

AH_TOP([#pragma once])
AC_OUTPUT
