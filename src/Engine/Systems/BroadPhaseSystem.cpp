#include "BroadPhaseSystem.h"
#include "Utils.hpp"
#include <algorithm>
#include "../ECS/World.h"
#include "../Components/RigidBodyComponent.hpp"

void BroadPhaseSystem::OnStartUpdate(float _dt)
{
    m_candidatePairs.clear();
}

void BroadPhaseSystem::OnUpdate(float _dt, EntityId _e, ShapeComponent& _shape, TransformComponent& _transform)
{
    if (_shape.toHash)
    {
        ComputeWorldAABB(_shape, _transform);
        InsertIntoGrid(_e, _shape);
        _shape.toHash = false;
    }
}

void BroadPhaseSystem::OnEndUpdate(float _dt)
{
    BuildCandidatePairs();
}

void BroadPhaseSystem::ComputeWorldAABB(ShapeComponent& _shape, TransformComponent& _transform)
{
    const XMFLOAT3& pos   = _transform.world.GetPosition();
    const XMFLOAT3& scale = _transform.world.GetScale();

    // Appliquer l'offset local (tourné par la rotation monde).
    XMFLOAT3 center = pos;
    if (_shape.localOffset.x != 0.0f || _shape.localOffset.y != 0.0f || _shape.localOffset.z != 0.0f)
    {
        XMVECTOR q      = XMLoadFloat4(&_transform.world.GetRotation());
        XMMATRIX rot    = XMMatrixRotationQuaternion(q);
        XMVECTOR offset = XMVector3Transform(
            XMVectorSet(_shape.localOffset.x, _shape.localOffset.y, _shape.localOffset.z, 0.0f),
            rot
        );
        XMFLOAT3 off;
        XMStoreFloat3(&off, offset);
        center = { pos.x + off.x, pos.y + off.y, pos.z + off.z };
    }

    _shape.worldCenter = center;

    switch (_shape.type)
    {
        case ShapeType::Sphere:
        {
            // Rayon monde = rayon local * max(scale).
            float maxScale    = Max(Max(scale.x, scale.y), scale.z);
            float worldRadius = _shape.shape.sphere.radius * maxScale;
            _shape.worldRadius = worldRadius;

            _shape.aabb.min = { center.x - worldRadius, center.y - worldRadius, center.z - worldRadius };
            _shape.aabb.max = { center.x + worldRadius, center.y + worldRadius, center.z + worldRadius };
            break;
        }

        case ShapeType::Box:
        {
            // AABB d'un OBB : on projette les axes orientés sur chaque axe monde.
            XMVECTOR q   = XMLoadFloat4(&_transform.world.GetRotation());
            XMMATRIX rot = XMMatrixRotationQuaternion(q);

            XMFLOAT3 axisX, axisY, axisZ;
            XMStoreFloat3(&axisX, XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(1,0,0,0), rot)));
            XMStoreFloat3(&axisY, XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0,1,0,0), rot)));
            XMStoreFloat3(&axisZ, XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0,0,1,0), rot)));

            const XMFLOAT3& h = _shape.shape.box.halfExtents;
            float hx = h.x * scale.x;
            float hy = h.y * scale.y;
            float hz = h.z * scale.z;

            float wx = fabsf(axisX.x)*hx + fabsf(axisY.x)*hy + fabsf(axisZ.x)*hz;
            float wy = fabsf(axisX.y)*hx + fabsf(axisY.y)*hy + fabsf(axisZ.y)*hz;
            float wz = fabsf(axisX.z)*hx + fabsf(axisY.z)*hy + fabsf(axisZ.z)*hz;

            _shape.worldRadius = sqrtf(wx*wx + wy*wy + wz*wz);
            _shape.aabb.min = { center.x - wx, center.y - wy, center.z - wz };
            _shape.aabb.max = { center.x + wx, center.y + wy, center.z + wz };
            break;
        }

        case ShapeType::Capsule:
        {
            // La capsule est orientée selon l'axe Y local. On tourne cet axe.
            XMVECTOR q   = XMLoadFloat4(&_transform.world.GetRotation());
            XMMATRIX rot = XMMatrixRotationQuaternion(q);

            XMFLOAT3 worldUp;
            XMStoreFloat3(&worldUp, XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0,1,0,0), rot)));

            float r  = _shape.shape.capsule.radius * Max(scale.x, scale.z);
            float hh = _shape.shape.capsule.halfHeight * scale.y;

            // Centres des deux hémisphères en espace monde.
            XMFLOAT3 topCenter =
            {
                center.x + worldUp.x * hh,
                center.y + worldUp.y * hh,
                center.z + worldUp.z * hh
            };
            XMFLOAT3 botCenter =
            {
                center.x - worldUp.x * hh,
                center.y - worldUp.y * hh,
                center.z - worldUp.z * hh
            };

            _shape.worldRadius = hh + r;

            _shape.aabb.min =
            {
                Min(topCenter.x, botCenter.x) - r,
                Min(topCenter.y, botCenter.y) - r,
                Min(topCenter.z, botCenter.z) - r
            };
            _shape.aabb.max =
            {
                Max(topCenter.x, botCenter.x) + r,
                Max(topCenter.y, botCenter.y) + r,
                Max(topCenter.z, botCenter.z) + r
            };
            break;
        }
    }
}

void BroadPhaseSystem::InsertIntoGrid(EntityId _e, ShapeComponent& _shape)
{
    int cellSize = m_grid.cellSize;

    int cxMin = static_cast<int>(floorf(_shape.aabb.min.x / cellSize));
    int cyMin = static_cast<int>(floorf(_shape.aabb.min.y / cellSize));
    int czMin = static_cast<int>(floorf(_shape.aabb.min.z / cellSize));
    int cxMax = static_cast<int>(floorf(_shape.aabb.max.x / cellSize));
    int cyMax = static_cast<int>(floorf(_shape.aabb.max.y / cellSize));
    int czMax = static_cast<int>(floorf(_shape.aabb.max.z / cellSize));

    for (int x = cxMin; x <= cxMax; x++)
        for (int y = cyMin; y <= cyMax; y++)
            for (int z = czMin; z <= czMax; z++)
                m_grid.Insert(HashCell(x, y, z), _e);
}

void BroadPhaseSystem::BuildCandidatePairs()
{
    m_pairKeys.clear();
    m_grid.ForEachCell([&](const Vector<EntityId>& bucket)
        {
            for (size_t i = 0; i < bucket.size(); i++)
            {
                EntityId e1 = bucket[i];
                BodyType e1Type = BodyType::Static;

                if (world->HasComponent<RigidBodyComponent>(e1))
                    e1Type = world->GetComponent<RigidBodyComponent>(e1).type;

                if (e1Type != BodyType::Dynamic)
                    continue;
                
                for (size_t j = i + 1; j < bucket.size(); j++)
                {
                    EntityId e2 = bucket[j];
                    m_pairKeys.push_back(MakePairKey(e1, e2));
                }
            }
        });

    std::sort(m_pairKeys.begin(), m_pairKeys.end());
    auto last = std::unique(m_pairKeys.begin(), m_pairKeys.end());
    m_pairKeys.erase(last, m_pairKeys.end());

    m_candidatePairs.reserve(m_pairKeys.size());
    for (uint64 key : m_pairKeys)
    {
        EntityId a = static_cast<EntityId>(key >> 32);
        EntityId b = static_cast<EntityId>(key & 0xFFFFFFFF);

        ShapeComponent& shapeA = world->GetComponent<ShapeComponent>(a);
        ShapeComponent& shapeB = world->GetComponent<ShapeComponent>(b);

        if (shapeA.aabb.Overlaps(shapeB.aabb))
            m_candidatePairs.push_back({ a, b });
    }
}

uint32 BroadPhaseSystem::HashCell(int _x, int _y, int _z) const
{
    return static_cast<uint32>(
        (_x * 73856093) ^
        (_y * 19349663) ^
        (_z * 83492791) );
}

uint64 BroadPhaseSystem::MakePairKey(EntityId _a, EntityId _b) const
{
    if (_a > _b) std::swap(_a, _b);
    return (static_cast<uint64>(static_cast<uint32>(_a)) << 32) | static_cast<uint32>(_b);
}
