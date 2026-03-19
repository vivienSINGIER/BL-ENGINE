#ifndef SERVER_H_DEFINED
#define SERVER_H_DEFINED

#include "INetworkBase.h"


struct ClientInfo
{
	sockaddr_in udpAddr;
	EntityId clientId;
};

class Server : public INetworkBase
{
public:
	Server();

private:
	std::vector<ClientInfo> m_clients;

};

#endif // !SERVER_H_DEFINED
