#include "Engine/Network/Sockets.h"
#include <iostream>
#include <sstream>

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
		error << "Erreur initialisation socket [" << SocketsMethods::GetError() << "]";
		throw std::runtime_error(error.str());
	}
}

Sockets::~Sockets()
{
	SocketsMethods::CloseSocket(static_cast<int>(m_socket));
}

bool Sockets::Connect(const String& _ip, int _port)
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

String Sockets::GetIP(sockaddr_in _addr)
{
	char ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &_addr.sin_addr, ipStr, sizeof(ipStr));
	return String(ipStr);
}

Array<int, 4> Sockets::GetIP4(sockaddr_in _addr)
{
	String ipStr = GetIP(_addr);
	Array<int, 4> result;
	int count = 0;

	String temp;
	for (char c : ipStr)
	{
		if (c == '.')
		{
			result[count] = std::stoi(temp);
			temp.clear();
			count++;
		}
	}
	result[count] = std::stoi(temp);
	temp.clear();
	count++;

	return result;
}

int Sockets::GetPort(sockaddr_in _addr)
{
	return ntohs(_addr.sin_port);
}

bool Sockets::ConnectTCP(const String& _ip, int _port)
{
	sockaddr_in server;
	if (inet_pton(AF_INET, _ip.c_str(), &server.sin_addr) <= 0)
		return false;
	server.sin_family = AF_INET;
	server.sin_port = htons(_port);
	return connect(m_socket, (sockaddr*)&server, sizeof(server)) == 0;
}

bool Sockets::ConnectUDP(const String& _ip, int _port)
{
	sockaddr_in server = {};
	server.sin_family = AF_INET;
	server.sin_port = htons(_port);

	if (inet_pton(AF_INET, _ip.c_str(), &server.sin_addr) <= 0)
		return false;  // NE PAS �craser avec INADDR_ANY ici

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
