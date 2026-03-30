#ifndef TEST_LIGHTS_H_DEFINED
#define TEST_LIGHTS_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"

class TestLights : public Test
{
public:
    struct TestScript : public IScript
    {
        void Update(float dt) override
        {
            TransformComponent& t = GetComponent<TransformComponent>();
            LightComponent& l = GetComponent<LightComponent>();

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

            if (InputManager::IsKeyDown(ENTER))
                l.color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
            if (InputManager::IsKeyDown(RSHIFT))
                l.color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
            if (InputManager::IsKeyDown(RCONTROL))
                l.color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
        }
    };
    
    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        Scene* scene = SceneManager::GetSceneWithName("Default");

        RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
        uint32 shaderId = RessourceManager::AddShader("LitColored", ShaderFactory::CreateLitColored(EngineManager::GetDevice()));
        Material* white = RessourceManager::GetShader(shaderId)->CreateMaterial();
        RessourceManager::AddMaterial("White", white);
        Material* debugMat = RessourceManager::GetShader(shaderId)->CreateMaterial();
        debugMat->SetFloat4("DiffuseAlbedo", XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
        RessourceManager::AddMaterial("Debug", debugMat);
        RessourceManager::AddCamera("Default");
        
        EntityId e = scene->world->CreateEntity();
        scene->world->AddComponent<TransformComponent>(e);
        MeshRenderer& m = scene->world->AddComponent<MeshRenderer>(e);
        m.geoId = RessourceManager::GetGeometryId("Cube");
        m.materialId = RessourceManager::GetMaterialId("Debug");

        EntityId e1 = scene->world->CreateEntity();
        TransformComponent& t1 = scene->world->AddComponent<TransformComponent>(e1);
        t1.local.SetPosition(XMFLOAT3(1.0f, 1.0f, -5.0f));
        CameraComponent& cam = scene->world->AddComponent<CameraComponent>(e1);
        cam.camId = RessourceManager::GetCameraId("Default");
        cam.isMainCamera = true;

        EntityId e2 = scene->world->CreateEntity();
        TransformComponent& t2 = scene->world->AddComponent<TransformComponent>(e2);
        t2.local.SetPosition(XMFLOAT3(0.0f, 2.0f, 0.0f));
        LightComponent& l = scene->world->AddComponent<LightComponent>(e2);
        l.SetPoint(1.0f, 10.0f, 1);
        scene->world->AddScript<TestScript>(e2);
        
        EngineManager::GetInstance().Run();
    }
};

#endif