#include "ColliderSystem.h"

void ColliderSystem::OnStartUpdate(float _dt)
{
	m_vContacts.clear();
	m_partitionGrid.cells.clear();
	m_candidatePairs.clear();
}

void ColliderSystem::OnUpdate(float _dt, EntityId _e, ColliderComponent& _collider, TransformComponent& _transform)
{
	AABB aabb = CalculateWorldAABB(_collider, _transform);
	InsertIntoPartitionGrid(_e, aabb);

}

void ColliderSystem::OnEndUpdate(float _dt)
{
}

AABB ColliderSystem::CalculateWorldAABB(ColliderComponent& _collider, TransformComponent& _transform)
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
			XMINT2 cellKey = { x, y };
			m_partitionGrid.cells[cellKey].push_back(entity);
		}
	}
}

void ColliderSystem::BuildCandidatePairs()
{
	for (auto& cell : m_partitionGrid.cells)
	{
		Vector<EntityId>& entities = cell.second;
		for (size_t i = 0; i < entities.size(); ++i)
		{
			for (size_t j = i + 1; j < entities.size(); ++j)
			{
				m_candidatePairs.emplace_back(entities[i], entities[j]);
			}
		}
	}
}
