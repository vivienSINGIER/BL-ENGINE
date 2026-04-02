#ifndef LEVEL_MANAGER_H_DEFINED
#define LEVEL_MANAGER_H_DEFINED

#include "../Engine/Engine.h"

class LevelManager
{
public:
	static void Init(int _nbPlayer);
	static void LoadLevel(int levelNb, int nbPlayer);
	static void ReloadLabyrinthe(int _width, int _height);
	static void SetCellSize(int _cellSize) { m_cellSize = _cellSize; }

private:
	static int m_cellSize;
	static Scene* m_scene;
	static int m_levelSize;
	static int m_levelNb;
	static int m_nbPlayer;

};

#endif // !LEVEL_MANAGER_H_DEFINED

