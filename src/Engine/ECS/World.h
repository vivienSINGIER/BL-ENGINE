#ifndef WORLD_H_DEFINED
#define WORLD_H_DEFINED

#include "../define.h"

#include "ArchetypeRegistry.h"
#include "EntityManager.h"
#include "Query.hpp"
#include "ComponentCommandQueue.h"

struct IScript;

class World 
{
public:
    World();

    EntityManager entityManager;

    Vector<EntityId> GetEntities();
    
    EntityId CreateEntity(EntityId _id = 0, bool isCopied = false);
    void DestroyEntity(EntityId _entity);
    
    void SetActive(EntityId _entity);
    void SetInactive(EntityId _entity);
    bool IsActive(EntityId _entity);

    void AddRawComponent(EntityId _e, ComponentId _cid, uint64 _size, const void* _data);
    void RemoveRawComponent(EntityId _e, ComponentId _cid);
    void* GetRawComponent(EntityId _e, ComponentId _cid);
    
    void AddRawScript(EntityId _e, ComponentId _cid);
    void RemoveRawScript(EntityId _e, ComponentId _cid);
    IScript* GetRawScript(EntityId _e, ComponentId _cid);
    
    template <typename T> T& AddComponent(EntityId _e, bool _isClientSide = false, T const& _val = T{});
    template <typename T> void RemoveComponent(EntityId _e, bool _isClientSide = false);
    template <typename T> T& GetComponent(EntityId _e);
    template <typename T> bool HasComponent(EntityId _e);
    template <typename T> void SetActiveComponent(EntityId _e, bool _value);
    template <typename T> bool IsActiveComponent(EntityId _e);

    template <typename T> T& AddScript(EntityId _e, bool _isClientSide = false);
    template <typename T> void RemoveScript(EntityId _e, bool _isClientSide = false);
    template <typename T> T& GetScript(EntityId _e);
    template <typename T> bool HasScript(EntityId _e);
    template <typename T> void SetActiveScript(EntityId _e, bool _value);
    template <typename T> bool IsActiveScript(EntityId _e);

    template <typename... Args> void NotifyScripts(EntityId _e, void (IScript::*fn)(Args...), Args... args);
    
    void Update(float _dt);

    void RegisterQuery(QueryBase* _query);
    void OnArchetypeCreated(Archetype* _arch);

private:
    uint32 m_sceneId = 0;
    
    ArchetypeRegistry m_archetypeRegistry;
    Vector<QueryBase*> m_queries;
    ComponentCommandQueue m_commandQueue;
    
    void MoveEntity(EntityId _entity, EntityRecord& rec, Archetype* _src, Archetype* _dst);
    void RemoveFromArchetype(EntityId _e, EntityRecord& _rec);
    
    Archetype* GetOrCreateEdge(Archetype* _src, ComponentId _cid, bool _add);

    static void TryMatchQuery(QueryBase* _query, Archetype* _arch);

    friend class ComponentCommandQueue;
    friend class Scene;
};

#include "World.inl"
#include "ISystem.inl"
#include "ComponentRegistry.inl"

#endif
