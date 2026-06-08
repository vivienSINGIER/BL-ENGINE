#ifndef TEST_UI_H_DEFINED
#define TEST_UI_H_DEFINED

#include "Test.hpp"
#include "../Engine/Engine.h"
#include "Network/Client.h"

class TestUI : public Test
{
public:
    struct TestScript : public IScript
    {
        void Update(float _dt) override
        {
            if (HasComponent<TextComponent>() == false)
                return;
            
            TextComponent& t = GetComponent<TextComponent>();

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
    
    void Run(LPSTR lpCmdLine = nullptr)
    {
        String args = lpCmdLine;

        bool isHost = args.find("--host") != String::npos;

        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        Device* pDevice = EngineManager::GetDevice();
        ComponentRegistry::RegisterScript<TestScript>();
        
        RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
        uint32 shaderId = RessourceManager::AddShader("LitColored", ShaderFactory::CreateLitColored(EngineManager::GetDevice()));
        Material* white = RessourceManager::GetShader(shaderId)->CreateMaterial();
        RessourceManager::AddMaterial("White", white);
        
        RessourceManager::AddCamera("Default");
        
        RessourceManager::AddSprite("Square", SpriteFactory::BuildRoundedRectangle(EngineManager::GetDevice(), 30, 30, 5));
        uint32 uiShaderId = RessourceManager::AddUiShader("UiDefault", ShaderFactory::CreateUIBasic(EngineManager::GetDevice()));

        Texture* splashscreen = pDevice->CreateTexture(RES("/Textures/Splashscreen.dds"));
        RessourceManager::AddTexture("Splashscreen", splashscreen);
        UiMaterial* uiMaterial = RessourceManager::GetUiShader(uiShaderId)->CreateMaterial();
        RessourceManager::AddUiMaterial("UiDefault", uiMaterial);
        uiMaterial->SetTexture("Image", RessourceManager::GetTexture("Splashscreen"));

        UiMaterial* uiMaterial2 = RessourceManager::GetUiShader(uiShaderId)->CreateMaterial();
        RessourceManager::AddUiMaterial("UiRed", uiMaterial2);
        uiMaterial2->SetFloat4("Color", {1.0f, 0.0f, 0.0f, 1.0f});
        
        UiMaterial* uiMaterial3 = RessourceManager::GetUiShader(uiShaderId)->CreateMaterial();
        RessourceManager::AddUiMaterial("UiGreen", uiMaterial3);
        uiMaterial3->SetFloat4("Color", {0.0f, 1.0f, 0.0f, 1.0f});
        
        RenderFont* font = pDevice->CreateRenderFont(RES("/Font/Valentine.ttf"), 150.0f);
        RessourceManager::AddFont("Valentine", font);
        Text* text = pDevice->CreateText(font);
        text->SetString("Test");
        RessourceManager::AddText("Label", text);

        EngineManager::GetInstance().HostServer();
    
        Scene* scene = SceneManager::SetCurrentScene("Default");
        
        EntityId e = scene->world->CreateEntity();
        TextComponent& t = scene->world->AddComponent<TextComponent>(e);
        t.textId = RessourceManager::GetTextId("Label");
        
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