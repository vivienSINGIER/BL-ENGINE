#include "SendSystem.h"

#include "EngineManager.h"
#include "ECS/World.h"
#include "Network/Client.h"
#include "Network/Server.h"

void SendSystem::OnInit()
{
    
}

void SendSystem::OnStartUpdate(float _dt)
{
    m_client = EngineManager::GetClient();
    m_server = EngineManager::GetServer();
}

void SendSystem::OnUpdate(float _dt, EntityId _e, NetworkComponent& _n)
{
    if (_n.dirty == false)
        return;
    
    _n.dirty = false;
    
    EntityRecord& rec = world->entityManager.GetRecord(_e);
    Archetype* arch = rec.archetype;
    
    Packet p;
    p.header.type = PacketType::Update;
    p.header.entityId = _e;
    p.header.sceneId = SceneManager::GetCurrentScene()->GetId();
    
    for (auto& [cid, col] : arch->storage.columns)
    {
        if (_n.mask.test(cid) == false) continue;
        if (p.update.componentCount >= 10) continue;
        
        ComponentEntry entry;
        entry.ComponentId = cid;
        entry.size = ComponentRegistry::GetSize(cid);
        memcpy(entry.data, arch->storage.GetRaw(cid, rec.row), entry.size);
        
        p.update.components[p.update.componentCount] = entry;
        p.update.componentCount++;
    }
    
    _n.mask.reset();
    
    if (m_server != nullptr)
        m_server->RegisterPacket(p);
    else
        m_client->RegisterPacket(p);
}

void SendSystem::OnEndUpdate(float _dt)
{
    if (m_client != nullptr)
        m_client->Update(_dt);
    if (m_server != nullptr)
        m_server->Update(_dt);
}


