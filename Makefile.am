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
SUBDIRS = source

lib_LTLIBRARIES=libtime_conversion.la
libtime_converion_la_CPPFLAGS = -D_ANSC_LINUX -D_ANSC_USER -I$(top_srcdir)/../hal/include
libtime_conversion_la_LDFLAGS= -lccsp_common
libtime_conversion_la_LDFLAGS= -lhal_platform
libtime_conversion_la_SOURCES= source/TimeConv/time_conversion.c
libtime_conversion_la_DEPENDENCIES=
libtime_conversion_la_LIBADD=$(libtime_conversion_la_DEPENDENCIES)
