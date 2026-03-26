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
                physic.ToggleGravity();
            }

            if (InputManager::IsKeyPressed(W))
            {
                XMFLOAT3 scale = transform.GetScale();
                transform.SetScale(XMFLOAT3(scale.x + 0.5f * dt, scale.y, scale.z));
            }
        }
    };

    struct TestScript : public IScript
    {
        void Update(float dt) override
        {
            TransformComponent& t = GetComponent<TransformComponent>();

            if (InputManager::IsKeyPressed(Z))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, 1.0f * dt));
            if (InputManager::IsKeyPressed(S))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, -1.0f * dt));
            if (InputManager::IsKeyPressed(Q))
                t.local.Move(XMFLOAT3(-1.0f * dt, 0.0f, 0.0f));
            if (InputManager::IsKeyPressed(D))
                t.local.Move(XMFLOAT3(1.0f * dt, 0.0f, 0.0f));
            if (InputManager::IsKeyPressed(SPACE))
                t.local.Move(XMFLOAT3(0.0f, 1.0f * dt, 0.0f));
            if (InputManager::IsKeyPressed(LCONTROL))
                t.local.Move(XMFLOAT3(0.0f, -1.0f * dt, 0.0f));
        }
    };

    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Collision");
        Scene* scene = SceneManager::GetSceneWithName("Default");
        scene->world.RegisterSystem<TransformSystem>(Phase::Update);
        scene->world.RegisterSystem<MeshRendererSystem>(Phase::Render);
        scene->world.RegisterSystem<CameraSystem>(Phase::PreRender);
        scene->world.RegisterSystem<LightSystem>(Phase::PreRender);
		scene->world.RegisterSystem<PhysicIntegrateSystem>(Phase::FixedUpdate);
		ColliderSystem* colSys = scene->world.RegisterSystem<ColliderSystem>(Phase::FixedUpdate);
        colSys->InitializePartitionGrid(XMINT2(100, 100), 10);
        colSys->SetContactManager(EngineManager::GetContactManager());
        PhysicSystem* physSys = scene->world.RegisterSystem<PhysicSystem>(Phase::FixedUpdate);
		physSys->SetContactManager(EngineManager::GetContactManager());

        Material* mat = RessourceManager::GetShader("Color")->CreateMaterial();
        RessourceManager::AddMaterial("debug", mat);
        mat->SetFloat4("DiffuseAlbedo", XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));

        EntityId ground = scene->world.CreateEntity();
        TransformComponent& tGround = scene->world.AddComponent<TransformComponent>(ground);
        scene->world.AddComponent<ColliderComponent>(ground);
        PhysicComponent& physicGround = scene->world.AddComponent<PhysicComponent>(ground);
		physicGround.massInverse = 0.0f;
		physicGround.staticFriction = 0.8f;
		physicGround.dynamicFriction = 0.6f;
		physicGround.restitution = 0.0f;
        MeshRenderer& mGround = scene->world.AddComponent<MeshRenderer>(ground);
        mGround.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        mGround.material = mat;
        tGround.local.SetPosition(XMFLOAT3(0.0f, -1.0f, 0.0f));
        tGround.local.SetScale(XMFLOAT3(50.0f, 1.0f, 50.f));

        EntityId e = scene->world.CreateEntity();
        TransformComponent& transform = scene->world.AddComponent<TransformComponent>(e);
		scene->world.AddComponent<ColliderComponent>(e);
        PhysicComponent& physic = scene->world.AddComponent<PhysicComponent>(e);
        physic.type = BodyType::Dynamic;
		physic.SetMass(1.0f);
        physic.useGravity = false;
        physic.rotation = true;
        MeshRenderer& m = scene->world.AddComponent<MeshRenderer>(e);
        m.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        transform.local.SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

		EntityId e1 = scene->world.CreateEntity();
		TransformComponent& t1 = scene->world.AddComponent<TransformComponent>(e1);
		scene->world.AddComponent<ColliderComponent>(e1);
        PhysicComponent& physic1 = scene->world.AddComponent<PhysicComponent>(e1);
        physic1.type = BodyType::Dynamic;
        physic1.SetMass(1.0f);
		physic1.velocity = XMFLOAT3(-3.0f, 0.0f, 0.0f);
		physic1.useGravity = true;
		physic1.rotation = true;
		MeshRenderer& m1 = scene->world.AddComponent<MeshRenderer>(e1);
		m1.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        t1.local.SetPosition(XMFLOAT3(5.0f, 0.0f, 0.5f));
        scene->world.AddScript<MoveScript>(e1);

        EntityId e2 = scene->world.CreateEntity();
        TransformComponent& t2 = scene->world.AddComponent<TransformComponent>(e2);
        t2.local.SetPosition(XMFLOAT3(0.0f, 0.0f, -5.0f));
        CameraComponent& cam = scene->world.AddComponent<CameraComponent>(e2);
        cam.camera = RessourceManager::GetCamera(RessourceManager::AddCamera("Default"));
        cam.isMainCamera = true;
        scene->world.AddScript<TestScript>(e2);

        EntityId light = scene->world.CreateEntity();
        TransformComponent& tLight = scene->world.AddComponent<TransformComponent>(light);
        tLight.local.SetPosition(XMFLOAT3(2.0f, 2.0f, 0.0f));
        LightComponent& l = scene->world.AddComponent<LightComponent>(light);
        l.SetPoint(1.0f, 10.0f, 1);

        EngineManager::GetInstance().Run();
    }
};

#endif 