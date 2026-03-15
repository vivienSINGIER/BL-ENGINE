#ifndef SCENE_MANAGER_H_DEFINED
#define SCENE_MANAGER_H_DEFINED

#include "define.h"

class Scene;

class SceneManager
{
public:
	SceneManager();
	static SceneManager& GetInstance() { return *s_pSceneManager; }

	static Scene* GetCurrentScene() { return s_pSceneManager->m_pCurrentScene; }

	static Scene* GetSceneWithName(String const& _name);

	static Scene* CreateScene(String const& _name);

	template <typename SceneType>
	static SceneType* CreateSceneType(String const& _name);

	static void ChangeCurrentScene(Scene* _pScene);
	static void ChangeCurrentScene(String const& _name);

private:
	inline static SceneManager* s_pSceneManager = nullptr;

	UnorderedMap<String, Scene*> m_mScenes;
	Scene* m_pCurrentScene = nullptr;
};

#include "SceneManager.inl"

#endif

