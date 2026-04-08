#ifndef SCRIPT_MOVEMENT_H_DEFINED
#define SCRIPT_MOVEMENT_H_DEFINED

#include "../Engine/Engine.h"

class Movement
{
public:
	struct ScriptMovement : public IScript
	{
		void Awake() override;
		void Update(float dt) override;
		void OnCollision(EntityId _otherId) override;
		void Reload();
	private:
		float m_yaw;
		float m_pitch;
		bool m_cursorLocked;
		EntityId m_glowStick[3];
	};
};

#endif // !SCRIPT_MOVEMENT_H_DEFINED
