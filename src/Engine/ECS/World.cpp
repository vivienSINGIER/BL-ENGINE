#include "World.h"
#include "ComponentRegistry.h"
#include "Network/Client.h"

World::World()
{
    m_archetypeRegistry.SetWorld(this);
}

Vector<EntityId> World::GetEntities()
{
    Vector<EntityId> entities;
    for (Archetype* archetype : m_archetypeRegistry.All())
    {
        for (EntityId id : archetype->entities)
            entities.push_back(id);
    }
    return entities;
}

EntityId World::CreateEntity(EntityId _id, bool isCopied, ComponentMask _requiredMask)
{
    EntityId e;
    if (isCopied == true)
        e = entityManager.Create(_id, true);
    else
        e = entityManager.Create();

    Archetype* root = m_archetypeRegistry.GetOrCreate(ComponentMask{});
    EntityRecord& rec = entityManager.GetRecord(e);
    rec.archetype = root;
    rec.row = (uint32)root->entities.size();

    root->entities.push_back(e);
    root->storage.FinishPush();

    m_commandQueue.EmplaceCreate(e);
    if (_requiredMask.count() > 0)
        m_commandQueue.SetRequiredMask(_id, _requiredMask);
    
    return e;
}

void World::DestroyEntity(EntityId _entity)
{
    assert(entityManager.IsAlive(_entity) && "Destroying dead entity");

    m_commandQueue.EmplaceDestroy(_entity);
}

void World::SetActive(EntityId _entity)
{
    EntityRecord& rec = entityManager.GetRecord(_entity);
    rec.isActive = true;
}

void World::SetInactive(EntityId _entity)
{
    EntityRecord& rec = entityManager.GetRecord(_entity);
    rec.isActive = false;
}

bool World::IsActive(EntityId _entity)
{
    return entityManager.GetRecord(_entity).isActive;
}

void World::AddRawComponent(EntityId _e, ComponentId _cid, uint64 _size, const void* _data)
{
    assert(entityManager.IsAlive(_e) && "Can't add component to dead entity");
    assert(ComponentRegistry::IsRegistered(_cid) && "Component is not registered");
    assert(!ComponentRegistry::IsScript(_cid) && "Component should not be a script");

    m_commandQueue.EmplaceAddRaw(_e, _cid, _size, _data);
}

void World::RemoveRawComponent(EntityId _e, ComponentId _cid)
{
    assert(entityManager.IsAlive(_e) && "Can't remove component from dead entity");
    assert(ComponentRegistry::IsRegistered(_cid) && "Component is not registered");

    m_commandQueue.EmplaceRemoveRaw(_e, _cid);
}

void* World::GetRawComponent(EntityId _e, ComponentId _cid)
{
    assert(entityManager.IsAlive(_e) && "Can't access component from dead entity");
    assert(ComponentRegistry::IsRegistered(_cid) && "Component is not registered");

    ComponentId cid = _cid;
    EntityRecord& rec = entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    assert(src->mask.test(cid) && "Component not present");

    void* stored = src->storage.GetRaw(cid, rec.row);
    return stored;
}

void World::AddRawScript(EntityId _e, ComponentId _cid)
{
    assert(ComponentRegistry::IsRegistered(_cid) && "Script is not registered");
    assert(ComponentRegistry::IsScript(_cid) && "Script should not be a component");
    
    ScriptRegistry* reg = nullptr;
    if (!HasComponent<ScriptRegistry>(_e))
        reg = &AddComponent<ScriptRegistry>(_e);
    else
        reg = &GetComponent<ScriptRegistry>(_e);
    
    void* ptr = m_commandQueue.EmplaceAddRaw(_e, _cid, ComponentRegistry::GetSize(_cid), nullptr);
    IScript* script = ComponentRegistry::ConstructScript(_cid, ptr);
    
    script->sceneId = m_sceneId;
    script->entity = _e;
    script->m_isSynced = false;
    
    ComponentId cid = _cid;
    reg->push_back(cid);

    script->Awake();
}

void World::RemoveRawScript(EntityId _e, ComponentId _cid)
{
    assert(ComponentRegistry::IsRegistered(_cid) && "Script is not registered");
    
    GetRawScript(_e, _cid)->Destroy();
    m_commandQueue.EmplaceRemoveRaw(_e, _cid);

    ScriptRegistry& reg = GetComponent<ScriptRegistry>(_e);
    reg.remove(_cid);
}

IScript* World::GetRawScript(EntityId _e, ComponentId _cid)
{
    assert(entityManager.IsAlive(_e) && "Can't access script from dead entity");
    assert(ComponentRegistry::IsRegistered(_cid) && "Script is not registered");

    ComponentId cid = _cid;
    EntityRecord& rec = entityManager.GetRecord(_e);
    Archetype* src = rec.archetype;

    assert(src->mask.test(cid) && "Component not present");

    IScript* stored = reinterpret_cast<IScript*>(src->storage.GetRaw(cid, rec.row));
    return stored;
}

void World::Update(float _dt)
{
    SystemScheduler::Get().Run(_dt);
    m_commandQueue.Flush(this);
}

void World::RegisterQuery(QueryBase* _query)
{
    m_queries.push_back(_query);

    for (Archetype* archetype : m_archetypeRegistry.All())
    {
        TryMatchQuery(_query, archetype);
    }
}

void World::OnArchetypeCreated(Archetype* _arch)
{
    for (QueryBase* query : m_queries)
    {
        TryMatchQuery(query, _arch);
    }
}

void World::MoveEntity(EntityId _entity, EntityRecord& _rec, Archetype* _src, Archetype* _dst)
{
    uint32 srcRow = _rec.row;

    // Copies columns from src to dest
    for (auto& [cid, col] : _src->storage.columns)
    {
        if (_dst->mask.test(cid))
        {
            uint64 stride = _src->storage.strides[cid];
            Byte* raw = _src->storage.GetRaw(cid, srcRow);

            Vector<Byte>& dstCol = _dst->storage.columns[cid];
            dstCol.insert(dstCol.end(), raw, raw + stride);
            _dst->storage.strides[cid] = stride;
        }
    }
    for (auto& [cid, col] : _src->storage.activeStates)
    {
        if (_dst->mask.test(cid))
        {
            Vector<bool>& srcCol = _src->storage.activeStates[cid];
            Vector<bool>& dstCol = _dst->storage.activeStates[cid];

            dstCol.insert(dstCol.end(), srcCol.begin(), srcCol.end());
        }
    }

    uint32 dstRow = (uint32)_dst->entities.size();
    _dst->entities.push_back(_entity);
    _dst->storage.FinishPush();

    RemoveFromArchetype(_entity, _rec);
    
    // Updates entity records
    _rec.archetype = _dst;
    _rec.row = dstRow;
}

void World::RemoveFromArchetype(EntityId _e, EntityRecord& _rec)
{
    Archetype* src = _rec.archetype;
    uint32 srcRow  = _rec.row;

    EntityId last = src->entities.back();
    if (last != _e)
    {
        entityManager.GetRecord(last).row = srcRow;
    }

    src->storage.SwapRemove(srcRow);
    src->entities[srcRow] = last;
    src->entities.pop_back();
}

Archetype* World::GetOrCreateEdge(Archetype* _src, ComponentId _cid, bool _add)
{
    Array<Archetype*, MAX_COMPONENTS>& edges = _add ? _src->edgeAdd : _src->edgeRemove;

    if (edges[_cid] != nullptr)
    {
        return edges[_cid];
    }

    ComponentMask newMask = _src->mask;
    _add ? newMask.set(_cid) : newMask.reset(_cid);

    Archetype* dst = m_archetypeRegistry.GetOrCreate(newMask);
    edges[_cid] = dst;

    Array<Archetype*, MAX_COMPONENTS>& opEdges = !_add ? dst->edgeAdd : dst->edgeRemove;
    if (opEdges[_cid] == nullptr)
    {
        opEdges[_cid] = _src;
    }
    
    return dst;
}

void World::TryMatchQuery(QueryBase* _query, Archetype* _arch)
{
    bool match = true;
    if ((_query->required & _arch->mask) != _query->required)
        match = false;

    for (ComponentMask& mask : _query->orMasks)
    {
        if ((mask & _arch->mask).any() == false)
            match = false;
    }

    if (match)
        _query->matched.push_back(_arch);
}
