#ifndef COLLIDER_SYSTEM_H_DEFINED
#define COLLIDER_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
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
    void InsertIntoPartitionGrid(EntityId _e, const ColliderComponent& _collider);
    void BuildCandidatePairs();
    void NarrowPhase();

    void UpdateCollider(ColliderComponent& _collider, TransformComponent& _transform);
    void CalculateWorldAABB(ColliderComponent& _collider, TransformComponent& _transform);

    void ResetContactHolder();
    void StoreCurrentContact(EntityId _a, EntityId _b);

    // Collision tests
    bool CheckOBBToOBB(ColliderComponent& _boxA, ColliderComponent& _boxB);
    bool CheckSphereToSphere(ColliderComponent& _sphereA, TransformComponent& _transformA, ColliderComponent& _sphereB, TransformComponent& _transformB);
    bool CheckBoxToSphere(ColliderComponent& _box, TransformComponent& _transformBox, ColliderComponent& _sphere, TransformComponent& _transformSphere);

    // SAT
    bool OverlapOnAxis(const OBB& _obbA, const OBB& _obbB, const XMFLOAT3& _axis, float& _minOverlap, int& _minAxisIndex, int _currAxisIndex);
    float OBBRadius(const OBB& _obb, const XMFLOAT3& _axis);
    XMFLOAT3 OBBSupportPoint(const OBB& _obb, const XMFLOAT3& _dir);

    // Focntions d'aides
    bool PointInOBB(const XMFLOAT3& _point, const OBB& _obb);
    void GetOBBCorners(const OBB& _obb, XMFLOAT3 _outCorners[8]);
    void BuildOBBContactPoints(const OBB& _boxA, const OBB& _boxB);
    XMFLOAT3 AveragePoints(XMFLOAT3* _points, int _count);

	// Fonctions pour le clipping des faces d'OBB
    void GetFaceVertices(const OBB& _obb, int _axisIndex, bool _positiveFace, XMFLOAT3 _outVertices[4]);
    int ClipPolygonAgainstPlane(const XMFLOAT3* _input, int _inputCount, XMFLOAT3* _output, const XMFLOAT3& _planePoint, const XMFLOAT3& _planeNormal);
    bool BuildOBBContactManifoldByClipping(const OBB& _boxA, const OBB& _boxB, const XMFLOAT3& _normal, int _minAxisIndex);

    unsigned long long MakePairKey(EntityId _a, EntityId _b) const;

private:
    PartitionGrid m_partitionGrid;
    Vector<std::pair<EntityId, EntityId>> m_candidatePairs;
    ContactManager* m_pContactManager = nullptr;
    Contact m_contactHolder;
};

#endif // !COLLIDER_SYSTEM_H_DEFINED