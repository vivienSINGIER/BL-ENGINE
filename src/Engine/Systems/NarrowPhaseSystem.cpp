#include "NarrowPhaseSystem.h"
#include <cfloat>
#include "../ECS/World.h"
#include <Utils.hpp>
#include <iostream>
#include "../ECS/Script.h"

void NarrowPhaseSystem::Update(float _dt)
{
    m_cache.BeginFrame();
    if (m_broadPhase == nullptr)
        return;

    for (const CandidatePair& pair : m_broadPhase->GetCandidatePairs())
        ProcessPair(pair.a, pair.b);
}

bool NarrowPhaseSystem::ProcessPair(EntityId _a, EntityId _b)
{
    if (!world->HasComponent<ColliderComponent>(_a) || !world->HasComponent<ColliderComponent>(_b))
        return false;

    ColliderComponent&    shapeA = world->GetComponent<ColliderComponent>(_a);
    ColliderComponent&    shapeB = world->GetComponent<ColliderComponent>(_b);
    TransformComponent& transA = world->GetComponent<TransformComponent>(_a);
    TransformComponent& transB = world->GetComponent<TransformComponent>(_b);

    GJKSimplex simplex;
    if (!GJK(shapeA, transA, shapeB, transB, simplex))
        return false;

    XMFLOAT3 normal, contactA, contactB;
    float    penetration;

    if (!EPA(shapeA, transA, shapeB, transB, simplex, normal, penetration, contactA, contactB))
        return false;

    ContactManifold manifold;
    manifold.a           = _a;
    manifold.b           = _b;
    manifold.normal      = Snap(normal, 0.1f);
    manifold.penetration = penetration;
    manifold.isTrigger   = shapeA.isTrigger || shapeB.isTrigger;

    BuildManifold(manifold, shapeA, transA, shapeB, transB, normal, penetration, contactA, contactB);

    m_cache.AddManifold(manifold);
    shapeA.isTrigger ? world->NotifyScripts(_a, &IScript::OnTrigger, _b) : world->NotifyScripts(_a, &IScript::OnCollision, _b);
    shapeB.isTrigger ? world->NotifyScripts(_b, &IScript::OnTrigger, _a) : world->NotifyScripts(_b, &IScript::OnCollision, _a);
    
    return true;
}

XMFLOAT3 NarrowPhaseSystem::Support(ColliderComponent& _collider, TransformComponent& _transform, const XMFLOAT3& _dir) const
{
    switch (_collider.type)
    {
        case ShapeType::Box:     return SupportBox    (_collider, _transform, _dir);
        case ShapeType::Sphere:  return SupportSphere (_collider, _transform, _dir);
        case ShapeType::Capsule: return SupportCapsule(_collider, _transform, _dir);
    }
    return { 0,0,0 };
}

XMFLOAT3 NarrowPhaseSystem::SupportBox(ColliderComponent& _collider, TransformComponent& _transform, const XMFLOAT3& _dir) const
{
    // Transformer la direction en espace local pour éviter de tourner les 8 coins.
    // En espace local, le support d'un AABB centré en zéro est simplement
    // sign(localDir) * halfExtents.
    XMVECTOR q       = XMLoadFloat4(&_transform.world.GetRotation());
    XMMATRIX rotInv  = XMMatrixTranspose(XMMatrixRotationQuaternion(q));
    XMVECTOR dirW    = XMVectorSet(_dir.x, _dir.y, _dir.z, 0.0f);
    XMVECTOR dirL    = XMVector3Transform(dirW, rotInv);

    XMFLOAT3 localDir;
    XMStoreFloat3(&localDir, dirL);

    const XMFLOAT3& h     = _collider.shape.box.halfExtents;
    const XMFLOAT3& scale = _transform.world.GetScale();

    XMFLOAT3 localSupport =
    {
        (localDir.x >= 0.0f ? 1.0f : -1.0f) * h.x * scale.x,
        (localDir.y >= 0.0f ? 1.0f : -1.0f) * h.y * scale.y,
        (localDir.z >= 0.0f ? 1.0f : -1.0f) * h.z * scale.z
    };

    // Appliquer l'offset local.
    localSupport.x += _collider.localOffset.x;
    localSupport.y += _collider.localOffset.y;
    localSupport.z += _collider.localOffset.z;

    // Retransformer en espace monde.
    XMMATRIX rot = XMMatrixRotationQuaternion(q);
    XMVECTOR worldSup = XMVector3Transform(XMVectorSet(localSupport.x, localSupport.y, localSupport.z, 0.0f), rot);

    const XMFLOAT3& pos = _transform.world.GetPosition();
    XMFLOAT3 result;
    XMStoreFloat3(&result, worldSup);
    return { result.x + pos.x, result.y + pos.y, result.z + pos.z };
}

XMFLOAT3 NarrowPhaseSystem::SupportSphere(ColliderComponent& _collider, TransformComponent& _transform, const XMFLOAT3& _dir) const
{
    // Support d'une sphère : centre + rayon * normalize(direction).
    const XMFLOAT3& scale = _transform.world.GetScale();
    float maxScale        = Max(Max(scale.x, scale.y), scale.z);
    float worldRadius     = _collider.shape.sphere.radius * maxScale;

    XMFLOAT3 normDir = Normalize(_dir);
    const XMFLOAT3& pos = _transform.world.GetPosition();

    return
    {
        pos.x + _collider.localOffset.x + normDir.x * worldRadius,
        pos.y + _collider.localOffset.y + normDir.y * worldRadius,
        pos.z + _collider.localOffset.z + normDir.z * worldRadius
    };
}

XMFLOAT3 NarrowPhaseSystem::SupportCapsule(ColliderComponent& _collider, TransformComponent& _transform, const XMFLOAT3& _dir) const
{
    // La capsule est un segment + sphère.
    // Support = choisir l'extrémité du segment la plus dans la direction,
    //           puis ajouter rayon * normalize(direction).
    XMVECTOR q   = XMLoadFloat4(&_transform.world.GetRotation());
    XMMATRIX rot = XMMatrixRotationQuaternion(q);

    const XMFLOAT3& scale = _transform.world.GetScale();
    float r  = _collider.shape.capsule.radius     * Max(scale.x, scale.z);
    float hh = _collider.shape.capsule.halfHeight * scale.y;

    // Axe Y local tourné en espace monde.
    XMFLOAT3 worldUp;
    XMStoreFloat3(&worldUp, XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0,1,0,0), rot)));

    const XMFLOAT3& pos = _transform.world.GetPosition();

    // Les deux extrémités du segment central.
    XMFLOAT3 topPoint = { pos.x + worldUp.x*hh, pos.y + worldUp.y*hh, pos.z + worldUp.z*hh };
    XMFLOAT3 botPoint = { pos.x - worldUp.x*hh, pos.y - worldUp.y*hh, pos.z - worldUp.z*hh };

    // Choisir l'extrémité la plus dans la direction donnée.
    XMFLOAT3 bestPoint = (Dot(_dir, topPoint) >= Dot(_dir, botPoint)) ? topPoint : botPoint;

    // Ajouter le rayon dans la direction.
    XMFLOAT3 normDir = Normalize(_dir);
    return { bestPoint.x + normDir.x*r, bestPoint.y + normDir.y*r, bestPoint.z + normDir.z*r };
}

GJKSupportPoint NarrowPhaseSystem::MinkowskiSupport(
    ColliderComponent& _colliderA, TransformComponent& _transformA,
    ColliderComponent& _colliderB, TransformComponent& _transformB,
    const XMFLOAT3& _dir) const
{
    GJKSupportPoint sp;
    sp.pointA    = Support(_colliderA, _transformA, _dir);
    XMFLOAT3 negDir = { -_dir.x, -_dir.y, -_dir.z };
    sp.pointB    = Support(_colliderB, _transformB, negDir);
    sp.minkowski = Subtract(sp.pointA, sp.pointB);
    return sp;
}

bool NarrowPhaseSystem::GJK(
    ColliderComponent& _colliderA, TransformComponent& _transformA,
    ColliderComponent& _colliderB, TransformComponent& _transformB,
    GJKSimplex& _outSimplex)
{
    // Direction initiale : axe entre les centres.
    const XMFLOAT3& posA = _transformA.world.GetPosition();
    const XMFLOAT3& posB = _transformB.world.GetPosition();
    XMFLOAT3 direction   = Subtract(posA, posB);

    if (LengthSq(direction) < 1e-8f)
        direction = { 1, 0, 0 };

    GJKSupportPoint support = MinkowskiSupport(_colliderA, _transformA, _colliderB, _transformB, direction);
    _outSimplex.Push(support);

    // Nouvelle direction : vers l'origine depuis le premier point.
    direction = { -support.minkowski.x, -support.minkowski.y, -support.minkowski.z };

    for (int iter = 0; iter < kGJKMaxIterations; ++iter)
    {
        if (LengthSq(direction) < 1e-10f)
            return true;

        support = MinkowskiSupport(_colliderA, _transformA, _colliderB, _transformB, direction);

        // Si le nouveau point ne dépasse pas l'origine dans cette direction,
        // l'origine est hors de A⊖B : pas d'intersection.
        if (Dot(support.minkowski, direction) < 0.0f)
            return false;

        _outSimplex.Push(support);

        if (UpdateSimplex(_outSimplex, direction))
            return true;
    }

    return false;
}

bool NarrowPhaseSystem::UpdateSimplex(GJKSimplex& _simplex, XMFLOAT3& _direction)
{
    switch (_simplex.count)
    {
        case 2: return UpdateLine      (_simplex, _direction);
        case 3: return UpdateTriangle  (_simplex, _direction);
        case 4: return UpdateTetrahedron(_simplex, _direction);
    }
    return false;
}

bool NarrowPhaseSystem::UpdateLine(GJKSimplex& _simplex, XMFLOAT3& _direction)
{
    const XMFLOAT3& a = _simplex.points[0].minkowski;
    const XMFLOAT3& b = _simplex.points[1].minkowski;

    XMFLOAT3 ab = Subtract(b, a);
    XMFLOAT3 ao = { -a.x, -a.y, -a.z };

    if (Dot(ab, ao) > 0.0f)
        _direction = TripleProduct(ab, ao, ab);
    else
    {
        _simplex.count = 1;
        _direction = ao;
    }

    return false;
}

bool NarrowPhaseSystem::UpdateTriangle(GJKSimplex& _simplex, XMFLOAT3& _direction)
{
    const XMFLOAT3& a = _simplex.points[0].minkowski;
    const XMFLOAT3& b = _simplex.points[1].minkowski;
    const XMFLOAT3& c = _simplex.points[2].minkowski;

    XMFLOAT3 ab  = Subtract(b, a);
    XMFLOAT3 ac  = Subtract(c, a);
    XMFLOAT3 ao  = { -a.x, -a.y, -a.z };
    XMFLOAT3 abc = Cross(ab, ac);

    if (Dot(Cross(abc, ac), ao) > 0.0f)
    {
        if (Dot(ac, ao) > 0.0f)
        {
            // Garder a et c.
            _simplex.points[1] = _simplex.points[2];
            _simplex.count = 2;
            _direction = TripleProduct(ac, ao, ac);
        }
        else
        {
            _simplex.count = 2;
            return UpdateLine(_simplex, _direction);
        }
    }
    else if (Dot(Cross(ab, abc), ao) > 0.0f)
    {
        _simplex.count = 2;
        return UpdateLine(_simplex, _direction);
    }
    else
    {
        if (Dot(abc, ao) > 0.0f)
            _direction = abc;
        else
        {
            // Inverser le triangle.
            std::swap(_simplex.points[1], _simplex.points[2]);
            _direction = { -abc.x, -abc.y, -abc.z };
        }
    }

    return false;
}

bool NarrowPhaseSystem::UpdateTetrahedron(GJKSimplex& _simplex, XMFLOAT3& _direction)
{
    const XMFLOAT3& a = _simplex.points[0].minkowski;
    const XMFLOAT3& b = _simplex.points[1].minkowski;
    const XMFLOAT3& c = _simplex.points[2].minkowski;
    const XMFLOAT3& d = _simplex.points[3].minkowski;

    XMFLOAT3 ab  = Subtract(b, a);
    XMFLOAT3 ac  = Subtract(c, a);
    XMFLOAT3 ad  = Subtract(d, a);
    XMFLOAT3 ao  = { -a.x, -a.y, -a.z };

    XMFLOAT3 abc = Cross(ab, ac);
    XMFLOAT3 acd = Cross(ac, ad);
    XMFLOAT3 adb = Cross(ad, ab);

    // Tester les 3 faces visibles depuis l'origine.
    if (Dot(abc, ao) > 0.0f)
    {
        // Face ABC visible : garder ABC, chercher dans cette direction.
        _simplex.points[3] = _simplex.points[0];
        _simplex.points[0] = _simplex.points[0];
        _simplex.count = 3;
        return UpdateTriangle(_simplex, _direction);
    }
    if (Dot(acd, ao) > 0.0f)
    {
        _simplex.points[1] = _simplex.points[2];
        _simplex.points[2] = _simplex.points[3];
        _simplex.count = 3;
        return UpdateTriangle(_simplex, _direction);
    }
    if (Dot(adb, ao) > 0.0f)
    {
        _simplex.points[2] = _simplex.points[1];
        _simplex.points[1] = _simplex.points[3];
        _simplex.count = 3;
        return UpdateTriangle(_simplex, _direction);
    }

    // L'origine est à l'intérieur du tétraèdre.
    return true;
}

bool NarrowPhaseSystem::EPA(
    ColliderComponent& _colliderA, TransformComponent& _transformA,
    ColliderComponent& _colliderB, TransformComponent& _transformB,
    GJKSimplex& _simplex,
    XMFLOAT3& _outNormal, float& _outPenetration,
    XMFLOAT3& _outContactA, XMFLOAT3& _outContactB)
{
    Vector<EPAFace> faces;

    // S'assurer qu'on a un tétraèdre valide.
    if (_simplex.count < 4)
        return false;

    // Initialiser le polytope avec les 4 faces du tétraèdre GJK.
    faces.push_back(MakeFace(_simplex.points[0], _simplex.points[1], _simplex.points[2]));
    faces.push_back(MakeFace(_simplex.points[0], _simplex.points[1], _simplex.points[3]));
    faces.push_back(MakeFace(_simplex.points[0], _simplex.points[2], _simplex.points[3]));
    faces.push_back(MakeFace(_simplex.points[1], _simplex.points[2], _simplex.points[3]));

    for (int iter = 0; iter < kEPAMaxIterations; ++iter)
    {
        int closestIdx = FindClosestFace(faces);
        if (closestIdx < 0)
            return false;

        const EPAFace& closest = faces[closestIdx];

        // Chercher un point support dans la direction de la face la plus proche.
        GJKSupportPoint support = MinkowskiSupport(
            _colliderA, _transformA, _colliderB, _transformB, closest.normal);

        float newDist = Dot(support.minkowski, closest.normal);

        // Convergé si le nouveau point n'est pas significativement plus loin.
        if (newDist - closest.distance < kEPATolerance)
        {
            _outNormal      = closest.normal;
            _outPenetration = closest.distance;

            // Interpolation barycentrique pour retrouver les points de contact.
            // On utilise les coordonnées barycentriques de la projection de
            // l'origine sur la face du polytope.
            XMFLOAT3 p = Scale(closest.normal, closest.distance);

            XMFLOAT3 v0 = Subtract(closest.b.minkowski, closest.a.minkowski);
            XMFLOAT3 v1 = Subtract(closest.c.minkowski, closest.a.minkowski);
            XMFLOAT3 v2 = Subtract(p,                   closest.a.minkowski);

            float d00 = Dot(v0, v0);
            float d01 = Dot(v0, v1);
            float d11 = Dot(v1, v1);
            float d20 = Dot(v2, v0);
            float d21 = Dot(v2, v1);
            float denom = d00*d11 - d01*d01;

            if (fabsf(denom) < 1e-8f)
            {
                _outContactA = closest.a.pointA;
                _outContactB = closest.a.pointB;
            }
            else
            {
                float v = (d11*d20 - d01*d21) / denom;
                float w = (d00*d21 - d01*d20) / denom;
                float u = 1.0f - v - w;

                // Clamp pour robustesse numérique.
                u = Max(0.0f, Min(1.0f, u));
                v = Max(0.0f, Min(1.0f, v));
                w = Max(0.0f, Min(1.0f, w));
                float sum = u + v + w;
                if (sum > 0.0f) { u /= sum; v /= sum; w /= sum; }

                _outContactA =
                {
                    u*closest.a.pointA.x + v*closest.b.pointA.x + w*closest.c.pointA.x,
                    u*closest.a.pointA.y + v*closest.b.pointA.y + w*closest.c.pointA.y,
                    u*closest.a.pointA.z + v*closest.b.pointA.z + w*closest.c.pointA.z
                };
                _outContactB =
                {
                    u*closest.a.pointB.x + v*closest.b.pointB.x + w*closest.c.pointB.x,
                    u*closest.a.pointB.y + v*closest.b.pointB.y + w*closest.c.pointB.y,
                    u*closest.a.pointB.z + v*closest.b.pointB.z + w*closest.c.pointB.z
                };
            }

            return true;
        }

        Vector<std::pair<GJKSupportPoint, GJKSupportPoint>> edges;

        for (int i = (int)faces.size() - 1; i >= 0; --i)
        {
            const EPAFace& face = faces[i];
            if (Dot(Subtract(support.minkowski, face.a.minkowski), face.normal) > 0.0f)
            {
                edges.push_back({ face.a, face.b });
                edges.push_back({ face.b, face.c });
                edges.push_back({ face.c, face.a });
                faces.erase(faces.begin() + i);
            }
        }

        // Supprimer les arêtes en double (partagées par deux faces supprimées).
        for (int i = (int)edges.size() - 1; i >= 0; --i)
        {
            for (int j = (int)edges.size() - 1; j >= 0; --j)
            {
                if (i == j) continue;
                // Arête dupliquée si les deux extrémités sont inversées.
                XMFLOAT3 diffA = Subtract(edges[i].first.minkowski,  edges[j].second.minkowski);
                XMFLOAT3 diffB = Subtract(edges[i].second.minkowski, edges[j].first.minkowski);
                if (LengthSq(diffA) < 1e-8f && LengthSq(diffB) < 1e-8f)
                {
                    edges.erase(edges.begin() + max(i,j));
                    edges.erase(edges.begin() + min(i,j));
                    --i;
                    break;
                }
            }
        }

        // Ajouter les nouvelles faces connectées au point support.
        for (const auto& edge : edges)
            faces.push_back(MakeFace(support, edge.first, edge.second));
    }

    return false;
}

EPAFace NarrowPhaseSystem::MakeFace(const GJKSupportPoint& _a, const GJKSupportPoint& _b, const GJKSupportPoint& _c) const
{
    EPAFace face;
    face.a = _a;
    face.b = _b;
    face.c = _c;

    XMFLOAT3 ab = Subtract(_b.minkowski, _a.minkowski);
    XMFLOAT3 ac = Subtract(_c.minkowski, _a.minkowski);
    face.normal  = Normalize(Cross(ab, ac));
    face.distance = Dot(face.normal, _a.minkowski);

    // S'assurer que la normale pointe vers l'extérieur (loin de l'origine).
    if (face.distance < 0.0f)
    {
        face.normal   = { -face.normal.x, -face.normal.y, -face.normal.z };
        face.distance = -face.distance;
        std::swap(face.b, face.c);
    }

    return face;
}

int NarrowPhaseSystem::FindClosestFace(const Vector<EPAFace>& _faces) const
{
    if (_faces.empty())
        return -1;

    int   bestIdx  = 0;
    float bestDist = _faces[0].distance;

    for (int i = 1; i < (int)_faces.size(); ++i)
    {
        if (_faces[i].distance < bestDist)
        {
            bestDist = _faces[i].distance;
            bestIdx  = i;
        }
    }

    return bestIdx;
}

void NarrowPhaseSystem::BuildManifold(ContactManifold& _manifold,
    ColliderComponent& _colliderA, TransformComponent& _transformA,
    ColliderComponent& _colliderB, TransformComponent& _transformB,
    const XMFLOAT3& _normal, float _penetration,
    const XMFLOAT3& _contactA, const XMFLOAT3& _contactB)
{
    // Point de contact monde = milieu des points de contact sur chaque forme.
    XMFLOAT3 contactWorld =
    {
        (_contactA.x + _contactB.x) * 0.5f,
        (_contactA.y + _contactB.y) * 0.5f,
        (_contactA.z + _contactB.z) * 0.5f
    };

    ContactPoint cp;
    cp.position = contactWorld;
    cp.localPointA = _contactA;
    cp.localPointB = _contactB;
    cp.penetration = _penetration;
    _manifold.points[0] = cp;
    _manifold.pointCount = 1;

    // Pour box-box, tenter de générer un manifold complet par clipping.
    if (_colliderA.type == ShapeType::Box && _colliderB.type == ShapeType::Box)
    {
        // Face de référence : la face de A la plus antiparallèle à la normale.
        XMVECTOR qA  = XMLoadFloat4(&_transformA.world.GetRotation());
        XMMATRIX rotA = XMMatrixRotationQuaternion(qA);
        const XMFLOAT3& scaleA = _transformA.world.GetScale();
        const XMFLOAT3& hA     = _colliderA.shape.box.halfExtents;

        XMFLOAT3 axesA[3];
        XMStoreFloat3(&axesA[0], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(1,0,0,0), rotA)));
        XMStoreFloat3(&axesA[1], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0,1,0,0), rotA)));
        XMStoreFloat3(&axesA[2], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0,0,1,0), rotA)));

        int    refAxis = 0;
        float  refDot  = fabsf(Dot(_normal, axesA[0]));
        for (int i = 1; i < 3; ++i)
        {
            float d = fabsf(Dot(_normal, axesA[i]));
            if (d > refDot) { refDot = d; refAxis = i; }
        }

        // Face de référence centrée autour du point support.
        bool positiveFace = Dot(_normal, axesA[refAxis]) > 0.0f;
        float hRef = (refAxis==0 ? hA.x*scaleA.x : refAxis==1 ? hA.y*scaleA.y : hA.z*scaleA.z);
        const XMFLOAT3& posA = _transformA.world.GetPosition();
        float sign = positiveFace ? 1.0f : -1.0f;

        XMFLOAT3 refCenter =
        {
            posA.x + axesA[refAxis].x * hRef * sign,
            posA.y + axesA[refAxis].y * hRef * sign,
            posA.z + axesA[refAxis].z * hRef * sign
        };

        // Construire les 4 coins de la face de référence.
        int ax1 = (refAxis + 1) % 3;
        int ax2 = (refAxis + 2) % 3;
        float h1 = (ax1==0 ? hA.x*scaleA.x : ax1==1 ? hA.y*scaleA.y : hA.z*scaleA.z);
        float h2 = (ax2==0 ? hA.x*scaleA.x : ax2==1 ? hA.y*scaleA.y : hA.z*scaleA.z);

        XMFLOAT3 refFace[4] =
        {
            Add(Add(refCenter, Scale(axesA[ax1], h1)), Scale(axesA[ax2], h2)),
            Add(Subtract(refCenter, Scale(axesA[ax1], h1)), Scale(axesA[ax2], h2)),
            Subtract(Subtract(refCenter, Scale(axesA[ax1], h1)), Scale(axesA[ax2], h2)),
            Add(Subtract(refCenter, Scale(axesA[ax2], h2)), Scale(axesA[ax1], h1))
        };

        // Face incidente de B.
        XMVECTOR qB  = XMLoadFloat4(&_transformB.world.GetRotation());
        XMMATRIX rotB = XMMatrixRotationQuaternion(qB);
        const XMFLOAT3& scaleB = _transformB.world.GetScale();
        const XMFLOAT3& hB     = _colliderB.shape.box.halfExtents;

        XMFLOAT3 axesB[3];
        XMStoreFloat3(&axesB[0], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(1,0,0,0), rotB)));
        XMStoreFloat3(&axesB[1], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0,1,0,0), rotB)));
        XMStoreFloat3(&axesB[2], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0,0,1,0), rotB)));

        int    incAxis = 0;
        float  incDot  = fabsf(Dot(_normal, axesB[0]));
        for (int i = 1; i < 3; ++i)
        {
            float d = fabsf(Dot(_normal, axesB[i]));
            if (d > incDot) { incDot = d; incAxis = i; }
        }

        bool positiveFaceB = Dot(_normal, axesB[incAxis]) < 0.0f;
        float hIncRef = (incAxis==0 ? hB.x*scaleB.x : incAxis==1 ? hB.y*scaleB.y : hB.z*scaleB.z);
        const XMFLOAT3& posB2 = _transformB.world.GetPosition();
        float signB = positiveFaceB ? 1.0f : -1.0f;

        XMFLOAT3 incCenter =
        {
            posB2.x + axesB[incAxis].x * hIncRef * signB,
            posB2.y + axesB[incAxis].y * hIncRef * signB,
            posB2.z + axesB[incAxis].z * hIncRef * signB
        };

        int ix1 = (incAxis + 1) % 3;
        int ix2 = (incAxis + 2) % 3;
        float ih1 = (ix1==0 ? hB.x*scaleB.x : ix1==1 ? hB.y*scaleB.y : hB.z*scaleB.z);
        float ih2 = (ix2==0 ? hB.x*scaleB.x : ix2==1 ? hB.y*scaleB.y : hB.z*scaleB.z);

        XMFLOAT3 incFace[4] =
        {
            Add(Add(incCenter, Scale(axesB[ix1], ih1)), Scale(axesB[ix2], ih2)),
            Add(Subtract(incCenter, Scale(axesB[ix1], ih1)), Scale(axesB[ix2], ih2)),
            Subtract(Subtract(incCenter, Scale(axesB[ix1], ih1)), Scale(axesB[ix2], ih2)),
            Add(Subtract(incCenter, Scale(axesB[ix2], ih2)), Scale(axesB[ix1], ih1))
        };

        // Clipping de la face incidente par les plans latéraux de la face de référence.
        XMFLOAT3 clip0[8], clip1[8], clip2[8], clip3[8];
        for (int i = 0; i < 4; ++i) clip0[i] = incFace[i];
        int count = 4;

        // Plan -ax1 (bord gauche de la face de référence)
        count = ClipPolygonAgainstPlane(clip0, count, clip1,
            Subtract(refCenter, Scale(axesA[ax1], h1)),   // point sur le plan
            axesA[ax1]);                                   // normale : garder côté +ax1
        if (count <= 0) return;

        // Plan +ax1 (bord droit)
        count = ClipPolygonAgainstPlane(clip1, count, clip2,
            Add(refCenter, Scale(axesA[ax1], h1)),
            { -axesA[ax1].x, -axesA[ax1].y, -axesA[ax1].z });
        if (count <= 0) return;

        // Plan -ax2 (bord bas)
        count = ClipPolygonAgainstPlane(clip2, count, clip3,
            Subtract(refCenter, Scale(axesA[ax2], h2)),
            axesA[ax2]);
        if (count <= 0) return;

        // Plan +ax2 (bord haut)
        XMFLOAT3 clip4[8];
        count = ClipPolygonAgainstPlane(clip3, count, clip4,
            Add(refCenter, Scale(axesA[ax2], h2)),
            { -axesA[ax2].x, -axesA[ax2].y, -axesA[ax2].z });
        if (count <= 0) return;

        // Garder uniquement les points sous le plan de la face de référence.
        XMFLOAT3 facePlaneNormal = positiveFace ? axesA[refAxis] : XMFLOAT3{ -axesA[refAxis].x, -axesA[refAxis].y, -axesA[refAxis].z };

        _manifold.pointCount = 0;
        for (int i = 0; i < count && _manifold.pointCount < ContactManifold::kMaxPoints; ++i)
        {
            float sep = Dot(Subtract(clip4[i], refCenter), facePlaneNormal);
            if (sep <= 0.01f)
            {
                // Projeter le point sur le plan de la face de référence.
                // Cela garantit que tous les points sont coplanaires — indispensable
                // pour que le solver applique des impulsions uniformes sur la face.
                XMFLOAT3 projected =
                {
                    clip4[i].x - facePlaneNormal.x * sep,
                    clip4[i].y - facePlaneNormal.y * sep,
                    clip4[i].z - facePlaneNormal.z * sep
                };

                ContactPoint newCp;
                newCp.position = projected;
                newCp.localPointA = projected;
                newCp.localPointB = projected;
                newCp.penetration = Max(0.0f, -sep);
                _manifold.points[_manifold.pointCount++] = newCp;
            }
        }

        if (_manifold.pointCount == 0)
        {
            _manifold.points[0] = cp;
            _manifold.pointCount = 1;
        }
    }
}

int NarrowPhaseSystem::ClipPolygonAgainstPlane(
    const XMFLOAT3* _in, int _inCount, XMFLOAT3* _out,
    const XMFLOAT3& _planePoint, const XMFLOAT3& _planeNormal) const
{
    int outCount = 0;

    for (int i = 0; i < _inCount; ++i)
    {
        const XMFLOAT3& curr = _in[i];
        const XMFLOAT3& next = _in[(i + 1) % _inCount];

        float dCurr = Dot(Subtract(curr, _planePoint), _planeNormal);
        float dNext = Dot(Subtract(next, _planePoint), _planeNormal);

        if (dCurr >= 0.0f)
            _out[outCount++] = curr;

        if ((dCurr >= 0.0f) != (dNext >= 0.0f))
        {
            float t = dCurr / (dCurr - dNext);
            _out[outCount++] =
            {
                curr.x + t * (next.x - curr.x),
                curr.y + t * (next.y - curr.y),
                curr.z + t * (next.z - curr.z)
            };
        }
    }

    return outCount;
}
