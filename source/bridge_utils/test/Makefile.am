AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER
AM_CFLAGS += -D_GNU_SOURCE
AM_CFLAGS += -fno-exceptions -Os -Wall -Wextra

AM_LDFLAGS =  -lpthread -lrt -lccsp_common -lz $(DBUS_LIBS)

AM_CPPFLAGS = -Wall -g -Werror
AM_CXXFLAGS = -std=c++11 -lstdc++fs

ACLOCAL_AMFLAGS = -I m4
bin_PROGRAMS = bridgeUtils_gtest.bin
bridgeUtils_gtest_bin_CPPFLAGS = -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/gtest \
					  -I${top_srcdir}/gtest/include \
					  -I${top_srcdir}/source \
					  -I${top_srcdir}/source/include \
					  -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp \
					  -I$(top_srcdir)/source/bridge_utils/include \
                      -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ovsagent \
                      -fPIC -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/dbus-1.0 \
                      -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/dbus-1.0/include \
                      $(DBUS_CFLAGS)

bridgeUtils_gtest_bin_SOURCES =  bridgeUtilsTest.cpp \					  
                      gtest_main.cpp \
					  ../bridge_utils_bin/bridge_util.c \
					  ../bridge_utils_bin/bridge_creation.c \
                      mocks/mock_syscfg.cpp \
					  mocks/mock_sysevent.cpp \
					  mocks/mock_psm.cpp \
					  mocks/mock_file_io.cpp \
					  mocks/mock_socket.cpp \
					  mocks/mock_fd.cpp \
					  mocks/mock_messagebus.cpp \
					  mocks/mock_util.cpp \
					  mocks/mock_ovs.cpp \
					  mocks/mock_bridge_util_generic.cpp
					 

bridgeUtils_gtest_bin_LDFLAGS = -lgtest -lgmock -lgcov -lrt -lprivilege  -lccsp_common -lbridge_utils -lsecure_wrapper
