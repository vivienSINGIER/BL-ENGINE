#ifndef SYSTEM_SCHEDULER_INL_DEFINED
#define SYSTEM_SCHEDULER_INL_DEFINED 

#include "SystemScheduler.h"

template <typename T, typename ... Args>
T* SystemScheduler::AddSystem(Phase _phase, Args&&... args)
{
    T* system = new T(args);
    m_phases[_phase] = system;
    return system;
}

#endif
