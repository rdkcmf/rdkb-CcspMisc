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
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "mocks/mock_ovs.h"

using namespace std;

extern OvsMock * g_ovsMock;

extern "C" bool ovs_agent_api_get_config(OVS_TABLE table, void ** ppConfig)
{
    if (!g_ovsMock)
    {
        return false;
    }
    return g_ovsMock->ovs_agent_api_get_config(table, ppConfig);
}

extern "C" bool ovs_agent_api_interact(ovs_interact_request * request, ovs_interact_cb callback)
{
    if (!g_ovsMock)
    {
        return false;
    }
    return g_ovsMock->ovs_agent_api_interact(request, callback);
}
