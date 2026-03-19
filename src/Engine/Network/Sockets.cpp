#include "Engine/Network/Sockets.h"
#include <iostream>
#include <sstream>
#include <WS2tcpip.h>

Sockets::Sockets(Type _type)
{
	m_type = _type;
	m_addr = {};

	if (m_type == TCP)
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	else
		m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_socket == INVALID_SOCKET)
	{
		std::ostringstream error;
		error << "Erreur initialisation socket [" << SocketsMethods::GetError << "]";
		throw std::runtime_error(error.str());
	}
}

Sockets::~Sockets()
{
	SocketsMethods::CloseSocket(m_socket);
}

bool Sockets::Connect(const std::string& _ip, int _port)
{
	if (m_type == TCP)
	{
		return ConnectTCP(_ip, _port);
	}
	else
	{
		return ConnectUDP(_ip, _port);
	}
}

int Sockets::Send(const char* _data, unsigned int len, sockaddr_in& target)
{
	if (m_type == TCP)
	{
		return SendTCP(_data, len);
	}
	else
	{
		return SendUDP(_data, len, target);
	}
}

int Sockets::Receive(char* _buffer, unsigned int len, sockaddr_in& target)
{
	if (m_type == TCP)
	{
		return ReceiveTCP(_buffer, len);
	}
	else
	{
		return ReceiveUDP(_buffer, len, target);
	}
}

bool Sockets::ConnectTCP(const std::string& _ip, int _port)
{
	sockaddr_in server;
	if (inet_pton(AF_INET, _ip.c_str(), &server.sin_addr) <= 0)
		return false;
	server.sin_family = AF_INET;
	server.sin_port = htons(_port);
	return connect(m_socket, (sockaddr*)&server, sizeof(server)) == 0;
}

bool Sockets::ConnectUDP(const std::string& _ip, int _port)
{
	sockaddr_in server = {};
	server.sin_family = AF_INET;
	server.sin_port = htons(_port);

	if (inet_pton(AF_INET, _ip.c_str(), &server.sin_addr) <= 0)
		return false;  // NE PAS écraser avec INADDR_ANY ici

	if (bind(m_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
		return false;

	m_addr = server;  // copie par valeur, pas un pointeur local
	return true;
}

int Sockets::SendTCP(const char* _data, unsigned int len)
{
	unsigned short networkLen = htons(len);
	return send(m_socket, reinterpret_cast<const char*>(&networkLen), sizeof(networkLen), 0) == sizeof(networkLen)
		&& send(m_socket, reinterpret_cast<const char*>(_data), len, 0) == len;
}

int Sockets::SendUDP(const char* _data, unsigned int len, sockaddr_in& target)
{
	return sendto(m_socket, _data, len, 0, reinterpret_cast<sockaddr*>(&target), sizeof(target));
}

int Sockets::ReceiveTCP(char* _buffer, unsigned int len)
{
	return recv(m_socket, _buffer, len, 0);
}

int Sockets::ReceiveUDP(char* _buffer, unsigned int len, sockaddr_in& target)
{
	int addrSize = sizeof(sockaddr_in);
	return recvfrom(m_socket, _buffer, len, 0, reinterpret_cast<sockaddr*>(&target), &addrSize);
}
