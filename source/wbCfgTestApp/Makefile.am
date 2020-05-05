AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER
AM_CFLAGS += -D_ANSC_LITTLE_ENDIAN_
AM_CFLAGS += -DCCSP_INC_no_asm_sigcontext_h

bin_PROGRAMS = wbCfgTestApp

wbCfgTestApp_SOURCES = wbCfgTestApp.c

wbCfgTestApp_LDFLAGS = -lwebconfig_framework -lccsp_common -lrt -lz -lpthread
