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

            if (InputManager::IsKeyPressed(E))
            {
                transform.AddYPR(XMFLOAT3(XM_PIDIV4 * dt, 0.0f , 0.0f));
            }

            if (InputManager::IsKeyPressed(A))
            {
                transform.AddYPR(XMFLOAT3(0.0f, -XM_PIDIV4 * dt, 0.0f));
            }

            if (InputManager::IsKeyDown(G))
            {
                PhysicComponent& physic = world->GetComponent<PhysicComponent>(entity);
                if (physic.useGravity)
                    physic.useGravity = false;
                else
                    physic.useGravity = true;
            }

            if (InputManager::IsKeyPressed(W))
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
            ColliderComponent& collider = world->GetComponent<ColliderComponent>(1);
            Transform& transform = world->GetComponent<TransformComponent>(entity).local;

            transform.SetPosition(collider.aabb.min);
        }
    };

    struct DebugScript2 : public IScript
    {
        void Update(float dt) override
        {
            ColliderComponent& collider = world->GetComponent<ColliderComponent>(1);
            Transform& transform = world->GetComponent<TransformComponent>(entity).local;

            transform.SetPosition(collider.aabb.max);
        }
    };

    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Collision");
        Scene* scene = SceneManager::GetSceneWithName("Default");
        scene->world.RegisterSystem<TransformSystem>(Phase::Update);
        scene->world.RegisterSystem<MeshRendererSystem>(Phase::Render);
		ColliderSystem* sys = scene->world.RegisterSystem<ColliderSystem>(Phase::FixedUpdate);
        scene->world.RegisterSystem<PhysicSystem>(Phase::FixedUpdate);
        sys->InitializePartitionGrid(XMINT2(100, 100), 10);

        EntityId e = scene->world.CreateEntity();
        scene->world.AddComponent<TransformComponent>(e);
		scene->world.AddComponent<ColliderComponent>(e);
        scene->world.AddComponent<PhysicComponent>(e);
        MeshRenderer& m = scene->world.AddComponent<MeshRenderer>(e);
        m.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
		scene->world.GetComponent<TransformComponent>(e).local.SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
        scene->world.GetComponent<TransformComponent>(e).local.AddYPR(XMFLOAT3(0.0f, 0.0f, 0.0f));

		EntityId e2 = scene->world.CreateEntity();
		scene->world.AddComponent<TransformComponent>(e2);
		scene->world.AddComponent<ColliderComponent>(e2);
        PhysicComponent& physic = scene->world.AddComponent<PhysicComponent>(e2);
        physic.type = BodyType::Dynamic;
        physic.useGravity = false;
		MeshRenderer& m2 = scene->world.AddComponent<MeshRenderer>(e2);
		m2.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        scene->world.GetComponent<TransformComponent>(e2).local.SetPosition(XMFLOAT3(0.0f, 2.0f, 0.0f));
        scene->world.AddScript<MoveScript>(e2);

        Material* mat = RessourceManager::GetShader("Color")->CreateMaterial();
        RessourceManager::AddMaterial("debug", mat);
        mat->SetFloat4("DiffuseAlbedo", XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

        Camera cam;
        XMFLOAT3 pos = XMFLOAT3(0.0f, 1.0f, -5.0f);
        cam.SetPos(pos); 
        XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
        cam.LookAt(target);

        EngineManager::GetDevice()->SetMainCamera(&cam);

        EngineManager::GetInstance().Run();
    }

};

#endif 