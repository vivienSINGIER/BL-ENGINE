#ifndef TEST_UI_H_DEFINED
#define TEST_UI_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"
#include "Network/Client.h"

class TestUI : public Test
{
public:
    struct TestScript : public IScript
    {
        void Update(float _dt) override
        {
            if (HasComponent<UiImageComponent>() == false)
                return;
            
            UiImageComponent& t = GetComponent<UiImageComponent>();

            if (InputManager::IsKey(Z))
                t.transform.Move(XMFLOAT2(0.0f, 100.0f * _dt));
            if (InputManager::IsKey(S))
                t.transform.Move(XMFLOAT2(0.0f, -100.0f * _dt));
            if (InputManager::IsKey(Q))
                t.transform.Move(XMFLOAT2(-100.0f * _dt, 0.0f));
            if (InputManager::IsKey(D))
                t.transform.Move(XMFLOAT2(100.0f * _dt, 0.0f));
            if (InputManager::IsKey(A))
                t.transform.Rotate(1.0f * _dt);
            if (InputManager::IsKey(E))
                t.transform.Rotate(-1.0f * _dt);
            if (InputManager::IsKey(X))
                t.transform.Scale(1.01f);
            if (InputManager::IsKey(W))
                t.transform.Scale(0.99f);
        }
    };
    
    static void Run(LPSTR lpCmdLine = nullptr)
    {
        String args = lpCmdLine;

        bool isHost = args.find("--host") != String::npos;

        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        ComponentRegistry::RegisterScript<TestScript>();
        
        RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
        uint32 shaderId = RessourceManager::AddShader("LitColored", ShaderFactory::CreateLitColored(EngineManager::GetDevice()));
        Material* white = RessourceManager::GetShader(shaderId)->CreateMaterial();
        RessourceManager::AddMaterial("White", white);
        
        RessourceManager::AddCamera("Default");
        
        RessourceManager::AddSprite("Square", SpriteFactory::BuildRectangle(EngineManager::GetDevice(), 30, 30));
        uint32 uiShaderId = RessourceManager::AddUiShader("UiDefault", ShaderFactory::CreateUIBasic(EngineManager::GetDevice()));
        UiMaterial* uiMaterial = RessourceManager::GetUiShader(uiShaderId)->CreateMaterial();
        RessourceManager::AddUiMaterial("UiDefault", uiMaterial);

        EngineManager::GetInstance().HostServer();
    
        Scene* scene = SceneManager::SetCurrentScene("Default");
        
        EntityId e = scene->world->CreateEntity();
        UiImageComponent& img = scene->world->AddComponent<UiImageComponent>(e);
        img.materialId = RessourceManager::GetUiMaterialId("UiDefault");
        img.spriteId = RessourceManager::GetSpriteId("Square");
        scene->world->AddScript<TestScript>(e);
        
        EntityId e1 = scene->world->CreateEntity();
        CameraComponent& cam = scene->world->AddComponent<CameraComponent>(e1);
        scene->world->AddComponent<TransformComponent>(e1);
        cam.isMainCamera = true;
        cam.camId = RessourceManager::GetCameraId("Default");

        EngineManager::GetInstance().Run();
    }
};

#endif