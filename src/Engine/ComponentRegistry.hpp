#ifndef COMPONENT_REGISTRY_HPP_DEFINED
#define COMPONENT_REGISTRY_HPP_DEFINED

#include <cassert>

#include "Engine.h"

struct ComponentRegistry 
{
public:
    static ComponentId Id()
    {
        static const ComponentId id = nextId();
        return id;
    }

private:
    static ComponentId nextId()
    {
        static ComponentId counter = 0;
        assert(counter < MAX_COMPONENTS && "Component count out of bounds");
        return counter++;
    }
};

#endif
