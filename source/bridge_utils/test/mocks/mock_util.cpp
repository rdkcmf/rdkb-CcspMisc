#include "mocks/mock_util.h"

using namespace std;

extern UtilMock * g_utilMock;

extern "C" int system(const char * cmd)
{
    if (!g_utilMock)
    {
        return 0;
    }
    return g_utilMock->system(cmd);
}