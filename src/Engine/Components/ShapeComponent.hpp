#ifndef SHAPE_COMPONENT_HPP_DEFINED
#define SHAPE_COMPONENT_HPP_DEFINED

#include "define.h"

// ─────────────────────────────────────────────────────────────────────────────
// ShapeType
//
//  Box     : OBB défini par ses demi-extents locaux.
//  Sphere  : sphère définie par un rayon local.
//  Capsule : cylindre avec deux hémisphères. Indispensable pour le character
//            controller (évite les coins qui accrochent sur les marches).
//            Défini par un rayon et une demi-hauteur (du centre à la base
//            d'un hémisphère).
// ─────────────────────────────────────────────────────────────────────────────
enum class ShapeType : uint8
{
    Box,
    Sphere,
    Capsule
};


// ─────────────────────────────────────────────────────────────────────────────
// Données de forme locales
//
//  Ces structs définissent la géométrie en espace corps (avant transform).
//  Elles sont utilisées par GJK via la fonction Support() du système.
// ─────────────────────────────────────────────────────────────────────────────
struct BoxShape
{
    XMFLOAT3 halfExtents = { 0.5f, 0.5f, 0.5f };
};

struct SphereShape
{
    float radius = 0.5f;
};

struct CapsuleShape
{
    float radius     = 0.5f;    // rayon du cylindre et des hémisphères
    float halfHeight = 1.0f;    // demi-hauteur du segment central (hors hémisphères)
    // La hauteur totale = 2 * halfHeight + 2 * radius
    // L'axe de la capsule est l'axe Y local
};


// ─────────────────────────────────────────────────────────────────────────────
// Cache AABB monde
//
//  Recalculé chaque frame par BroadPhaseSystem.
//  Utilisé uniquement pour la broad phase — ne pas lire dans le solver.
// ─────────────────────────────────────────────────────────────────────────────
struct AABB
{
    XMFLOAT3 min = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 max = { 0.0f, 0.0f, 0.0f };

    bool Overlaps(const AABB& _other) const
    {
        return
            min.x <= _other.max.x && max.x >= _other.min.x &&
            min.y <= _other.max.y && max.y >= _other.min.y &&
            min.z <= _other.max.z && max.z >= _other.min.z;
    }

    XMFLOAT3 Center() const
    {
        return
        {
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f,
            (min.z + max.z) * 0.5f
        };
    }
};


// ─────────────────────────────────────────────────────────────────────────────
// ShapeComponent
//
//  Structure principale. Sépare clairement :
//   1. Définition locale (shape, offset) — set once, lecture seule après init.
//   2. Cache monde (aabb, worldCenter, worldRadius) — écrit par BroadPhaseSystem.
//
//  Pourquoi une union ?
//  On n'alloue que ce dont on a besoin selon le type. Une capsule et une sphère
//  ne partagent pas la même structure. L'union garantit qu'un ShapeComponent
//  reste une taille fixe et prévisible pour le stockage ECS.
//
//  Pourquoi pas std::variant ?
//  std::variant ajoute un overhead de type et des constructeurs/destructeurs
//  non triviaux qui compliquent le stockage en Vector<Byte>. L'union POD est
//  plus adaptée aux composants ECS.
//
//  Note sur GJK :
//  GJK a besoin d'une seule chose par forme : une fonction Support(direction)
//  qui retourne le point le plus loin dans cette direction. Cette fonction
//  est implémentée dans NarrowPhaseSystem, pas ici. Ce composant est
//  purement des données.
// ─────────────────────────────────────────────────────────────────────────────
struct ShapeComponent
{
    // ─── Définition de la forme ───────────────────────────────────────────────
    ShapeType type = ShapeType::Box;

    union ShapeData
    {
        BoxShape     box;
        SphereShape  sphere;
        CapsuleShape capsule;

        ShapeData() : box() {}
    } shape;

    // Offset du collider par rapport à l'origine du TransformComponent.
    // Permet un collider décentré (tête d'un personnage, collider de véhicule).
    XMFLOAT3 localOffset   = { 0.0f, 0.0f, 0.0f };
    XMFLOAT4 localRotation = { 0.0f, 0.0f, 0.0f, 1.0f };  // quaternion identité

    // ─── Flags ────────────────────────────────────────────────────────────────
    bool isTrigger = false; // isTrigger : génère des événements OnEnter/OnExit mais pas de réponse physique.
    bool toHash = true; 

    // ─── Cache monde (écrit exclusivement par BroadPhaseSystem) ──────────────
    AABB     aabb;
    XMFLOAT3 worldCenter = { 0.0f, 0.0f, 0.0f };
    float    worldRadius = 0.0f;   // rayon de la sphere englobante (broad phase)

    // ─────────────────────────────────────────────────────────────────────────
    // Helpers d'initialisation
    // ─────────────────────────────────────────────────────────────────────────

    void SetBox(const XMFLOAT3& _halfExtents)
    {
        type                   = ShapeType::Box;
        shape.box.halfExtents  = _halfExtents;
    }

    void SetBox(float _hx, float _hy, float _hz)
    {
        SetBox({ _hx, _hy, _hz });
    }

    void SetSphere(float _radius)
    {
        type                  = ShapeType::Sphere;
        shape.sphere.radius   = _radius;
    }

    void SetCapsule(float _radius, float _halfHeight)
    {
        type                       = ShapeType::Capsule;
        shape.capsule.radius       = _radius;
        shape.capsule.halfHeight   = _halfHeight;
    }

    // Retourne le rayon de la sphère englobante locale (avant scale).
    // Utilisé pour initialiser worldRadius dans BroadPhaseSystem.
    float GetLocalBoundingRadius() const
    {
        switch (type)
        {
            case ShapeType::Box:
            {
                const XMFLOAT3& h = shape.box.halfExtents;
                return sqrtf(h.x*h.x + h.y*h.y + h.z*h.z);
            }
            case ShapeType::Sphere:
                return shape.sphere.radius;

            case ShapeType::Capsule:
                return shape.capsule.halfHeight + shape.capsule.radius;
        }
        return 0.0f;
    }
};

#endif // !SHAPE_COMPONENT_HPP_DEFINED
