#ifndef COLLIDER_COMPONENT_HPP_INCLUDED
#define COLLIDER_COMPONENT_HPP_INCLUDED

#include "define.h"
#include "../Core/Transform.h"

enum class ColliderType
{
    Box,
    Sphere
};

// Formes en espace local (définition géométrique, constante après init)
//
//  BoxShape    : défini par ses demi-extents locaux.
//                La rotation et la position viennent du TransformComponent
//                combiné avec localOffset / localRotation ci-dessous.
//
//  SphereShape : défini par son rayon local.
//                Le rayon monde = radius * max(scale.x, scale.y, scale.z).
//                On ne supporte pas les ellipsoïdes — une sphère scalée
//                non-uniformément doit devenir une box.
struct BoxShape
{
    XMFLOAT3 halfExtents = { 0.5f, 0.5f, 0.5f };
};

struct SphereShape
{
    float radius = 0.5f;
};

// Données monde en cache
//
//  Ces structures sont recalculées chaque frame par ColliderSystem.
//  Ne pas les modifier manuellement.
//
//  AABB : toujours valide, utilisée pour la broad phase (partition grid).
//  OBB  : valide uniquement pour ColliderType::Box.
//  Pour les sphères : seuls worldCenter et worldRadius sont utilisés.
struct AABB
{
    XMFLOAT3 min = { 0,0,0 };
    XMFLOAT3 max = { 0,0,0 };
};

struct OBB
{
    XMFLOAT3 center;
    XMFLOAT3 axes[3];       // axes orientés en espace monde (colonnes de la matrice de rotation)
    XMFLOAT3 halfExtents;   // demi-extents en espace monde (scalés)
};

// ColliderComponent
//
//  Séparation claire entre :
//   1. Définition locale (forme, offset) — set once, read-only après init.
//   2. Cache monde (aabb, obb, worldCenter, worldRadius) — écrit par ColliderSystem.
//
//  L'union garantit qu'on n'alloue que ce dont on a besoin selon le type.
//  Initialiser via SetBox() ou SetSphere() plutôt qu'en accédant à shape directement.
struct ColliderComponent
{
    //Définition de la forme
    ColliderType type = ColliderType::Box;

    union
    {
        BoxShape    box;
        SphereShape sphere;
    } shape;

    // Offset du collider par rapport à l'origine du TransformComponent.
    // Permet d'avoir un collider décentré (ex : collider de tête sur un personnage).
    XMFLOAT3 localOffset = { 0.0f, 0.0f, 0.0f };
    XMFLOAT4 localRotation = { 0.0f, 0.0f, 0.0f, 1.0f };  // quaternion identité

    // Flags
    // isTrigger : génère des événements OnEnter/OnExit mais pas de réponse physique.
    bool isTrigger = false;

    // Cache monde (écrit exclusivement par ColliderSystem::UpdateCollider)
    AABB     aabb;          // toujours valide — broad phase
    OBB      obb;           // valide si type == Box — narrow phase
    XMFLOAT3 worldCenter = { 0.0f, 0.0f, 0.0f };  // centre monde (box et sphere)
    float    worldRadius = 0.0f;                   // rayon monde (sphere uniquement)

    // Helpers d'initialisation
    inline void SetBox(const XMFLOAT3& _halfExtents)
    {
        type = ColliderType::Box;
        shape.box.halfExtents = _halfExtents;
    }

    inline void SetSphere(float _radius)
    {
        type = ColliderType::Sphere;
        shape.sphere.radius = _radius;
    }

    // Retourne les demi-extents de la box en espace monde (après scale du transform).
    // Uniquement valide après que ColliderSystem ait appelé UpdateCollider ce frame.
    inline const XMFLOAT3& GetWorldHalfExtents() const
    {
        return obb.halfExtents;
    }

    // Retourne le rayon en espace monde (après scale du transform).
    // Uniquement valide pour les sphères, après UpdateCollider.
    inline float GetWorldRadius() const
    {
        return worldRadius;
    }
};

#endif // !COLLIDER_COMPONENT_HPP_INCLUDED