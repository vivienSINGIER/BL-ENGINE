#include "ComponentCommandQueue.h"

#include "World.h"

#include "ArchetypeRegistry.h"

void ComponentCommandQueue::EmplaceDestroy(EntityId _e)
{
    Command cmd;
    cmd.entity = _e;
    m_toDestroy.emplace(m_toDestroy.begin(), cmd);
}

void ComponentCommandQueue::Flush(World* _pWorld)
{
    while (m_toAdd.empty() == false)
    {
        Command& cmd = m_toAdd.back();
            
        ComponentId cid = cmd.component;
        EntityRecord& rec = _pWorld->m_entityManager.GetRecord(cmd.entity);
        Archetype* src = rec.archetype;

        assert(!src->mask.test(cid) && "Component already present");
        
        Archetype* dst = _pWorld->GetOrCreateEdge(src, cid, true);

        _pWorld->MoveEntity(cmd.entity, rec, src, dst);
        (cmd.applyFunc)(cid, m_componentSideBuffer + cmd.offset, dst->storage);
        m_toAdd.pop_back();
    }
    while (m_toRemove.empty() == false)
    {
        Command& cmd = m_toRemove.back();

        ComponentId cid = cmd.component;
        EntityRecord& rec = _pWorld->m_entityManager.GetRecord(cmd.entity);
        Archetype* src = rec.archetype;

        assert(src->mask.test(cid) && "Component not present");
        
        Archetype* dst = _pWorld->GetOrCreateEdge(src, cid, false);
        _pWorld->MoveEntity(cmd.entity, rec, src, dst);
        m_toRemove.pop_back();
    }
    while (m_toDestroy.empty() == false)
    {
        Command& cmd = m_toRemove.back();

        ComponentId cid = cmd.component;
        EntityRecord& rec = _pWorld->m_entityManager.GetRecord(cmd.entity);
        Archetype* src = rec.archetype;

        assert(_pWorld->m_entityManager.IsAlive(cmd.entity) && "Destroying dead entity");
        
        _pWorld->NotifyScripts(cmd.entity, &IScript::Destroy);
    
        _pWorld->RemoveFromArchetype(cmd.entity, rec);
        _pWorld->m_entityManager.Destroy(cmd.entity);
        m_toDestroy.pop_back();
    }

    m_offset = 0;
}

