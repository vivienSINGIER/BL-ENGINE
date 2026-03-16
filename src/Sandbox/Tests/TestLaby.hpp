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

            int y = RandomInt(_yMin + 1, _yMax - 1);
            if (y % 2 == 1) y++;
            if (y >= _yMax) y -= 2;
            if (y <= _yMin) return;

            int x = RandomInt(_xMin, _xMax - 1);
            if (x % 2 == 0) x++;
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

            int x = RandomInt(_xMin + 1, _xMax - 1);
            if (x % 2 == 1) x++;
            if (x >= _xMax) x -= 2;
            if (x <= _xMin) return;

            int y = RandomInt(_yMin, _yMax - 1);
            if (y % 2 == 0) y++;
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

    static void Lobby(std::vector<std::vector<char>>& _grid, int _xMin, int _xMax, int _yMin, int _yMax)
    {
        for (int i = _xMin; i <= _xMax; i++)
            for (int j = _yMin; j <= _yMax; j++)
                _grid[i][j] = ' ';

        for (int i = _xMin; i <= _xMax; i++) { _grid[i][_yMin] = 'X'; _grid[i][_yMax] = 'X'; }
        for (int j = _yMin; j <= _yMax; j++) { _grid[_xMin][j] = 'X'; _grid[_xMax][j] = 'X'; }

        _grid[_xMin][(_yMin + _yMax) / 2] = ' '; // haut
        _grid[_xMax][(_yMin + _yMax) / 2] = ' '; // bas
        _grid[(_xMin + _xMax) / 2][_yMin] = ' '; // gauche
        _grid[(_xMin + _xMax) / 2][_yMax] = ' '; // droite
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

        int cx = m_widthGrid / 2;
        int cy = m_heightGrid / 2;
        if (cx % 2 == 0) cx--;
        if (cy % 2 == 0) cy--;

        int rW = 3, rH = 3;
        int rxMin = cx - rW; if (rxMin % 2 == 1) rxMin--;
        int rxMax = cx + rW; if (rxMax % 2 == 1) rxMax++;
        int ryMin = cy - rH; if (ryMin % 2 == 1) ryMin--;
        int ryMax = cy + rH; if (ryMax % 2 == 1) ryMax++;

        // Murs de séparation
        for (int i = 0; i < m_widthGrid; i++)
        {
            grid[i][ryMin] = 'X';
            grid[i][ryMax] = 'X';
        }
        for (int j = 0; j < m_heightGrid; j++)
        {
            grid[rxMin][j] = 'X';
            grid[rxMax][j] = 'X';
        }

        // 8 zones
        Recursive_division(grid, 0, rxMin, 0, ryMin);
        Recursive_division(grid, rxMax, m_widthGrid - 1, 0, ryMin);
        Recursive_division(grid, 0, rxMin, ryMax, m_heightGrid - 1);
        Recursive_division(grid, rxMax, m_widthGrid - 1, ryMax, m_heightGrid - 1);
        Recursive_division(grid, rxMin, rxMax, 0, ryMin);
        Recursive_division(grid, rxMin, rxMax, ryMax, m_heightGrid - 1);
        Recursive_division(grid, 0, rxMin, ryMin, ryMax);
        Recursive_division(grid, rxMax, m_widthGrid - 1, ryMin, ryMax);

        // Percer APRES la generation
        int midX = (rxMin + rxMax) / 2;
        int midY = (ryMin + ryMax) / 2;

        // Mur horizontal ryMin : une ouverture par zone (toutes les 2 cellules impaires)
        for (int i = 1; i < m_widthGrid - 1; i += 2)
            if (i != rxMin && i != rxMax)
                grid[i][ryMin] = ' ';

        // Mur horizontal ryMax
        for (int i = 1; i < m_widthGrid - 1; i += 2)
            if (i != rxMin && i != rxMax)
                grid[i][ryMax] = ' ';

        // Mur vertical rxMin
        for (int j = 1; j < m_heightGrid - 1; j += 2)
            if (j != ryMin && j != ryMax)
                grid[rxMin][j] = ' ';

        // Mur vertical rxMax
        for (int j = 1; j < m_heightGrid - 1; j += 2)
            if (j != ryMin && j != ryMax)
                grid[rxMax][j] = ' ';

        Lobby(grid, rxMin, rxMax, ryMin, ryMax);

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