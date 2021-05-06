/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef _BRIDGE_CREATION_H_
#define _BRIDGE_CREATION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>


#include "OvsAgentApi.h"

/**
 * @brief To add a transaction that has been initiated to the transaction store.
 *
 * @param[in] trans_uuid Transaction UUID unique identifier for the message/transaction.
 * @return boolean true indicating success, false indicating failure.
 */
bool create_bridge_api(ovs_interact_request *request, ovs_interact_cb callback);

bool brctl_interact(ovs_interact_request * request);

#endif

