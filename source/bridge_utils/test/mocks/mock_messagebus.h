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

#ifndef MOCK_MESSAGEBUS_H
#define MOCK_MESSAGEBUS_H


#include <gtest/gtest.h>
#include <gmock/gmock.h>

typedef void*(*CCSP_MESSAGE_BUS_MALLOC) ( size_t size ); // this signature is different from standard malloc
typedef void (*CCSP_MESSAGE_BUS_FREE)   ( void * ptr );

class MessageBusInterface {
public:
	virtual ~MessageBusInterface() {}
	virtual int CCSP_Message_Bus_Init(char *, char *, void **, CCSP_MESSAGE_BUS_MALLOC, CCSP_MESSAGE_BUS_FREE) = 0;
};

class MessageBusMock: public MessageBusInterface {
public:
	virtual ~MessageBusMock() {}
	MOCK_METHOD5(CCSP_Message_Bus_Init, int(char *, char *, void **, CCSP_MESSAGE_BUS_MALLOC, CCSP_MESSAGE_BUS_FREE));
};

#endif
