#ifndef QUERY_HPP_DEFINED
#define QUERY_HPP_DEFINED

#include "../define.h"
#include "ComponentId.hpp"

struct Archetype;

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
        (required.set(ComponentType::Id<Ts>()), ...);
    }

    template<typename... orTs>
    void Any()
    {
        ComponentMask mask;
        (mask.set(ComponentType::Id<orTs>()), ...);
        orMasks.push_back(mask);
    }
};

#endif