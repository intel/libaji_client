SUBDIRS =
DIST_SUBDIRS = 
AM_CPPFLAGS = 
AM_LDFLAGS =

if ! BUILD_WINDOWS
    SUBDIRS += 3rdparties/safestringlib
    DIST_SUBDIRS += 3rdparties/safestringlib

    AM_CPPFLAGS += \
        $(HOST_CPPFLAGS) \
        -I$(abs_top_srcdir)/3rdparties/safestringlib/include
endif



BAD_CFLAGS= \
    -I %D%/p4/quartus/h
    
include src/jtag/Makefile.am

