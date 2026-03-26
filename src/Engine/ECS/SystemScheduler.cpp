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
    for (int i = 0; i < Phase::Count; i++)
    {
        if (i == Phase::PreRender && (flag & CLIENT) == CLIENT)
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

        if (i == Phase::PostRender && (flag & CLIENT) == CLIENT)
            EngineManager::GetInstance().GetWindow()->Display();
    }
}
