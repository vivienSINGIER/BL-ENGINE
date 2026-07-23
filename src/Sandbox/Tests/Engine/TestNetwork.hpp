#ifndef TEST_NETWORK_H_DEFINED
#define TEST_NETWORK_H_DEFINED

#include "Test.hpp"
#include "../Engine/Engine.h"
#include "Network/Client.h"

class TestNetwork : public Test
{
public:
    struct TestScript : public IScript
    {
        void Awake()
        {
            MeshRenderer& m = AddComponent<MeshRenderer>();
            m.geoId = RessourceManager::GetGeometryId("Cube");
            m.materialId = RessourceManager::GetMaterialId("White");
            
            TransformComponent& t = AddComponent<TransformComponent>();
            t.local.SetPosition(Vect3f32(0.0f, 0.0f, 0.0f));
        }
        
        void Update(float _dt) override
        {
            
        }
    };
    
    struct TestScript2 : public IScript
    {
        uint32 clientId = 0;
        
        void Start()
        {
            if (EngineManager::IsServer() == false) return;

            MeshRenderer& m = AddComponent<MeshRenderer>();
            m.geoId = RessourceManager::GetGeometryId("Cube");
            m.materialId = RessourceManager::GetMaterialId("White");
            
            TransformComponent& t = AddComponent<TransformComponent>();
            t.local.SetPosition(Vect3f32(0.0f, 0.0f, -5.0f));
            
            OwnerComponent& o = AddComponent<OwnerComponent>();
            o.ownerId = clientId;
        }
        
        void OnSync(uint32 _clientId) override
        {
            if (HasComponent<OwnerComponent>() == false) return;
            
            OwnerComponent& o = GetComponent<OwnerComponent>();
            if (o.ownerId != _clientId)
                return;
            
            CameraComponent& cam = AddComponent<CameraComponent>();
            cam.camId = RessourceManager::GetCameraId("Default");
            cam.isMainCamera = true;
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
                t.local.Move(Vect3f32(0.0f, 0.0f, 1.0f * _dt));
            if (InputManager::IsKey(S, o.ownerId))
                t.local.Move(Vect3f32(0.0f, 0.0f, -1.0f * _dt));
            if (InputManager::IsKey(Q, o.ownerId))
                t.local.Move(Vect3f32(-1.0f * _dt, 0.0f, 0.0f));
            if (InputManager::IsKey(D, o.ownerId))
                t.local.Move(Vect3f32(1.0f * _dt, 0.0f, 0.0f));
            if (InputManager::IsKey(SPACE, o.ownerId))
                t.local.Move(Vect3f32(0.0f, 1.0f * _dt, 0.0f));
            if (InputManager::IsKey(LCONTROL, o.ownerId))
                t.local.Move(Vect3f32(0.0f, -1.0f * _dt, 0.0f));
        }
    };
    
    void Run(LPSTR lpCmdLine = nullptr)
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
            TestScript2& s = scene->world->AddScript<TestScript2>(e);
            s.clientId = 1;
            
            EntityId e2 = scene->world->CreateEntity();
            TestScript2& s2 = scene->world->AddScript<TestScript2>(e2);
            s2.clientId = 2;
        }
        else
            EngineManager::GetInstance().Connect("127.0.0.1", 1888);
        
        EngineManager::GetInstance().Run();
    }
};

#endif