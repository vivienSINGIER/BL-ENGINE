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

    static void Recursive_division(std::vector<std::vector<char>>& _grid,int _xMin, int _xMax, int _yMin, int _yMax)
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

    static void BetweenDivisionH(std::vector<std::vector<char>>& _grid, int _x, int _y, int _width)
    {
        _grid[_x][_y] = ' ';
        if (_x > 0 && _grid[_x - 1][_y] == 'X') _grid[_x - 1][_y] = ' ';
        if (_x < _width - 1 && _grid[_x + 1][_y] == 'X') _grid[_x + 1][_y] = ' ';
    }

    static void BetweenDivisionV(std::vector<std::vector<char>>& _grid, int _x, int _y, int _height)
    {
        _grid[_x][_y] = ' ';
        if (_y > 0 && _grid[_x][_y - 1] == 'X') _grid[_x][_y - 1] = ' ';
        if (_y < _height - 1 && _grid[_x][_y + 1] == 'X') _grid[_x][_y + 1] = ' ';
    }

    static void PierceWallH(std::vector<std::vector<char>>& _grid, int _y, int _xStart, int _xEnd, int _heightGrid)
    {
        std::vector<int> positions;
        for (int i = _xStart + 1; i < _xEnd; i += 2)
            positions.push_back(i);

        for (int k = positions.size() - 1; k > 0; k--)
            std::swap(positions[k], positions[RandomInt(0, k)]);

        for (int i : positions)
        {
            if (_grid[i][_y - 1] != 'X' && _grid[i][_y + 1] != 'X')
            {
                BetweenDivisionV(_grid, i, _y, _heightGrid);
                return;
            }
        }
        BetweenDivisionV(_grid, positions[0], _y, _heightGrid);
    }

    static void PierceWallV(std::vector<std::vector<char>>& _grid, int _x, int _yStart, int _yEnd, int _widthGrid)
    {
        std::vector<int> positions;
        for (int j = _yStart + 1; j < _yEnd; j += 2)
            positions.push_back(j);

        for (int k = positions.size() - 1; k > 0; k--)
            std::swap(positions[k], positions[RandomInt(0, k)]);

        for (int j : positions)
        {
            if (_grid[_x - 1][j] != 'X' && _grid[_x + 1][j] != 'X')
            {
                BetweenDivisionH(_grid, _x, j, _widthGrid);
                return;
            }
        }
        BetweenDivisionH(_grid, _x, positions[0], _widthGrid);
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

    static void Laby3d(std::vector<std::vector<char>>& _grid)
    {
		Scene* scene = SceneManager::GetSceneWithName("Default");

        for (int x = 0; x < _grid.size(); x++)
        {
            for (int y = 0; y < _grid[0].size(); y++)
                if (_grid[x][y] == 'X')
                {
                    EntityId e = scene->world->CreateEntity();
                    TransformComponent& t = scene->world->AddComponent<TransformComponent>(e);
                    MeshRenderer& m = scene->world->AddComponent<MeshRenderer>(e);
                    // m.geo = RessourceManager::GetGeometry("CUBE");
                    t.local.SetPosition(XMFLOAT3(x, 0.0f, y));
                }
        }
	}

    static void Run()
    {
		srand(time(nullptr));
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Transform");
  //       Scene* scene = SceneManager::GetSceneWithName("Default");
  //       scene->world->RegisterSystem<TransformSystem>(Phase::Update);
  //       scene->world->RegisterSystem<MeshRendererSystem>(Phase::Render);
		// scene->world->RegisterSystem<LightSystem>(Phase::PreRender);

		RessourceManager::AddGeometry("CUBE", GeometryFactory::BuildCube(EngineManager::GetDevice()));

        Material* mat = RessourceManager::GetShader("Color")->CreateMaterial();
        RessourceManager::AddMaterial("debug_ref", mat);
        mat->SetFloat4("DiffuseAlbedo", XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));

        Camera cam;
        XMFLOAT3 pos = XMFLOAT3( 50.0f, 20.0f, 50.0f);
        cam.SetPos(pos);
        XMFLOAT3 target = XMFLOAT3(0.1f, 0.0f, 0.0f);
        cam.LookAt(target);

		// EntityId ground = scene->world->CreateEntity();
		// TransformComponent& t = scene->world->AddComponent<TransformComponent>(ground);
		// MeshRenderer& m = scene->world->AddComponent<MeshRenderer>(ground);
		// // m.geo = RessourceManager::GetGeometry("CUBE");
		// t.local.SetPosition(XMFLOAT3(m_widthGrid / 2.0f, -1.0f, m_heightGrid / 2.0f));
		// t.local.SetScale(XMFLOAT3(m_widthGrid, 1.0f, m_heightGrid));
		// // m.material = mat;
  //
		// EntityId light = scene->world->CreateEntity();
		// TransformComponent& tLight = scene->world->AddComponent<TransformComponent>(light);
		// LightComponent& l = scene->world->AddComponent<LightComponent>(light);
  //       l.type = LightType::Directional;
		// l.SetStrength(0.5f);
		// tLight.local.LookTo(XMFLOAT3(-1.0f, -1.0f, -1.0f));


        EngineManager::GetDevice()->SetMainCamera(&cam);

		std::vector<std::vector<char>> grid(m_widthGrid, std::vector<char>(m_heightGrid, ' '));
	    
        Enclose(grid);

        int cx = m_widthGrid / 2;
        int cy = m_heightGrid / 2;
        if (cx % 2 == 0) cx--;
        if (cy % 2 == 0) cy--;

        int rW = 5, rH = 5;
        int rxMin = cx - rW; if (rxMin % 2 == 1) rxMin--;
        int rxMax = cx + rW; if (rxMax % 2 == 1) rxMax++;
        int ryMin = cy - rH; if (ryMin % 2 == 1) ryMin--;
        int ryMax = cy + rH; if (ryMax % 2 == 1) ryMax++;

        // Murs de s�paration
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

        PierceWallH(grid, ryMin, 0, rxMin, m_heightGrid);
        PierceWallH(grid, ryMin, rxMax, m_widthGrid - 1, m_heightGrid);

        PierceWallH(grid, ryMax, 0, rxMin, m_heightGrid);
        PierceWallH(grid, ryMax, rxMax, m_widthGrid - 1, m_heightGrid);

        PierceWallV(grid, rxMin, 0, ryMin, m_widthGrid);
        PierceWallV(grid, rxMin, ryMax, m_heightGrid - 1, m_widthGrid);

        PierceWallV(grid, rxMax, 0, ryMin, m_widthGrid);
        PierceWallV(grid, rxMax, ryMax, m_heightGrid - 1, m_widthGrid);

        Lobby(grid, rxMin, rxMax, ryMin, ryMax);
		Laby3d(grid);

		system("cls");
        bfs_check(grid);
		printGrid(grid);

        EngineManager::GetInstance().Run();

    }

private:

	static const int m_widthGrid = 41;
    static const int m_heightGrid = 41;

};

#endif
