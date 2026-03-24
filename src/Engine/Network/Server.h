#ifndef SERVER_H_DEFINED
#define SERVER_H_DEFINED

#include "INetworkBase.h"
#include "Network.h"

struct ClientInfo
{
	sockaddr_in udpAddr;
	String ip;
	int port;
};

class Server : public INetworkBase
{
public:
	Server();
	void Initialize(std::string _ip, int _port);
	void Update(float _dt) override;
	void Shutdown() { m_isRunning = false; }
	
	void SendPackets() override;
	void RegisterTargetedPacket(Packet _packet, sockaddr_in _addr);
	void SendGeneralReliablePacket(Packet _packet);

	ClientInfo* FindClient(const sockaddr_in& _addr);
	void AddClient(const sockaddr_in& _addr);

	void QueueSyncPackets(const sockaddr_in& _addr);
	
private:
	static DWORD WINAPI ReceiveThread(LPVOID lpParam);
	Vector<ClientInfo> m_clients;

	Vector<Pair<Packet, sockaddr_in>> m_targetedPackets;
	
	bool m_isRunning = false;
};

#endif // !SERVER_H_DEFINED
