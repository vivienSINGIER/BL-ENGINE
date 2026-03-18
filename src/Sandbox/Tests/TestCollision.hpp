#ifndef TEST_COLLISION_HPP_DEFINED
#define TEST_COLLISION_HPP_DEFINED

class TestCollision : public Test
{
public:
    struct MoveScript : public IScript
    {
        void Update(float dt) override
        {
            Transform transform = world->GetComponent<TransformComponent>(entity).transform;
            InputManager::HandleInput();

            if (InputManager::IsKeyDown(Z))
            {
                transform.MoveWorld(XMFLOAT3(0.0f, 0.0f, 1.0f));
            }

            if (InputManager::IsKeyDown(Q))
            {
                transform.MoveWorld(XMFLOAT3(-1.0f, 0.0f, 0.0f));
            }

            if (InputManager::IsKeyDown(S))
            {
                transform.MoveWorld(XMFLOAT3(0.0f, 0.0f, -1.0f));
            }

            if (InputManager::IsKeyDown(D))
            {
                transform.MoveWorld(XMFLOAT3(1.0f, 0.0f, 0.0f));
            }
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
		scene->world.GetComponent<TransformComponent>(e).transform.SetWorldPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

		EntityId e2 = scene->world.CreateEntity();
		scene->world.AddComponent<TransformComponent>(e2);
		scene->world.AddComponent<ColliderComponent>(e2);
		MeshRenderer& m2 = scene->world.AddComponent<MeshRenderer>(e2);
		m2.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        scene->world.GetComponent<TransformComponent>(e2).transform.SetWorldPosition(XMFLOAT3(1.0f, 0.0f, 0.0f));
        scene->world.AddScript<MoveScript>(e2);

        Camera cam;
        XMFLOAT3 pos = XMFLOAT3(0.0f, -3.0f, -3.0f);
        cam.SetPos(pos); 
        XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
        cam.LookAt(target);

        EngineManager::GetDevice()->SetMainCamera(&cam);

        EngineManager::GetInstance().Run();
    }

};

#endif 