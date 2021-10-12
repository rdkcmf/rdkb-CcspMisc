#ifndef MOCK_FD_H
#define MOCK_FD_H


#include <gtest/gtest.h>
#include <gmock/gmock.h>

class FileDescriptorInterface
{
    public:
        virtual ~FileDescriptorInterface() {}
        virtual int ioctl(int, unsigned long, void*) = 0;
};
class FileDescriptorMock : public FileDescriptorInterface
{
    public:
        virtual ~FileDescriptorMock() {}
        MOCK_METHOD3(ioctl, int(int, unsigned long, void *));
};

#endif




