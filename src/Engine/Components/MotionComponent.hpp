#ifndef MOTION_COMPONENT_HPP_DEFINED
#define MOTION_COMPONENT_HPP_DEFINED

#include "define.h"

// ─────────────────────────────────────────────────────────────────────────────
// MotionComponent
//
//  Contient l'état cinématique du corps : vitesses, accumulateurs de forces,
//  et gestion du sleep.
//
//  Séparé de RigidBodyComponent intentionnellement :
//   - RigidBodyComponent = propriétés quasi-statiques (masse, matériau, type)
//   - MotionComponent    = état dynamique qui change chaque frame
//
//  Cette séparation permet de récupérer uniquement les vitesses sans charger
//  les tenseurs, ce qui est utile pour le character controller et les triggers.
//
//  Note sur le sleep :
//  Un corps s'endort quand sa vitesse linéaire et angulaire sont toutes deux
//  sous le seuil pendant kSleepTimeThreshold secondes consécutives.
//  Un corps endormi est skippé par l'intégrateur et le solver — mais il
//  se réveille dès qu'un contact le touche (géré par PhysicSystem).
// ─────────────────────────────────────────────────────────────────────────────
struct MotionComponent
{
    // ─── État cinématique ─────────────────────────────────────────────────────
    XMFLOAT3 linearVelocity  = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 angularVelocity = { 0.0f, 0.0f, 0.0f };

    // ─── Accumulateurs (remis à zéro après intégration) ───────────────────────
    XMFLOAT3 force  = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 torque = { 0.0f, 0.0f, 0.0f };

    // ─── Sleep ────────────────────────────────────────────────────────────────
    bool  isSleeping = false;
    float sleepTimer = 0.0f;

    // ─────────────────────────────────────────────────────────────────────────
    // API publique — utilisée par les scripts et systèmes gameplay
    // ─────────────────────────────────────────────────────────────────────────

    // Impulsion instantanée (changement de vitesse immédiat, indépendant du dt).
    // Usage : saut, coup, explosion.
    void AddLinearImpulse(const XMFLOAT3& _impulse)
    {
        linearVelocity.x += _impulse.x;
        linearVelocity.y += _impulse.y;
        linearVelocity.z += _impulse.z;
        WakeUp();
    }

    void AddAngularImpulse(const XMFLOAT3& _impulse)
    {
        angularVelocity.x += _impulse.x;
        angularVelocity.y += _impulse.y;
        angularVelocity.z += _impulse.z;
        WakeUp();
    }

    // Force continue (appliquée via F=ma dans l'intégrateur, réinitialisée chaque frame).
    // Usage : moteur, vent, poussée.
    void AddForce(const XMFLOAT3& _force)
    {
        force.x += _force.x;
        force.y += _force.y;
        force.z += _force.z;
        WakeUp();
    }

    // Torque continu (analogue à AddForce pour la rotation).
    void AddTorque(const XMFLOAT3& _torque)
    {
        torque.x += _torque.x;
        torque.y += _torque.y;
        torque.z += _torque.z;
        WakeUp();
    }

    // Vitesse au point _r (r = point - centerOfMass).
    // Utilisé par le solver pour calculer la vitesse relative aux contacts.
    XMFLOAT3 GetVelocityAtPoint(const XMFLOAT3& _r) const
    {
        // v_point = linearVelocity + angularVelocity × r
        return
        {
            linearVelocity.x + (angularVelocity.y * _r.z - angularVelocity.z * _r.y),
            linearVelocity.y + (angularVelocity.z * _r.x - angularVelocity.x * _r.z),
            linearVelocity.z + (angularVelocity.x * _r.y - angularVelocity.y * _r.x)
        };
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Sleep
    // ─────────────────────────────────────────────────────────────────────────

    void WakeUp()
    {
        isSleeping = false;
        sleepTimer = 0.0f;
    }

    void Sleep()
    {
        isSleeping      = true;
        sleepTimer      = 0.0f;
        linearVelocity  = { 0.0f, 0.0f, 0.0f };
        angularVelocity = { 0.0f, 0.0f, 0.0f };
        force           = { 0.0f, 0.0f, 0.0f };
        torque          = { 0.0f, 0.0f, 0.0f };
    }
};

#endif // !MOTION_COMPONENT_HPP_DEFINED
