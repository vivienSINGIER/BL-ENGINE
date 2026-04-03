#include "GlowStick.h"


void GlowStick::Awake()
{
	m_scene = SceneManager::GetSceneWithName("MainScene")->GetId();

	TransformComponent& t = SceneManager::GetSceneWithId(m_scene)->world->AddComponent<TransformComponent>(entity);
	MeshRenderer& mr = SceneManager::GetSceneWithId(m_scene)->world->AddComponent<MeshRenderer>(entity);
	mr.geoId = RessourceManager::GetGeometryId("GlowStick");
	mr.materialId = RessourceManager::GetMaterialId("DoorMaterial");
	LightComponent& l = SceneManager::GetSceneWithId(m_scene)->world->AddComponent<LightComponent>(entity);
	l.type = LightType::Point;
	l.SetStrength(0.3f);
	l.SetPoint(1.0f, 10.0f);
	l.SetColor(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));

	SceneManager::GetSceneWithId(m_scene)->world->SetInactive(entity);
}

void GlowStick::Update(float _dt)
{

}

void GlowStick::DropGlowStick(float _x, float _y, float _z)
{
	SceneManager::GetSceneWithId(m_scene)->world->SetActive(entity);
	TransformComponent& t = SceneManager::GetSceneWithId(m_scene)->world->GetComponent<TransformComponent>(entity);
	t.local.SetPosition(XMFLOAT3(_x, _y, _z));
}

void GlowStick::ChangeColor(XMFLOAT4 _color)
{
	LightComponent& l = SceneManager::GetSceneWithId(m_scene)->world->GetComponent<LightComponent>(entity);
	l.SetColor(_color);
}