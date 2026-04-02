#include "MainScene.h"
#include <iostream>
#include "../Gameplay/Script/ScriptMovement.h"
#include "../Gameplay/LevelManager.h"

void MainScene::OnInit()
{
	m_dayDuration = 20.0f;
	m_nightDuration = 5.0f;
	m_opened = false;
	m_started = false;
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
	l.SetColor(XMFLOAT4(1.0f,0.8f,0.35f, 1.0f));
	
}

void MainScene::OnUpdate(float _dt)
{

    TransformComponent& lt = world->GetComponent<TransformComponent>(m_light);
	LightComponent& l = world->GetComponent<LightComponent>(m_light);

	if(InputManager::IsKeyDown(H))
	{
		m_started = true;
	}
	if(InputManager::IsKeyDown(J))
	{
		LevelManager::LoadLevel();
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
                for (int i = 0; i < 4; i++)
                    LevelManager::CloseDoor(i);
			}
        }
        else if (m_isNight == true)
        {
            m_timer += _dt;
			l.SetStrength(0.0f);

            if (m_timer >= m_nightDuration)
            {
                m_timer = 0.0f;
                m_isDay = true;
                m_isNight = false;
                LevelManager::LoadLevel();

            }
        }
    }
}

void MainScene::OnStart()
{
	LevelManager::Init(4);
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
