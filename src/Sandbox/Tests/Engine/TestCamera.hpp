#ifndef TEST_CAMERA_H_DEFINED
#define TEST_CAMERA_H_DEFINED

#include "Test.hpp"
#include "../Engine/Engine.h"

class TestCamera : public Test
{
public:
    struct TestScript : public IScript
    {
        void Update(float dt) override
        {
            TransformComponent& t = GetComponent<TransformComponent>();

            if (InputManager::IsKey(Z))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, 1.0f * dt));
            if (InputManager::IsKey(S))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, -1.0f * dt));
            if (InputManager::IsKey(Q))
                t.local.Move(XMFLOAT3(-1.0f * dt, 0.0f, 0.0f));
            if (InputManager::IsKey(D))
                t.local.Move(XMFLOAT3(1.0f * dt, 0.0f, 0.0f));
            if (InputManager::IsKey(SPACE))
                t.local.Move(XMFLOAT3(0.0f, 1.0f * dt, 0.0f));
            if (InputManager::IsKey(LCONTROL))
                t.local.Move(XMFLOAT3(0.0f, -1.0f * dt, 0.0f));
        }
    };
    
    void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        Scene* scene = SceneManager::GetSceneWithName("Default");

        EntityId e = scene->world->CreateEntity();
        scene->world->AddComponent<TransformComponent>(e);
        MeshRenderer& m = scene->world->AddComponent<MeshRenderer>(e);
        m.geoId = RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
        
        RessourceManager::AddCamera("Default");

        EntityId e1 = scene->world->CreateEntity();
        TransformComponent& t1 = scene->world->AddComponent<TransformComponent>(e1);
        t1.local.SetPosition(XMFLOAT3(0.0f, 0.0f, -5.0f));

        CameraComponent& cam = scene->world->AddComponent<CameraComponent>(e1);
        cam.camId = RessourceManager::GetCameraId("Default");
        cam.isMainCamera = true;
        scene->world->AddScript<TestScript>(e1);
        
        EngineManager::GetInstance().Run();
    }
};

#endif