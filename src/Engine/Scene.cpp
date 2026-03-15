#ifndef SCENE_CPP_DEFINED
#define SCENE_CPP_DEFINED

#include "Scene.h"

void Scene::Init(String const& _name)
{
    m_name = _name;

    OnInit();
}

void Scene::Update(float _dt)
{
    world.Update(_dt);

    OnUpdate(_dt);
}

#endif
