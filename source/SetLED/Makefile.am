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
bin_PROGRAMS = SetLED

AM_CFLAGS = -I$(top_srcdir)/source -I$(top_srcdir)/source/include -I$(top_srcdir)/source/util -I$(top_srcdir)/../hal/include
hardware_platform = i686-linux-gnu
AM_LDFLAGS =  -lpthread -lz -lccsp_common
if CCSP_ARCH_ARM
AM_LDFLAGS += -lhal_plaform -lsysevent -lutapi -lutctx -lulog
endif

SetLED_SOURCES = SetLED.c

if CCSP_ARCH_ARM
SetLED_CPPFLAGS = -D_ANSC_LINUX -D_ANSC_USER -I$(top_srcdir)/../hal/include
SetLED_LDFLAGS =  -lhal_platform
endif

if CCSP_ARCH_ARM
SetLED_LDFLAGS += -lhal_platform
endif

SetLED_LDADD=$(EXTRA_DEPENDENCIES)


