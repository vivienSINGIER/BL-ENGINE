#include "BroadPhaseSystem.h"
#include "Utils.hpp"
#include <algorithm>
#include "../ECS/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// SpatialHashGrid — suppression ciblée
// ─────────────────────────────────────────────────────────────────────────────

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

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────

void BroadPhaseSystem::OnStartUpdate(float _dt)
{
    m_candidatePairs.clear();
}

void BroadPhaseSystem::OnUpdate(float _dt, EntityId _e, ColliderComponent& _collider, TransformComponent& _transform)
{
    EntityGridState& state = m_entityStates[_e];

    // Si l'entité n'a pas bougé depuis la dernière frame, on ne touche pas à la grille.
    if (HasMoved(state, _transform) == false)
        return;

    // L'entité a bougé (ou c'est la première frame) → mise à jour.
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
    if (!world->HasComponent<RigidBodyComponent>(_e))
        return true; // trigger sans RigidBody

    const RigidBodyComponent& r = world->GetComponent<RigidBodyComponent>(_e);
    return r.type == BodyType::Dynamic;
}

void BroadPhaseSystem::BuildCandidatePairs()
{
    m_pairKeys.clear();

    m_grid.ForEachCell([&](uint32 /*key*/, const CellBucket& bucket)
        {
            if (bucket.dynamic.empty())
                return;

            for (EntityId src : bucket.dynamic)
            {
                for (EntityId tgt : bucket.all)
                {
                    if (src == tgt) continue;
                    m_pairKeys.push_back(MakePairKey(src, tgt));
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

        if (!world->HasComponent<ColliderComponent>(a) ||
            !world->HasComponent<ColliderComponent>(b)) continue;

        ColliderComponent& shapeA = world->GetComponent<ColliderComponent>(a);
        ColliderComponent& shapeB = world->GetComponent<ColliderComponent>(b);

        if (shapeA.aabb.Overlaps(shapeB.aabb))
            m_candidatePairs.push_back({ a, b });
    }
}

void BroadPhaseSystem::ComputeWorldAABB(ColliderComponent& _collider, TransformComponent& _transform)
{
    const XMFLOAT3& pos = _transform.world.GetPosition();
    const XMFLOAT3& scale = _transform.world.GetScale();

    XMFLOAT3 center = pos;
    if (_collider.localOffset.x != 0.0f || _collider.localOffset.y != 0.0f || _collider.localOffset.z != 0.0f)
    {
        XMVECTOR q = XMLoadFloat4(&_transform.world.GetRotation());
        XMMATRIX rot = XMMatrixRotationQuaternion(q);
        XMVECTOR off = XMVector3Transform(XMVectorSet(_collider.localOffset.x, _collider.localOffset.y, _collider.localOffset.z, 0.0f), rot);
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
        XMVECTOR q = XMLoadFloat4(&_transform.world.GetRotation());
        XMMATRIX rot = XMMatrixRotationQuaternion(q);
        XMFLOAT3 ax, ay, az;
        XMStoreFloat3(&ax, XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(1, 0, 0, 0), rot)));
        XMStoreFloat3(&ay, XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), rot)));
        XMStoreFloat3(&az, XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), rot)));

        const XMFLOAT3& h = _collider.shape.box.halfExtents;
        float hx = h.x * scale.x, hy = h.y * scale.y, hz = h.z * scale.z;
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
        XMVECTOR q = XMLoadFloat4(&_transform.world.GetRotation());
        XMMATRIX rot = XMMatrixRotationQuaternion(q);
        XMFLOAT3 up;
        XMStoreFloat3(&up, XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), rot)));
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