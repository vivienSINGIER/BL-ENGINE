#ifndef TEST_LABY_H_DEFINED
#define TEST_LABY_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"
#include <random>


class TestLaby: public Test
{
public:
    
    static int RandomInt(int _min, int _max)
    {
		return _min + rand() % (_max - _min + 1);
	}

    static void Enclose(std::vector<std::vector<char>>& _grid)
    {
		int h = _grid.size();
		int w = _grid[0].size();

        for (int x = 0; x < h; x++)
        {
            _grid[x][0] = 'X';
            _grid[x][w - 1] = 'X';
		}
        for (int y = 0; y < w; y++)
        {
            _grid[0][y] = 'X';
            _grid[h - 1][y] = 'X';
		}
	}

    static void Recursive_division(std::vector<std::vector<char>>& _grid, int _xMin, int _xMax, int _yMin, int _yMax)
    {
        if (_yMax - _yMin > _xMax - _xMin)
        {
            if(_yMax - _yMin <= 2) return;

            int x = RandomInt(_xMin + 1, _xMax );
            if ((x - _xMin) % 2 == 0)
                x += (RandomInt(0, 2) == 0) ? 1 : -1;

			int y = RandomInt(_yMin + 2, _yMax - 1);
			if ((y - _yMin) % 2 == 1)
				y += (RandomInt(0, 2) == 0) ? 1 : -1;

			for (int i = _xMin + 1; i < _xMax; i++)
				if (i != x)
					_grid[i][y] = 'X';

            if (y - _yMin > 2)
            {
                Recursive_division(_grid, _xMin, _xMax, _yMin, y);
				system("cls");
				printGrid(_grid);
				Sleep(500);
            }
           
            if (_yMax - y > 2)
            {
                Recursive_division(_grid, _xMin, _xMax, y, _yMax);
                system("cls");
                printGrid(_grid);
                Sleep(500);
            }
        }
        else
        {
			if (_xMax - _xMin <= 2) return;

			int x = RandomInt(_xMin + 2, _xMax - 1);
			if ((x - _xMin) % 2 == 1)
				x += (RandomInt(0, 2) == 0) ? 1 : -1;

			int y = RandomInt(_yMin + 1, _yMax);
			if ((y - _yMin) % 2 == 0)
				y += (RandomInt(0, 2) == 0) ? 1 : -1;

			for (int i = _yMin + 1; i < _yMax; i++)
				if (i != y)
                    _grid[x][i] = 'X';

            if (x - _xMin > 2)
            {
                Recursive_division(_grid, _xMin, x, _yMin, _yMax);
                system("cls");
                printGrid(_grid);
                Sleep(500);
            }

            if (_xMax - x > 2)
            {
                Recursive_division(_grid, x, _xMax, _yMin, _yMax);
                system("cls");
                printGrid(_grid);
                Sleep(500);
            }

        }
    }

    static void printGrid(std::vector<std::vector<char>>& _grid)
    {
        for (auto& row : _grid)
        {
            for (auto& cell : row)
				std::cout << (cell == 'X' ? "W" : " ");
            std::cout << std::endl;
        }
	}


    static void Run()
    {
		srand(time(nullptr));
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
        Scene* scene = SceneManager::GetSceneWithName("Default");
        scene->world.RegisterSystem<TransformSystem>(Phase::Update);
        scene->world.RegisterSystem<MeshRendererSystem>(Phase::Render);

        EntityId e = scene->world.CreateEntity();
        scene->world.AddComponent<TransformComponent>(e);
        MeshRenderer& m = scene->world.AddComponent<MeshRenderer>(e);
        m.geo = GeometryFactory::BuildCube(EngineManager::GetDevice());


        Camera cam;
        XMFLOAT3 pos = XMFLOAT3(0.0f, -3.0f, -3.0f);
        cam.SetPos(pos);
        XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
        cam.LookAt(target);

        EngineManager::GetDevice()->SetMainCamera(&cam);

		std::vector<std::vector<char>> grid(m_widthGrid, std::vector<char>(m_heightGrid, ' '));
	    
		Enclose(grid);

		Recursive_division(grid, 0, m_widthGrid - 1, 0, m_heightGrid - 1);
		system("cls");
		printGrid(grid);

        EngineManager::GetInstance().Run();

    }

private:

	static const int m_widthGrid = 21;
    static const int m_heightGrid = 41;

};

#endif