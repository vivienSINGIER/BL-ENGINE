#include "ColliderSystem.h"

void ColliderSystem::OnUpdate(float _dt, ColliderComponent& _collider, TransformComponent& _transform)
{

}

void ColliderSystem::BuildBSP(std::vector<Segment3D>& _segments, int _depth)
{

}

bool ColliderSystem::CheckBoxToBox(ColliderComponent& _boxA, TransformComponent& _transformA, ColliderComponent& _boxB, TransformComponent& _transformB)
{
	if (_boxA.type != ColliderType::Box || _boxB.type != ColliderType::Box) return false;

	XMFLOAT3 posA = _transformA.transform.GetWorldPosition();
	XMFLOAT3 posB = _transformB.transform.GetWorldPosition();
	XMFLOAT3 halfA = _boxA.box.halfExtents;
	XMFLOAT3 halfB = _boxB.box.halfExtents;

	if (posA.x + halfA.x < posB.x - halfB.x || posA.x - halfA.x > posB.x + halfB.x) return false; 
	if (posA.y + halfA.y < posB.y - halfB.y || posA.y - halfA.y > posB.y + halfB.y) return false;
	if (posA.z + halfA.z < posB.z - halfB.z || posA.z - halfA.z > posB.z + halfB.z) return false;

	return true;
}

bool ColliderSystem::CheckSphereToSphere(ColliderComponent& _sphereA, TransformComponent& _transformA, ColliderComponent& _sphereB, TransformComponent& _transformB)
{
	if (_sphereA.type != ColliderType::Sphere || _sphereB.type != ColliderType::Sphere) return false;

	XMFLOAT3 posA = _transformA.transform.GetWorldPosition();
	XMFLOAT3 posB = _transformB.transform.GetWorldPosition();
	float radiusA = _sphereA.sphere.radius;
	float radiusB = _sphereB.sphere.radius;

	float dx = posA.x - posB.x;
	float dy = posA.y - posB.y;
	float dz = posA.z - posB.z;

	float d = dx * dx + dy * dy + dz * dz;
	if (d > (radiusA + radiusB) * (radiusA + radiusB)) return false;

	return true;
}
