#include "LabyrintheManager.h"
#include "LabyrintheHelper.h"

int      LabyrintheManager::m_cellSize = 0;
Scene* LabyrintheManager::m_scene = nullptr;
Vector<EntityId> LabyrintheManager::m_Entities;
int      LabyrintheManager::m_levelNb = 0;
EntityId LabyrintheManager::m_doors[4] = {};
int      LabyrintheManager::m_nbPlayer = 0;

void LabyrintheManager::Laby3d(Vector<Vector<char>>& _grid)
{

    float gridW = (float)_grid.size();
    float gridH = (float)_grid[0].size();

    float offsetX = gridW * 0.5f - 0.5f;
    float offsetY = gridH * 0.5f - 0.5f;

    float wallHeight = 15.0f;

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

                float cx = x * m_cellSize + (lenX - 1) * m_cellSize / 2 - offsetX * m_cellSize;
                float cy = y * m_cellSize + (lenY - 1) * m_cellSize / 2 - offsetY * m_cellSize;

                EntityId e = m_scene->world->CreateEntity();
                m_Entities.push_back(e);
                TransformComponent& tWall = m_scene->world->AddComponent<TransformComponent>(e);
                MeshRenderer& m = m_scene->world->AddComponent<MeshRenderer>(e);
                m.geoId = RessourceManager::GetGeometryId("Cube");
                m.materialId = RessourceManager::GetMaterialId("WallMaterial");
                tWall.local.SetPosition(XMFLOAT3(cx, wallHeight * 0.5f, cy));
                tWall.local.SetScale(XMFLOAT3((float)lenX * m_cellSize, wallHeight, (float)lenY * m_cellSize));
                /*ColliderComponent& col = world->AddComponent<ColliderComponent>(e);
                PhysicComponent& phys = world->AddComponent<PhysicComponent>(e);
                phys.SetStatic();*/
            }
            if (_grid[x][y] == 'D')
            {
                EntityId doorEntity = m_scene->world->CreateEntity();
                m_Entities.push_back(doorEntity);
                m_doors[doorCount] = doorEntity;
                TransformComponent& tDoor = m_scene->world->AddComponent<TransformComponent>(doorEntity);
                MeshRenderer& m = m_scene->world->AddComponent<MeshRenderer>(doorEntity);
                m.geoId = RessourceManager::GetGeometryId("Cube");
                m.materialId = RessourceManager::GetMaterialId("DoorMaterial");
                tDoor.local.SetScale(XMFLOAT3(m_cellSize, wallHeight, m_cellSize));
                float cx = x * m_cellSize - offsetX * m_cellSize;
                float cy = y * m_cellSize - offsetY * m_cellSize;
                tDoor.local.SetPosition(XMFLOAT3(cx, wallHeight * 0.5f, cy));
                ColliderComponent& col = m_scene->world->AddComponent<ColliderComponent>(doorEntity);
                PhysicComponent& phys = m_scene->world->AddComponent<PhysicComponent>(doorEntity);
                phys.SetStatic();
                doorCount++;
            }
        }
    }

    EntityId ground = m_scene->world->CreateEntity();
    m_Entities.push_back(ground);
    TransformComponent& tGround = m_scene->world->AddComponent<TransformComponent>(ground);
    MeshRenderer& m = m_scene->world->AddComponent<MeshRenderer>(ground);
    m.geoId = RessourceManager::GetGeometryId("Cube");
    m.materialId = RessourceManager::GetMaterialId("GroundMaterial");
    tGround.local.SetPosition(XMFLOAT3(0.0f, -0.5f, 0.0f));
    tGround.local.SetScale(XMFLOAT3(gridW * m_cellSize, 1.0f, gridH * m_cellSize));
    ColliderComponent& col = m_scene->world->AddComponent<ColliderComponent>(ground);
    PhysicComponent& phys = m_scene->world->AddComponent<PhysicComponent>(ground);
    phys.SetStatic();

}

void LabyrintheManager::Init(int _cellSize, Scene* _scene, int _nbPlayer)
{
    m_cellSize = _cellSize;
	m_scene = _scene;
	m_levelNb = 1;
	m_nbPlayer = _nbPlayer;
}

void LabyrintheManager::CreateLabyrinthe(int _width, int _height)
{
    std::vector<std::vector<char>> grid(_width, std::vector<char>(_height, ' '));

    LabyrintheHelper::Enclose(grid);

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
    LabyrintheHelper::Recursive_division(grid, 0, rxMin, 0, ryMin);
    LabyrintheHelper::Recursive_division(grid, rxMax, _width - 1, 0, ryMin);
    LabyrintheHelper::Recursive_division(grid, 0, rxMin, ryMax, _height - 1);
    LabyrintheHelper::Recursive_division(grid, rxMax, _width - 1, ryMax, _height - 1);
    LabyrintheHelper::Recursive_division(grid, rxMin, rxMax, 0, ryMin);
    LabyrintheHelper::Recursive_division(grid, rxMin, rxMax, ryMax, _height - 1);
    LabyrintheHelper::Recursive_division(grid, 0, rxMin, ryMin, ryMax);
    LabyrintheHelper::Recursive_division(grid, rxMax, _width - 1, ryMin, ryMax);

    LabyrintheHelper::PierceWallH(grid, ryMin, 0, rxMin, _height);
    LabyrintheHelper::PierceWallH(grid, ryMin, rxMax, _width - 1, _height);

    LabyrintheHelper::PierceWallH(grid, ryMax, 0, rxMin, _height);
    LabyrintheHelper::PierceWallH(grid, ryMax, rxMax, _width - 1, _height);

    LabyrintheHelper::PierceWallV(grid, rxMin, 0, ryMin, _width);
    LabyrintheHelper::PierceWallV(grid, rxMin, ryMax, _height - 1, _width);

    LabyrintheHelper::PierceWallV(grid, rxMax, 0, ryMin, _width);
    LabyrintheHelper::PierceWallV(grid, rxMax, ryMax, _height - 1, _width);

    LabyrintheHelper::Lobby(grid, rxMin, rxMax, ryMin, ryMax);
    Laby3d(grid);
    SpawnItems(grid, 3, rxMin, rxMax, ryMin, ryMax);

    LabyrintheHelper::bfs_check(grid);
    LabyrintheHelper::printGrid(grid);

}

void LabyrintheManager::SpawnItems(Vector<Vector<char>>& _grid, int _count, int _xMin, int _xMax, int _yMin, int _yMax)
{
    int width = _grid.size();
    int height = _grid[0].size();

    std::vector<std::pair<int, int>> emptyCells;

    _count += (m_levelNb - 1) * m_nbPlayer;

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            if (_grid[x][y] == ' ' && !(x >= _xMin && x <= _xMax && y >= _yMin && y <= _yMax))
                emptyCells.push_back({ x, y });
        }
    }

    for (int i = emptyCells.size() - 1; i > 0; i--)
        std::swap(emptyCells[i], emptyCells[LabyrintheHelper::RandomInt(0, i)]);

    int placed = 0;

    for (const auto& cell : emptyCells)
    {
        if (placed >= _count) break;
        int x = cell.first;
        int y = cell.second;
        EntityId itemEntity = m_scene->world->CreateEntity();
        m_Entities.push_back(itemEntity);
        TransformComponent& tItem = m_scene->world->AddComponent<TransformComponent>(itemEntity);
        MeshRenderer& m = m_scene->world->AddComponent<MeshRenderer>(itemEntity);
        m.geoId = RessourceManager::GetGeometryId("WaterBottle");
        m.materialId = RessourceManager::GetMaterialId("WaterBottleMaterial");

        ColliderComponent& col = m_scene->world->AddComponent<ColliderComponent>(itemEntity);
        PhysicComponent& phys = m_scene->world->AddComponent<PhysicComponent>(itemEntity);
        phys.SetStatic();

        float offsetX = _grid.size() * 0.5f * m_cellSize - m_cellSize / 2;
        float offsetY = _grid[0].size() * 0.5f * m_cellSize - m_cellSize / 2;
        tItem.local.SetPosition(XMFLOAT3(x * m_cellSize - offsetX, 1.0f, y * m_cellSize - offsetY));
        _grid[x][y] = 'I';
        placed++;
    }
}

void LabyrintheManager::DestroyLabyrinthe()
{
    for (EntityId e : m_Entities)
        m_scene->world->DestroyEntity(e);

    m_Entities.clear();
}

void LabyrintheManager::ReloadDoor()
{
    for (int i = 0; i < 4; i++)
        m_doors[i] = 0;
}

void LabyrintheManager::CloseDoor(int doorIndex)
{
    if (doorIndex < 0 || doorIndex >= 4) return;
    if (m_doors[doorIndex] != 0)
		m_scene->world->SetActive(m_doors[doorIndex]);
}

void LabyrintheManager::OpenDoor(int doorIndex)
{
    if (doorIndex < 0 || doorIndex >= 4) return;
    if (m_doors[doorIndex] != 0)
        m_scene->world->SetInactive(m_doors[doorIndex]);
}
