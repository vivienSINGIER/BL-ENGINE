#ifndef ISYSTEM_H_DEFINED
#define ISYSTEM_H_DEFINED

#include "define.h"
#include "Query.hpp"

class World;

struct  ISystem
{
    uint8 networkFlags = NetworkFlag::NONE;

    virtual void Update(float _dt) = 0;
    virtual void OnRegister(World* _world) = 0;

    virtual ~ISystem() = default;
};

template <typename... TComponents>
struct System : public ISystem 
{
    Query<TComponents...> query;
    World* world;

    void OnRegister(World* _world);
    
    template <typename TComponent>
    void SetNetworkDirty(EntityId _e);

    virtual void OnInit() {};
    
    virtual void OnStartUpdate(float _dt) {};
    virtual void OnUpdate(float _dt, EntityId _e,  TComponents&... _components) {};
    virtual void OnEndUpdate(float _dt) {};

    virtual void Update(float _dt);
    
    virtual ~System() = default;
};

template <typename TScript>
struct ScriptSystem : System<TScript>
{
    void OnUpdate(float _dt, EntityId _e, TScript& _script);
};

#endif