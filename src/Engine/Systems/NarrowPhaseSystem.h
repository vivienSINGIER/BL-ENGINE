#ifndef NARROW_PHASE_SYSTEM_H_DEFINED
#define NARROW_PHASE_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/ColliderComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "../ContactManifold.hpp"
#include "BroadPhaseSystem.h"

struct GJKSupportPoint
{
    XMFLOAT3 pointA;    // point le plus loin sur A dans la direction d
    XMFLOAT3 pointB;    // point le plus loin sur B dans la direction -d
    XMFLOAT3 minkowski; // pointA - pointB (point sur la différence de Minkowski)
};

// Simplexe GJK : 1 à 4 points en 3D.
struct GJKSimplex
{
    GJKSupportPoint points[4];
    int count = 0;

    void Push(const GJKSupportPoint& _p)
    {
        // Décale pour garder les nouveaux points en tête.
        for (int i = min(count, 3); i > 0; --i)
            points[i] = points[i-1];
        points[0] = _p;
        count = min(count + 1, 4);
    }
};

// Face du polytope EPA.
struct EPAFace
{
    GJKSupportPoint a, b, c;
    XMFLOAT3 normal;    // normale sortante (pointe vers l'extérieur du polytope)
    float    distance;  // distance de l'origine à la face
};

class NarrowPhaseSystem : public System<ColliderComponent, TransformComponent>
{
public:
    void Update(float _dt) override;

    void SetBroadPhaseSystem(BroadPhaseSystem* _bp) { m_broadPhase = _bp; }

    ContactManifoldCache& GetManifoldCache() { return m_cache; }

private:

    bool ProcessPair(EntityId _a, EntityId _b);

    XMFLOAT3 Support(ColliderComponent& _collider, TransformComponent& _transform, const XMFLOAT3& _dir) const;

    XMFLOAT3 SupportBox    (ColliderComponent& _collider, TransformComponent& _transform, const XMFLOAT3& _dir) const;
    XMFLOAT3 SupportSphere (ColliderComponent& _collider, TransformComponent& _transform, const XMFLOAT3& _dir) const;
    XMFLOAT3 SupportCapsule(ColliderComponent& _collider, TransformComponent& _transform, const XMFLOAT3& _dir) const;

    // Support de la différence de Minkowski A ⊖ B.
    GJKSupportPoint MinkowskiSupport(
        ColliderComponent& _colliderA, TransformComponent& _transformA,
        ColliderComponent& _colliderB, TransformComponent& _transformB,
        const XMFLOAT3& _dir) const;

    bool GJK (ColliderComponent& _colliderA,  TransformComponent& _transformA, ColliderComponent& _colliderB, TransformComponent& _transformB, GJKSimplex& _outSimplex);

    // Met à jour le simplexe et la direction de recherche.
    // Retourne true si l'origine est dans le simplexe (intersection).
    bool UpdateSimplex(GJKSimplex& _simplex, XMFLOAT3& _direction);
    bool UpdateLine     (GJKSimplex& _simplex, XMFLOAT3& _direction);
    bool UpdateTriangle (GJKSimplex& _simplex, XMFLOAT3& _direction);
    bool UpdateTetrahedron(GJKSimplex& _simplex, XMFLOAT3& _direction);

    bool EPA(ColliderComponent& _colliderA, TransformComponent& _transformA,
             ColliderComponent& _colliderB, TransformComponent& _transformB,
             GJKSimplex& _simplex,
             XMFLOAT3& _outNormal, float& _outPenetration,
             XMFLOAT3& _outContactA, XMFLOAT3& _outContactB);

    EPAFace MakeFace(const GJKSupportPoint& _a, const GJKSupportPoint& _b,
                     const GJKSupportPoint& _c) const;

    int FindClosestFace(const Vector<EPAFace>& _faces) const;

    // Génère jusqu'à 4 points de contact depuis la normale EPA.
    void BuildManifold(ContactManifold& _manifold,
                       ColliderComponent& _colliderA, TransformComponent& _transformA,
                       ColliderComponent& _colliderB, TransformComponent& _transformB,
                       const XMFLOAT3& _normal, float _penetration,
                       const XMFLOAT3& _contactA, const XMFLOAT3& _contactB);

    // Clipping Sutherland-Hodgman pour la réduction box-box.
    int ClipPolygonAgainstPlane(const XMFLOAT3* _in, int _inCount, XMFLOAT3* _out,
                                const XMFLOAT3& _planePoint, const XMFLOAT3& _planeNormal) const;

    inline XMFLOAT3 Cross(const XMFLOAT3& _a, const XMFLOAT3& _b) const
    {
        return { _a.y*_b.z - _a.z*_b.y,
                 _a.z*_b.x - _a.x*_b.z,
                 _a.x*_b.y - _a.y*_b.x };
    }

    inline float Dot(const XMFLOAT3& _a, const XMFLOAT3& _b) const
    {
        return _a.x*_b.x + _a.y*_b.y + _a.z*_b.z;
    }

    inline XMFLOAT3 Subtract(const XMFLOAT3& _a, const XMFLOAT3& _b) const
    {
        return { _a.x-_b.x, _a.y-_b.y, _a.z-_b.z };
    }

    inline XMFLOAT3 Add(const XMFLOAT3& _a, const XMFLOAT3& _b) const
    {
        return { _a.x+_b.x, _a.y+_b.y, _a.z+_b.z };
    }

    inline XMFLOAT3 Scale(const XMFLOAT3& _v, float _s) const
    {
        return { _v.x*_s, _v.y*_s, _v.z*_s };
    }

    inline float LengthSq(const XMFLOAT3& _v) const
    {
        return _v.x*_v.x + _v.y*_v.y + _v.z*_v.z;
    }

    inline XMFLOAT3 Normalize(const XMFLOAT3& _v) const
    {
        float len = sqrtf(LengthSq(_v));
        if (len < 1e-8f) return { 0,1,0 };
        float inv = 1.0f / len;
        return { _v.x*inv, _v.y*inv, _v.z*inv };
    }

    // Triple produit vectoriel : (A × B) × C
    inline XMFLOAT3 TripleProduct(const XMFLOAT3& _a, const XMFLOAT3& _b, const XMFLOAT3& _c) const
    {
        return Cross(Cross(_a, _b), _c);
    }

private:
    BroadPhaseSystem*    m_broadPhase = nullptr;
    ContactManifoldCache m_cache;

    static constexpr int   kGJKMaxIterations = 64;
    static constexpr int   kEPAMaxIterations = 64;
    static constexpr float kEPATolerance     = 0.1f;
};

#endif // !NARROW_PHASE_SYSTEM_H_DEFINED
