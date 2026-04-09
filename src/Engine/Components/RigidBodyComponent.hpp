#ifndef RIGID_BODY_COMPONENT_HPP_DEFINED
#define RIGID_BODY_COMPONENT_HPP_DEFINED

#include "define.h"

enum class BodyType : uint8
{
    Static,
    Dynamic,
    Kinematic
};

struct RigidBodyComponent
{
    // Masse
    float mass        = 1.0f;
    float massInverse = 1.0f;   // 0 pour Static et Kinematic

    // Tenseur d'inertie
    // Espace corps (constant après init, calculé depuis la géométrie du shape).
    float inertiaTensorBody[9]        = { 1,0,0, 0,1,0, 0,0,1 };
    float inertiaTensorBodyInverse[9] = { 1,0,0, 0,1,0, 0,0,1 };

    // Espace monde (recalculé chaque frame par PhysicIntegrateSystem).
    // I_world_inv = R * I_body_inv * R^T
    float inertiaTensorWorldInverse[9] = { 1,0,0, 0,1,0, 0,0,1 };

    bool inertiaDirty = true;

    // Propriétés matériau 
    float restitution     = 0.4f;   // 0 = parfaitement inélastique, 1 = parfaitement élastique
    float staticFriction  = 0.1f;
    float dynamicFriction = 0.05f;

    // Configuration
    BodyType type          = BodyType::Dynamic;
    bool     allowRotation = true;
    bool     useGravity    = true;
    float    gravityScale  = 1.0f;

    // Amortissement
    // Modèle multiplicatif : 1 / (1 + damping * dt). Stable inconditionnellement.
    // 0 = aucun amortissement. Valeurs typiques : 0.05 – 0.3.
    float linearDamping  = 0.15f;
    float angularDamping = 0.2f;
    float dragCoefficient = 0.1f;

    void SetMass(float _mass)
    {
        mass        = _mass;
        massInverse = (_mass > 0.0f) ? 1.0f / _mass : 0.0f;
		inertiaDirty = true;
    }

    void SetStatic()
    {
        type        = BodyType::Static;
        massInverse = 0.0f;
		inertiaDirty = true;
        ZeroTensors();
    }

    void SetKinematic()
    {
        type        = BodyType::Kinematic;
        massInverse = 0.0f;
        inertiaDirty = true;
        ZeroTensors();
    }

    // Initialise un tenseur diagonal (cas box/sphère).
    void SetDiagonalInertiaTensor(float _ixx, float _iyy, float _izz)
    {
        // Body
        float* b = inertiaTensorBody;
        b[0]=_ixx; b[1]=0;    b[2]=0;
        b[3]=0;    b[4]=_iyy; b[5]=0;
        b[6]=0;    b[7]=0;    b[8]=_izz;

        // Body inverse
        float* bi = inertiaTensorBodyInverse;
        bi[0] = (_ixx > 0.0f) ? 1.0f / _ixx : 0.0f;
        bi[1] = 0; bi[2] = 0; bi[3] = 0;
        bi[4] = (_iyy > 0.0f) ? 1.0f / _iyy : 0.0f;
        bi[5] = 0; bi[6] = 0; bi[7] = 0;
        bi[8] = (_izz > 0.0f) ? 1.0f / _izz : 0.0f;

        inertiaDirty = true;
    }

    // Met tous les tenseurs à zéro (Static/Kinematic).
    void ZeroTensors()
    {
        for (int i = 0; i < 9; ++i)
        {
            inertiaTensorBody[i]        = 0.0f;
            inertiaTensorBodyInverse[i] = 0.0f;
            inertiaTensorWorldInverse[i] = 0.0f;
        }
    }
};

#endif // !RIGID_BODY_COMPONENT_HPP_DEFINED
