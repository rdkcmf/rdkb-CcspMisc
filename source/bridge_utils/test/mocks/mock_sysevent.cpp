#include "mocks/mock_sysevent.h"

using namespace std;

extern SyseventMock * g_syseventMock;

extern "C" int sysevent_open(char *ip, unsigned short port, int version, char *id, token_t *token)
{
    if(!g_syseventMock)
    {
        return 0;
    }
    return g_syseventMock->sysevent_open(ip, port, version, id, token);
}

extern "C" int sysevent_close(const int fd, const token_t token)
{
	if(!g_syseventMock)
	{
	    return 0;
	}
    return g_syseventMock->sysevent_close(fd, token);
}	

extern "C" int sysevent_get(const int fd, const token_t token, const char *inbuf, char *outbuf, int outbytes)
{
	if(!g_syseventMock)
	{
		return 0;
	}
	return g_syseventMock->sysevent_get(fd, token, inbuf,outbuf, outbytes);
}

extern "C" int sysevent_set(const int fd, const token_t token, const char *name, const char *value, int conf_req)
{
	if(!g_syseventMock)
	{
		return 0;
	}
	return g_syseventMock->sysevent_set(fd, token, name, value, conf_req);
}