#ifndef SCRIPT_INL_DEFINED
#define SCRIPT_INL_DEFINED

#include "Scene.h"
#include "SceneManager.h"
#include "Script.h"
#include "World.h"

template<typename T>
T& IScript::AddComponent()
{
    Scene* s = SceneManager::GetSceneWithId(sceneId);
    return s->world->AddComponent<T>(entity);
}
template<typename T>
void IScript::RemoveComponent()
{
    assert(m_isStarted && "Cannot call RemoveComponent before Start, use Start instead of Awake");
    Scene* s = SceneManager::GetSceneWithId(sceneId);
    s->world->RemoveComponent<T>(entity);
}
template<typename T>
T& IScript::GetComponent()
{
    assert(m_isStarted && "Cannot call GetComponent before Start, use Start instead of Awake");
    Scene* s = SceneManager::GetSceneWithId(sceneId);
    return s->world->GetComponent<T>(entity);
}
template<typename T>
bool IScript::HasComponent()
{
    Scene* s = SceneManager::GetSceneWithId(sceneId);
    return s->world->HasComponent<T>(entity);
}


#endif