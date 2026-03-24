#ifndef SCENE_CPP_DEFINED
#define SCENE_CPP_DEFINED

#include "Scene.h"
#include "Engine.h"

void Scene::Init(String const& _name)
{
    m_name = _name;

    world.RegisterSystem<TransformSystem>(Phase::Update);
    world.RegisterSystem<MeshRendererSystem>(Phase::Render);
    world.RegisterSystem<CameraSystem>(Phase::PreRender);
    world.RegisterSystem<LightSystem>(Phase::PreRender);
    world.RegisterSystem<ReceiveSystem>(Phase::NetworkReceive);
    world.RegisterSystem<SendSystem>(Phase::NetworkSend);
    
    OnInit();
}

void Scene::Update(float _dt)
{
    world.Update(_dt);

    OnUpdate(_dt);
}

#endif
