#ifndef COLLIDER_SYSTEM_H_DEFINED
#define COLLIDER_SYSTEM_H_DEFINED

#include "../ECS/ISystem.hpp"
#include "../Components/ColliderComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "../ContactManager.hpp"

struct PartitionGrid
{
	int cellSize = 0;
	int numCellsX = 0;
	int numCellsY = 0;
	Vector<Vector<Vector<EntityId>>> cells; 
};

class ColliderSystem : public System<ColliderComponent, TransformComponent>
{
public:
	void OnStartUpdate(float _dt) override;
	void OnUpdate(float _dt, EntityId _e, ColliderComponent& _collider, TransformComponent& _transform) override;
	void OnEndUpdate(float _dt) override;

	void InitializePartitionGrid(XMINT2 _mapSize, int _cellSize);
	void SetContactManager(ContactManager* _contactManager) { m_pContactManager = _contactManager; }

private:
	void ClearPartitionGrid();
	void InsertIntoPartitionGrid(EntityId _e, ColliderComponent& _collider);
	void BuildCandidatePairs();
	void NarrowPhase();

	void UpdateCollider(ColliderComponent& _collider, TransformComponent& _transform);
	void CalculateWorldAABB(ColliderComponent& _collider, TransformComponent& _transform);
	void ResetContactHolder();

	bool CheckOBBToOBB(ColliderComponent& _boxA, ColliderComponent& _boxB);
	bool CheckSphereToSphere(ColliderComponent& _sphereA, TransformComponent& _transformA, ColliderComponent& _sphereB, TransformComponent& _transformB);
	bool CheckBoxToSphere(ColliderComponent& _box, TransformComponent& _transformBox, ColliderComponent& _sphere, TransformComponent& _transformSphere);

	XMFLOAT3 OBBSupportPoint(OBB& _obb, XMFLOAT3& _dir);
	float OBBRadius(OBB& _obb, XMFLOAT3& _axis);
	bool PointInOBB(XMFLOAT3& point, OBB& _obb);
	void GetOBBCorners(OBB& _obb, XMFLOAT3 _outCorners[8]);
	void BuildOBBContactPoints(OBB& _boxA, OBB& _boxB);
	XMFLOAT3 AveragePoints(XMFLOAT3* points, int count);
	bool OverlapOnAxis(OBB& _obbA, OBB& _obbB, XMFLOAT3& _axis, float& _minDistance, int& _minAxeIndex, int _currAxeIndex);

	PartitionGrid m_partitionGrid;
	Vector<std::pair<EntityId, EntityId>> m_candidatePairs;
	ContactManager* m_pContactManager = nullptr;

	Contact m_contactHolder;
};

#endif // !COLLIDER_SYSTEM_H_DEFINED