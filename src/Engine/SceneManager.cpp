#include "SceneManager.h"

#include "EngineManager.h"
#include "Scene.h"
#include "Network/Packet.hpp"
#include "Network/Server.h"

SceneManager::SceneManager()
{
	s_pSceneManager = this;

	Scene* pDefaultScene = CreateScene("Default");

	m_pCurrentScene = pDefaultScene;
}

Scene* SceneManager::GetSceneWithName(String const& _name)
{
	for (auto [name, id] : s_pSceneManager->m_sceneIds)
	{
		if (name == _name)
			return s_pSceneManager->m_scenes[id];
	}

	return nullptr;
}

Scene* SceneManager::GetSceneWithId(uint32 _id)
{
	for (auto [name, id] : s_pSceneManager->m_sceneIds)
	{
		if (id == _id)
			return s_pSceneManager->m_scenes[id];
	}

	return nullptr;
}

Scene* SceneManager::CreateScene(String const& _name, int32 _id)
{
	assert(_name.size() < 25 && "Scene name is too big");

	if (GetSceneWithName(_name) != nullptr)
		return GetSceneWithName(_name);
	
	Scene* pNewScene = new Scene();

	uint32 id;
	if (_id == -1)
		id = s_pSceneManager->m_scenes.size();
	else
		id = (uint32)_id;
	
	pNewScene->Init(_name, id);

	s_pSceneManager->m_sceneIds[_name] = id;
	s_pSceneManager->m_scenes.push_back(pNewScene);

	if (EngineManager::GetServer() != nullptr)
	{
		Server* pServer = EngineManager::GetServer();

		Packet p;
		p.header.type = PacketType::AddScene;
		p.header.sceneId = id;
		p.addScene.nameSize = _name.size();
		memcpy(p.addScene.name, _name.c_str(), _name.size());

		pServer->SendGeneralReliablePacket(p);
	}
	
	return pNewScene;
}

Scene* SceneManager::SetCurrentScene(Scene* _pScene)
{
	if (_pScene == nullptr)
		return s_pSceneManager->m_pCurrentScene;
	if (s_pSceneManager->GetSceneWithName(_pScene->GetName()) == nullptr)
		s_pSceneManager->m_pCurrentScene;

	s_pSceneManager->m_pCurrentScene->OnEnd();
	s_pSceneManager->m_pCurrentScene = _pScene;
	s_pSceneManager->m_pCurrentScene->OnStart();

	SendSetScenePacket(s_pSceneManager->m_sceneIds[_pScene->GetName()]);

	return s_pSceneManager->m_pCurrentScene;
}

Scene* SceneManager::SetCurrentScene(String const& _name)
{
	return SetCurrentScene(s_pSceneManager->GetSceneWithName(_name));
}

Scene* SceneManager::SetCurrentScene(uint32 _id)
{
	return SetCurrentScene(s_pSceneManager->GetSceneWithId(_id));
}

void SceneManager::SendSetScenePacket(uint32 _id)
{
	if (EngineManager::GetServer() == nullptr) return;
	
	Server* pServer = EngineManager::GetServer();

	Packet p;
	p.header.type = PacketType::SetScene;
	p.header.sceneId = _id;

	pServer->SendGeneralReliablePacket(p);
}
