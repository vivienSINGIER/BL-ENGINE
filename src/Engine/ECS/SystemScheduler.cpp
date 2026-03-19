#include "SystemScheduler.h"

#include "EngineManager.h"
#include "ISystem.hpp"
#include "Generic/Base/Window.h"

void SystemScheduler::Run(float _dt)
{
    NetworkFlag flag = EngineManager::GetNetworkFlag();

    m_accumulator += _dt;
    for (int i = 0; i < Phase::Count; i++)
    {
        if (i == Phase::PreRender && (flag & ServerOnly) == ServerOnly)
            EngineManager::GetInstance().GetWindow()->Clear();

        for (ISystem* sys : m_phases[i])
        {
            if (i == Phase::FixedUpdate)
            {
                while (m_accumulator >= 0.016666667f)
                {
                    sys->Update(0.016666667f);
                    m_accumulator -= 0.016666667f;
                }
            }
            else if ( (sys->networkFlags & flag) == sys->networkFlags )
                sys->Update(_dt);
        }

        if (i == Phase::PostRender && (flag & ServerOnly) == ServerOnly)
            EngineManager::GetInstance().GetWindow()->Display();
    }
}
