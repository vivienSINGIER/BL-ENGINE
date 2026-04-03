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
    void Disconnect();
    sockaddr_in& GetServerAddress() { return m_serverAddress; }
    
    void SetId(uint32 _id) { m_id = _id; }
    uint32 GetId() { return m_id; }

private:
    static DWORD WINAPI ReceiveThread(LPVOID _lpParam);

    sockaddr_in m_serverAddress;
    uint32 m_id;;
    
    bool m_isConnected;
    bool m_isRunning;

    friend class EngineManager;
    friend struct ReceiveSystem;
};

#endif // !CLIENT_H_DEFINED

