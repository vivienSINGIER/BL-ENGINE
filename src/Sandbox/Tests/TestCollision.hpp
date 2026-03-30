#ifndef TEST_COLLISION_HPP_DEFINED
#define TEST_COLLISION_HPP_DEFINED

#include "../Engine/Engine.h"

class TestCollision : public Test
{
public:
    struct MoveScript : public IScript
    {
        void Update(float dt) override
        {
            Transform& transform = GetComponent<TransformComponent>().local;
            InputManager::HandleInput();

            if (InputManager::IsKey(Z))
            {
                transform.Move(XMFLOAT3(0.0f, 0.0f, 1.0f * dt));
            }

            if (InputManager::IsKey(Q))
            {
                transform.Move(XMFLOAT3(-1.0f * dt, 0.0f, 0.0f));
            }

            if (InputManager::IsKey(S))
            {
                transform.Move(XMFLOAT3(0.0f, 0.0f, -1.0f * dt));
            }

            if (InputManager::IsKey(D))
            {
                transform.Move(XMFLOAT3(1.0f * dt, 0.0f, 0.0f));
            }

            if (InputManager::IsKey(E))
            {
                transform.AddYPR(XMFLOAT3(XM_PIDIV4 * dt, 0.0f , 0.0f));
            }

            if (InputManager::IsKey(A))
            {
                transform.AddYPR(XMFLOAT3(-XM_PIDIV4 * dt, 0.0f, 0.0f));
            }

            if (InputManager::IsKey(W))
            {
                XMFLOAT3 scale = transform.GetScale();
                transform.SetScale(XMFLOAT3(scale.x + 0.5f * dt, scale.y, scale.z));
            }
        }
    };

    struct DebugScript : public IScript
    {
        void Update(float dt) override
        {
            ColliderComponent& collider = GetComponent<ColliderComponent>();
            Transform& transform = GetComponent<TransformComponent>().local;

            transform.SetPosition(collider.aabb.min);
        }
    };

    struct DebugScript2 : public IScript
    {
        void Update(float dt) override
        {
            ColliderComponent& collider = GetComponent<ColliderComponent>();
            Transform& transform = GetComponent<TransformComponent>().local;

            transform.SetPosition(collider.aabb.max);
        }
    };

    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Collision");
        Scene* scene = SceneManager::GetSceneWithName("Default");
		ColliderSystem* sys = SystemScheduler::Get().GetSystem<ColliderSystem>();
        sys->InitializePartitionGrid(XMINT2(100, 100), 10);
        
        RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
        uint32 shaderId = RessourceManager::AddShader("LitColored", ShaderFactory::CreateLitColored(EngineManager::GetDevice()));
        Material* white = RessourceManager::GetShader(shaderId)->CreateMaterial();
        RessourceManager::AddMaterial("White", white);
        Material* debugMat = RessourceManager::GetShader(shaderId)->CreateMaterial();
        debugMat->SetFloat4("DiffuseAlbedo", XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
        RessourceManager::AddMaterial("Debug", debugMat);

        EntityId e = scene->world->CreateEntity();
        scene->world->AddComponent<TransformComponent>(e);
		scene->world->AddComponent<ColliderComponent>(e);
        MeshRenderer& m = scene->world->AddComponent<MeshRenderer>(e);
        m.geoId = RessourceManager::GetGeometryId("Cube");
        m.materialId = RessourceManager::GetMaterialId("White");
		scene->world->GetComponent<TransformComponent>(e).local.SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
        scene->world->GetComponent<TransformComponent>(e).local.AddYPR(XMFLOAT3(0.0f, 0.0f, 0.0f));

		EntityId e2 = scene->world->CreateEntity();
		scene->world->AddComponent<TransformComponent>(e2);
		scene->world->AddComponent<ColliderComponent>(e2);
		MeshRenderer& m2 = scene->world->AddComponent<MeshRenderer>(e2);
		m2.geoId = RessourceManager::GetGeometryId("Cube");
        m2.materialId = RessourceManager::GetMaterialId("White");
        scene->world->GetComponent<TransformComponent>(e2).local.SetPosition(XMFLOAT3(2.0f, 0.0f, 0.0f));
        scene->world->AddScript<MoveScript>(e2);

        Material* mat = RessourceManager::GetShader("Color")->CreateMaterial();
        RessourceManager::AddMaterial("debug", mat);
        mat->SetFloat4("DiffuseAlbedo", XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

        EntityId debug = scene->world->CreateEntity();
        scene->world->AddComponent<TransformComponent>(debug);
        MeshRenderer& m3 = scene->world->AddComponent<MeshRenderer>(debug);
        m3.geoId = RessourceManager::GetGeometryId("Cube");
        m3.materialId = RessourceManager::GetMaterialId("Debug");
        scene->world->GetComponent<TransformComponent>(debug).local.SetScale(0.1f);
        scene->world->AddScript<DebugScript>(debug);

        EntityId debug2 = scene->world->CreateEntity();
        scene->world->AddComponent<TransformComponent>(debug2);
        MeshRenderer& m4 = scene->world->AddComponent<MeshRenderer>(debug2);
        m4.geoId = RessourceManager::GetGeometryId("Cube");
        m4.materialId = RessourceManager::GetMaterialId("Debug");
        scene->world->GetComponent<TransformComponent>(debug2).local.SetScale(0.1f);
        scene->world->AddScript<DebugScript2>(debug2);

        Camera cam;
        XMFLOAT3 pos = XMFLOAT3(0.0f, -5.0f, -1.0f);
        cam.SetPos(pos); 
        XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
        cam.LookAt(target);

        EngineManager::GetDevice()->SetMainCamera(&cam);

        EngineManager::GetInstance().Run();
    }

};

#endif 