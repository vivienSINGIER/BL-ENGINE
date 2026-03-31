#pragma once
// ============================================================
//  NarrowPhase.hpp
//  Tests de collision précis :
//    - OBB vs OBB  (SAT + clipping de manifold, conservé de ton moteur)
//    - Sphere vs Sphere
//    - OBB vs Sphere
//    - Capsule vs Sphere
//    - Capsule vs OBB  (GJK simplifié)
//
//  Toutes les fonctions retournent un Contact rempli ou rien.
// ============================================================
#include "RigidBody.hpp"
#include "Contact.hpp"
#include <optional>
#include <array>
#include <cfloat>

// ============================================================
//  Helpers OBB
// ============================================================
namespace NP_Internal
{

struct OBBData
{
    XMFLOAT3 center;
    XMFLOAT3 axes[3];
    XMFLOAT3 halfExtents;

    static OBBData From(const RigidBody& rb)
    {
        OBBData o;
        o.center      = rb.GetColliderCenter();
        o.halfExtents = rb.halfExtents;

        XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(&rb.orientation));
        XMVECTOR ax = XMVector3TransformNormal(XMVectorSet(1,0,0,0), rot);
        XMVECTOR ay = XMVector3TransformNormal(XMVectorSet(0,1,0,0), rot);
        XMVECTOR az = XMVector3TransformNormal(XMVectorSet(0,0,1,0), rot);
        XMStoreFloat3(&o.axes[0], XMVector3Normalize(ax));
        XMStoreFloat3(&o.axes[1], XMVector3Normalize(ay));
        XMStoreFloat3(&o.axes[2], XMVector3Normalize(az));
        return o;
    }
};

inline float OBBRadius(const OBBData& o, const XMFLOAT3& axis)
{
    return o.halfExtents.x * fabsf(V3Dot(axis, o.axes[0]))
         + o.halfExtents.y * fabsf(V3Dot(axis, o.axes[1]))
         + o.halfExtents.z * fabsf(V3Dot(axis, o.axes[2]));
}

inline XMFLOAT3 OBBSupport(const OBBData& o, const XMFLOAT3& dir)
{
    XMFLOAT3 p = o.center;
    for (int i = 0; i < 3; ++i)
    {
        float s = V3Dot(dir, o.axes[i]);
        float e[] = {o.halfExtents.x, o.halfExtents.y, o.halfExtents.z};
        p = V3Add(p, V3Scale(o.axes[i], s >= 0 ? e[i] : -e[i]));
    }
    return p;
}

inline void GetOBBCorners(const OBBData& o, XMFLOAT3 out[8])
{
    XMFLOAT3 ex = V3Scale(o.axes[0], o.halfExtents.x);
    XMFLOAT3 ey = V3Scale(o.axes[1], o.halfExtents.y);
    XMFLOAT3 ez = V3Scale(o.axes[2], o.halfExtents.z);
    int k=0;
    for (int sx : {-1,1}) for (int sy : {-1,1}) for (int sz : {-1,1})
        out[k++] = V3Add(o.center, V3Add(V3Scale(ex,sx), V3Add(V3Scale(ey,sy), V3Scale(ez,sz))));
}

inline bool OverlapOnAxis(const OBBData& a, const OBBData& b, const XMFLOAT3& axis,
                           float& minOverlap, int& minIdx, int currIdx)
{
    float len = V3LenSq(axis);
    if (len < 1e-8f) return true;  // axe dégénéré → skip
    XMFLOAT3 normAxis = V3Scale(axis, 1.0f/sqrtf(len));

    XMFLOAT3 d = V3Sub(b.center, a.center);
    float dist  = fabsf(V3Dot(d, normAxis));
    float rA    = OBBRadius(a, normAxis);
    float rB    = OBBRadius(b, normAxis);
    float overlap = rA + rB - dist;
    if (overlap <= 0.0f) return false;
    if (overlap < minOverlap) { minOverlap = overlap; minIdx = currIdx; }
    return true;
}

// Clipping d'un polygone contre un plan (Sutherland-Hodgman)
inline int ClipPolyVsPlane(const XMFLOAT3* in, int inCount, XMFLOAT3* out,
                             const XMFLOAT3& planePoint, const XMFLOAT3& planeNormal)
{
    int outCount = 0;
    for (int i = 0; i < inCount; ++i)
    {
        const XMFLOAT3& curr = in[i];
        const XMFLOAT3& next = in[(i+1)%inCount];
        float dCurr = V3Dot(V3Sub(curr, planePoint), planeNormal);
        float dNext = V3Dot(V3Sub(next, planePoint), planeNormal);
        if (dCurr <= 0.0f) out[outCount++] = curr;
        if ((dCurr < 0.0f) != (dNext < 0.0f))
        {
            float t = dCurr / (dCurr - dNext);
            out[outCount++] = V3Add(curr, V3Scale(V3Sub(next, curr), t));
        }
    }
    return outCount;
}

inline void GetFaceVertices(const OBBData& o, int axis, bool positive, XMFLOAT3 out[4])
{
    XMFLOAT3 ex[3] = {
        V3Scale(o.axes[0], o.halfExtents.x),
        V3Scale(o.axes[1], o.halfExtents.y),
        V3Scale(o.axes[2], o.halfExtents.z)
    };
    XMFLOAT3 faceCenter = V3Add(o.center, V3Scale(ex[axis], positive ? 1.0f : -1.0f));
    int u = (axis+1)%3, v = (axis+2)%3;
    out[0] = V3Add(V3Add(faceCenter,  ex[u]),  ex[v]);
    out[1] = V3Add(V3Sub(faceCenter,  ex[u]),  ex[v]);
    out[2] = V3Sub(V3Sub(faceCenter,  ex[u]),  ex[v]);
    out[3] = V3Sub(V3Add(faceCenter,  ex[u]),  ex[v]);
}

} // namespace NP_Internal

// ============================================================
//  NarrowPhase
// ============================================================
class NarrowPhase
{
public:
    // ---- Dispatch principal ----
    std::optional<Contact> Test(const RigidBody& rbA, const RigidBody& rbB)
    {
        using S = ColliderShape;
        auto sa = rbA.shape, sb = rbB.shape;

        if (sa == S::Box    && sb == S::Box)    return OBBvsOBB(rbA, rbB);
        if (sa == S::Sphere && sb == S::Sphere) return SphereVsSphere(rbA, rbB);
        if (sa == S::Box    && sb == S::Sphere) return OBBvsSphere(rbA, rbB);
        if (sa == S::Sphere && sb == S::Box)
        {
            auto c = OBBvsSphere(rbB, rbA);
            if (c) c->normal = V3Neg(c->normal);
            return c;
        }
        if (sa == S::Capsule && sb == S::Sphere)  return CapsuleVsSphere(rbA, rbB);
        if (sa == S::Sphere  && sb == S::Capsule)
        {
            auto c = CapsuleVsSphere(rbB, rbA);
            if (c) c->normal = V3Neg(c->normal);
            return c;
        }
        if (sa == S::Capsule && sb == S::Box)
        {
            auto c = OBBvsCapsule(rbB, rbA);
            if (c) c->normal = V3Neg(c->normal);
            return c;
        }
        if (sa == S::Box && sb == S::Capsule) return OBBvsCapsule(rbA, rbB);
        if (sa == S::Capsule && sb == S::Capsule) return CapsuleVsCapsule(rbA, rbB);
        return std::nullopt;
    }

private:
    using OBBData = NP_Internal::OBBData;

    // ---- OBB vs OBB (SAT + clipping) ----
    std::optional<Contact> OBBvsOBB(const RigidBody& rbA, const RigidBody& rbB)
    {
        OBBData boxA = OBBData::From(rbA);
        OBBData boxB = OBBData::From(rbB);

        XMFLOAT3 axes[15] = {
            boxA.axes[0], boxA.axes[1], boxA.axes[2],
            boxB.axes[0], boxB.axes[1], boxB.axes[2],
            V3Cross(boxA.axes[0], boxB.axes[0]), V3Cross(boxA.axes[0], boxB.axes[1]),
            V3Cross(boxA.axes[0], boxB.axes[2]), V3Cross(boxA.axes[1], boxB.axes[0]),
            V3Cross(boxA.axes[1], boxB.axes[1]), V3Cross(boxA.axes[1], boxB.axes[2]),
            V3Cross(boxA.axes[2], boxB.axes[0]), V3Cross(boxA.axes[2], boxB.axes[1]),
            V3Cross(boxA.axes[2], boxB.axes[2])
        };

        float minOverlap = FLT_MAX;
        int   minAxis    = -1;

        for (int i = 0; i < 15; ++i)
            if (!NP_Internal::OverlapOnAxis(boxA, boxB, axes[i], minOverlap, minAxis, i))
                return std::nullopt;

        // Construit le contact
        Contact c;
        c.idA = rbA.id; c.idB = rbB.id;

        XMFLOAT3 normal = V3Normalize(axes[minAxis]);
        XMFLOAT3 delta  = V3Sub(boxB.center, boxA.center);
        if (V3Dot(delta, normal) < 0) normal = V3Neg(normal);

        c.normal = normal;
        c.BuildTangentFrame();

        // Manifold par face-clipping (axes face : 0-5)
        bool built = false;
        if (minAxis < 6)
            built = _BuildOBBManifoldClipping(boxA, boxB, c, minAxis);

        if (!built)
        {
            // Fallback : support points
            XMFLOAT3 pa = NP_Internal::OBBSupport(boxA, normal);
            XMFLOAT3 pb = NP_Internal::OBBSupport(boxB, V3Neg(normal));
            c.points[0].position    = V3Scale(V3Add(pa,pb), 0.5f);
            c.points[0].penetration = minOverlap;
            c.pointCount = 1;
        }
        return c;
    }

    bool _BuildOBBManifoldClipping(const OBBData& boxA, const OBBData& boxB,
                                    Contact& c, int minAxis)
    {
        // Détermine reference/incident face
        bool refIsA = minAxis < 3;
        int  axisIdx = minAxis % 3;
        const OBBData* refBox = refIsA ? &boxA : &boxB;
        const OBBData* incBox = refIsA ? &boxB : &boxA;
        XMFLOAT3 refNormal = refIsA ? c.normal : V3Neg(c.normal);

        bool posFace = V3Dot(refNormal, refBox->axes[axisIdx]) >= 0;

        XMFLOAT3 refFace[4], incFace[4];
        NP_Internal::GetFaceVertices(*refBox, axisIdx, posFace,  refFace);

        // Incident face : face de incBox la plus alignée contre refNormal
        int incAxis = 0;
        float maxDot = -FLT_MAX;
        bool  incPos = false;
        for (int i = 0; i < 3; ++i)
        {
            float d = V3Dot(V3Neg(refNormal), incBox->axes[i]);
            if (fabsf(d) > maxDot) { maxDot = fabsf(d); incAxis = i; incPos = (d >= 0); }
        }
        NP_Internal::GetFaceVertices(*incBox, incAxis, incPos, incFace);

        // Clipping Sutherland-Hodgman contre les 4 bords de la face reference
        XMFLOAT3 buf0[8], buf1[8], buf2[8], buf3[8], buf4[8];
        for (int i = 0; i < 4; ++i) buf0[i] = incFace[i];
        int cnt = 4;

        int refU = (axisIdx+1)%3, refV = (axisIdx+2)%3;
        XMFLOAT3 refCenter = refFace[0]; // approx
        // Recalcule le vrai centre de la face ref
        refCenter = V3Scale(V3Add(V3Add(refFace[0],refFace[1]),V3Add(refFace[2],refFace[3])), 0.25f);

        XMFLOAT3 eu = V3Scale(refBox->axes[refU], refBox->halfExtents.x * (refU==0?1:refU==1?1:1));
        // En réalité les extents correspondent à l'axe
        float eU[] = {refBox->halfExtents.x, refBox->halfExtents.y, refBox->halfExtents.z};
        float eV[] = {refBox->halfExtents.x, refBox->halfExtents.y, refBox->halfExtents.z};

        auto clip4 = [&](XMFLOAT3* in, int inC, XMFLOAT3* out, const XMFLOAT3& pp, const XMFLOAT3& pn) {
            return NP_Internal::ClipPolyVsPlane(in, inC, out, pp, pn);
        };

        cnt = clip4(buf0, cnt, buf1,
            V3Add(refCenter, V3Scale(refBox->axes[refU],  eU[refU])),  refBox->axes[refU]);
        if (cnt <= 0) return false;
        cnt = clip4(buf1, cnt, buf2,
            V3Add(refCenter, V3Scale(refBox->axes[refU], -eU[refU])), V3Neg(refBox->axes[refU]));
        if (cnt <= 0) return false;
        cnt = clip4(buf2, cnt, buf3,
            V3Add(refCenter, V3Scale(refBox->axes[refV],  eV[refV])),  refBox->axes[refV]);
        if (cnt <= 0) return false;
        cnt = clip4(buf3, cnt, buf4,
            V3Add(refCenter, V3Scale(refBox->axes[refV], -eV[refV])), V3Neg(refBox->axes[refV]));
        if (cnt <= 0) return false;

        // Garde uniquement les points sous la face référence
        XMFLOAT3 faceNormal = posFace ? refBox->axes[axisIdx] : V3Neg(refBox->axes[axisIdx]);
        c.pointCount = 0;
        for (int i = 0; i < cnt && c.pointCount < MAX_CONTACT_POINTS; ++i)
        {
            float sep = V3Dot(V3Sub(buf4[i], refFace[0]), faceNormal);
            if (sep <= 0.02f)
            {
                c.points[c.pointCount].position    = buf4[i];
                c.points[c.pointCount].penetration = -sep;
                ++c.pointCount;
            }
        }
        return c.pointCount > 0;
    }

    // ---- Sphere vs Sphere ----
    std::optional<Contact> SphereVsSphere(const RigidBody& a, const RigidBody& b)
    {
        XMFLOAT3 delta = V3Sub(b.GetColliderCenter(), a.GetColliderCenter());
        float d2 = V3LenSq(delta);
        float rSum = a.radius + b.radius;
        if (d2 > rSum*rSum) return std::nullopt;

        Contact c; c.idA=a.id; c.idB=b.id;
        float d = sqrtf(d2);
        c.normal = d < 1e-6f ? XMFLOAT3{0,1,0} : V3Scale(delta, 1.0f/d);
        c.BuildTangentFrame();
        XMFLOAT3 pa = V3Add(a.GetColliderCenter(), V3Scale(c.normal, a.radius));
        XMFLOAT3 pb = V3Sub(b.GetColliderCenter(), V3Scale(c.normal, b.radius));
        c.points[0].position    = V3Scale(V3Add(pa,pb), 0.5f);
        c.points[0].penetration = rSum - d;
        c.pointCount = 1;
        return c;
    }

    // ---- OBB vs Sphere ----
    std::optional<Contact> OBBvsSphere(const RigidBody& box, const RigidBody& sphere)
    {
        OBBData o = OBBData::From(box);
        XMFLOAT3 sc = sphere.GetColliderCenter();
        XMFLOAT3 d  = V3Sub(sc, o.center);

        float lx = Clampf(V3Dot(d, o.axes[0]), -o.halfExtents.x, o.halfExtents.x);
        float ly = Clampf(V3Dot(d, o.axes[1]), -o.halfExtents.y, o.halfExtents.y);
        float lz = Clampf(V3Dot(d, o.axes[2]), -o.halfExtents.z, o.halfExtents.z);

        XMFLOAT3 closest = V3Add(o.center,
            V3Add(V3Add(V3Scale(o.axes[0],lx), V3Scale(o.axes[1],ly)), V3Scale(o.axes[2],lz)));

        XMFLOAT3 diff = V3Sub(sc, closest);
        float d2 = V3LenSq(diff);
        if (d2 > sphere.radius*sphere.radius) return std::nullopt;

        Contact c; c.idA=box.id; c.idB=sphere.id;
        float dist = sqrtf(d2);
        c.normal = dist > 1e-6f ? V3Scale(diff, 1.0f/dist)
                                 : _BoxFaceNormal(o, {lx,ly,lz});
        c.BuildTangentFrame();
        c.points[0].position    = closest;
        c.points[0].penetration = sphere.radius - dist;
        c.pointCount = 1;
        return c;
    }

    XMFLOAT3 _BoxFaceNormal(const OBBData& o, const XMFLOAT3& local)
    {
        float dx = o.halfExtents.x - fabsf(local.x);
        float dy = o.halfExtents.y - fabsf(local.y);
        float dz = o.halfExtents.z - fabsf(local.z);
        if (dx <= dy && dx <= dz)
            return local.x >= 0 ? o.axes[0] : V3Neg(o.axes[0]);
        if (dy <= dz)
            return local.y >= 0 ? o.axes[1] : V3Neg(o.axes[1]);
        return local.z >= 0 ? o.axes[2] : V3Neg(o.axes[2]);
    }

    // ---- Capsule vs Sphere ----
    std::optional<Contact> CapsuleVsSphere(const RigidBody& cap, const RigidBody& sphere)
    {
        // Segment de la capsule en world space
        XMFLOAT3 capCenter = cap.GetColliderCenter();
        // L'axe local Y de la capsule
        XMVECTOR qv = XMLoadFloat4(&cap.orientation);
        XMVECTOR axisV = XMVector3Rotate(XMVectorSet(0,1,0,0), qv);
        XMFLOAT3 axis; XMStoreFloat3(&axis, axisV);
        XMFLOAT3 a = V3Add(capCenter, V3Scale(axis,  cap.capsuleHalfHeight));
        XMFLOAT3 b = V3Add(capCenter, V3Scale(axis, -cap.capsuleHalfHeight));

        // Point le plus proche du centre sphère sur le segment
        XMFLOAT3 sc = sphere.GetColliderCenter();
        XMFLOAT3 ab = V3Sub(b, a);
        float t = Clampf(V3Dot(V3Sub(sc, a), ab) / Maxf(V3LenSq(ab), 1e-8f), 0, 1);
        XMFLOAT3 closest = V3Add(a, V3Scale(ab, t));

        XMFLOAT3 diff = V3Sub(sc, closest);
        float d2 = V3LenSq(diff);
        float rSum = cap.radius + sphere.radius;
        if (d2 > rSum*rSum) return std::nullopt;

        float dist = sqrtf(d2);
        Contact c; c.idA=cap.id; c.idB=sphere.id;
        c.normal = dist > 1e-6f ? V3Scale(diff, 1.0f/dist) : XMFLOAT3{0,1,0};
        c.BuildTangentFrame();
        c.points[0].position    = closest;
        c.points[0].penetration = rSum - dist;
        c.pointCount = 1;
        return c;
    }

    // ---- Capsule vs OBB ----
    std::optional<Contact> OBBvsCapsule(const RigidBody& box, const RigidBody& cap)
    {
        // Approche : teste les deux extrémités + point le plus proche → 1-2 contacts
        XMFLOAT3 capCenter = cap.GetColliderCenter();
        XMVECTOR qv = XMLoadFloat4(&cap.orientation);
        XMVECTOR axisV = XMVector3Rotate(XMVectorSet(0,1,0,0), qv);
        XMFLOAT3 axis; XMStoreFloat3(&axis, axisV);
        XMFLOAT3 segA = V3Add(capCenter, V3Scale(axis,  cap.capsuleHalfHeight));
        XMFLOAT3 segB = V3Add(capCenter, V3Scale(axis, -cap.capsuleHalfHeight));

        // Crée des sphères temporaires aux deux extrémités + milieu
        RigidBody fakeSphere = cap;
        fakeSphere.shape = ColliderShape::Sphere;

        Contact best; int bestCount = 0; float bestPen = 0;
        for (const XMFLOAT3& pt : {segA, segB, capCenter})
        {
            fakeSphere.position = pt;
            auto c = OBBvsSphere(box, fakeSphere);
            if (c && c->points[0].penetration > bestPen)
            {
                best = *c;
                best.idA = box.id; best.idB = cap.id;
                bestPen = c->points[0].penetration;
            }
        }
        if (bestPen > 0) return best;
        return std::nullopt;
    }

    // ---- Capsule vs Capsule ----
    std::optional<Contact> CapsuleVsCapsule(const RigidBody& a, const RigidBody& b)
    {
        auto getSegment = [](const RigidBody& rb, XMFLOAT3& p0, XMFLOAT3& p1)
        {
            XMFLOAT3 c = rb.GetColliderCenter();
            XMVECTOR qv = XMLoadFloat4(&rb.orientation);
            XMVECTOR axisV = XMVector3Rotate(XMVectorSet(0,1,0,0), qv);
            XMFLOAT3 ax; XMStoreFloat3(&ax, axisV);
            p0 = V3Add(c, V3Scale(ax,  rb.capsuleHalfHeight));
            p1 = V3Add(c, V3Scale(ax, -rb.capsuleHalfHeight));
        };

        XMFLOAT3 p0,p1,q0,q1;
        getSegment(a, p0, p1);
        getSegment(b, q0, q1);

        // Distance segment-segment
        XMFLOAT3 d1 = V3Sub(p1,p0), d2 = V3Sub(q1,q0), r = V3Sub(p0,q0);
        float a11 = V3Dot(d1,d1), a22 = V3Dot(d2,d2), b1 = V3Dot(d1,r), b2 = V3Dot(d2,r);
        float denom = a11*a22 - V3Dot(d1,d2)*V3Dot(d1,d2);
        float s,t;
        if (fabsf(denom) < 1e-8f) { s=0; t=b2/Maxf(a22,1e-8f); }
        else { s=Clampf((V3Dot(d1,d2)*b2-a22*b1)/denom,0,1); t=Clampf((V3Dot(d1,d2)*s+b2)/Maxf(a22,1e-8f),0,1); }

        XMFLOAT3 ca = V3Add(p0, V3Scale(d1,s));
        XMFLOAT3 cb = V3Add(q0, V3Scale(d2,t));
        XMFLOAT3 diff = V3Sub(cb,ca);
        float d2val = V3LenSq(diff);
        float rSum = a.radius + b.radius;
        if (d2val > rSum*rSum) return std::nullopt;

        float dist = sqrtf(d2val);
        Contact c; c.idA=a.id; c.idB=b.id;
        c.normal = dist > 1e-6f ? V3Scale(diff,1.0f/dist) : XMFLOAT3{0,1,0};
        c.BuildTangentFrame();
        c.points[0].position    = V3Scale(V3Add(ca,cb),0.5f);
        c.points[0].penetration = rSum - dist;
        c.pointCount = 1;
        return c;
    }
};
