#ifndef TEST_COLLISION_HPP_DEFINED
#define TEST_COLLISION_HPP_DEFINED

class TestCollision : public Test
{
public:
    struct MoveScript : public IScript
    {
        void Update(float dt) override
        {
            Transform& transform = world->GetComponent<TransformComponent>(entity).local;
            InputManager::HandleInput();

            if (InputManager::IsKeyPressed(Z))
            {
                transform.Move(XMFLOAT3(0.0f, 0.0f, 1.0f * dt));
            }

            if (InputManager::IsKeyPressed(Q))
            {
                transform.Move(XMFLOAT3(-1.0f * dt, 0.0f, 0.0f));
            }

            if (InputManager::IsKeyPressed(S))
            {
                transform.Move(XMFLOAT3(0.0f, 0.0f, -1.0f * dt));
            }

            if (InputManager::IsKeyPressed(D))
            {
                transform.Move(XMFLOAT3(1.0f * dt, 0.0f, 0.0f));
            }

            if (InputManager::IsKeyPressed(E))
            {
                transform.AddYPR(XMFLOAT3(XM_PIDIV4 * dt, 0.0f , 0.0f));
            }

            if (InputManager::IsKeyPressed(A))
            {
                transform.AddYPR(XMFLOAT3(-XM_PIDIV4 * dt, 0.0f, 0.0f));
            }
        }
    };

    struct DebugScript : public IScript
    {
        void Update(float dt) override
        {
            ColliderComponent& collider = world->GetComponent<ColliderComponent>(1);
            Transform& transform = world->GetComponent<TransformComponent>(entity).local;

            //transform.SetPosition(collider.boundingBox.min);
        }
    };

    struct DebugScript2 : public IScript
    {
        void Update(float dt) override
        {
            ColliderComponent& collider = world->GetComponent<ColliderComponent>(1);
            Transform& transform = world->GetComponent<TransformComponent>(entity).local;

            //transform.SetPosition(collider.boundingBox.max);
        }
    };

    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Collision");
        Scene* scene = SceneManager::GetSceneWithName("Default");
        scene->world.RegisterSystem<TransformSystem>(Phase::Update);
        scene->world.RegisterSystem<MeshRendererSystem>(Phase::Render);
		ColliderSystem* sys = scene->world.RegisterSystem<ColliderSystem>(Phase::Update);
        sys->InitializePartitionGrid(XMINT2(100, 100), 10);

        EntityId e = scene->world.CreateEntity();
        scene->world.AddComponent<TransformComponent>(e);
		scene->world.AddComponent<ColliderComponent>(e);
        MeshRenderer& m = scene->world.AddComponent<MeshRenderer>(e);
        m.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
		scene->world.GetComponent<TransformComponent>(e).local.SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
        scene->world.GetComponent<TransformComponent>(e).local.AddYPR(XMFLOAT3(0.0f, 0.0f, 0.0f));

		EntityId e2 = scene->world.CreateEntity();
		scene->world.AddComponent<TransformComponent>(e2);
		scene->world.AddComponent<ColliderComponent>(e2);
		MeshRenderer& m2 = scene->world.AddComponent<MeshRenderer>(e2);
		m2.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        scene->world.GetComponent<TransformComponent>(e2).local.SetPosition(XMFLOAT3(2.0f, 0.0f, 0.0f));
        scene->world.AddScript<MoveScript>(e2);

        Material* mat = RessourceManager::GetShader("Color")->CreateMaterial();
        RessourceManager::AddMaterial("debug", mat);
        mat->SetFloat4("DiffuseAlbedo", XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

        EntityId debug = scene->world.CreateEntity();
        scene->world.AddComponent<TransformComponent>(debug);
        MeshRenderer& m3 = scene->world.AddComponent<MeshRenderer>(debug);
        m3.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        m3.material = mat;
        scene->world.GetComponent<TransformComponent>(debug).local.SetScale(0.1f);
        scene->world.AddScript<DebugScript>(debug);

        EntityId debug2 = scene->world.CreateEntity();
        scene->world.AddComponent<TransformComponent>(debug2);
        MeshRenderer& m4 = scene->world.AddComponent<MeshRenderer>(debug2);
        m4.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        m4.material = mat;
        scene->world.GetComponent<TransformComponent>(debug2).local.SetScale(0.1f);
        scene->world.AddScript<DebugScript2>(debug2);

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