#ifndef LABYRINTHE_MANAGER_H_DEFINED
#define LABYRINTHE_MANAGER_H_DEFINED

#include "../Engine/Engine.h"

class LabyrintheManager
{
public:
	static void Init(int _cellSize, Scene* _scene, int _nbPlayer);
	static void CreateLabyrinthe(int _width, int _height);
	static void Laby3d(Vector<Vector<char>>& _grid);
	static void DestroyLabyrinthe();

	static void SetLevelNb(int _levelNb) { m_levelNb = _levelNb; }
	static void ReloadDoor();
	
	static void CloseDoor(int doorIndex);
	static void OpenDoor(int doorIndex);
	static void OpenRandomDoor();

private:
	static float m_cellSize;
	static Scene* m_scene;
	static Vector<EntityId> m_Entities;
	static int m_levelNb;
	static EntityId m_doors[4];
	static int m_nbPlayer;

	static int m_doorOpenedNight;
};
#endif // !LABYRINTHE_MANAGER_H_DEFINED
