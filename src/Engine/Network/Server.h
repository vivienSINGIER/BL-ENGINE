#ifndef SERVER_H_DEFINED
#define SERVER_H_DEFINED

#include "INetworkBase.h"


struct ClientInfo
{
	sockaddr_in udpAddr;
	String ip;
	int port;
	EntityId clientId;
};

class Server : public INetworkBase
{
public:
	Server();
	void Update(float _dt) override;
	void SendPackets() override;
	ClientInfo* FindClient(EntityId _id);
	void Initialize(std::string _ip, int _port);
	void Shutdown() { m_isRunning = false; }

	void AddClient(const sockaddr_in& _addr, EntityId _id);

private:
	static DWORD WINAPI ReceiveThread(LPVOID lpParam);
	std::vector<ClientInfo> m_clients;

	bool m_isRunning = false;
};

#endif // !SERVER_H_DEFINED
