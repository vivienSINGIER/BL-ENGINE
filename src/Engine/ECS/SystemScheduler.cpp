#include "SystemScheduler.h"

#include "EngineManager.h"
#include "ISystem.hpp"
#include "Generic/Base/Window.h"

void SystemScheduler::Run(float _dt)
{
    m_accumulator += _dt;
    const float fixedDt = 1.0f / 60.0f;

    for (int i = 0; i < Phase::Count; i++)
    {
        if (i == Phase::PreRender)
            EngineManager::GetInstance().GetWindow()->Clear();

        if (i == Phase::FixedUpdate)
        {
            while (m_accumulator >= fixedDt)
            {
                for (ISystem* sys : m_phases[i])
                    sys->Update(fixedDt);

                m_accumulator -= fixedDt;
            }
        }
        else
        {
            for (ISystem* sys : m_phases[i])
                sys->Update(_dt);
        }

        if (i == Phase::PostRender)
            EngineManager::GetInstance().GetWindow()->Display();
    }
}
