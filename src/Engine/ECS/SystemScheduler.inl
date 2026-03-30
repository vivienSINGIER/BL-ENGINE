#ifndef SYSTEM_SCHEDULER_INL_DEFINED
#define SYSTEM_SCHEDULER_INL_DEFINED 

#include "Scene.h"
#include "SceneManager.h"
#include "SystemScheduler.h"

template <typename T, typename ... Args>
T* SystemScheduler::RegisterSystem(Phase _phase, uint8 _nFlag, Args&&... args)
{
    T* system = new T(std::forward<Args>(args)...);
    system->networkFlags = _nFlag;
    system->OnRegister(SceneManager::GetCurrentScene()->world);
    m_phases[_phase].push_back(system);
    return system;
}

template <typename T>
T* SystemScheduler::GetSystem()
{
    for (auto& phase : m_phases)
    {
        for (auto& system : phase)
        {
            T* result = dynamic_cast<T*>(system);
            if (result != nullptr) return result;
        }
    }

    return nullptr;
}

#endif
