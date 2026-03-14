#ifndef WORLD_INL_DEFINED
#define WORLD_INL_DEFINED

#include <wrl/internal.h>

#include "World.h"

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

template <typename T, typename ... Args>
T* World::RegisterSystem(Phase _phase, Args&&... args)
{
    T* sys = m_systemScheduler.AddSystem<T>(_phase, std::forward<Args>(args)...);
    sys->OnRegister(*this);
    return sys;
}

#endif
