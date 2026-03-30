#ifndef TEST_NETWORK_H_DEFINED
#define TEST_NETWORK_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"

class TestNetwork : public Test
{
public:
    struct TestScript : public IScript
    {
        void Awake()
        {
            MeshRenderer& m = world->AddComponent<MeshRenderer>(entity);
            m.geoId = RessourceManager::GetGeometryId("Cube");
            m.materialId = RessourceManager::GetMaterialId("White");
            
            TransformComponent& t = world->AddComponent<TransformComponent>(entity);
            t.local.SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
        }
        
        void Update(float _dt) override
        {
            
        }
    };
    
    struct TestScript2 : public IScript
    {
        void Awake()
        {
            if (EngineManager::GetServer() == nullptr) return;
            
            CameraComponent& cam = world->AddComponent<CameraComponent>(entity);
            cam.isMainCamera = true;
            cam.camId = RessourceManager::GetCameraId("Default");
            
            TransformComponent& t = world->AddComponent<TransformComponent>(entity);
            t.local.SetPosition(XMFLOAT3(0.0f, 0.0f, -5.0f));
        }
        
        void Update(float _dt) override
        {
            TransformComponent& t = GetComponent<TransformComponent>();

            if (InputManager::IsKey(Z))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, 1.0f * _dt));
            if (InputManager::IsKey(S))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, -1.0f * _dt));
            if (InputManager::IsKey(Q))
                t.local.Move(XMFLOAT3(-1.0f * _dt, 0.0f, 0.0f));
            if (InputManager::IsKey(D))
                t.local.Move(XMFLOAT3(1.0f * _dt, 0.0f, 0.0f));
            if (InputManager::IsKey(SPACE))
                t.local.Move(XMFLOAT3(0.0f, 1.0f * _dt, 0.0f));
            if (InputManager::IsKey(LCONTROL))
                t.local.Move(XMFLOAT3(0.0f, -1.0f * _dt, 0.0f));
        }
    };
    
    static void Run(LPSTR lpCmdLine = nullptr)
    {
        String args = lpCmdLine;

        bool isHost = args.find("--host") != String::npos;

        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        ComponentRegistry::RegisterScript<TestScript>();
        ComponentRegistry::RegisterScript<TestScript2>();
        
        RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
        uint32 shaderId = RessourceManager::AddShader("LitColored", ShaderFactory::CreateLitColored(EngineManager::GetDevice()));
        Material* white = RessourceManager::GetShader(shaderId)->CreateMaterial();
        RessourceManager::AddMaterial("White", white);
        
        RessourceManager::AddCamera("Default");
        
        if (isHost)
        {
            EngineManager::GetInstance().HostServer();
        
            Scene* scene = SceneManager::SetCurrentScene("Default");
        
            EntityId e = scene->world->CreateEntity();
            scene->world->AddScript<TestScript>(e);
            
            EntityId e2 = scene->world->CreateEntity();
            scene->world->AddScript<TestScript2>(e2);
        }
        else
            EngineManager::GetInstance().Connect("127.0.0.1", 1888);
        
        EngineManager::GetInstance().Run();
    }
};

#endif