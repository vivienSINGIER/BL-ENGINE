#ifndef WORLD_H_DEFINED
#define WORLD_H_DEFINED

#include <functional>

#include "../define.h"

#include "ArchetypeRegistry.h"
#include "EntityManager.h"
#include "Query.hpp"
#include "SystemScheduler.h"

struct IScript;

class World 
{
public:
    World();

    EntityId CreateEntity();
    void DestroyEntity(EntityId _entity);
    
    void SetActive(EntityId _entity);
    void SetInactive(EntityId _entity);
    bool IsActive(EntityId _entity);
    
    template <typename T> T& AddComponent(EntityId _e, T const& _val = T{});
    template <typename T> void RemoveComponent(EntityId _e);
    template <typename T> T& GetComponent(EntityId _e);
    template <typename T> bool HasComponent(EntityId _e);
    template <typename T> void SetActiveComponent(EntityId _e, bool _value);
    template <typename T> bool IsActiveComponent(EntityId _e);

    template <typename T> T& AddScript(EntityId _e);
    template <typename T> void RemoveScript(EntityId _e);
    template <typename T> T& GetScript(EntityId _e);
    template <typename T> bool HasScript(EntityId _e);
    template <typename T> void SetActiveScript(EntityId _e, bool _value);
    template <typename T> bool IsActiveScript(EntityId _e);

    template <typename... Args> void NotifyScripts(EntityId _e, void (IScript::*fn)(Args...), Args... args);
    
    template <typename T, typename... Args>
    T* RegisterSystem(Phase _phase, uint8 _flag = NetworkFlag::None, Args&&... args);
    void Update(float _dt);

    void RegisterQuery(QueryBase* _query);
    void OnArchetypeCreated(Archetype* _arch);

private:
    EntityManager m_entityManager;
    ArchetypeRegistry m_archetypeRegistry;
    SystemScheduler m_systemScheduler;
    Vector<QueryBase*> m_queries;
    
    UnorderedMap<ComponentId, std::function<IScript*(EntityId, World&)>> m_scriptSystems;
    
    void MoveEntity(EntityId _entity, EntityRecord& rec, Archetype* _src, Archetype* _dst);
    void RemoveFromArchetype(EntityId _e, EntityRecord& _rec);

    template <typename T> void CheckScriptSystem();
    
    Archetype* GetOrCreateEdge(Archetype* _src, ComponentId _cid, bool _add);

    static void TryMatchQuery(QueryBase* _query, Archetype* _arch);
};

#include "World.inl"

#endif
