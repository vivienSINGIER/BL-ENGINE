#include "SystemScheduler.h"

#include "ISystem.hpp"

void SystemScheduler::Run(float _dt)
{
    for (int i = 0; i < Phase::Count; i++)
    {
        for (ISystem* sys : m_phases[(Phase)i])
        {
            sys->Update(_dt);
        }
    }
}
