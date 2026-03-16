#ifndef TEST_LABY_H_DEFINED
#define TEST_LABY_H_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"
#include <random>
#include <queue>


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

    static void Recursive_division(std::vector<std::vector<char>>& _grid,
        int _xMin, int _xMax, int _yMin, int _yMax)
    {
        if (_yMax - _yMin > _xMax - _xMin)
        {
            if (_yMax - _yMin <= 2) return;

            // Mur horizontal : choisit un y PAIR dans la sous-région
            int y = RandomInt(_yMin + 1, _yMax - 1);
            if (y % 2 == 1) y++;  // force y pair
            if (y >= _yMax) y -= 2;
            if (y <= _yMin) return;

            // Passage : choisit un x IMPAIR dans la sous-région
            int x = RandomInt(_xMin, _xMax - 1);
            if (x % 2 == 0) x++;  // force x impair
            if (x >= _xMax) x -= 2;
            if (x <= _xMin) return;

            for (int i = _xMin + 1; i < _xMax; i++)
                if (i != x)
                    _grid[i][y] = 'X';

            if (y - _yMin > 2)
                Recursive_division(_grid, _xMin, _xMax, _yMin, y);
            if (_yMax - y > 2)
                Recursive_division(_grid, _xMin, _xMax, y, _yMax);
        }
        else
        {
            if (_xMax - _xMin <= 2) return;

            // Mur vertical : choisit un x PAIR dans la sous-région
            int x = RandomInt(_xMin + 1, _xMax - 1);
            if (x % 2 == 1) x++;  // force x pair
            if (x >= _xMax) x -= 2;
            if (x <= _xMin) return;

            // Passage : choisit un y IMPAIR dans la sous-région
            int y = RandomInt(_yMin, _yMax - 1);
            if (y % 2 == 0) y++;  // force y impair
            if (y >= _yMax) y -= 2;
            if (y <= _yMin) return;

            for (int i = _yMin + 1; i < _yMax; i++)
                if (i != y)
                    _grid[x][i] = 'X';

            if (x - _xMin > 2)
                Recursive_division(_grid, _xMin, x, _yMin, _yMax);
            if (_xMax - x > 2)
                Recursive_division(_grid, x, _xMax, _yMin, _yMax);
        }
    }

    static void printGrid(std::vector<std::vector<char>>& _grid)
    {
        for (const auto& row : _grid) {
            for (char c : row) {
                if (c == 'X') std::cout << "WW";
                else if (c == '?') std::cout << "??"; // visible !
                else               std::cout << "  ";
            }
            std::cout << '\n';
        }
	}

    static void bfs_check(std::vector<std::vector<char>>& grid) {
        int h = grid.size(), w = grid[0].size();
        std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
        std::queue<std::pair<int, int>> q;

        q.push({ 1, 1 });
        visited[1][1] = true;

        int dx[] = { 0, 0, 1, -1 };
        int dy[] = { 1, -1, 0, 0 };

        while (!q.empty()) {
            auto [y, x] = q.front(); q.pop();
            for (int d = 0; d < 4; d++) {
                int nx = x + dx[d], ny = y + dy[d];
                if (nx >= 0 && nx < w && ny >= 0 && ny < h
                    && !visited[ny][nx] && grid[ny][nx] != 'X') {
                    visited[ny][nx] = true;
                    q.push({ ny, nx });
                }
            }
        }

        int unreachable = 0;
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                if (grid[y][x] == ' ' && !visited[y][x]) {
                    grid[y][x] = '?'; // zone inaccessible
                    unreachable++;
                }

        std::cerr << "Cellules inaccessibles : " << unreachable << "\n";
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
        bfs_check(grid);
		printGrid(grid);

        EngineManager::GetInstance().Run();

    }

private:

	static const int m_widthGrid = 21;
    static const int m_heightGrid = 41;

};

#endif