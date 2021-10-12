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

