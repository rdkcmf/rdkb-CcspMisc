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
