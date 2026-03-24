#ifndef CLIENT_H_DEFINED
#define CLIENT_H_DEFINED

#include "INetworkBase.h"

class Client : public INetworkBase
{
public:
    Client();
    
    void Init();
    void Update(float _dt) override;
    void Shutdown() { m_isRunning = false; }
    
    void SendPackets() override;

    void Connect(sockaddr_in _addr);
    sockaddr_in& GetServerAddress() { return m_serverAddress; }

private:
    static DWORD WINAPI ReceiveThread(LPVOID _lpParam);

    sockaddr_in m_serverAddress;
    
    bool m_isConnected;
    bool m_isRunning;
};

#endif // !CLIENT_H_DEFINED

