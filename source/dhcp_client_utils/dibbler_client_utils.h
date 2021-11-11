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

#define DIBBLER_CLIENT_PATH         "/usr/sbin/dibbler-client"

#define DIBBLER_CONFIG_FILE "/etc/dibbler/client.conf"
#define DIBBLER_TMP_CONFIG_FILE "/tmp/dibbler/client-tmp.conf"
#define DIBBLER_TEMPLATE_CONFIG_FILE "/tmp/dibbler/client-template.conf"

pid_t start_dibbler (dhcp_params * params, dhcp_opt_list * req_opt_list, dhcp_opt_list * send_opt_list);
