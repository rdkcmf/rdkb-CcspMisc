#include "mocks/mock_messagebus.h"

using namespace std;

extern MessageBusMock * g_messagebusMock;

extern "C" int
CCSP_Message_Bus_Init
(
    char *component_id,
    char *config_file,
    void **bus_handle,
    CCSP_MESSAGE_BUS_MALLOC mallocfc,
    CCSP_MESSAGE_BUS_FREE   freefc
)
{
	if(!g_messagebusMock)
	{
		return -1;
    }
    return g_messagebusMock->CCSP_Message_Bus_Init(component_id, config_file, bus_handle, mallocfc, freefc);
}