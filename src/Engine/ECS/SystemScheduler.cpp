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
    const float fixedDt = 1.0f / 60.0f;

    for (int i = 0; i < Phase::Count; i++)
    {
        if (i == Phase::PreRender && (flag & CLIENT) == CLIENT)
            EngineManager::GetInstance().GetWindow()->Clear();

        if (i == Phase::FixedUpdate)
        {
            while (m_accumulator >= fixedDt)
            {
                for (ISystem* sys : m_phases[i])
                    sys->Update(fixedDt);

                m_accumulator -= fixedDt;
            }
            else if ( (sys->networkFlags & flag) )
                sys->Update(_dt);
        }

        if (i == Phase::PostRender && (flag & CLIENT) == CLIENT)
            EngineManager::GetInstance().GetWindow()->Display();
    }
}
