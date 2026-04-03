#include "Magu.h"
#include "GlowStickManager.h"

void Magu::Awake()
{
	
	TransformComponent& t = SceneManager::GetSceneWithId(sceneId)->world->AddComponent<TransformComponent>(entity);
	t.local.SetPosition(XMFLOAT3(0.0f, 0.5f, 0.0f));

	MeshRenderer& mr = SceneManager::GetSceneWithId(sceneId)->world->AddComponent<MeshRenderer>(entity);
	mr.geoId = RessourceManager::GetGeometryId("Cube");
	mr.materialId = RessourceManager::GetMaterialId("White");

	PickNewWanderTarget();
}

void Magu::Update(float _dt)
{
	switch (m_state)
	{
	case MaguState::WANDERING:
	{
		MoveTowards(m_targetPosition, _dt);
		m_wanderTimer += _dt;

		if (ReachedTagert(m_targetPosition) || m_wanderTimer >= m_wanderInterval)
			PickNewWanderTarget();

		XMFLOAT3 pos = GetMyPos();
		int idx = GlowStickManager::FindClosestGlowStick(pos.x, pos.z, detectionRadius);
		if (idx >= 0 && IsPlayerFarEnough(GlowStickManager::GetEntry(idx).x, GlowStickManager::GetEntry(idx).z))
		{
			m_state = MaguState::STEALING;
			m_targetStickIndex = idx;
			GlowStickManager::GetEntry(idx).active = false;
		}
		break;
	}

	case MaguState::STEALING:
	{
		auto& entry = GlowStickManager::GetEntry(m_targetStickIndex);
		XMFLOAT3 dest = { entry.x, 0.5f, entry.z };
		MoveTowards(m_targetPosition, _dt);

		if (ReachedTagert(dest, 1.0f))
		{
			SceneManager::GetSceneWithId(sceneId)->world->SetInactive(entry.entity);
			m_carryTimer = 0.0f;
			m_state = MaguState::CARRYING;
			PickNewWanderTarget();
		}
		break;
	}

	case MaguState::CARRYING:
	{
		MoveTowards(m_targetPosition, _dt);
		m_carryTimer += _dt;

		if (m_carryTimer >= carryDuration || ReachedTagert(m_targetPosition))
			m_state = MaguState::DROPING;

		break;
	}

	case MaguState::DROPING:
	{
		XMFLOAT3 pos = GetMyPos();
		auto& entry = GlowStickManager::GetEntry(m_targetStickIndex);

		TransformComponent& t = SceneManager::GetSceneWithId(sceneId)->world->GetComponent<TransformComponent>(entry.entity);
		t.local.SetPosition(XMFLOAT3(pos.x, 0.5f, pos.z));
		entry.x = pos.x;
		entry.z = pos.z;
		entry.active = true;

		SceneManager::GetSceneWithId(sceneId)->world->SetActive(entry.entity);
		m_targetStickIndex = -1;

		PickNewWanderTarget();
		m_state = MaguState::WANDERING;
		break;
	}
	}
}

void Magu::SetSizeLabyrinthe(float _size)
{
	m_labyrintheSize = _size;
}

void Magu::PickNewWanderTarget()
{
	float angle = ((float)(rand() % 628)) * 0.01f; 
	float dist = 4.0f + (float)(rand() % (int)m_labyrintheSize);
	XMFLOAT3 pos = GetMyPos();
	m_targetPosition = { pos.x + cosf(angle) * dist, 0.5f, pos.z + sinf(angle) * dist };
	m_wanderTimer = 0.0f;
	m_wanderInterval = 2.0f + (float)(rand() % 30) * 0.1f;
}

void Magu::MoveTowards(XMFLOAT3 _dest, float _dt)
{
	auto* world = SceneManager::GetSceneWithId(sceneId)->world;
	TransformComponent& t = world->GetComponent<TransformComponent>(entity);
	XMFLOAT3 pos = t.local.GetPosition();

	float dx = _dest.x - pos.x;
	float dz = _dest.z - pos.z;
	float len = sqrtf(dx * dx + dz * dz);
	if (len < 0.01f) return;

	float step = moveSpeed * _dt;
	if (step > len) step = len;

	pos.x += (dx / len) * step;
	pos.z += (dz / len) * step;
	t.local.SetPosition(pos);
}

bool Magu::ReachedTagert(XMFLOAT3 _dest, float _threshold)
{
	XMFLOAT3 pos = GetMyPos();
	float dx = _dest.x - pos.x;
	float dz = _dest.z - pos.z;
	return (dx * dx + dz * dz) <= (_threshold * _threshold);
}

XMFLOAT3 Magu::GetMyPos()
{
	auto* world = SceneManager::GetSceneWithId(sceneId)->world;
	return world->GetComponent<TransformComponent>(entity).local.GetPosition();
}

bool Magu::IsPlayerFarEnough(float _x, float _z)
{

}
