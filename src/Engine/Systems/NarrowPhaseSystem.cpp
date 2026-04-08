#include "NarrowPhaseSystem.h"
#include "../ECS/World.h"
#include <algorithm>
#include <cfloat>

inline bool NearlyEqual(const XMFLOAT3& a, const XMFLOAT3& b, float eps = kContactMergeEpsilon)
{
    XMFLOAT3 d = Subtract(a, b);
    return LengthSq(d) <= eps * eps;
}

struct Plane
{
    XMFLOAT3 n;
    float d; // intérieur si dot(n, p) - d <= 0
};

struct Segment
{
    XMFLOAT3 a;
    XMFLOAT3 b;
};

static const int kEdgeAxisA[9] = { 0,0,0, 1,1,1, 2,2,2 };
static const int kEdgeAxisB[9] = { 0,1,2, 0,1,2, 0,1,2 };

static constexpr float kEdgePreferenceEpsilon = 0.02f;
static constexpr float kEdgeValidationTolerance = 0.1f;

inline bool IsPointNearOBB(const XMFLOAT3& p, const NarrowOBB& box, float tolerance)
{
    XMFLOAT3 d = Subtract(p, box.c);

    float lx = Dot(d, box.u[0]);
    float ly = Dot(d, box.u[1]);
    float lz = Dot(d, box.u[2]);

    return
        fabsf(lx) <= box.e.x + tolerance &&
        fabsf(ly) <= box.e.y + tolerance &&
        fabsf(lz) <= box.e.z + tolerance;
}

inline int TangentAxis0(int normalAxis)
{
    return (normalAxis == 0) ? 1 : 0;
}

inline int TangentAxis1(int normalAxis)
{
    return (normalAxis == 2) ? 1 : 2;
}

int ClipPolygonAgainstPlane(const XMFLOAT3* input, int inputCount, XMFLOAT3* output, const Plane& plane)
{
    if (inputCount <= 0)
        return 0;

    int outCount = 0;

    XMFLOAT3 prev = input[inputCount - 1];
    float prevDist = Dot(plane.n, prev) - plane.d;

    for (int i = 0; i < inputCount; ++i)
    {
        XMFLOAT3 curr = input[i];
        float currDist = Dot(plane.n, curr) - plane.d;

        bool prevInside = (prevDist <= 0.f);
        bool currInside = (currDist <= 0.f);

        if (prevInside && currInside)
        {
            output[outCount++] = curr;
        }
        else if (prevInside && !currInside)
        {
            float t = prevDist / (prevDist - currDist);
            output[outCount++] =
            {
                prev.x + (curr.x - prev.x) * t,
                prev.y + (curr.y - prev.y) * t,
                prev.z + (curr.z - prev.z) * t
            };
        }
        else if (!prevInside && currInside)
        {
            float t = prevDist / (prevDist - currDist);
            output[outCount++] =
            {
                prev.x + (curr.x - prev.x) * t,
                prev.y + (curr.y - prev.y) * t,
                prev.z + (curr.z - prev.z) * t
            };
            output[outCount++] = curr;
        }

        prev = curr;
        prevDist = currDist;
    }

    return outCount;
}

void AddContactPointUnique(ContactInfo& manifold, const XMFLOAT3& p, float penetration)
{
    for (int i = 0; i < manifold.pointCount; ++i)
    {
        if (NearlyEqual(manifold.points[i].position, p))
        {
            manifold.points[i].penetration = Max(manifold.points[i].penetration, penetration);
            return;
        }
    }

    if (manifold.pointCount < kMaxContactPoints)
    {
        manifold.points[manifold.pointCount].position = p;
        manifold.points[manifold.pointCount].penetration = penetration;
        ++manifold.pointCount;
        return;
    }

    int shallowest = 0;
    for (int i = 1; i < manifold.pointCount; ++i)
    {
        if (manifold.points[i].penetration < manifold.points[shallowest].penetration)
            shallowest = i;
    }

    if (penetration > manifold.points[shallowest].penetration)
    {
        manifold.points[shallowest].position = p;
        manifold.points[shallowest].penetration = penetration;
    }
}

FaceQuad BuildFaceQuad(const NarrowOBB& box, int faceAxis, float sign)
{
    FaceQuad q;

    const int t0 = TangentAxis0(faceAxis);
    const int t1 = TangentAxis1(faceAxis);

    const float faceOffset = sign * GetComponent(box.e, faceAxis);

    q.normal = Mul(box.u[faceAxis], sign);
    q.center = Add(box.c, Mul(box.u[faceAxis], faceOffset));

    q.axis1 = box.u[t0];
    q.axis2 = box.u[t1];

    q.extent1 = GetComponent(box.e, t0);
    q.extent2 = GetComponent(box.e, t1);

    XMFLOAT3 e1 = Mul(q.axis1, q.extent1);
    XMFLOAT3 e2 = Mul(q.axis2, q.extent2);

    q.vertices[0] = Add(Add(q.center, e1), e2);
    q.vertices[1] = Add(Subtract(q.center, e1), e2);
    q.vertices[2] = Subtract(Subtract(q.center, e1), e2);
    q.vertices[3] = Add(Subtract(q.center, e2), e1);

    return q;
}

int FindIncidentFaceAxis(const NarrowOBB& incidentBox, const XMFLOAT3& referenceNormal)
{
    float bestAbsDot = -FLT_MAX;
    int axis = 0;

    for (int i = 0; i < 3; ++i)
    {
        float d = fabsf(Dot(referenceNormal, incidentBox.u[i]));
        if (d > bestAbsDot)
        {
            bestAbsDot = d;
            axis = i;
        }
    }
    return axis;
}

float FindIncidentFaceSign(const NarrowOBB& incidentBox, int axis, const XMFLOAT3& referenceNormal)
{
    float d = Dot(referenceNormal, incidentBox.u[axis]);
    return (d >= 0.f) ? -1.f : 1.f;
}

Segment GetSupportEdge(const NarrowOBB& box, int edgeAxis, const XMFLOAT3& direction)
{
    Segment seg;

    XMFLOAT3 center = box.c;

    for (int i = 0; i < 3; ++i)
    {
        if (i == edgeAxis)
            continue;

        float sign = (Dot(direction, box.u[i]) >= 0.f) ? 1.f : -1.f;
        center = Add(center, Mul(box.u[i], sign * GetComponent(box.e, i)));
    }

    float extent = GetComponent(box.e, edgeAxis);
    XMFLOAT3 along = Mul(box.u[edgeAxis], extent);

    seg.a = Subtract(center, along);
    seg.b = Add(center, along);
    return seg;
}

void ClosestPtSegmentSegment(const XMFLOAT3& p1, const XMFLOAT3& q1, const XMFLOAT3& p2, const XMFLOAT3& q2, 
    float& s, float& t, XMFLOAT3& c1, XMFLOAT3& c2)
{
    XMFLOAT3 d1 = Subtract(q1, p1);
    XMFLOAT3 d2 = Subtract(q2, p2);
    XMFLOAT3 r = Subtract(p1, p2);

    float a = Dot(d1, d1);
    float e = Dot(d2, d2);
    float f = Dot(d2, r);

    if (a <= 1e-8f && e <= 1e-8f)
    {
        s = t = 0.f;
        c1 = p1;
        c2 = p2;
        return;
    }

    if (a <= 1e-8f)
    {
        s = 0.f;
        t = std::clamp(f / e, 0.f, 1.f);
    }
    else
    {
        float c = Dot(d1, r);

        if (e <= 1e-8f)
        {
            t = 0.f;
            s = std::clamp(-c / a, 0.f, 1.f);
        }
        else
        {
            float b = Dot(d1, d2);
            float denom = a * e - b * b;

            if (denom != 0.f)
                s = std::clamp((b * f - c * e) / denom, 0.f, 1.f);
            else
                s = 0.f;

            t = (b * s + f) / e;

            if (t < 0.f)
            {
                t = 0.f;
                s = std::clamp(-c / a, 0.f, 1.f);
            }
            else if (t > 1.f)
            {
                t = 1.f;
                s = std::clamp((b - c) / a, 0.f, 1.f);
            }
        }
    }

    c1 = Add(p1, Mul(d1, s));
    c2 = Add(p2, Mul(d2, t));
}

void NarrowPhaseSystem::Update(float _dt)
{
    m_results.clear();
    m_triggerResults.clear();
    
    if (m_broadPhase == nullptr)
        return;

    const Vector<CandidatePair>& pairs = m_broadPhase->GetCandidatePairs();

    for (const CandidatePair& pair : pairs)
    {
        if (!world->HasComponent<ColliderComponent>(pair.a) ||
            !world->HasComponent<ColliderComponent>(pair.b))
            continue;

        ColliderComponent& colA = world->GetComponent<ColliderComponent>(pair.a);
        ColliderComponent& colB = world->GetComponent<ColliderComponent>(pair.b);

        ContactInfo contact = Dispatch(colA, colB);

        if (contact.hit == false)
            continue;

        CollisionResult result{ pair.a, pair.b, contact };

        if (colA.isTrigger || colB.isTrigger)
            m_triggerResults.push_back(result);
        else
            m_results.push_back(result);
    }
}

NarrowSphere NarrowPhaseSystem::BuildSphere(const ColliderComponent& _c)
{
    return NarrowSphere{ _c.worldCenter, _c.worldRadius };
}

NarrowOBB NarrowPhaseSystem::BuildOBB(const ColliderComponent& _c)
{
    NarrowOBB obb;
    obb.c = _c.worldCenter;
    obb.u[0] = NormalizeSafe(_c.worldAxes[0], { 1.f, 0.f, 0.f });
    obb.u[1] = NormalizeSafe(_c.worldAxes[1], { 0.f, 1.f, 0.f });
    obb.u[2] = NormalizeSafe(_c.worldAxes[2], { 0.f, 0.f, 1.f });
    obb.e = _c.worldHalfExtents;
    return obb;
}

XMFLOAT3 NarrowPhaseSystem::ClosestPtPointOBB(const XMFLOAT3& _p, const NarrowOBB& _b)
{
    // d = vecteur du centre de la boîte vers P
    XMFLOAT3 d = Subtract(_p, _b.c);

    // On part du centre et on "avance" le long de chaque axe
    XMFLOAT3 q = _b.c;

    for (int i = 0; i < 3; ++i)
    {
        // Projection de d sur l'axe i
        float dist = Dot(d, _b.u[i]);

        // Clamp à l'extent
        const float ei = (i == 0) ? _b.e.x : (i == 1) ? _b.e.y : _b.e.z;
        if (dist > ei) dist = ei;
        if (dist < -ei) dist = -ei;

        // Accumulation dans le repère monde
        q.x += dist * _b.u[i].x;
        q.y += dist * _b.u[i].y;
        q.z += dist * _b.u[i].z;
    }

    return q;
}

SATResult NarrowPhaseSystem::ComputeSATOBBOBB(const NarrowOBB& _a, const NarrowOBB& _b)
{
    float R[3][3];
    float AbsR[3][3];

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            R[i][j] = Dot(_a.u[i], _b.u[j]);
            AbsR[i][j] = fabsf(R[i][j]) + kOBBEpsilon;
        }
    }

    XMFLOAT3 tWorld = Subtract(_b.c, _a.c);
    float t[3] =
    {
        Dot(tWorld, _a.u[0]),
        Dot(tWorld, _a.u[1]),
        Dot(tWorld, _a.u[2])
    };

    const float eA[3] = { _a.e.x, _a.e.y, _a.e.z };
    const float eB[3] = { _b.e.x, _b.e.y, _b.e.z };

    float ra, rb, tp, pen;

    float bestFacePen = FLT_MAX;
    SeparatingAxisType bestFaceType = SeparatingAxisType::FaceA;
    int bestFaceAxisA = -1;
    int bestFaceAxisB = -1;
    XMFLOAT3 bestFaceNormal = { 0,1,0 };

    float bestEdgePen = FLT_MAX;
    int bestEdgeAxisA = -1;
    int bestEdgeAxisB = -1;
    XMFLOAT3 bestEdgeNormal = { 0,1,0 };

    // Axes de face de A
    for (int i = 0; i < 3; ++i)
    {
        ra = eA[i];
        rb = eB[0] * AbsR[i][0] + eB[1] * AbsR[i][1] + eB[2] * AbsR[i][2];
        pen = (ra + rb) - fabsf(t[i]);

        if (pen < 0.f)
            return SATResult{};

        if (pen < bestFacePen)
        {
            bestFacePen = pen;
            bestFaceType = SeparatingAxisType::FaceA;
            bestFaceAxisA = i;
            bestFaceAxisB = -1;

            float sign = (t[i] >= 0.f) ? -1.f : 1.f;
            bestFaceNormal = Mul(_a.u[i], sign); // de B vers A
        }
    }

    // Axes de face de B
    for (int i = 0; i < 3; ++i)
    {
        ra = eA[0] * AbsR[0][i] + eA[1] * AbsR[1][i] + eA[2] * AbsR[2][i];
        rb = eB[i];
        tp = t[0] * R[0][i] + t[1] * R[1][i] + t[2] * R[2][i];
        pen = (ra + rb) - fabsf(tp);

        if (pen < 0.f)
            return SATResult{};

        if (pen < bestFacePen)
        {
            bestFacePen = pen;
            bestFaceType = SeparatingAxisType::FaceB;
            bestFaceAxisA = -1;
            bestFaceAxisB = i;

            float sign = (tp >= 0.f) ? -1.f : 1.f;
            bestFaceNormal = Mul(_b.u[i], sign); // de B vers A
        }
    }

    // Axes croisés edge-edge
    for (int k = 0; k < 9; ++k)
    {
        const int i = kEdgeAxisA[k];
        const int j = kEdgeAxisB[k];

        XMFLOAT3 axis = Cross(_a.u[i], _b.u[j]);
        float axisLenSq = LengthSq(axis);

        if (axisLenSq < 1e-10f)
            continue;

        switch (k)
        {
        case 0:
            ra = eA[1] * AbsR[2][0] + eA[2] * AbsR[1][0];
            rb = eB[1] * AbsR[0][2] + eB[2] * AbsR[0][1];
            tp = t[2] * R[1][0] - t[1] * R[2][0];
            break;
        case 1:
            ra = eA[1] * AbsR[2][1] + eA[2] * AbsR[1][1];
            rb = eB[0] * AbsR[0][2] + eB[2] * AbsR[0][0];
            tp = t[2] * R[1][1] - t[1] * R[2][1];
            break;
        case 2:
            ra = eA[1] * AbsR[2][2] + eA[2] * AbsR[1][2];
            rb = eB[0] * AbsR[0][1] + eB[1] * AbsR[0][0];
            tp = t[2] * R[1][2] - t[1] * R[2][2];
            break;
        case 3:
            ra = eA[0] * AbsR[2][0] + eA[2] * AbsR[0][0];
            rb = eB[1] * AbsR[1][2] + eB[2] * AbsR[1][1];
            tp = t[0] * R[2][0] - t[2] * R[0][0];
            break;
        case 4:
            ra = eA[0] * AbsR[2][1] + eA[2] * AbsR[0][1];
            rb = eB[0] * AbsR[1][2] + eB[2] * AbsR[1][0];
            tp = t[0] * R[2][1] - t[2] * R[0][1];
            break;
        case 5:
            ra = eA[0] * AbsR[2][2] + eA[2] * AbsR[0][2];
            rb = eB[0] * AbsR[1][1] + eB[1] * AbsR[1][0];
            tp = t[0] * R[2][2] - t[2] * R[0][2];
            break;
        case 6:
            ra = eA[0] * AbsR[1][0] + eA[1] * AbsR[0][0];
            rb = eB[1] * AbsR[2][2] + eB[2] * AbsR[2][1];
            tp = t[1] * R[0][0] - t[0] * R[1][0];
            break;
        case 7:
            ra = eA[0] * AbsR[1][1] + eA[1] * AbsR[0][1];
            rb = eB[0] * AbsR[2][2] + eB[2] * AbsR[2][0];
            tp = t[1] * R[0][1] - t[0] * R[1][1];
            break;
        default:
            ra = eA[0] * AbsR[1][2] + eA[1] * AbsR[0][2];
            rb = eB[0] * AbsR[2][1] + eB[1] * AbsR[2][0];
            tp = t[1] * R[0][2] - t[0] * R[1][2];
            break;
        }

        pen = (ra + rb) - fabsf(tp);

        if (pen < 0.f)
            return SATResult{};

        if (pen < bestEdgePen)
        {
            bestEdgePen = pen;
            bestEdgeAxisA = i;
            bestEdgeAxisB = j;

            XMFLOAT3 n = NormalizeSafe(axis, _a.u[0]);
            if (Dot(n, Subtract(_a.c, _b.c)) < 0.f)
                n = Inverse(n);

            bestEdgeNormal = n; // de B vers A
        }
    }

    SATResult result;
    result.hit = true;

    result.bestFaceType = bestFaceType;
    result.bestFaceAxisA = bestFaceAxisA;
    result.bestFaceAxisB = bestFaceAxisB;
    result.bestFaceNormal = bestFaceNormal;
    result.bestFacePenetration = bestFacePen;

    // Préférence forte pour les faces.
    // EdgeEdge n’est pris que s’il est réellement meilleur.
    if (bestEdgeAxisA != -1 && bestEdgePen < bestFacePen - kEdgePreferenceEpsilon)
    {
        result.axisType = SeparatingAxisType::EdgeEdge;
        result.axisIndexA = bestEdgeAxisA;
        result.axisIndexB = bestEdgeAxisB;
        result.normal = bestEdgeNormal;
        result.penetration = bestEdgePen;
    }
    else
    {
        result.axisType = bestFaceType;
        result.axisIndexA = bestFaceAxisA;
        result.axisIndexB = bestFaceAxisB;
        result.normal = bestFaceNormal;
        result.penetration = bestFacePen;
    }

    return result;
}

ContactInfo NarrowPhaseSystem::BuildFaceFaceManifold(const NarrowOBB& referenceBox, const NarrowOBB& incidentBox, int referenceAxis, const XMFLOAT3& normalBtoA, float penetration)
{
    ContactInfo manifold;
    manifold.hit = true;
    manifold.normal = normalBtoA;

    // Normale de la face de référence orientée vers la boîte incidente
    XMFLOAT3 refNormalOut = Inverse(normalBtoA);

    // Signe de la face de référence dans la box de référence
    float refSign = (Dot(refNormalOut, referenceBox.u[referenceAxis]) >= 0.f) ? 1.f : -1.f;

    FaceQuad refFace = BuildFaceQuad(referenceBox, referenceAxis, refSign);

    // Face incidente = face la plus opposée à refNormalOut
    int incAxis = FindIncidentFaceAxis(incidentBox, refNormalOut);
    float incSign = FindIncidentFaceSign(incidentBox, incAxis, refNormalOut);
    FaceQuad incFace = BuildFaceQuad(incidentBox, incAxis, incSign);

    Plane sidePlanes[4];

    // intérieur si dot(n, p) - d <= 0
    sidePlanes[0].n = refFace.axis1;
    sidePlanes[0].d = Dot(sidePlanes[0].n, refFace.center) + refFace.extent1;

    sidePlanes[1].n = Inverse(refFace.axis1);
    sidePlanes[1].d = Dot(sidePlanes[1].n, refFace.center) + refFace.extent1;

    sidePlanes[2].n = refFace.axis2;
    sidePlanes[2].d = Dot(sidePlanes[2].n, refFace.center) + refFace.extent2;

    sidePlanes[3].n = Inverse(refFace.axis2);
    sidePlanes[3].d = Dot(sidePlanes[3].n, refFace.center) + refFace.extent2;

    XMFLOAT3 buffer0[8];
    XMFLOAT3 buffer1[8];

    int count = 4;
    for (int i = 0; i < 4; ++i)
        buffer0[i] = incFace.vertices[i];

    count = ClipPolygonAgainstPlane(buffer0, count, buffer1, sidePlanes[0]);
    if (count <= 0) return ContactInfo{};

    count = ClipPolygonAgainstPlane(buffer1, count, buffer0, sidePlanes[1]);
    if (count <= 0) return ContactInfo{};

    count = ClipPolygonAgainstPlane(buffer0, count, buffer1, sidePlanes[2]);
    if (count <= 0) return ContactInfo{};

    count = ClipPolygonAgainstPlane(buffer1, count, buffer0, sidePlanes[3]);
    if (count <= 0) return ContactInfo{};

    // Plan de la face de référence
    float refPlaneD = Dot(refNormalOut, refFace.center);

    manifold.hit = true;
    manifold.normal = normalBtoA;

    for (int i = 0; i < count; ++i)
    {
        const XMFLOAT3& p = buffer0[i];

        float separation = Dot(refNormalOut, p) - refPlaneD;
        float pointPenetration = -separation;

        if (pointPenetration >= -1e-4f)
        {
            XMFLOAT3 projected = Subtract(p, Mul(refNormalOut, separation));
            AddContactPointUnique(manifold, projected, Max(0.f, pointPenetration));
        }
    }

    if (manifold.pointCount == 0)
    {
        float separation = Dot(refNormalOut, incFace.center) - refPlaneD;
        float pointPenetration = -separation;
        if (pointPenetration >= -1e-4f)
        {
            XMFLOAT3 projected = Subtract(incFace.center, Mul(refNormalOut, separation));
            AddContactPointUnique(manifold, projected, Max(0.f, pointPenetration));
        }
    }

    if (manifold.pointCount == 0)
        return ContactInfo{};

    return manifold;
}

ContactInfo NarrowPhaseSystem::BuildEdgeEdgeManifold(const NarrowOBB& _a, const NarrowOBB& _b, const SATResult& sat)
{
    ContactInfo manifold;
    manifold.hit = true;
    manifold.normal = sat.normal;

    Segment edgeA = GetSupportEdge(_a, sat.axisIndexA, Inverse(sat.normal));
    Segment edgeB = GetSupportEdge(_b, sat.axisIndexB, sat.normal);

    float s = 0.f, t = 0.f;
    XMFLOAT3 c1, c2;
    ClosestPtSegmentSegment(edgeA.a, edgeA.b, edgeB.a, edgeB.b, s, t, c1, c2);

    XMFLOAT3 contact = Mul(Add(c1, c2), 0.5f);

    // Validation : on rejette les points edge-edge aberrants
    if (!IsPointNearOBB(contact, _a, kEdgeValidationTolerance) ||
        !IsPointNearOBB(contact, _b, kEdgeValidationTolerance))
    {
        return ContactInfo{};
    }

    manifold.pointCount = 1;
    manifold.points[0].position = contact;
    manifold.points[0].penetration = sat.penetration;

    return manifold;
}

ContactInfo NarrowPhaseSystem::TestSphereSphere(const NarrowSphere& _a, const NarrowSphere& _b)
{
    XMFLOAT3 d = Subtract(_a.center, _b.center);
    float dist2 = LengthSq(d);
    float sumR = _a.radius + _b.radius;

    if (dist2 > sumR * sumR)
        return ContactInfo{};

    float dist = sqrtf(dist2);

    ContactInfo result;
    result.hit = true;

    if (dist > 1e-6f)
        result.normal = { d.x / dist, d.y / dist, d.z / dist };
    else
        result.normal = { 0.f, 1.f, 0.f };

    result.pointCount = 1;
    result.points[0].penetration = sumR - dist;
    result.points[0].position =
    {
        _b.center.x + result.normal.x * _b.radius,
        _b.center.y + result.normal.y * _b.radius,
        _b.center.z + result.normal.z * _b.radius
    };

    return result;
}

ContactInfo NarrowPhaseSystem::TestSphereOBB(const NarrowSphere& _s, const NarrowOBB& _b)
{
    XMFLOAT3 d = Subtract(_s.center, _b.c);

    float local[3] =
    {
        Dot(d, _b.u[0]),
        Dot(d, _b.u[1]),
        Dot(d, _b.u[2])
    };

    float clamped[3] =
    {
        std::fmax(-_b.e.x, std::fmin(local[0], _b.e.x)),
        std::fmax(-_b.e.y, std::fmin(local[1], _b.e.y)),
        std::fmax(-_b.e.z, std::fmin(local[2], _b.e.z))
    };

    bool inside =
        (local[0] >= -_b.e.x && local[0] <= _b.e.x) &&
        (local[1] >= -_b.e.y && local[1] <= _b.e.y) &&
        (local[2] >= -_b.e.z && local[2] <= _b.e.z);

    XMFLOAT3 closest = _b.c;
    closest = Add(closest, Mul(_b.u[0], clamped[0]));
    closest = Add(closest, Mul(_b.u[1], clamped[1]));
    closest = Add(closest, Mul(_b.u[2], clamped[2]));

    ContactInfo result{};

    if (!inside)
    {
        XMFLOAT3 v = Subtract(_s.center, closest);
        float dist2 = LengthSq(v);

        if (dist2 > _s.radius * _s.radius)
            return ContactInfo{};

        float dist = sqrtf(dist2);

        result.hit = true;
        result.pointCount = 1;
        result.points[0].position = closest;
        result.points[0].penetration = _s.radius - dist;

        if (dist > 1e-6f)
            result.normal = { v.x / dist, v.y / dist, v.z / dist };
        else
            result.normal = _b.u[1];

        return result;
    }

    float distToFace[3] =
    {
        _b.e.x - fabsf(local[0]),
        _b.e.y - fabsf(local[1]),
        _b.e.z - fabsf(local[2])
    };

    int bestAxis = 0;
    if (distToFace[1] < distToFace[bestAxis]) bestAxis = 1;
    if (distToFace[2] < distToFace[bestAxis]) bestAxis = 2;

    float sign = (local[bestAxis] >= 0.f) ? 1.f : -1.f;

    result.hit = true;
    result.normal = Mul(_b.u[bestAxis], sign);
    result.pointCount = 1;

    float faceCoord[3] = { local[0], local[1], local[2] };
    faceCoord[bestAxis] = sign * GetComponent(_b.e, bestAxis);

    result.points[0].position = _b.c;
    result.points[0].position = Add(result.points[0].position, Mul(_b.u[0], faceCoord[0]));
    result.points[0].position = Add(result.points[0].position, Mul(_b.u[1], faceCoord[1]));
    result.points[0].position = Add(result.points[0].position, Mul(_b.u[2], faceCoord[2]));
    result.points[0].penetration = _s.radius + distToFace[bestAxis];

    return result;
}

ContactInfo NarrowPhaseSystem::TestOBBOBB(const NarrowOBB& _a, const NarrowOBB& _b)
{
    SATResult sat = ComputeSATOBBOBB(_a, _b);

    if (!sat.hit)
        return ContactInfo{};

    switch (sat.axisType)
    {
    case SeparatingAxisType::FaceA:
        return BuildFaceFaceManifold(
            _a,
            _b,
            sat.axisIndexA,
            sat.normal,
            sat.penetration);

    case SeparatingAxisType::FaceB:
    {
        ContactInfo m = BuildFaceFaceManifold(
            _b,
            _a,
            sat.axisIndexB,
            Inverse(sat.normal),
            sat.penetration);

        m.normal = sat.normal;
        return m;
    }

    case SeparatingAxisType::EdgeEdge:
    {
        ContactInfo m = BuildEdgeEdgeManifold(_a, _b, sat);
        if (m.hit && m.pointCount > 0)
            return m;

        // Fallback propre vers le meilleur axe de face connu
        if (sat.bestFaceType == SeparatingAxisType::FaceA)
        {
            return BuildFaceFaceManifold(
                _a,
                _b,
                sat.bestFaceAxisA,
                sat.bestFaceNormal,
                sat.bestFacePenetration);
        }
        else
        {
            ContactInfo mf = BuildFaceFaceManifold(
                _b,
                _a,
                sat.bestFaceAxisB,
                Inverse(sat.bestFaceNormal),
                sat.bestFacePenetration);

            mf.normal = sat.bestFaceNormal;
            return mf;
        }
    }

    default:
        return ContactInfo{};
    }
}

ContactInfo NarrowPhaseSystem::Dispatch(ColliderComponent& _a, ColliderComponent& _b)
{
    ShapeType ta = _a.type;
    ShapeType tb = _b.type;

    // Sphere — Sphere
    if (ta == ShapeType::Sphere && tb == ShapeType::Sphere)
        return DispatchSphereSphere(_a, _b);

    // Sphere — Box  (dans les deux sens)
    if (ta == ShapeType::Sphere && tb == ShapeType::Box)
        return DispatchSphereOBB(_a, _b);

    if (ta == ShapeType::Box && tb == ShapeType::Sphere)
    {
        ContactInfo c = DispatchSphereOBB(_b, _a);
        // On inverse la normale pour qu'elle pointe de B vers A
        // (convention : normale de _b vers _a)
        c.normal = Inverse(c.normal);
        return c;
    }

    // Box — Box
    if (ta == ShapeType::Box && tb == ShapeType::Box)
        return DispatchOBBOBB(_a, _b);

    // Capsule : non implémenté pour l'instant
    return ContactInfo{};
}

ContactInfo NarrowPhaseSystem::DispatchSphereSphere(ColliderComponent& _a, ColliderComponent& _b)
{
    NarrowSphere a = BuildSphere(_a);
    NarrowSphere b = BuildSphere(_b);
    return TestSphereSphere(a, b);
}

ContactInfo NarrowPhaseSystem::DispatchSphereOBB(ColliderComponent& _sphere, ColliderComponent& _box)
{
    NarrowSphere s = BuildSphere(_sphere);
    NarrowOBB b = BuildOBB(_box);
    return TestSphereOBB(s, b);
}

ContactInfo NarrowPhaseSystem::DispatchOBBOBB(ColliderComponent& _a, ColliderComponent& _b)
{
    NarrowOBB a = BuildOBB(_a);
    NarrowOBB b = BuildOBB(_b);
    return TestOBBOBB(a, b);
}