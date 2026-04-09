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
            if (bucket.IsEmpty() == false)
                _fn(key, bucket);
    }
};

struct EntityGridState
{
    XMFLOAT3 lastPosition = { 1e38f, 1e38f, 1e38f }; // sentinelle -> force update à l'init
    XMFLOAT4 lastRotation = { 0, 0, 0, 1 };
    XMFLOAT3 lastScale = { 1, 1, 1 };
    Vector<uint32> occupiedKeys;  // clés de cellules actuellement occupées
    bool isDynamicSrc = false;    // source de paires ?
};


class BroadPhaseSystem : public System<ColliderComponent, TransformComponent>
{
public:
    void OnStartUpdate(float _dt) override;
    void OnUpdate(float _dt, EntityId _e, ColliderComponent& _collider, TransformComponent& _transform) override;
    void OnEndUpdate(float _dt) override;

    void SetCellSize(float _cellSize) { m_cellSize = _cellSize; }

    // Appelé quand une entité est détruite — nettoie son état et ses cellules.
    void OnEntityDestroyed(EntityId _e);

    const Vector<CandidatePair>& GetCandidatePairs() const { return m_candidatePairs; }
    void ClearAll();

private:
    // AABB
    void ComputeWorldAABB(ColliderComponent& _shape, TransformComponent& _transform);

    // Grille 
    void UpdateEntityInGrid(EntityId _e, ColliderComponent& _collider, TransformComponent& _transform, EntityGridState& _state);
    void RemoveEntityFromGrid(EntityId _e, EntityGridState& _state);
    void InsertEntityIntoGrid(EntityId _e, ColliderComponent& _collider, EntityGridState& _state);

    // Paires
    void BuildCandidatePairs();

    // Helpers 
    bool HasMoved(const EntityGridState& _state, TransformComponent& _transform) const;
    bool IsDynamicSource(EntityId _e) const;

    inline uint32 HashCell(int _x, int _y, int _z) const
    {
        return static_cast<uint32>((_x * 73856093) ^ (_y * 19349663) ^ (_z * 83492791));
    }

    static constexpr float kMovedEpsilon = 1e-5f;

private:
    float m_cellSize = 4.0f;
    SpatialHashGrid m_grid;

    // État par entité — persisté entre frames.
    std::unordered_map<EntityId, EntityGridState> m_entityStates;

    Vector<std::pair<EntityId, EntityId>> m_pairKeys;
    Vector<CandidatePair> m_candidatePairs;
};

#endif // !BROAD_PHASE_SYSTEM_H_DEFINED