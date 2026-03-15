#ifndef SCENE_MANAGER_INL_DEFINED
#define SCENE_MANAGER_INL_DEFINED 

#include "SceneManager.h"
#include "Scene.h"

template <typename SceneType>
SceneType* SceneManager::CreateSceneType(String const& _name)
{
	SceneType* pNewScene = new SceneType;

	Scene* pScene = pNewScene;

	s_pSceneManager->m_mScenes[_name] = pScene;

	pScene->Init(_name);

	return pNewScene;
}

#endif