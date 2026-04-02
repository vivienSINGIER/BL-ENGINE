#ifndef LEVEL_MANAGER_H_DEFINED
#define LEVEL_MANAGER_H_DEFINED

#include "../Engine/Engine.h"

class LevelManager
{
public:
	static void Init(int _nbPlayer);
	static void LoadLevel();
	static void ReloadLabyrinthe(int _width, int _height);
	static void SetCellSize(int _cellSize) { m_cellSize = _cellSize; }
	static void CloseDoor(int doorIndex);
	static void OpenDoor(int doorIndex);
	static float GetLightPosXStart() { return m_lightPosXStart; }
	static float GetLightTravelDistance() { return m_lightTravelDistance; }

private:
	static int m_cellSize;
	static Scene* m_scene;
	static int m_levelSize;
	static int m_levelNb;
	static int m_nbPlayer;

	static float m_lightPosXStart;
	static float m_lightTravelDistance;

};

#endif // !LEVEL_MANAGER_H_DEFINED

