AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER
AM_CFLAGS += -D_ANSC_LITTLE_ENDIAN_
AM_CFLAGS += -DCCSP_INC_no_asm_sigcontext_h
AM_LDFLAGS = -lccsp_common
AM_LDFLAGS += -lpthread
AM_LDFLAGS += -lrt
AM_LDFLAGS +=
AM_LDFLAGS += -lwebconfig_framework
AM_LDFLAGS += -lz

AM_CPPFLAGS = -Wall -Werror
ACLOCAL_AMFLAGS = -I m4
hardware_platform = i686-linux-gnu
bin_PROGRAMS = wbCfgTestApp
wbCfgTestApp_CPPFLAGS = -I$(top_srcdir)/../CcspCommonLibrary/WebConfig_Framework -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/custom -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/include -I$(top_srcdir)/../CcspCommonLibrary/source/debug_api/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/include/linux -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/package/slap/include $(CPPFLAGS)
wbCfgTestApp_LDFLAGS = -lwebconfig_framework -lccsp_common -lpthread -lrt -lz
wbCfgTestApp_SOURCES = wbCfgTestApp.c
