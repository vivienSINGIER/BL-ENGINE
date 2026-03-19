#include "ColliderSystem.h"
#include "Utils.hpp"
#include <iostream>

void ColliderSystem::OnStartUpdate(float _dt)
{
	ClearPartitionGrid();
	m_candidatePairs.clear();
}

void ColliderSystem::OnUpdate(float _dt, EntityId _e, ColliderComponent& _collider, TransformComponent& _transform)
{
	_collider.contact.other = -1; //Valeur par defaut
	_collider.contact.penetration = 0.0f;
	_collider.contact.normal = { 0.0f, 0.0f, 0.0f };
	CalculateWorldAABB(_collider, _transform);
	InsertIntoPartitionGrid(_e, _collider);
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

	for (int x = 0; x < m_partitionGrid.numCellsX; x++)
	{
		Vector<Vector<EntityId>> vvEntitiesTemp;
		for (int y = 0; y < m_partitionGrid.numCellsY; y++)
		{
			Vector<EntityId> vEntitiesTemp;
			vvEntitiesTemp.push_back(vEntitiesTemp);
		}
		m_partitionGrid.cells.push_back(vvEntitiesTemp);
	}
}

void ColliderSystem::ClearPartitionGrid()
{
	for (int x = 0; x < m_partitionGrid.numCellsX; x++)
	{
		for (int y = 0; y < m_partitionGrid.numCellsY; y++)
		{
			m_partitionGrid.cells[x][y].clear();
		}
	}
}

void ColliderSystem::UpdateCollider(ColliderComponent& _collider, TransformComponent& _transform)
{
	_collider.colliderTransform.SetPosition(_transform.world.GetPosition());
	_collider.colliderTransform.SetRotationQuaternion(_transform.world.GetRotation());

	XMFLOAT3 pos = _transform.world.GetPosition();
	XMFLOAT3 scale = Mul(_collider.colliderTransform.GetScale(), _transform.world.GetScale());

	_collider.obb.center = pos;
	_collider.obb.halfExtents = { scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f };

	XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(&_transform.world.GetRotation()));

	XMVECTOR axisX = XMVector3TransformNormal(XMVectorSet(1, 0, 0, 0), rot);
	XMVECTOR axisY = XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), rot);
	XMVECTOR axisZ = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), rot);

	XMStoreFloat3(&_collider.obb.axes[0], XMVector3Normalize(axisX));
	XMStoreFloat3(&_collider.obb.axes[1], XMVector3Normalize(axisY));
	XMStoreFloat3(&_collider.obb.axes[2], XMVector3Normalize(axisZ));
}

void ColliderSystem::CalculateWorldAABB(ColliderComponent& _collider, TransformComponent& _transform)
{
	UpdateCollider(_collider, _transform);

	if (_collider.type == ColliderType::Sphere)
	{
		const XMFLOAT3 c = _transform.world.GetPosition();
		const float r = _collider.colliderTransform.GetScale().x * 0.5f;

		_collider.aabb.min = { c.x - r, c.y - r, c.z - r };
		_collider.aabb.max = { c.x + r, c.y + r, c.z + r };
		return;
	}

	OBB& obb = _collider.obb;

	XMFLOAT3 ex = Mul(obb.axes[0], obb.halfExtents.x);
	XMFLOAT3 ey = Mul(obb.axes[1], obb.halfExtents.y);
	XMFLOAT3 ez = Mul(obb.axes[2], obb.halfExtents.z);

	float wx = abs(ex.x) + abs(ey.x) + abs(ez.x);
	float wy = abs(ex.y) + abs(ey.y) + abs(ez.y);
	float wz = abs(ex.z) + abs(ey.z) + abs(ez.z);

	_collider.aabb.min = { obb.center.x - wx, obb.center.y - wy, obb.center.z - wz };
	_collider.aabb.max = { obb.center.x + wx, obb.center.y + wy, obb.center.z + wz };
}

float ColliderSystem::OBBRadius(OBB& obb, XMFLOAT3& axis)
{
	float result = 0.0f;
	result += obb.halfExtents.x * abs(Dot(axis, obb.axes[0]));
	result += obb.halfExtents.y * abs(Dot(axis, obb.axes[1]));
	result += obb.halfExtents.z * abs(Dot(axis, obb.axes[2]));

	return result;
}

void ColliderSystem::InsertIntoPartitionGrid(EntityId entity, ColliderComponent& _colliderComponent)
{
	int cellXMin = static_cast<int>(floorf(_colliderComponent.aabb.min.x / m_partitionGrid.cellSize));
	int cellYMin = static_cast<int>(floorf(_colliderComponent.aabb.min.y / m_partitionGrid.cellSize));
	int cellXMax = static_cast<int>(floorf(_colliderComponent.aabb.max.x / m_partitionGrid.cellSize));
	int cellYMax = static_cast<int>(floorf(_colliderComponent.aabb.max.y / m_partitionGrid.cellSize));

	cellXMin = Clamp(cellXMin, 0, m_partitionGrid.numCellsX - 1);
	cellYMin = Clamp(cellYMin, 0, m_partitionGrid.numCellsY - 1);
	cellXMax = Clamp(cellXMax, 0, m_partitionGrid.numCellsX - 1);
	cellYMax = Clamp(cellYMax, 0, m_partitionGrid.numCellsY - 1);

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
			isColliding = CheckOBBToOBB(colliderA, colliderB);
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
			colliderA.contact.other = entityB;
			colliderB.contact.other = entityA;
			std::cout << "Collision" << std::endl;
		}
	}
}

bool ColliderSystem::OverlapOnAxis(OBB& a, OBB& b, XMFLOAT3& axis, float& _minDistance, int& _minAxeIndex, int _currAxeIndex)
{
	if (Dot(axis, axis) < 1e-6f)
		return true;

	XMFLOAT3 n = Normalize(axis);
	XMFLOAT3 centerDelta = Subtract(b.center, a.center);

	float distance = abs(Dot(centerDelta, n));
	float ra = OBBRadius(a, n);
	float rb = OBBRadius(b, n);

	if (_minDistance > distance && distance > 0.0f)
	{
		_minDistance = ra + rb - distance;
		_minAxeIndex = _currAxeIndex;
	}

	return distance <= (ra + rb);
}

bool ColliderSystem::CheckOBBToOBB(ColliderComponent& _boxA, ColliderComponent& _boxB)
{
	//Test SAT
	XMFLOAT3 axes[15] =
	{
		_boxA.obb.axes[0], _boxA.obb.axes[1], _boxA.obb.axes[2],
		_boxB.obb.axes[0], _boxB.obb.axes[1], _boxB.obb.axes[2],

		Cross(_boxA.obb.axes[0], _boxB.obb.axes[0]),
		Cross(_boxA.obb.axes[0], _boxB.obb.axes[1]),
		Cross(_boxA.obb.axes[0], _boxB.obb.axes[2]),

		Cross(_boxA.obb.axes[1], _boxB.obb.axes[0]),
		Cross(_boxA.obb.axes[1], _boxB.obb.axes[1]),
		Cross(_boxA.obb.axes[1], _boxB.obb.axes[2]),

		Cross(_boxA.obb.axes[2], _boxB.obb.axes[0]),
		Cross(_boxA.obb.axes[2], _boxB.obb.axes[1]),
		Cross(_boxA.obb.axes[2], _boxB.obb.axes[2])
	};

	//avec SAT, garder l’axe de pénétration minimale
	float minDistance = FLT_MAX;
	int minAxeIndex = -1;

	for (int i = 0; i < 15; ++i)
	{
		if (OverlapOnAxis(_boxA.obb, _boxB.obb, axes[i], minDistance, minAxeIndex, i) == false)
			return false;
	}

	if (_boxA.isTrigger || _boxB.isTrigger) //Pas de calcul de contact si trigger
		return true;

	XMFLOAT3 normal = Normalize(axes[minAxeIndex]);
	XMFLOAT3 centerDelta = Subtract(_boxB.obb.center, _boxA.obb.center);

	if (Dot(centerDelta, normal) < 0.0f)
		normal = Inverse(normal);

	_boxA.contact.normal = Inverse(normal);
	_boxB.contact.normal = normal;

	_boxA.contact.penetration = minDistance;
	_boxB.contact.penetration = minDistance;

	return true;
}

bool ColliderSystem::CheckSphereToSphere(ColliderComponent& _sphereA, TransformComponent& _transformA, ColliderComponent& _sphereB, TransformComponent& _transformB)
{
	XMFLOAT3 posA = _transformA.world.GetPosition();
	XMFLOAT3 posB = _transformB.world.GetPosition();
	float radiusA = _sphereA.colliderTransform.GetScale().x * _transformA.world.GetScale().x * 0.5f;
	float radiusB = _sphereB.colliderTransform.GetScale().x * _transformB.world.GetScale().x * 0.5f;

	float dx = posA.x - posB.x;
	float dy = posA.y - posB.y;
	float dz = posA.z - posB.z;

	float d2 = dx * dx + dy * dy + dz * dz;
	if (d2 > (radiusA + radiusB) * (radiusA + radiusB))
		return false;

	if (_sphereA.isTrigger || _sphereB.isTrigger) //Pas de calcul de contact si trigger
		return true;

	if (d2 < 1e-6f)
	{
		//Normale arbitraire si les centres sont presque au même endroit
		_sphereA.contact.normal = { 0, 1, 0 }; 
		_sphereB.contact.normal = { 0, -1, 0 };

		float penetration = radiusA + radiusB;
		_sphereA.contact.penetration = penetration;
		_sphereB.contact.penetration = penetration;
	}
	else
	{
		XMFLOAT3 normal = Normalize(Subtract(posB, posA));
		_sphereA.contact.normal = Inverse(normal);
		_sphereB.contact.normal = normal;

		float penetration = radiusA + radiusB - sqrt(d2);
		_sphereA.contact.penetration = penetration;
		_sphereB.contact.penetration = penetration;
	}
	return true;
}

bool ColliderSystem::CheckBoxToSphere(ColliderComponent& _box, TransformComponent& _transformBox, ColliderComponent& _sphere, TransformComponent& _transformSphere)
{
	OBB& obb = _box.obb;
	XMFLOAT3 spherePosition = _transformSphere.world.GetPosition();
	float sphereRadius = _sphere.colliderTransform.GetScale().x * 0.5f;

	XMFLOAT3 d = Subtract(spherePosition, obb.center);
	XMFLOAT3 closest = obb.center;

	float distX = Dot(d, obb.axes[0]);
	float distY = Dot(d, obb.axes[1]);
	float distZ = Dot(d, obb.axes[2]);

	distX = Clamp(distX, -obb.halfExtents.x, obb.halfExtents.x);
	distY = Clamp(distY, -obb.halfExtents.y, obb.halfExtents.y);
	distZ = Clamp(distZ, -obb.halfExtents.z, obb.halfExtents.z);

	XMFLOAT3 temp = Mul(obb.axes[0], distX);
	XMFLOAT3 temp1 = Mul(obb.axes[1], distY);
	XMFLOAT3 temp2 = Mul(obb.axes[2], distZ);

	closest = Add(closest, temp);
	closest = Add(closest, temp1);
	closest = Add(closest, temp2);

	XMFLOAT3 delta = Subtract(spherePosition, closest);
	float d2 = Dot(delta, delta);

	if (d2 > sphereRadius * sphereRadius)
		return false;

	if (_box.isTrigger || _sphere.isTrigger) //Pas de calcul de contact si trigger
		return true;

	//normale = sphère - point proche
	XMFLOAT3 normal = Normalize(Subtract(spherePosition, closest));
	_box.contact.normal = Inverse(normal);
	_sphere.contact.normal = normal;

	//pénétration = rayon - distance
	float penetration = sphereRadius - sqrt(d2);
	_box.contact.penetration = penetration;
	_sphere.contact.penetration = penetration;

	return true;
}
