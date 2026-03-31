#include "MainScene.h"
#include "Labyrinthe.hpp"
#include "../Gameplay/Script/ScriptMovement.h"

void MainScene::OnInit()
{
    RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
    uint32 shaderId = RessourceManager::AddShader("LitColored", ShaderFactory::CreateLitColored(EngineManager::GetDevice()));
    Material* white = RessourceManager::GetShader(shaderId)->CreateMaterial();
    RessourceManager::AddMaterial("White", white);
	RessourceManager::AddCamera("Default");

	ComponentRegistry::RegisterScript<Labyrinthe::LabyScript>();
	ComponentRegistry::RegisterScript<Movement::ScriptMovement>();

	m_camera = world->CreateEntity();
	TransformComponent& t = world->AddComponent<TransformComponent>(m_camera);
	t.world.SetPosition(XMFLOAT3(0.0f, 4.0f, -19.0f));
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
	
	EntityId e = world->CreateEntity();
	world->AddScript<Labyrinthe::LabyScript>(e);
}

void MainScene::OnUpdate(float _dt)
{

}

void MainScene::OnStart()
{
}

void MainScene::OnEnd()
{
}
