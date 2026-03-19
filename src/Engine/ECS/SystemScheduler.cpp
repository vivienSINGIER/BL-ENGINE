#include "SystemScheduler.h"

#include "EngineManager.h"
#include "ISystem.hpp"
#include "Generic/Base/Window.h"

void SystemScheduler::Run(float _dt)
{
    NetworkFlag flag = EngineManager::GetNetworkFlag();

    for (int i = 0; i < Phase::Count; i++)
    {
        if (i == Phase::PreRender && (flag & ServerOnly) == ServerOnly)
            EngineManager::GetInstance().GetWindow()->Clear();
        
        for (ISystem* sys : m_phases[i])
        {
            if((sys->networkFlags & flag) == sys->networkFlags)
                sys->Update(_dt);
        }

        if (i == Phase::PostRender && (flag & ServerOnly) == ServerOnly)
            EngineManager::GetInstance().GetWindow()->Display();
    }
}
