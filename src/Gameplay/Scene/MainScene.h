#ifndef MAIN_SCENE_H_DEFINED
#define MAIN_SCENE_H_DEFINED

#include "../Engine/Engine.h"

class MainScene : public Scene
{
public:
	void OnInit() override;
	void OnUpdate(float _dt) override;
	void OnStart() override;
	void OnEnd() override;
	void LoadRessources() override;

private:
	EntityId m_camera;
	EntityId m_test;
	EntityId m_laby;
	EntityId m_light;

	EntityId m_player[4];

	XMFLOAT3 m_lightPos;
	float m_lightTravelDistance;
	float m_lightPosXStart;

	bool m_opened;

	bool m_isDay;
	bool m_isNight;
	bool m_gasStarted;
	float m_timer;
	float m_dayDuration;
	float m_nightDuration;
	XMFLOAT3 m_dayColorStart;
	
	bool m_started;
	
	void SetStarted(bool _started) { m_started = _started; }

	int m_levelSize;

};

#endif // !MAIN_SCENE_H_DEFINED
