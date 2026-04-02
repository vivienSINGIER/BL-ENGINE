#ifndef BROAD_PHASE_SYSTEM_H_DEFINED
#define BROAD_PHASE_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/ColliderComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include <unordered_map>

struct CandidatePair
{
    EntityId a;
    EntityId b;
};

// ─────────────────────────────────────────────────────────────────────────────
// CellBucket
// ─────────────────────────────────────────────────────────────────────────────
struct CellBucket
{
    Vector<EntityId> dynamic; // sources : Dynamic + triggers
    Vector<EntityId> all;     // cibles  : tout le monde

    void Clear() { dynamic.clear(); all.clear(); }
    bool IsEmpty() const { return all.empty(); }
};

struct SpatialHashGrid
{
    std::unordered_map<uint32, CellBucket> cells;

    void Clear()
    {
        for (auto& [key, bucket] : cells)
            bucket.Clear();
    }

    void InsertAll(uint32 _key, EntityId _e) { cells[_key].all.push_back(_e); }
    void InsertDynamic(uint32 _key, EntityId _e) { cells[_key].dynamic.push_back(_e); }

    void RemoveAll(uint32 _key, EntityId _e);
    void RemoveDynamic(uint32 _key, EntityId _e);

    template<typename Fn>
    void ForEachCell(Fn&& _fn) const
    {
        for (const auto& [key, bucket] : cells)
            if (!bucket.IsEmpty())
                _fn(key, bucket);
    }
};


// ─────────────────────────────────────────────────────────────────────────────
// EntityGridState
//
//  État mémorisé par entité dans le broad phase.
//  Permet de détecter si une entité a bougé et de nettoyer ses anciennes
//  cellules sans parcourir toute la grille.
// ─────────────────────────────────────────────────────────────────────────────
struct EntityGridState
{
    XMFLOAT3      lastPosition = { 1e38f, 1e38f, 1e38f }; // sentinelle → force update à l'init
    XMFLOAT4      lastRotation = { 0, 0, 0, 1 };
    XMFLOAT3      lastScale = { 1, 1, 1 };
    Vector<uint32> occupiedKeys;                            // clés de cellules actuellement occupées
    bool           isDynamicSrc = false;                    // source de paires ?
};


// ─────────────────────────────────────────────────────────────────────────────
// BroadPhaseSystem
//
//  Grille unifiée avec tracking par entité.
//
//  Principe :
//   Chaque entité mémorise sa dernière position/rotation/scale connue.
//   Dans OnUpdate, on compare l'état actuel avec l'état mémorisé.
//   Si identique → l'entité n'a pas bougé → on ne touche pas à la grille.
//   Si différent → on retire l'entité de ses anciennes cellules, on recalcule
//                  l'AABB, on insère dans les nouvelles cellules.
//
//  Coût par frame :
//   - Entité statique  : 3 comparaisons float3 → 0 travail après init.
//   - Entité kinematic : idem, sauf si elle a bougé ce frame.
//   - Entité dynamic   : toujours mis à jour (sa position change chaque frame).
//
//  Pas besoin d'API externe pour marquer les entités dirty —
//  le changement de position est détecté automatiquement.
// ─────────────────────────────────────────────────────────────────────────────
class BroadPhaseSystem : public System<ColliderComponent, TransformComponent>
{
public:
    void OnStartUpdate(float _dt) override;
    void OnUpdate(float _dt, EntityId _e, ColliderComponent& _shape, TransformComponent& _transform) override;
    void OnEndUpdate(float _dt) override;

    void SetCellSize(float _cellSize) { m_cellSize = _cellSize; }

    // Appelé quand une entité est détruite — nettoie son état et ses cellules.
    void OnEntityDestroyed(EntityId _e);

    const Vector<CandidatePair>& GetCandidatePairs() const { return m_candidatePairs; }

private:
    // ─── AABB ─────────────────────────────────────────────────────────────────
    void ComputeWorldAABB(ColliderComponent& _shape, TransformComponent& _transform);

    // ─── Grille ───────────────────────────────────────────────────────────────
    void UpdateEntityInGrid(EntityId _e, ColliderComponent& _shape, TransformComponent& _transform, EntityGridState& _state);
    void RemoveEntityFromGrid(EntityId _e, EntityGridState& _state);
    void InsertEntityIntoGrid(EntityId _e, ColliderComponent& _shape, EntityGridState& _state);

    // ─── Paires ───────────────────────────────────────────────────────────────
    void BuildCandidatePairs();

    // ─── Helpers ──────────────────────────────────────────────────────────────
    bool HasMoved(const EntityGridState& _state, TransformComponent& _transform) const;
    bool IsDynamicSource(EntityId _e) const;

    inline uint32 HashCell(int _x, int _y, int _z) const
    {
        return static_cast<uint32>(
            (_x * 73856093) ^ (_y * 19349663) ^ (_z * 83492791));
    }

    inline uint64 MakePairKey(EntityId _a, EntityId _b) const
    {
        if (_a > _b) std::swap(_a, _b);
        return (static_cast<uint64>(static_cast<uint32>(_a)) << 32) |
            static_cast<uint32>(_b);
    }

    static constexpr float kMovedEpsilon = 1e-5f;

private:
    float          m_cellSize = 4.0f;
    SpatialHashGrid m_grid;

    // État par entité — persisté entre frames.
    std::unordered_map<EntityId, EntityGridState> m_entityStates;

    Vector<uint64>        m_pairKeys;
    Vector<CandidatePair> m_candidatePairs;
};

#endif // !BROAD_PHASE_SYSTEM_H_DEFINED