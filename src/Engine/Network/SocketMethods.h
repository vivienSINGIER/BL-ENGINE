#ifndef SOCKET_METHODS_H_DEFINED
#define SOCKET_METHODS_H_DEFINED
#define _SOCKET int
#define INVALID_SOCKET ((int)-1)

namespace SocketsMethods
{
    bool Start();
    void Release();
    int GetError();
    void CloseSocket(_SOCKET socket);
}

#endif
