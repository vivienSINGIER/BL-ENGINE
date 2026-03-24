#ifndef TEST_NETWORK_H_DEFINED
#define TEST_NETWORK_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"

class TestNetwork : public Test
{
public:
    struct TestScript : public IScript
    {
        void Update(float _dt) override
        {
            if (InputManager::IsKeyPressed(K))
            {
                SceneManager::CreateScene("Test");
            }

            if (InputManager::IsKeyPressed(L))
            {
                SceneManager::SetCurrentScene("Test");
            }
        }
    };
    
    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        EngineManager::GetInstance().HostServer();
        
        Scene* scene = SceneManager::SetCurrentScene("Default");
        
        EntityId e = scene->world.CreateEntity();
        scene->world.AddScript<TestScript>(e);
        
        EngineManager::GetInstance().Run();
    }
};

#endif