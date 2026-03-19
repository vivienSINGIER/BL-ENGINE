#ifndef SOCKETS_H_DEFINED
#define SOCKETS_H_DEFINED

#include <string>
#include "define.h"
#include "Engine/Network/SocketMethods.h"

enum Type
{
	TCP,
	UDP
};

class Sockets
{
public:
	Sockets(Type _type);
	~Sockets();

	bool Connect(const std::string& _ip, int _port);
	int Send(const char* _data, unsigned int len, sockaddr_in& target);
	int Receive(char* _buffer, unsigned int len, sockaddr_in& target);

	sockaddr_in GetAddr() const { return m_addr; }

private:
	bool ConnectTCP(const std::string& _ip, int _port);
	bool ConnectUDP(const std::string& _ip, int _port);
	int SendTCP(const char* _data, unsigned int len);
	int SendUDP(const char* _data, unsigned int len, sockaddr_in& target);
	int ReceiveTCP(char* _buffer, unsigned int len);
	int ReceiveUDP(char* _buffer, unsigned int len, sockaddr_in& target);

	Type m_type;
	SOCKET m_socket;
	sockaddr_in m_addr;
};

#endif // !SOCKETS_DEFINED
