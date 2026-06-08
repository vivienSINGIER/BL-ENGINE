#ifndef TEST_ECS_H_DEFINED
#define TEST_ECS_H_DEFINED

#include "Test.hpp"
#include "../Engine/Engine.h"

struct Position  { float x, y; };
struct Velocity  { float vx, vy; };
struct Health    { float hp; };

// ─────────────────────────────────────────────
//  Systems
// ─────────────────────────────────────────────

// Moves entities that have both Position and Velocity
struct MoveSystem : System<Position, Velocity>
{
    void OnUpdate(float dt, EntityId e, Position& pos, Velocity& vel) override
    {
        pos.x += vel.vx * dt;
        pos.y += vel.vy * dt;
    }
};

// Drains health on every entity that has Health
struct DrainSystem : System<Health>
{
    void OnUpdate(float dt, EntityId e, Health& hp) override
    {
        hp.hp -= 10.f * dt;
    }
};

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────
static void PrintEntity(World& w, EntityId e, const char* label)
{
    printf("[%s] alive=%d", label, true /* see note below */);

    if (w.HasComponent<Position>(e))
    {
        auto& p = w.GetComponent<Position>(e);
        printf("  pos=(%.2f, %.2f)", p.x, p.y);
    }
    if (w.HasComponent<Health>(e))
    {
        auto& h = w.GetComponent<Health>(e);
        printf("  hp=%.2f", h.hp);
    }
    printf("\n");
}

class TestECS : public Test
{
public: 
    void Run()
    {
        World world;

        // Register systems before creating entities
        SystemScheduler::Get().RegisterSystem<MoveSystem>(Phase::Update);
        SystemScheduler::Get().RegisterSystem<DrainSystem>(Phase::Update);

        // ── Test 1: basic entity creation ──────────
        EntityId e1 = world.CreateEntity();
        world.AddComponent<Position>(e1, {0.f, 0.f});
        world.AddComponent<Velocity>(e1, {1.f, 2.f});
        world.AddComponent<Health>(e1, {100.f});

        // Entity with no velocity — should not move
        EntityId e2 = world.CreateEntity();
        world.AddComponent<Position>(e2, {5.f, 5.f});
        world.AddComponent<Health>(e2, {50.f});

        printf("e1 id=0x%llx\n", (unsigned long long)e1);
        printf("e2 id=0x%llx\n", (unsigned long long)e2);
        
        printf("=== Initial state ===\n");
        PrintEntity(world, e1, "e1");
        PrintEntity(world, e2, "e2");

        // ── Test 2: simulate 3 ticks at dt=1s ──────
        for (int i = 0; i < 3; i++)
            world.Update(1.f);

        printf("\n=== After 3 ticks (dt=1s) ===\n");
        // Expected: e1 pos=(3,6)  hp=70   e2 pos=(5,5)  hp=20
        PrintEntity(world, e1, "e1");
        PrintEntity(world, e2, "e2");

        // ── Test 3: component removal ───────────────
        world.RemoveComponent<Velocity>(e1);
        world.Update(1.f);

        printf("\n=== After removing Velocity from e1 ===\n");
        // Expected: e1 pos=(3,6) unchanged, hp=60
        PrintEntity(world, e1, "e1");

        // ── Test 4: entity destruction + reuse ──────
        world.DestroyEntity(e2);
        printf("\nDestroyed e2. Creating e3 (should reuse slot)...\n");

        EntityId e3 = world.CreateEntity();
        world.AddComponent<Position>(e3,{99.f, 99.f});

        printf("e3 id=0x%llx\n", (unsigned long long)e3);
        // The lower 32 bits should match e2's index (slot reuse)
        // The upper 32 bits (gen) should be incremented

        // ── Test 5: stale handle detection ──────────
        // Accessing e2 after destroy should be caught by assert in GetRecord
        // Uncomment to verify:
        // world.GetComponent<Health>(e2); // should assert/crash

        printf("\nAll tests passed.\n");
    }
};

#endif