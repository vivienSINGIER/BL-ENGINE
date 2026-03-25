#ifndef ENTITY_MANAGER_H_DEFINED
#define ENTITY_MANAGER_H_DEFINED

#include "../define.h"

struct Archetype;

constexpr uint32_t EntityIndex(EntityId e) { return uint32_t(e); }
constexpr uint32 EntityGen(EntityId _id) { return uint32(_id >> 32); }
constexpr EntityId MakeEntity(uint32 _id, uint32 _gen)
{
    return uint64(_id) | (uint64(_gen) << 32);
}

struct EntityRecord
{
    Archetype* archetype = nullptr;
    bool isActive = true;
    uint32 row = 0;
    uint32 gen = 0;
};

class EntityManager 
{
public:
    EntityId Create(EntityId id = 0, bool isSetId = false);
    void Destroy(EntityId _e);
    bool IsAlive(EntityId _e);
    EntityRecord& GetRecord(EntityId _e);
    
private:
    Vector<EntityRecord> m_vRecords;
    Vector<uint32> m_freeList;

};

#endif
