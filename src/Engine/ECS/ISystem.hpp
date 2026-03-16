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

template <typename... TComponents>
struct System : public ISystem
{
    Query<TComponents...> query;
    World* world;

    void OnRegister(World& _world)
    {
        _world.RegisterQuery(&query);
        world = &_world;
    }

    virtual void OnUpdate(float _dt, TComponents&... _components) = 0;

    void Update(float _dt)
    {
        for (Archetype* arch : query.matched)
        {
            for (uint64 i = 0; i < arch->storage.count; i++)
            {
                if (world->IsActive(arch->entities.at(i)) == false) continue;
                
                OnUpdate(_dt, arch->storage.Get<TComponents>(ComponentRegistry::Id<TComponents>(), i)...);
            }
        }
    }
    
    virtual ~System() = default;
};

template <typename TScript>
struct ScriptSystem : System<TScript>
{
    void OnUpdate(float _dt, TScript& _script)
    {
        if (_script.m_isStarted == false)
        {
            _script.m_isStarted = true;
            _script.Start();
        }
        
        _script.Update(_dt);
    }
};

#endif