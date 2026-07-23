#ifndef TRANSFORM_SYSTEM_H_DEFINED
#define TRANSFORM_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/TransformComponent.hpp"

class TransformSystem : public System<TransformComponent>
{
public:
    void OnUpdate(float _dt, EntityId _e, TransformComponent& _t) override;

private:
    void UpdateMatrix(TransformComponent& _t, EntityId _e);
};

#endif
