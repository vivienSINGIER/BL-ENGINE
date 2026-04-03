#ifndef GLOW_STICK_H_DEFINED
#define GLOW_STICK_H_DEFINED

#include "../Engine/Engine.h"


struct GlowStick : public IScript
{
public:
	void Awake() override;
	void Update(float _dt) override;

	void DropGlowStick(float _x, float _y, float _z);
	void ChangeColor(XMFLOAT4 _color);

private:
	uint32 m_scene;

};
#endif // !GLOW_STICK_H_DEFINED
