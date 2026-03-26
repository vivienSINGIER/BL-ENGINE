#include "ReceiveSystem.h"

#include "EngineManager.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Network/Client.h"
#include "Network/Server.h"

#include "../ECS/World.h"

void ReceiveSystem::OnInit()
{
    
}

void ReceiveSystem::OnStartUpdate(float _dt)
{
    m_client = EngineManager::GetClient();
    m_server = EngineManager::GetServer();
}

void ReceiveSystem::OnUpdate(float _dt, EntityId _e, NetworkComponent& _network)
{
    
}

void ReceiveSystem::OnEndUpdate(float _dt)
{
    if (m_client != nullptr)
        HandleClientReceive();
    if (m_server != nullptr)
        HandleServerReceive();
}

void ReceiveSystem::HandleClientReceive()
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
            case PacketType::AddScene:
                {
                    m_client->SendAck(p.header.ackId, m_client->GetServerAddress());
                    
                    if (EngineManager::GetServer() != nullptr)
                        break;

                    SceneManager::CreateScene(p.addScene.name, p.header.sceneId);
                        
                    break;
                }
            case PacketType::SetScene:
                {
                    m_client->SendAck(p.header.ackId, m_client->GetServerAddress());
                    
                    if (EngineManager::GetServer() != nullptr)
                        break;

                    SceneManager::SetCurrentScene(p.header.sceneId);

                    break;
                }
            case PacketType::Spawn:
                {
                    Server* server = EngineManager::GetServer();
                    Scene* scene = SceneManager::GetSceneWithId(p.header.sceneId);
                    if (scene == nullptr) break;

                    if (scene->world->entityManager.IsAlive(p.header.entityId) == true && server == nullptr)
                        break;
                    
                    m_client->SendAck(p.header.ackId, m_client->GetServerAddress());

                    if (server != nullptr)
                        break;

                    scene->world->CreateEntity(p.header.entityId, true);

                    break;
                }
            case PacketType::AddComponent:
                {
                    Server* server = EngineManager::GetServer();
                    Scene* scene = SceneManager::GetSceneWithId(p.header.sceneId);
                    if (scene == nullptr) break;

                    if (scene->world->entityManager.IsAlive(p.header.entityId) == true && server == nullptr)
                        break;

                    m_client->SendAck(p.header.ackId, m_client->GetServerAddress());

                    if (server != nullptr)
                        break;

                    scene->world->AddRawComponent(p.header.entityId, p.addComponent.ComponentId, p.addComponent.size, p.addComponent.data);
                }
            case PacketType::AddScript:
                {
                    Server* server = EngineManager::GetServer();
                    Scene* scene = SceneManager::GetSceneWithId(p.header.sceneId);
                    if (scene == nullptr) break;

                    if (scene->world->entityManager.IsAlive(p.header.entityId) == true && server == nullptr)
                        break;

                    m_client->SendAck(p.header.ackId, m_client->GetServerAddress());

                    if (server != nullptr)
                        break;

                    
                }
            default:
                break;
        }
    }
    vReceived.clear();
    m_client->GetCritSection().Leave();
}

void ReceiveSystem::HandleServerReceive()
{
    Vector<ReceivedPacket>& vReceived = m_server->GetReceived();

    m_server->GetCritSection().Enter();
    for (ReceivedPacket& received : vReceived)
    {
        Packet p = received.packet;
        sockaddr_in addr = received.sender;
        
        switch (p.header.type)
        {
            case PacketType::Ack:
                {
                    m_server->OnAckReceived(p.header.ackId);
                    
                    break;
                }
            case PacketType::Connect:
                {
                    m_server->AddClient(addr);

                    Packet np;
                    np.header.type = PacketType::ConnectAck;
                    np.header.ackId = p.header.ackId;
                    np.connect.addr = m_server->GetSocket()->GetAddr();
                        
                    m_server->RegisterTargetedPacket(np, addr);

                    m_server->QueueSyncPackets(addr);
                    
                    break;
                }
            default:
                break;
        }
    }
    vReceived.clear();
    m_server->GetCritSection().Leave();
}


