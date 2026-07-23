#include "LightSystem.h"

#include "EngineManager.h"

void LightSystem::OnStartUpdate(float _dt)
{
    count = 0;
    lightDescriptors.clear();
}

void LightSystem::OnUpdate(float _dt, EntityId _e, LightComponent& _l, TransformComponent& _t)
{
    if ( count < MAX_LIGHTS)
    {
        LightDescriptor desc;
        desc.light.Color = _l.color;
        desc.light.FalloffStart = _l.falloffStart;
        desc.light.FalloffEnd = _l.falloffEnd;
        desc.light.Strength = _l.strength;
        desc.light.SpotPower = _l.spotPower;

        desc.type = _l.type;

        desc.light.Position = _t.world.GetPosition();
        desc.light.Direction = _t.world.GetForward();
        lightDescriptors.push_back(desc);
        count++;
    }
}

void LightSystem::OnEndUpdate(float _dt)
{
    EngineManager::GetDevice()->SetLights(lightDescriptors);
}
