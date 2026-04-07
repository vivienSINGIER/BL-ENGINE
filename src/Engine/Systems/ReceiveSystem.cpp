#include "ReceiveSystem.h"

#include "EngineManager.h"
#include "InputManager.h"
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
                    m_client->SetId(p.connect.cliendId);
                    break;
                }
            case PacketType::DisconnectAck:
                {
                    m_client->OnAckReceived(p.header.ackId);
                    m_client->Disconnect();
                };
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
                    Scene* scene = SceneManager::GetSceneWithId(p.header.sceneId);
                    if (scene == nullptr) break;
                    
                    m_client->SendAck(p.header.ackId, m_client->GetServerAddress());
                    
                    if (scene->world->entityManager.IsAlive(p.header.entityId)) break;
                    if (EngineManager::GetServer() != nullptr) break;

                    scene->world->CreateEntity(p.header.entityId, true, p.createEntity.componentMask);

                    break;
                }
            case PacketType::AddComponent:
                {
                    Scene* scene = SceneManager::GetSceneWithId(p.header.sceneId);
                    if (scene == nullptr) break;

                    if (!scene->world->entityManager.IsAlive(p.header.entityId)) break;
                    
                    m_client->SendAck(p.header.ackId, m_client->GetServerAddress());

                    if (EngineManager::GetServer() != nullptr) break;

                    scene->world->AddRawComponent(p.header.entityId, p.addComponent.ComponentId, p.addComponent.size, p.addComponent.data);
                    break;
                }
            case PacketType::AddScript:
                {
                    Scene* scene = SceneManager::GetSceneWithId(p.header.sceneId);
                    if (scene == nullptr) break;

                    if (!scene->world->entityManager.IsAlive(p.header.entityId)) break;
                    
                    m_client->SendAck(p.header.ackId, m_client->GetServerAddress());

                    if (EngineManager::GetServer() != nullptr) break;

                    scene->world->AddRawScript(p.header.entityId, p.addScript.ComponentId);
                    break;
                }
            case PacketType::RemoveComponent:
                {
                    Scene* scene = SceneManager::GetSceneWithId(p.header.sceneId);
                    if (scene == nullptr) break;

                    if (!scene->world->entityManager.IsAlive(p.header.entityId)) break;
                    
                    m_client->SendAck(p.header.ackId, m_client->GetServerAddress());

                    if (EngineManager::GetServer() != nullptr) break;
                    
                    if (ComponentRegistry::IsScript(p.removeComponent.cid))
                        scene->world->RemoveRawScript(p.header.entityId, p.removeComponent.cid);
                    else
                        scene->world->RemoveRawComponent(p.header.entityId, p.removeComponent.cid);
                    
                    break;
                }
            case PacketType::Delete:
                {
                    Scene* scene = SceneManager::GetSceneWithId(p.header.sceneId);
                    if (scene == nullptr) break;

                    if (!scene->world->entityManager.IsAlive(p.header.entityId)) break;

                    m_client->SendAck(p.header.ackId, m_client->GetServerAddress());
                    
                    if (EngineManager::GetServer() != nullptr) break;
                    
                    scene->world->DestroyEntity(p.header.entityId);
                    break;
                }
            case PacketType::Update:
                {
                    if (p.header.clientId == m_client->GetId())
                        break;
                    if (m_server != nullptr && p.header.clientId == 0)
                        break;
                    HandleUpdatePacket(p);
                    break;
                }
            case PacketType::SetActiveState:
            {
                Scene* scene = SceneManager::GetSceneWithId(p.header.sceneId);
                if (scene == nullptr) break;

                if (!scene->world->entityManager.IsAlive(p.header.entityId)) break;

                m_client->SendAck(p.header.ackId, m_client->GetServerAddress());
                
                if (EngineManager::GetServer() != nullptr) break;
                    
                if (p.setActiveState.isEntity)
                {
                    if (p.setActiveState.isActive)
                        scene->world->SetActive(p.header.entityId);
                    else 
                        scene->world->SetInactive(p.header.entityId);
                }
                else
                {
                    if (ComponentRegistry::IsScript(p.setActiveState.cid))
                        scene->world->SetActiveScriptRaw(p.header.entityId, p.setActiveState.cid, p.setActiveState.isActive);
                    else
                        scene->world->SetActiveComponentRaw(p.header.entityId, p.setActiveState.cid, p.setActiveState.isActive); 
                }
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
                    
                    ClientInfo* c = m_server->FindClient(addr);

                    if (c->isConnected == true) break;
                    c->isConnected = true;
                    
                    Packet np;
                    np.header.type = PacketType::ConnectAck;
                    np.header.ackId = p.header.ackId;
                    np.connect.addr = m_server->GetSocket()->GetAddr();
                    np.connect.cliendId = c->id;
                        
                    m_server->RegisterTargetedPacket(np, addr);

                    m_server->QueueSyncPackets(addr);
                    
                    break;
                }
            case PacketType::Disconnect:
                {
                    ClientInfo* c = m_server->FindClient(addr);

                    c->isConnected = false;

                    Packet np;
                    np.header.type = PacketType::DisconnectAck;
                    np.header.ackId = p.header.ackId;
                        
                    m_server->RegisterTargetedPacket(np, addr);
                    
                    break;
                }
            case PacketType::MousePosUpdate:
            case PacketType::KeyUpdate:
            case PacketType::MouseButtonUpdate:
                {
                    InputManager::UpdateFromPacket(p, m_server->FindClient(addr)->id);
                    break;
                }
            default:
                break;
        }
    }
    vReceived.clear();
    m_server->GetCritSection().Leave();
}

void ReceiveSystem::HandleUpdatePacket(Packet& p)
{
    Scene* scene = SceneManager::GetSceneWithId(p.header.sceneId);
    if (scene == nullptr) return;
    
    if (scene->world->entityManager.IsAlive(p.header.entityId) == false)
        return;
    
    EntityRecord& rec = scene->world->entityManager.GetRecord(p.header.entityId);
    Archetype* arch = rec.archetype;
    
    for (auto& [cid, col] : arch->storage.columns)
    {
        for (int i = 0; i < p.update.componentCount; i++)
        {
            ComponentEntry& entry = p.update.components[i];
            
            if (cid != entry.ComponentId) continue;

            if (ComponentRegistry::IsScript(cid))
            {
                IScript* before = reinterpret_cast<IScript*>(arch->storage.GetRaw(cid, rec.row));
                IScript* s = reinterpret_cast<IScript*>(entry.data);
                s->m_isSynced = before->m_isSynced;
                memcpy(arch->storage.GetRaw(cid, rec.row), before, entry.size);
            }
            else
                memcpy(arch->storage.GetRaw(cid, rec.row), entry.data, entry.size);
        }
    }
}


