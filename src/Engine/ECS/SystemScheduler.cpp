#include "SystemScheduler.h"

#include "EngineManager.h"
#include "ISystem.hpp"
#include "Generic/Base/Window.h"

void SystemScheduler::Run(float _dt)
{
    for (int i = 0; i < Phase::Count; i++)
    {
        if (i == Phase::PreRender)
            EngineManager::GetInstance().GetWindow()->Clear();
        
        for (ISystem* sys : m_phases[i])
        {
            sys->Update(_dt);
        }

        if (i == Phase::PostRender)
            EngineManager::GetInstance().GetWindow()->Display();
    }
}
