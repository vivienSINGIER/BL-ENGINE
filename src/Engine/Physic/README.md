# Moteur Physique — Architecture Source Engine Style

## Vue d'ensemble

```
PhysicsWorld.hpp          ← Orchestrateur (IPhysicsEnvironment)
├── BroadPhase.hpp        ← Dynamic BVH 3D  (remplace la grille 2D)
├── NarrowPhase.hpp       ← SAT OBB/Sphere/Capsule + clipping
├── ConstraintSolver.hpp  ← PGS + Warm Starting + Cône de Coulomb
├── Integrator.hpp        ← Symplectic Euler + quaternion ω*q
├── Contact.hpp           ← Contact + ContactCache (warm starting)
├── RigidBody.hpp         ← Corps rigide découplé de l'ECS
├── PhysicsMath.hpp       ← Wrappers DirectXMath + Mat3
└── PhysicsECSBridge.hpp  ← Connexion ECS existant ↔ PhysicsWorld
```

---

## Ce qui change par rapport à ton moteur

### 1. Broad Phase : Grille 2D → Dynamic BVH 3D

| Ancien                        | Nouveau                                  |
|-------------------------------|------------------------------------------|
| Grille 2D (X/Y uniquement)    | BVH 3D hiérarchique (type Bullet dbvt)   |
| Insertion O(n) par grille     | Insertion O(log n) par SAH               |
| Queries O(n²) par cellule     | Queries O(log n + k)                     |
| Rebuild complet chaque frame  | Fat AABB : rebuild seulement si sortie   |

### 2. Solver : Impulse séquentielle → PGS avec Warm Starting

| Ancien                                      | Nouveau                                         |
|---------------------------------------------|-------------------------------------------------|
| 4 itérations, pas de cache                  | 8 itérations + impulses réinjectées (frame N-1) |
| Masse effective recalculée par itération    | Pré-calculée avant les itérations (PreSolve)    |
| Friction appliquée après normal (2 passes)  | Friction intégrée par point (cône de Coulomb)   |
| Baumgarte pur (pollue les vitesses)         | Split Impulse : correction pos/vel séparées     |
| Restitution toujours appliquée              | Seuil de restitution (contacts quasi-statiques) |

### 3. Tenseur d'inertie : diagonale locale → tenseur world space

```cpp
// Ancien : composantes X/Y/Z indépendantes (approximation)
_physic.inertieInverse.x = 1.f / _physic.inertie.x;

// Nouveau : tenseur 3x3 en world space (R * I_local * R^T)
rb.inertia.UpdateWorldTensor(rb.orientation);
XMFLOAT3 dw = rb.inertia.ApplyInvWorld(V3Cross(r, impulse));
```
Cela donne une réponse angulaire correcte pour les corps inclinés.

### 4. Intégration quaternion correcte

```cpp
// Ancien : XMQuaternionRotationRollPitchYaw (angles d'Euler — gimbal lock)
XMVECTOR qDelta = XMQuaternionRotationRollPitchYaw(da.x, da.y, da.z);

// Nouveau : dq/dt = 0.5 * [0,ω] * q (formulation différentielle)
XMFLOAT4 omegaQ = { ω.x*0.5f*dt, ω.y*0.5f*dt, ω.z*0.5f*dt, 0 };
q = XMQuaternionNormalize(q + omegaQ * q);
```

### 5. Nouvelles formes : Capsule

```cpp
rb->MakeCapsule(mass, radius, halfHeight);
// Tests : Capsule/Sphere, Capsule/OBB, Capsule/Capsule
```

### 6. RigidBody découplé de l'ECS

Le `RigidBody` est maintenant une structure autonome dans `PhysicsWorld`.
L'ECS ne fait que lire le résultat via `PullTransforms()`.
Cela permet :
- De passer le physics world à un thread séparé
- De faire du sub-stepping sans impacter le rendu
- De sérialiser/désérialiser l'état physique indépendamment

---

## Intégration dans ton ECS

```cpp
// Init
PhysicsWorld g_physWorld;
g_physWorld.SetGravity({0, -9.81f, 0});

PhysicsECSBridge bridge(&g_physWorld);

// Enregistrement des entités
PhysicsBodyDesc desc;
desc.entityId   = myEntity;
desc.shape      = ColliderShape::Box;
desc.halfExtents = {0.5f, 0.5f, 0.5f};
desc.mass       = 1.0f;
desc.useGravity = true;
bridge.RegisterBody(desc);

// Frame loop
void Update(float dt)
{
    g_physWorld.Step(dt);

    bridge.PullTransforms([&](EntityId id, const XMFLOAT3& pos, const XMFLOAT4& rot)
    {
        world->GetComponent<TransformComponent>(id).local.SetPosition(pos);
        world->GetComponent<TransformComponent>(id).local.SetRotationQuaternion(rot);
    });
}
```

---

## Paramètres de tuning

```cpp
g_physWorld.SetSolverIterations(8);           // + d'itérations = + stable, + lent
g_physWorld.SetPositionCorrectionFactor(0.2f);// Agressivité correction pénétration
g_physWorld.SetRestitutionThreshold(0.2f);    // Vitesse min pour rebond
```

---

## Fichiers à inclure

Un seul include suffit :
```cpp
#include "Physics/PhysicsWorld.hpp"
#include "Physics/PhysicsECSBridge.hpp"
```
