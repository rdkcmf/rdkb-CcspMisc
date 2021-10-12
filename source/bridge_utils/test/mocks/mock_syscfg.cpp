#include "mocks/mock_syscfg.h"

using namespace std;

extern SyscfgMock * g_syscfgMock;

extern "C" int syscfg_init() 
{
    if (!g_syscfgMock)
    {
        return 0;
    }
    return g_syscfgMock->syscfg_init();
}

extern "C" int syscfg_get(const char *ns, const char *name, char *out_value, int outbufsz) 
{
	if(!g_syscfgMock)
	{
	    return 0;
	}
    return g_syscfgMock->syscfg_get(ns, name, out_value, outbufsz);
}