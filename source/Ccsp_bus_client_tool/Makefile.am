AM_CFLAGS = $(DBUS_CFLAGS)
AM_CFLAGS += -DCCSP_INC_no_asm_sigcontext_h
AM_LDFLAGS = -L/usr/lib/i386-linux-gnu
AM_LDFLAGS += -ldbus-1
AM_LDFLAGS += -lpthread
AM_LDFLAGS += -lrt
AM_LDFLAGS += -L$(top_builddir)/../CcspCommonLibrary/.libs
AM_LDFLAGS += -lccsp_common

AM_CPPFLAGS = -Wall -Werror
ACLOCAL_AMFLAGS = -I m4
hardware_platform = i686-linux-gnu
bin_PROGRAMS = dmcli
dmcli_CPPFLAGS = -I$(top_builddir)/../CcspCommonLibrary/source/ccsp/custom -I$(top_builddir)/../CcspCommonLibrary/source/ccsp/include -I$(top_builddir)/../CcspCommonLibrary/source/debug_api/include -I$(top_builddir)/../CcspCommonLibrary/source/cosa/include -I$(top_builddir)/../CcspCommonLibrary/source/cosa/include/linux -I$(top_builddir)/../CcspCommonLibrary/source/ccsp/components/include -I$(top_builddir)/../CcspCommonLibrary/source/cosa/package/slap/include $(CPPFLAGS)
dmcli_SOURCES = ccsp_message_bus_client_tool.c
dmcli_LDFLAGS =  -L/usr/lib/i386-linux-gnu -ldbus-1 -lpthread -lrt -L$(top_builddir)/../CcspCommonLibrary/.libs -lccsp_common