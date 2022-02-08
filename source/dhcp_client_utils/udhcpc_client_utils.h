/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define UDHCPC_CLIENT                    "udhcpc"
#define UDHCPC_CLIENT_PATH               "/sbin/"UDHCPC_CLIENT
#define UDHCP_PIDFILE                    "/tmp/udhcpc.%s.pid"
#define UDHCPC_SERVICE_SCRIPT_FILE       "/etc/udhcpc.script"
#define UDHCPC_SERVICE_EXE               "/usr/bin/service_udhcpc"
#define UDHCP_PIDFILE_PATTERN            "-p "UDHCP_PIDFILE" "
#define UDHCPC_TERMINATE_TIMEOUT         (5 * MSECS_IN_SEC)

pid_t start_udhcpc (dhcp_params * params, dhcp_opt_list * req_opt_list, dhcp_opt_list * send_opt_list);
