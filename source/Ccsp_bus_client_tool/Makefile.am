AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER
AM_CFLAGS += -D_ANSC_LITTLE_ENDIAN_
AM_CFLAGS += -DCCSP_INC_no_asm_sigcontext_h
AM_LDFLAGS = -lccsp_common
AM_LDFLAGS += -lthreadutil
AM_LDFLAGS += -lrt
AM_LDFLAGS += -lssl
AM_LDFLAGS += -lcrypto
AM_LDFLAGS += -lz

AM_CPPFLAGS = -Wall -Werror
ACLOCAL_AMFLAGS = -I m4
hardware_platform = i686-linux-gnu
bin_PROGRAMS = dmcli
dmcli_CPPFLAGS = $(CPPFLAGS)
dmcli_SOURCES = ccsp_message_bus_client_tool.c
dmcli_LDFLAGS = -lccsp_common -lthreadutil -lrt -lssl -lcrypto -lz
