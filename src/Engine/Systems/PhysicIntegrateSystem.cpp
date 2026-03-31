#include "PhysicIntegrateSystem.h"
#include <iostream>

namespace
{
    constexpr float kSleepLinearThreshold = 0.05f;
    constexpr float kSleepAngularThreshold = 0.05f;
    constexpr float kSleepTimeThreshold = 0.5f;

    constexpr float kMinAngularVelocitySq = 1e-6f;
}

void PhysicIntegrateSystem::OnStartUpdate(float _dt)
{
}

void PhysicIntegrateSystem::OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, ColliderComponent& _collider, TransformComponent& _transform)
{
    if (_physic.type != BodyType::Dynamic)
        return;

    if (_physic.isSleeping)
        return;

    // 1. Tenseur d'inertie
    // Le tenseur corps est recalculé ici car la forme ou la masse peuvent avoir
    // changé depuis la dernière frame (SetMass appelé depuis un script, etc.).
    // Pour des formes primaires (box, sphere) c'est une poignée de multiplications.
    ComputeBodyInertiaTensor(_physic, _collider);

    // Le tenseur monde est mis à jour uniquement si le corps peut tourner.
    // Pour un corps sans rotation, I_world_inv reste nul — le solver le sait.
    if (_physic.allowRotation)
        UpdateWorldInertiaTensor(_physic, _transform);

    // ─── 2. Intégration linéaire ──────────────────────────────────────────────
    XMFLOAT3 deltaPosition = IntegrateLinearVelocity(_physic, _dt);
    _transform.local.Move(deltaPosition);

    // ─── 3. Intégration angulaire ─────────────────────────────────────────────
    if (_physic.allowRotation)
    {
        XMFLOAT3 deltaAngle = IntegrateAngularVelocity(_physic, _dt);
        UpdateQuaternion(_transform, deltaAngle);
    }
    else
    {
        // Forcer à zéro : un autre système (ex: PhysicSystem) peut avoir
        // écrit une vitesse angulaire résiduelle sur un corps sans rotation.
        _physic.angularVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
        _physic.torque = XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    // ─── 4. Sleep ─────────────────────────────────────────────────────────────
    float linearSq = NormSquared(_physic.linearVelocity);
    float angularSq = NormSquared(_physic.angularVelocity);

    bool lowMotion =
        linearSq < (kSleepLinearThreshold * kSleepLinearThreshold) &&
        angularSq < (kSleepAngularThreshold * kSleepAngularThreshold);

    if (lowMotion)
    {
        _physic.sleepTimer += _dt;

        if (_physic.sleepTimer >= kSleepTimeThreshold)
        {
            _physic.Sleep();
        }
    }
    else
    {
        _physic.sleepTimer = 0.0f;
    }
}

void PhysicIntegrateSystem::OnEndUpdate(float _dt)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// Tenseurs d'inertie
// ─────────────────────────────────────────────────────────────────────────────

void PhysicIntegrateSystem::ComputeBodyInertiaTensor(PhysicComponent& _physic, ColliderComponent& _collider)
{
    if (!_physic.allowRotation)
    {
        _physic.inertiaTensorBody = XMMatrixIdentity();
        _physic.inertiaTensorBodyInverse = XMMatrixIdentity();
        return;
    }

    float m = _physic.mass;
    float ix = 0.0f, iy = 0.0f, iz = 0.0f;

    if (_collider.type == ColliderType::Box)
    {
        // Tenseur d'inertie d'un pavé droit de masse m,
        // demi-extents (hx, hy, hz) → dimensions réelles (2hx, 2hy, 2hz).
        const XMFLOAT3& h = _collider.shape.box.halfExtents;

        float w = h.x * 2.0f;   // largeur
        float ht = h.y * 2.0f;  // hauteur
        float d = h.z * 2.0f;   // profondeur

        ix = (1.0f / 12.0f) * m * (ht * ht + d * d);
        iy = (1.0f / 12.0f) * m * (w * w + d * d);
        iz = (1.0f / 12.0f) * m * (w * w + ht * ht);
    }
    else // Sphere
    {
        // Tenseur d'inertie d'une sphère pleine de masse m et rayon r.
        float r = _collider.shape.sphere.radius;
        float i = (2.0f / 5.0f) * m * r * r;
        ix = iy = iz = i;
    }

    // Matrice diagonale — XMMatrixScaling construit exactement ça.
    // La ligne/colonne W vaut 0 0 0 1, ce qui est correct pour un tenseur 3x3
    // stocké dans une XMMATRIX 4x4 (la composante W n'est jamais lue).
    _physic.inertiaTensorBody = XMMatrixScaling(ix, iy, iz);

    float invIx = (ix > 0.0f) ? 1.0f / ix : 0.0f;
    float invIy = (iy > 0.0f) ? 1.0f / iy : 0.0f;
    float invIz = (iz > 0.0f) ? 1.0f / iz : 0.0f;

    _physic.inertiaTensorBodyInverse = XMMatrixScaling(invIx, invIy, invIz);
}

void PhysicIntegrateSystem::UpdateWorldInertiaTensor(PhysicComponent& _physic, TransformComponent& _transform)
{
    // I_world     = R * I_body     * R^T
    // I_world_inv = R * I_body_inv * R^T
    //
    // XMMatrixTranspose est le moyen le moins coûteux d'obtenir R^T puisque
    // R est une matrice de rotation pure (orthonormale → R^T == R^-1).

    XMVECTOR q = XMLoadFloat4(&_transform.local.GetRotation());
    XMMATRIX R = XMMatrixRotationQuaternion(q);
    XMMATRIX RT = XMMatrixTranspose(R);

    _physic.inertiaTensorWorld = R * _physic.inertiaTensorBody * RT;
    _physic.inertiaTensorWorldInverse = R * _physic.inertiaTensorBodyInverse * RT;
}

// ─────────────────────────────────────────────────────────────────────────────
// Intégration linéaire
// ─────────────────────────────────────────────────────────────────────────────

XMFLOAT3 PhysicIntegrateSystem::IntegrateLinearVelocity(PhysicComponent& _physic, float _dt)
{
    // Accumulation des accélérations (on travaille en accélérations, pas en forces,
    // pour éviter de multiplier/diviser par la masse à chaque étape).
    XMFLOAT3 acceleration = { 0.0f, 0.0f, 0.0f };

    // Contribution des forces accumulées (F = ma → a = F/m)
    acceleration = Add(acceleration, Mul(_physic.force, _physic.massInverse));

    // Gravité : appliquée directement comme accélération, pas comme force,
    // ce qui la rend indépendante de la masse (comportement correct en physique).
    if (_physic.useGravity)
        acceleration = Add(acceleration, Mul(m_gravity, _physic.gravityScale));

    // Amortissement linéaire (modèle stable : facteur multiplicatif sur la vitesse).
    // Plus stable numériquement que d'ajouter une force de frein.
    float dampingFactor = 1.0f / (1.0f + _physic.linearDamping * _dt);

    // Euler symplectique : on met à jour la vitesse avant la position.
    _physic.linearVelocity = Mul(
        Add(_physic.linearVelocity, Mul(acceleration, _dt)),
        dampingFactor
    );

    // Remise à zéro des forces — les scripts réappliquent chaque frame ce dont ils ont besoin.
    _physic.force = XMFLOAT3(0.0f, 0.0f, 0.0f);

    return Mul(_physic.linearVelocity, _dt);
}

// ─────────────────────────────────────────────────────────────────────────────
// Intégration angulaire
// ─────────────────────────────────────────────────────────────────────────────

XMFLOAT3 PhysicIntegrateSystem::IntegrateAngularVelocity(PhysicComponent& _physic, float _dt)
{
    // α = I_world_inv · τ
    // On utilise XMVector3Transform pour multiplier le couple par le tenseur inverse monde.
    XMVECTOR torqueV = XMVectorSet(_physic.torque.x, _physic.torque.y, _physic.torque.z, 0.0f);
    XMVECTOR alphaV = XMVector3Transform(torqueV, _physic.inertiaTensorWorldInverse);

    XMFLOAT3 angularAcceleration;
    XMStoreFloat3(&angularAcceleration, alphaV);

    // Intégration de la vitesse angulaire.
    _physic.angularVelocity = Add(_physic.angularVelocity, Mul(angularAcceleration, _dt));

    // Amortissement angulaire (même modèle que linéaire).
    float dampingFactor = 1.0f / (1.0f + _physic.angularDamping * _dt);
    _physic.angularVelocity = Mul(_physic.angularVelocity, dampingFactor);

    // Seuil numérique : en dessous on force à zéro pour éviter la dérive.
    if (NormSquared(_physic.angularVelocity) < kMinAngularVelocitySq)
        _physic.angularVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

    _physic.torque = XMFLOAT3(0.0f, 0.0f, 0.0f);

    return Mul(_physic.angularVelocity, _dt);
}

// ─────────────────────────────────────────────────────────────────────────────
// Mise à jour du quaternion
// ─────────────────────────────────────────────────────────────────────────────

void PhysicIntegrateSystem::UpdateQuaternion(TransformComponent& _transform, const XMFLOAT3& _deltaAngle)
{
    // On crée un quaternion représentant la rotation différentielle de cette frame.
    // XMQuaternionRotationRollPitchYaw est correct pour de petits angles (ce que
    // _deltaAngle est toujours à 60Hz avec des vitesses angulaires raisonnables).
    XMVECTOR qCurrent = XMLoadFloat4(&_transform.local.GetRotation());
    XMVECTOR qDelta = XMQuaternionRotationRollPitchYaw(_deltaAngle.x, _deltaAngle.y, _deltaAngle.z);

    // Composition : qNew = qDelta * qCurrent (ordre important — la rotation delta
    // est appliquée dans l'espace monde, donc elle se multiplie à gauche).
    XMVECTOR qNew = XMQuaternionNormalize(XMQuaternionMultiply(qDelta, qCurrent));

    XMFLOAT4 out;
    XMStoreFloat4(&out, qNew);
    _transform.local.SetRotationQuaternion(out);
}