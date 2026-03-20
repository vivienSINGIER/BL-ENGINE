#ifndef CLIENT_H_DEFINED
#define CLIENT_H_DEFINED

#include "INetworkBase.h"

class Client : public INetworkBase
{
    Client();
    
    void Init();
    void Update(float _dt) override;
    void Shutdown() { m_isRunning = false; }
    
    void SendPackets() override;

private:
    static DWORD WINAPI ReceiveThread(LPVOID _lpParam);

    sockaddr_in m_serverAddress;
    
    bool m_isConnected;
    bool m_isRunning;
    
    void TryConnexion(String const& _ip, int _port = 1888);
};

#endif // !CLIENT_H_DEFINED

