#ifndef ISYSTEM_HPP_DEFINED
#define ISYSTEM_HPP_DEFINED

#include "World.h"

struct ISystem 
{
    virtual void OnRegister(World& _world) = 0;
    virtual void Update(float _dt) = 0;
    virtual ~ISystem() = default;

    bool isDirty = false;
};

#endif