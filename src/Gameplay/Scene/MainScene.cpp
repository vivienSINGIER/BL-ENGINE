#include "MainScene.h"
#include <iostream>
#include "../Gameplay/Script/ScriptMovement.h"
#include "../Gameplay/LevelManager.h"
#include "../Gameplay/GlowStick.h"
#include "../Gameplay/GameManager.h"
#include "../Gameplay/Script/Magu.h"
#include "../Gameplay/Script/CamPitchScript.h"
#include "../Gameplay/Script/FoodStorageScript.h"
#include "../Gameplay/Script/PlayerHealth.hpp"
#include "../Gameplay/GasManager.h"
#include "../Gameplay/InventoryManager.h"
#include "../Gameplay/ItemManager.h"

void MainScene::OnInit()
{
	m_dayDuration = 5.0f;
	m_nightDuration = 5.0f;
	m_opened = false;
	m_started = false;
	m_isDay = true;
	m_isNight = false;
	m_gasStarted = false;

	m_dayColorStart = XMFLOAT3(1.0f, 0.7f, 0.5f);

	ComponentRegistry::RegisterScript<Movement::ScriptMovement>();
	ComponentRegistry::RegisterScript<GlowStick>();
    ComponentRegistry::RegisterScript<Magu>();
	ComponentRegistry::RegisterScript<CamPitchScript>();
	ComponentRegistry::RegisterScript<FoodStorageScript>();

	ComponentRegistry::RegisterComponent<ItemCollectableComponent>();
	ComponentRegistry::RegisterComponent<PlayerHealthComponent>();

	m_playerCube = world->CreateEntity();
	m_player[0] = m_playerCube;
	TransformComponent& pt = world->AddComponent<TransformComponent>(m_playerCube);
	pt.local.SetPosition(XMFLOAT3(0.0f, 1.0f, 0.0f));
	pt.world.LookTo(XMFLOAT3(-1.0f, -1.0f, -1.0f));
	ColliderComponent& col = world->AddComponent<ColliderComponent>(m_playerCube);
	RigidBodyComponent& rbP = world->AddComponent<RigidBodyComponent>(m_playerCube);
	rbP.SetMass(1000.0f);
	rbP.allowRotation = false;
	rbP.useGravity = true;
	MotionComponent& motion = world->AddComponent<MotionComponent>(m_playerCube);
	world->AddScript<Movement::ScriptMovement>(m_playerCube);
	PlayerHealthComponent& health = world->AddComponent<PlayerHealthComponent>(m_playerCube);
	health.maxHealth = 100.0f;
	health.Reset();

    m_camera = world->CreateEntity();
    TransformComponent& t = world->AddComponent<TransformComponent>(m_camera);
    t.local.SetPosition(XMFLOAT3(0.0f, 0.5f, 0.0f));
    CameraComponent& cam = world->AddComponent<CameraComponent>(m_camera);
    cam.camId = RessourceManager::GetCameraId("Default");
    cam.isMainCamera = true;
	t.SetParent(m_playerCube);
	world->AddScript<CamPitchScript>(m_camera);

	m_light = world->CreateEntity();
	TransformComponent& lt = world->AddComponent<TransformComponent>(m_light);
	lt.local.SetPosition(XMFLOAT3(0.0f, 30.0f,0.0f));
	LightComponent& l = world->AddComponent<LightComponent>(m_light);
	l.type = LightType::Point;
	l.SetStrength(1.0f);
	l.SetPoint(1.0f, 500.0f);
	l.SetColor(XMFLOAT4(1.0f,0.8f,0.35f, 1.0f));
	
}

void MainScene::OnUpdate(float _dt)
{
    if(m_skipNextFrame)
    {
        m_skipNextFrame = false;
        return;
	}

    TransformComponent& camT = world->GetComponent<TransformComponent>(m_camera);
    TransformComponent& lt = world->GetComponent<TransformComponent>(m_light);
	LightComponent& l = world->GetComponent<LightComponent>(m_light);


	if(InputManager::IsKeyDown(H))
	{
		m_started = true;
	}
    if (InputManager::IsKeyDown(J))
    {
        GameManager::CollectFood();
    }


    if (m_started == true)
    {
        if (m_isDay == true)
        {
			m_lightPosXStart = LevelManager::GetLightPosXStart();
			m_lightTravelDistance = LevelManager::GetLightTravelDistance();
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
				LevelManager::OpenDoor(i);
            
			lt.local.SetPosition(XMFLOAT3(m_lightPos.x, m_lightPos.y, 0.0f));


            if(m_timer >= m_dayDuration)
            {
                m_timer = 0.0f;
                m_isDay = false;
                m_isNight = true;
				m_opened = false;
                for (int i = 0; i < 4; i++)
                    LevelManager::CloseDoor(i);
			}
        }
        else if (m_isNight == true)
        {
            m_timer += _dt;
			l.SetStrength(0.0f);
            if(m_gasStarted == false)
            {
                LevelManager::StartGas(m_nightDuration);
                m_gasStarted = true;
			}

			LevelManager::NightGas(_dt, m_nightDuration);
            if (m_opened == false)
            {
                LevelManager::OpenRandomDoor();
            }

			GasManager::Update(_dt, m_timer, m_nightDuration, this, m_player, 1);

            if (world->HasComponent<PlayerHealthComponent>(m_playerCube))
            {
                PlayerHealthComponent& hp = world->GetComponent<PlayerHealthComponent>(m_playerCube);
                if (hp.isDead)
                {
                    std::cout << "You died! Restarting level..." << std::endl;
                    hp.Reset();
                    InventoryManager::ResetInventory();
                    PreserveInventory();
                    GameManager::Reset();
                    LevelManager::ResetGas();
                    LevelManager::ReloadLevel();
					m_skipNextFrame = true;
                    world->GetScript<Movement::ScriptMovement>(m_playerCube).Reload();
                    m_timer = 0.0f;
                    m_isDay = true;
                    m_isNight = false;
                    m_gasStarted = false;
                    return;
                }
            }

            if (m_timer >= m_nightDuration)
            {
                
                GameManager::TryValidateQuota(camT.local.GetPosition());

                m_timer = 0.0f;
                m_isDay = true;
                m_isNight = false;
				m_opened = true;
				m_gasStarted = false;
				LevelManager::ResetGas();

				GameManager::NextLevel();

                if(GameManager::GetGameState() == GameState::LOSE)
                {
                    std::cout << "You lost! Restarting level..." << std::endl;
					InventoryManager::ResetInventory();
                    PreserveInventory();
					GameManager::Reset();
                    LevelManager::ReloadLevel();
                    m_skipNextFrame = true;
                    world->GetScript<Movement::ScriptMovement>(m_playerCube).Reload();
                    return;
                }

				PreserveInventory();
                LevelManager::LoadLevel();
                ReRegisterInventoryItem();
				world->GetScript<Movement::ScriptMovement>(m_playerCube).Reload();
            }
        }
    }
}

void MainScene::OnStart()
{
	LevelManager::Init(4);
	GameManager::Init(4);
    LevelManager::SetPlayer(0, m_playerCube);

	m_skipNextFrame = true;
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
	RessourceManager::AddGeometry("Medic", GeometryFactory::LoadGeometry(EngineManager::GetDevice(), "../../res/Obj/Medic.obj"));
	RessourceManager::AddGeometry("GlowStick", GeometryFactory::LoadGeometry(EngineManager::GetDevice(), "../../res/Obj/GlowStick.obj"));

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

    Texture* waterBottleTexture = EngineManager::GetDevice()->CreateTexture(L"../../res/Textures/Obj/Water.dds");
    RessourceManager::AddTexture("WaterBottle", waterBottleTexture);
    Material* waterBottleMat = RessourceManager::GetShader(shaderTextId)->CreateMaterial();
    waterBottleMat->SetTexture("Albedo", RessourceManager::GetTexture("WaterBottle"));
    RessourceManager::AddMaterial("WaterBottleMaterial", waterBottleMat);

	Texture* medicTexture = EngineManager::GetDevice()->CreateTexture(L"../../res/Textures/Obj/Medic.dds");
    RessourceManager::AddTexture("Medic", medicTexture);
    Material* medicMat = RessourceManager::GetShader(shaderTextId)->CreateMaterial();
    medicMat->SetTexture("Albedo", RessourceManager::GetTexture("Medic"));
	RessourceManager::AddMaterial("MedicMaterial", medicMat);

	Texture* gasTexture = EngineManager::GetDevice()->CreateTexture(L"../../res/Textures/Obj/gas.dds");
	RessourceManager::AddTexture("Gas", gasTexture);
	Material* gasMat = RessourceManager::GetShader(shaderTextId)->CreateMaterial();
	gasMat->SetTexture("Albedo", RessourceManager::GetTexture("Gas"));
	RessourceManager::AddMaterial("GasMaterial", gasMat);
}

void MainScene::PreserveInventory()
{
	LevelManager::UnregisterEntity(InventoryManager::GetInventory(), InventoryManager::GetItemCount());

	Scene* s = this;

    for(int i = 0; i < InventoryManager::GetItemCount(); i++)
    {
        EntityId item = InventoryManager::GetInventory()[i];
        if (item == 0) continue;
        if (!s->world->HasComponent<TransformComponent>(item)) continue;
        TransformComponent& t = s->world->GetComponent<TransformComponent>(item);
        t.RemoveParent();
        t.local.SetPosition(XMFLOAT3(0.0f, -100.0f, 0.0f));
        s->world->SetInactive(item);
	}
}

void MainScene::ReRegisterInventoryItem()
{
    for(int i = 0; i < InventoryManager::GetItemCount(); i++)
    {
		EntityId item = InventoryManager::GetInventory()[i];
        if(item != 0)
			ItemManager::RegisterItem(item);
	}
}
