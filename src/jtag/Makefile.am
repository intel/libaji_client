lib_LTLIBRARIES = %D%/libaji_client.la
bin_PROGRAMS = %D%/jtagquery


#####################
# libaji_client
#####################

%C%_libaji_client_la_SOURCES = 
%C%_libaji_client_la_CPPFLAGS =
%C%_libaji_client_la_LDFLAGS = 
%C%_libaji_client_la_LDLIBS =
%C%_libaji_client_la_LIBADD =

%C%_libaji_client_la_CPPFLAGS += -DLITTLE=1 -DBIG=2 -DENDIAN=LITTLE
%C%_libaji_client_la_CPPFLAGS += -DWINDOWS=1 -DUNIX=2 -DLINUX=3

if BUILD_WINDOWS
%C%_libaji_client_la_CPPFLAGS += -DPORT=WINDOWS -DSYS=WINDOWS
else
%C%_libaji_client_la_CPPFLAGS += -DPORT=UNIX -DSYS=LINUX
endif

# Compile: For configuring AJI Client
%C%_libaji_client_la_CPPFLAGS  += \
    -DMODE_64_BIT \
    -DJTAG_DLLEXPORT=AJI_DLLEXPORT \
    -D_GLIBCXX_USE_CXX11_ABI=0 \
    -fvisibility=hidden
    
# Compile: For optimizing AJI Client
%C%_libaji_client_la_CPPFLAGS  += \
    -O1 \
    -D_GNU_SOURCE -D_REENTRANT

# Compile: Preferred flags used for optimizing AJI Client
%C%_libaji_client_la_CPPFLAGS  += \
    -Wno-error=coverage-mismatch -ggdb1 -mmmx -msse \
    -fno-strict-aliasing -finline -fno-inline-functions -fno-builtin -momit-leaf-frame-pointer -fomit-frame-pointer \
    -funwind-tables -Wno-system-headers -Wno-extra -Wno-switch -Wno-unused-function -Wno-unknown-pragmas \
    -Wno-comment -Wno-parentheses -Wno-strict-overflow -Werror -Wno-error=deprecated-declarations -fopenmp \
    -Wno-misleading-indentation \
    -Wno-deprecated -Wno-attributes -Wno-unused-but-set-variable -Wno-nonnull-compare -Wno-unused-const-variable \
    -flifetime-dse=1 -frecord-gcc-switches \
    -Werror=delete-incomplete

#Compile: Others
%C%_libaji_client_la_CPPFLAGS  += \
    -Wformat -Wformat-security -Werror=format-security \
    -D_FORTIFY_SOURCE=2 \
    -fPIE \
    -fPIC -shared \
    \
    -fstack-clash-protection \
    -Wall \
    \
    -mindirect-branch-register \
    -mindirect-branch=thunk \
    -mfunction-return=thunk

#Compile: Include directories
%C%_libaji_client_la_CPPFLAGS  += \
    -I$(abs_top_srcdir)/src/h/


%C%_libaji_client_la_SOURCES += \
    %D%/../h/aji_sys.h \
    %D%/../h/aji.h \
    %D%/../h/aji_glibc_sys.h \
    %D%/../h/aji_macros_sys.h \
    %D%/../h/gen_string_sys.h  \
    %D%/../h/secure_getenv.h \
    %D%/../h/win_type_sys.h  \
    \
    %D%/jtag_client_aji.cpp     %D%/jtag_client_aji.h \
    %D%/jtag_client_link.cpp    %D%/jtag_client_link.h  \
    %D%/jtag_platform.h  \
    %D%/jtag_configuration.cpp  %D%/jtag_configuration.h  \
    %D%/jtag_tcpclient.cpp      %D%/jtag_tcpclient.h  \
    %D%/jtag_tcplink.cpp        %D%/jtag_tcplink.h  \
    %D%/jtag_tcpsock.cpp        %D%/jtag_tcpsock.h  \
    %D%/jtag_message.h  \
    %D%/jtag_protocol.cpp       %D%/jtag_protocol.h  \
    %D%/jtag_common.h  \
    %D%/jtag_client_chain.cpp   %D%/jtag_client_chain.h  \
    %D%/jtag_client_open.cpp    %D%/jtag_client_open.h  \
    %D%/jtag_raw_fifo.cpp       %D%/jtag_raw_fifo.h  \
    %D%/jtag_client_hub.cpp     %D%/jtag_client_hub.h  \
    %D%/jtag_md5_pd.cpp        %D%/jtag_md5_pd.h  \
    %D%/jtag_utils.cpp          %D%/jtag_utils.h  \
    %D%/jtag_client_node.cpp  \
    %D%/jtag_configuration_file.cpp     %D%/jtag_configuration_file.h  \
    %D%/jtag_configuration_memory.cpp   %D%/jtag_configuration_memory.h  


if BUILD_WINDOWS
%C%_libaji_client_la_SOURCES += %D%/jtag_platform_win.cpp \
	%D%/jtag_configuration_win.cpp %D%/jtag_configuration_win.h \
    %D%/strnlen_s.cpp %D%/strnlen_s.h 


## to spport building win32-dll
## See https://autotools.io/libtool/windows.html
%C%_libaji_client_la_LDFLAGS += -no-undefined -avoid-version
%C%_libaji_client_la_LIBADD += -lws2_32

else##

%C%_libaji_client_la_SOURCES += \
    %D%/jtag_platform_unix.cpp
#    %D%/../../3rdparties/safestringlib/safeclib/libSafeString.la

%C%_libaji_client_la_LIBADD += \
%D%/../../3rdparties/safestringlib/safeclib/libSafeString.la

endif

%C%_libaji_client_la_CPPFLAGS +=  \
    $(AM_CPPFLAGS) \
   -I%D%/../../3rdparties/safestringlib/include

if BUILD_WINDOWS
##(windows) Compile: Others
%C%_libaji_client_la_CPPFLAGS  += \
    -fno-stack-protector

##(windows) Link: Others
%C%_libaji_client_la_LDFLAGS += \
     -Wl,--dynamicbase,--export-dynamic \
     -Wl,--nxcompat

%C%_libaji_client_la_LDFLAGS +=  \
        -lkernel32 \
        -luser32 \
        -lgdi32 \
        -lnetapi32 \
        -ladvapi32 \
        -lcomctl32 \
        -lcomdlg32 \
        -lshell32 \
        -lws2_32 \
        -lssp \
        -static-libgcc \
        -static-libstdc++

else

##(linux) Compile: Others
%C%_libaji_client_la_CPPFLAGS  += \
    -fstack-protector-strong

##(linux) Link: Others
%C%_libaji_client_la_LDFLAGS += \
    -z relro \
    -z now \
    -z noexecstack \
    \
    -pie \
    \
    -rdynamic

%C%_libaji_client_la_LDFLAGS +=  \
        -Wl,--verbose \
        -fopenmp \
        -static-libgcc \
        -static-libstdc++ \
        -ldl 

%C%_libaji_client_la_LDFLAGS += -Wl,-fuse-ld=gold 

endif


#####################
#jtagquery
#####################

%C%_jtagquery_SOURCES =
%C%_jtagquery_CPPFLAGS =
%C%_jtagquery_LDFLAGS = 
%C%_jtagquery_LDADD =
%C%_jtagquery_LDLIBS =

%C%_jtagquery_CPPFLAGS += \
    $(AM_CPPFLAGS) \
    $(%C%_libaji_client_la_CPPFLAGS)

%C%_jtagquery_LDFLAGS += \
    -all-static \
    $(%C%_libaji_client_la_LDFLAGS)


%C%_jtagquery_LDADD += %D%/libaji_client.la

if BUILD_WINDOWS
%C%_jtagquery_SOURCES += \
    %D%/jtag_config_status_win.cpp

else

%C%_jtagquery_LDADD += $(top_builddir)/3rdparties/safestringlib/safeclib/libSafeString.la

endif

%C%_jtagquery_SOURCES += \
    %D%/jtagconfig.cpp \
    %D%/ver_sys.cpp  %D%/ver_sys.h

