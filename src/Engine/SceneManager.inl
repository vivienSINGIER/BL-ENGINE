#ifndef SCENE_MANAGER_INL_DEFINED
#define SCENE_MANAGER_INL_DEFINED 

#include "Network/Server.h"

template <typename SceneType>
SceneType* SceneManager::CreateSceneType(String const& _name, int32 _id)
{
	assert(_name.size() < 25 && "Scene name is too big");
	
	SceneType* pNewScene = new SceneType();

	uint32 id;
	if (_id == -1)
		id = s_pSceneManager->m_scenes.size();
	else
		id = (uint32)_id;

	s_pSceneManager->m_sceneIds[_name] = id;
	s_pSceneManager->m_scenes.push_back(pNewScene);

	pNewScene->Init(_name, id);
	pNewScene->LoadRessources();

	if (EngineManager::GetServer() != nullptr)
	{
		Server* pServer = EngineManager::GetServer();

		Packet p;
		p.header.type = PacketType::AddScene;
		p.header.sceneId = id;
		p.addScene.nameSize = _name.size();
		memcpy(p.addScene.name, _name.c_str(), _name.size());

		pServer->RegisterPacket(p);
	}
	
	return pNewScene;
}

#endif