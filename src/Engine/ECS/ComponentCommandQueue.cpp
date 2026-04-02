#include "ComponentCommandQueue.h"

#include "World.h"

#include "ArchetypeRegistry.h"
#include "EngineManager.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Components/NetworkComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Systems/BroadPhaseSystem.h"
#include "Network/Packet.hpp"
#include "Network/Server.h"

void ComponentCommandQueue::EmplaceCreate(EntityId _e)
{
    Command cmd;
    cmd.entity = _e;
    m_toCreate.emplace(m_toCreate.begin(), cmd);
}

void* ComponentCommandQueue::EmplaceAddRaw(EntityId _e, ComponentId _cid, uint64 _size, const void* _data)
{
    Command cmd;
    cmd.entity = _e;
    cmd.component = _cid;
    cmd.size = _size;
    cmd.offset = m_offset;
    cmd.isScript = ComponentRegistry::IsScript(_cid);
        
    Byte* ptr = reinterpret_cast<Byte*>(m_componentSideBuffer + m_offset);
    if (_data != nullptr)
        memcpy(ptr, _data, _size);

    cmd.applyFunc = [_size](ComponentId _cid, const void* _data, ComponentStorage& _storage) -> void
    {
        _storage.PushRaw(_cid, static_cast<const Byte*>(_data), _size);
    };

    m_offset += cmd.size;
    m_toAdd.emplace(m_toAdd.begin(), cmd);

    return ptr;
}

void ComponentCommandQueue::EmplaceRemoveRaw(EntityId _e, ComponentId _cid)
{
    Command cmd;
    cmd.entity = _e;
    cmd.component = _cid;
    cmd.isScript = ComponentRegistry::IsScript(_cid);
    m_toRemove.emplace(m_toRemove.begin(), cmd);
}

void ComponentCommandQueue::EmplaceDestroy(EntityId _e)
{
    Command cmd;
    cmd.entity = _e;
    m_toDestroy.emplace(m_toDestroy.begin(), cmd);
}

void ComponentCommandQueue::Flush(World* _pWorld)
{
    while (m_toCreate.empty() == false)
    {
        FlushCreate(_pWorld);
    }
    while (m_toAdd.empty() == false)
    {
        FlushAdd(_pWorld);
    }
    while (m_toRemove.empty() == false)
    {
        FlushRemove(_pWorld);
    }
    while (m_toDestroy.empty() == false)
    {
        FlushDestroy(_pWorld);
    }

    m_offset = 0;
}

void ComponentCommandQueue::FlushCreate(World* _pWorld)
{
    Command& cmd = m_toCreate.back();
        
    m_toCreate.pop_back();

    Server* server = EngineManager::GetServer();
    if (server == nullptr) return;

    _pWorld->AddComponent<NetworkComponent>(cmd.entity);
    
    Packet p;
    p.header.type = PacketType::Spawn;
    p.header.entityId = cmd.entity;
    p.header.sceneId = SceneManager::GetCurrentScene()->GetId();

    server->SendGeneralReliablePacket(p);
}

void ComponentCommandQueue::FlushAdd(World* _pWorld)
{
    Command& cmd = m_toAdd.back();
            
    ComponentId cid = cmd.component;
    EntityRecord& rec = _pWorld->entityManager.GetRecord(cmd.entity);
    Archetype* src = rec.archetype;

    if (src->mask.test(cid)) // Component already present
        return;
        
    Archetype* dst = _pWorld->GetOrCreateEdge(src, cid, true);

    _pWorld->MoveEntity(cmd.entity, rec, src, dst);
    (cmd.applyFunc)(cid, m_componentSideBuffer + cmd.offset, dst->storage);

    m_toAdd.pop_back();
        
    Server* server = EngineManager::GetServer();
    if (server == nullptr) return;
        
    Packet p;

    if (cmd.isScript == false)
    {
        p.header.type = PacketType::AddComponent;
        p.header.entityId = cmd.entity;
        p.header.sceneId = SceneManager::GetCurrentScene()->GetId();

        p.addComponent.ComponentId = cid;
        p.addComponent.size = cmd.size;
        memcpy(p.addComponent.data, m_componentSideBuffer + cmd.offset, cmd.size);   
    }
    else
    {
        p.header.type = PacketType::AddScript;
        p.header.entityId = cmd.entity;
        p.header.sceneId = SceneManager::GetCurrentScene()->GetId();

        p.addScript.ComponentId = cid;
    }
        
    server->SendGeneralReliablePacket(p);
}

void ComponentCommandQueue::FlushRemove(World* _pWorld)
{
    Command& cmd = m_toRemove.back();

    ComponentId cid = cmd.component;
    EntityRecord& rec = _pWorld->entityManager.GetRecord(cmd.entity);
    Archetype* src = rec.archetype;

    if (!src->mask.test(cid)) // Component not present
        return;
        
    Archetype* dst = _pWorld->GetOrCreateEdge(src, cid, false);
    _pWorld->MoveEntity(cmd.entity, rec, src, dst);
    m_toRemove.pop_back();
    
    Server* server = EngineManager::GetServer();
    if (server == nullptr) return;
        
    Packet p;
    p.header.type = PacketType::RemoveComponent;
    p.header.entityId = cmd.entity;
    p.header.sceneId = SceneManager::GetCurrentScene()->GetId();
    p.removeComponent.cid = cmd.component;
    
    server->SendGeneralReliablePacket(p);
}

void ComponentCommandQueue::FlushDestroy(World* _pWorld)
{
    Command& cmd = m_toRemove.back();

    ComponentId cid = cmd.component;
    EntityRecord& rec = _pWorld->entityManager.GetRecord(cmd.entity);
    Archetype* src = rec.archetype;

    assert(_pWorld->entityManager.IsAlive(cmd.entity) && "Destroying dead entity");

    if (_pWorld->HasComponent<ColliderComponent>(cmd.entity))
		SystemScheduler::Get().GetSystem<BroadPhaseSystem>()->OnEntityDestroyed(cmd.entity);

    _pWorld->NotifyScripts(cmd.entity, &IScript::Destroy);
    _pWorld->RemoveFromArchetype(cmd.entity, rec);
    _pWorld->entityManager.Destroy(cmd.entity);

    m_toDestroy.pop_back();
    
    Server* server = EngineManager::GetServer();
    if (server == nullptr) return;
        
    Packet p;
    p.header.type = PacketType::Delete;
    p.header.entityId = cmd.entity;
    p.header.sceneId = SceneManager::GetCurrentScene()->GetId();
    
    server->SendGeneralReliablePacket(p);
}

