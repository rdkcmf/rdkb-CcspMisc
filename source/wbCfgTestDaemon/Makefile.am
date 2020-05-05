AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER
AM_CFLAGS += -D_ANSC_LITTLE_ENDIAN_
AM_CFLAGS += -DCCSP_INC_no_asm_sigcontext_h

bin_PROGRAMS = wbCfgTestDaemon

wbCfgTestDaemon_SOURCES = wbCfgTestDaemon.c

wbCfgTestDaemon_LDFLAGS = -lccsp_common -lrt -lz -lpthread
