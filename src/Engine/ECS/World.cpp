#include "World.h"

World::World()
{
    m_archetypeRegistry.SetWorld(this);
}

EntityId World::CreateEntity()
{
    EntityId e = m_entityManager.Create();

    Archetype* root = m_archetypeRegistry.GetOrCreate(ComponentMask{});
    EntityRecord& rec = m_entityManager.GetRecord(e);
    rec.archetype = root;
    rec.row = (uint32)root->entities.size();

    root->entities.push_back(e);
    root->storage.FinishPush();

    return e;
}

void World::DestroyEntity(EntityId _entity)
{
    assert(m_entityManager.IsAlive(_entity) && "Destroying dead entity");

    NotifyScripts(_entity, &IScript::Destroy);
    
    EntityRecord& rec = m_entityManager.GetRecord(_entity);
    RemoveFromArchetype(_entity, rec);
    m_entityManager.Destroy(_entity);
}

void World::Update(float _dt)
{
    m_systemScheduler.Run(_dt);
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

    // Copies columns for src to dest
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
        m_entityManager.GetRecord(last).row = srcRow;
    }

    src->storage.SwapRemove(srcRow);
    src->entities[srcRow] = last;
    src->entities.pop_back();
}

Archetype* World::GetOrCreateEdge(Archetype* _src, ComponentId _cid, bool _add)
{
    Array<Archetype*, MAX_COMPONENTS>& edges = _add ? _src->EdgeAdd : _src->EdgeRemove;

    if (edges[_cid] != nullptr)
    {
        return edges[_cid];
    }

    ComponentMask newMask = _src->mask;
    _add ? newMask.set(_cid) : newMask.reset(_cid);

    Archetype* dst = m_archetypeRegistry.GetOrCreate(newMask);
    edges[_cid] = dst;
    return dst;
}

void World::TryMatchQuery(QueryBase* _query, Archetype* _arch)
{
    if ((_query->required & _arch->mask) == _query->required)
        _query->matched.push_back(_arch);
}
