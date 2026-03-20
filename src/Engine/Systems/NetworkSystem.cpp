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
    
}

void NetworkSystem::HandleServerReceive()
{
    Vector<Packet>& vReceived = m_server->GetReceived();

    for (Packet& received : vReceived)
    {
        switch (received.header.type)
        {
            case PacketType::Connect:
            {
                m_server->AddClient(received.connect.addr);

                Packet p;
                p.header.type = PacketType::ConnectAck;
                p.header.ackId = received.header.ackId;
                    
                m_server->SendReliablePacket(p, received.connect.addr);
            }
            default:
                break;
        }
    }
}


