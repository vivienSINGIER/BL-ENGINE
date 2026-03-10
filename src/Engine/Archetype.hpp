#ifndef ARCHETYPE_HPP_DEFINED
#define ARCHETYPE_HPP_DEFINED

#include "Engine.h"
#include "ComponentStorage.hpp"

struct Archetype
{
    ComponentMask mask;
    ComponentStorage storage;
    Vector<EntityId> entities;
    
    Array<EntityId, MAX_COMPONENTS> EdgeAdd{};
    Array<EntityId, MAX_COMPONENTS> EdgeRemove{};
};

#endif
