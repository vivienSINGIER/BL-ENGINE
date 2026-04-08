#include "BroadPhaseSystem.h"
#include "Utils.hpp"
#include <algorithm>
#include "../ECS/World.h"

namespace
{
    inline XMFLOAT4 MulQuat(const XMFLOAT4& a, const XMFLOAT4& b)
    {
        return
        {
            a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
            a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
            a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
        };
    }

    inline XMFLOAT4 NormalizeQuatSafe(const XMFLOAT4& q)
    {
        float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
        if (lenSq <= 1e-12f)
            return { 0.f, 0.f, 0.f, 1.f };

        float invLen = 1.0f / sqrtf(lenSq);
        return { q.x * invLen, q.y * invLen, q.z * invLen, q.w * invLen };
    }

    inline XMFLOAT4 GetColliderWorldRotation(const ColliderComponent& collider, TransformComponent& transform)
    {
        XMFLOAT4 qWorld = transform.world.GetRotation();
        XMFLOAT4 qLocal = collider.localRotation;
        return NormalizeQuatSafe(MulQuat(qWorld, qLocal));
    }
}

void SpatialHashGrid::RemoveAll(uint32 _key, EntityId _e)
{
    auto it = cells.find(_key);
    if (it == cells.end()) return;

    auto& v = it->second.all;
    v.erase(std::remove(v.begin(), v.end(), _e), v.end());
}

void SpatialHashGrid::RemoveDynamic(uint32 _key, EntityId _e)
{
    auto it = cells.find(_key);
    if (it == cells.end()) return;

    auto& v = it->second.dynamic;
    v.erase(std::remove(v.begin(), v.end(), _e), v.end());
}

void BroadPhaseSystem::OnStartUpdate(float _dt)
{
    m_candidatePairs.clear();
}

void BroadPhaseSystem::OnUpdate(float _dt, EntityId _e, ColliderComponent& _collider, TransformComponent& _transform)
{
    EntityGridState& state = m_entityStates[_e];

    if (HasMoved(state, _transform) == false)
        return;

    ComputeWorldAABB(_collider, _transform);
    UpdateEntityInGrid(_e, _collider, _transform, state);
}

void BroadPhaseSystem::OnEndUpdate(float _dt)
{
    BuildCandidatePairs();
}

bool BroadPhaseSystem::HasMoved(const EntityGridState& _state, TransformComponent& _transform) const
{
    const XMFLOAT3& pos = _transform.world.GetPosition();
    const XMFLOAT4& rot = _transform.world.GetRotation();
    const XMFLOAT3& scale = _transform.world.GetScale();

    float eps = kMovedEpsilon;

    // Position.
    if (fabsf(pos.x - _state.lastPosition.x) > eps ||
        fabsf(pos.y - _state.lastPosition.y) > eps ||
        fabsf(pos.z - _state.lastPosition.z) > eps)
        return true;

    // Rotation (quaternion — composante W suffit pour détecter une rotation).
    if (fabsf(rot.x - _state.lastRotation.x) > eps ||
        fabsf(rot.y - _state.lastRotation.y) > eps ||
        fabsf(rot.z - _state.lastRotation.z) > eps ||
        fabsf(rot.w - _state.lastRotation.w) > eps)
        return true;

    // Scale.
    if (fabsf(scale.x - _state.lastScale.x) > eps ||
        fabsf(scale.y - _state.lastScale.y) > eps ||
        fabsf(scale.z - _state.lastScale.z) > eps)
        return true;

    return false;
}

void BroadPhaseSystem::UpdateEntityInGrid(EntityId _e, ColliderComponent& _collider, TransformComponent& _transform, EntityGridState& _state)
{
    // Retrait des anciennes cellules.
    RemoveEntityFromGrid(_e, _state);

    // Insertion dans les nouvelles cellules.
    _state.isDynamicSrc = IsDynamicSource(_e);
    InsertEntityIntoGrid(_e, _collider, _state);

    // Mémoriser l'état courant.
    _state.lastPosition = _transform.world.GetPosition();
    _state.lastRotation = _transform.world.GetRotation();
    _state.lastScale = _transform.world.GetScale();
}

void BroadPhaseSystem::RemoveEntityFromGrid(EntityId _e, EntityGridState& _state)
{
    for (uint32 key : _state.occupiedKeys)
    {
        m_grid.RemoveAll(key, _e);
        if (_state.isDynamicSrc)
            m_grid.RemoveDynamic(key, _e);
    }
    _state.occupiedKeys.clear();
}

void BroadPhaseSystem::InsertEntityIntoGrid(EntityId _e, ColliderComponent& _collider, EntityGridState& _state)
{
    int cxMin = static_cast<int>(floorf(_collider.aabb.min.x / m_cellSize));
    int cyMin = static_cast<int>(floorf(_collider.aabb.min.y / m_cellSize));
    int czMin = static_cast<int>(floorf(_collider.aabb.min.z / m_cellSize));
    int cxMax = static_cast<int>(floorf(_collider.aabb.max.x / m_cellSize));
    int cyMax = static_cast<int>(floorf(_collider.aabb.max.y / m_cellSize));
    int czMax = static_cast<int>(floorf(_collider.aabb.max.z / m_cellSize));

    for (int x = cxMin; x <= cxMax; ++x)
    {
        for (int y = cyMin; y <= cyMax; ++y)
        {
            for (int z = czMin; z <= czMax; ++z)
            {
                uint32 key = HashCell(x, y, z);

                m_grid.InsertAll(key, _e);
                if (_state.isDynamicSrc)
                    m_grid.InsertDynamic(key, _e);

                _state.occupiedKeys.push_back(key);
            }
        }
    }
}

void BroadPhaseSystem::OnEntityDestroyed(EntityId _e)
{
    auto it = m_entityStates.find(_e);
    if (it == m_entityStates.end()) return;

    RemoveEntityFromGrid(_e, it->second);
    m_entityStates.erase(it);
}

bool BroadPhaseSystem::IsDynamicSource(EntityId _e) const
{
    if (world->HasComponent<ColliderComponent>(_e))
    {
        const ColliderComponent& c = world->GetComponent<ColliderComponent>(_e);
        if (c.isTrigger)
            return true;
    }

    if (!world->HasComponent<RigidBodyComponent>(_e))
        return true;

    const RigidBodyComponent& r = world->GetComponent<RigidBodyComponent>(_e);
    return r.type == BodyType::Dynamic;
}

void BroadPhaseSystem::BuildCandidatePairs()
{
    m_pairKeys.clear();
    m_candidatePairs.clear();

    m_grid.ForEachCell([&](uint32 /*key*/, const CellBucket& bucket)
        {
            if (bucket.dynamic.empty())
                return;

            for (EntityId src : bucket.dynamic)
            {
                if (world->IsActive(src) == false)	continue;
                for (EntityId tgt : bucket.all)
                {
                    if (world->IsActive(tgt) == false)	continue;
                    if (src == tgt) continue;
                    // Normalisation (lo, hi) pour que (A,B) et (B,A) soient identiques.
                    EntityId lo = (src < tgt) ? src : tgt;
                    EntityId hi = (src < tgt) ? tgt : src;

                    m_pairKeys.emplace_back(lo, hi);
                }
            }
        });

    // Tri puis suppression des doublons.
    std::sort(m_pairKeys.begin(), m_pairKeys.end());
    m_pairKeys.erase(std::unique(m_pairKeys.begin(), m_pairKeys.end()), m_pairKeys.end());

    // Filtre AABB et construction des paires candidates finales.
    m_candidatePairs.reserve(m_pairKeys.size());
    for (const auto& [lo, hi] : m_pairKeys)
    {
        if (!world->HasComponent<ColliderComponent>(lo) ||
            !world->HasComponent<ColliderComponent>(hi)) continue;

        ColliderComponent& shapeA = world->GetComponent<ColliderComponent>(lo);
        ColliderComponent& shapeB = world->GetComponent<ColliderComponent>(hi);

        if (shapeA.aabb.Overlaps(shapeB.aabb))
            m_candidatePairs.push_back({ lo, hi });
    }
}

void BroadPhaseSystem::ComputeWorldAABB(ColliderComponent& _collider, TransformComponent& _transform)
{
    const XMFLOAT3& pos = _transform.world.GetPosition();
    const XMFLOAT3& scale = _transform.world.GetScale();

    XMFLOAT3 center = pos;

    XMFLOAT4 colliderWorldRot = GetColliderWorldRotation(_collider, _transform);
    XMVECTOR qCollider = XMLoadFloat4(&colliderWorldRot);
    XMMATRIX colliderRotMat = XMMatrixRotationQuaternion(qCollider);

    if (_collider.localOffset.x != 0.0f ||
        _collider.localOffset.y != 0.0f ||
        _collider.localOffset.z != 0.0f)
    {
        XMVECTOR qTransform = XMLoadFloat4(&_transform.world.GetRotation());
        XMMATRIX transformRotMat = XMMatrixRotationQuaternion(qTransform);
        XMVECTOR off = XMVector3TransformNormal(
            XMVectorSet(_collider.localOffset.x,
                _collider.localOffset.y,
                _collider.localOffset.z, 0.f), transformRotMat);
        XMFLOAT3 o;
        XMStoreFloat3(&o, off);
        center = { pos.x + o.x, pos.y + o.y, pos.z + o.z };
    }

    _collider.worldCenter = center;

    switch (_collider.type)
    {
    case ShapeType::Sphere:
    {
        float r = _collider.shape.sphere.radius * Max(Max(scale.x, scale.y), scale.z);
        _collider.worldRadius = r;
        _collider.aabb.min = { center.x - r, center.y - r, center.z - r };
        _collider.aabb.max = { center.x + r, center.y + r, center.z + r };
        break;
    }

    case ShapeType::Box:
    {
        // Axes unitaires en world space (rotation du transform * rotation locale du collider)
        XMStoreFloat3(&_collider.worldAxes[0], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(1, 0, 0, 0), colliderRotMat)));
        XMStoreFloat3(&_collider.worldAxes[1], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), colliderRotMat)));
        XMStoreFloat3(&_collider.worldAxes[2], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), colliderRotMat)));

        // Demi-extents mis à l'échelle (lus par NarrowPhase pour le SAT)
        const XMFLOAT3& h = _collider.shape.box.halfExtents;
        _collider.worldHalfExtents = { h.x * scale.x, h.y * scale.y, h.z * scale.z };

        // AABB monde = projection de l'OBB sur les axes monde
        const XMFLOAT3& ax = _collider.worldAxes[0];
        const XMFLOAT3& ay = _collider.worldAxes[1];
        const XMFLOAT3& az = _collider.worldAxes[2];
        const float hx = _collider.worldHalfExtents.x;
        const float hy = _collider.worldHalfExtents.y;
        const float hz = _collider.worldHalfExtents.z;

        float wx = fabsf(ax.x) * hx + fabsf(ay.x) * hy + fabsf(az.x) * hz;
        float wy = fabsf(ax.y) * hx + fabsf(ay.y) * hy + fabsf(az.y) * hz;
        float wz = fabsf(ax.z) * hx + fabsf(ay.z) * hy + fabsf(az.z) * hz;

        _collider.worldRadius = sqrtf(wx * wx + wy * wy + wz * wz);
        _collider.aabb.min = { center.x - wx, center.y - wy, center.z - wz };
        _collider.aabb.max = { center.x + wx, center.y + wy, center.z + wz };
        break;
    }

    case ShapeType::Capsule:
    {
        XMFLOAT3 up;
        XMStoreFloat3(&up, XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), colliderRotMat)));

        float r = _collider.shape.capsule.radius * Max(scale.x, scale.z);
        float hh = _collider.shape.capsule.halfHeight * scale.y;

        XMFLOAT3 top = { center.x + up.x * hh, center.y + up.y * hh, center.z + up.z * hh };
        XMFLOAT3 bot = { center.x - up.x * hh, center.y - up.y * hh, center.z - up.z * hh };

        _collider.worldRadius = hh + r;
        _collider.aabb.min = { Min(top.x,bot.x) - r, Min(top.y,bot.y) - r, Min(top.z,bot.z) - r };
        _collider.aabb.max = { Max(top.x,bot.x) + r, Max(top.y,bot.y) + r, Max(top.z,bot.z) + r };
        break;
    }
    }
}