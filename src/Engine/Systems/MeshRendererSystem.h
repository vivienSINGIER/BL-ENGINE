#ifndef MESH_RENDERER_SYSTEM_H_DEFINED
#define MESH_RENDERER_SYSTEM_H_DEFINED

#include "../Components/TransformComponent.hpp"
#include "../ECS/ISystem.hpp"

struct MeshRenderer;

struct MeshRendererSystem : System<MeshRenderer, TransformComponent>
{
    void OnUpdate(float _dt, MeshRenderer& _mesh, TransformComponent& _transform) override;
};

#endif
