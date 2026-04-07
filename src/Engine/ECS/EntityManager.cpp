#include "EntityManager.h"

EntityId EntityManager::Create(EntityId id, bool isSetId)
{
    if (isSetId == true)
    {
        uint32 index = EntityIndex(id);
        uint32 gen   = EntityGen(id);

        if  (index >= m_vRecords.size())
        {
            uint32 oldSize = (uint32)m_vRecords.size();
            m_vRecords.resize(index + 1, {nullptr, false, 0, 0});

            for (uint32 i = oldSize; i < index; i++)
                m_freeList.push_back(i);
        }

        assert(m_vRecords[index].isActive == false && "Entity already exists");

        auto it = std::find(m_freeList.begin(), m_freeList.end(), index);
        if (it != m_freeList.end())
            m_freeList.erase(it);

        m_vRecords[index].isActive = true;
        m_vRecords[index].gen = gen;
        return MakeEntity(index, gen);
    }
    
    if (m_freeList.empty() == false)
    {
        uint32 index = m_freeList.back();
        m_freeList.pop_back();

        m_vRecords[index].gen++;
        m_vRecords[index].isActive = true;
        return MakeEntity(index, m_vRecords[index].gen);
    }

    uint32 index = (uint32)m_vRecords.size();
    m_vRecords.push_back({nullptr, true, 0, 1}); // Row is set to 0 until components are stored in the archetype
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
    uint32 gen = EntityGen(_e);
    return index < m_vRecords.size() && m_vRecords[index].gen == EntityGen(_e);
}

EntityRecord& EntityManager::GetRecord(EntityId _e)
{
    assert(IsAlive(_e) && "Entity record is inaccessible");
    return m_vRecords[EntityIndex(_e)];
}

