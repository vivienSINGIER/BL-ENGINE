#ifndef CAM_PITCH_SCRIPT_H_DEFINED
#define CAM_PITCH_SCRIPT_H_DEFINED

#include "../Engine/Engine.h"

struct CamPitchScript : public IScript
{
	void Awake() override;
	void Update(float dt) override;

private:
	float m_pitch;
	bool m_cursorLocked;
};

#endif // !CAM_PITCH_SCRIPT_H_DEFINED


