#include "mocks/mock_psm.h"

using namespace std;

extern PsmMock * g_psmMock;

extern "C" int PSM_Get_Record_Value2
(
    void*                       bus_handle,
    char const * const          pSubSystemPrefix,
    char const * const          pRecordName,
    unsigned int *              ulRecordType,
    char**                      pValue
)
{
	if(!g_psmMock)
	{
		return 0;
	}
	return g_psmMock->PSM_Get_Record_Value2(bus_handle, pSubSystemPrefix, pRecordName, ulRecordType, pValue);
}
