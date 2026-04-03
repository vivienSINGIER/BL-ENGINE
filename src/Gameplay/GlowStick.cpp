#include "GlowStick.h"


void GlowStick::Awake()
{

	TransformComponent& t = SceneManager::GetSceneWithId(sceneId)->world->AddComponent<TransformComponent>(entity);
	MeshRenderer& mr = SceneManager::GetSceneWithId(sceneId)->world->AddComponent<MeshRenderer>(entity);
	mr.geoId = RessourceManager::GetGeometryId("GlowStick");
	mr.materialId = RessourceManager::GetMaterialId("DoorMaterial");
	LightComponent& l = SceneManager::GetSceneWithId(sceneId)->world->AddComponent<LightComponent>(entity);
	l.type = LightType::Point;
	l.SetStrength(0.3f);
	l.SetPoint(1.0f, 10.0f);
	l.SetColor(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));

	SceneManager::GetSceneWithId(sceneId)->world->SetInactive(entity);
}

void GlowStick::Update(float _dt)
{

}

void GlowStick::DropGlowStick(float _x, float _y, float _z)
{
	SceneManager::GetSceneWithId(sceneId)->world->SetActive(entity);
	TransformComponent& t = SceneManager::GetSceneWithId(sceneId)->world->GetComponent<TransformComponent>(entity);
	t.local.SetPosition(XMFLOAT3(_x, _y, _z));
}

void GlowStick::ChangeColor(XMFLOAT4 _color)
{
	LightComponent& l = SceneManager::GetSceneWithId(sceneId)->world->GetComponent<LightComponent>(entity);
	l.SetColor(_color);
}