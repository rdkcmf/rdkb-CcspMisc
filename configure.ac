##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2015 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
#                                              -*- Autoconf -*-
# Process this file with autoconf to produce configure script.
#

AC_PREREQ([2.65])
AC_INIT([CcspMisc], [1.0], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([subdir-objects])
LT_INIT
GTEST_ENABLE_FLAG = ""

AC_ARG_ENABLE([gtestapp],
             AS_HELP_STRING([--enable-gtestapp],[enable Gtest support (default is no)]),
             [
               case "${enableval}" in
                yes) GTEST_SUPPORT_ENABLED=true
                     GTEST_ENABLE_FLAG="-DGTEST_ENABLE"
                     m4_if(m4_sysval,[0],[AC_CONFIG_FILES([source/test/Makefile])]);;
                no) GTEST_SUPPORT_ENABLED=false AC_MSG_ERROR([Gtest support is disabled]);;
                 *) AC_MSG_ERROR([bad value ${enableval} for --enable-gtestapp ]);;
               esac
             ],
             [echo "Gtestapp is disabled"])
AM_CONDITIONAL([WITH_GTEST_SUPPORT], [test x$GTEST_SUPPORT_ENABLED = xtrue])

AC_PREFIX_DEFAULT(`pwd`)
AC_ENABLE_SHARED
AC_DISABLE_STATIC
# Specify ccsp cpu arch

AC_ARG_WITH([ccsp-arch],
[AC_HELP_STRING([--with-ccsp-arch={arm,atom,pc,mips}],
                [specify the ccsp board CPU platform])],
[case x"$withval" in
   xarm)
     CCSP_ARCH=arm
     ;;
   xatom)
     CCSP_ARCH=atom
     ;;
   xpc)
     CCSP_ARCH=pc
     ;;
   xmips)
     CCSP_ARCH=mips
     ;;
   *)
     AC_MSG_ERROR([$withval is an invalid option to --with-ccsp-arch])
     ;;
 esac],
[CCSP_ARCH=''])
if test x"${CCSP_ARCH}" != x; then
  AC_DEFINE_UNQUOTED(CCSP_ARCH, "$CCSP_ARCH",
                     [The board CPU architecture])
fi

AM_CONDITIONAL(CCSP_ARCH_ARM, test "x$CCSP_ARCH" = xarm)
AM_CONDITIONAL(CCSP_ARCH_ATOM, test "x$CCSP_ARCH" = xatom)
AM_CONDITIONAL(CCSP_ARCH_PC, test "x$CCSP_ARCH" = xpc)
AM_CONDITIONAL(CCSP_ARCH_MIPS, test "x$CCSP_ARCH" = xmips)

AC_ARG_ENABLE([notifylease],
[  --enable-notifylease    Turn on leasenotify ],
[case "${enableval}" in
  yes) NOTIFYLEASE_ENABLE=true ;;
  no)  NOTIFYLEASE_ENABLE=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-notifylease]) ;;
esac],[NOTIFYLEASE_ENABLE=false])
AM_CONDITIONAL([NOTIFYLEASE_ENABLE], [test x$NOTIFYLEASE_ENABLE = xtrue])

AC_ARG_ENABLE([setLED],
 [AS_HELP_STRING([--enable-setLED], [Turn on setLED])], ,
 [setled_enable=true])
AM_CONDITIONAL([SETLED_ENABLE], [test x$setled_enable = xtrue])

AC_ARG_ENABLE([multipartUtilEnable],
[  --enable-multipartUtilEnable    Turn on multipartUtility ],
[case "${enableval}" in
  yes) MULTIPART_UTIL_ENABLE=true ;;
  no)  MULTIPART_UTIL_ENABLE=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-multipartUtilEnable]) ;;
esac],[MULTIPART_UTIL_ENABLE=false])
AM_CONDITIONAL([MULTIPART_UTIL_ENABLE], [test x$MULTIPART_UTIL_ENABLE = xtrue])

AC_ARG_ENABLE([bridgeUtilsBin],
[  --enable-bridgeUtilsBin    Turn on bridgeUtilsBin ],
[case "${enableval}" in
  yes) BRIDGE_UTILS_BIN_ENABLE=true ;;
  no)  BRIDGE_UTILS_BIN_ENABLE=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-bridgeUtilsBin]) ;;
esac],[BRIDGE_UTILS_BIN_ENABLE=false])
AM_CONDITIONAL([BRIDGE_UTILS_BIN_ENABLE], [test x$BRIDGE_UTILS_BIN_ENABLE = xtrue])

AC_ARG_ENABLE([wbCfgTestAppEnable],
[  --enable-wbCfgTestAppEnable    Turn on wbCfgTestApp ],
[case "${enableval}" in
  yes) WEBCFG_TESTAPP_ENABLE=true ;;
  no)  WEBCFG_TESTAPP_ENABLE=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-wbCfgTestAppEnable]) ;;
esac],[WEBCFG_TESTAPP_ENABLE=false])
AM_CONDITIONAL([WEBCFG_TESTAPP_ENABLE], [test x$WEBCFG_TESTAPP_ENABLE = xtrue])


AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_CC_C_O
AM_PROG_LIBTOOL(libtool)

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES(
	source/Psmcli/Makefile
	source/LTime/Makefile
	source/TimeConv/Makefile
	source/parcon/Makefile
	source/webcfg_decoder/Makefile
	source/MemFrag_Calc/Makefile
	source/Makefile
	source/dhcp_client_utils/Makefile
	Makefile
)

AM_COND_IF([NOTIFYLEASE_ENABLE], [AM_COND_IF([CCSP_ARCH_ARM],
           [AC_CONFIG_FILES([source/Notify_Lease/Makefile])])], [])

AM_COND_IF([SETLED_ENABLE], [AC_CONFIG_FILES([source/SetLED/Makefile])])

AM_COND_IF([MULTIPART_UTIL_ENABLE], [AC_CONFIG_FILES([source/multipartRoot/Makefile])])

AM_COND_IF([WEBCFG_TESTAPP_ENABLE], [AC_CONFIG_FILES([source/wbCfgTestDaemon/Makefile] [source/wbCfgTestApp/Makefile] )])

AM_COND_IF([BRIDGE_UTILS_BIN_ENABLE], [AC_CONFIG_FILES([source/bridge_utils/bridge_utils_bin/Makefile] [source/bridge_utils/Makefile] )])

AC_SUBST(GTEST_ENABLE_FLAG)

AC_OUTPUT

