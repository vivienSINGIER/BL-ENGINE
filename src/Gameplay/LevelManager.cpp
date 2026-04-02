#include "LevelManager.h"
#include "LabyrintheManager.h"

#include <iostream>

void LevelManager::Init(int _nbPlayer)
{
	m_cellSize = 3.0f;
	m_levelNb = 1;
	m_nbPlayer = _nbPlayer;

	m_scene = SceneManager::GetSceneWithName("MainScene");
	LabyrintheManager::Init(m_cellSize, m_scene, m_nbPlayer);
	LoadLevel(m_levelNb, m_nbPlayer);
}

void LevelManager::LoadLevel(int levelNb, int nbPlayer)
{

	m_levelSize = 21 + ((levelNb - 1) * 2 * nbPlayer);

	if (m_levelSize > 51) m_levelSize = 51;

	m_lightPosXStart = -m_levelSize * m_cellSize * 0.5f - 5.0f;

	m_lightTravelDistance = m_levelSize * m_cellSize + 10.0f;

	ReloadLabyrinthe(m_levelSize, m_levelSize);
	std::cout << "Level " << levelNb << " loaded with " << nbPlayer << " player(s) " << "LevelSize " << m_levelSize << std::endl;
}


void LevelManager::ReloadLabyrinthe(int _width, int _height)
{
	LabyrintheManager::ReloadDoor();
	LabyrintheManager::DestroyLabyrinthe();
	LabyrintheManager::CreateLabyrinthe(_width, _height);

	m_levelNb++;
}

