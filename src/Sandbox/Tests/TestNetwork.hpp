#ifndef TEST_NETWORK_H_DEFINED
#define TEST_NETWORK_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"

class TestNetwork : public Test
{
public:
    struct TestScript : public IScript
    {
        void Start()
        {
            MeshRenderer& m = AddComponent<MeshRenderer>();
            m.geoId = RessourceManager::GetGeometryId("Cube");
            m.materialId = RessourceManager::GetMaterialId("White");
            
            TransformComponent& t = AddComponent<TransformComponent>();
            t.local.SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
        }
        
        void Update(float _dt) override
        {
            
        }
    };
    
    struct TestScript2 : public IScript
    {
        uint32 camId;
        uint32 clientId;
        
        void Start()
        {
            if (EngineManager::GetServer() == nullptr) return;

            MeshRenderer& m = AddComponent<MeshRenderer>();
            m.geoId = RessourceManager::GetGeometryId("Cube");
            m.materialId = RessourceManager::GetMaterialId("White");
            
            CameraComponent& cam = AddComponent<CameraComponent>();
            cam.isMainCamera = true;
            cam.camId = camId;
            
            TransformComponent& t = AddComponent<TransformComponent>();
            t.local.SetPosition(XMFLOAT3(0.0f, 0.0f, -5.0f));

            OwnerComponent& o = AddComponent<OwnerComponent>();
            o.ownerId = clientId;
        }
        
        void Update(float _dt) override
        {
            if (HasComponent<TransformComponent>() == false)
                return;
            if (HasComponent<OwnerComponent>() == false)
                return;
            
            TransformComponent& t = GetComponent<TransformComponent>();
            OwnerComponent& o = GetComponent<OwnerComponent>();
            
            if (InputManager::IsKey(Z, o.ownerId))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, 1.0f * _dt));
            if (InputManager::IsKey(S, o.ownerId))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, -1.0f * _dt));
            if (InputManager::IsKey(Q, o.ownerId))
                t.local.Move(XMFLOAT3(-1.0f * _dt, 0.0f, 0.0f));
            if (InputManager::IsKey(D, o.ownerId))
                t.local.Move(XMFLOAT3(1.0f * _dt, 0.0f, 0.0f));
            if (InputManager::IsKey(SPACE, o.ownerId))
                t.local.Move(XMFLOAT3(0.0f, 1.0f * _dt, 0.0f));
            if (InputManager::IsKey(LCONTROL, o.ownerId))
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
        
        RessourceManager::AddCamera("Default1");
        RessourceManager::AddCamera("Default2");
        
        if (isHost)
        {
            EngineManager::GetInstance().HostServer();
        
            Scene* scene = SceneManager::SetCurrentScene("Default");
        
            EntityId e = scene->world->CreateEntity();
            TestScript2& s = scene->world->AddScript<TestScript2>(e);
            s.clientId = 1;
            s.camId = RessourceManager::GetCameraId("Default1");
            
            EntityId e2 = scene->world->CreateEntity();
            TestScript2& s2 = scene->world->AddScript<TestScript2>(e2);
            s.clientId = 2;
            s.camId = RessourceManager::GetCameraId("Default2");
        }
        else
            EngineManager::GetInstance().Connect("127.0.0.1", 1888);
        
        EngineManager::GetInstance().Run();
    }
};

#endif