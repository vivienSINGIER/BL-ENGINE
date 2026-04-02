#ifndef LABYRINTHE_HELPER_H_DEFINED
#define LABYRINTHE_HELPER_H_DEFINED

#include "../Engine/Engine.h"

class LabyrintheHelper
{
public:
	static int RandomInt(int _min, int _max);
	static void Enclose(Vector<Vector<char>>& _grid);
	static void Recursive_division(Vector<Vector<char>>& _grid, int _xMin, int _xMax, int _yMin, int _yMax);
	static void Lobby(Vector<Vector<char>>& _grid, int _xMin, int _xMax, int _yMin, int _yMax);
	static void BetweenDivisionH(Vector<Vector<char>>& _grid, int _x, int _y, int _width);
	static void BetweenDivisionV(Vector<Vector<char>>& _grid, int _x, int _y, int _height);
	static void PierceWallH(Vector<Vector<char>>& _grid, int _y, int _xStart, int _xEnd, int _heightGrid);
	static void PierceWallV(Vector<Vector<char>>& _grid, int _x, int _yStart, int _yEnd, int _widthGrid);
	static void printGrid(Vector<Vector<char>>& _grid);
	static void bfs_check(Vector<Vector<char>>& grid);
};

#endif // !LABYRINTHE_HELPER_H_DEFINED
