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

		EntityId GetHealthText() const { return m_healthText; }
	private:
		float m_yaw;
		float m_pitch;
		bool m_cursorLocked;
		EntityId m_glowStick[3];
		EntityId m_itemInHand = 0;
		float m_throwStrength = 10.0f;
		bool m_isGrounded = false;
		bool m_jumpConsumed = false;

		EntityId m_healthText;
	};
};

#endif // !SCRIPT_MOVEMENT_H_DEFINED
