#ifndef QUERY_HPP_DEFINED
#define QUERY_HPP_DEFINED

#include "../define.h"
#include "Archetype.hpp"
#include "ArchetypeRegistry.h"
#include "ComponentRegistry.hpp"

struct QueryBase
{
    ComponentMask      required;
    Vector<Archetype*> matched;
    Vector<ComponentMask> orMasks;
    virtual ~QueryBase() = default;
}; 

template<typename... Ts>
struct Query : QueryBase
{
    Query()
    {
        (required.set(ComponentRegistry::Id<Ts>()), ...);
    }

    template<typename... orTs>
    void Any()
    {
        ComponentMask mask;
        (mask.set(ComponentRegistry::Id<orTs>()), ...);
        orMasks.push_back(mask);
    }
};

#endif