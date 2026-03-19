#ifndef ARCHETYPE_HPP_DEFINED
#define ARCHETYPE_HPP_DEFINED

#include "../define.h"
#include "ComponentStorage.hpp"
#include "EntityManager.h"

struct Archetype
{
    ComponentMask mask;
    ComponentStorage storage;
    Vector<EntityId> entities;
    
    Array<Archetype*, MAX_COMPONENTS> edgeAdd{};
    Array<Archetype*, MAX_COMPONENTS> edgeRemove{};

    Archetype(ComponentMask _mask) : mask(_mask) {}
};

#endif
