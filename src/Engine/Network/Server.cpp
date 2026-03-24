#include "Server.h"

#include "Scene.h"
#include "SceneManager.h"

Server::Server() : INetworkBase()
{
}

void Server::Update(float _dt)
{
	TickAck(_dt);
	SendPackets();
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

	for (int i = 0; i < m_targetedPackets.size(); i++)
	{
		m_socket->Send(m_targetedPackets[i].first.Data(), m_targetedPackets[i].first.Size(), m_targetedPackets[i].second);
	}
	m_targetedPackets.clear();
	
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

void Server::RegisterTargetedPacket(Packet _packet, sockaddr_in _addr)
{
	m_targetedPackets.push_back({_packet, _addr});
}

void Server::SendGeneralReliablePacket(Packet _packet)
{
	for(int i = 0; i < m_clients.size(); i++)
	{
		PendingPacket pending;
		pending.packet     = _packet;
		pending.target     = m_clients.at(i).udpAddr;
		pending.packet.header.ackId = ACK_COUNT++;
		pending.ackId = pending.packet.header.ackId;
		pending.timer      = 0.0f;
		pending.retryCount = 0;
		pending.canResend = true;

		m_pendingPackets.push_back(pending);	
	}
}

ClientInfo* Server::FindClient(const sockaddr_in& _addr)
{
	String ip = Sockets::GetIP(_addr);
	int port = Sockets::GetPort(_addr);

	for (ClientInfo& cInfo : m_clients)
	{
		if (cInfo.ip == ip && cInfo.port == port)
			return &cInfo;
	}
	
	return nullptr;
}

void Server::Initialize(std::string _ip, int _port)
{
	m_socket->Connect(_ip, _port);
	m_isRunning = true;
	CreateThread(NULL, 0, ReceiveThread, this, 0, NULL);
}

void Server::AddClient(const sockaddr_in& _addr)
{
	if (FindClient(_addr) != nullptr)
		return;
	
	ClientInfo client;
	client.udpAddr = _addr;
	client.ip = GetSocket()->GetIP(_addr);
	client.port = GetSocket()->GetPort(_addr);
	m_clients.push_back(client);
}

void Server::QueueSyncPackets(const sockaddr_in& _addr)
{
	UnorderedMap<String, uint32>& sceneInfo = SceneManager::GetInstance().GetSceneInfos();

	for (auto& [name, id] : sceneInfo)
	{
		Packet p;
		p.header.type = PacketType::AddScene;
		p.addScene.sceneId = id;
		p.addScene.nameSize = name.size();
		memcpy(p.addScene.name, name.c_str(), name.size());

		SendReliablePacket(p, _addr);
	}

	uint32 currSceneId = sceneInfo[SceneManager::GetCurrentScene()->GetName()];
	Packet setSceneP;
	setSceneP.header.type = PacketType::SetScene;
	setSceneP.addScene.sceneId = currSceneId;
	SendReliablePacket(setSceneP, _addr);
}

DWORD WINAPI Server::ReceiveThread(LPVOID lpParam)
{
	Server* server = static_cast<Server*>(lpParam);
	char* buffer = new char[BUFFER_SIZE];
	sockaddr_in sender;
	while (server->m_isRunning)
	{
		memset(buffer, 0, BUFFER_SIZE);
		int bytesRead = server->GetSocket()->Receive(buffer, BUFFER_SIZE, sender);
		if (bytesRead > 0)
		{
			Packet packet;
			memcpy(&packet, buffer, bytesRead);

			ReceivedPacket receivedPacket;
			receivedPacket.packet = packet;
			receivedPacket.sender = sender;
			
			server->m_packetProtection.Enter();
			server->m_receivedPackets.push_back(receivedPacket);
			server->m_packetProtection.Leave();
		}
	}
	delete[] buffer;
	return 0;
}
