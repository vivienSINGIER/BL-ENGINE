#include "SceneManager.h"
#include "Scene.h"

SceneManager::SceneManager()
{
	s_pSceneManager = this;

	Scene* pDefaultScene = CreateScene("Default");

	m_pCurrentScene = pDefaultScene;
}

Scene* SceneManager::GetSceneWithName(String const& _name)
{
	for (auto [name, pScene] : s_pSceneManager->m_mScenes)
	{
		if (name == _name)
			return pScene;
	}

	return nullptr;
}

Scene* SceneManager::CreateScene(String const& _name)
{
	Scene* pNewScene = new Scene;
	pNewScene->Init(_name);

	s_pSceneManager->m_mScenes[_name] = pNewScene;

	return pNewScene;
}

void SceneManager::ChangeCurrentScene(Scene* _pScene)
{
	if (_pScene == nullptr) return;

	if (!s_pSceneManager->m_mScenes.contains(_pScene->GetName()))
		s_pSceneManager->CreateScene(_pScene->GetName());

	s_pSceneManager->m_pCurrentScene->OnEnd();

	s_pSceneManager->m_pCurrentScene = _pScene;
	s_pSceneManager->m_pCurrentScene->OnStart();
}

void SceneManager::ChangeCurrentScene(String const& _name)
{
	ChangeCurrentScene(s_pSceneManager->GetSceneWithName(_name));
}
