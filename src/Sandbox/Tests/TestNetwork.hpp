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
            
        }
    };
    
    static void Run(LPSTR lpCmdLine = nullptr)
    {
        String args = lpCmdLine;

        bool isHost = args.find("--host") != String::npos;

        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        if (isHost)
        {
            EngineManager::GetInstance().HostServer();
        
            Scene* scene = SceneManager::SetCurrentScene("Default");
        
            EntityId e = scene->world.CreateEntity();
            scene->world.AddScript<TestScript>(e);
        }
        else
            EngineManager::GetInstance().Connect("127.0.0.1", 1888);
        
        EngineManager::GetInstance().Run();
    }
};

#endif