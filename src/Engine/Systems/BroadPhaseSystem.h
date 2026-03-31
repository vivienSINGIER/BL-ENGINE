#ifndef BROAD_PHASE_SYSTEM_H_DEFINED
#define BROAD_PHASE_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/ShapeComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/TransformComponent.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// CandidatePair
//
//  Paire d'entités candidates à la narrow phase.
//  Stockée dans un buffer plat trié — pas de HashMap, pas d'allocation.
// ─────────────────────────────────────────────────────────────────────────────
struct CandidatePair
{
    EntityId a;
    EntityId b;
};


// ─────────────────────────────────────────────────────────────────────────────
// BroadPhaseSystem
//
//  Responsabilités :
//   1. Calculer l'AABB monde et worldCenter/worldRadius pour chaque entité.
//   2. Insérer chaque entité dans le spatial hash 3D.
//   3. Construire la liste des paires candidates (AABB overlaps) sans doublons.
//
//  Algorithme :
//   - Chaque entité est insérée dans toutes les cellules recouvertes par son AABB.
//   - Les entrées sont triées par cellKey (O(N log N), séquentiel).
//   - Pour chaque run de même clé, on génère les paires (i, j).
//   - Les paires sont dédupliquées par tri + std::unique sur uint64.
//
//  Résultat accessible via GetCandidatePairs() — lu par NarrowPhaseSystem.
//
//  Ordre d'exécution : premier système de la phase FixedUpdate.
// ─────────────────────────────────────────────────────────────────────────────
class BroadPhaseSystem : public System<ShapeComponent, TransformComponent>
{
public:
    void OnStartUpdate(float _dt) override;
    void OnUpdate(float _dt, EntityId _e, ShapeComponent& _shape, TransformComponent& _transform) override;
    void OnEndUpdate(float _dt) override;

    void SetCellSize(float _cellSize) { m_cellSize = _cellSize; }

    const Vector<CandidatePair>& GetCandidatePairs() const { return m_candidatePairs; }

private:
    // ─── AABB ─────────────────────────────────────────────────────────────────
    void ComputeWorldAABB(ShapeComponent& _shape, TransformComponent& _transform);

    // ─── Spatial hash ─────────────────────────────────────────────────────────
    void InsertIntoSpatialHash(EntityId _e, ShapeComponent& _shape);
    void BuildCandidatePairs();

    inline uint32 HashCell(int _x, int _y, int _z) const
    {
        return static_cast<uint32>(
            (_x * 73856093) ^
            (_y * 19349663) ^
            (_z * 83492791)
        );
    }

    inline uint64 MakePairKey(EntityId _a, EntityId _b) const
    {
        if (_a > _b) std::swap(_a, _b);
        return (static_cast<uint64>(static_cast<uint32>(_a)) << 32) |
                static_cast<uint32>(_b);
    }

private:
    float m_cellSize = 4.0f;

    // Entrée du spatial hash : clé de cellule + entité.
    struct SpatialHashEntry
    {
        uint32   cellKey;
        EntityId entityId;

        bool operator<(const SpatialHashEntry& _o) const
        {
            return cellKey < _o.cellKey;
        }
    };

    Vector<SpatialHashEntry> m_entries;     // buffer plat — trié en OnEndUpdate
    Vector<uint64>           m_pairKeys;    // clés de paires avant déduplication
    Vector<CandidatePair>    m_candidatePairs;
};

#endif // !BROAD_PHASE_SYSTEM_H_DEFINED
