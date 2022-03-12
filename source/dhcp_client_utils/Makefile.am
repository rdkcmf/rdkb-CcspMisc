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
AM_CFLAGS =

AM_CPPFLAGS = -Wall -Werror

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libdhcp_client_utils.la
libdhcp_client_utils_la_SOURCES = dhcpv4_client_utils.c udhcpc_client_utils.c dhcpv6_client_utils.c dibbler_client_utils.c dhcp_client_common.c
libdhcp_client_utils_la_LDFLAGS = -lhal_platform -lsysevent -lsyscfg
