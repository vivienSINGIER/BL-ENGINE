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
    // La grille n'est PAS vidée — elle est maintenue en continu.
}

void BroadPhaseSystem::OnUpdate(float _dt, EntityId _e, ShapeComponent& _shape, TransformComponent& _transform)
{
    EntityGridState& state = m_entityStates[_e];

    // Si l'entité n'a pas bougé depuis la dernière frame, on ne touche pas à la grille.
    if (!HasMoved(state, _transform))
        return;

    // L'entité a bougé (ou c'est la première frame) → mise à jour.
    ComputeWorldAABB(_shape, _transform);
    UpdateEntityInGrid(_e, _shape, _transform, state);
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

// ─────────────────────────────────────────────────────────────────────────────
// Mise à jour d'une entité dans la grille
//
//  1. Retirer l'entité de ses anciennes cellules (stockées dans state.occupiedKeys).
//  2. Recalculer les nouvelles cellules depuis l'AABB courante.
//  3. Insérer dans les nouvelles cellules et mémoriser les clés.
//  4. Mettre à jour l'état mémorisé (position, rotation, scale).
// ─────────────────────────────────────────────────────────────────────────────

void BroadPhaseSystem::UpdateEntityInGrid(EntityId _e, ShapeComponent& _shape, TransformComponent& _transform, EntityGridState& _state)
{
    // Retrait des anciennes cellules.
    RemoveEntityFromGrid(_e, _state);

    // Insertion dans les nouvelles cellules.
    _state.isDynamicSrc = IsDynamicSource(_e);
    InsertEntityIntoGrid(_e, _shape, _state);

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

void BroadPhaseSystem::InsertEntityIntoGrid(EntityId _e, ShapeComponent& _shape, EntityGridState& _state)
{
    int cxMin = static_cast<int>(floorf(_shape.aabb.min.x / m_cellSize));
    int cyMin = static_cast<int>(floorf(_shape.aabb.min.y / m_cellSize));
    int czMin = static_cast<int>(floorf(_shape.aabb.min.z / m_cellSize));
    int cxMax = static_cast<int>(floorf(_shape.aabb.max.x / m_cellSize));
    int cyMax = static_cast<int>(floorf(_shape.aabb.max.y / m_cellSize));
    int czMax = static_cast<int>(floorf(_shape.aabb.max.z / m_cellSize));

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

// ─────────────────────────────────────────────────────────────────────────────
// Nettoyage à la destruction d'une entité
// ─────────────────────────────────────────────────────────────────────────────

void BroadPhaseSystem::OnEntityDestroyed(EntityId _e)
{
    auto it = m_entityStates.find(_e);
    if (it == m_entityStates.end()) return;

    RemoveEntityFromGrid(_e, it->second);
    m_entityStates.erase(it);
}

// ─────────────────────────────────────────────────────────────────────────────
// IsDynamicSource
// ─────────────────────────────────────────────────────────────────────────────

bool BroadPhaseSystem::IsDynamicSource(EntityId _e) const
{
    if (!world->HasComponent<RigidBodyComponent>(_e))
        return true; // trigger sans RigidBody

    const RigidBodyComponent& r = world->GetComponent<RigidBodyComponent>(_e);
    return r.type == BodyType::Dynamic;
}

// ─────────────────────────────────────────────────────────────────────────────
// Construction des paires candidates — identique à la version précédente
// ─────────────────────────────────────────────────────────────────────────────

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

        if (!world->HasComponent<ShapeComponent>(a) ||
            !world->HasComponent<ShapeComponent>(b)) continue;

        ShapeComponent& shapeA = world->GetComponent<ShapeComponent>(a);
        ShapeComponent& shapeB = world->GetComponent<ShapeComponent>(b);

        if (shapeA.aabb.Overlaps(shapeB.aabb))
            m_candidatePairs.push_back({ a, b });
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeWorldAABB
// ─────────────────────────────────────────────────────────────────────────────

void BroadPhaseSystem::ComputeWorldAABB(ShapeComponent& _shape, TransformComponent& _transform)
{
    const XMFLOAT3& pos = _transform.world.GetPosition();
    const XMFLOAT3& scale = _transform.world.GetScale();

    XMFLOAT3 center = pos;
    if (_shape.localOffset.x != 0.0f || _shape.localOffset.y != 0.0f || _shape.localOffset.z != 0.0f)
    {
        XMVECTOR q = XMLoadFloat4(&_transform.world.GetRotation());
        XMMATRIX rot = XMMatrixRotationQuaternion(q);
        XMVECTOR off = XMVector3Transform(XMVectorSet(_shape.localOffset.x, _shape.localOffset.y, _shape.localOffset.z, 0.0f), rot);
        XMFLOAT3 o;
        XMStoreFloat3(&o, off);
        center = { pos.x + o.x, pos.y + o.y, pos.z + o.z };
    }

    _shape.worldCenter = center;

    switch (_shape.type)
    {
    case ShapeType::Sphere:
    {
        float r = _shape.shape.sphere.radius * Max(Max(scale.x, scale.y), scale.z);
        _shape.worldRadius = r;
        _shape.aabb.min = { center.x - r, center.y - r, center.z - r };
        _shape.aabb.max = { center.x + r, center.y + r, center.z + r };
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

        const XMFLOAT3& h = _shape.shape.box.halfExtents;
        float hx = h.x * scale.x, hy = h.y * scale.y, hz = h.z * scale.z;
        float wx = fabsf(ax.x) * hx + fabsf(ay.x) * hy + fabsf(az.x) * hz;
        float wy = fabsf(ax.y) * hx + fabsf(ay.y) * hy + fabsf(az.y) * hz;
        float wz = fabsf(ax.z) * hx + fabsf(ay.z) * hy + fabsf(az.z) * hz;
        _shape.worldRadius = sqrtf(wx * wx + wy * wy + wz * wz);
        _shape.aabb.min = { center.x - wx, center.y - wy, center.z - wz };
        _shape.aabb.max = { center.x + wx, center.y + wy, center.z + wz };
        break;
    }
    case ShapeType::Capsule:
    {
        XMVECTOR q = XMLoadFloat4(&_transform.world.GetRotation());
        XMMATRIX rot = XMMatrixRotationQuaternion(q);
        XMFLOAT3 up;
        XMStoreFloat3(&up, XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), rot)));
        float r = _shape.shape.capsule.radius * Max(scale.x, scale.z);
        float hh = _shape.shape.capsule.halfHeight * scale.y;
        XMFLOAT3 top = { center.x + up.x * hh, center.y + up.y * hh, center.z + up.z * hh };
        XMFLOAT3 bot = { center.x - up.x * hh, center.y - up.y * hh, center.z - up.z * hh };
        _shape.worldRadius = hh + r;
        _shape.aabb.min = { Min(top.x,bot.x) - r, Min(top.y,bot.y) - r, Min(top.z,bot.z) - r };
        _shape.aabb.max = { Max(top.x,bot.x) + r, Max(top.y,bot.y) + r, Max(top.z,bot.z) + r };
        break;
    }
    }
}