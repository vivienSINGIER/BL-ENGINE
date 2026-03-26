#ifndef TEST_TRANSFORM_H_DEFINED
#define TEST_TRANSFORM_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"

class TestTransform : public Test
{
public:
    struct TestScript : public IScript
    {
        float moveDir = 1.0f;
        float scaleFactor = 1.1f;
        float activetimer = 0.0f;
        
        void Update(float dt) override
        {
            TransformComponent& t = GetComponent<TransformComponent>();

            if (t.local.GetPosition().x > 3.0f)
                moveDir = -1.0f;
            if (t.local.GetPosition().x < -3.0f)
                moveDir = 1.0f;

            if (t.local.GetScale().x > 1.0f)
                scaleFactor = 0.999f;
            if (t.local.GetScale().x < 0.1f)
                scaleFactor = 1.001f;
            
            t.local.AddYPR(XMFLOAT3(1.0f * dt, 1.0f * dt, 0.0f));
            t.local.Move(XMFLOAT3(moveDir * dt, 0.0f, 0.0f));
            t.local.Scale(XMFLOAT3(scaleFactor, scaleFactor, scaleFactor));

            activetimer += dt;
        }
    };

    struct TestScript2 : public IScript
    {
        void Update(float dt) override
        {
            TransformComponent& t = GetComponent<TransformComponent>();
            
            t.local.AddYPR(XMFLOAT3(1.0f * dt, 1.0f * dt, 0.0f));
            t.local.Scale(1.000001f);
            t.local.Move(XMFLOAT3(0.1f * dt, 0.0f, 0.0f));
        }
    };
    
    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        Scene* scene = SceneManager::GetSceneWithName("Default");

        EntityId e = scene->world->CreateEntity();
        scene->world->AddComponent<TransformComponent>(e);
        MeshRenderer& m = scene->world->AddComponent<MeshRenderer>(e);
        m.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        scene->world->AddScript<TestScript>(e);

        EntityId e1 = scene->world->CreateEntity();
        TransformComponent& t1 = scene->world->AddComponent<TransformComponent>(e1);
        t1.SetParent(e);
        t1.local.SetPosition(XMFLOAT3(2.0f, 0.0f, 0.0f));
        MeshRenderer& m1 = scene->world->AddComponent<MeshRenderer>(e1);
        m.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        scene->world->AddScript<TestScript2>(e1);

        Camera cam;
        XMFLOAT3 pos = XMFLOAT3(0.0f, -5.0f, -5.0f);
        cam.SetPos(pos);
        XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
        cam.LookAt(target);
        
        EngineManager::GetDevice()->SetMainCamera(&cam);
        
        EngineManager::GetInstance().Run();
    }
};

#endif