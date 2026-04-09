#include "LabyrintheManager.h"
#include "LabyrintheHelper.h"
#include "ItemManager.h"
#include <iostream>
#include "../Gameplay/Script/Magu.h"
#include "GlowStickManager.h"
#include "../Gameplay/Script/FoodStorageScript.h"
#include "../Gameplay/GasManager.h"

float LabyrintheManager::m_cellSize = 0;
Scene* LabyrintheManager::m_scene = nullptr;
Vector<EntityId> LabyrintheManager::m_Entities;
int LabyrintheManager::m_levelNb = 0;
EntityId LabyrintheManager::m_doors[4] = {};
int LabyrintheManager::m_nbPlayer = 0;
int LabyrintheManager::m_doorOpenedNight = 0;
float LabyrintheManager::m_gasWallTargetX[4] = {};
float LabyrintheManager::m_gasWallStartX[4] = {};
float LabyrintheManager::m_gasTimer = 0.0f;
float LabyrintheManager::m_gasDuration = 0.0f;
EntityId LabyrintheManager::m_wallsGas[4] = {};
Vector<EntityId> LabyrintheManager::m_MaguEntities;
EntityId LabyrintheManager::m_player[4] = {};
float LabyrintheManager::m_labySize = 0.0f;

void LabyrintheManager::Laby3d(Vector<Vector<char>>& _grid)
{
    float gridW = (float)_grid.size();
    float gridH = (float)_grid[0].size();

    float offsetX = gridW * 0.5f - 0.5f;
    float offsetY = gridH * 0.5f - 0.5f;

    float wallHeight = 15.0f;
    int doorCount = 0;

    float halfW = gridW * m_cellSize * 0.5f;
    float halfH = gridH * m_cellSize * 0.5f;
    float gasHeight = 20.0f;
    float gasThickness = m_cellSize;

    m_gasWallStartX[0] = -halfW;
    m_gasWallStartX[1] = halfW;
    m_gasWallStartX[2] = -halfH;
    m_gasWallStartX[3] = halfH;

    XMFLOAT3 startPos[4] = {
        XMFLOAT3(-halfW, gasHeight * 0.5f, 0.0f),
        XMFLOAT3(halfW, gasHeight * 0.5f, 0.0f),
        XMFLOAT3(0.0f,  gasHeight * 0.5f, -halfH),
        XMFLOAT3(0.0f,  gasHeight * 0.5f,  halfH),
    };

    XMFLOAT3 gasScale[4] = {
        XMFLOAT3(gasThickness, gasHeight, gridH * m_cellSize + gasThickness),
        XMFLOAT3(gasThickness, gasHeight, gridH * m_cellSize + gasThickness),
        XMFLOAT3(gridW * m_cellSize + gasThickness, gasHeight, gasThickness),
        XMFLOAT3(gridW * m_cellSize + gasThickness, gasHeight, gasThickness),
    };

    for (int i = 0; i < 4; i++)
    {
        EntityId gasWall = m_scene->world->CreateEntity();
        m_Entities.push_back(gasWall);
        m_wallsGas[i] = gasWall;
        TransformComponent& tGas = m_scene->world->AddComponent<TransformComponent>(gasWall);
        MeshRenderer& mr = m_scene->world->AddComponent<MeshRenderer>(gasWall);
        mr.geoId = RessourceManager::GetGeometryId("Cube");
        mr.materialId = RessourceManager::GetMaterialId("GasMaterial");
        tGas.local.SetPosition(startPos[i]);
        tGas.local.SetScale(gasScale[i]);
        m_scene->world->SetInactive(gasWall);
    }

    Vector<Vector<bool>> visited(_grid.size(), Vector<bool>(_grid[0].size(), false));

    for (int x = 0; x < (int)_grid.size(); x++)
    {
        for (int y = 0; y < (int)_grid[0].size(); y++)
        {
            if (_grid[x][y] == 'X' && !visited[x][y])
            {
                int lenY = 1;
                while (y + lenY < (int)_grid[0].size() && _grid[x][y + lenY] == 'X' && !visited[x][y + lenY])
                    lenY++;

                int lenX = 1;
                while (x + lenX < (int)_grid.size())
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

                float cx = x * m_cellSize + (lenX - 1) * m_cellSize / 2.0f - offsetX * m_cellSize;
                float cy = y * m_cellSize + (lenY - 1) * m_cellSize / 2.0f - offsetY * m_cellSize;

                EntityId e = m_scene->world->CreateEntity();
                m_Entities.push_back(e);
                TransformComponent& tWall = m_scene->world->AddComponent<TransformComponent>(e);
                MeshRenderer& mr = m_scene->world->AddComponent<MeshRenderer>(e);
                mr.geoId = RessourceManager::GetGeometryId("Cube");
                mr.materialId = RessourceManager::GetMaterialId("WallMaterial");
                tWall.local.SetPosition(XMFLOAT3(cx, wallHeight * 0.5f - 2.0f, cy));
                tWall.local.SetScale(XMFLOAT3((float)lenX * m_cellSize, wallHeight, (float)lenY * m_cellSize));
				ColliderComponent& col = m_scene->world->AddComponent<ColliderComponent>(e);
				RigidBodyComponent& rbW = m_scene->world->AddComponent<RigidBodyComponent>(e);
				rbW.SetStatic();
            }

            if (_grid[x][y] == 'D')
            {
                EntityId doorEntity = m_scene->world->CreateEntity();
                m_Entities.push_back(doorEntity);
                m_doors[doorCount] = doorEntity;
                TransformComponent& tDoor = m_scene->world->AddComponent<TransformComponent>(doorEntity);
                MeshRenderer& mr = m_scene->world->AddComponent<MeshRenderer>(doorEntity);
                mr.geoId = RessourceManager::GetGeometryId("Cube");
                mr.materialId = RessourceManager::GetMaterialId("DoorMaterial");
                tDoor.local.SetScale(XMFLOAT3(m_cellSize, wallHeight, m_cellSize));
                float cx = x * m_cellSize - offsetX * m_cellSize;
                float cy = y * m_cellSize - offsetY * m_cellSize;
                tDoor.local.SetPosition(XMFLOAT3(cx, wallHeight * 0.5 - 2.0f, cy));
                ColliderComponent& col = m_scene->world->AddComponent<ColliderComponent>(doorEntity);
				RigidBodyComponent& rbD = m_scene->world->AddComponent<RigidBodyComponent>(doorEntity);
				rbD.SetStatic();
                doorCount++;
            }
            if(_grid[x][y] == 'F')
            {
				EntityId foodStorage = m_scene->world->CreateEntity();
				m_Entities.push_back(foodStorage);
				TransformComponent& tFood = m_scene->world->AddComponent<TransformComponent>(foodStorage);
				MeshRenderer& mr = m_scene->world->AddComponent<MeshRenderer>(foodStorage);
				mr.geoId = RessourceManager::GetGeometryId("Cube");
				mr.materialId = RessourceManager::GetMaterialId("DoorMaterial");
				tFood.local.SetPosition(XMFLOAT3(x* m_cellSize - offsetX * m_cellSize, 0.0f, y* m_cellSize - offsetY * m_cellSize));
				ColliderComponent& col = m_scene->world->AddComponent<ColliderComponent>(foodStorage);
				col.SetBox(0.5f, 0.5f, 0.5f);
				col.isTrigger = true;
				m_scene->world->AddScript<FoodStorageScript>(foodStorage);
			}
        }
    }

    EntityId ground = m_scene->world->CreateEntity();
    m_Entities.push_back(ground);
    TransformComponent& tGround = m_scene->world->AddComponent<TransformComponent>(ground);
    MeshRenderer& mr = m_scene->world->AddComponent<MeshRenderer>(ground);
    mr.geoId = RessourceManager::GetGeometryId("Cube");
    mr.materialId = RessourceManager::GetMaterialId("GroundMaterial");
    tGround.local.SetPosition(XMFLOAT3(0.0f, -1.0f, 0.0f));
    tGround.local.SetScale(XMFLOAT3(gridW * m_cellSize, 2.0f, gridH * m_cellSize));
    ColliderComponent& col = m_scene->world->AddComponent<ColliderComponent>(ground);
    col.SetBox(0.5f, 0.5f, 0.5f);
	RigidBodyComponent& rbG = m_scene->world->AddComponent<RigidBodyComponent>(ground);
	rbG.SetStatic();
	rbG.dynamicFriction = 18.0f;
	rbG.staticFriction = 20.0f;

}

void LabyrintheManager::Init(int _cellSize, Scene* _scene, int _nbPlayer)
{
    m_cellSize = _cellSize;
	m_scene = _scene;
	m_levelNb = 1;
	m_nbPlayer = _nbPlayer;
	ItemManager::Init(_scene);
	GlowStickManager::Init(_scene);
}

void LabyrintheManager::CreateLabyrinthe(int _width, int _height)
{
    std::vector<std::vector<char>> grid(_width, std::vector<char>(_height, ' '));

    LabyrintheHelper::Enclose(grid);

	m_doorOpenedNight = LabyrintheHelper::RandomInt(0, 3);

    m_labySize = _width / 2;

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

    float offsetX = (_width * 0.5f - 0.5f) * m_cellSize;
    float offsetY = (_height * 0.5f - 0.5f) * m_cellSize;

    m_gasWallTargetX[0] = rxMin * m_cellSize - offsetX;
    m_gasWallTargetX[1] = rxMax * m_cellSize - offsetX;
    m_gasWallTargetX[2] = ryMin * m_cellSize - offsetY;
    m_gasWallTargetX[3] = ryMax * m_cellSize - offsetY;

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
	ItemManager::SpawnItems(grid, 15, rxMin, rxMax, ryMin, ryMax, m_cellSize, m_levelNb, m_nbPlayer);
	SpawnMagu(1 + m_levelNb / 2);

    LabyrintheHelper::bfs_check(grid);
    LabyrintheHelper::printGrid(grid);

	GasManager::SetGasWalls(m_gasWallStartX, m_gasWallTargetX);
}

void LabyrintheManager::DestroyLabyrinthe()
{
    ItemManager::DestroyItems();
    GlowStickManager::Clear(); 
    m_MaguEntities.clear();    
    for (EntityId e : m_Entities)
        m_scene->world->DestroyEntity(e);
    m_Entities.clear();

    SystemScheduler::Get().GetSystem<BroadPhaseSystem>()->ClearAll();
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

void LabyrintheManager::OpenRandomDoor()
{
    m_scene->world->SetInactive(m_doors[m_doorOpenedNight]);
}

void LabyrintheManager::NightGas(float _dt, float _nightDuration)
{
    m_gasTimer += _dt;
    float t = m_gasTimer / m_gasDuration;
    if (t > 1.0f) t = 1.0f;

    for (int i = 0; i < 4; i++)
    {
        if (m_wallsGas[i] == 0) continue;
        TransformComponent& tr = m_scene->world->GetComponent<TransformComponent>(m_wallsGas[i]);
        float pos = m_gasWallStartX[i] + (m_gasWallTargetX[i] - m_gasWallStartX[i]) * t;
        if (i < 2)
            tr.local.SetPosition(XMFLOAT3(pos, 10.0f, 0.0f));
        else
            tr.local.SetPosition(XMFLOAT3(0.0f, 10.0f, pos));
    }
}

void LabyrintheManager::StartGas(float _nightDuration)
{
    m_gasTimer = 0.0f;
    m_gasDuration = _nightDuration;
    for (int i = 0; i < 4; i++)
        if (m_wallsGas[i] != 0)
            m_scene->world->SetActive(m_wallsGas[i]);
}

void LabyrintheManager::ResetGas()
{
    m_gasTimer = 0.0f;
    for (int i = 0; i < 4; i++)
        if (m_wallsGas[i] != 0)
            m_scene->world->SetInactive(m_wallsGas[i]);
}

void LabyrintheManager::SpawnMagu(int _count)
{
    for (int i = 0; i < _count; i++)
    {
        EntityId e = m_scene->world->CreateEntity();
        m_MaguEntities.push_back(e);
        m_Entities.push_back(e);
        m_scene->world->AddScript<Magu>(e).SetSizeLabyrinthe(m_labySize);
    }
}

void LabyrintheManager::UnregisterEntity(EntityId* _entity, int _count)
{
    for(int i = 0; i < _count; i++)
    {
		if (_entity[i] == 0) continue;

        for(int j = 0; j < (int)m_Entities.size(); j++)
        {
            if (m_Entities[j] == _entity[i])
            {
                m_Entities.erase(m_Entities.begin() + j);
                break;
            }
		}
	}
}

void LabyrintheManager::SetPlayer(int playerIndex, EntityId playerEntity)
{
    if (playerIndex < 0 || playerIndex >= m_nbPlayer) return;
    m_player[playerIndex] = playerEntity;
    for (EntityId magu : m_MaguEntities)
    {
        Magu& maguScript = m_scene->world->GetScript<Magu>(magu);
		maguScript.SetPlayer(playerIndex, playerEntity);
    }
}