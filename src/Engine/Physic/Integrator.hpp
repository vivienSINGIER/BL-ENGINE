#pragma once
// ============================================================
//  Integrator.hpp
//  Intégration symplectic Euler (semi-implicite) :
//    v(n+1) = v(n) + a(n)*dt
//    x(n+1) = x(n) + v(n+1)*dt   ← utilise la nouvelle vitesse
//
//  + Intégration quaternion correcte via ω*q
//  + Sleep system amélioré (énergie cinétique pondérée)
// ============================================================
#include "RigidBody.hpp"
#include <cmath>

namespace
{
    constexpr float kLinearSleepThresholdSq  = 0.05f * 0.05f;
    constexpr float kAngularSleepThresholdSq = 0.05f * 0.05f;
    constexpr float kSleepTimeThreshold      = 0.5f;
    constexpr float kLinearDamping           = 0.02f;
    constexpr float kAngularDamping          = 0.05f;
}

class Integrator
{
public:
    XMFLOAT3 gravity = {0, -9.81f, 0};

    void Integrate(RigidBody& rb, float dt)
    {
        if (rb.bodyType != BodyType::Dynamic) return;
        if (rb.isSleeping) return;

        // ---- Accélération linéaire ----
        XMFLOAT3 accel = V3Scale(rb.forceAccum, rb.massInv);
        if (rb.useGravity)
            accel = V3Add(accel, gravity);

        // Vitesse : symplectic Euler
        rb.linearVelocity = V3Add(rb.linearVelocity, V3Scale(accel, dt));

        // Damping linéaire (air drag)
        rb.linearVelocity = V3Scale(rb.linearVelocity, 1.0f / (1.0f + kLinearDamping * dt));

        // Position
        rb.position = V3Add(rb.position, V3Scale(rb.linearVelocity, dt));

        // ---- Rotation ----
        if (rb.allowRotation)
        {
            // Met à jour le tenseur world avant de l'utiliser
            rb.inertia.UpdateWorldTensor(rb.orientation);

            XMFLOAT3 angAccel = rb.inertia.ApplyInvWorld(rb.torqueAccum);
            rb.angularVelocity = V3Add(rb.angularVelocity, V3Scale(angAccel, dt));

            // Damping angulaire
            rb.angularVelocity = V3Scale(rb.angularVelocity, 1.0f / (1.0f + kAngularDamping * dt));

            _IntegrateQuaternion(rb, dt);
        }

        // Clear forces
        rb.forceAccum  = V3Zero();
        rb.torqueAccum = V3Zero();

        // ---- Sleep ----
        _UpdateSleep(rb, dt);
    }

private:
    // Intégration du quaternion via ω (plus stable que Euler angles)
    //   dq/dt = 0.5 * [0, ω] * q
    void _IntegrateQuaternion(RigidBody& rb, float dt)
    {
        XMVECTOR q = XMLoadFloat4(&rb.orientation);

        // Quaternion pur représentant ω
        XMFLOAT4 omegaQ = { rb.angularVelocity.x * 0.5f * dt,
                             rb.angularVelocity.y * 0.5f * dt,
                             rb.angularVelocity.z * 0.5f * dt,
                             0.0f };
        XMVECTOR dq = XMLoadFloat4(&omegaQ);

        q = XMQuaternionNormalize(XMVectorAdd(q, XMQuaternionMultiply(dq, q)));
        XMStoreFloat4(&rb.orientation, q);
    }

    void _UpdateSleep(RigidBody& rb, float dt)
    {
        // Vecteur vitesse projeté sans la composante support
        XMFLOAT3 velForSleep = rb.linearVelocity;

        if (rb.hasSupportContact)
        {
            // Retire la composante normale au sol (comme Source Engine)
            XMFLOAT3 n = V3Normalize(rb.supportNormal);
            float vn = V3Dot(velForSleep, n);
            velForSleep = V3Sub(velForSleep, V3Scale(n, vn));

            // Supprime le micro-rebond résiduel
            if (fabsf(vn) < 0.15f)
                rb.linearVelocity = V3Sub(rb.linearVelocity, V3Scale(n, vn));
        }

        float linSq = V3LenSq(velForSleep);
        float angSq = V3LenSq(rb.angularVelocity);

        bool lowMotion = linSq < kLinearSleepThresholdSq
                      && angSq < kAngularSleepThresholdSq;

        if (lowMotion && rb.hasSupportContact)
        {
            rb.sleepTimer += dt;
            if (rb.sleepTimer >= kSleepTimeThreshold)
                rb.Sleep();
        }
        else
        {
            rb.sleepTimer = 0.0f;
        }

        // Reset support contact (doit être resoumis chaque frame par le solver)
        rb.hasSupportContact = false;
        rb.supportNormal     = {0,1,0};
    }
};
