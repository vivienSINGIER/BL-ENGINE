#ifndef ISYSTEM_HPP_DEFINED
#define ISYSTEM_HPP_DEFINED

#include "World.h"
#include "Query.hpp"

struct  ISystem
{
    virtual void Update(float _dt) = 0;
    virtual void OnRegister(World& _world) = 0;
    
    virtual ~ISystem() = default;
};

template <typename TSystem, typename... TComponents>
struct System : public ISystem
{
    Query<TComponents...> query;

    void OnRegister(World& _world)
    {
        _world.RegisterQuery(&query);
    }

    virtual void OnUpdate(float _dt, TComponents&... _components) = 0;

    void Update(float _dt)
    {
        for (Archetype* arch : query.matched)
        {
            for (uint64 i = 0; i < arch->storage.count; i++)
            {
                OnUpdate(_dt, arch->storage.Get<TComponents>(ComponentRegistry::Id<TComponents>(), i)...);
            }
        }
    }
    
    virtual ~System() = default;
};

#endif