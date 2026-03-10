#ifndef SYSTEM_SCHEDULER_HPP_DEFINED
#define SYSTEM_SCHEDULER_HPP_DEFINED

#include "Archetype.hpp"
#include "Engine.h"

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
    Map<Phase, Vector<ISystem*>> m_phases;
};

#include "SystemScheduler.inl"

#endif