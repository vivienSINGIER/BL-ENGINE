#include "LevelManager.h"
#include "LabyrintheManager.h"

#include <iostream>

void LevelManager::LoadLevel(int levelNb, int nbPlayer)
{
	cellSize = 3.0f;
	m_doorOpenedNight = RandomInt(0, 3);
	m_levelSize = 21 + ((levelNb - 1) * 2 * nbPlayer);

	if (m_levelSize > 51) m_levelSize = 51;

	m_lightPosXStart = -m_levelSize * cellSize * 0.5f - 5.0f;

	m_lightTravelDistance = m_levelSize * cellSize + 10.0f;

	ReloadLabyrinthe(m_levelSize, m_levelSize);
	std::cout << "Level " << levelNb << " loaded with " << nbPlayer << " player(s) " << "LevelSize " << m_levelSize << std::endl;
	std::cout << "Labyrinthe generated with " << m_Entities.size() << " entities.\n";
}


void LevelManager::ReloadLabyrinthe(int _width, int _height)
{
	m_loadLaby = true;

	for (int i = 0; i < 4; i++)
		m_doors[i] = 0;

	DestroyLabyrinthe();
	LabyrintheManager::CreateLabyrinthe(_width, _height);

	m_loadLaby = false;
	m_levelNb++;
}

void LevelManager::DestroyLabyrinthe()
{
	for (EntityId e : m_Entities)
		world->DestroyEntity(e);

	m_Entities.clear();
	m_groundEntity = 0;
}
