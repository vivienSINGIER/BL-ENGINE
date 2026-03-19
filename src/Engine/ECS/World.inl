#ifndef WORLD_INL_DEFINED
#define WORLD_INL_DEFINED

#include <wrl/internal.h>

#include "ISystem.hpp"
#include "World.h"
#include "Script.hpp"

template <typename T>
T& World::AddComponent(EntityId _e, T const& _val)
{
    assert(m_entityManager.IsAlive(_e) && "Can't add component to dead entity");

    ComponentId cid = ComponentRegistry::Id<T>();
    EntityRecord& rec = m_entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    assert(!src->mask.test(cid) && "Component already present");

    Archetype* dst = GetOrCreateEdge(src, cid, true);

    MoveEntity(_e, rec, src, dst);

    dst->storage.Push<T>(cid, _val);

    rec = m_entityManager.GetRecord(_e);
    T& stored = dst->storage.Get<T>(cid, rec.row);
    return stored;
}

template <typename T>
void World::RemoveComponent(EntityId _e)
{
    assert(m_entityManager.IsAlive(_e) && "Can't remove component from dead entity");

    ComponentId cid = ComponentRegistry::Id<T>();
    EntityRecord& rec = m_entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    assert(src->mask.test(cid) && "Component not present");

    Archetype* dst = GetOrCreateEdge(src, cid, false);
    MoveEntity(_e, rec, src, dst);
}

template <typename T>
T& World::GetComponent(EntityId _e)
{
    assert(m_entityManager.IsAlive(_e) && "Can't access component from dead entity");

    ComponentId cid = ComponentRegistry::Id<T>();
    EntityRecord& rec = m_entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    assert(src->mask.test(cid) && "Component not present");

    T& stored = src->storage.Get<T>(cid, rec.row);
    return stored;
}

template <typename T>
bool World::HasComponent(EntityId _e)
{
    assert(m_entityManager.IsAlive(_e) && "Can't access component from dead entity");

    ComponentId cid = ComponentRegistry::Id<T>();
    EntityRecord& rec = m_entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    return src->mask.test(cid);
}

template <typename T>
void World::SetActiveComponent(EntityId _e, bool _value)
{
    assert(m_entityManager.IsAlive(_e) && "Can't set active component on dead entity");

    ComponentId cid = ComponentRegistry::Id<T>();
    EntityRecord& rec = m_entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    assert(src->mask.test(cid) && "Component not present");

    src->storage.SetActive(cid, rec.row, _value);
}

template <typename T>
bool World::IsActiveComponent(EntityId _e)
{
    assert(m_entityManager.IsAlive(_e) && "Can't set active component on dead entity");

    ComponentId cid = ComponentRegistry::Id<T>();
    EntityRecord& rec = m_entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    assert(src->mask.test(cid) && "Component not present");

    return src->storage.GetActive(cid, rec.row);
}

template <typename T>
T& World::AddScript(EntityId _e)
{
    if (!HasComponent<ScriptRegistry>(_e))
        AddComponent<ScriptRegistry>(_e);

    AddComponent<T>(_e);

    CheckScriptSystem<T>();
    
    T& script = GetComponent<T>(_e);
    script.world = this;
    script.entity = _e;

    ComponentId cid = ComponentRegistry::Id<T>();
    ScriptRegistry& reg = GetComponent<ScriptRegistry>(_e);
    reg.push_back(cid);

    script.Awake();

    return script;
}

template <typename T>
void World::RemoveScript(EntityId _e)
{
    GetComponent<T>(_e).Destroy();
    RemoveComponent<T>(_e);

    ScriptRegistry& reg = GetComponent<ScriptRegistry>(_e);
    ComponentId cid = ComponentRegistry::Id<T>();
    reg.remove(cid);
}

template <typename T>
T& World::GetScript(EntityId _e)
{
    return GetComponent<T>(_e);
}

template <typename T>
bool World::HasScript(EntityId _e)
{
    return HasComponent<T>(_e);
}

template <typename T>
void World::SetActiveScript(EntityId _e, bool _value)
{
    return SetActiveComponent<T>(_e, _value);
}

template <typename T>
bool World::IsActiveScript(EntityId _e)
{
    return IsActiveComponent<T>(_e);
}

template <typename ... Args>
void World::NotifyScripts(EntityId _e, void(IScript::*_fn)(Args...), Args... args)
{
    assert(HasComponent<ScriptRegistry>(_e) && "No scripts linked to entity");

    auto& reg = GetComponent<ScriptRegistry>(_e);
    for (int i = 0; i < reg.count; i++ )
    {
        IScript* script = m_scriptSystems[reg.ownedScripts[i]](_e, *this);
        (script->*_fn)(args...);
    }
}

template <typename T, typename ... Args>
T* World::RegisterSystem(Phase _phase, uint8 _flag, Args&&... args)
{
    T* sys = m_systemScheduler.AddSystem<T>(_phase, std::forward<Args>(args)...);
    sys->networkFlags = _flag;
    sys->OnRegister(*this);
    return sys;
}

template <typename T>
void World::CheckScriptSystem()
{
    ComponentId cid = ComponentRegistry::Id<T>();
    if (m_scriptSystems.find(cid) == m_scriptSystems.end())
    {
        m_scriptSystems[cid] = [](EntityId _entity, World& _world) -> IScript*
        {
            return &_world.GetComponent<T>(_entity);
        };
        RegisterSystem<ScriptSystem<T>>(Phase::Update);
    }
}

#endif
