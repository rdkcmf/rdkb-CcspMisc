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
typedef unsigned int token_t;

class SyseventInterface {
public:
	virtual ~SyseventInterface() {}
	virtual int sysevent_open(char *, unsigned short, int, char *, token_t*) = 0;
	virtual int sysevent_close(const int, const token_t) = 0;
	virtual int sysevent_get(const int, const token_t, const char *, char *, int) =0;
	virtual int sysevent_set(const int, const token_t, const char *, const char *, int) = 0;
};

class SyseventMock : public SyseventInterface {
public:
	virtual ~SyseventMock() {}
	MOCK_METHOD5(sysevent_open, int(char *, unsigned short, int, char *, token_t*));
	MOCK_METHOD2(sysevent_close, int(const int, const token_t));
	MOCK_METHOD5(sysevent_get, int(const int, const token_t, const char *, char *, int));
	MOCK_METHOD5(sysevent_set, int(const int, const token_t, const char *, const char *, int));
};


