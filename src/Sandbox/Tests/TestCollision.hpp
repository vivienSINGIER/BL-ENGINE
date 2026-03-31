#ifndef TEST_COLLISION_HPP_DEFINED
#define TEST_COLLISION_HPP_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"

class TestCollision : public Test
{
public:
    struct CameraScript : public IScript
    {
        void Update(float dt) override
        {
            TransformComponent& t = GetComponent<TransformComponent>();

            if (InputManager::IsKey(Z))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, 2.0f * dt));
            if (InputManager::IsKey(S))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, -2.0f * dt));
            if (InputManager::IsKey(Q))
                t.local.Move(XMFLOAT3(-2.0f * dt, 0.0f, 0.0f));
            if (InputManager::IsKey(D))
                t.local.Move(XMFLOAT3(2.0f * dt, 0.0f, 0.0f));
            if (InputManager::IsKey(SPACE))
                t.local.Move(XMFLOAT3(0.0f, 2.0f * dt, 0.0f));
            if (InputManager::IsKey(LCONTROL))
                t.local.Move(XMFLOAT3(0.0f, -2.0f * dt, 0.0f));
        }
    };

    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Collision Reference");

        Scene* scene = SceneManager::GetSceneWithName("Default");
        World* world = scene->world;

        RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
        uint32 shaderId = RessourceManager::AddShader("LitColored", ShaderFactory::CreateLitColored(EngineManager::GetDevice()));
        Material* white = RessourceManager::GetShader(shaderId)->CreateMaterial();
        RessourceManager::AddMaterial("White", white);

        RessourceManager::AddCamera("Default");

        ComponentRegistry::RegisterScript<CameraScript>();

        uint32 mat = RessourceManager::GetMaterialId("White");
		uint32 otherMat = RessourceManager::GetMaterialId("Default");

        // -----------------------------
        // CAMERA
        // -----------------------------
        EntityId camera = world->CreateEntity();
        TransformComponent& tCamera = world->AddComponent<TransformComponent>(camera);
        tCamera.local.SetPosition(XMFLOAT3(0.0f, 4.0f, -19.0f));

        CameraComponent& cam = world->AddComponent<CameraComponent>(camera);
        cam.camId = RessourceManager::GetCameraId("Default");
        cam.isMainCamera = true;

        world->AddScript<CameraScript>(camera);

        // -----------------------------
        // LIGHT
        // -----------------------------
        EntityId light = world->CreateEntity();
        TransformComponent& tLight = world->AddComponent<TransformComponent>(light);
        tLight.local.SetPosition(XMFLOAT3(8.0f, 12.0f, -4.0f));

        LightComponent& l = world->AddComponent<LightComponent>(light);
        l.SetPoint(1.0f, 50.0f, 1);

        EngineManager::GetInstance().Run();
    }
};

#endif