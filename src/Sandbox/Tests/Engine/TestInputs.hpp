#ifndef TEST_INPUTS_H_DEFINED
#define TEST_INPUTS_H_DEFINED

#include "Test.hpp"
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

            if (InputManager::IsKey(Z))
            {
				std::cout << "Z is pressed" << std::endl;
            }
        }
    };

    void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        Scene* scene = SceneManager::GetSceneWithName("Default");

        EntityId e = scene->world->CreateEntity();
        scene->world->AddComponent<TransformComponent>(e);
        MeshRenderer& m = scene->world->AddComponent<MeshRenderer>(e);
        m.geoId = RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
        scene->world->AddScript<TestScript>(e);

        Camera cam;
        Transform camT;
        camT.SetPosition(Vect3f32(0.0f, -3.0f, -3.0f));
        camT.LookAt({0.0f, 0.0f, 0.0f});
        
        cam.SetWorld(camT.GetMatrix());

        EngineManager::GetDevice()->SetMainCamera(&cam);

        EngineManager::GetInstance().Run();
    }
};

#endif