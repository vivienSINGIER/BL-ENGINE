#ifndef SYSTEM_SCHEDULER_HPP_DEFINED
#define SYSTEM_SCHEDULER_HPP_DEFINED

#include "Archetype.hpp"
#include "../define.h"

struct ISystem;

enum Phase
{
    PreUpdate, Update, PostUpdate, PreRender, Render, PostRender, Count
};

class SystemScheduler
{
public:
    template <typename T, typename... Args>
    T* AddSystem(Phase _phase, Args&&... args);
    
    void Run(float _dt);
    
    
private:
    Array<Vector<ISystem*>, Phase::Count> m_phases;
};

#include "SystemScheduler.inl"

#endif