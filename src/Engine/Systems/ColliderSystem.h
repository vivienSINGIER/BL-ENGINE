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
	float cellSize;
	int numCellsX;
	int numCellsY;
	UnorderedMap<XMINT2, Vector<EntityId>> cells;
};

class ColliderSystem : public System<ColliderComponent,TransformComponent>
{
public:
	void OnUpdate(float _dt, ColliderComponent& _collider, TransformComponent& _transform) override;

private:
	void BroadPhase();
	void NarrowPhase();

	void BuildCandidatePairs();

	AABB CalculateWorldAABB(ColliderComponent& _collider, TransformComponent& _transform);
	void InsertIntoPartitionGrid(EntityId entity, const AABB& aabb);

	PartitionGrid m_partitionGrid;
	Vector<Contact> m_vContacts;
	Vector<std::pair<EntityId, EntityId>> m_candidatePairs;
};

#endif // !COLLIDER_SYSTEM_H_DEFINED