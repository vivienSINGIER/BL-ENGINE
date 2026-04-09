#include "LevelManager.h"
#include "LabyrintheManager.h"
#include <iostream>

float   LevelManager::m_cellSize = 0;
Scene* LevelManager::m_scene = nullptr;
int   LevelManager::m_levelSize = 0;
int   LevelManager::m_levelNb = 0;
int   LevelManager::m_nbPlayer = 0;
float LevelManager::m_lightPosXStart = 0.0f;
float LevelManager::m_lightTravelDistance = 0.0f;
bool  LevelManager::m_levelLoaded = false;

void LevelManager::Init(int _nbPlayer)
{
	m_cellSize = 3.0f;
	m_levelNb = 0;
	m_nbPlayer = _nbPlayer;

	m_scene = SceneManager::GetSceneWithName("MainScene");
	LabyrintheManager::Init(m_cellSize, m_scene, m_nbPlayer);
	LoadLevel();
}

void LevelManager::LoadLevel()
{ 
	m_levelNb++;
	m_levelSize = 21 + ((m_levelNb - 1) * 2 * m_nbPlayer);

	if (m_levelSize > 51) m_levelSize = 51;

	m_lightPosXStart = -m_levelSize * m_cellSize * 0.5f - 5.0f;

	m_lightTravelDistance = m_levelSize * m_cellSize + 10.0f;

	ReloadLabyrinthe(m_levelSize, m_levelSize);
	LabyrintheManager::SetLevelNb(m_levelNb);
	std::cout << "Level " << m_levelNb << " loaded with " << m_nbPlayer << " player(s) " << "LevelSize " << m_levelSize << std::endl;
}

void LevelManager::ReloadLevel()
{
	m_levelSize = 21 + ((m_levelNb - 1) * 2 * m_nbPlayer);

	if (m_levelSize > 51) m_levelSize = 51;

	m_lightPosXStart = -m_levelSize * m_cellSize * 0.5f - 5.0f;

	m_lightTravelDistance = m_levelSize * m_cellSize + 10.0f;

	ReloadLabyrinthe(m_levelSize, m_levelSize);
	LabyrintheManager::SetLevelNb(m_levelNb);
	std::cout << "Level " << m_levelNb << " loaded with " << m_nbPlayer << " player(s) " << "LevelSize " << m_levelSize << std::endl;
}


void LevelManager::ReloadLabyrinthe(int _width, int _height)
{
	LabyrintheManager::ReloadDoor();
	LabyrintheManager::DestroyLabyrinthe();
	LabyrintheManager::CreateLabyrinthe(_width, _height);
}

void LevelManager::CloseDoor(int doorIndex)
{
	LabyrintheManager::CloseDoor(doorIndex);
}

void LevelManager::OpenDoor(int doorIndex)
{
	LabyrintheManager::OpenDoor(doorIndex);
}

void LevelManager::OpenRandomDoor()
{
	LabyrintheManager::OpenRandomDoor();
}

void LevelManager::StartGas(float _nightDuration)
{
	LabyrintheManager::StartGas(_nightDuration);
}
void LevelManager::NightGas(float _dt, float _nightDuration)
{
	LabyrintheManager::NightGas(_dt, _nightDuration);
}

void LevelManager::ResetGas()
{
	LabyrintheManager::ResetGas();
}

void LevelManager::SetPlayer(int playerIndex, EntityId playerEntity)
{
	LabyrintheManager::SetPlayer(playerIndex, playerEntity);
}

bool LevelManager::LevelLoaded()
{
	return m_levelLoaded;
}

void LevelManager::UnregisterEntity(EntityId* _entity, int _count)
{
	LabyrintheManager::UnregisterEntity(_entity, _count);
}

