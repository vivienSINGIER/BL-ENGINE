#ifndef LIGHTSYSTEM_H_DEFINED
#define LIGHTSYSTEM_H_DEFINED

#include "Components/LightComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "ECS/ISystem.hpp"

struct LightSystem : public System<LightComponent, TransformComponent>
{
public:
    void OnUpdate(float _dt, LightComponent& _light, TransformComponent& _transform) override;    
};

#endif
