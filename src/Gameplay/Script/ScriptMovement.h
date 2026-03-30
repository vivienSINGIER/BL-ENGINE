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
	private:
		float m_yaw;
		float m_pitch;
	};
};

#endif // !SCRIPT_MOVEMENT_H_DEFINED
