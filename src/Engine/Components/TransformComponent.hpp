#ifndef TRANSFORM_COMPONENT_H_DEFINED
#define TRANSFORM_COMPONENT_H_DEFINED

#include "define.h"
#include "../Core/Transform.h"

struct TransformComponent
{
    Transform transform;

    EntityId parent;
    
    static constexpr uint64 MAX_CHILDREN = 16;
    EntityId children[MAX_CHILDREN] = {};
    uint64 count = 0;
    
    void push_back(EntityId cid) { children[count++] = cid; }
    void remove(EntityId cid)
    {
        if (count == 0) return;
        
        for (int i = 0; i < count - 1; i++)
        {
            if (children[i] == cid)
            {
                children[i] = children[count - 1];
            }
        }
        
        count--;
    }
};

#endif