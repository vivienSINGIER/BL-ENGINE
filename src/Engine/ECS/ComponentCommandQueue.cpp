#include "ComponentCommandQueue.h"

#include "World.h"

#include "ArchetypeRegistry.h"
#include "EngineManager.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Components/NetworkComponent.hpp"
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
    cmd.isScript = ComponentRegistry::IsScript(_cid);

    cmd.applyFunc = [_size](ComponentId _cid, const void* _data, ComponentStorage& _storage) -> void
    {
        _storage.PushRaw(_cid, static_cast<const Byte*>(_data), _size);
    };
    
    m_toAdd.emplace(m_toAdd.begin(), cmd);
    Command& ref = m_toAdd.front();;
    
    ref.data.resize(ref.size);
    void* ptr = ref.data.data();
    if (_data != nullptr)
        memcpy(ptr, _data, _size);
    
    if (ref.isScript)
        ComponentRegistry::ConstructScript(ref.component, ptr);
    
    if (m_requiredMasks.contains(_e))
    {
        m_requiredMasks[_e].reset(cmd.component);
        if (m_requiredMasks[_e] == 0)
            m_requiredMasks.erase(_e);
    }
    
    return ptr;
}

void ComponentCommandQueue::SetRequiredMask(EntityId _e, ComponentMask _mask)
{
    m_requiredMasks[_e] = _mask;
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
    auto it = m_toAdd.end();
    while (it != m_toAdd.begin())
    {
        --it;
        FlushAdd(_pWorld, it);
        if (m_toAdd.empty()) break;
    }
    while (m_toRemove.empty() == false)
    {
        FlushRemove(_pWorld);
    }
    while (m_toDestroy.empty() == false)
    {
        FlushDestroy(_pWorld);
    }
}

void* ComponentCommandQueue::GetComponent(EntityId _e, ComponentId _cid)
{
    if (m_toAdd.empty() == true) return nullptr;
    
    for (Command& cmd : m_toAdd)
    {
        if (cmd.entity != _e)       continue;
        if (cmd.component != _cid)  continue;
        
        return cmd.data.data();
    }
    
    return nullptr;
}

bool ComponentCommandQueue::HasComponent(EntityId _e, ComponentId _cid)
{
    if (m_toAdd.empty() == true) return false;
    
    for (Command& cmd : m_toAdd)
    {
        if (cmd.entity != _e)       continue;
        if (cmd.component != _cid)  continue;
        
        return true;
    }
    
    return false;
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

void ComponentCommandQueue::FlushAdd(World* _pWorld, Vector<Command>::iterator& _it)
{
    Command& cmd = *_it;
            
    if (m_requiredMasks.contains(cmd.entity)) return;
    
    ComponentId cid = cmd.component;
    EntityRecord& rec = _pWorld->entityManager.GetRecord(cmd.entity);
    Archetype* src = rec.archetype;

    if (src->mask.test(cid))
    {
        _it = m_toAdd.erase(_it);
        return;
    }
        
    Archetype* dst = _pWorld->GetOrCreateEdge(src, cid, true);

    _pWorld->MoveEntity(cmd.entity, rec, src, dst);
    (cmd.applyFunc)(cid, cmd.data.data(), dst->storage);
        
    Server* server = EngineManager::GetServer();
    if (server == nullptr || cmd.isClientSide == true)
    {
        _it = m_toAdd.erase(_it);
        return;
    }
        
    Packet p;

    if (cmd.isScript == false)
    {
        p.header.type = PacketType::AddComponent;
        p.header.entityId = cmd.entity;
        p.header.sceneId = SceneManager::GetCurrentScene()->GetId();

        p.addComponent.ComponentId = cid;
        p.addComponent.size = cmd.size;
        memcpy(p.addComponent.data, cmd.data.data(), cmd.size);   
    }
    else
    {
        p.header.type = PacketType::AddScript;
        p.header.entityId = cmd.entity;
        p.header.sceneId = SceneManager::GetCurrentScene()->GetId();

        p.addScript.ComponentId = cid;
    }
        
    server->SendGeneralReliablePacket(p);
    _it = m_toAdd.erase(_it);
}

void ComponentCommandQueue::FlushRemove(World* _pWorld)
{
    Command& cmd = m_toRemove.back();

    ComponentId cid = cmd.component;
    EntityRecord& rec = _pWorld->entityManager.GetRecord(cmd.entity);
    Archetype* src = rec.archetype;
    
    if (!src->mask.test(cid)) // Component not present
    {
        m_toRemove.pop_back();
        return;
    }
        
    Archetype* dst = _pWorld->GetOrCreateEdge(src, cid, false);
    _pWorld->MoveEntity(cmd.entity, rec, src, dst);
    
    m_toRemove.pop_back();
    
    Server* server = EngineManager::GetServer();
    if (server == nullptr) return;
    if (cmd.isClientSide == true) return;
        
    Packet p;
    p.header.type = PacketType::RemoveComponent;
    p.header.entityId = cmd.entity;
    p.header.sceneId = SceneManager::GetCurrentScene()->GetId();
    p.removeComponent.cid = cmd.component;
    
    server->SendGeneralReliablePacket(p);
}

void ComponentCommandQueue::FlushDestroy(World* _pWorld)
{
    Command& cmd = m_toDestroy.back();

    ComponentId cid = cmd.component;
    EntityRecord& rec = _pWorld->entityManager.GetRecord(cmd.entity);
    Archetype* src = rec.archetype;

    assert(_pWorld->entityManager.IsAlive(cmd.entity) && "Destroying dead entity");
        
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

