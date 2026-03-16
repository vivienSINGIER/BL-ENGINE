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

            if (t.transform.GetWorldPosition().x > 3.0f)
                moveDir = -1.0f;
            if (t.transform.GetWorldPosition().x < -3.0f)
                moveDir = 1.0f;

            if (t.transform.GetWorldScale().x > 3.0f)
                scaleFactor = 0.999f;
            if (t.transform.GetWorldScale().x < 0.5f)
                scaleFactor = 1.001f;
            
            t.transform.AddLocalYPR(XMFLOAT3(1.0f * dt, 1.0f * dt, 0.0f));
            t.transform.MoveWorld(XMFLOAT3(moveDir * dt, 0.0f, 0.0f));
            t.transform.ScaleWorld(XMFLOAT3(scaleFactor, scaleFactor, scaleFactor));

            activetimer += dt;
            if (activetimer > 1.0f)
                world->SetActiveComponent<MeshRenderer>(entity, false);
            if (activetimer > 2.0f)
            {
                world->SetActiveComponent<MeshRenderer>(entity, true);
                activetimer = 0.0f;
            }
        }
    };
    
    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        Scene* scene = SceneManager::GetSceneWithName("Default");
        scene->world.RegisterSystem<TransformSystem>(Phase::Update);
        scene->world.RegisterSystem<MeshRendererSystem>(Phase::Render);

        EntityId e = scene->world.CreateEntity();
        scene->world.AddComponent<TransformComponent>(e);
        MeshRenderer& m = scene->world.AddComponent<MeshRenderer>(e);
        m.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        scene->world.AddScript<TestScript>(e);

        Camera cam;
        XMFLOAT3 pos = XMFLOAT3(0.0f, -3.0f, -3.0f);
        cam.SetPos(pos);
        XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
        cam.LookAt(target);
        
        EngineManager::GetDevice()->SetMainCamera(&cam);
        
        EngineManager::GetInstance().Run();
    }
};

#endif