#include "PhysicIntegrateSystem.h"
#include "../Core/Utils.hpp"

void PhysicIntegrateSystem::OnUpdate(float _dt, EntityId _e, RigidBodyComponent& _rigid, MotionComponent& _motion, ColliderComponent& _shape, TransformComponent& _transform)
{
    if (_rigid.type != BodyType::Dynamic) return;
    if (_motion.isSleeping) return;

    // Recalcul uniquement si nécessaire.
    if (_rigid.inertiaDirty)
    {
        ComputeBodyInertiaTensor(_rigid, _shape);
        _rigid.inertiaDirty = false;
    }

    if (_rigid.allowRotation)
        UpdateWorldInertiaTensor(_rigid, _transform);

    // Intégration normale
    XMFLOAT3 deltaPos = IntegrateLinearVelocity(_rigid, _motion, _dt);
    _transform.local.Move(deltaPos);

    if (_rigid.allowRotation)
    {
        XMFLOAT3 deltaAngle = IntegrateAngularVelocity(_rigid, _motion, _dt);
        UpdateQuaternion(_transform, deltaAngle);
    }
    else
    {
        _motion.angularVelocity = { 0,0,0 };
        _motion.torque = { 0,0,0 };
    }

    // Sleep
	float linearSq = NormSquared(_motion.linearVelocity);
	float angularSq = NormSquared(_motion.angularVelocity);

    bool lowMotion = linearSq < (kSleepLinearThreshold) && angularSq < (kSleepAngularThreshold);

    if (lowMotion) 
    { 
        _motion.sleepTimer += _dt;

        if (_motion.sleepTimer >= kSleepTimeThreshold)
            _motion.Sleep();
    }
    else 
    {
        _motion.sleepTimer = 0.0f; 
    }
}

void PhysicIntegrateSystem::ComputeBodyInertiaTensor(RigidBodyComponent& _rigid, ColliderComponent& _shape)
{
    if (!_rigid.allowRotation)
    {
        _rigid.ZeroTensors();
        return;
    }

    float m = _rigid.mass;
    float ixx = 0.0f, iyy = 0.0f, izz = 0.0f;

    switch (_shape.type)
    {
        case ShapeType::Box:
        {
            const XMFLOAT3& h = _shape.shape.box.halfExtents;
            float w = h.x * 2.0f;
            float ht = h.y * 2.0f;
            float d  = h.z * 2.0f;

            ixx = (1.0f / 12.0f) * m * (ht*ht + d*d);
            iyy = (1.0f / 12.0f) * m * (w*w  + d*d);
            izz = (1.0f / 12.0f) * m * (w*w  + ht*ht);
            break;
        }

        case ShapeType::Sphere:
        {
            float r = _shape.shape.sphere.radius;
            ixx = iyy = izz = (2.0f / 5.0f) * m * r * r;
            break;
        }

        case ShapeType::Capsule:
        {
            float r  = _shape.shape.capsule.radius;
            float hh = _shape.shape.capsule.halfHeight;
            float h  = hh * 2.0f; // hauteur du cylindre central

            // Répartition de la masse entre cylindre et deux hémisphères.
            float volCyl = 3.14159265f * r * r * h;
            float volSph = (4.0f / 3.0f) * 3.14159265f * r * r * r;
            float totalVol = volCyl + volSph;

            float massCyl = (totalVol > 0.0f) ? m * volCyl / totalVol : 0.0f;
            float massSph = m - massCyl;

            // Cylindre.
            float ixx_cyl = (massCyl / 12.0f) * (3.0f * r*r + h*h);
            float iyy_cyl = (massCyl / 2.0f)  * r * r;

            // Chaque hémisphère via Huygens (décalé de hh + 3r/8 du centre).
            float d_sph   = hh + (3.0f * r) / 8.0f;
            float ixx_sph = (2.0f / 5.0f) * (massSph * 0.5f) * r*r
                          + (massSph * 0.5f) * d_sph * d_sph;
            float iyy_sph = (2.0f / 5.0f) * (massSph * 0.5f) * r*r;

            ixx = izz = ixx_cyl + 2.0f * ixx_sph;
            iyy       = iyy_cyl + 2.0f * iyy_sph;
            break;
        }
    }

    _rigid.SetDiagonalInertiaTensor(ixx, iyy, izz);
}

void PhysicIntegrateSystem::UpdateWorldInertiaTensor(RigidBodyComponent& _rigid,  TransformComponent& _transform)
{
    const float* bi = _rigid.inertiaTensorBodyInverse;

    XMMATRIX I_body_inv(
        bi[0], bi[1], bi[2], 0,
        bi[3], bi[4], bi[5], 0,
        bi[6], bi[7], bi[8], 0,
        0,     0,     0,     1
    );

    XMVECTOR q   = XMLoadFloat4(&_transform.local.GetRotation());
    XMMATRIX R   = XMMatrixRotationQuaternion(q);
    XMMATRIX RT  = XMMatrixTranspose(R);

    XMMATRIX I_world_inv = R * I_body_inv * RT;

    XMFLOAT4X4 tmp;
    XMStoreFloat4x4(&tmp, I_world_inv);

    float* wi = _rigid.inertiaTensorWorldInverse;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            wi[r*3+c] = tmp.m[r][c];
}

XMFLOAT3 PhysicIntegrateSystem::IntegrateLinearVelocity(RigidBodyComponent& _rigid, MotionComponent& _motion, float _dt)
{
    XMFLOAT3 accel = { 0.0f, 0.0f, 0.0f };

    // F = ma → a = F * (1/m)
    accel.x += _motion.force.x * _rigid.massInverse;
    accel.y += _motion.force.y * _rigid.massInverse;
    accel.z += _motion.force.z * _rigid.massInverse;

    // Gravité : accélération directe
    if (_rigid.useGravity)
    {
        accel.x += m_gravity.x * _rigid.gravityScale;
        accel.y += m_gravity.y * _rigid.gravityScale;
        accel.z += m_gravity.z * _rigid.gravityScale;
    }

    // Intégration vitesse
    _motion.linearVelocity.x += accel.x * _dt;
    _motion.linearVelocity.y += accel.y * _dt;
    _motion.linearVelocity.z += accel.z * _dt;

    // Amortissement linéaire.
    float damp = 1.0f / (1.0f + _rigid.linearDamping * _dt);
    _motion.linearVelocity.x *= damp;
    _motion.linearVelocity.y *= damp;
    _motion.linearVelocity.z *= damp;

    // Résistance de l'air
    // F_drag = -dragCoefficient * |v| * v
    if (_rigid.dragCoefficient > 0.0f)
    {
		float speed = sqrt(NormSquared(_motion.linearVelocity));

        if (speed > 0.1f)
        {
            // F_drag * (1/m) = -drag * |v| * v * massInverse
            float dragAccel = _rigid.dragCoefficient * speed * _rigid.massInverse;

            float dragDamp = 1.0f / (1.0f + dragAccel * _dt);
            _motion.linearVelocity.x *= dragDamp;
            _motion.linearVelocity.y *= dragDamp;
            _motion.linearVelocity.z *= dragDamp;
        }
    }

    // Remise à zéro des forces — les scripts réappliquent chaque frame.
    _motion.force = { 0.0f, 0.0f, 0.0f };

    return
    {
        _motion.linearVelocity.x * _dt,
        _motion.linearVelocity.y * _dt,
        _motion.linearVelocity.z * _dt
    };
}

XMFLOAT3 PhysicIntegrateSystem::IntegrateAngularVelocity(RigidBodyComponent& _rigid, MotionComponent& _motion, float _dt)
{
    // α = I_world_inv · τ
    const float* wi = _rigid.inertiaTensorWorldInverse;
    XMFLOAT3 alpha =
    {
        wi[0]*_motion.torque.x + wi[1]*_motion.torque.y + wi[2]*_motion.torque.z,
        wi[3]*_motion.torque.x + wi[4]*_motion.torque.y + wi[5]*_motion.torque.z,
        wi[6]*_motion.torque.x + wi[7]*_motion.torque.y + wi[8]*_motion.torque.z
    };

    _motion.angularVelocity.x += alpha.x * _dt;
    _motion.angularVelocity.y += alpha.y * _dt;
    _motion.angularVelocity.z += alpha.z * _dt;

    // Amortissement angulaire.
    float damp = 1.0f / (1.0f + _rigid.angularDamping * _dt);
    _motion.angularVelocity.x *= damp;
    _motion.angularVelocity.y *= damp;
    _motion.angularVelocity.z *= damp;

    // Seuil numérique — évite la dérive flottante sur les corps quasi-immobiles.
	float aSq = NormSquared(_motion.angularVelocity);
    if (aSq < kMinAngularVelocitySq)
        _motion.angularVelocity = { 0.0f, 0.0f, 0.0f };

    _motion.torque = { 0.0f, 0.0f, 0.0f };

    return
    {
        _motion.angularVelocity.x * _dt,
        _motion.angularVelocity.y * _dt,
        _motion.angularVelocity.z * _dt
    };
}

void PhysicIntegrateSystem::UpdateQuaternion(TransformComponent& _transform, const XMFLOAT3& _deltaAngle)
{
    XMVECTOR qCurrent = XMLoadFloat4(&_transform.local.GetRotation());
    XMVECTOR qDelta   = XMQuaternionRotationRollPitchYaw(_deltaAngle.x, _deltaAngle.y, _deltaAngle.z);
    XMVECTOR qNew     = XMQuaternionNormalize(XMQuaternionMultiply(qDelta, qCurrent));

    XMFLOAT4 out;
    XMStoreFloat4(&out, qNew);
    _transform.local.SetRotationQuaternion(out);
}
