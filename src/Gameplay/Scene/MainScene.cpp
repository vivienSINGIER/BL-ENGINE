#include "MainScene.h"
#include <iostream>
#include "../Gameplay/Script/ScriptMovement.h"

void MainScene::OnInit()
{
	m_loadLaby = false;
	m_dayDuration = 20.0f;
	m_nightDuration = 5.0f;
	m_opened = false;
	m_started = false;
	m_levelNb = 1;
	m_nbPlayer = 4;
	m_isDay = true;
	m_isNight = false;

	m_dayColorStart = XMFLOAT3(1.0f, 0.7f, 0.5f);

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

	m_light = world->CreateEntity();
	TransformComponent& lt = world->AddComponent<TransformComponent>(m_light);
	lt.local.SetPosition(XMFLOAT3(0.0f, 30.0f,0.0f));
	LightComponent& l = world->AddComponent<LightComponent>(m_light);
	l.type = LightType::Point;
	l.SetStrength(1.0f);
	l.SetPoint(1.0f, 200.0f);
	
}

void MainScene::OnUpdate(float _dt)
{
    if (m_loadLaby == true) return;

    TransformComponent& lt = world->GetComponent<TransformComponent>(m_light);
	LightComponent& l = world->GetComponent<LightComponent>(m_light);

	if(InputManager::IsKeyDown(H))
	{
		m_started = true;
	}
	if(InputManager::IsKeyDown(J))
	{
		LoadLevel(m_levelNb, m_nbPlayer);
	}

    if (m_started == true)
    {
        if (m_isDay == true)
        {
            l.SetStrength(1.0f);
            m_timer += _dt;
			float t = m_timer / m_dayDuration;

            m_lightPos.x = m_lightPosXStart + t * m_lightTravelDistance;
			m_lightPos.y = 18.0f * sinf(t * XM_PI) + 18.0f;

            float yNormalized = m_lightPos.y / 36.0f;

            // Couleurs
            XMFLOAT3 sunsetColor = XMFLOAT3(1.0f, 0.5f, 0.2f);   // orange
            XMFLOAT3 noonColor = XMFLOAT3(1.0f, 0.95f, 0.8f);  // jaune/blanc chaud

            l.SetColor(XMFLOAT4(
                sunsetColor.x * (1.0f - yNormalized) + noonColor.x * yNormalized,
                sunsetColor.y * (1.0f - yNormalized) + noonColor.y * yNormalized,
                sunsetColor.z * (1.0f - yNormalized) + noonColor.z * yNormalized,
                1.0f
            ));

            for (int i = 0; i < 4; i++)
                if (m_doors[i] != 0)
                    world->SetInactive(m_doors[i]);
            
			lt.local.SetPosition(XMFLOAT3(m_lightPos.x, m_lightPos.y, 0.0f));
			std::cout << "Light position: " << m_lightPos.x << ", " << m_lightPos.y << ", " << 0.0f << std::endl;

            if(m_timer >= m_dayDuration)
            {
                m_timer = 0.0f;
                m_isDay = false;
                m_isNight = true;
                for (int i = 0; i < 4; i++)
                    if (m_doors[i] != 0)
                        world->SetActive(m_doors[i]);
			}
        }
        else if (m_isNight == true)
        {
            m_timer += _dt;
			l.SetStrength(0.0f);
			world->SetInactive(m_doors[m_doorOpenedNight]);

            if (m_timer >= m_nightDuration)
            {
                m_timer = 0.0f;
                m_isDay = true;
                m_isNight = false;
				LoadLevel(m_levelNb, m_nbPlayer);

            }
        }
    }
}

void MainScene::OnStart()
{
    LoadLevel(m_levelNb, m_nbPlayer);
}

void MainScene::OnEnd()
{
}

void MainScene::LoadRessources()
{
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
}



void MainScene::SpawnItems(Vector<Vector<char>>& _grid, int _count,int _xMin, int _xMax, int _yMin, int _yMax)
{
	int width = _grid.size();
	int height = _grid[0].size();

	std::vector<std::pair<int, int>> emptyCells;

    _count += (m_levelNb - 1) * m_nbPlayer;

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

void MainScene::LoadLevel(int levelNb, int nbPlayer)
{
    cellSize = 3.0f;
	m_doorOpenedNight = RandomInt(0, 3);
	m_levelSize = 21 + ((levelNb - 1) * 2 * nbPlayer);

	if (m_levelSize > 51) m_levelSize = 51;

	m_lightPosXStart = -m_levelSize * cellSize * 0.5f - 5.0f;

	m_lightTravelDistance = m_levelSize * cellSize + 10.0f;

	ReloadLabyrinthe(m_levelSize, m_levelSize);
	std::cout << "Level " << levelNb << " loaded with " << nbPlayer << " player(s) " << "LevelSize " << m_levelSize << std::endl;
    std::cout << "Labyrinthe generated with " << m_Entities.size() << " entities.\n";
}

void MainScene::Laby3d(Vector<Vector<char>>& _grid)
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
	SpawnItems(grid, 3, rxMin, rxMax, ryMin, ryMax);

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
	m_levelNb++;
}

void MainScene::DestroyLabyrinthe()
{
    for (EntityId e : m_Entities)
        world->DestroyEntity(e);

    m_Entities.clear();
	m_groundEntity = 0;
}
