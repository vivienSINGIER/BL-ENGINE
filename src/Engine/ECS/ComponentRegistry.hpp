#ifndef COMPONENT_REGISTRY_HPP_DEFINED
#define COMPONENT_REGISTRY_HPP_DEFINED

#include "../define.h"

struct ComponentRegistry 
{
public:
    template<typename T>
    static ComponentId Id()
    {
        static const ComponentId id = NextId();
        return id;
    }

private: 
    static ComponentId NextId()
    {
        static ComponentId counter = 0;
        assert(counter < MAX_COMPONENTS && "Component count out of bounds");
        return counter++;
    }
};

#endif
