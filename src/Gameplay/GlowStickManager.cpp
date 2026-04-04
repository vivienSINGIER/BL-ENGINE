#include "GlowStickManager.h"

Vector<GlowStickEntry> GlowStickManager::m_glowSticks;
Scene* GlowStickManager::m_scene = nullptr;

void GlowStickManager::Init(Scene* scene)
{
	m_scene = scene;
}

void GlowStickManager::Clear()
{
	m_glowSticks.clear();
}

void GlowStickManager::AddGlowStick(EntityId _entity, float _x, float _y, float _z)
{
	m_glowSticks.push_back({ _entity, _x, _y, _z, true });
}

void GlowStickManager::UpdatePosition(EntityId _entity, float _x, float _y, float _z)
{
	for (GlowStickEntry& entry : m_glowSticks)
	{
		if (entry.entity == _entity)
		{
			entry.x = _x;
			entry.y = _y;
			entry.z = _z;
			return;
		}
	}
}

int GlowStickManager::FindClosestGlowStick(float _x, float _z, float _radius)
{
	int closestIndex = -1;
	float closestDistSq = _radius * _radius;
	for (int i = 0; i < (int)m_glowSticks.size(); i++)
	{
		if (!m_glowSticks[i].active)
			continue;
		float dx = m_glowSticks[i].x - _x;
		float dz = m_glowSticks[i].z - _z;
		float distSq = dx * dx + dz * dz;
		if (distSq < closestDistSq)
		{
			closestDistSq = distSq;
			closestIndex = i;
		}
	}
	return closestIndex;
}

GlowStickEntry& GlowStickManager::GetEntry(int _index)
{
	return m_glowSticks[_index];
}
