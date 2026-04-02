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

void LevelManager::Init(int _nbPlayer)
{
	m_cellSize = 3.0f;
	m_levelNb = 1;
	m_nbPlayer = _nbPlayer;

	m_scene = SceneManager::GetSceneWithName("MainScene");
	LabyrintheManager::Init(m_cellSize, m_scene, m_nbPlayer);
	LoadLevel();
}

void LevelManager::LoadLevel()
{
	m_levelSize = 21 + ((m_levelNb - 1) * 2 * m_nbPlayer);

	if (m_levelSize > 51) m_levelSize = 51;

	m_lightPosXStart = -m_levelSize * m_cellSize * 0.5f - 5.0f;

	m_lightTravelDistance = m_levelSize * m_cellSize + 10.0f;

	ReloadLabyrinthe(m_levelSize, m_levelSize);
	std::cout << "Level " << m_levelNb << " loaded with " << m_nbPlayer << " player(s) " << "LevelSize " << m_levelSize << std::endl;
}


void LevelManager::ReloadLabyrinthe(int _width, int _height)
{
	LabyrintheManager::ReloadDoor();
	LabyrintheManager::DestroyLabyrinthe();
	LabyrintheManager::CreateLabyrinthe(_width, _height);

	m_levelNb++;
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

