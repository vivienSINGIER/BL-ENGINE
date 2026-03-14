#ifndef WORLD_H_DEFINED
#define WORLD_H_DEFINED

#include "ArchetypeRegistry.h"
#include "Engine.h"
#include "EntityManager.h"
#include "Query.hpp"
#include "SystemScheduler.h"

class World 
{
public:
    World();

    EntityId CreateEntity();
    void DestroyEntity(EntityId _entity);

    template <typename T> T& AddComponent(EntityId _e, T const& _val = T{});
    template <typename T> void RemoveComponent(EntityId _e);
    template <typename T> T& GetComponent(EntityId _e);
    template <typename T> bool HasComponent(EntityId _e);

    template <typename T, typename... Args>
    T* RegisterSystem(Phase _phase, Args&&... args);
    void Update(float _dt);

    void RegisterQuery(QueryBase* _query);
    void OnArchetypeCreated(Archetype* _arch);

private:
    EntityManager m_entityManager;
    ArchetypeRegistry m_archetypeRegistry;
    SystemScheduler m_systemScheduler;
    Vector<QueryBase*> m_queries;
    
    void MoveEntity(EntityId _entity, EntityRecord& rec, Archetype* _src, Archetype* _dst);
    void RemoveFromArchetype(EntityId _e, EntityRecord& _rec);

    Archetype* GetOrCreateEdge(Archetype* _src, ComponentId _cid, bool _add);

    static void TryMatchQuery(QueryBase* _query, Archetype* _arch);
};

#include "World.inl"

#endif
