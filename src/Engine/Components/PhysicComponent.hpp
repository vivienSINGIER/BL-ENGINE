#ifndef PHYSIC_COMPONENT_HPP_DEFINED
#define PHYSIC_COMPONENT_HPP_DEFINED

#include "define.h"

enum class BodyType
{
	Static,
	Dynamic
};

struct PhysicComponent
{
    float mass = 1.0f;
    float massInverse = 1.0f;

    XMFLOAT3 velocity = { 0,0,0 };
    XMFLOAT3 acceleration = { 0,0,0 };

    XMFLOAT3 forces = { 0,0,0 };

    BodyType type;
    bool useGravity = false;
};

#endif // !PHYSIC_COMPONENT_HPP_DEFINED



//Faire une résolution de pénétration simple
//Avant même les impulsions, tu peux faire une correction de position pour empêcher les objets de rester imbriqués.
//Principe :
//si deux objets se chevauchent on les sépare selon la normale de collision.
//
//Exemple simple :
//
//si A et B dynamiques : chacun bouge de moitié
//si A statique et B dynamique : seul B bouge
//
//Conceptuellement :
//
//correction = normal * penetration;
//
//Puis réparti selon les masses inverses.
//Ça évite l’effet “objets collés dans le sol”.