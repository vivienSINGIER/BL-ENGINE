#include "NetworkSystem.h"

#include "Network/Client.h"
#include "Network/Server.h"

void NetworkSystem::OnInit()
{
    m_client = EngineManager::GetClient();
    m_server = EngineManager::GetServer();
}

void NetworkSystem::OnStartUpdate(float _dt)
{
    if (m_client != nullptr)
        m_client->Update(_dt);
    if (m_server != nullptr)
        m_server->Update(_dt);
}

void NetworkSystem::OnUpdate(float _dt, EntityId _e, NetworkComponent& _network)
{
    
}

void NetworkSystem::OnEndUpdate(float _dt)
{
    if (m_client != nullptr)
        HandleClientReceive();
    if (m_server != nullptr)
        HandleServerReceive();
}

void NetworkSystem::HandleClientReceive()
{
    Vector<ReceivedPacket>& vReceived = m_client->GetReceived();

    m_client->GetCritSection().Enter();
    for (ReceivedPacket& received : vReceived)
    {
        Packet p = received.packet;
        sockaddr_in addr = received.sender;
        
        switch (p.header.type)
        {
            case PacketType::ConnectAck:
            {
                m_client->OnAckReceived(p.header.ackId);
                m_client->Connect(p.connect.addr);
                break;
            }
            default:
                break;
        }
    }
    vReceived.clear();
    m_client->GetCritSection().Leave();
}

void NetworkSystem::HandleServerReceive()
{
    Vector<ReceivedPacket>& vReceived = m_server->GetReceived();

    m_server->GetCritSection().Enter();
    for (ReceivedPacket& received : vReceived)
    {
        Packet p = received.packet;
        sockaddr_in addr = received.sender;
        
        switch (p.header.type)
        {
            case PacketType::Connect:
            {
                m_server->AddClient(addr);

                Packet np;
                np.header.type = PacketType::ConnectAck;
                np.header.ackId = p.header.ackId;
                np.connect.addr = m_server->GetSocket()->GetAddr();
                    
                m_server->RegisterTargetedPacket(np, addr);
                break;
            }
            default:
                break;
        }
    }
    vReceived.clear();
    m_server->GetCritSection().Leave();
}


