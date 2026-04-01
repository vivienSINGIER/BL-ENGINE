#ifndef MAIN_SCENE_H_DEFINED
#define MAIN_SCENE_H_DEFINED

#include "../Engine/Engine.h"

class MainScene : public Scene
{
public:
	void OnInit() override;
	void OnUpdate(float _dt) override;
	void OnStart() override;
	void OnEnd() override;

private:
	EntityId m_camera;
	EntityId m_test;
	EntityId m_laby;

	bool m_loadLaby;
	EntityId m_doors[4];
	bool m_opened;
	float m_timer;
	float m_openDuration;

	bool m_started;

	//LABYRINTHE//
	void CreateLabyrinthe(int _width, int _height);
	void Laby3d(Vector<Vector<char>>& _grid);
	void SetStarted(bool _started) { m_started = _started; }
	void ReloadLabyrinthe(int _width, int _height);
	void DestroyLabyrinthe();
	int RandomInt(int _min, int _max);
	void Enclose(Vector<Vector<char>>& _grid);
	void Recursive_division(Vector<Vector<char>>& _grid, int _xMin, int _xMax, int _yMin, int _yMax);
	void Lobby(Vector<Vector<char>>& _grid, int _xMin, int _xMax, int _yMin, int _yMax);
	void BetweenDivisionH(Vector<Vector<char>>& _grid, int _x, int _y, int _width);
	void BetweenDivisionV(Vector<Vector<char>>& _grid, int _x, int _y, int _height);
	void PierceWallH(Vector<Vector<char>>& _grid, int _y, int _xStart, int _xEnd, int _heightGrid);
	void PierceWallV(Vector<Vector<char>>& _grid, int _x, int _yStart, int _yEnd, int _widthGrid);
	void printGrid(Vector<Vector<char>>& _grid);
	void bfs_check(Vector<Vector<char>>& grid);

	Vector<EntityId> m_wallEntities;
	EntityId m_groundEntity;
};

#endif // !MAIN_SCENE_H_DEFINED
