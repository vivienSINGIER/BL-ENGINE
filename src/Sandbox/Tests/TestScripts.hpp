#ifndef TEST_SCRIPTS_H_DEFINED
#define TEST_SCRIPTS_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────
// static void PrintEntity(World& w, EntityId e, const char* label)
// {
//     printf("[%s] alive=%d", label, true /* see note below */);
//
//     if (w.HasComponent<Position>(e))
//     {
//         auto& p = w.GetComponent<Position>(e);
//         printf("  pos=(%.2f, %.2f)", p.x, p.y);
//     }
//     if (w.HasComponent<Health>(e))
//     {
//         auto& h = w.GetComponent<Health>(e);
//         printf("  hp=%.2f", h.hp);
//     }
//     printf("\n");
// }

struct TestScript : public IScript
{
    float x = 0.0f;
    float y = 0.0f;

    float speed = 1.0f;

    void Awake() override
    {
        x = 1.0f;
    }
    
    void Start() override
    {
        y = -1.0f;
    }

    void Update(float dt) override
    {
        x += speed * dt;
        y += speed * dt;
    }

    void Destroy() override
    {
        x = 0.0f;
        y = 0.0f;
    }
};

class TestScripts : public Test
{
public: 
    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Scripts");
        World world;

        // ── Test 1: basic entity creation ──────────
        EntityId e1 = world.CreateEntity();
        world.AddScript<TestScript>(e1);

        // ── Test 2: simulate 3 ticks at dt=1s ──────
        for (int i = 0; i < 3; i++)
            world.Update(1.f);

        world.DestroyEntity(e1);
        printf("\nAll tests passed.\n");
    }
};

#endif