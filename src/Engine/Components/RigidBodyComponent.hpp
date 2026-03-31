#ifndef RIGID_BODY_COMPONENT_HPP_DEFINED
#define RIGID_BODY_COMPONENT_HPP_DEFINED

#include "define.h"

// ─────────────────────────────────────────────────────────────────────────────
// BodyType
//
//  Static    : masse infinie, ne bouge jamais. massInverse = 0.
//  Dynamic   : simulé complètement.
//  Kinematic : déplacé manuellement (plateformes, portes). Génère des contacts
//              comme un corps solide mais n'est pas affecté par les impulsions.
//              massInverse = 0, inertiaTensorInvBody = 0.
// ─────────────────────────────────────────────────────────────────────────────
enum class BodyType : uint8
{
    Static,
    Dynamic,
    Kinematic
};


// ─────────────────────────────────────────────────────────────────────────────
// RigidBodyComponent
//
//  Contient les propriétés invariantes ou quasi-invariantes du corps :
//   - masse et son inverse
//   - tenseur d'inertie en espace corps (calculé une fois à l'init)
//   - tenseur d'inertie en espace monde (recalculé chaque frame par le système)
//   - propriétés matériau (friction, restitution)
//
//  Pourquoi float[9] plutôt que XMMATRIX ?
//  Les composants sont stockés dans Vector<Byte> par l'ECS. XMMATRIX requiert
//  un alignement 16 octets que le vecteur ne garantit pas. On stocke donc la
//  matrice 3x3 row-major en float[9] et on la charge dans une XMMATRIX locale
//  uniquement dans les systèmes qui en ont besoin.
//
//  Convention float[9] : indices [row * 3 + col].
//   [0][1][2]
//   [3][4][5]
//   [6][7][8]
//
//  Pour une forme primaire (box, sphère), le tenseur corps est diagonal :
//   [Ixx  0    0  ]
//   [0    Iyy  0  ]
//   [0    0    Izz]
//  → seuls les indices 0, 4, 8 sont non nuls.
// ─────────────────────────────────────────────────────────────────────────────
struct RigidBodyComponent
{
    // ─── Masse ────────────────────────────────────────────────────────────────
    float mass        = 1.0f;
    float massInverse = 1.0f;   // 0 pour Static et Kinematic

    // ─── Tenseur d'inertie ────────────────────────────────────────────────────
    // Espace corps (constant après init, calculé depuis la géométrie du shape).
    float inertiaTensorBody[9]        = { 1,0,0, 0,1,0, 0,0,1 };
    float inertiaTensorBodyInverse[9] = { 1,0,0, 0,1,0, 0,0,1 };

    // Espace monde (recalculé chaque frame par PhysicIntegrateSystem).
    // I_world_inv = R * I_body_inv * R^T
    float inertiaTensorWorldInverse[9] = { 1,0,0, 0,1,0, 0,0,1 };

    // ─── Propriétés matériau ──────────────────────────────────────────────────
    float restitution     = 0.0f;   // 0 = parfaitement inélastique, 1 = parfaitement élastique
    float staticFriction  = 0.6f;
    float dynamicFriction = 0.4f;

    // ─── Configuration ────────────────────────────────────────────────────────
    BodyType type          = BodyType::Dynamic;
    bool     allowRotation = true;
    bool     useGravity    = true;
    float    gravityScale  = 1.0f;

    // ─── Amortissement ────────────────────────────────────────────────────────
    // Modèle multiplicatif : 1 / (1 + damping * dt). Stable inconditionnellement.
    // 0 = aucun amortissement. Valeurs typiques : 0.05 – 0.3.
    float linearDamping  = 0.05f;
    float angularDamping = 2.0f;

    // ─────────────────────────────────────────────────────────────────────────
    // Helpers d'initialisation
    // ─────────────────────────────────────────────────────────────────────────

    void SetMass(float _mass)
    {
        mass        = _mass;
        massInverse = (_mass > 0.0f) ? 1.0f / _mass : 0.0f;
    }

    void SetStatic()
    {
        type        = BodyType::Static;
        massInverse = 0.0f;
        ZeroTensors();
    }

    void SetKinematic()
    {
        type        = BodyType::Kinematic;
        massInverse = 0.0f;
        ZeroTensors();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Helpers tenseur
    // ─────────────────────────────────────────────────────────────────────────

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
