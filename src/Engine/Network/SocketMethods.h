#ifndef SOCKET_METHODS_H_DEFINED
#define SOCKET_METHODS_H_DEFINED
#define _SOCKET int
#define _INVALID_SOCKET ((int)-1)

#pragma comment(lib, "Ws2_32.lib")

namespace SocketsMethods
{
    bool Start();
    void Release();
    int GetError();
    void CloseSocket(_SOCKET socket);
}

#endif
