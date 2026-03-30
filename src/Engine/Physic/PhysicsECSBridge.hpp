#pragma once
// ============================================================
//  PhysicsECSBridge.hpp
//  Pont entre PhysicsWorld et ton ECS existant.
//
//  Ce système ECS :
//  1) Pousse les transforms ECS → PhysicsWorld (pour kinematic/sync)
//  2) Tire les positions PhysicsWorld → transforms ECS
//  3) Gère la création/destruction des RigidBody
//
//  Usage avec ton ECS :
//
//    class PhysicsBridgeSystem : public System<PhysicComponent, TransformComponent>
//    {
//        PhysicsECSBridge bridge;
//        ...
//    };
// ============================================================
#include "PhysicsWorld.hpp"

// Ces types doivent correspondre à ceux de ton ECS
// Adapte les includes selon ta structure
// #include "../Components/TransformComponent.hpp"
// #include "../Components/PhysicComponent.hpp"
// #include "../Components/ColliderComponent.hpp"

// ============================================================
//  Données de configuration pour créer un RigidBody depuis
//  les anciens composants ECS
// ============================================================
struct PhysicsBodyDesc
{
    EntityId      entityId    = INVALID_ENTITY;

    BodyType      bodyType    = BodyType::Dynamic;
    ColliderShape shape       = ColliderShape::Box;

    float         mass        = 1.0f;
    XMFLOAT3      halfExtents = {0.5f,0.5f,0.5f};
    float         radius      = 0.5f;
    float         capsuleHalfHeight = 0.5f;
    XMFLOAT3      colliderOffset    = {0,0,0};

    float         restitution    = 0.2f;
    float         staticFriction  = 0.6f;
    float         dynamicFriction = 0.4f;

    bool          useGravity    = false;
    bool          allowRotation = true;
    bool          isTrigger     = false;

    XMFLOAT3      initialPosition    = {0,0,0};
    XMFLOAT4      initialOrientation = {0,0,0,1};
    XMFLOAT3      initialVelocity    = {0,0,0};
};

// ============================================================
//  PhysicsECSBridge
// ============================================================
class PhysicsECSBridge
{
public:
    explicit PhysicsECSBridge(PhysicsWorld* world) : m_world(world) {}

    // ---- Création d'un corps depuis la config ECS ----
    RigidBody* RegisterBody(const PhysicsBodyDesc& desc)
    {
        RigidBody* rb = m_world->CreateBody(desc.entityId);

        rb->restitution    = desc.restitution;
        rb->staticFriction  = desc.staticFriction;
        rb->dynamicFriction = desc.dynamicFriction;
        rb->useGravity      = desc.useGravity;
        rb->allowRotation   = desc.allowRotation;
        rb->colliderOffset  = desc.colliderOffset;

        switch (desc.shape)
        {
        case ColliderShape::Box:
            rb->MakeBox(desc.mass, desc.halfExtents);
            break;
        case ColliderShape::Sphere:
            rb->MakeSphere(desc.mass, desc.radius);
            break;
        case ColliderShape::Capsule:
            rb->MakeCapsule(desc.mass, desc.radius, desc.capsuleHalfHeight);
            break;
        }

        if (desc.bodyType == BodyType::Static)
            rb->SetStatic();

        rb->position       = desc.initialPosition;
        rb->orientation    = desc.initialOrientation;
        rb->linearVelocity = desc.initialVelocity;

        return rb;
    }

    void UnregisterBody(EntityId id)
    {
        m_world->DestroyBody(id);
    }

    // ---- Sync ECS → Physics (à appeler avant Step si Transform modifié manuellement) ----
    void PushTransform(EntityId id, const XMFLOAT3& pos, const XMFLOAT4& rot)
    {
        m_world->SyncBodyFromECS(id, pos, rot);
    }

    // ---- Sync Physics → ECS (à appeler après Step) ----
    //  Fournir un lambda : [](EntityId, XMFLOAT3 pos, XMFLOAT4 rot) { ... }
    template<typename SyncFn>
    void PullTransforms(SyncFn&& fn)
    {
        m_world->ForEachBody([&](RigidBody& rb)
        {
            if (rb.bodyType == BodyType::Static) return;
            fn(rb.id, rb.position, rb.orientation);
        });
    }

    PhysicsWorld* GetWorld() { return m_world; }

private:
    PhysicsWorld* m_world = nullptr;
};

// ============================================================
//  Exemple d'intégration dans ton pipeline ECS
//  (à mettre dans ton World/SceneManager)
// ============================================================
/*

// Dans ton initialisation :
PhysicsWorld     g_physWorld;
PhysicsECSBridge g_bridge(&g_physWorld);

g_physWorld.SetGravity({0, -9.81f, 0});
g_physWorld.SetSolverIterations(8);

// Création d'un sol statique :
PhysicsBodyDesc floorDesc;
floorDesc.entityId    = floorEntity;
floorDesc.bodyType    = BodyType::Static;
floorDesc.shape       = ColliderShape::Box;
floorDesc.halfExtents = {50, 0.5f, 50};
floorDesc.initialPosition = {0, -0.5f, 0};
g_bridge.RegisterBody(floorDesc);

// Création d'une boîte dynamique :
PhysicsBodyDesc boxDesc;
boxDesc.entityId    = boxEntity;
boxDesc.mass        = 1.0f;
boxDesc.shape       = ColliderShape::Box;
boxDesc.halfExtents = {0.5f,0.5f,0.5f};
boxDesc.useGravity  = true;
boxDesc.allowRotation = true;
boxDesc.initialPosition = {0, 5, 0};
g_bridge.RegisterBody(boxDesc);

// Chaque frame :
void Update(float dt)
{
    g_physWorld.Step(dt);

    g_bridge.PullTransforms([&](EntityId id, const XMFLOAT3& pos, const XMFLOAT4& rot)
    {
        TransformComponent& t = world->GetComponent<TransformComponent>(id);
        t.local.SetPosition(pos);
        t.local.SetRotationQuaternion(rot);
    });
}

// Trigger callback :
g_physWorld.SetTriggerCallback([](uint32_t a, uint32_t b, bool enter) {
    if (enter)
        printf("Entity %u entered trigger %u\n", a, b);
});

// Raycast :
PhysicsWorld::RayHit hit;
if (g_physWorld.Raycast(camPos, camDir, 100.0f, hit))
    printf("Hit entity %u at t=%.2f\n", hit.entityId, hit.t);

*/
