AM_CFLAGS = -DCCSP_INC_no_asm_sigcontext_h
AM_LDFLAGS = -L$(top_builddir)/../CcspCommonLibrary/.libs
AM_LDFLAGS += -lccsp_common
AM_LDFLAGS += -L/var/tmp/pc-rdkb/lib
AM_LDFLAGS += -ldbus-1
AM_LDFLAGS += -lthreadutil
AM_LDFLAGS += -lrt
AM_LDFLAGS += -lssl
AM_LDFLAGS += -lcrypto
AM_LDFLAGS += -lz

AM_CPPFLAGS = -Wall -Werror
ACLOCAL_AMFLAGS = -I m4
hardware_platform = i686-linux-gnu
bin_PROGRAMS = psmcli
psmcli_CPPFLAGS = -I/var/tmp/pc-rdkb/include/dbus-1.0 -I$(top_builddir)/../CcspCommonLibrary/source/ccsp/custom -I$(top_builddir)/../CcspCommonLibrary/source/ccsp/include -I$(top_builddir)/../CcspCommonLibrary/source/debug_api/include -I$(top_builddir)/../CcspCommonLibrary/source/cosa/include -I$(top_builddir)/../CcspCommonLibrary/source/cosa/include/linux -I$(top_builddir)/../CcspCommonLibrary/source/ccsp/components/include -I$(top_builddir)/../CcspCommonLibrary/source/cosa/package/slap/include $(CPPFLAGS)
psmcli_SOURCES = psmcli.c
psmcli_LDFLAGS =  -L$(top_builddir)/../CcspCommonLibrary/.libs -lccsp_common -L/var/tmp/pc-rdkb/lib -ldbus-1 -lthreadutil -lrt -lssl -lcrypto -lz