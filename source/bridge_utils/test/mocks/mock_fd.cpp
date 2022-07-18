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

#include "mocks/mock_fd.h"

using namespace std;

extern FileDescriptorMock * g_fdMock;   /* This is just a declaration! The actual mock
                                           obj is defined globally in the test file. */
// Mock Method
extern "C" int ioctl(int fd, unsigned long request, ...)
{
    void *flags = NULL;
    if (!g_fdMock)
    {
        return 0;
    }
    return g_fdMock->ioctl(fd, request, flags);
}

