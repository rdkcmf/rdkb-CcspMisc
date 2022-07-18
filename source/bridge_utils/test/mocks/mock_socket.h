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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class SocketInterface
{
    public:
        virtual ~SocketInterface() {}
        virtual int socket(int, int, int) = 0;
        virtual int close(int) = 0;
};	

class SocketMock : public SocketInterface
{
    public:
        virtual ~SocketMock() {}
        MOCK_METHOD3(socket, int(int, int, int));
        MOCK_METHOD1(close, int(int));

};
