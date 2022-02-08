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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sysevent/sysevent.h>
#include "dhcp_client_utils.h"

#define DIBBLER_CLIENT                    "dibbler-client"
#define DIBBLER_CLIENT_PATH               "/usr/sbin/"DIBBLER_CLIENT
#define DIBBLER_CLIENT_RUN_CMD            "start"
#define DIBBLER_CLIENT_CMD                DIBBLER_CLIENT" "DIBBLER_CLIENT_RUN_CMD
#define DIBBLER_DFT_PATH                  "/etc/dibbler/"
#define DIBBLER_CLIENT_CONFIG_FILE        "client.conf"
#define DIBBLER_CLIENT_CONFIG_FILE_PATH   DIBBLER_DFT_PATH DIBBLER_CLIENT_CONFIG_FILE
#define DIBBLER_TMP_CONFIG_FILE      "/tmp/dibbler/client-tmp.conf"
#define DIBBLER_TEMPLATE_CONFIG_FILE "/tmp/dibbler/client-template.conf"
#define DIBBLER_CLIENT_PIDFILE       "/tmp/dibbler/client.pid"
#define DIBBLER_CLIENT_TERMINATE_TIMEOUT  (5 * MSECS_IN_SEC)

typedef struct {
    dhcp_params * if_param;
    dhcp_opt_list * req_opt_list;
    dhcp_opt_list * send_opt_list;
    char config_path [BUFLEN_128];
} dibbler_client_info;

pid_t start_dibbler (dhcp_params * params, dhcp_opt_list * req_opt_list, dhcp_opt_list * send_opt_list);
int stop_dibbler (dhcp_params * params);
