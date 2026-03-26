#ifndef LIGHTSYSTEM_H_DEFINED
#define LIGHTSYSTEM_H_DEFINED

#include "Components/LightComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "ECS/ISystem.h"

struct LightSystem : public System<LightComponent, TransformComponent>
{
public:
    void OnStartUpdate(float _dt) override;
    void OnUpdate(float _dt, EntityId _e, LightComponent& _l, TransformComponent& _t) override;
    void OnEndUpdate(float _dt) override;

private:
    Vector<LightDescriptor> lightDescriptors = Vector<LightDescriptor>();
    int count = 0;
};

#endif
