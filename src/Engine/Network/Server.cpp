#include "Server.h"

#include "Scene.h"
#include "SceneManager.h"
#include "../ECS/World.h"

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
		if (m_clients[i].isConnected == false) continue;
		
		for (int j = 0; j < m_packets.size(); j++)
		{
			m_packets[i].header.clientId = 0;
			m_socket->Send(m_packets[j].Data(), m_packets[j].Size(), m_clients[i].udpAddr);
		}
	}
	m_packets.clear();

	for (int i = 0; i < m_targetedPackets.size(); i++)
	{
		m_targetedPackets[i].first.header.clientId = 0;
		m_socket->Send(m_targetedPackets[i].first.Data(), m_targetedPackets[i].first.Size(), m_targetedPackets[i].second);
	}
	m_targetedPackets.clear();
	
	for(int i = 0; i < m_pendingPackets.size(); i++)
	{
		PendingPacket& pending = m_pendingPackets[i];
		if (pending.canResend)
		{
			pending.packet.header.clientId = 0;
			m_socket->Send(pending.packet.Data(), pending.packet.Size(), pending.target);
			pending.timer = 0.0f;
			pending.canResend = false;
			std::cout << "Resend pending message :" << pending.ackId << " | Try nb : " << (int)pending.retryCount << std::endl;
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
		if (m_clients[i].isConnected == false) continue;
		
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

void Server::Initialize(String _ip, int _port)
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
	client.id = m_clientIdCount++;
	m_clients.push_back(client);
}

void Server::QueueSyncPackets(const sockaddr_in& _addr)
{
	UnorderedMap<String, uint32>& sceneInfo = SceneManager::GetInstance().GetSceneInfos();

	for (auto& [name, id] : sceneInfo)
	{
		Packet p;
		p.header.type = PacketType::AddScene;
		p.header.sceneId = id;
		p.addScene.nameSize = name.size();
		memcpy(p.addScene.name, name.c_str(), name.size());

		SendReliablePacket(p, _addr);

		Scene* s = SceneManager::GetSceneWithId(id);
		
		for (EntityId e : s->world->GetEntities())
		{
			QueueEntitySyncPackets(e, id, _addr);
		}
	}

	uint32 currSceneId = sceneInfo[SceneManager::GetCurrentScene()->GetName()];
	Packet setSceneP;
	setSceneP.header.type = PacketType::SetScene;
	setSceneP.header.sceneId = currSceneId;
	SendReliablePacket(setSceneP, _addr);
}

void Server::QueueEntitySyncPackets(EntityId _e, uint32 _sceneId, const sockaddr_in& _addr)
{
	Scene* s = SceneManager::GetSceneWithId(_sceneId);
	
	Packet sp;
	sp.header.type = PacketType::Spawn;
	sp.header.sceneId = _sceneId;
	sp.header.entityId = _e;
	
	EntityRecord& rec = s->world->entityManager.GetRecord(_e);
	Archetype* arch = rec.archetype;
	sp.createEntity.componentMask = arch->mask;
	ComponentRegistry::ClearClientSideBits(sp.createEntity.componentMask);
	
	SendReliablePacket(sp, _addr);
	
	if (rec.isActive == false)
	{
		Packet p;
		p.header.type = PacketType::SetActiveState;
		p.header.entityId = _e;
		p.header.sceneId = SceneManager::GetCurrentScene()->GetId();
    
		p.setActiveState.isActive = false;
		p.setActiveState.isEntity = true;
    
		SendReliablePacket(p, _addr);
	}
	
	for (auto& [cid, data] : arch->storage.columns)
	{
		if (ComponentRegistry::IsClientOnly(cid)) continue;
		
		uint64 stride = arch->storage.strides[cid];

		Packet acP;

		if (ComponentRegistry::IsScript(cid) == false)
		{
			acP.header.type = PacketType::AddComponent;
			acP.header.sceneId = _sceneId;
			acP.header.entityId = _e;

			acP.addComponent.ComponentId = cid;
			acP.addComponent.size = stride;
			memcpy(acP.addComponent.data, data.data() + stride * rec.row, stride);
		}
		else
		{
			acP.header.type = PacketType::AddScript;
			acP.header.sceneId = _sceneId;
			acP.header.entityId = _e;

			acP.addScript.ComponentId = cid;
		}
		
		SendReliablePacket(acP, _addr);
		
		if (arch->storage.GetActive(cid, rec.row) == false)
		{
			Packet p;
			p.header.type = PacketType::SetActiveState;
			p.header.entityId = _e;
			p.header.sceneId = SceneManager::GetCurrentScene()->GetId();
    
			p.setActiveState.isActive = false;
			p.setActiveState.isEntity = false;
			p.setActiveState.cid = cid;
    
			SendReliablePacket(p, _addr);
		}
	}
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

