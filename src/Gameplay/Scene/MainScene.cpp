#include "MainScene.h"
#include "../Gameplay/Script/ScriptMovement.h"
#include "../Gameplay/Script/LabyrintheS.h"

void MainScene::OnInit()
{
	m_loadLaby = false;

    RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
    uint32 shaderId = RessourceManager::AddShader("LitColored", ShaderFactory::CreateLitColored(EngineManager::GetDevice()));
    Material* white = RessourceManager::GetShader(shaderId)->CreateMaterial();
    RessourceManager::AddMaterial("White", white);
	RessourceManager::AddCamera("Default");

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

	ComponentRegistry::RegisterScript<Labyrinthe::LabyScript>();
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
	
	m_laby = world->CreateEntity();
	world->AddScript<Labyrinthe::LabyScript>(m_laby);
	
}

void MainScene::OnUpdate(float _dt)
{
	if(InputManager::IsKeyDown(H))
	{
		Labyrinthe::LabyScript& labyScript = world->GetScript<Labyrinthe::LabyScript>(m_laby);
		labyScript.SetStarted(true);
	}
	if(InputManager::IsKeyDown(J))
	{
		Labyrinthe::LabyScript& labyScript = world->GetScript<Labyrinthe::LabyScript>(m_laby);
		labyScript.SetStarted(false);
		labyScript.ReloadLabyrinthe(31, 31);
	}
}

void MainScene::OnStart()
{
	
}

void MainScene::OnEnd()
{
}
