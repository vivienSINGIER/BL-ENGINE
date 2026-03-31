#include "BroadPhaseSystem.h"
#include "Utils.hpp"
#include <algorithm>
#include "../ECS/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────

void BroadPhaseSystem::OnStartUpdate(float _dt)
{
    // Réinitialisation sans réallocation si la capacité est déjà suffisante.
    m_entries.clear();
    m_candidatePairs.clear();
}

void BroadPhaseSystem::OnUpdate(float _dt, EntityId _e,
    ShapeComponent& _shape, TransformComponent& _transform)
{
    ComputeWorldAABB(_shape, _transform);
    InsertIntoSpatialHash(_e, _shape);
}

void BroadPhaseSystem::OnEndUpdate(float _dt)
{
    BuildCandidatePairs();
}

// ─────────────────────────────────────────────────────────────────────────────
// Calcul de l'AABB monde
// ─────────────────────────────────────────────────────────────────────────────

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

// ─────────────────────────────────────────────────────────────────────────────
// Spatial hash — insertion
// ─────────────────────────────────────────────────────────────────────────────

void BroadPhaseSystem::InsertIntoSpatialHash(EntityId _e, ShapeComponent& _shape)
{
    int cxMin = static_cast<int>(floorf(_shape.aabb.min.x / m_cellSize));
    int cyMin = static_cast<int>(floorf(_shape.aabb.min.y / m_cellSize));
    int czMin = static_cast<int>(floorf(_shape.aabb.min.z / m_cellSize));
    int cxMax = static_cast<int>(floorf(_shape.aabb.max.x / m_cellSize));
    int cyMax = static_cast<int>(floorf(_shape.aabb.max.y / m_cellSize));
    int czMax = static_cast<int>(floorf(_shape.aabb.max.z / m_cellSize));

    for (int x = cxMin; x <= cxMax; ++x)
        for (int y = cyMin; y <= cyMax; ++y)
            for (int z = czMin; z <= czMax; ++z)
                m_entries.push_back({ HashCell(x, y, z), _e });
}

// ─────────────────────────────────────────────────────────────────────────────
// Construction des paires candidates
//
//  1. Tri par cellKey  →  O(N log N), accès séquentiel, cache-friendly.
//  2. Pour chaque run de même clé, génère toutes les paires (i, j).
//  3. Tri des clés de paires + std::unique  →  déduplication sans HashMap.
// ─────────────────────────────────────────────────────────────────────────────

void BroadPhaseSystem::BuildCandidatePairs()
{
    if (m_entries.empty())
        return;

    std::sort(m_entries.begin(), m_entries.end());

    m_pairKeys.clear();

    size_t i = 0;
    while (i < m_entries.size())
    {
        // Trouver la fin du run de même cellKey.
        size_t j = i + 1;
        while (j < m_entries.size() && m_entries[j].cellKey == m_entries[i].cellKey)
            ++j;

        // Générer toutes les paires dans ce run.
        for (size_t a = i; a < j; ++a)
            for (size_t b = a + 1; b < j; ++b)
                m_pairKeys.push_back(MakePairKey(m_entries[a].entityId, m_entries[b].entityId));

        i = j;
    }

    // Déduplication.
    std::sort(m_pairKeys.begin(), m_pairKeys.end());
    auto last = std::unique(m_pairKeys.begin(), m_pairKeys.end());
    m_pairKeys.erase(last, m_pairKeys.end());

    // Reconstruction des paires (EntityId, EntityId).
    m_candidatePairs.reserve(m_pairKeys.size());
    for (uint64 key : m_pairKeys)
    {
        EntityId a = static_cast<EntityId>(key >> 32);
        EntityId b = static_cast<EntityId>(key & 0xFFFFFFFF);

        // Test AABB final — filtre les faux positifs du hash (collisions de clé).
        ShapeComponent& shapeA = world->GetComponent<ShapeComponent>(a);
        ShapeComponent& shapeB = world->GetComponent<ShapeComponent>(b);

        if (shapeA.aabb.Overlaps(shapeB.aabb))
            m_candidatePairs.push_back({ a, b });
    }
}
