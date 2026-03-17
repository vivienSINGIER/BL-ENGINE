#ifndef TRANSFORM_SYSTEM_H_DEFINED
#define TRANSFORM_SYSTEM_H_DEFINED

#include "../ECS/ISystem.hpp"
#include "../Components/TransformComponent.hpp"

class TransformSystem : public System<TransformComponent>
{
public:
    void OnUpdate(float _dt, EntityId _e, TransformComponent& _t) override;

private:
    bool IsDirty(Transform& _transform, DIRTY_FLAG _flag);
};

#endif
