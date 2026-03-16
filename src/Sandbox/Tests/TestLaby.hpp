#ifndef TEST_LABY_H_DEFINED
#define TEST_LABY_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"

class TestLaby: public Test
{
public:
    
    static void GenerateGrid()
    {
        for(int i = 0; i < 10; i++)
        {
            for(int j = 0; j < 10; j++)
            {
                std::cout << "X";
            }
            std::cout << std::endl;
		}
    }

    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        Scene* scene = SceneManager::GetSceneWithName("Default");
        scene->world.RegisterSystem<TransformSystem>(Phase::Update);
        scene->world.RegisterSystem<MeshRendererSystem>(Phase::Render);

        EntityId e = scene->world.CreateEntity();
        scene->world.AddComponent<TransformComponent>(e);
        MeshRenderer& m = scene->world.AddComponent<MeshRenderer>(e);
        m.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());


        Camera cam;
        XMFLOAT3 pos = XMFLOAT3(0.0f, -3.0f, -3.0f);
        cam.SetPos(pos);
        XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
        cam.LookAt(target);

        EngineManager::GetDevice()->SetMainCamera(&cam);

		std::cout << "Generating grid..." << std::endl;
        GenerateGrid();

        EngineManager::GetInstance().Run();

    }

private:

};

#endif