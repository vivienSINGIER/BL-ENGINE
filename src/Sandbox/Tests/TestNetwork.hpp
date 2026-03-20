#ifndef TEST_NETWORK_H_DEFINED
#define TEST_NETWORK_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"

class TestNetwork : public Test
{
public:
    struct TestScript : public IScript
    {
        
    };
    
    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        Scene* scene = SceneManager::GetSceneWithName("Default");
        scene->world.RegisterSystem<TransformSystem>(Phase::Update);
        scene->world.RegisterSystem<MeshRendererSystem>(Phase::Render);
        scene->world.RegisterSystem<CameraSystem>(Phase::PreRender);
        scene->world.RegisterSystem<LightSystem>(Phase::PreRender);
        scene->world.RegisterSystem<NetworkSystem>(Phase::Update);

        EngineManager::GetInstance().HostServer();
        
        EngineManager::GetInstance().Run();
    }
};

#endif