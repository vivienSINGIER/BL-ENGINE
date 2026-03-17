#ifndef TEST_COLLISION_HPP_DEFINED
#define TEST_COLLISION_HPP_DEFINED

class TestCollision : public Test
{
public:
    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Collision");
        Scene* scene = SceneManager::GetSceneWithName("Default");
        scene->world.RegisterSystem<TransformSystem>(Phase::Update);
        scene->world.RegisterSystem<MeshRendererSystem>(Phase::Render);
		scene->world.RegisterSystem<ColliderSystem>(Phase::Update);

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
        scene->world.GetComponent<TransformComponent>(e2).transform.SetWorldPosition(XMFLOAT3(10.0f, 0.0f, 0.0f));

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