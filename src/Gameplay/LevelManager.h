#ifndef LEVEL_MANAGER_H_DEFINED
#define LEVEL_MANAGER_H_DEFINED

#include "../Engine/Engine.h"

class LevelManager
{
public:
	void LoadLevel(int levelNb, int nbPlayer);
	void ReloadLabyrinthe(int _width, int _height);
	void DestroyLabyrinthe();
};

#endif // !LEVEL_MANAGER_H_DEFINED

