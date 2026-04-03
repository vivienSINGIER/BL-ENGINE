#ifndef COMPONENTID_HPP_DEFINED
#define COMPONENTID_HPP_DEFINED

#include "../define.h"

struct ComponentType
{
    template<typename T>
    static ComponentId Id()
    {
        static ComponentId id = counter++;
        return id;
    }

    static ComponentId Count() { return counter; }
private:
    static inline ComponentId counter = 0;
};

#endif