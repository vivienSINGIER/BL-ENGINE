#ifndef TEST_TRANSFORM_COMPONENT_H_DEFINED
#define TEST_TRANSFORM_COMPONENT_H_DEFINED

#include "Test.hpp"
#include "../Engine/Engine.h"

class TestTransformComponent : public Test
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
            
            t.local.AddYPR(Vect3f32(1.0f * dt, 1.0f * dt, 0.0f));
            t.local.Move(Vect3f32(moveDir * dt, 0.0f, 0.0f));
            t.local.Scale(Vect3f32(scaleFactor, scaleFactor, scaleFactor));

            activetimer += dt;
        }
    };

    struct TestScript2 : public IScript
    {
        void Update(float dt) override
        {
            TransformComponent& t = GetComponent<TransformComponent>();
            
            t.local.AddYPR(Vect3f32(1.0f * dt, 1.0f * dt, 0.0f));
            t.local.Scale(1.000001f);
            t.local.Move(Vect3f32(0.1f * dt, 0.0f, 0.0f));
        }
    };
    
    void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        Scene* scene = SceneManager::GetSceneWithName("Default");
        ComponentRegistry::RegisterScript<TestScript>();
        ComponentRegistry::RegisterScript<TestScript2>();

        EntityId e = scene->world->CreateEntity();
        scene->world->AddComponent<TransformComponent>(e);
        MeshRenderer& m = scene->world->AddComponent<MeshRenderer>(e);
        m.geoId = RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
        scene->world->AddScript<TestScript>(e);

        EntityId e1 = scene->world->CreateEntity();
        TransformComponent& t1 = scene->world->AddComponent<TransformComponent>(e1);
        t1.SetParent(e);
        t1.local.SetPosition(Vect3f32(2.0f, 0.0f, 0.0f));
        MeshRenderer& m1 = scene->world->AddComponent<MeshRenderer>(e1);
        m.geoId = RessourceManager::GetGeometryId("Cube");
        scene->world->AddScript<TestScript2>(e1);

        Camera cam;
        Transform camT;
        camT.SetPosition(Vect3f32(0.0f, -5.0f, -5.0f));
        camT.LookAt({0.0f, 0.0f, 0.0f});
        
        cam.SetWorld(camT.GetMatrix());
        
        EngineManager::GetDevice()->SetMainCamera(&cam);
        
        EngineManager::GetInstance().Run();
    }
};

#endif