#include "ColliderSystem.h"
#include "Utils.hpp"

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
	BuildCandidatePairs();
	NarrowPhase();
}

void ColliderSystem::InitializePartitionGrid(XMINT2 _mapSize, int _cellSize)
{
	assert(_cellSize > 0 && "Cell size must be greater than zero.");
	assert(_mapSize.x > 0 && _mapSize.y > 0 && "Map size must be greater than zero.");
	assert(_mapSize.x % _cellSize == 0 && _mapSize.y % _cellSize == 0 && "Map size must be divisible by cell size.");

	m_partitionGrid.cellSize = _cellSize;
	m_partitionGrid.numCellsX = (_mapSize.x + _cellSize - 1) / _cellSize;
	m_partitionGrid.numCellsY = (_mapSize.y + _cellSize - 1) / _cellSize;
}

void ColliderSystem::ClearPartitionGrid()
{
	m_partitionGrid.cells.clear();
}

AABB ColliderSystem::CalculateWorldAABB(ColliderComponent& _collider, TransformComponent& _transform)
{
	XMFLOAT3 center = _transform.transform.GetWorldPosition();

	AABB aabb = { XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) };

	if (_collider.type == ColliderType::Box)
	{
		aabb.min = 
		{
			center.x - _collider.colliderTransform.GetLocalScale().x * 0.5f,
			center.y - _collider.colliderTransform.GetLocalScale().y * 0.5f,
			center.z - _collider.colliderTransform.GetLocalScale().z * 0.5f
		};
		aabb.max = 
		{
			center.x + _collider.colliderTransform.GetLocalScale().x * 0.5f,
			center.y + _collider.colliderTransform.GetLocalScale().y * 0.5f,
			center.z + _collider.colliderTransform.GetLocalScale().z * 0.5f
		};
	}
	else if (_collider.type == ColliderType::Sphere)
	{
		aabb.min = 
		{
			center.x - _collider.colliderTransform.GetLocalScale().x * 0.5f,
			center.y - _collider.colliderTransform.GetLocalScale().y * 0.5f,
			center.z - _collider.colliderTransform.GetLocalScale().z * 0.5f
		};
		aabb.max = 
		{
			center.x + _collider.colliderTransform.GetLocalScale().x * 0.5f,
			center.y + _collider.colliderTransform.GetLocalScale().y * 0.5f,
			center.z + _collider.colliderTransform.GetLocalScale().z * 0.5f
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
			m_partitionGrid.cells[x][y].push_back(entity);
		}
	}
}

void ColliderSystem::BuildCandidatePairs()
{
	for (int x = 0; x < m_partitionGrid.cells.size(); x++)
	{
		for (int y = 0; y < m_partitionGrid.cells[x].size(); y++)
		{
			Vector<EntityId>& entities = m_partitionGrid.cells[x][y];
			for (size_t i = 0; i < entities.size(); ++i)
			{
				for (size_t j = i + 1; j < entities.size(); ++j)
				{
					m_candidatePairs.emplace_back(entities[i], entities[j]);
				}
			}
		}
	}
}

void ColliderSystem::NarrowPhase()
{
	for (auto& pair : m_candidatePairs)
	{
		EntityId entityA = pair.first;
		EntityId entityB = pair.second;

		ColliderComponent& colliderA = world->GetComponent<ColliderComponent>(entityA);
		ColliderComponent& colliderB = world->GetComponent<ColliderComponent>(entityB);
		TransformComponent& transformA = world->GetComponent<TransformComponent>(entityA);
		TransformComponent& transformB = world->GetComponent<TransformComponent>(entityB);

		bool isColliding = false;

		if (colliderA.type == ColliderType::Box && colliderB.type == ColliderType::Box)
		{
			isColliding = CheckBoxToBox(colliderA, transformA, colliderB, transformB);
		}
		else if (colliderA.type == ColliderType::Sphere && colliderB.type == ColliderType::Sphere)
		{
			isColliding = CheckSphereToSphere(colliderA, transformA, colliderB, transformB);
		}
		else if ((colliderA.type == ColliderType::Box && colliderB.type == ColliderType::Sphere) ||
				 (colliderA.type == ColliderType::Sphere && colliderB.type == ColliderType::Box))
		{
			if (colliderA.type == ColliderType::Box)
				isColliding = CheckBoxToSphere(colliderA, transformA, colliderB, transformB);
			else
				isColliding = CheckBoxToSphere(colliderB, transformB, colliderA, transformA);
		}

		if (isColliding)
		{
			m_vContacts.push_back({ entityA, entityB });
		}
	}
}

bool ColliderSystem::CheckBoxToBox(ColliderComponent& _boxA, TransformComponent& _transformA, ColliderComponent& _boxB, TransformComponent& _transformB)
{
	XMFLOAT3 posA = _transformA.transform.GetWorldPosition();
	XMFLOAT3 posB = _transformB.transform.GetWorldPosition();
	XMFLOAT3 sizeA = _boxA.colliderTransform.GetWorldScale();
	XMFLOAT3 sizeB = _boxB.colliderTransform.GetWorldScale();

	if (posA.x + sizeA.x * 0.5f < posB.x - sizeB.x * 0.5f || posA.x - sizeA.x * 0.5f > posB.x + sizeB.x * 0.5f) return false;
	if (posA.y + sizeA.y * 0.5f < posB.y - sizeB.y * 0.5f || posA.y - sizeA.y * 0.5f > posB.y + sizeB.y * 0.5f) return false;
	if (posA.z + sizeA.z * 0.5f < posB.z - sizeB.z * 0.5f || posA.z - sizeA.z * 0.5f > posB.z + sizeB.z * 0.5f) return false;

	return true;
}

bool ColliderSystem::CheckSphereToSphere(ColliderComponent& _sphereA, TransformComponent& _transformA, ColliderComponent& _sphereB, TransformComponent& _transformB)
{
	XMFLOAT3 posA = _transformA.transform.GetWorldPosition();
	XMFLOAT3 posB = _transformB.transform.GetWorldPosition();
	float radiusA = _sphereA.colliderTransform.GetWorldScale().x * 0.5f;
	float radiusB = _sphereB.colliderTransform.GetWorldScale().x * 0.5f;

	float dx = posA.x - posB.x;
	float dy = posA.y - posB.y;
	float dz = posA.z - posB.z;

	float d = dx * dx + dy * dy + dz * dz;
	if (d > (radiusA + radiusB) * (radiusA + radiusB))
		return false;

	return true;
}

bool ColliderSystem::CheckBoxToSphere(ColliderComponent& _box, TransformComponent& _transformBox, ColliderComponent& _sphere, TransformComponent& _transformSphere)
{
	XMFLOAT3 boxPos = _transformBox.transform.GetWorldPosition();
	XMFLOAT3 boxSize = _box.colliderTransform.GetWorldScale();

	XMFLOAT3 spherePos = _transformSphere.transform.GetWorldPosition();
	float sphereRadius = _sphere.colliderTransform.GetWorldScale().x * 0.5f;

	float closestX = Max(boxPos.x - boxSize.x * 0.5f, Min(spherePos.x, boxPos.x + boxSize.x * 0.5f));
	float closestY = Max(boxPos.y - boxSize.y * 0.5f, Min(spherePos.y, boxPos.y + boxSize.y * 0.5f));
	float closestZ = Max(boxPos.z - boxSize.z * 0.5f, Min(spherePos.z, boxPos.z + boxSize.z * 0.5f));

	float dx = closestX - spherePos.x;
	float dy = closestY - spherePos.y;
	float dz = closestZ - spherePos.z;

	float d = dx * dx + dy * dy + dz * dz;
	if (d > sphereRadius * sphereRadius)
		return false;

	return true;
}
