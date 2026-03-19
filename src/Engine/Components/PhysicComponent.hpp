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
    float invMass = 1.0f;

    XMFLOAT3 velocity = { 0,0,0 };
    XMFLOAT3 acceleration = { 0,0,0 };

    XMFLOAT3 forces = { 0,0,0 };

    bool isStatic = false;
    bool useGravity = true;
};

#endif // !PHYSIC_COMPONENT_HPP_DEFINED














//2. Séparer les rôles dans les components
//
//Tu as déjà la partie collision.
//Il te faut ajouter une vraie notion de corps physique.
//
//Typiquement :
//
//struct RigidbodyComponent
//{
//    float mass = 1.0f;
//    float invMass = 1.0f;
//
//    XMFLOAT3 velocity = { 0,0,0 };
//    XMFLOAT3 acceleration = { 0,0,0 };
//
//    XMFLOAT3 forces = { 0,0,0 };
//
//    bool isStatic = false;
//    bool useGravity = true;
//};

//Ensuite plus tard :
//
//restitution(rebond)
//
//friction
//
//linear damping
//
//angular velocity
//
//inertia tensor
//
//3. Faire d’abord une intégration très simple
//
//Avant même la réponse de collision, il faut pouvoir déplacer un objet avec une vitesse.
//
//Pipeline minimal par frame :
//
//accumuler les forces
//
//calculer accélération
//
//mettre à jour la vitesse
//
//mettre à jour la position
//
//vider les forces
//
//Exemple conceptuel :
//
//acceleration = forces * invMass;
//if (useGravity) acceleration += gravity;
//
//velocity += acceleration * dt;
//position += velocity * dt;
//
//forces = { 0,0,0 };
//
//Au début, ça suffit largement.
//5. Garder ton pipeline collision en 2 étapes
//
//Tu as déjà la bonne direction :
//
//Broad phase
//
//Trouver les paires candidates :
//
//grille spatiale
//
//ou sweep and prune plus tard
//
//Narrow phase
//
//Tester précisément :
//
//sphere / sphere
//
//OBB / sphere
//
//OBB / OBB
//
//À ce stade, la collision ne doit pas juste répondre true / false.
//Elle doit produire des infos de contact.
//
//6. Introduire une structure de contact
//
//Au lieu de juste savoir “ça collisionne”, il faut savoir :
//
//normale de collision
//
//profondeur de pénétration
//
//point de contact éventuel
//
//entité A / entité B
//
//Exemple :
//
//struct Contact
//{
//    EntityId entityA;
//    EntityId entityB;
//
//    XMFLOAT3 normal;
//    float penetration;
//
//    XMFLOAT3 contactPoint;
//};
//
//C’est la transition entre “détection” et “physique”.
//
//7. Modifier le narrow phase pour retourner des contacts
//
//Par exemple :
//
//Sphere vs Sphere
//
//normale = direction entre centres
//
//pénétration = (rA + rB) - distance
//
//OBB vs Sphere
//
//point le plus proche sur l’OBB
//
//normale = sphère - point proche
//
//pénétration = rayon - distance
//
//OBB vs OBB
//
//avec SAT, garder l’axe de pénétration minimale
//
//cette normale devient la normale de contact
//
//la plus petite overlap devient la pénétration
//
//Au début, tu peux même ne pas calculer de vrai point de contact précis, et commencer juste avec :
//
//normale
//
//pénétration

//8. Faire une résolution de pénétration simple
//
//Avant même les impulsions, tu peux faire une correction de position pour empêcher les objets de rester imbriqués.
//
//Principe :
//
//si deux objets se chevauchent,
//
//on les sépare selon la normale de collision.
//
//Exemple simple :
//
//si A et B dynamiques : chacun bouge de moitié
//
//si A statique et B dynamique : seul B bouge
//
//Conceptuellement :
//
//correction = normal * penetration;
//
//Puis réparti selon les masses inverses.
//
//Ça évite l’effet “objets collés dans le sol”.