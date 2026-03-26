#ifndef TEST_INPUTS_H_DEFINED
#define TEST_INPUTS_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"
#include <InputManager.h> 

class TestInputs : public Test
{
public:
    struct TestScript : public IScript
    {

        void Update(float dt) override
        {
			InputManager::HandleInput();

            if (InputManager::IsKeyPressed(Z))
            {
				std::cout << "Z is pressed" << std::endl;
            }
        }
    };

    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        Scene* scene = SceneManager::GetSceneWithName("Default");

        EntityId e = scene->world->CreateEntity();
        scene->world->AddComponent<TransformComponent>(e);
        MeshRenderer& m = scene->world->AddComponent<MeshRenderer>(e);
        m.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());
        scene->world->AddScript<TestScript>(e);

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