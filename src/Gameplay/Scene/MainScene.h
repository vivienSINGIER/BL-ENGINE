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

	XMFLOAT3 m_lightPos;
	float m_lightTravelDistance;
	float m_lightPosXStart;

	bool m_loadLaby;
	bool m_opened;

	bool m_isDay;
	bool m_isNight;
	float m_timer;
	float m_dayDuration;
	float m_nightDuration;
	XMFLOAT3 m_dayColorStart;
	
	float cellSize;

	bool m_started;


	//LABYRINTHE//
	
	void SetStarted(bool _started) { m_started = _started; }

	int m_doorOpenedNight;
	int m_levelSize;

	//LEVEL MANAGER//

	int m_levelNb;
	int m_nbPlayer;
};

#endif // !MAIN_SCENE_H_DEFINED
