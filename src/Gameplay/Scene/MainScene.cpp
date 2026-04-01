#include "MainScene.h"
#include <iostream>
#include "../Gameplay/Script/ScriptMovement.h"

void MainScene::OnInit()
{
	m_loadLaby = false;
	m_openDuration = 5.0f;
	m_opened = false;
	m_started = false;

    RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
    uint32 shaderId = RessourceManager::AddShader("LitColored", ShaderFactory::CreateLitColored(EngineManager::GetDevice()));
    Material* white = RessourceManager::GetShader(shaderId)->CreateMaterial();
    RessourceManager::AddMaterial("White", white);
	RessourceManager::AddCamera("Default");

	RessourceManager::AddGeometry("WaterBottle", GeometryFactory::LoadGeometry(EngineManager::GetDevice(), "../../res/Obj/WaterBottle.obj"));
	Camera* camObj = RessourceManager::GetCamera("Default");
	camObj->nearPlane = 0.01f;

	Texture* wallTexture = EngineManager::GetDevice()->CreateTexture(L"../../res/Textures/Bricks/bricks.dds");
	RessourceManager::AddTexture("Wall", wallTexture);
	uint32 shaderTextId = RessourceManager::AddShader("Textured", ShaderFactory::CreateLitTextured(EngineManager::GetDevice()));
	Material* wallMat = RessourceManager::GetShader(shaderTextId)->CreateMaterial();
	wallMat->SetTexture("Albedo", RessourceManager::GetTexture("Wall"));
	RessourceManager::AddMaterial("WallMaterial", wallMat);

	Texture* groundTexture = EngineManager::GetDevice()->CreateTexture(L"../../res/Textures/Rock/Albedo.dds");
	RessourceManager::AddTexture("Ground", groundTexture);
	Material* groundMat = RessourceManager::GetShader(shaderTextId)->CreateMaterial();
	groundMat->SetTexture("Albedo", RessourceManager::GetTexture("Ground"));
	RessourceManager::AddMaterial("GroundMaterial", groundMat);

	Texture* doorTexture = EngineManager::GetDevice()->CreateTexture(L"../../res/Textures/Wood/Albedo.dds");
	RessourceManager::AddTexture("Door", doorTexture);
	Material* doorMat = RessourceManager::GetShader(shaderTextId)->CreateMaterial();
	doorMat->SetTexture("Albedo", RessourceManager::GetTexture("Door"));
	RessourceManager::AddMaterial("DoorMaterial", doorMat);

	Texture* waterBottleTexture = EngineManager::GetDevice()->CreateTexture(L"../../res/Textures/Wood/Water.dds");
	RessourceManager::AddTexture("WaterBottle", waterBottleTexture);
	Material* waterBottleMat = RessourceManager::GetShader(shaderTextId)->CreateMaterial();
	waterBottleMat->SetTexture("Albedo", RessourceManager::GetTexture("WaterBottle"));
	RessourceManager::AddMaterial("WaterBottleMaterial", waterBottleMat);

	ComponentRegistry::RegisterScript<Movement::ScriptMovement>();

	m_camera = world->CreateEntity();
	TransformComponent& t = world->AddComponent<TransformComponent>(m_camera);
	t.local.SetPosition(XMFLOAT3(0.0f, 1.0f, 0.0f));
	t.world.LookTo(XMFLOAT3(-1.0f,-1.0f,-1.0f));
	CameraComponent& cam = world->AddComponent<CameraComponent>(m_camera);
	cam.camId = RessourceManager::GetCameraId("Default");
	cam.isMainCamera = true;
	world->AddScript<Movement::ScriptMovement>(m_camera);
	PhysicComponent& phys = world->AddComponent<PhysicComponent>(m_camera);
	ColliderComponent& col = world->AddComponent<ColliderComponent>(m_camera);
	phys.SetMass(1.0f);
	phys.ToggleGravity();

	EntityId light = world->CreateEntity();
	TransformComponent& lt = world->AddComponent<TransformComponent>(light);
	lt.local.SetPosition(XMFLOAT3(0.0f, 50.0f,0.0f));
	LightComponent& l = world->AddComponent<LightComponent>(light);
	l.type = LightType::Point;
	l.SetStrength(1.0f);
	l.SetPoint(1.0f, 100.0f);
	
}

void MainScene::OnUpdate(float _dt)
{
    if (m_loadLaby == true) return;

	if(InputManager::IsKeyDown(H))
	{
		m_started = true;
	}
	if(InputManager::IsKeyDown(J))
	{
		std::cout << "Reloading Labyrinthe...\n";
		ReloadLabyrinthe(31, 31);
	}

    if (m_started == true)
    {
        m_timer += _dt;
        if (m_timer >= m_openDuration)
        {
            m_timer = 0.0f;
            m_opened = !m_opened;
            if (m_opened)
            {
                for (int i = 0; i < 4; i++)
                {
                    if (m_doors[i] != 0)
                    {
                        world->SetInactive(m_doors[i]);
                    }
                }
            }
            else
            {
                for (int i = 0; i < 4; i++)
                {
                    if (m_doors[i] != 0)
                    {
                        world->SetActive(m_doors[i]);
                    }
                }
            }
        }
    }
}

void MainScene::OnStart()
{
	CreateLabyrinthe(21, 21);
}

void MainScene::OnEnd()
{
}

int MainScene::RandomInt(int _min, int _max)
{
    return _min + rand() % (_max - _min + 1);
}

void MainScene::Enclose(Vector<Vector<char>>& _grid)
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

void MainScene::Recursive_division(Vector<Vector<char>>& _grid, int _xMin, int _xMax, int _yMin, int _yMax)
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

void MainScene::Lobby(Vector<Vector<char>>& _grid, int _xMin, int _xMax, int _yMin, int _yMax)
{
    for (int i = _xMin; i <= _xMax; i++)
        for (int j = _yMin; j <= _yMax; j++)
            _grid[i][j] = ' ';

    for (int i = _xMin; i <= _xMax; i++) { _grid[i][_yMin] = 'X'; _grid[i][_yMax] = 'X'; }
    for (int j = _yMin; j <= _yMax; j++) { _grid[_xMin][j] = 'X'; _grid[_xMax][j] = 'X'; }

    _grid[_xMin][(_yMin + _yMax) / 2] = 'D'; // haut
    _grid[_xMax][(_yMin + _yMax) / 2] = 'D'; // bas
    _grid[(_xMin + _xMax) / 2][_yMin] = 'D'; // gauche
    _grid[(_xMin + _xMax) / 2][_yMax] = 'D'; // droite
}

void MainScene::BetweenDivisionH(Vector<Vector<char>>& _grid, int _x, int _y, int _width)
{
    _grid[_x][_y] = ' ';
    if (_x > 0 && _grid[_x - 1][_y] == 'X') _grid[_x - 1][_y] = ' ';
    if (_x < _width - 1 && _grid[_x + 1][_y] == 'X') _grid[_x + 1][_y] = ' ';
}

void MainScene::BetweenDivisionV(Vector<Vector<char>>& _grid, int _x, int _y, int _height)
{
    _grid[_x][_y] = ' ';
    if (_y > 0 && _grid[_x][_y - 1] == 'X') _grid[_x][_y - 1] = ' ';
    if (_y < _height - 1 && _grid[_x][_y + 1] == 'X') _grid[_x][_y + 1] = ' ';
}

void MainScene::PierceWallH(Vector<Vector<char>>& _grid, int _y, int _xStart, int _xEnd, int _heightGrid)
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

void MainScene::PierceWallV(Vector<Vector<char>>& _grid, int _x, int _yStart, int _yEnd, int _widthGrid)
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

void MainScene::printGrid(Vector<Vector<char>>& _grid)
{
    for (const auto& row : _grid) {
        for (char c : row) {
            if (c == 'X') std::cout << "WW";
            else if (c == '?') std::cout << "??"; // visible !
            else if (c == 'D') std::cout << "DD";
			else if (c == 'I') std::cout << "II";
            else               std::cout << "  ";
        }
        std::cout << '\n';
    }
}

void MainScene::bfs_check(Vector<Vector<char>>& grid)
{
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

void MainScene::SpawnItems(Vector<Vector<char>>& _grid, int _count,int _xMin, int _xMax, int _yMin, int _yMax)
{
	int width = _grid.size();
	int height = _grid[0].size();

	std::vector<std::pair<int, int>> emptyCells;

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            if(_grid[x][y] == ' ' && !(x >= _xMin && x <= _xMax && y >= _yMin && y <= _yMax))
				emptyCells.push_back({ x, y });
        }
	}

    for(int i = emptyCells.size() - 1; i > 0; i--)
		std::swap(emptyCells[i], emptyCells[RandomInt(0, i)]);

	int placed = 0;

    for (const auto& cell : emptyCells)
    {
        if (placed >= _count) break;
        int x = cell.first;
        int y = cell.second;
        EntityId itemEntity = world->CreateEntity();
        m_Entities.push_back(itemEntity);
        TransformComponent& tItem = world->AddComponent<TransformComponent>(itemEntity);
        MeshRenderer& m = world->AddComponent<MeshRenderer>(itemEntity);
        m.geoId = RessourceManager::GetGeometryId("WaterBottle");
        m.materialId = RessourceManager::GetMaterialId("WaterBottleMaterial");

		ColliderComponent& col = world->AddComponent<ColliderComponent>(itemEntity);
		PhysicComponent& phys = world->AddComponent<PhysicComponent>(itemEntity);
		phys.SetStatic();

        cellSize = 3.0f;
        float offsetX = _grid.size() * 0.5f * cellSize - cellSize / 2;
        float offsetY = _grid[0].size() * 0.5f * cellSize - cellSize / 2;
        tItem.local.SetPosition(XMFLOAT3(x * cellSize - offsetX, 1.0f, y * cellSize - offsetY));
		_grid[x][y] = 'I';
        placed++;
	}
}

void MainScene::Laby3d(Vector<Vector<char>>& _grid)
{

    float gridW = (float)_grid.size();
    float gridH = (float)_grid[0].size();

    float offsetX = gridW * 0.5f - 0.5f; 
    float offsetY = gridH * 0.5f - 0.5f;

    float wallHeight = 15.0f;

    cellSize = 3.0f;

    int doorCount = 0;

    Vector<Vector<bool>> visited(_grid.size(), Vector<bool>(_grid[0].size(), false));

    for (int x = 0; x < _grid.size(); x++)
    {
        for (int y = 0; y < _grid[0].size(); y++)
        {
            if (_grid[x][y] == 'X' && !visited[x][y])
            {
                int lenY = 1;

                while (y + lenY < _grid[0].size() && _grid[x][y + lenY] == 'X' && !visited[x][y + lenY])
                    lenY++;

                int lenX = 1;
                while (x + lenX < _grid.size())
                {
                    bool rowValid = true;
                    for (int j = 0; j < lenY; j++)
                    {
                        if (_grid[x + lenX][y + j] != 'X' || visited[x + lenX][y + j])
                        {
                            rowValid = false;
                            break;
                        }
                    }
                    if (!rowValid) break;
                    lenX++;
                }

                for (int dx = 0; dx < lenX; dx++)
                    for (int dy = 0; dy < lenY; dy++)
                        visited[x + dx][y + dy] = true;

                float cx = x * cellSize + (lenX - 1) * cellSize / 2 - offsetX * cellSize;
                float cy = y * cellSize + (lenY - 1) * cellSize / 2 - offsetY * cellSize;

                EntityId e = world->CreateEntity();
                m_Entities.push_back(e);
                TransformComponent& tWall = world->AddComponent<TransformComponent>(e);
                MeshRenderer& m = world->AddComponent<MeshRenderer>(e);
                m.geoId = RessourceManager::GetGeometryId("Cube");
                m.materialId = RessourceManager::GetMaterialId("WallMaterial");
                tWall.local.SetPosition(XMFLOAT3(cx, wallHeight * 0.5f, cy));
                tWall.local.SetScale(XMFLOAT3((float)lenX * cellSize, wallHeight, (float)lenY * cellSize));
                /*ColliderComponent& col = world->AddComponent<ColliderComponent>(e);
                PhysicComponent& phys = world->AddComponent<PhysicComponent>(e);
                phys.SetStatic();*/
            }
            if (_grid[x][y] == 'D')
            {
                EntityId doorEntity = world->CreateEntity();
                m_Entities.push_back(doorEntity);
                m_doors[doorCount] = doorEntity;
                TransformComponent& tDoor = world->AddComponent<TransformComponent>(doorEntity);
                MeshRenderer& m = world->AddComponent<MeshRenderer>(doorEntity);
                m.geoId = RessourceManager::GetGeometryId("Cube");
                m.materialId = RessourceManager::GetMaterialId("DoorMaterial");
                tDoor.local.SetScale(XMFLOAT3(cellSize, wallHeight, cellSize));
                float cx = x * cellSize - offsetX * cellSize;
                float cy = y * cellSize - offsetY * cellSize;
                tDoor.local.SetPosition(XMFLOAT3(cx, wallHeight * 0.5f, cy));
                ColliderComponent& col = world->AddComponent<ColliderComponent>(doorEntity);
                PhysicComponent& phys = world->AddComponent<PhysicComponent>(doorEntity);
                phys.SetStatic();
                doorCount++;
            }
        }
    }


    EntityId ground = world->CreateEntity();
	m_groundEntity = ground;
    m_Entities.push_back(ground);
    TransformComponent& tGround = world->AddComponent<TransformComponent>(ground);
    MeshRenderer& m = world->AddComponent<MeshRenderer>(ground);
    m.geoId = RessourceManager::GetGeometryId("Cube");
    m.materialId = RessourceManager::GetMaterialId("GroundMaterial");
    tGround.local.SetPosition(XMFLOAT3(0.0f, -0.5f, 0.0f));
    tGround.local.SetScale(XMFLOAT3(gridW * cellSize, 1.0f, gridH * cellSize));
    ColliderComponent& col = world->AddComponent<ColliderComponent>(ground);
    PhysicComponent& phys = world->AddComponent<PhysicComponent>(ground);
    phys.SetStatic();
}

void MainScene::CreateLabyrinthe(int _width, int _height)
{
    std::vector<std::vector<char>> grid(_width, std::vector<char>(_height, ' '));

    Enclose(grid);

    int cx = _width / 2;
    int cy = _height / 2;
    if (cx % 2 == 0) cx--;
    if (cy % 2 == 0) cy--;

    int rW = 2, rH = 2;
    int rxMin = cx - rW; if (rxMin % 2 == 1) rxMin--;
    int rxMax = cx + rW; if (rxMax % 2 == 1) rxMax++;
    int ryMin = cy - rH; if (ryMin % 2 == 1) ryMin--;
    int ryMax = cy + rH; if (ryMax % 2 == 1) ryMax++;

    // Murs de séparation
    for (int i = 0; i < _width; i++)
    {
        grid[i][ryMin] = 'X';
        grid[i][ryMax] = 'X';
    }
    for (int j = 0; j < _height; j++)
    {
        grid[rxMin][j] = 'X';
        grid[rxMax][j] = 'X';
    }

    // 8 zones
    Recursive_division(grid, 0, rxMin, 0, ryMin);
    Recursive_division(grid, rxMax, _width - 1, 0, ryMin);
    Recursive_division(grid, 0, rxMin, ryMax, _height - 1);
    Recursive_division(grid, rxMax, _width - 1, ryMax, _height - 1);
    Recursive_division(grid, rxMin, rxMax, 0, ryMin);
    Recursive_division(grid, rxMin, rxMax, ryMax, _height - 1);
    Recursive_division(grid, 0, rxMin, ryMin, ryMax);
    Recursive_division(grid, rxMax, _width - 1, ryMin, ryMax);

    PierceWallH(grid, ryMin, 0, rxMin, _height);
    PierceWallH(grid, ryMin, rxMax, _width - 1, _height);

    PierceWallH(grid, ryMax, 0, rxMin, _height);
    PierceWallH(grid, ryMax, rxMax, _width - 1, _height);

    PierceWallV(grid, rxMin, 0, ryMin, _width);
    PierceWallV(grid, rxMin, ryMax, _height - 1, _width);

    PierceWallV(grid, rxMax, 0, ryMin, _width);
    PierceWallV(grid, rxMax, ryMax, _height - 1, _width);

    Lobby(grid, rxMin, rxMax, ryMin, ryMax);
    Laby3d(grid);
	SpawnItems(grid, 10, rxMin, rxMax, ryMin, ryMax);

    bfs_check(grid);
    printGrid(grid);

}

void MainScene::ReloadLabyrinthe(int _width, int _height)
{
    m_loadLaby = true;

    for (int i = 0; i < 4; i++)
        m_doors[i] = 0;

    DestroyLabyrinthe();
    CreateLabyrinthe(_width, _height);

    m_loadLaby = false;
}

void MainScene::DestroyLabyrinthe()
{
    for (EntityId e : m_Entities)
        world->DestroyEntity(e);

    m_Entities.clear();
	m_groundEntity = 0;
}
