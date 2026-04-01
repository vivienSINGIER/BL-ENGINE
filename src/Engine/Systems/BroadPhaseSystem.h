#ifndef BROAD_PHASE_SYSTEM_H_DEFINED
#define BROAD_PHASE_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/ShapeComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/TransformComponent.hpp"

struct CandidatePair
{
    EntityId a;
    EntityId b;
};

struct Cell
{
    Vector<EntityId> dynamics;
    Vector<EntityId> others;
};

struct SpatialHashGrid
{
    float cellSize = 5.0f;
    UnorderedMap<uint32, Vector<Cell>> cells;

    void Clear()
    {
        for (auto& [key, bucket] : cells)
            bucket.clear();
    }

    void Insert(uint32 _key, EntityId _e, BodyType _type)
    {
        BodyType eType = BodyType::Static;

        if (_type == BodyType::Dynamic)  
            cells[_key];
    }

    template<typename Fn>
    void ForEachCell(Fn&& _fn) const
    {
        for (const auto& [key, bucket] : cells)
        {
            if (bucket.empty() == false)
                _fn(bucket);
        }
    }
};

class BroadPhaseSystem : public System<ShapeComponent, TransformComponent>
{
public:
    void OnStartUpdate(float _dt) override;
    void OnUpdate(float _dt, EntityId _e, ShapeComponent& _shape, TransformComponent& _transform) override;
    void OnEndUpdate(float _dt) override;

    void SetCellSize(float _cellSize) { m_grid.cellSize = _cellSize; }

    const Vector<CandidatePair>& GetCandidatePairs() const { return m_candidatePairs; }

private:
    // AABB
    void ComputeWorldAABB(ShapeComponent& _shape, TransformComponent& _transform);
    void InsertIntoGrid(EntityId _e, ShapeComponent& _shape);
    void BuildCandidatePairs();

    uint32 HashCell(int _x, int _y, int _z) const;
    uint64 MakePairKey(EntityId _a, EntityId _b) const;


    SpatialHashGrid         m_grid;  
    Vector<uint64>          m_pairKeys;
    Vector<CandidatePair>   m_candidatePairs;
};

#endif // !BROAD_PHASE_SYSTEM_H_DEFINED
