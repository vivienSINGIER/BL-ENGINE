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
        OnInit();
        _world.RegisterQuery(&query);
        world = &_world;
    }

    virtual void OnInit() {};
    
    virtual void OnStartUpdate(float _dt) {};
    virtual void OnUpdate(float _dt, EntityId _e,  TComponents&... _components) {};
    virtual void OnEndUpdate(float _dt) {};

    virtual void Update(float _dt)
    {
        OnStartUpdate(_dt);
        for (Archetype* arch : query.matched)
        {
            for (uint64 i = 0; i < arch->storage.count; i++)
            {
                if (world->IsActive(arch->entities.at(i)) == false) continue;
                if (arch->storage.IsRowActive(i, query.required) == false) continue;
                
                OnUpdate(_dt, arch->entities.at(i), arch->storage.Get<TComponents>(ComponentRegistry::Id<TComponents>(), i)...);
            }
        }
        OnEndUpdate(_dt);
    }
    
    virtual ~System() = default;
};

template <typename TScript>
struct ScriptSystem : System<TScript>
{
    void OnUpdate(float _dt, EntityId _e, TScript& _script)
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