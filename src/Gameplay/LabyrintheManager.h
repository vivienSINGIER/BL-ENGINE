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

	static void NightGas(float _dt, float _nightDuration);
	static void StartGas(float _nightDuration);
	static void ResetGas();

	static void SetPlayer(int playerIndex, EntityId playerEntity);
	static void SpawnMagu(int _count);

	static void SetLevel(int _levelNb) { m_levelNb = _levelNb; }
private:
	static float m_cellSize;
	static float m_labySize;
	static Scene* m_scene;
	static Vector<EntityId> m_Entities;
	static Vector<EntityId> m_MaguEntities;
	static int m_levelNb;
	static EntityId m_doors[4];
	static int m_nbPlayer;
	static EntityId m_player[4];

	static int m_doorOpenedNight;
	static float m_gasWallTargetX[4];
	static float m_gasWallStartX[4];  
	static float m_gasTimer;
	static float m_gasDuration;
	static EntityId m_wallsGas[4];
};
#endif // !LABYRINTHE_MANAGER_H_DEFINED
