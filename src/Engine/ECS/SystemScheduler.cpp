#include "SystemScheduler.h"

#include "EngineManager.h"
#include "ISystem.h"
#include "Generic/Base/Window.h"
#include "World.h"

void SystemScheduler::BindWorld(World* world)
{
    for (auto& phase : m_phases)
    {
        for (ISystem* sys : phase)
        {
            sys->OnRegister(world);
        }
    }
}

void SystemScheduler::Run(float _dt)
{
    uint8 flag = EngineManager::GetNetworkFlag();

    m_accumulator += _dt;
    for (int i = 0; i < Phase::Count; ++i)
    {
        if (i == Phase::PreRender && (flag & CLIENT) == CLIENT)
			EngineManager::GetInstance().GetWindow()->Clear();

        if (i == Phase::FixedUpdate)
        {
			const float m_fixedDeltaTime = 1.0f / 60.0f;
            while (m_accumulator >= m_fixedDeltaTime)
            {
                for (ISystem* sys : m_phases[i])
                {
                    if ((sys->networkFlags & flag))
                        sys->Update(m_fixedDeltaTime);
                }
                m_accumulator -= m_fixedDeltaTime;
            }
        }
        else
        {
            for (ISystem* sys : m_phases[i])
            {
                if ((sys->networkFlags & flag))
                    sys->Update(_dt);
            }
        }
        

		if (i == Phase::PostRender && (flag & CLIENT) == CLIENT)
			EngineManager::GetInstance().GetWindow()->Display();
	}
}
