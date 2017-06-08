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
if PARODUS_ENABLE
if CCSP_ARCH_ARM
PARODUS_OPT = parodusStart
endif

if NOTIFYLEASE_ENABLE
NOTIFY = Notify_Lease
endif
endif


SUBDIRS = Psmcli LTime print_uptime $(NOTIFY) $(PARODUS_OPT)

