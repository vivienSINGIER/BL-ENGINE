#ifndef WORLD_INL_DEFINED
#define WORLD_INL_DEFINED

#include <wrl/internal.h>

#include "ISystem.h"
#include "Script.h"
#include "Network/Client.h"

template <typename T>
T& World::AddComponent(EntityId _e, T const& _val)
{
    assert(IsAlive(_e) && "Can't add component to dead entity");
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Component is not registered");
    assert(!ComponentRegistry::IsScript(ComponentType::Id<T>()) && "Component should not be a script");

    return m_commandQueue.EmplaceAdd<T>(_e, _val);
}

template <typename T>
void World::RemoveComponent(EntityId _e)
{
    assert(IsAlive(_e) && "Can't remove component from dead entity");
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Component is not registered");

    m_commandQueue.EmplaceRemove<T>(_e);
}

template <typename T>
T& World::GetComponent(EntityId _e)
{
    assert(IsAlive(_e) && "Can't access component from dead entity");
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Component is not registered");

    ComponentId cid = ComponentType::Id<T>();
    EntityRecord& rec = entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    bool isInArch = src->mask.test(cid);
    
    if (isInArch)
    {
        T& stored = src->storage.Get<T>(cid, rec.row);
        return stored;   
    }
    
    void* ptr = m_commandQueue.GetComponent(_e, cid);
    assert(ptr && "Component not present");
    
    T& stored = *reinterpret_cast<T*>(ptr);
    return stored;
}

template <typename T>
bool World::HasComponent(EntityId _e)
{
    assert(IsAlive(_e) && "Can't access component from dead entity");
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Component is not registered");

    ComponentId cid = ComponentType::Id<T>();
    EntityRecord& rec = entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    return src->mask.test(cid) || m_commandQueue.HasComponent(_e, cid);
}

template <typename T>
void World::SetActiveComponent(EntityId _e, bool _value)
{
    assert(IsAlive(_e) && "Can't set active component on dead entity");
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Component is not registered");

    ComponentId cid = ComponentType::Id<T>();
    EntityRecord& rec = entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    assert(src->mask.test(cid) && "Component not present");

    src->storage.SetActive(cid, rec.row, _value);
    
    if (EngineManager::IsServer() == false) return;
    
    Packet p;
    p.header.type = PacketType::SetActiveState;
    p.header.entityId = _e;
    p.header.sceneId = SceneManager::GetCurrentScene()->GetId();
    
    p.setActiveState.isActive = _value;
    p.setActiveState.isEntity = false;
    p.setActiveState.cid = cid;
    
    EngineManager::GetServer()->SendGeneralReliablePacket(p);
}

template <typename T>
bool World::IsActiveComponent(EntityId _e)
{
    assert(IsAlive(_e) && "Can't set active component on dead entity");
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Component is not registered");

    ComponentId cid = ComponentType::Id<T>();
    EntityRecord& rec = entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    assert(src->mask.test(cid) && "Component not present");

    return src->storage.GetActive(cid, rec.row);
}

template <typename T>
T& World::AddScript(EntityId _e)
{
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Script is not registered");
    assert(ComponentRegistry::IsScript(ComponentType::Id<T>()) && "Script should not be a component");
    
    ScriptRegistry* reg = nullptr;
    if (!HasComponent<ScriptRegistry>(_e))
        reg = &AddComponent<ScriptRegistry>(_e);
    else
        reg = &GetComponent<ScriptRegistry>(_e);

    T& script = m_commandQueue.EmplaceAdd<T>(_e);
    
    script.sceneId = m_sceneId;
    script.entity = _e;
    script.m_isSynced = false;

    ComponentId cid = ComponentType::Id<T>();
    reg->push_back(cid);

    script.Awake();

    return script;
}

template <typename T>
void World::RemoveScript(EntityId _e)
{
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Script is not registered");
    
    GetComponent<T>(_e).Destroy();
    m_commandQueue.EmplaceRemove<T>(_e);

    ScriptRegistry& reg = GetComponent<ScriptRegistry>(_e);
    ComponentId cid = ComponentType::Id<T>();
    reg.remove(cid);
}

template <typename T>
T& World::GetScript(EntityId _e)
{
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Script is not registered");
    return GetComponent<T>(_e);
}

template <typename T>
bool World::HasScript(EntityId _e)
{
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Script is not registered");
    return HasComponent<T>(_e);
}

template <typename T>
void World::SetActiveScript(EntityId _e, bool _value)
{
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Script is not registered");
    SetActiveComponent<T>(_e, _value);
    
    Packet p;
    p.header.type = PacketType::SetActiveState;
    p.header.entityId = _e;
    p.header.sceneId = SceneManager::GetCurrentScene()->GetId();
    
    p.setActiveState.isActive = _value;
    p.setActiveState.isEntity = false;
    p.setActiveState.cid = ComponentType::Id<T>();
    
    EngineManager::GetServer()->SendGeneralReliablePacket(p);
}

template <typename T>
bool World::IsActiveScript(EntityId _e)
{
    assert(ComponentRegistry::IsRegistered(ComponentType::Id<T>()) && "Script is not registered");
    return IsActiveComponent<T>(_e);
}

template <typename ... Args>
void World::NotifyScripts(EntityId _e, void(IScript::*_fn)(Args...), Args... args)
{
    assert(HasComponent<ScriptRegistry>(_e) && "No scripts linked to entity");

    auto& reg = GetComponent<ScriptRegistry>(_e);
    for (int i = 0; i < reg.count; i++ )
    {
        IScript* script = ComponentRegistry::GetScript(reg.ownedScripts[i], _e, *this);
        (script->*_fn)(args...);
    }
}

#endif
