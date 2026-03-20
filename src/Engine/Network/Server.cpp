#include "Server.h"

Server::Server() : INetworkBase()
{
}

void Server::Update(float _dt)
{
	TickAck(_dt);
}

void Server::SendPackets()
{
	for(int i = 0; i < m_clients.size(); i++)
	{
		for (int j = 0; j < m_packets.size(); j++)
		{
			m_socket->Send(m_packets[j].Data(), m_packets[j].Size(), m_clients[i].udpAddr);
		}
	}
	m_packets.clear();

	for(int i = 0; i < m_pendingPackets.size(); i++)
	{
		PendingPacket& pending = m_pendingPackets[i];
		if (pending.canResend)
		{
			m_socket->Send(pending.packet.Data(), pending.packet.Size(), pending.target);
			pending.timer = 0.0f;
			pending.retryCount++;
			pending.canResend = false;
		}
	}
}

ClientInfo* Server::FindClient(EntityId _id)
{
	for (int i = 0; i < m_clients.size(); i++)
	{
		if (m_clients[i].clientId == _id)
		{
			return &m_clients[i];
		}
	}
	return nullptr;
}

void Server::Initialize(std::string _ip, int _port)
{
	m_socket->Connect(_ip, _port);
	m_isRunning = true;
}

void Server::AddClient(const sockaddr_in& _addr, EntityId _id)
{
	ClientInfo client;
	client.udpAddr = _addr;
	client.ip = GetSocket()->GetIP(_addr);
	client.port = GetSocket()->GetPort(_addr);
	client.clientId = _id;
	m_clients.push_back(client);
}

DWORD WINAPI Server::ReceiveThread(LPVOID lpParam)
{
	Server* server = static_cast<Server*>(lpParam);
	char* buffer = new char[BUFFER_SIZE];
	sockaddr_in sender;
	while (server->m_isRunning)
	{
		int bytesRead = server->GetSocket()->Receive(buffer, sizeof(Packet), sender);
		if (bytesRead > 0)
		{
			Packet packet;
			memcpy(&packet, buffer, BUFFER_SIZE);

			server->m_packetProtection.Enter();
			server->m_receivedPackets.push_back(packet);
			server->m_packetProtection.Leave();
		}
	}
	delete[] buffer;
	return 0;
}
