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

#ifndef MOCK_FILE_IO_H
#define MOCK_FILE_IO_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class FileIOInterface
{
    public:
        virtual ~FileIOInterface() {}
        virtual char * fgets(char *, int, FILE *) = 0;
        virtual FILE * popen(const char *, const char *) = 0;
        virtual int pclose(FILE *) = 0;
        virtual int fclose(FILE *) = 0;
        virtual int unlink(const char *) = 0;
        virtual int access(const char *, int ) = 0;
};

class FileIOMock : public FileIOInterface
{
    public:
        virtual ~FileIOMock() {}
        MOCK_METHOD3(fgets, char *(char *, int, FILE *));
        MOCK_METHOD2(popen, FILE *(const char *, const char *));
        MOCK_METHOD1(pclose, int (FILE *));
        MOCK_METHOD1(fclose, int (FILE *));
        MOCK_METHOD1(unlink, int(const char *));
        MOCK_METHOD2(access, int(const char*, int));
};

#endif
