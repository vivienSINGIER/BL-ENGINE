#pragma once
// ============================================================
//  RigidBody.hpp
//  Données physiques d'un corps rigide, style VPhysics.
//  Découplé du Transform ECS : le PhysicsWorld écrit
//  en retour dans le Transform via un callback.
// ============================================================
#include "PhysicsMath.hpp"
#include <cstdint>
#include <functional>

using EntityId = std::uint32_t;
constexpr EntityId INVALID_ENTITY = ~0u;

enum class BodyType : uint8_t
{
    Static,     // masse infinie, ne bouge jamais
    Kinematic,  // déplacé manuellement, pas d'impulse reçue
    Dynamic     // corps complet
};

enum class ColliderShape : uint8_t { Box, Sphere, Capsule };

// ---- Tenseur d'inertie local (diagonale) ---- 
struct InertiaTensor
{
    XMFLOAT3 local   = {1,1,1};   // composantes locales
    XMFLOAT3 invLocal = {1,1,1};  // inverses locales

    // Tenseur en world space (mis à jour chaque frame si rotation active)
    Mat3 world;
    Mat3 worldInv;

    void SetFromBox(float mass, const XMFLOAT3& halfExtents)
    {
        float w = halfExtents.x*2, h = halfExtents.y*2, d = halfExtents.z*2;
        local.x = (1.f/12.f)*mass*(h*h+d*d);
        local.y = (1.f/12.f)*mass*(w*w+d*d);
        local.z = (1.f/12.f)*mass*(w*w+h*h);
        _ComputeInvLocal();
    }

    void SetFromSphere(float mass, float radius)
    {
        float i = (2.f/5.f)*mass*radius*radius;
        local = {i,i,i};
        _ComputeInvLocal();
    }

    void SetFromCapsule(float mass, float radius, float halfHeight)
    {
        // Approximation cylindre
        float r2 = radius*radius;
        float h2 = (halfHeight*2)*(halfHeight*2);
        local.x = local.z = (1.f/12.f)*mass*(3*r2+h2);
        local.y = 0.5f*mass*r2;
        _ComputeInvLocal();
    }

    // Recalcule world/worldInv depuis la rotation quaternion courante
    void UpdateWorldTensor(const XMFLOAT4& quat)
    {
        Mat3 R = Mat3::FromRotation(quat);
        world    = Mat3::SandwichDiag(R, local);
        worldInv = Mat3::SandwichDiag(R, invLocal);
    }

    XMFLOAT3 ApplyInvWorld(const XMFLOAT3& v) const { return worldInv.Mul(v); }

private:
    void _ComputeInvLocal()
    {
        invLocal.x = local.x > 0 ? 1.f/local.x : 0;
        invLocal.y = local.y > 0 ? 1.f/local.y : 0;
        invLocal.z = local.z > 0 ? 1.f/local.z : 0;
    }
};

// ============================================================
struct RigidBody
{
    EntityId id = INVALID_ENTITY;
    BodyType bodyType = BodyType::Dynamic;

    // ---- Masse ----
    float mass       = 1.0f;
    float massInv    = 1.0f;
    InertiaTensor inertia;

    // ---- Matériau ----
    float restitution     = 0.2f;
    float staticFriction  = 0.6f;
    float dynamicFriction = 0.4f;

    // ---- État cinématique ----
    XMFLOAT3 position        = {0,0,0};
    XMFLOAT4 orientation     = {0,0,0,1};   // quaternion
    XMFLOAT3 linearVelocity  = {0,0,0};
    XMFLOAT3 angularVelocity = {0,0,0};

    // ---- Accumulateurs de forces (vidés chaque frame) ----
    XMFLOAT3 forceAccum  = {0,0,0};
    XMFLOAT3 torqueAccum = {0,0,0};

    // ---- Flags ----
    bool useGravity      = false;
    bool allowRotation   = true;
    bool isSleeping      = false;
    float sleepTimer     = 0.0f;

    // ---- Support contact (pour le sleep sur sol) ----
    bool      hasSupportContact = false;
    XMFLOAT3  supportNormal     = {0,1,0};

    // ---- Collider shape data ----
    ColliderShape shape = ColliderShape::Box;
    XMFLOAT3      halfExtents = {0.5f,0.5f,0.5f};  // Box / Capsule
    float         radius = 0.5f;                     // Sphere / Capsule
    float         capsuleHalfHeight = 0.5f;          // Capsule uniquement
    // Offset local du collider (centre ≠ position du pivot)
    XMFLOAT3      colliderOffset = {0,0,0};

    // ---- API forces ----
    void AddForce(const XMFLOAT3& f)  { forceAccum  = V3Add(forceAccum, f);  WakeUp(); }
    void AddTorque(const XMFLOAT3& t) { torqueAccum = V3Add(torqueAccum, t); WakeUp(); }
    void AddForceAtPoint(const XMFLOAT3& f, const XMFLOAT3& worldPoint)
    {
        forceAccum  = V3Add(forceAccum, f);
        XMFLOAT3 r  = V3Sub(worldPoint, position);
        torqueAccum = V3Add(torqueAccum, V3Cross(r, f));
        WakeUp();
    }
    void AddImpulse(const XMFLOAT3& j)
    {
        linearVelocity = V3Add(linearVelocity, V3Scale(j, massInv));
        WakeUp();
    }
    void AddAngularImpulse(const XMFLOAT3& j)
    {
        if (!allowRotation) return;
        XMFLOAT3 dw = inertia.ApplyInvWorld(j);
        angularVelocity = V3Add(angularVelocity, dw);
        WakeUp();
    }

    // ---- Setup ----
    void SetMass(float m)
    {
        mass    = m;
        massInv = (m > 0) ? 1.f/m : 0.f;
    }
    void SetStatic()
    {
        bodyType = BodyType::Static;
        massInv  = 0;
        inertia.invLocal = {0,0,0};
        inertia.worldInv = {};
        Sleep();
    }
    void MakeBox(float m, const XMFLOAT3& he)
    {
        shape = ColliderShape::Box;
        halfExtents = he;
        SetMass(m);
        inertia.SetFromBox(m, he);
    }
    void MakeSphere(float m, float r)
    {
        shape = ColliderShape::Sphere;
        radius = r;
        halfExtents = {r,r,r};
        SetMass(m);
        inertia.SetFromSphere(m, r);
    }
    void MakeCapsule(float m, float r, float hh)
    {
        shape = ColliderShape::Capsule;
        radius = r;
        capsuleHalfHeight = hh;
        halfExtents = {r, hh+r, r};
        SetMass(m);
        inertia.SetFromCapsule(m, r, hh);
    }

    // ---- Sleep ----
    void WakeUp()  { isSleeping = false; sleepTimer = 0; }
    void Sleep()
    {
        isSleeping      = true;
        sleepTimer      = 0;
        linearVelocity  = {0,0,0};
        angularVelocity = {0,0,0};
        forceAccum      = {0,0,0};
        torqueAccum     = {0,0,0};
    }

    // ---- Requêtes ----
    XMFLOAT3 GetVelocityAtPoint(const XMFLOAT3& worldPoint) const
    {
        XMFLOAT3 r = V3Sub(worldPoint, position);
        return V3Add(linearVelocity, V3Cross(angularVelocity, r));
    }

    XMFLOAT3 GetColliderCenter() const
    {
        // Applique l'offset local en world space (approximation sans rotation pour offset nul)
        if (colliderOffset.x == 0 && colliderOffset.y == 0 && colliderOffset.z == 0)
            return position;
        XMVECTOR p = XMLoadFloat3(&position);
        XMVECTOR off = XMLoadFloat3(&colliderOffset);
        XMVECTOR q = XMLoadFloat4(&orientation);
        XMVECTOR rotated = XMVector3Rotate(off, q);
        XMFLOAT3 result;
        XMStoreFloat3(&result, XMVectorAdd(p, rotated));
        return result;
    }
};
