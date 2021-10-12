#include "mocks/mock_socket.h"

using namespace std;

extern SocketMock * g_socketMock;

extern "C" int socket(int domain, int type, int protocol)
{
    if (!g_socketMock)
    {
        return -1;
    }
    return g_socketMock->socket(domain, type, protocol);
}

extern "C" int close(int sockfd)
{
    if (!g_socketMock)
    {
        return -1;
    }
    return g_socketMock->close(sockfd);
}

