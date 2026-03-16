#ifndef COLLIDER_SYSTEM_H_DEFINED
#define COLLIDER_SYSTEM_H_DEFINED

#include "../ECS/ISystem.hpp"
#include "../Components/ColliderComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "Utils.hpp"

struct BSP
{
    Axis split;
    BSP* childs[2];
    std::vector<Segment3D> segments;
    int nbSegments;
};

class ColliderSystem : public System<ColliderComponent,TransformComponent>
{
public:
	void OnUpdate(float _dt, ColliderComponent& _collider, TransformComponent& _transform) override;

private:
	void BuildBSP(std::vector<Segment3D>& _segments, int _depth);

	bool CheckBoxToBox(ColliderComponent& _boxA, TransformComponent& _transformA, ColliderComponent& _boxB, TransformComponent& _transformB);
	bool CheckSphereToSphere(ColliderComponent& _sphereA, TransformComponent& _transformA, ColliderComponent& _sphereB, TransformComponent& _transformB);

	BSP m_bsp;
};

#endif // !COLLIDER_SYSTEM_H_DEFINED