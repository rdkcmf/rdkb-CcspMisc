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
