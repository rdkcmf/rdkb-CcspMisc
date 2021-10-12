#include "mocks/mock_bridge_util_generic.h"

using namespace std;

extern BridgeUtilsGenericMock * g_bridgeUtilsGenericMock;

extern "C" int HandlePreConfigVendorGeneric(void *bridgeInfo,int InstanceNumber)
{
    if (!g_bridgeUtilsGenericMock)
    {
        return 0;
    }
    return g_bridgeUtilsGenericMock->HandlePreConfigVendorGeneric(bridgeInfo, InstanceNumber);
}

extern "C" int HandlePostConfigVendorGeneric(void *bridgeInfo,int InstanceNumber)
{
    if (!g_bridgeUtilsGenericMock)
    {
        return 0;
    }
    return g_bridgeUtilsGenericMock->HandlePostConfigVendorGeneric(bridgeInfo, InstanceNumber);
}
/*
extern "C" char * getVendorIfaces()
{
    if (!g_bridgeUtilsGenericMock)
    {
        return 0;
    }
    return g_bridgeUtilsGenericMock->getVendorIfaces();
}
*/
