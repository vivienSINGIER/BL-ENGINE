#ifndef GLOW_STICK_MANAGER_H_DEFINED
#define GLOW_STICK_MANAGER_H_DEFINED

#include "../Engine/Engine.h"

struct GlowStickEntry
{
	EntityId entity;
	float x, y, z;
	bool active;
};

class GlowStickManager
{
public:
	static void Init(Scene* scene);
	static void Clear();

	static void AddGlowStick(EntityId _entity, float _x, float _y, float _z);
	static void UpdatePosition(EntityId _entity, float _x, float _y, float _z);

	static int FindClosestGlowStick(float _x, float _z, float _radius);
	static GlowStickEntry& GetEntry(int _index);
	static int Count() { return (int)m_glowSticks.size(); }

private:
	static Vector<GlowStickEntry> m_glowSticks;
	static Scene* m_scene;
};
#endif // !GLOW_STICK_MANAGER_H_DEFINED


