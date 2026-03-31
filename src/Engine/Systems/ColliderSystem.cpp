#include "ColliderSystem.h"
#include "Utils.hpp"
#include <unordered_set>
#include <cfloat>
#include "../ECS/World.h"

void ColliderSystem::OnStartUpdate(float _dt)
{
    ClearPartitionGrid();
    m_candidatePairs.clear();

    if (m_pContactManager != nullptr)
        m_pContactManager->Clear();
}

void ColliderSystem::OnUpdate(float _dt, EntityId _e, ColliderComponent& _collider, TransformComponent& _transform)
{
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

    m_partitionGrid.cells.clear();
    m_partitionGrid.cells.reserve(m_partitionGrid.numCellsX);

    for (int x = 0; x < m_partitionGrid.numCellsX; ++x)
    {
        Vector<Vector<EntityId>> column;
        column.reserve(m_partitionGrid.numCellsY);

        for (int y = 0; y < m_partitionGrid.numCellsY; ++y)
            column.push_back(Vector<EntityId>());

        m_partitionGrid.cells.push_back(column);
    }
}

void ColliderSystem::ClearPartitionGrid()
{
    for (int x = 0; x < m_partitionGrid.numCellsX; ++x)
    {
        for (int y = 0; y < m_partitionGrid.numCellsY; ++y)
            m_partitionGrid.cells[x][y].clear();
    }
}

void ColliderSystem::UpdateCollider(ColliderComponent& _collider, TransformComponent& _transform)
{
    _collider.colliderTransform.SetPosition(_transform.local.GetPosition());
    _collider.colliderTransform.SetRotationQuaternion(_transform.local.GetRotation());

    XMFLOAT3 pos = _transform.local.GetPosition();
    XMFLOAT3 scale = Mul(_collider.colliderTransform.GetScale(), _transform.local.GetScale());

    _collider.obb.center = pos;
    _collider.obb.halfExtents = { scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f };

    XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(&_transform.local.GetRotation()));

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
        const XMFLOAT3 c = _transform.local.GetPosition();
        const float r = _collider.colliderTransform.GetScale().x * 0.5f;

        _collider.aabb.min = { c.x - r, c.y - r, c.z - r };
        _collider.aabb.max = { c.x + r, c.y + r, c.z + r };
        return;
    }

    const OBB& obb = _collider.obb;

    XMFLOAT3 ex = Mul(obb.axes[0], obb.halfExtents.x);
    XMFLOAT3 ey = Mul(obb.axes[1], obb.halfExtents.y);
    XMFLOAT3 ez = Mul(obb.axes[2], obb.halfExtents.z);

    float wx = abs(ex.x) + abs(ey.x) + abs(ez.x);
    float wy = abs(ex.y) + abs(ey.y) + abs(ez.y);
    float wz = abs(ex.z) + abs(ey.z) + abs(ez.z);

    _collider.aabb.min = { obb.center.x - wx, obb.center.y - wy, obb.center.z - wz };
    _collider.aabb.max = { obb.center.x + wx, obb.center.y + wy, obb.center.z + wz };
}

void ColliderSystem::ResetContactHolder()
{
    m_contactHolder.a = -1;
    m_contactHolder.b = -1;
    m_contactHolder.normal = { 0.0f, 0.0f, 0.0f };
    m_contactHolder.penetration = 0.0f;
    m_contactHolder.pointCount = 0;
}

void ColliderSystem::StoreCurrentContact(EntityId _a, EntityId _b)
{
    m_contactHolder.a = _a;
    m_contactHolder.b = _b;

    if (m_pContactManager != nullptr)
        m_pContactManager->AddContact(m_contactHolder);
}

unsigned long long ColliderSystem::MakePairKey(EntityId _a, EntityId _b) const
{
    if (_a > _b)
        std::swap(_a, _b);

    return (static_cast<unsigned long long>(static_cast<unsigned int>(_a)) << 32) |
        static_cast<unsigned int>(_b);
}

void ColliderSystem::InsertIntoPartitionGrid(EntityId _e, const ColliderComponent& _collider)
{
    int cellXMin = static_cast<int>(floorf(_collider.aabb.min.x / m_partitionGrid.cellSize));
    int cellYMin = static_cast<int>(floorf(_collider.aabb.min.y / m_partitionGrid.cellSize));
    int cellXMax = static_cast<int>(floorf(_collider.aabb.max.x / m_partitionGrid.cellSize));
    int cellYMax = static_cast<int>(floorf(_collider.aabb.max.y / m_partitionGrid.cellSize));

    cellXMin = Clamp(cellXMin, 0, m_partitionGrid.numCellsX - 1);
    cellYMin = Clamp(cellYMin, 0, m_partitionGrid.numCellsY - 1);
    cellXMax = Clamp(cellXMax, 0, m_partitionGrid.numCellsX - 1);
    cellYMax = Clamp(cellYMax, 0, m_partitionGrid.numCellsY - 1);

    for (int x = cellXMin; x <= cellXMax; ++x)
    {
        for (int y = cellYMin; y <= cellYMax; ++y)
            m_partitionGrid.cells[x][y].push_back(_e);
    }
}

void ColliderSystem::BuildCandidatePairs()
{
    std::unordered_set<unsigned long long> uniquePairs;

    for (int x = 0; x < m_partitionGrid.cells.size(); ++x)
    {
        for (int y = 0; y < m_partitionGrid.cells[x].size(); ++y)
        {
            Vector<EntityId>& entities = m_partitionGrid.cells[x][y];

            for (size_t i = 0; i < entities.size(); ++i)
            {
                for (size_t j = i + 1; j < entities.size(); ++j)
                {
                    EntityId a = entities[i];
                    EntityId b = entities[j];

                    unsigned long long key = MakePairKey(a, b);
                    if (uniquePairs.insert(key).second)
                        m_candidatePairs.emplace_back(a, b);
                }
            }
        }
    }
}

void ColliderSystem::NarrowPhase()
{
    for (const auto& pair : m_candidatePairs)
    {
        ResetContactHolder();

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
        else if (colliderA.type == ColliderType::Box && colliderB.type == ColliderType::Sphere)
        {
            isColliding = CheckBoxToSphere(colliderA, transformA, colliderB, transformB);
        }
        else if (colliderA.type == ColliderType::Sphere && colliderB.type == ColliderType::Box)
        {
            isColliding = CheckBoxToSphere(colliderB, transformB, colliderA, transformA);

            if (isColliding)
                m_contactHolder.normal = Inverse(m_contactHolder.normal);
        }

        if (isColliding)
            StoreCurrentContact(entityA, entityB);
    }
}

float ColliderSystem::OBBRadius(const OBB& _obb, const XMFLOAT3& _axis)
{
    float result = 0.0f;
    result += _obb.halfExtents.x * abs(Dot(_axis, _obb.axes[0]));
    result += _obb.halfExtents.y * abs(Dot(_axis, _obb.axes[1]));
    result += _obb.halfExtents.z * abs(Dot(_axis, _obb.axes[2]));
    return result;
}

XMFLOAT3 ColliderSystem::OBBSupportPoint(const OBB& _obb, const XMFLOAT3& _dir)
{
    XMFLOAT3 result = _obb.center;

    float sx = (Dot(_dir, _obb.axes[0]) >= 0.0f) ? _obb.halfExtents.x : -_obb.halfExtents.x;
    float sy = (Dot(_dir, _obb.axes[1]) >= 0.0f) ? _obb.halfExtents.y : -_obb.halfExtents.y;
    float sz = (Dot(_dir, _obb.axes[2]) >= 0.0f) ? _obb.halfExtents.z : -_obb.halfExtents.z;

    result = Add(result, Mul(_obb.axes[0], sx));
    result = Add(result, Mul(_obb.axes[1], sy));
    result = Add(result, Mul(_obb.axes[2], sz));

    return result;
}

bool ColliderSystem::PointInOBB(const XMFLOAT3& _point, const OBB& _obb)
{
    XMFLOAT3 d = Subtract(_point, _obb.center);

    float x = Dot(d, _obb.axes[0]);
    float y = Dot(d, _obb.axes[1]);
    float z = Dot(d, _obb.axes[2]);

    float eps = 1e-6f;

    return abs(x) <= _obb.halfExtents.x + eps &&
        abs(y) <= _obb.halfExtents.y + eps &&
        abs(z) <= _obb.halfExtents.z + eps;
}

void ColliderSystem::GetOBBCorners(const OBB& _obb, XMFLOAT3 _outCorners[8])
{
    XMFLOAT3 ex = Mul(_obb.axes[0], _obb.halfExtents.x);
    XMFLOAT3 ey = Mul(_obb.axes[1], _obb.halfExtents.y);
    XMFLOAT3 ez = Mul(_obb.axes[2], _obb.halfExtents.z);

    int index = 0;

    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int sy = -1; sy <= 1; sy += 2)
        {
            for (int sz = -1; sz <= 1; sz += 2)
            {
                XMFLOAT3 p = _obb.center;
                p = Add(p, Mul(ex, (float)sx));
                p = Add(p, Mul(ey, (float)sy));
                p = Add(p, Mul(ez, (float)sz));
                _outCorners[index++] = p;
            }
        }
    }
}

void ColliderSystem::BuildOBBContactPoints(const OBB& _boxA, const OBB& _boxB)
{
    XMFLOAT3 cornersA[8];
    XMFLOAT3 cornersB[8];

    GetOBBCorners(_boxA, cornersA);
    GetOBBCorners(_boxB, cornersB);

    for (int i = 0; i < 8 && m_contactHolder.pointCount < 4; ++i)
    {
        if (PointInOBB(cornersA[i], _boxB))
            m_contactHolder.points[m_contactHolder.pointCount++].position = cornersA[i];
    }

    for (int i = 0; i < 8 && m_contactHolder.pointCount < 4; ++i)
    {
        if (PointInOBB(cornersB[i], _boxA))
            m_contactHolder.points[m_contactHolder.pointCount++].position = cornersB[i];
    }
}

XMFLOAT3 ColliderSystem::AveragePoints(XMFLOAT3* _points, int _count)
{
    XMFLOAT3 result = { 0.0f, 0.0f, 0.0f };

    if (_count <= 0)
        return result;

    for (int i = 0; i < _count; ++i)
        result = Add(result, _points[i]);

    return Mul(result, 1.0f / (float)_count);
}

bool ColliderSystem::OverlapOnAxis(const OBB& _obbA, const OBB& _obbB, const XMFLOAT3& _axis,
    float& _minOverlap, int& _minAxisIndex, int _currAxisIndex)
{
    if (Dot(_axis, _axis) < 1e-6f)
        return true;

    XMFLOAT3 n = Normalize(_axis);
    XMFLOAT3 centerDelta = Subtract(_obbB.center, _obbA.center);

    float distance = abs(Dot(centerDelta, n));
    float ra = OBBRadius(_obbA, n);
    float rb = OBBRadius(_obbB, n);

    float overlap = (ra + rb) - distance;

    if (overlap < 0.0f)
        return false;

    if (overlap < _minOverlap)
    {
        _minOverlap = overlap;
        _minAxisIndex = _currAxisIndex;
    }

    return true;
}

void ColliderSystem::GetFaceVertices(const OBB& _obb, int _axisIndex, bool _positiveFace, XMFLOAT3 _outVertices[4])
{
    const XMFLOAT3& faceAxis = _obb.axes[_axisIndex];

    int axis1 = (_axisIndex + 1) % 3;
    int axis2 = (_axisIndex + 2) % 3;

    float sign = _positiveFace ? 1.0f : -1.0f;

    float mainExtent = (_axisIndex == 0) ? _obb.halfExtents.x :
        (_axisIndex == 1) ? _obb.halfExtents.y :
        _obb.halfExtents.z;

    float extent1 = (axis1 == 0) ? _obb.halfExtents.x :
        (axis1 == 1) ? _obb.halfExtents.y :
        _obb.halfExtents.z;

    float extent2 = (axis2 == 0) ? _obb.halfExtents.x :
        (axis2 == 1) ? _obb.halfExtents.y :
        _obb.halfExtents.z;

    XMFLOAT3 center = Add(_obb.center, Mul(faceAxis, sign * mainExtent));
    XMFLOAT3 a1 = _obb.axes[axis1];
    XMFLOAT3 a2 = _obb.axes[axis2];

    _outVertices[0] = Add(Add(center, Mul(a1, extent1)), Mul(a2, extent2));
    _outVertices[1] = Add(Add(center, Mul(a1, -extent1)), Mul(a2, extent2));
    _outVertices[2] = Add(Add(center, Mul(a1, -extent1)), Mul(a2, -extent2));
    _outVertices[3] = Add(Add(center, Mul(a1, extent1)), Mul(a2, -extent2));
}

int ColliderSystem::ClipPolygonAgainstPlane(const XMFLOAT3* _input, int _inputCount,
    XMFLOAT3* _output,
    const XMFLOAT3& _planePoint,
    const XMFLOAT3& _planeNormal)
{
    if (_inputCount <= 0)
        return 0;

    int outputCount = 0;

    for (int i = 0; i < _inputCount; ++i)
    {
        const XMFLOAT3& current = _input[i];
        const XMFLOAT3& previous = _input[(i + _inputCount - 1) % _inputCount];

        float distCurrent = Dot(Subtract(current, _planePoint), _planeNormal);
        float distPrevious = Dot(Subtract(previous, _planePoint), _planeNormal);

        bool insideCurrent = distCurrent <= 0.0f;
        bool insidePrevious = distPrevious <= 0.0f;

        if (insideCurrent != insidePrevious)
        {
            XMFLOAT3 edge = Subtract(current, previous);
            float denom = Dot(edge, _planeNormal);

            if (abs(denom) > 1e-6f)
            {
                float t = -distPrevious / denom;
                _output[outputCount++] = Add(previous, Mul(edge, t));
            }
        }

        if (insideCurrent)
            _output[outputCount++] = current;
    }

    return outputCount;
}

bool ColliderSystem::BuildOBBContactManifoldByClipping(const OBB& _boxA, const OBB& _boxB, const XMFLOAT3& _normal, int _minAxisIndex)
{
    if (_minAxisIndex < 0 || _minAxisIndex > 5)
        return false;

    const OBB* referenceBox = nullptr;
    const OBB* incidentBox = nullptr;
    XMFLOAT3 referenceNormal = _normal;
    int referenceAxis = -1;

    if (_minAxisIndex < 3)
    {
        referenceBox = &_boxA;
        incidentBox = &_boxB;
        referenceAxis = _minAxisIndex;
    }
    else
    {
        referenceBox = &_boxB;
        incidentBox = &_boxA;
        referenceAxis = _minAxisIndex - 3;
        referenceNormal = Inverse(_normal);
    }

    bool referencePositiveFace = Dot(referenceNormal, referenceBox->axes[referenceAxis]) >= 0.0f;

    XMFLOAT3 referenceFace[4];
    GetFaceVertices(*referenceBox, referenceAxis, referencePositiveFace, referenceFace);

    int incidentAxis = 0;
    float minDot = FLT_MAX;

    for (int i = 0; i < 3; ++i)
    {
        float d0 = Dot(referenceNormal, incidentBox->axes[i]);
        float d1 = Dot(referenceNormal, Inverse(incidentBox->axes[i]));

        if (d0 < minDot)
        {
            minDot = d0;
            incidentAxis = i;
        }
        if (d1 < minDot)
        {
            minDot = d1;
            incidentAxis = i;
        }
    }

    bool incidentPositiveFace = Dot(referenceNormal, incidentBox->axes[incidentAxis]) < 0.0f;

    XMFLOAT3 incidentFace[4];
    GetFaceVertices(*incidentBox, incidentAxis, incidentPositiveFace, incidentFace);

    int axis1 = (referenceAxis + 1) % 3;
    int axis2 = (referenceAxis + 2) % 3;

    XMFLOAT3 refCenter = AveragePoints(referenceFace, 4);
    XMFLOAT3 refAxis1 = referenceBox->axes[axis1];
    XMFLOAT3 refAxis2 = referenceBox->axes[axis2];

    float refExtent1 = (axis1 == 0) ? referenceBox->halfExtents.x :
        (axis1 == 1) ? referenceBox->halfExtents.y :
        referenceBox->halfExtents.z;

    float refExtent2 = (axis2 == 0) ? referenceBox->halfExtents.x :
        (axis2 == 1) ? referenceBox->halfExtents.y :
        referenceBox->halfExtents.z;

    XMFLOAT3 clip0[8];
    XMFLOAT3 clip1[8];
    XMFLOAT3 clip2[8];
    XMFLOAT3 clip3[8];
    XMFLOAT3 clip4[8];

    for (int i = 0; i < 4; ++i)
        clip0[i] = incidentFace[i];

    int count = 4;

    XMFLOAT3 planePoint = Add(refCenter, Mul(refAxis1, refExtent1));
    XMFLOAT3 planeNormal = refAxis1;
    count = ClipPolygonAgainstPlane(clip0, count, clip1, planePoint, planeNormal);
    if (count <= 0) return false;

    planePoint = Add(refCenter, Mul(refAxis1, -refExtent1));
    planeNormal = Inverse(refAxis1);
    count = ClipPolygonAgainstPlane(clip1, count, clip2, planePoint, planeNormal);
    if (count <= 0) return false;

    planePoint = Add(refCenter, Mul(refAxis2, refExtent2));
    planeNormal = refAxis2;
    count = ClipPolygonAgainstPlane(clip2, count, clip3, planePoint, planeNormal);
    if (count <= 0) return false;

    planePoint = Add(refCenter, Mul(refAxis2, -refExtent2));
    planeNormal = Inverse(refAxis2);
    count = ClipPolygonAgainstPlane(clip3, count, clip4, planePoint, planeNormal);
    if (count <= 0) return false;

    XMFLOAT3 facePlanePoint = referenceFace[0];
    XMFLOAT3 facePlaneNormal = referencePositiveFace ? referenceBox->axes[referenceAxis] : Inverse(referenceBox->axes[referenceAxis]);

    m_contactHolder.pointCount = 0;

    for (int i = 0; i < count && m_contactHolder.pointCount < 4; ++i)
    {
        float separation = Dot(Subtract(clip4[i], facePlanePoint), facePlaneNormal);

        if (separation <= 0.01f)
            m_contactHolder.points[m_contactHolder.pointCount++].position = clip4[i];
    }

    return m_contactHolder.pointCount > 0;
}

bool ColliderSystem::CheckOBBToOBB(ColliderComponent& _boxA, ColliderComponent& _boxB)
{
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

    float minOverlap = FLT_MAX;
    int minAxisIndex = -1;

    for (int i = 0; i < 15; ++i)
    {
        if (OverlapOnAxis(_boxA.obb, _boxB.obb, axes[i], minOverlap, minAxisIndex, i) == false)
            return false;
    }

    if (_boxA.isTrigger || _boxB.isTrigger)
        return true;

    XMFLOAT3 normal = Normalize(axes[minAxisIndex]);
    XMFLOAT3 centerDelta = Subtract(_boxB.obb.center, _boxA.obb.center);

    if (Dot(centerDelta, normal) < 0.0f)
        normal = Inverse(normal);

    m_contactHolder.normal = normal;
    m_contactHolder.penetration = minOverlap;
    m_contactHolder.pointCount = 0;

    bool built = false;

    if (minAxisIndex >= 0 && minAxisIndex <= 5)
        built = BuildOBBContactManifoldByClipping(_boxA.obb, _boxB.obb, normal, minAxisIndex);

    if (built == false)
    {
        BuildOBBContactPoints(_boxA.obb, _boxB.obb);

        if (m_contactHolder.pointCount <= 0)
        {
            XMFLOAT3 pointA = OBBSupportPoint(_boxA.obb, normal);
            XMFLOAT3 pointB = OBBSupportPoint(_boxB.obb, Inverse(normal));
            m_contactHolder.points[0].position = Mul(Add(pointA, pointB), 0.5f);
            m_contactHolder.pointCount = 1;
        }
    }

    return true;
}

bool ColliderSystem::CheckSphereToSphere(ColliderComponent& _sphereA, TransformComponent& _transformA,
    ColliderComponent& _sphereB, TransformComponent& _transformB)
{
    XMFLOAT3 posA = _transformA.local.GetPosition();
    XMFLOAT3 posB = _transformB.local.GetPosition();

    float radiusA = _sphereA.colliderTransform.GetScale().x * _transformA.local.GetScale().x * 0.5f;
    float radiusB = _sphereB.colliderTransform.GetScale().x * _transformB.local.GetScale().x * 0.5f;

    XMFLOAT3 delta = Subtract(posB, posA);
    float d2 = Dot(delta, delta);
    float radiusSum = radiusA + radiusB;

    if (d2 > radiusSum * radiusSum)
        return false;

    if (_sphereA.isTrigger || _sphereB.isTrigger)
        return true;

    float distance = sqrtf(d2);

    if (distance < 1e-6f)
    {
        m_contactHolder.normal = { 0.0f, 1.0f, 0.0f };
        m_contactHolder.penetration = radiusSum;
        m_contactHolder.pointCount = 1;
        m_contactHolder.points[0].position = posA;
        return true;
    }

    XMFLOAT3 normal = Mul(delta, 1.0f / distance);

    XMFLOAT3 pointA = Add(posA, Mul(normal, radiusA));
    XMFLOAT3 pointB = Subtract(posB, Mul(normal, radiusB));
    XMFLOAT3 contactPoint = Mul(Add(pointA, pointB), 0.5f);

    m_contactHolder.normal = normal;
    m_contactHolder.penetration = radiusSum - distance;
    m_contactHolder.pointCount = 1;
    m_contactHolder.points[0].position = contactPoint;

    return true;
}

bool ColliderSystem::CheckBoxToSphere(ColliderComponent& _box, TransformComponent& _transformBox,
    ColliderComponent& _sphere, TransformComponent& _transformSphere)
{
    OBB& obb = _box.obb;

    XMFLOAT3 spherePosition = _transformSphere.local.GetPosition();
    XMFLOAT3 sphereScale = Mul(_sphere.colliderTransform.GetScale(), _transformSphere.local.GetScale());
    float sphereRadius = sphereScale.x * 0.5f;

    XMFLOAT3 d = Subtract(spherePosition, obb.center);

    float localX = Dot(d, obb.axes[0]);
    float localY = Dot(d, obb.axes[1]);
    float localZ = Dot(d, obb.axes[2]);

    float clampedX = Clamp(localX, -obb.halfExtents.x, obb.halfExtents.x);
    float clampedY = Clamp(localY, -obb.halfExtents.y, obb.halfExtents.y);
    float clampedZ = Clamp(localZ, -obb.halfExtents.z, obb.halfExtents.z);

    XMFLOAT3 closest = obb.center;
    closest = Add(closest, Mul(obb.axes[0], clampedX));
    closest = Add(closest, Mul(obb.axes[1], clampedY));
    closest = Add(closest, Mul(obb.axes[2], clampedZ));

    XMFLOAT3 delta = Subtract(spherePosition, closest);
    float d2 = Dot(delta, delta);
    float radius2 = sphereRadius * sphereRadius;

    if (d2 > radius2)
        return false;

    if (_box.isTrigger || _sphere.isTrigger)
        return true;

    if (d2 > 1e-6f)
    {
        XMFLOAT3 normal = Normalize(delta);
        float penetration = sphereRadius - sqrtf(d2);

        m_contactHolder.normal = normal;
        m_contactHolder.penetration = penetration;
        m_contactHolder.pointCount = 1;
        m_contactHolder.points[0].position = closest;
        return true;
    }

    float dx = obb.halfExtents.x - abs(localX);
    float dy = obb.halfExtents.y - abs(localY);
    float dz = obb.halfExtents.z - abs(localZ);

    XMFLOAT3 normal = { 0.0f, 0.0f, 0.0f };
    float distanceToSurface = 0.0f;

    if (dx <= dy && dx <= dz)
    {
        normal = (localX >= 0.0f) ? obb.axes[0] : Inverse(obb.axes[0]);
        distanceToSurface = dx;
    }
    else if (dy <= dx && dy <= dz)
    {
        normal = (localY >= 0.0f) ? obb.axes[1] : Inverse(obb.axes[1]);
        distanceToSurface = dy;
    }
    else
    {
        normal = (localZ >= 0.0f) ? obb.axes[2] : Inverse(obb.axes[2]);
        distanceToSurface = dz;
    }

    m_contactHolder.normal = normal;
    m_contactHolder.penetration = sphereRadius + distanceToSurface;
    m_contactHolder.pointCount = 1;
    m_contactHolder.points[0].position = closest;

    return true;
}