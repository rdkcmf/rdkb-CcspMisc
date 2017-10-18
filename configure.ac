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

if test "x$CCSP_ARCH" = xarm ||
   test "x$CCSP_ARCH" = xmips
then
  AC_ARG_ENABLE([parodus],
  [  --enable-parodus    Turn on parodus],
  [case "${enableval}" in
    yes) PARODUS_ENABLE=true ;;
    no)  PARODUS_ENABLE=false ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-parodus]) ;;
  esac],[PARODUS_ENABLE=false])
fi

AM_CONDITIONAL([PARODUS_ENABLE], [test x$PARODUS_ENABLE = xtrue])

AC_ARG_ENABLE([notifylease],
[  --enable-notifylease    Turn on leasenotify ],
[case "${enableval}" in
  yes) NOTIFYLEASE_ENABLE=true ;;
  no)  NOTIFYLEASE_ENABLE=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-notifylease]) ;;
esac],[NOTIFYLEASE_ENABLE=false])
AM_CONDITIONAL([NOTIFYLEASE_ENABLE], [test x$NOTIFYLEASE_ENABLE = xtrue])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
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
	source/parcon/Makefile
    source/print_uptime/Makefile
	source/Makefile
	Makefile
)


AM_COND_IF([PARODUS_ENABLE], 
           [AC_CONFIG_FILES([source/parodusStart/Makefile])], [])


AM_COND_IF([NOTIFYLEASE_ENABLE], [AM_COND_IF([CCSP_ARCH_ARM],
           [AC_CONFIG_FILES([source/Notify_Lease/Makefile])])], [])

AC_OUTPUT

