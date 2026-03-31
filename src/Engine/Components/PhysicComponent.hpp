#ifndef PHYSIC_COMPONENT_HPP_DEFINED
#define PHYSIC_COMPONENT_HPP_DEFINED

#include "define.h"
#include "../Core/Utils.hpp"

enum class BodyType
{
    Static,
    Dynamic,
    Kinematic
};

// PhysicComponent
//
//  Remarques sur les tenseurs d'inertie :
//
//  Le tenseur d'inertie est une matrice 3x3 qui relie le couple angulaire à
//  l'accélération angulaire : τ = I · α.
//
//  On stocke deux paires :
//   - Body space  (constant, calculé une fois à l'init selon la forme du collider)
//   - World space (recalculé chaque frame quand allowRotation == true)
//                  I_world     = R * I_body     * R^T
//                  I_world_inv = R * I_body_inv * R^T
//
//  Le solver n'utilise QUE inertiaTensorWorldInverse.
//  Le body space est conservé pour le recalcul world à chaque frame.
//
//  On utilise XMMATRIX (4x4) pour profiter de l'alignement SIMD de DirectXMath.
//  Seule la sous-matrice 3x3 supérieure gauche est significative.
//
//  ATTENTION : XMMATRIX requiert un alignement 16 octets. Ce composant doit
//  être alloué dans un pool aligné (alignas(16) ou allocation via _mm_malloc).
struct alignas(16) PhysicComponent
{
    // Masse 
    float mass = 1.0f;
    float massInverse = 1.0f;   // 0 pour Static et Kinematic

    // Tenseur d'inertie
    // Espace corps —> constant après init, reflète la géométrie du collider.
    XMMATRIX inertiaTensorBody;
    XMMATRIX inertiaTensorBodyInverse;

    // Espace monde -> mis à jour par PhysicIntegrateSystem chaque frame.
    // Seule cette paire est lue par PhysicSystem (solver).
    XMMATRIX inertiaTensorWorld;
    XMMATRIX inertiaTensorWorldInverse;

    // État cinématique
    XMFLOAT3 linearVelocity = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 angularVelocity = { 0.0f, 0.0f, 0.0f };

    // Accumulateurs (remis à zéro après intégration)
    XMFLOAT3 force = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 torque = { 0.0f, 0.0f, 0.0f };

    // Propriétés matériau
    float restitution = 0.0f;
    float staticFriction = 0.6f;
    float dynamicFriction = 0.4f;

    // Amortissement par corps
    // Permet d'avoir des comportements différents par entité (eau, air, vide).
    // 0 = aucun amortissement. Valeurs typiques : 0.05 – 0.5.
    float linearDamping = 0.05f;
    float angularDamping = 0.1f;

    // Multiplicateur de gravité 
    // 0 = pas de gravité, 1 = gravité normale, -1 = gravité inversée.
    float gravityScale = 1.0f;

    // Configuration
    BodyType type = BodyType::Dynamic;
    bool     useGravity = false;
    bool     allowRotation = false;

    // Sleep
    bool  isSleeping = false;
    float sleepTimer = 0.0f;

    // Méthodes utilitaires

    inline void SetMass(float _mass)
    {
        mass = _mass;
        massInverse = (_mass > 0.0f) ? 1.0f / _mass : 0.0f;
    }

    // Configure le corps comme statique.
    // Tous les tenseurs sont mis à zéro : le solver ne leur applique rien.
    inline void SetStatic()
    {
        type = BodyType::Static;
        massInverse = 0.0f;

        inertiaTensorBody = XMMatrixIdentity();
        inertiaTensorBodyInverse = XMMatrixIdentity();
        inertiaTensorWorld = XMMatrixIdentity();
        inertiaTensorWorldInverse = XMMatrixIdentity();

        Sleep();
    }

    // Configure le corps comme kinematic.
    // Pas de simulation, mais génère des contacts côté solver.
    inline void SetKinematic()
    {
        type = BodyType::Kinematic;
        massInverse = 0.0f;

        inertiaTensorBody = XMMatrixIdentity();
        inertiaTensorBodyInverse = XMMatrixIdentity();
        inertiaTensorWorld = XMMatrixIdentity();
        inertiaTensorWorldInverse = XMMatrixIdentity();

        isSleeping = false;
    }

    inline void AddForce(const XMFLOAT3& _f)
    {
        force = Add(force, _f);
        WakeUp();
    }

    inline void AddForceAtPoint(const XMFLOAT3& _f, const XMFLOAT3& _point, const XMFLOAT3& _center)
    {
        force = Add(force, _f);

        XMFLOAT3 r = Subtract(_point, _center);
        torque = Add(torque, Cross(r, _f));

        WakeUp();
    }

    inline void AddTorque(const XMFLOAT3& _t)
    {
        torque = Add(torque, _t);
        WakeUp();
    }

    inline void AddLinearVelocity(const XMFLOAT3& _v)
    {
        linearVelocity = Add(linearVelocity, _v);
        WakeUp();
    }

    inline void AddAngularVelocity(const XMFLOAT3& _v)
    {
        angularVelocity = Add(angularVelocity, _v);
        WakeUp();
    }

    inline void WakeUp()
    {
        isSleeping = false;
        sleepTimer = 0.0f;
    }

    inline void Sleep()
    {
        isSleeping = true;
        sleepTimer = 0.0f;
        linearVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
        angularVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
        force = XMFLOAT3(0.0f, 0.0f, 0.0f);
        torque = XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    // Retourne la vitesse en un point du corps (vitesse linéaire + contribution angulaire).
    // _r = point - centerOfMass
    inline XMFLOAT3 GetVelocityAtPoint(const XMFLOAT3& _r) const
    {
        return Add(linearVelocity, Cross(angularVelocity, _r));
    }
};

#endif // !PHYSIC_COMPONENT_HPP_DEFINED