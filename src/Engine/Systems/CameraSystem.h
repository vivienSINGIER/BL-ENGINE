#ifndef CAMERA_SYSTEM_H_DEFINED
#define CAMERA_SYSTEM_H_DEFINED

#include "../ECS/ISystem.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/TransformComponent.hpp"

struct CameraSystem : public System<CameraComponent, TransformComponent>
{
    void OnUpdate(float _dt, CameraComponent& _camera, TransformComponent& _transform) override;
};

#endif
