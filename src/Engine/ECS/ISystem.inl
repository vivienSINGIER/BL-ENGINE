#ifndef ISYSTEM_INL_DEFINED
#define ISYSTEM_INL_DEFINED

#include "ISystem.h"
#include "Components/NetworkComponent.hpp"

template <typename... TComponents>
void System<TComponents...>::OnRegister(World* _world)
{
    OnInit();
    query.matched.clear();
    _world->RegisterQuery(&query);
    world = _world;
}

template <typename ... TComponents>
template <typename TComponent>
void System<TComponents...>::SetNetworkDirty(EntityId _e)
{
    if (world->HasComponent<NetworkComponent>(_e) == true)
    {
        NetworkComponent& n = world->GetComponent<NetworkComponent>(_e);
        n.dirty = true;
        n.mask.set(ComponentType::Id<TComponent>());
    }
}

template <typename... TComponents>
void System<TComponents...>::Update(float _dt)
{
    OnStartUpdate(_dt);
    for (Archetype* arch : query.matched)
    {
        for (uint64 i = 0; i < arch->storage.count; i++)
        {
            if (world->IsActive(arch->entities.at(i)) == false) continue;
            if (arch->storage.IsRowActive(i, query.required) == false) continue;
                
            OnUpdate(_dt, arch->entities.at(i), arch->storage.Get<TComponents>(ComponentType::Id<TComponents>(), i)...);
        }
    }
    OnEndUpdate(_dt);
}

template <typename TScript>
void ScriptSystem<TScript>::OnUpdate(float _dt, EntityId _e, TScript& _script)
{
    if (_script.m_isStarted == false)
    {
        _script.m_isStarted = true;
        _script.Start();
    }
    if (_script.m_isSynced == false)
    {
        _script.m_isSynced = true;
        _script.OnSync(EngineManager::GetClient()->GetId());
    }
        
    _script.Update(_dt);
    this->SetNetworkDirty<TScript>(_e);
}


#endif
