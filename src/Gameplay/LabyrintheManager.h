#ifndef LABYRINTHE_MANAGER_H_DEFINED
#define LABYRINTHE_MANAGER_H_DEFINED

#include "../Engine/Engine.h"

class LabyrintheManager
{
public:
	static void CreateLabyrinthe(int _width, int _height);
	static void Laby3d(Vector<Vector<char>>& _grid);
	static void SpawnItems(Vector<Vector<char>>& _grid, int _count, int _xMin, int _xMax, int _yMin, int _yMax);
};
#endif // !LABYRINTHE_MANAGER_H_DEFINED
