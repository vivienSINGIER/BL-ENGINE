#include "EntityManager.h"

EntityId EntityManager::Create()
{
    if (m_freeList.empty() == false)
    {
        uint32 index = m_freeList.back();
        m_freeList.pop_back();

        m_vRecords[index].gen++;
        m_vRecords[index].isActive = true;
        return MakeEntity(index, m_vRecords[index].gen);
    }

    uint32 index = (uint32)m_vRecords.size();
    m_vRecords.push_back({nullptr, true, 0, 0}); // Row is set to 0 until components are stored in the archetype
    return MakeEntity(index, m_vRecords.back().gen);
}

void EntityManager::Destroy(EntityId _e)
{
    uint32 index = EntityIndex(_e);

    assert(IsAlive(_e) && "Double destroy detected");

    m_vRecords[index].archetype = nullptr;
    m_freeList.push_back(index);
}

bool EntityManager::IsAlive(EntityId _e)
{
    uint32 index = EntityIndex(_e);
    return index < m_vRecords.size() && m_vRecords[index].gen == EntityGen(_e);
}

EntityRecord& EntityManager::GetRecord(EntityId _e)
{
    assert(IsAlive(_e) && "Entity record is inaccessible");
    return m_vRecords[EntityIndex(_e)];
}

