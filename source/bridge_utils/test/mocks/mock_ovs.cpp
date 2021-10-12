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