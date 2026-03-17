#include "ColliderSystem.h"

void ColliderSystem::OnUpdate(float _dt, ColliderComponent& _collider, TransformComponent& _transform)
{
}

void ColliderSystem::BroadPhase()
{
	m_partitionGrid.cells.clear();
	for (Archetype* arch : query.matched)
	{
		for (uint64 i = 0; i < arch->storage.count; i++)
		{
			arch->storage.Get<TransformComponent>(ComponentRegistry::Id<TransformComponent>(), i);
		}
	}
}

void ColliderSystem::NarrowPhase()
{

}

void ColliderSystem::BuildCandidatePairs()
{

}

AABB ColliderSystem::CalculateWorldAABB(TransformComponent& _transform)
{
	XMFLOAT3 center = _transform.transform.GetWorldPosition();

	AABB aabb = { XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) };

	if (_collider.type == ColliderType::Box)
	{
		aabb.min = 
		{
			center.x - _collider.box.halfExtents.x,
			center.y - _collider.box.halfExtents.y,
			center.z - _collider.box.halfExtents.z
		};
		aabb.max = 
		{
			center.x + _collider.box.halfExtents.x,
			center.y + _collider.box.halfExtents.y,
			center.z + _collider.box.halfExtents.z
		};
	}
	else if (_collider.type == ColliderType::Sphere)
	{
		aabb.min = 
		{
			center.x - _collider.sphere.radius,
			center.y - _collider.sphere.radius,
			center.z - _collider.sphere.radius
		};
		aabb.max = 
		{
			center.x + _collider.sphere.radius,
			center.y + _collider.sphere.radius,
			center.z + _collider.sphere.radius
		};
	}

	return aabb;
}

void ColliderSystem::InsertIntoPartitionGrid(EntityId entity, const AABB& aabb)
{
	int cellXMin = static_cast<int>(aabb.min.x / m_partitionGrid.cellSize);
	int cellYMin = static_cast<int>(aabb.min.y / m_partitionGrid.cellSize);
	int cellXMax = static_cast<int>(aabb.max.x / m_partitionGrid.cellSize);
	int cellYMax = static_cast<int>(aabb.max.y / m_partitionGrid.cellSize);

	for (int x = cellXMin; x <= cellXMax; ++x)
	{
		for (int y = cellYMin; y <= cellYMax; ++y)
		{
			XMFLOAT2 cellKey = { static_cast<float>(x), static_cast<float>(y) };
			m_partitionGrid.cells[cellKey].push_back(entity);
		}
	}
}
