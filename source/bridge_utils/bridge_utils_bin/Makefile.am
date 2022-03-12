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

AM_CFLAGS = -D_GNU_SOURCE
AM_CFLAGS += -Wall -Wextra

bin_PROGRAMS = bridgeUtils

bridgeUtils_CPPFLAGS = -I$(top_srcdir)/include \
                       -I$(top_srcdir)/source/include \
                       -I$(top_srcdir)/source/bridge_utils/include \
                       -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp \
                       -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ovsagent \
                       $(DBUS_CFLAGS)

bridgeUtils_SOURCES = bridge_util.c bridge_creation.c main.c bridge_util_generic.c

bridgeUtils_LDFLAGS =  -lrt -lccsp_common -lsyscfg -lsysevent -lprivilege -lbridge_utils -lsecure_wrapper
