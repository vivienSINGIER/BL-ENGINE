#ifndef NARROW_PHASE_SYSTEM_H_DEFINED
#define NARROW_PHASE_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/ColliderComponent.hpp"
#include "../Core/Utils.hpp"
#include "BroadPhaseSystem.h"

static constexpr float kOBBEpsilon = 1e-5f;
static constexpr float kContactMergeEpsilon = 1e-4f;
static constexpr int   kMaxContactPoints = 4;

struct NarrowSphere
{
    XMFLOAT3 center;
    float radius;
};

struct NarrowOBB
{
    XMFLOAT3 c;      // centre monde
    XMFLOAT3 u[3];   // axes unitaires monde
    XMFLOAT3 e;      // half extents monde
};

struct ContactPoint
{
    XMFLOAT3 position = { 0, 0, 0 };
    float penetration = 0.f;
};

struct ContactInfo
{
    bool hit = false;
    XMFLOAT3 normal = { 0, 1, 0 }; // convention : de B vers A
    int pointCount = 0;
    ContactPoint points[kMaxContactPoints];
};

struct CollisionResult
{
    EntityId entityA;
    EntityId entityB;
    ContactInfo contact;
};

enum class SeparatingAxisType
{
    FaceA,
    FaceB,
    EdgeEdge
};

struct SATResult
{
    bool hit = false;

    SeparatingAxisType axisType = SeparatingAxisType::FaceA;
    int axisIndexA = -1;
    int axisIndexB = -1;

    XMFLOAT3 normal = { 0, 1, 0 }; // de B vers A
    float penetration = 0.f;

    // Meilleur axe de face conservé pour fallback si EdgeEdge est invalide.
    SeparatingAxisType bestFaceType = SeparatingAxisType::FaceA;
    int bestFaceAxisA = -1;
    int bestFaceAxisB = -1;
    XMFLOAT3 bestFaceNormal = { 0, 1, 0 };
    float bestFacePenetration = 0.f;
};

struct FaceQuad
{
    XMFLOAT3 normal = { 0, 1, 0 };
    XMFLOAT3 center = { 0, 0, 0 };

    XMFLOAT3 axis1 = { 1, 0, 0 };
    XMFLOAT3 axis2 = { 0, 0, 1 };

    float extent1 = 0.f;
    float extent2 = 0.f;

    XMFLOAT3 vertices[4];
};

class NarrowPhaseSystem : public System<>
{
public:
    void Update(float _dt) override;

    void SetBroadPhaseSystem(BroadPhaseSystem* _bp) { m_broadPhase = _bp; }

    const Vector<CollisionResult>& GetResults() const { return m_results; }
    const Vector<CollisionResult>& GetTriggerResults() const { return m_triggerResults; }

private:
    // Build narrow-phase primitives from colliders.
    NarrowSphere BuildSphere(const ColliderComponent& _collider) const;
    NarrowOBB BuildOBB(const ColliderComponent& _collider) const;

    // Primitive tests.
    ContactInfo TestSphereSphere(const NarrowSphere& _a, const NarrowSphere& _b) const;
    ContactInfo TestSphereOBB(const NarrowSphere& _sphere, const NarrowOBB& _box) const;
    ContactInfo TestOBBOBB(const NarrowOBB& _a, const NarrowOBB& _b) const;

    // OBB helpers.
    XMFLOAT3 ClosestPtPointOBB(const XMFLOAT3& _p, const NarrowOBB& _b) const;
    SATResult ComputeSATOBBOBB(const NarrowOBB& _a, const NarrowOBB& _b) const;

    ContactInfo BuildFaceFaceManifold(const NarrowOBB& _referenceBox,
        const NarrowOBB& _incidentBox,
        int _referenceAxis,
        const XMFLOAT3& _normalBtoA,
        float _penetration) const;

    ContactInfo BuildEdgeEdgeManifold(const NarrowOBB& _a,
        const NarrowOBB& _b,
        const SATResult& _sat) const;

    // Collider dispatch.
    ContactInfo Dispatch(ColliderComponent& _a, ColliderComponent& _b) const;
    ContactInfo DispatchSphereSphere(ColliderComponent& _a, ColliderComponent& _b) const;
    ContactInfo DispatchSphereOBB(ColliderComponent& _sphere, ColliderComponent& _box) const;
    ContactInfo DispatchOBBOBB(ColliderComponent& _a, ColliderComponent& _b) const;

private:
    BroadPhaseSystem* m_broadPhase = nullptr;

    Vector<CollisionResult> m_results;
    Vector<CollisionResult> m_triggerResults;
};

#endif
