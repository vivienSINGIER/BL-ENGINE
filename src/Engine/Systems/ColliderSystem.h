#ifndef COLLIDER_SYSTEM_H_DEFINED
#define COLLIDER_SYSTEM_H_DEFINED

#include "../ECS/ISystem.hpp"
#include "../Components/ColliderComponent.hpp"
#include "../Components/TransformComponent.hpp"

struct AABB
{
	XMFLOAT3 min;
	XMFLOAT3 max;
};

struct Contact 
{
	EntityId a;
	EntityId b;
	XMFLOAT3 normal;
	float penetration;
};

struct PartitionGrid
{
	int cellSize;
	int numCellsX;
	int numCellsY;
	Vector<Vector<Vector<EntityId>>> cells; 
};

class ColliderSystem : public System<ColliderComponent, TransformComponent>
{
public:
	void OnStartUpdate(float _dt) override;
	void OnUpdate(float _dt, EntityId _e, ColliderComponent& _collider, TransformComponent& _transform) override;
	void OnEndUpdate(float _dt) override;

	void InitializePartitionGrid(XMINT2 _mapSize, int _cellSize);

private:
	void ClearPartitionGrid();
	void InsertIntoPartitionGrid(EntityId entity, const AABB& aabb);
	void BuildCandidatePairs();
	void NarrowPhase();

	AABB CalculateWorldAABB(ColliderComponent& _collider, TransformComponent& _transform);

	bool CheckBoxToBox(ColliderComponent& _boxA, TransformComponent& _transformA, ColliderComponent& _boxB, TransformComponent& _transformB);
	bool CheckSphereToSphere(ColliderComponent& _sphereA, TransformComponent& _transformA, ColliderComponent& _sphereB, TransformComponent& _transformB);
	bool CheckBoxToSphere(ColliderComponent& _box, TransformComponent& _transformBox, ColliderComponent& _sphere, TransformComponent& _transformSphere);
	
	PartitionGrid m_partitionGrid;
	Vector<Contact> m_vContacts;
	Vector<std::pair<EntityId, EntityId>> m_candidatePairs;
};

#endif // !COLLIDER_SYSTEM_H_DEFINED