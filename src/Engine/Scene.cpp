#ifndef SCENE_CPP_DEFINED
#define SCENE_CPP_DEFINED

#include "Scene.h"
#include "Engine.h"

Scene::Scene()
{
    world = new World();
}

Scene::~Scene()
{
    delete world;
}

void Scene::Init(String const& _name, uint32 _id)
{
    m_name = _name;
    m_id = _id;
    world->m_sceneId = _id;
    
    OnInit();
}

void Scene::Update(float _dt)
{
    OnUpdate(_dt);
    world->Update(_dt);
}

#endif
